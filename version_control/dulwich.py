# nw_s: dulwich.py |310c0f0701b39141f2fb8161619b1402#
#!/usr/bin/python -u
#
# dulwich - Simple command-line interface to Dulwich
# Copyright (C) 2008-2011 Jelmer Vernooij <jelmer@samba.org>
# vim: expandtab
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
"""Simple command-line interface to Dulwich>

This is a very simple command-line wrapper for Dulwich. It is by
no means intended to be a full-blown Git command-line interface but just
a way to test Dulwich.
"""

import os
import sys
from getopt import getopt
import optparse
import signal

def signal_int(signal, frame):
    sys.exit(1)

signal.signal(signal.SIGINT, signal_int)

from dulwich import porcelain
from dulwich.client import get_transport_and_path
from dulwich.errors import ApplyDeltaError
from dulwich.index import Index
from dulwich.pack import Pack, sha_to_hex
from dulwich.patch import write_tree_diff
from dulwich.repo import Repo

# nw_s: class Command |b94bcbeadbc289cdd5f04b0dfb0194d3#
class Command(object):
    """A Dulwich subcommand."""

    def run(self, args):
        """Run the command."""
        raise NotImplementedError(self.run)
# nw_e: class Command #


# nw_s: function cmd_archive |201fe0e96862a260617b0301cc6dcc81#
class cmd_archive(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])
        client, path = get_transport_and_path(args.pop(0))
        location = args.pop(0)
        committish = args.pop(0)
        porcelain.archive(location, committish, outstream=sys.stdout,
            errstream=sys.stderr)
# nw_e: function cmd_archive #

# nw_s: function cmd_add |c8a5edeccb6059841ebcbe1392573718#
class cmd_add(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])

        porcelain.add(".", paths=args)
# nw_e: function cmd_add #

# nw_s: function cmd_rm |3d6935947e7bca8c9c2945c64f9aaae4#
class cmd_rm(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])

        porcelain.rm(".", paths=args)
# nw_e: function cmd_rm #

# nw_s: function cmd_fetch_pack |02c3e8ce545036ab4c647f00eb7d29d2#
class cmd_fetch_pack(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["all"])
        opts = dict(opts)
        client, path = get_transport_and_path(args.pop(0))
        r = Repo(".")
        if "--all" in opts:
            determine_wants = r.object_store.determine_wants_all
        else:
            determine_wants = lambda x: [y for y in args if not y in r.object_store]
        client.fetch(path, r, determine_wants)
# nw_e: function cmd_fetch_pack #

# nw_s: function cmd_fetch |9bd9ba15af64c95ba3a293cd94b5aa56#
class cmd_fetch(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])
        opts = dict(opts)
        client, path = get_transport_and_path(args.pop(0))
        r = Repo(".")
        if "--all" in opts:
            determine_wants = r.object_store.determine_wants_all
        refs = client.fetch(path, r, progress=sys.stdout.write)
        print("Remote refs:")
        for item in refs.items():
            print("%s -> %s" % item)
# nw_e: function cmd_fetch #

# nw_s: function cmd_log |718c269ece36fe53e30d4865375c7ec0#
class cmd_log(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        parser.add_option("--reverse", dest="reverse", action="store_true",
                          help="Reverse order in which entries are printed")
        parser.add_option("--name-status", dest="name_status", action="store_true",
                          help="Print name/status for each changed file")
        options, args = parser.parse_args(args)

        porcelain.log(".", paths=args, reverse=options.reverse,
                      name_status=options.name_status,
                      outstream=sys.stdout)
# nw_e: function cmd_log #

# nw_s: function cmd_diff |4d4394342e988944a5d22ecd58018e1f#
class cmd_diff(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])

        if args == []:
            print("Usage: dulwich diff COMMITID")
            sys.exit(1)

        r = Repo(".")
        commit_id = args[0]
        commit = r[commit_id]
        parent_commit = r[commit.parents[0]]

        write_tree_diff(sys.stdout, r.object_store, parent_commit.tree, commit.tree)

# nw_e: function cmd_diff #

# nw_s: function cmd_dump_pack |c293e284be60eeb94beadbba171aae8d#
class cmd_dump_pack(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])

        if args == []:
            print("Usage: dulwich dump-pack FILENAME")
            sys.exit(1)

        basename, _ = os.path.splitext(args[0])
        x = Pack(basename)
        print("Object names checksum: %s" % x.name())
        print("Checksum: %s" % sha_to_hex(x.get_stored_checksum()))
        if not x.check():
            print("CHECKSUM DOES NOT MATCH")
        print("Length: %d" % len(x))
        for name in x:
            try:
                print("\t%s" % x[name])
            except KeyError as k:
                print("\t%s: Unable to resolve base %s" % (name, k))
            except ApplyDeltaError as e:
                print("\t%s: Unable to apply delta: %r" % (name, e))
# nw_e: function cmd_dump_pack #

# nw_s: function cmd_dump_index |1ae1d2569bc7b650c2344d3191af7b01#
class cmd_dump_index(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])

        if args == []:
            print("Usage: dulwich dump-index FILENAME")
            sys.exit(1)

        filename = args[0]
        idx = Index(filename)

        for o in idx:
            print(o, idx[o])
# nw_e: function cmd_dump_index #

# nw_s: function cmd_init |1e949ede587b8b6c7cd1787b61fe9927#
class cmd_init(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["bare"])
        opts = dict(opts)

        if args == []:
            path = os.getcwd()
        else:
            path = args[0]

        porcelain.init(path, bare=("--bare" in opts))
# nw_e: function cmd_init #

# nw_s: function cmd_clone |6b003c5b86460736c0bee4ab8c37a956#
class cmd_clone(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["bare"])
        opts = dict(opts)

        if args == []:
            print("usage: dulwich clone host:path [PATH]")
            sys.exit(1)

        source = args.pop(0)
        if len(args) > 0:
            target = args.pop(0)
        else:
            target = None

        porcelain.clone(source, target, bare=("--bare" in opts))
# nw_e: function cmd_clone #

# nw_s: function cmd_commit |0876628047a7e82f46528687420f7247#
class cmd_commit(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["message"])
        opts = dict(opts)

        porcelain.commit(".", message=opts["--message"])
# nw_e: function cmd_commit #

# nw_s: function cmd_commit_tree |7dfb86da3a1e48eb2368301a8a30030b#
class cmd_commit_tree(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["message"])
        if args == []:
            print("usage: dulwich commit-tree tree")
            sys.exit(1)
        opts = dict(opts)
        porcelain.commit_tree(".", tree=args[0], message=opts["--message"])
# nw_e: function cmd_commit_tree #

# nw_s: function cmd_update_server_info |5b00750056eec59556010cfe152140e7#
class cmd_update_server_info(Command):

    def run(self, args):
        porcelain.update_server_info(".")

# nw_e: function cmd_update_server_info #

# nw_s: function cmd_symbolic_ref |91cdda38e5e7ab832cd882e4c38d847a#
class cmd_symbolic_ref(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["ref-name", "force"])
        if not args:
            print("Usage: dulwich symbolic-ref REF_NAME [--force]")
            sys.exit(1)

        ref_name = args.pop(0)
        porcelain.symbolic_ref(".", ref_name=ref_name, force='--force' in args)

# nw_e: function cmd_symbolic_ref #

# nw_s: function cmd_show |28404a4f8bca2f13fbdd34d88bae4883#
class cmd_show(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])

        porcelain.show(".", args)
# nw_e: function cmd_show #

# nw_s: function cmd_diff_tree |cbf7e50cce902b050c309f2140e14334#
class cmd_diff_tree(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])
        if len(args) < 2:
            print("Usage: dulwich diff-tree OLD-TREE NEW-TREE")
            sys.exit(1)
        porcelain.diff_tree(".", args[0], args[1])

# nw_e: function cmd_diff_tree #

# nw_s: function cmd_rev_list |74859b312f13b929ede40ffb333b3211#
class cmd_rev_list(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])
        if len(args) < 1:
            print('Usage: dulwich rev-list COMMITID...')
            sys.exit(1)

        porcelain.rev_list('.', args)

# nw_e: function cmd_rev_list #

# nw_s: function cmd_tag |14c10aae907cd4c72f015eba0c190d32#
class cmd_tag(Command):

    def run(self, args):
        opts, args = getopt(args, '', [])
        if len(args) < 2:
            print('Usage: dulwich tag NAME')
            sys.exit(1)

        porcelain.tag('.', args[0])
# nw_e: function cmd_tag #

# nw_s: function cmd_repack |5a0d7d747a328c655e0e7b06fa73d290#
class cmd_repack(Command):

    def run(self, args):
        opts, args = getopt(args, "", [])
        opts = dict(opts)

        porcelain.repack('.')

# nw_e: function cmd_repack #

# nw_s: function cmd_reset |7706c80d4e14e0e7aa0c7ae1d3084073#
class cmd_reset(Command):

    def run(self, args):
        opts, args = getopt(args, "", ["hard", "soft", "mixed"])
        opts = dict(opts)

        mode = ""
        if "--hard" in opts:
            mode = "hard"
        elif "--soft" in opts:
            mode = "soft"
        elif "--mixed" in opts:
            mode = "mixed"

        porcelain.reset('.', mode=mode, *args)

# nw_e: function cmd_reset #

# nw_s: function cmd_daemon |99370780a4c0041b8dc83abaf04b09d2#
class cmd_daemon(Command):

    def run(self, args):
        from dulwich import log_utils
        from dulwich.protocol import TCP_GIT_PORT
        parser = optparse.OptionParser()
        parser.add_option("-l", "--listen_address", dest="listen_address",
                          default="localhost",
                          help="Binding IP address.")
        parser.add_option("-p", "--port", dest="port", type=int,
                          default=TCP_GIT_PORT,
                          help="Binding TCP port.")
        options, args = parser.parse_args(args)

        log_utils.default_logging_config()
        if len(args) >= 1:
            gitdir = args[0]
        else:
            gitdir = '.'
        from dulwich import porcelain
        porcelain.daemon(gitdir, address=options.listen_address,
                         port=options.port)

# nw_e: function cmd_daemon #

# nw_s: function cmd_web_daemon |51e25ed3339d0fe8df297344c6de0039#
class cmd_web_daemon(Command):

    def run(self, args):
        from dulwich import log_utils
        parser = optparse.OptionParser()
        parser.add_option("-l", "--listen_address", dest="listen_address",
                          default="",
                          help="Binding IP address.")
        parser.add_option("-p", "--port", dest="port", type=int,
                          default=8000,
                          help="Binding TCP port.")
        options, args = parser.parse_args(args)

        log_utils.default_logging_config()
        if len(args) >= 1:
            gitdir = args[0]
        else:
            gitdir = '.'
        from dulwich import porcelain
        porcelain.web_daemon(gitdir, address=options.listen_address,
                             port=options.port)

# nw_e: function cmd_web_daemon #

# nw_s: function cmd_receive_pack |971c83c50e29bd08815f67874c632a94#
class cmd_receive_pack(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        options, args = parser.parse_args(args)
        if len(args) >= 1:
            gitdir = args[0]
        else:
            gitdir = '.'
        porcelain.receive_pack(gitdir)

# nw_e: function cmd_receive_pack #

# nw_s: function cmd_upload_pack |65c778e3ffa1d8c31e791aca9e9ff0b0#
class cmd_upload_pack(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        options, args = parser.parse_args(args)
        if len(args) >= 1:
            gitdir = args[0]
        else:
            gitdir = '.'
        porcelain.upload_pack(gitdir)
# nw_e: function cmd_upload_pack #

# nw_s: function cmd_status |e60c427199c7a056c18f39f6af774b97#
class cmd_status(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        options, args = parser.parse_args(args)
        if len(args) >= 1:
            gitdir = args[0]
        else:
            gitdir = '.'

        status = porcelain.status(gitdir)
        if any(names for (kind, names) in status.staged.items()):
            sys.stdout.write("Changes to be committed:\n\n")
            for kind, names in status.staged.items():
                for name in names:
                    sys.stdout.write("\t%s: %s\n" % (
                        kind, name.decode(sys.getfilesystemencoding())))
            sys.stdout.write("\n")
        if status.unstaged:
            sys.stdout.write("Changes not staged for commit:\n\n")
            for name in status.unstaged:
                sys.stdout.write("\t%s\n" %
                        name.decode(sys.getfilesystemencoding()))
            sys.stdout.write("\n")
        if status.untracked:
            sys.stdout.write("Untracked files:\n\n")
            for name in status.untracked:
                sys.stdout.write("\t%s\n" % name)
            sys.stdout.write("\n")
# nw_e: function cmd_status #

# nw_s: function cmd_ls_remote |bc5fc05f9f7533b37f784caa754ccf94#
class cmd_ls_remote(Command):

    def run(self, args):
        opts, args = getopt(args, '', [])
        if len(args) < 1:
            print('Usage: dulwich ls-remote URL')
            sys.exit(1)
        refs = porcelain.ls_remote(args[0])
        for ref in sorted(refs):
            sys.stdout.write("%s\t%s\n" % (ref, refs[ref]))
# nw_e: function cmd_ls_remote #

# nw_s: function cmd_ls_tree |83a89af0fd0149b6bbfa9c6c2ac275f0#
class cmd_ls_tree(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        parser.add_option("-r", "--recursive", action="store_true",
                          help="Recusively list tree contents.")
        parser.add_option("--name-only", action="store_true",
                          help="Only display name.")
        options, args = parser.parse_args(args)
        try:
            treeish = args.pop(0)
        except IndexError:
            treeish = None
        porcelain.ls_tree(
            '.', treeish, outstream=sys.stdout, recursive=options.recursive,
            name_only=options.name_only)
# nw_e: function cmd_ls_tree #

# nw_s: function cmd_pack_objects |7a82c663d6ae1fb2f9712800b1bb971d#
class cmd_pack_objects(Command):

    def run(self, args):
        opts, args = getopt(args, '', ['stdout'])
        opts = dict(opts)
        if len(args) < 1 and not '--stdout' in args:
            print('Usage: dulwich pack-objects basename')
            sys.exit(1)
        object_ids = [l.strip() for l in sys.stdin.readlines()]
        basename = args[0]
        if '--stdout' in opts:
            packf = getattr(sys.stdout, 'buffer', sys.stdout)
            idxf = None
            close = []
        else:
            packf = open(basename + '.pack', 'w')
            idxf = open(basename + '.idx', 'w')
            close = [packf, idxf]

        porcelain.pack_objects('.', object_ids, packf, idxf)
        for f in close:
            f.close()
# nw_e: function cmd_pack_objects #

# nw_s: function cmd_pull |ded42762fd9b5a930ea51ae2224adbe6#
class cmd_pull(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        options, args = parser.parse_args(args)
        try:
            from_location = args[0]
        except IndexError:
            from_location = None

        porcelain.pull('.', from_location)
# nw_e: function cmd_pull #

# nw_s: function cmd_remote_add |cbbddaae5b0138e42a2adcce19128019#
class cmd_remote_add(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        options, args = parser.parse_args(args)
        porcelain.remote_add('.', args[0], args[1])

# nw_e: function cmd_remote_add #

# nw_s: function cmd_remote |1a7180840dbc699bbb429ae042ffcb7e#
class cmd_remote(Command):

    subcommands = {
        "add": cmd_remote_add,
    }

    def run(self, args):
        if not args:
            print("Supported subcommands: %s" % ', '.join(self.subcommands.keys()))
            return False
        cmd = args[0]
        try:
            cmd_kls = self.subcommands[cmd]
        except KeyError:
            print('No such subcommand: %s' % args[0])
            return False
        return cmd_kls(args[1:])

# nw_e: function cmd_remote #

# nw_s: function cmd_help |77be736aa9701b222b6c7cb4da068a77#
class cmd_help(Command):

    def run(self, args):
        parser = optparse.OptionParser()
        parser.add_option("-a", "--all", dest="all",
                          action="store_true",
                          help="List all commands.")
        options, args = parser.parse_args(args)

        if options.all:
            print('Available commands:')
            for cmd in sorted(commands):
                print('  %s' % cmd)
        else:
            print("""\
The dulwich command line tool is currently a very basic frontend for the
Dulwich python module. For full functionality, please see the API reference.

For a list of supported commands, see 'dulwich help -a'.
""")
# nw_e: function cmd_help #

# nw_s: constant commands |e7759fa5720526124be1c104038a9be2#
commands = {
    "add": cmd_add,
    "archive": cmd_archive,
    "clone": cmd_clone,
    "commit": cmd_commit,
    "commit-tree": cmd_commit_tree,
    "daemon": cmd_daemon,
    "diff": cmd_diff,
    "diff-tree": cmd_diff_tree,
    "dump-pack": cmd_dump_pack,
    "dump-index": cmd_dump_index,
    "fetch-pack": cmd_fetch_pack,
    "fetch": cmd_fetch,
    "help": cmd_help,
    "init": cmd_init,
    "log": cmd_log,
    "ls-remote": cmd_ls_remote,
    "ls-tree": cmd_ls_tree,
    "pack-objects": cmd_pack_objects,
    "pull": cmd_pull,
    "receive-pack": cmd_receive_pack,
    "remote": cmd_remote,
    "repack": cmd_repack,
    "reset": cmd_reset,
    "rev-list": cmd_rev_list,
    "rm": cmd_rm,
    "show": cmd_show,
    "status": cmd_status,
    "symbolic-ref": cmd_symbolic_ref,
    "tag": cmd_tag,
    "update-server-info": cmd_update_server_info,
    "upload-pack": cmd_upload_pack,
    "web-daemon": cmd_web_daemon,
    }
# nw_e: constant commands #

# nw_s: toplevel main |9d13abaeb7044bb67bac94c13068bac5#
# nw_s: sanity check [[sys.argv]] |f0fb7e8aa2de6f04f68a98f088db8910#
if len(sys.argv) < 2:
    print("Usage: %s <%s> [OPTIONS...]" % (sys.argv[0], "|".join(commands.keys())))
    sys.exit(1)
# nw_e: sanity check [[sys.argv]] #
cmd = sys.argv[1]
try:
    cmd_kls = commands[cmd]
except KeyError:
    print("No such subcommand: %s" % cmd)
    sys.exit(1)

# TODO(jelmer): Return non-0 on errors
cmd_kls().run(sys.argv[2:])
# nw_e: toplevel main #

# nw_e: dulwich.py #
