# nw_s: dulwich/client.py |be55bb89dee5d5263cd5afab622663de#
# client.py -- Implementation of the client side git protocols
# Copyright (C) 2008-2013 Jelmer Vernooij <jelmer@samba.org>
#
# nw_s: dulwich license |4f4806644f741500c5d9caa7c853fdcc#
# Dulwich is dual-licensed under the Apache License, Version 2.0 and the GNU
# General Public License as public by the Free Software Foundation; version 2.0
# or (at your option) any later version. You can redistribute it and/or
# modify it under the terms of either of these two licenses.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# You should have received a copy of the licenses; if not, see
# <http://www.gnu.org/licenses/> for a copy of the GNU General Public License
# and <http://www.apache.org/licenses/LICENSE-2.0> for a copy of the Apache
# License, Version 2.0.
#
# nw_e: dulwich license #
"""Client side support for the Git protocol.

The Dulwich client supports the following capabilities:

 * thin-pack
 * multi_ack_detailed
 * multi_ack
 * side-band-64k
 * ofs-delta
 * quiet
 * report-status
 * delete-refs

Known capabilities that are not supported:

 * shallow
 * no-progress
 * include-tag
"""

from contextlib import closing
from io import BytesIO, BufferedReader
import dulwich
import select
import socket
import subprocess
import sys

try:
    from urllib import quote as urlquote
    from urllib import unquote as urlunquote
except ImportError:
    from urllib.parse import quote as urlquote
    from urllib.parse import unquote as urlunquote

try:
    import urllib2
    import urlparse
except ImportError:
    import urllib.request as urllib2
    import urllib.parse as urlparse

from dulwich.errors import (
    GitProtocolError,
    NotGitRepository,
    SendPackError,
    UpdateRefsError,
    )
from dulwich.protocol import (
    _RBUFSIZE,
    capability_agent,
    CAPABILITY_DELETE_REFS,
    CAPABILITY_MULTI_ACK,
    CAPABILITY_MULTI_ACK_DETAILED,
    CAPABILITY_OFS_DELTA,
    CAPABILITY_QUIET,
    CAPABILITY_REPORT_STATUS,
    CAPABILITY_SIDE_BAND_64K,
    CAPABILITY_THIN_PACK,
    CAPABILITIES_REF,
    COMMAND_DONE,
    COMMAND_HAVE,
    COMMAND_WANT,
    SIDE_BAND_CHANNEL_DATA,
    SIDE_BAND_CHANNEL_PROGRESS,
    SIDE_BAND_CHANNEL_FATAL,
    PktLineParser,
    Protocol,
    ProtocolFile,
    TCP_GIT_PORT,
    ZERO_SHA,
    extract_capabilities,
    )
from dulwich.pack import (
    write_pack_objects,
    )
from dulwich.refs import (
    read_info_refs,
    )


def _fileno_can_read(fileno):
    """Check if a file descriptor is readable."""
    return len(select.select([fileno], [], [], 0)[0]) > 0


# nw_s: constant client.COMMON_CAPABILITIES |e46abe912b898da5ad28de420070da24#
COMMON_CAPABILITIES = [CAPABILITY_OFS_DELTA, CAPABILITY_SIDE_BAND_64K]
# nw_e: constant client.COMMON_CAPABILITIES #
# nw_s: constant client.FETCH_CAPABILITIES |779d3eca9a9d0212ee09114a2137de55#
FETCH_CAPABILITIES = ([CAPABILITY_THIN_PACK, 
                       CAPABILITY_MULTI_ACK,
                       CAPABILITY_MULTI_ACK_DETAILED] +
                      COMMON_CAPABILITIES)
# nw_e: constant client.FETCH_CAPABILITIES #
# nw_s: constant client.SEND_CAPABILITIES |9e158364345d6f4167c7be2a11d43943#
SEND_CAPABILITIES = [CAPABILITY_REPORT_STATUS] + COMMON_CAPABILITIES
# nw_e: constant client.SEND_CAPABILITIES #


class ReportStatusParser(object):
    """Handle status as reported by servers with 'report-status' capability.
    """

    def __init__(self):
        self._done = False
        self._pack_status = None
        self._ref_status_ok = True
        self._ref_statuses = []

    def check(self):
        """Check if there were any errors and, if so, raise exceptions.

        :raise SendPackError: Raised when the server could not unpack
        :raise UpdateRefsError: Raised when refs could not be updated
        """
        if self._pack_status not in (b'unpack ok', None):
            raise SendPackError(self._pack_status)
        if not self._ref_status_ok:
            ref_status = {}
            ok = set()
            for status in self._ref_statuses:
                if b' ' not in status:
                    # malformed response, move on to the next one
                    continue
                status, ref = status.split(b' ', 1)

                if status == b'ng':
                    if b' ' in ref:
                        ref, status = ref.split(b' ', 1)
                else:
                    ok.add(ref)
                ref_status[ref] = status
            # TODO(jelmer): don't assume encoding of refs is ascii.
            raise UpdateRefsError(', '.join([
                ref.decode('ascii') for ref in ref_status if ref not in ok]) +
                ' failed to update', ref_status=ref_status)

    def handle_packet(self, pkt):
        """Handle a packet.

        :raise GitProtocolError: Raised when packets are received after a
            flush packet.
        """
        if self._done:
            raise GitProtocolError("received more data after status report")
        if pkt is None:
            self._done = True
            return
        if self._pack_status is None:
            self._pack_status = pkt.strip()
        else:
            ref_status = pkt.strip()
            self._ref_statuses.append(ref_status)
            if not ref_status.startswith(b'ok '):
                self._ref_status_ok = False


def read_pkt_refs(proto):
    server_capabilities = None
    refs = {}
    # Receive refs from server
    for pkt in proto.read_pkt_seq():
        (sha, ref) = pkt.rstrip(b'\n').split(None, 1)
        if sha == b'ERR':
            raise GitProtocolError(ref)
        if server_capabilities is None:
            (ref, server_capabilities) = extract_capabilities(ref)
        refs[ref] = sha

    if len(refs) == 0:
        return None, set([])
    if refs == {CAPABILITIES_REF: ZERO_SHA}:
        refs = {}
    return refs, set(server_capabilities)

# nw_s: class GitClient |d04bc6e79944a8a1533e0d4c28a4e2b5#
# TODO(durin42): this doesn't correctly degrade if the server doesn't
# support some capabilities. This should work properly with servers
# that don't support multi_ack.
class GitClient(object):
    """Git smart server client.

    """

    # nw_s: [[GitClient]] methods |d9461abbc1220843c739842a2c50053d#
    def __init__(self, thin_packs=True, report_activity=None, quiet=False):
        """Create a new GitClient instance.

        :param thin_packs: Whether or not thin packs should be retrieved
        :param report_activity: Optional callback for reporting transport
            activity.
        """
        self._report_activity = report_activity
        self._report_status_parser = None
        # nw_s: [[GitClient.__init__()]] set capabilities |b83b0e827b2a48d2d47a80930b7c8f2f#
        self._fetch_capabilities = set(FETCH_CAPABILITIES)
        self._fetch_capabilities.add(capability_agent())
        self._send_capabilities = set(SEND_CAPABILITIES)
        self._send_capabilities.add(capability_agent())
        # nw_e: [[GitClient.__init__()]] set capabilities #
        # nw_s: [[GitClient.__init__()]] adjust capabilities |e2c7b7d3cd6ab2a61e29c2995dc7cec7#
        if quiet:
            self._send_capabilities.add(CAPABILITY_QUIET)
        # nw_e: [[GitClient.__init__()]] adjust capabilities #
        # nw_s: [[GitClient.__init__()]] adjust capabilities |6f1530ffa5cc1351538e1f99de11d060#
        if not thin_packs:
            self._fetch_capabilities.remove(CAPABILITY_THIN_PACK)
        # nw_e: [[GitClient.__init__()]] adjust capabilities #
    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |de46488578ee0cb61755a1a097cd4dea#
    def fetch(self, path, target, determine_wants=None, progress=None):
        """Fetch into a target repository.

        :param path: Path to fetch from (as bytestring)
        :param target: Target repository to fetch into
        :param determine_wants: Optional function to determine what refs
            to fetch. Receives dictionary of name->sha, should return
            list of shas to fetch. Defaults to all shas.
        :param progress: Optional progress function
        :return: Dictionary with all remote refs (not just those fetched)
        """
        if determine_wants is None:
            determine_wants = target.object_store.determine_wants_all

        # nw_s: [[GitClient.fetch()]] if thin pack capability |3f9ada335671da92f802b2312178dce7#
        if CAPABILITY_THIN_PACK in self._fetch_capabilities:
            # TODO(jelmer): Avoid reading entire file into memory and
            # only processing it after the whole file has been fetched.
            f = BytesIO()

            def commit():
                if f.tell():
                    f.seek(0)
                    target.object_store.add_thin_pack(f.read, None)

            def abort():
                pass
        # nw_e: [[GitClient.fetch()]] if thin pack capability #
        else:
            f, commit, abort = target.object_store.add_pack()
        try:
            result = self.fetch_pack(
                path, determine_wants, target.get_graph_walker(), f.write,
                progress)
        except:
            abort()
            raise
        else:
            commit()
        return result

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |991f221c749ff042cea2bf659177e807#
    def get_refs(self, path):
        """Retrieve the current refs from a git smart server.

        :param path: Path to the repo to fetch from. (as bytestring)
        """
        raise NotImplementedError(self.get_refs)

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |0d90479dee89ce53b2b5bfa13aaaa1f4#
    def get_url(self, path):
        """Retrieves full url to given path.

        :param path: Repository path (as string)
        :return: Url to path (as string)
        """
        raise NotImplementedError(self.get_url)

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |48ecd4fbd26c649baeff5aac0892a7c4#
    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        """Create an instance of this client from a urlparse.parsed object.

        :param parsedurl: Result of urlparse.urlparse()
        :return: A `GitClient` object
        """
        raise NotImplementedError(cls.from_parsedurl)
    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |1a6de32dbc19fe6a26a7181dc2bbb9fa#
    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param path: Remote path to fetch from
        :param determine_wants: Function determine what refs
            to fetch. Receives dictionary of name->sha, should return
            list of shas to fetch.
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        raise NotImplementedError(self.fetch_pack)

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |9c80e9f717e4f96fa566f5c5b0886adf#
    def send_pack(self, path, update_refs, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param update_refs: Function to determine changes to remote refs.
            Receive dict with existing remote refs, returns dict with
            changed refs (name -> sha, where sha=ZERO_SHA for deletions)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional progress function
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        raise NotImplementedError(self.send_pack)

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |bfe4f50c37dd7a48de3f583cfbcaffa2#
    def _handle_receive_pack_head(self, proto, capabilities, old_refs,
                                  new_refs):
        """Handle the head of a 'git-receive-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param old_refs: Old refs, as received from the server
        :param new_refs: Refs to change
        :return: (have, want) tuple
        """
        want = []
        have = [x for x in old_refs.values() if not x == ZERO_SHA]
        sent_capabilities = False

        for refname in new_refs:
            if not isinstance(refname, bytes):
                raise TypeError('refname is not a bytestring: %r' % refname)
            old_sha1 = old_refs.get(refname, ZERO_SHA)
            if not isinstance(old_sha1, bytes):
                raise TypeError('old sha1 for %s is not a bytestring: %r' %
                                (refname, old_sha1))
            new_sha1 = new_refs.get(refname, ZERO_SHA)
            if not isinstance(new_sha1, bytes):
                raise TypeError('old sha1 for %s is not a bytestring %r' %
                                (refname, new_sha1))

            if old_sha1 != new_sha1:
                if sent_capabilities:
                    proto.write_pkt_line(old_sha1 + b' ' + new_sha1 + b' ' +
                                         refname)
                else:
                    proto.write_pkt_line(
                        old_sha1 + b' ' + new_sha1 + b' ' + refname + b'\0' +
                        b' '.join(capabilities))
                    sent_capabilities = True
            if new_sha1 not in have and new_sha1 != ZERO_SHA:
                want.append(new_sha1)
        proto.write_pkt_line(None)
        return (have, want)

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |e4f63e268bf5bb9fcc42404e9a3ae5d6#
    def _handle_receive_pack_tail(self, proto, capabilities, progress=None):
        """Handle the tail of a 'git-receive-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param progress: Optional progress reporting function
        """
        if b"side-band-64k" in capabilities:
            if progress is None:
                def progress(x):
                    pass
            channel_callbacks = {2: progress}
            if CAPABILITY_REPORT_STATUS in capabilities:
                channel_callbacks[1] = PktLineParser(
                    self._report_status_parser.handle_packet).parse
            self._read_side_band64k_data(proto, channel_callbacks)
        else:
            if CAPABILITY_REPORT_STATUS in capabilities:
                for pkt in proto.read_pkt_seq():
                    self._report_status_parser.handle_packet(pkt)
        if self._report_status_parser is not None:
            self._report_status_parser.check()

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |92951f8a3eb967fec4352bbe5d3bf8b6#
    def _handle_upload_pack_head(self, proto, capabilities, graph_walker,
                                 wants, can_read):
        """Handle the head of a 'git-upload-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param graph_walker: GraphWalker instance to call .ack() on
        :param wants: List of commits to fetch
        :param can_read: function that returns a boolean that indicates
            whether there is extra graph data to read on proto
        """
        assert isinstance(wants, list) and isinstance(wants[0], bytes)
        proto.write_pkt_line(COMMAND_WANT + b' ' + wants[0] + b' ' +
                             b' '.join(capabilities) + b'\n')
        for want in wants[1:]:
            proto.write_pkt_line(COMMAND_WANT + b' ' + want + b'\n')
        proto.write_pkt_line(None)
        have = next(graph_walker)
        while have:
            proto.write_pkt_line(COMMAND_HAVE + b' ' + have + b'\n')
            if can_read():
                pkt = proto.read_pkt_line()
                parts = pkt.rstrip(b'\n').split(b' ')
                if parts[0] == b'ACK':
                    graph_walker.ack(parts[1])
                    if parts[2] in (b'continue', b'common'):
                        pass
                    elif parts[2] == b'ready':
                        break
                    else:
                        raise AssertionError(
                            "%s not in ('continue', 'ready', 'common)" %
                            parts[2])
            have = next(graph_walker)
        proto.write_pkt_line(COMMAND_DONE + b'\n')

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |a9d8bc00e8fe7bf8ce5938e815fe19f9#
    def _handle_upload_pack_tail(self, proto, capabilities, graph_walker,
                                 pack_data, progress=None, rbufsize=_RBUFSIZE):
        """Handle the tail of a 'git-upload-pack' request.

        :param proto: Protocol object to read from
        :param capabilities: List of negotiated capabilities
        :param graph_walker: GraphWalker instance to call .ack() on
        :param pack_data: Function to call with pack data
        :param progress: Optional progress reporting function
        :param rbufsize: Read buffer size
        """
        pkt = proto.read_pkt_line()
        while pkt:
            parts = pkt.rstrip(b'\n').split(b' ')
            if parts[0] == b'ACK':
                graph_walker.ack(parts[1])
            if len(parts) < 3 or parts[2] not in (
                    b'ready', b'continue', b'common'):
                break
            pkt = proto.read_pkt_line()
        if CAPABILITY_SIDE_BAND_64K in capabilities:
            if progress is None:
                # Just ignore progress data

                def progress(x):
                    pass
            self._read_side_band64k_data(proto, {
                SIDE_BAND_CHANNEL_DATA: pack_data,
                SIDE_BAND_CHANNEL_PROGRESS: progress}
            )
        else:
            while True:
                data = proto.read(rbufsize)
                if data == b"":
                    break
                pack_data(data)
    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |976a70c4ed179f7a94ffc208020d022b#
    def _read_side_band64k_data(self, proto, channel_callbacks):
        """Read per-channel data.

        This requires the side-band-64k capability.

        :param proto: Protocol object to read from
        :param channel_callbacks: Dictionary mapping channels to packet
            handlers to use. None for a callback discards channel data.
        """
        for pkt in proto.read_pkt_seq():
            channel = ord(pkt[:1])
            pkt = pkt[1:]
            try:
                cb = channel_callbacks[channel]
            except KeyError:
                raise AssertionError('Invalid sideband channel %d' % channel)
            else:
                if cb is not None:
                    cb(pkt)

    # nw_e: [[GitClient]] methods #
    # nw_s: [[GitClient]] methods |d8f510f2069e32ccd7cb2c39ff4f7577#
    def _parse_status_report(self, proto):
        unpack = proto.read_pkt_line().strip()
        if unpack != b'unpack ok':
            st = True
            # flush remaining error data
            while st is not None:
                st = proto.read_pkt_line()
            raise SendPackError(unpack)
        statuses = []
        errs = False
        ref_status = proto.read_pkt_line()
        while ref_status:
            ref_status = ref_status.strip()
            statuses.append(ref_status)
            if not ref_status.startswith(b'ok '):
                errs = True
            ref_status = proto.read_pkt_line()

        if errs:
            ref_status = {}
            ok = set()
            for status in statuses:
                if b' ' not in status:
                    # malformed response, move on to the next one
                    continue
                status, ref = status.split(b' ', 1)

                if status == b'ng':
                    if b' ' in ref:
                        ref, status = ref.split(b' ', 1)
                else:
                    ok.add(ref)
                ref_status[ref] = status
            raise UpdateRefsError(', '.join([
                ref for ref in ref_status if ref not in ok]) +
                b' failed to update', ref_status=ref_status)

    # nw_e: [[GitClient]] methods #
# nw_e: class GitClient #

# nw_s: class TraditionalGitClient |e8cd617a405e25b45cc601d5b6b20dda#
class TraditionalGitClient(GitClient):
    """Traditional Git client."""

    DEFAULT_ENCODING = 'utf-8'

    # nw_s: [[TraditionalGitClient]] methods |46535af056e5f04e5c5223b51df5fd5f#
    def __init__(self, path_encoding=DEFAULT_ENCODING, **kwargs):
        self._remote_path_encoding = path_encoding
        super(TraditionalGitClient, self).__init__(**kwargs)

    def _connect(self, cmd, path):
        """Create a connection to the server.

        This method is abstract - concrete implementations should
        implement their own variant which connects to the server and
        returns an initialized Protocol object with the service ready
        for use and a can_read function which may be used to see if
        reads would block.

        :param cmd: The git service name to which we should connect.
        :param path: The path we should pass to the service. (as bytestirng)
        """
        raise NotImplementedError()

    def send_pack(self, path, update_refs, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param update_refs: Function to determine changes to remote refs.
            Receive dict with existing remote refs, returns dict with
            changed refs (name -> sha, where sha=ZERO_SHA for deletions)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional callback called with progress updates
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        proto, unused_can_read = self._connect(b'receive-pack', path)
        with proto:
            old_refs, server_capabilities = read_pkt_refs(proto)
            negotiated_capabilities = (
                self._send_capabilities & server_capabilities)

            if CAPABILITY_REPORT_STATUS in negotiated_capabilities:
                self._report_status_parser = ReportStatusParser()
            report_status_parser = self._report_status_parser

            try:
                new_refs = orig_new_refs = update_refs(dict(old_refs))
            except:
                proto.write_pkt_line(None)
                raise

            if CAPABILITY_DELETE_REFS not in server_capabilities:
                # Server does not support deletions. Fail later.
                new_refs = dict(orig_new_refs)
                for ref, sha in orig_new_refs.items():
                    if sha == ZERO_SHA:
                        if CAPABILITY_REPORT_STATUS in negotiated_capabilities:
                            report_status_parser._ref_statuses.append(
                                b'ng ' + sha +
                                b' remote does not support deleting refs')
                            report_status_parser._ref_status_ok = False
                        del new_refs[ref]

            if new_refs is None:
                proto.write_pkt_line(None)
                return old_refs

            if len(new_refs) == 0 and len(orig_new_refs):
                # NOOP - Original new refs filtered out by policy
                proto.write_pkt_line(None)
                if report_status_parser is not None:
                    report_status_parser.check()
                return old_refs

            (have, want) = self._handle_receive_pack_head(
                proto, negotiated_capabilities, old_refs, new_refs)
            if (not want and
                    set(new_refs.items()).issubset(set(old_refs.items()))):
                return new_refs
            objects = generate_pack_contents(have, want)

            dowrite = len(objects) > 0
            dowrite = dowrite or any(old_refs.get(ref) != sha
                                     for (ref, sha) in new_refs.items()
                                     if sha != ZERO_SHA)
            if dowrite:
                write_pack(proto.write_file(), objects)

            self._handle_receive_pack_tail(
                proto, negotiated_capabilities, progress)
            return new_refs

    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param path: Remote path to fetch from
        :param determine_wants: Function determine what refs
            to fetch. Receives dictionary of name->sha, should return
            list of shas to fetch.
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        proto, can_read = self._connect(b'upload-pack', path)
        with proto:
            refs, server_capabilities = read_pkt_refs(proto)
            negotiated_capabilities = (
                self._fetch_capabilities & server_capabilities)

            if refs is None:
                proto.write_pkt_line(None)
                return refs

            try:
                wants = determine_wants(refs)
            except:
                proto.write_pkt_line(None)
                raise
            if wants is not None:
                wants = [cid for cid in wants if cid != ZERO_SHA]
            if not wants:
                proto.write_pkt_line(None)
                return refs
            self._handle_upload_pack_head(
                proto, negotiated_capabilities, graph_walker, wants, can_read)
            self._handle_upload_pack_tail(
                proto, negotiated_capabilities, graph_walker, pack_data,
                progress)
            return refs

    def get_refs(self, path):
        """Retrieve the current refs from a git smart server."""
        # stock `git ls-remote` uses upload-pack
        proto, _ = self._connect(b'upload-pack', path)
        with proto:
            refs, _ = read_pkt_refs(proto)
            proto.write_pkt_line(None)
            return refs

    def archive(self, path, committish, write_data, progress=None,
                write_error=None):
        proto, can_read = self._connect(b'upload-archive', path)
        with proto:
            proto.write_pkt_line(b"argument " + committish)
            proto.write_pkt_line(None)
            pkt = proto.read_pkt_line()
            if pkt == b"NACK\n":
                return
            elif pkt == b"ACK\n":
                pass
            elif pkt.startswith(b"ERR "):
                raise GitProtocolError(pkt[4:].rstrip(b"\n"))
            else:
                raise AssertionError("invalid response %r" % pkt)
            ret = proto.read_pkt_line()
            if ret is not None:
                raise AssertionError("expected pkt tail")
            self._read_side_band64k_data(proto, {
                SIDE_BAND_CHANNEL_DATA: write_data,
                SIDE_BAND_CHANNEL_PROGRESS: progress,
                SIDE_BAND_CHANNEL_FATAL: write_error})
    # nw_e: [[TraditionalGitClient]] methods #
# nw_e: class TraditionalGitClient #

# nw_s: class TCPGitClient |8e25975b08e8e83aa605eef810972253#
class TCPGitClient(TraditionalGitClient):
    """A Git Client that works over TCP directly (i.e. git://)."""

    # nw_s: [[TCPGitClient]] methods |9ffd7633c7cfc8a47efb9da9b74bd85e#
    def __init__(self, host, port=None, **kwargs):
        if port is None:
            port = TCP_GIT_PORT
        self._host = host
        self._port = port
        super(TCPGitClient, self).__init__(**kwargs)
    # nw_e: [[TCPGitClient]] methods #
    # nw_s: [[TCPGitClient]] methods |54af21be1c78d1e2aac15fd2e0bdcbcf#
    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(parsedurl.hostname, port=parsedurl.port, **kwargs)

    # nw_e: [[TCPGitClient]] methods #
    # nw_s: [[TCPGitClient]] methods |d9580b02ca8b2934d7548da7ba769b80#
    def get_url(self, path):
        netloc = self._host
        if self._port is not None and self._port != TCP_GIT_PORT:
            netloc += ":%d" % self._port
        return urlparse.urlunsplit(("git", netloc, path, '', ''))

    # nw_e: [[TCPGitClient]] methods #
    # nw_s: [[TCPGitClient]] methods |f9ab038e4534c0c632bd2010ce259544#
    def _connect(self, cmd, path):
        if not isinstance(cmd, bytes):
            raise TypeError(cmd)
        if not isinstance(path, bytes):
            path = path.encode(self._remote_path_encoding)
        sockaddrs = socket.getaddrinfo(
            self._host, self._port, socket.AF_UNSPEC, socket.SOCK_STREAM)
        s = None
        err = socket.error("no address found for %s" % self._host)
        for (family, socktype, proto, canonname, sockaddr) in sockaddrs:
            s = socket.socket(family, socktype, proto)
            s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            try:
                s.connect(sockaddr)
                break
            except socket.error as err:
                if s is not None:
                    s.close()
                s = None
        if s is None:
            raise err
        # -1 means system default buffering
        rfile = s.makefile('rb', -1)
        # 0 means unbuffered
        wfile = s.makefile('wb', 0)

        def close():
            rfile.close()
            wfile.close()
            s.close()

        proto = Protocol(rfile.read, wfile.write, close,
                         report_activity=self._report_activity)
        if path.startswith(b"/~"):
            path = path[1:]
        # TODO(jelmer): Alternative to ascii?
        proto.send_cmd(
            b'git-' + cmd, path, b'host=' + self._host.encode('ascii'))
        return proto, lambda: _fileno_can_read(s)
    # nw_e: [[TCPGitClient]] methods #
# nw_e: class TCPGitClient #

# nw_s: class SubprocessWrapper |1ff5dd8f52a8e72e6befd558ff0d5dc2#
class SubprocessWrapper(object):
    """A socket-like object that talks to a subprocess via pipes."""

    def __init__(self, proc):
        self.proc = proc
        if sys.version_info[0] == 2:
            self.read = proc.stdout.read
        else:
            self.read = BufferedReader(proc.stdout).read
        self.write = proc.stdin.write

    def can_read(self):
            return _fileno_can_read(self.proc.stdout.fileno())

    def close(self):
        self.proc.stdin.close()
        self.proc.stdout.close()
        if self.proc.stderr:
            self.proc.stderr.close()
        self.proc.wait()
# nw_e: class SubprocessWrapper #

# nw_s: function find_git_command |be536f02d6524c15b057558058be0ccf#
def find_git_command():
    """Find command to run for system Git (usually C Git).
    """
    if sys.platform == 'win32':  # support .exe, .bat and .cmd
        try:  # to avoid overhead
            import win32api
        except ImportError:  # run through cmd.exe with some overhead
            return ['cmd', '/c', 'git']
        else:
            status, git = win32api.FindExecutable('git')
            return [git]
    else:
        return ['git']
# nw_e: function find_git_command #

# nw_s: class SubprocessGitClient |3c33107e666d0d73e628d43889b04fa3#
class SubprocessGitClient(TraditionalGitClient):
    """Git client that talks to a server using a subprocess."""

    def __init__(self, **kwargs):
        self._connection = None
        self._stderr = None
        self._stderr = kwargs.get('stderr')
        if 'stderr' in kwargs:
            del kwargs['stderr']
        super(SubprocessGitClient, self).__init__(**kwargs)

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(**kwargs)

    git_command = None

    def _connect(self, service, path):
        if not isinstance(service, bytes):
            raise TypeError(service)
        if isinstance(path, bytes):
            path = path.decode(self._remote_path_encoding)
        if self.git_command is None:
            git_command = find_git_command()
        argv = git_command + [service.decode('ascii'), path]
        p = SubprocessWrapper(
            subprocess.Popen(argv, bufsize=0, stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=self._stderr))
        return Protocol(p.read, p.write, p.close,
                        report_activity=self._report_activity), p.can_read
# nw_e: class SubprocessGitClient #

# nw_s: class LocalGitClient |9acde62b6ddf2c29cae02bed5a555470#
class LocalGitClient(GitClient):
    """Git Client that just uses a local Repo."""

    # nw_s: [[LocalGitClient]] methods |d61b1ab64a5ebff1ebd3e7a98981ab34#
    def __init__(self, thin_packs=True, report_activity=None):
        """Create a new LocalGitClient instance.

        :param thin_packs: Whether or not thin packs should be retrieved
        :param report_activity: Optional callback for reporting transport
            activity.
        """
        self._report_activity = report_activity
        # Ignore the thin_packs argument
    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |30980849a554b5f35c50bd51b70e74a0#
    def fetch(self, path, target, determine_wants=None, progress=None):
        """Fetch into a target repository.

        :param path: Path to fetch from (as bytestring)
        :param target: Target repository to fetch into
        :param determine_wants: Optional function determine what refs
            to fetch. Receives dictionary of name->sha, should return
            list of shas to fetch. Defaults to all shas.
        :param progress: Optional progress function
        :return: Dictionary with all remote refs (not just those fetched)
        """
        with self._open_repo(path) as r:
            return r.fetch(target, determine_wants=determine_wants,
                           progress=progress)
    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |e87e5bfa3d5e1c489be546ec5a0e73e3#
    def get_refs(self, path):
        """Retrieve the current refs from a git smart server."""

        with self._open_repo(path) as target:
            return target.get_refs()
    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |ed879714836b214c1e94b9b46ef8ac02#
    @classmethod
    def _open_repo(cls, path):
        from dulwich.repo import Repo
        if not isinstance(path, str):
            path = path.decode(sys.getfilesystemencoding())
        return closing(Repo(path))
    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |611d4ec126b8d4dda09215885b584261#
    def get_url(self, path):
        return urlparse.urlunsplit(('file', '', path, '', ''))

    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |9310e8e85f614d4ed42a49f76b6eda57#
    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(**kwargs)

    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |28d55430ad2d4801fba756b688ea7c43#
    def send_pack(self, path, update_refs, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param update_refs: Function to determine changes to remote refs.
            Receive dict with existing remote refs, returns dict with
            changed refs (name -> sha, where sha=ZERO_SHA for deletions)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional progress function
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        if not progress:
            def progress(x):
                pass

        with self._open_repo(path) as target:
            old_refs = target.get_refs()
            new_refs = update_refs(dict(old_refs))

            have = [sha1 for sha1 in old_refs.values() if sha1 != ZERO_SHA]
            want = []
            for refname, new_sha1 in new_refs.items():
                if (new_sha1 not in have and
                        new_sha1 not in want and
                        new_sha1 != ZERO_SHA):
                    want.append(new_sha1)

            if (not want and
                    set(new_refs.items()).issubset(set(old_refs.items()))):
                return new_refs

            target.object_store.add_objects(generate_pack_contents(have, want))

            for refname, new_sha1 in new_refs.items():
                old_sha1 = old_refs.get(refname, ZERO_SHA)
                if new_sha1 != ZERO_SHA:
                    if not target.refs.set_if_equals(
                            refname, old_sha1, new_sha1):
                        progress('unable to set %s to %s' %
                                 (refname, new_sha1))
                else:
                    if not target.refs.remove_if_equals(refname, old_sha1):
                        progress('unable to remove %s' % refname)

        return new_refs

    # nw_e: [[LocalGitClient]] methods #
    # nw_s: [[LocalGitClient]] methods |8987c807abac0e98e94fa2df097b4fa8#
    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param path: Remote path to fetch from
        :param determine_wants: Function determine what refs
            to fetch. Receives dictionary of name->sha, should return
            list of shas to fetch.
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        with self._open_repo(path) as r:
            objects_iter = r.fetch_objects(
                determine_wants, graph_walker, progress)

            # Did the process short-circuit (e.g. in a stateless RPC call)?
            # Note that the client still expects a 0-object pack in most cases.
            if objects_iter is None:
                return
            write_pack_objects(ProtocolFile(None, pack_data), objects_iter)
            return r.get_refs()

    # nw_e: [[LocalGitClient]] methods #
# nw_e: class LocalGitClient #

# nw_s: class default_local_git_client_cls |3ce93482abb9247b9591e2ac733276b6#
# What Git client to use for local access
default_local_git_client_cls = LocalGitClient
# nw_e: class default_local_git_client_cls #

# nw_s: class SSHVendor |e7489df061f761e95410b20c1de33db9#
class SSHVendor(object):
    """A client side SSH implementation."""

    def connect_ssh(self, host, command, username=None, port=None):
        # This function was deprecated in 0.9.1
        import warnings
        warnings.warn(
            "SSHVendor.connect_ssh has been renamed to SSHVendor.run_command",
            DeprecationWarning)
        return self.run_command(host, command, username=username, port=port)

    def run_command(self, host, command, username=None, port=None):
        """Connect to an SSH server.

        Run a command remotely and return a file-like object for interaction
        with the remote command.

        :param host: Host name
        :param command: Command to run (as argv array)
        :param username: Optional ame of user to log in as
        :param port: Optional SSH port to use
        """
        raise NotImplementedError(self.run_command)
# nw_e: class SSHVendor #

# nw_s: class SubprocessSSHVendor |9f5e078362f8e0d08a44acb1e12455e9#
class SubprocessSSHVendor(SSHVendor):
    """SSH vendor that shells out to the local 'ssh' command."""

    def run_command(self, host, command, username=None, port=None):
        # FIXME: This has no way to deal with passwords..
        args = ['ssh', '-x']
        if port is not None:
            args.extend(['-p', str(port)])
        if username is not None:
            host = '%s@%s' % (username, host)
        args.append(host)
        proc = subprocess.Popen(args + [command], bufsize=0,
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE)
        return SubprocessWrapper(proc)
# nw_e: class SubprocessSSHVendor #

# nw_s: function ParamikoSSHVendor |8e0560fe5cb63bd0ebeb19f5876f6d3a#
def ParamikoSSHVendor(**kwargs):
    import warnings
    warnings.warn(
        "ParamikoSSHVendor has been moved to dulwich.contrib.paramiko_vendor.",
        DeprecationWarning)
    from dulwich.contrib.paramiko_vendor import ParamikoSSHVendor
    return ParamikoSSHVendor(**kwargs)
# nw_e: function ParamikoSSHVendor #

# nw_s: global get_ssh_vendor |ad32ddd06953039df321b0e618265629#
# Can be overridden by users
get_ssh_vendor = SubprocessSSHVendor
# nw_e: global get_ssh_vendor #

# nw_s: class SSHGitClient |c5ee94791557ca8f3e2394d7855c556f#
class SSHGitClient(TraditionalGitClient):

    # nw_s: [[SSHGitClient]] methods |997f598ab492af0caa6b9ee4a9cca98f#
    def __init__(self, host, port=None, username=None, vendor=None, **kwargs):
        self.host = host
        self.port = port
        self.username = username
        super(SSHGitClient, self).__init__(**kwargs)
        self.alternative_paths = {}
        if vendor is not None:
            self.ssh_vendor = vendor
        else:
            self.ssh_vendor = get_ssh_vendor()

    def get_url(self, path):
        netloc = self.host
        if self.port is not None:
            netloc += ":%d" % self.port

        if self.username is not None:
            netloc = urlquote(self.username, '@/:') + "@" + netloc

        return urlparse.urlunsplit(('ssh', netloc, path, '', ''))

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        return cls(host=parsedurl.hostname, port=parsedurl.port,
                   username=parsedurl.username, **kwargs)

    def _get_cmd_path(self, cmd):
        cmd = self.alternative_paths.get(cmd, b'git-' + cmd)
        assert isinstance(cmd, bytes)
        return cmd

    def _connect(self, cmd, path):
        if not isinstance(cmd, bytes):
            raise TypeError(cmd)
        if isinstance(path, bytes):
            path = path.decode(self._remote_path_encoding)
        if path.startswith("/~"):
            path = path[1:]
        argv = (self._get_cmd_path(cmd).decode(self._remote_path_encoding) +
                " '" + path + "'")
        con = self.ssh_vendor.run_command(
            self.host, argv, port=self.port, username=self.username)
        return (Protocol(con.read, con.write, con.close,
                         report_activity=self._report_activity),
                con.can_read)
    # nw_e: [[SSHGitClient]] methods #
# nw_e: class SSHGitClient #


def default_user_agent_string():
    return "dulwich/%s" % ".".join([str(x) for x in dulwich.__version__])


def default_urllib2_opener(config):
    if config is not None:
        proxy_server = config.get("http", "proxy")
    else:
        proxy_server = None
    handlers = []
    if proxy_server is not None:
        handlers.append(urllib2.ProxyHandler({"http": proxy_server}))
    opener = urllib2.build_opener(*handlers)
    if config is not None:
        user_agent = config.get("http", "useragent")
    else:
        user_agent = None
    if user_agent is None:
        user_agent = default_user_agent_string()
    opener.addheaders = [('User-agent', user_agent)]
    return opener

# nw_s: class HttpGitClient |67e01ef0a2bdfac63609726bb116dd4a#
class HttpGitClient(GitClient):

    def __init__(self, base_url, dumb=None, opener=None, config=None,
                 username=None, password=None, **kwargs):
        self._base_url = base_url.rstrip("/") + "/"
        self._username = username
        self._password = password
        self.dumb = dumb
        if opener is None:
            self.opener = default_urllib2_opener(config)
        else:
            self.opener = opener
        if username is not None:
            pass_man = urllib2.HTTPPasswordMgrWithDefaultRealm()
            pass_man.add_password(None, base_url, username, password)
            self.opener.add_handler(urllib2.HTTPBasicAuthHandler(pass_man))
        GitClient.__init__(self, **kwargs)

    def get_url(self, path):
        return self._get_url(path).rstrip("/")

    @classmethod
    def from_parsedurl(cls, parsedurl, **kwargs):
        auth, host = urllib2.splituser(parsedurl.netloc)
        password = parsedurl.password
        if password is not None:
            password = urlunquote(password)
        username = parsedurl.username
        if username is not None:
            username = urlunquote(username)
        # TODO(jelmer): This also strips the username
        parsedurl = parsedurl._replace(netloc=host)
        return cls(urlparse.urlunparse(parsedurl),
                   password=password, username=username, **kwargs)

    def __repr__(self):
        return "%s(%r, dumb=%r)" % (
            type(self).__name__, self._base_url, self.dumb)

    def _get_url(self, path):
        if not isinstance(path, str):
            # TODO(jelmer): this is unrelated to the local filesystem;
            # This is not necessarily the right encoding to decode the path
            # with.
            path = path.decode(sys.getfilesystemencoding())
        return urlparse.urljoin(self._base_url, path).rstrip("/") + "/"

    def _http_request(self, url, headers={}, data=None):
        req = urllib2.Request(url, headers=headers, data=data)
        try:
            resp = self.opener.open(req)
        except urllib2.HTTPError as e:
            if e.code == 404:
                raise NotGitRepository()
            if e.code != 200:
                raise GitProtocolError("unexpected http response %d" % e.code)
        return resp

    def _discover_references(self, service, url):
        assert url[-1] == "/"
        url = urlparse.urljoin(url, "info/refs")
        headers = {}
        if self.dumb is not False:
            url += "?service=%s" % service.decode('ascii')
            headers["Content-Type"] = "application/x-%s-request" % (
                service.decode('ascii'))
        resp = self._http_request(url, headers)
        try:
            content_type = resp.info().gettype()
        except AttributeError:
            content_type = resp.info().get_content_type()
        try:
            self.dumb = (not content_type.startswith("application/x-git-"))
            if not self.dumb:
                proto = Protocol(resp.read, None)
                # The first line should mention the service
                try:
                    [pkt] = list(proto.read_pkt_seq())
                except ValueError:
                    raise GitProtocolError(
                        "unexpected number of packets received")
                if pkt.rstrip(b'\n') != (b'# service=' + service):
                    raise GitProtocolError(
                        "unexpected first line %r from smart server" % pkt)
                return read_pkt_refs(proto)
            else:
                return read_info_refs(resp), set()
        finally:
            resp.close()

    def _smart_request(self, service, url, data):
        assert url[-1] == "/"
        url = urlparse.urljoin(url, service)
        headers = {
            "Content-Type": "application/x-%s-request" % service
        }
        resp = self._http_request(url, headers, data)
        try:
            content_type = resp.info().gettype()
        except AttributeError:
            content_type = resp.info().get_content_type()
        if content_type != (
                "application/x-%s-result" % service):
            raise GitProtocolError("Invalid content-type from server: %s"
                                   % content_type)
        return resp

    def send_pack(self, path, update_refs, generate_pack_contents,
                  progress=None, write_pack=write_pack_objects):
        """Upload a pack to a remote repository.

        :param path: Repository path (as bytestring)
        :param update_refs: Function to determine changes to remote refs.
            Receive dict with existing remote refs, returns dict with
            changed refs (name -> sha, where sha=ZERO_SHA for deletions)
        :param generate_pack_contents: Function that can return a sequence of
            the shas of the objects to upload.
        :param progress: Optional progress function
        :param write_pack: Function called with (file, iterable of objects) to
            write the objects returned by generate_pack_contents to the server.

        :raises SendPackError: if server rejects the pack data
        :raises UpdateRefsError: if the server supports report-status
                                 and rejects ref updates
        :return: new_refs dictionary containing the changes that were made
            {refname: new_ref}, including deleted refs.
        """
        url = self._get_url(path)
        old_refs, server_capabilities = self._discover_references(
            b"git-receive-pack", url)
        negotiated_capabilities = self._send_capabilities & server_capabilities

        if CAPABILITY_REPORT_STATUS in negotiated_capabilities:
            self._report_status_parser = ReportStatusParser()

        new_refs = update_refs(dict(old_refs))
        if new_refs is None:
            # Determine wants function is aborting the push.
            return old_refs
        if self.dumb:
            raise NotImplementedError(self.fetch_pack)
        req_data = BytesIO()
        req_proto = Protocol(None, req_data.write)
        (have, want) = self._handle_receive_pack_head(
            req_proto, negotiated_capabilities, old_refs, new_refs)
        if not want and set(new_refs.items()).issubset(set(old_refs.items())):
            return new_refs
        objects = generate_pack_contents(have, want)
        if len(objects) > 0:
            write_pack(req_proto.write_file(), objects)
        resp = self._smart_request("git-receive-pack", url,
                                   data=req_data.getvalue())
        try:
            resp_proto = Protocol(resp.read, None)
            self._handle_receive_pack_tail(
                resp_proto, negotiated_capabilities, progress)
            return new_refs
        finally:
            resp.close()

    def fetch_pack(self, path, determine_wants, graph_walker, pack_data,
                   progress=None):
        """Retrieve a pack from a git smart server.

        :param determine_wants: Callback that returns list of commits to fetch
        :param graph_walker: Object with next() and ack().
        :param pack_data: Callback called for each bit of data in the pack
        :param progress: Callback for progress reports (strings)
        :return: Dictionary with all remote refs (not just those fetched)
        """
        url = self._get_url(path)
        refs, server_capabilities = self._discover_references(
            b"git-upload-pack", url)
        negotiated_capabilities = (
            self._fetch_capabilities & server_capabilities)
        wants = determine_wants(refs)
        if wants is not None:
            wants = [cid for cid in wants if cid != ZERO_SHA]
        if not wants:
            return refs
        if self.dumb:
            raise NotImplementedError(self.send_pack)
        req_data = BytesIO()
        req_proto = Protocol(None, req_data.write)
        self._handle_upload_pack_head(
                req_proto, negotiated_capabilities, graph_walker, wants,
                lambda: False)
        resp = self._smart_request(
            "git-upload-pack", url, data=req_data.getvalue())
        try:
            resp_proto = Protocol(resp.read, None)
            self._handle_upload_pack_tail(
                resp_proto, negotiated_capabilities, graph_walker, pack_data,
                progress)
            return refs
        finally:
            resp.close()

    def get_refs(self, path):
        """Retrieve the current refs from a git smart server."""
        url = self._get_url(path)
        refs, _ = self._discover_references(
            b"git-upload-pack", url)
        return refs
# nw_e: class HttpGitClient #

# nw_s: function get_transport_and_path_from_url |8152e443d94fe8126e2514a8aae2e30e#
def get_transport_and_path_from_url(url, config=None, **kwargs):
    """Obtain a git client from a URL.

    :param url: URL to open (a unicode string)
    :param config: Optional config object
    :param thin_packs: Whether or not thin packs should be retrieved
    :param report_activity: Optional callback for reporting transport
        activity.
    :return: Tuple with client instance and relative path.
    """
    parsed = urlparse.urlparse(url)
    if parsed.scheme == 'git':
        return (TCPGitClient.from_parsedurl(parsed, **kwargs),
                parsed.path)
    # nw_s: [[get_transport_and_path_from_url()]] elif ssh |ffe62ec6b18d05a195f5ebbb00e8cae9#
    elif parsed.scheme in ('git+ssh', 'ssh'):
        return SSHGitClient.from_parsedurl(parsed, **kwargs), parsed.path
    # nw_e: [[get_transport_and_path_from_url()]] elif ssh #
    # nw_s: [[get_transport_and_path_from_url()]] elif http |47fc587b60abc87c2011943bf2cd787f#
    elif parsed.scheme in ('http', 'https'):
        return HttpGitClient.from_parsedurl(
            parsed, config=config, **kwargs), parsed.path
    # nw_e: [[get_transport_and_path_from_url()]] elif http #
    elif parsed.scheme == 'file':
        return default_local_git_client_cls.from_parsedurl(
            parsed, **kwargs), parsed.path

    raise ValueError("unknown scheme '%s'" % parsed.scheme)
# nw_e: function get_transport_and_path_from_url #

# nw_s: function client.get_transport_and_path |ef13b7d8eb8e5915f67916edd9f55319#
def get_transport_and_path(location, **kwargs):
    """Obtain a git client from a URL.

    :param location: URL or path (a string)
    :param config: Optional config object
    :param thin_packs: Whether or not thin packs should be retrieved
    :param report_activity: Optional callback for reporting transport
        activity.
    :return: Tuple with client instance and relative path.
    """
    # nw_s: [[get_transport_and_path()]] try parse location as a URL |45b3c10ecccf4101c9b049c5f0f22d25#
    # First, try to parse it as a URL
    try:
        return get_transport_and_path_from_url(location, **kwargs)
    except ValueError:
        pass
    # nw_s: [[get_transport_and_path()]] try parse location as a ssh URL |d8a9045239fa79adad0ff43322e9b42f#
    # First, try to parse it as a URL
    try:
        return get_transport_and_path_from_url(location, **kwargs)
    except ValueError:
        pass

    if ':' in location and '@' not in location:
        # SSH with no user@, zero or one leading slash.
        (hostname, path) = location.split(':', 1)
        return SSHGitClient(hostname, **kwargs), path
    elif ':' in location:
        # SSH with user@host:foo.
        user_host, path = location.split(':', 1)
        if '@' in user_host:
            user, host = user_host.rsplit('@', 1)
        else:
            user = None
            host = user_host
        return SSHGitClient(host, username=user, **kwargs), path
    # nw_e: [[get_transport_and_path()]] try parse location as a ssh URL #
    # nw_e: [[get_transport_and_path()]] try parse location as a URL #
    # Otherwise, assume it's a local path.
    return default_local_git_client_cls(**kwargs), location
# nw_e: function client.get_transport_and_path #
# nw_e: dulwich/client.py #
