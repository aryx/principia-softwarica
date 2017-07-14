# nw_s: dulwich/objects.py |6e9412f667b384d8fd6d636270beff1d#
# objects.py -- Access to base git objects
# Copyright (C) 2007 James Westby <jw+debian@jameswestby.net>
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
"""Access to base git objects."""

import binascii
from io import BytesIO
from collections import namedtuple
import os
import posixpath
import stat
import warnings
import zlib
from hashlib import sha1

from dulwich.errors import (
    ChecksumMismatch,
    NotBlobError,
    NotCommitError,
    NotTagError,
    NotTreeError,
    ObjectFormatException,
    )
from dulwich.file import GitFile


ZERO_SHA = b'0' * 40

# Header fields for commits
_TREE_HEADER = b'tree'
_PARENT_HEADER = b'parent'
_AUTHOR_HEADER = b'author'
_COMMITTER_HEADER = b'committer'
_ENCODING_HEADER = b'encoding'
_MERGETAG_HEADER = b'mergetag'
_GPGSIG_HEADER = b'gpgsig'

# Header fields for objects
_OBJECT_HEADER = b'object'
_TYPE_HEADER = b'type'
_TAG_HEADER = b'tag'
_TAGGER_HEADER = b'tagger'


# nw_s: constant S_IFGITLINK |cd08f9e822e536fb5480045cb9b32e8f#
S_IFGITLINK = 0o160000
# nw_e: constant S_IFGITLINK #

# nw_s: function objects.S_ISGITLINK |b38f91a099a18d59ea19764cfac3ffe9#
def S_ISGITLINK(m):
    """Check if a mode indicates a submodule.

    :param m: Mode to check
    :return: a ``boolean``
    """
    return (stat.S_IFMT(m) == S_IFGITLINK)
# nw_e: function objects.S_ISGITLINK #

# nw_s: function objects._decompress |08b89e6c8a0e5df1d590db189506412e#
def _decompress(string):
    dcomp = zlib.decompressobj()
    dcomped = dcomp.decompress(string)
    dcomped += dcomp.flush()
    return dcomped

# nw_e: function objects._decompress #

# nw_s: function sha_to_hex |37a337a5f01f96e35311cbc23857a54d#
def sha_to_hex(sha):
    """Takes a string and returns the hex of the sha within"""
    hexsha = binascii.hexlify(sha)
    assert len(hexsha) == 40, "Incorrect length of sha1 string: %d" % hexsha
    return hexsha
# nw_e: function sha_to_hex #

# nw_s: function hex_to_sha |378d890a42bd735a00a21f14f342ab8b#
def hex_to_sha(hex):
    """Takes a hex sha and returns a binary sha"""
    assert len(hex) == 40, "Incorrect length of hexsha: %s" % hex
    try:
        return binascii.unhexlify(hex)
    except TypeError as exc:
        if not isinstance(hex, bytes):
            raise
        raise ValueError(exc.args[0])
# nw_e: function hex_to_sha #

# nw_s: function valid_hexsha |1731a9f889f1c6c45ea664f39a55382c#
def valid_hexsha(hex):
    if len(hex) != 40:
        return False
    try:
        binascii.unhexlify(hex)
    except (TypeError, binascii.Error):
        return False
    else:
        return True
# nw_e: function valid_hexsha #

# nw_s: function hex_to_filename |e8e629bf7c6396c3cab266a1c13b6eed#
def hex_to_filename(path, hex):
    """Takes a hex sha and returns its filename relative to the given path."""
    # os.path.join accepts bytes or unicode, but all args must be of the same
    # type. Make sure that hex which is expected to be bytes, is the same type
    # as path.
    if getattr(path, 'encode', None) is not None:
        hex = hex.decode('ascii')
    dir = hex[:2]
    file = hex[2:]
    # Check from object dir
    return os.path.join(path, dir, file)

# nw_e: function hex_to_filename #

# nw_s: function filename_to_hex |370158f18cf210e0fe8ed77f3ecc0261#
def filename_to_hex(filename):
    """Takes an object filename and returns its corresponding hex sha."""
    # grab the last (up to) two path components
    names = filename.rsplit(os.path.sep, 2)[-2:]
    errmsg = "Invalid object filename: %s" % filename
    assert len(names) == 2, errmsg
    base, rest = names
    assert len(base) == 2 and len(rest) == 38, errmsg
    hex = (base + rest).encode('ascii')
    hex_to_sha(hex)
    return hex
# nw_e: function filename_to_hex #




def object_header(num_type, length):
    """Return an object header for the given numeric type and text length."""
    return (object_class(num_type).type_name +
            b' ' + str(length).encode('ascii') + b'\0')

# nw_s: function serializable_property |fe7d281f7ae936f697d0d4a8cb88d8fe#
def serializable_property(name, docstring=None):
    """A property that helps tracking whether serialization is necessary.
    """
    def set(obj, value):
        setattr(obj, "_"+name, value)
        obj._needs_serialization = True

    def get(obj):
        return getattr(obj, "_"+name)
    return property(get, set, doc=docstring)
# nw_e: function serializable_property #

# nw_s: function objects.object_class |bbe7c931243702cd2515cc4bb2c214fc#
def object_class(type):
    """Get the object class corresponding to the given type.

    :param type: Either a type name string or a numeric type.
    :return: The ShaFile subclass corresponding to the given type, or None if
        type is not a valid type name/number.
    """
    return _TYPE_MAP.get(type, None)
# nw_e: function objects.object_class #

# nw_s: function objects.check_hexsha |a5b6cb65a2202d28f481c9f4aa74ca2b#
def check_hexsha(hex, error_msg):
    """Check if a string is a valid hex sha string.

    :param hex: Hex string to check
    :param error_msg: Error message to use in exception
    :raise ObjectFormatException: Raised when the string is not valid
    """
    if not valid_hexsha(hex):
        raise ObjectFormatException("%s %s" % (error_msg, hex))
# nw_e: function objects.check_hexsha #

# nw_s: function objects.check_identity |29274262f673b4e9644cd8891955db97#
def check_identity(identity, error_msg):
    """Check if the specified identity is valid.

    This will raise an exception if the identity is not valid.

    :param identity: Identity string
    :param error_msg: Error message to use in exception
    """
    email_start = identity.find(b'<')
    email_end = identity.find(b'>')
    if (email_start < 0 or email_end < 0 or email_end <= email_start
            or identity.find(b'<', email_start + 1) >= 0
            or identity.find(b'>', email_end + 1) >= 0
            or not identity.endswith(b'>')):
        raise ObjectFormatException(error_msg)
# nw_e: function objects.check_identity #


def git_line(*items):
    """Formats items into a space sepreated line."""
    return b' '.join(items) + b'\n'


class FixedSha(object):
    """SHA object that behaves like hashlib's but is given a fixed value."""

    __slots__ = ('_hexsha', '_sha')

    def __init__(self, hexsha):
        if getattr(hexsha, 'encode', None) is not None:
            hexsha = hexsha.encode('ascii')
        if not isinstance(hexsha, bytes):
            raise TypeError('Expected bytes for hexsha, got %r' % hexsha)
        self._hexsha = hexsha
        self._sha = hex_to_sha(hexsha)

    def digest(self):
        """Return the raw SHA digest."""
        return self._sha

    def hexdigest(self):
        """Return the hex SHA digest."""
        return self._hexsha.decode('ascii')

# nw_s: class ShaFile |0fbe504d96a57540b87cd61ed5c37772#
class ShaFile(object):
    """A git SHA file."""

    __slots__ = ('_chunked_text', '_sha', '_needs_serialization')

    # nw_s: [[ShaFile]] methods |7da401958da2bc53dc6c80ec1752b004#
    def as_raw_string(self):
        """Return raw string with serialization of the object.

        :return: String object
        """
        return b''.join(self.as_raw_chunks())

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |2f44295f73cda48b0525b7e3639a354d#
    def __init__(self):
        """Don't call this directly"""
        self._sha = None
        self._chunked_text = []
        self._needs_serialization = True

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |fa91d6c234cf31873ad9d21d9a3c3cd3#
    def sha(self):
        """The SHA1 object that is the name of this object."""
        if self._sha is None or self._needs_serialization:
            # this is a local because as_raw_chunks() overwrites self._sha
            new_sha = sha1()
            new_sha.update(self._header())
            for chunk in self.as_raw_chunks():
                new_sha.update(chunk)
            self._sha = new_sha
        return self._sha

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |bc04d579d28b964932ad3150708a2334#
    def _header(self):
        return object_header(self.type_num, self.raw_length())
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |3942ad35f4eebc4b5b543760bef8d6c3#
    def check(self):
        """Check this object for internal consistency.

        :raise ObjectFormatException: if the object is malformed in some way
        :raise ChecksumMismatch: if the object was created with a SHA that does
            not match its contents
        """
        # TODO: if we find that error-checking during object parsing is a
        # performance bottleneck, those checks should be moved to the class's
        # check() method during optimization so we can still check the object
        # when necessary.
        old_sha = self.id
        try:
            self._deserialize(self.as_raw_chunks())
            self._sha = None
            new_sha = self.id
        except Exception as e:
            raise ObjectFormatException(e)
        if old_sha != new_sha:
            raise ChecksumMismatch(new_sha, old_sha)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |bed2a54445a9cbca3f160a104fa96123#
    @property
    def id(self):
        """The hex SHA of this object."""
        return self.sha().hexdigest().encode('ascii')

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |ff75f4e11a588860981bf6a0761aa1fe#
    def as_raw_chunks(self):
        """Return chunks with serialization of the object.

        :return: List of strings, not necessarily one per line
        """
        if self._needs_serialization:
            self._sha = None
            self._chunked_text = self._serialize()
            self._needs_serialization = False
        return self._chunked_text

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |abf78057465ac4570ac21f1ae892a58c#
    def _check_has_member(self, member, error_msg):
        """Check that the object has a given member variable.

        :param member: the member variable to check for
        :param error_msg: the message for an error if the member is missing
        :raise ObjectFormatException: with the given error_msg if member is
            missing or is None
        """
        if getattr(self, member, None) is None:
            raise ObjectFormatException(error_msg)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |a5d20a836c6a647e48948e3d1a483c74#
    @classmethod
    def from_path(cls, path):
        """Open a SHA file from disk."""
        with GitFile(path, 'rb') as f:
            return cls.from_file(f)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |33a2ee4c7565b88b1aaf2d24a72abaa4#
    @classmethod
    def from_file(cls, f):
        """Get the contents of a SHA file on disk."""
        try:
            obj = cls._parse_file(f)
            obj._sha = None
            return obj
        except (IndexError, ValueError):
            raise ObjectFormatException("invalid object header")

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |52ebd1ab36f6cc2c7096faa35bfc72c2#
    @classmethod
    def _parse_file(cls, f):
        map = f.read()
        # nw_s: [[ShaFile._parse_file()]] if legacy object |e65697fd9ca23c3255e2970ae31d9bdc#
        if cls._is_legacy_object(map):
            #pad: raise AssertionError('use legacy format')
            obj = cls._parse_legacy_object_header(map, f)
            obj._parse_legacy_object(map)
        # nw_e: [[ShaFile._parse_file()]] if legacy object #
        else:
            obj = cls._parse_object_header(map, f)
            obj._parse_object(map)
        return obj
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |9ead8628b09b24e6d8437e64be7fac91#
    @staticmethod
    def _parse_object_header(magic, f):
        """Parse a new style object, creating it but not reading the file."""
        num_type = (ord(magic[0:1]) >> 4) & 7
        obj_class = object_class(num_type)
        if not obj_class:
            raise ObjectFormatException("Not a known type %d" % num_type)
        return obj_class()

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |490f306f315aad867a90ac0a9acacba7#
    def _parse_object(self, map):
        """Parse a new style object, setting self._text."""
        # skip type and size; type must have already been determined, and
        # we trust zlib to fail if it's otherwise corrupted
        byte = ord(map[0:1])
        used = 1
        while (byte & 0x80) != 0:
            byte = ord(map[used:used+1])
            used += 1
        raw = map[used:]
        self.set_raw_string(_decompress(raw))

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |277886d001337c3207ec2ef4f9f89449#
    def set_raw_string(self, text, sha=None):
        """Set the contents of this object from a serialized string."""
        if not isinstance(text, bytes):
            raise TypeError('Expected bytes for text, got %r' % text)
        self.set_raw_chunks([text], sha)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |d0ab29eb0ab75c470eb7dbb0eb752bd6#
    def set_raw_chunks(self, chunks, sha=None):
        """Set the contents of this object from a list of chunks."""
        self._chunked_text = chunks
        self._deserialize(chunks)
        if sha is None:
            self._sha = None
        else:
            self._sha = FixedSha(sha)
        self._needs_serialization = False

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |c029222d38ccf9bbdd7fcf6eea4ba1e5#
    def _deserialize(self, chunks):
        raise NotImplementedError(self._deserialize)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |08750a5cfd4229ee9210360c5397c559#
    @staticmethod
    def from_raw_string(type_num, string, sha=None):
        """Creates an object of the indicated type from the raw string given.

        :param type_num: The numeric type of the object.
        :param string: The raw uncompressed contents.
        :param sha: Optional known sha for the object
        """
        obj = object_class(type_num)()
        obj.set_raw_string(string, sha)
        return obj

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |5e2f08e0e6a9358233cca0163a1d6759#
    def _serialize(self):
        raise NotImplementedError(self._serialize)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |1d171b56fc6a79d0b1b6a6bef8faaa78#
    @classmethod
    def _is_legacy_object(cls, magic):
        b0 = ord(magic[0:1])
        b1 = ord(magic[1:2])
        word = (b0 << 8) + b1
        return (b0 & 0x8F) == 0x08 and (word % 31) == 0
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |a157c5f331b308c36d585dfeb0e2a5d4#
    @staticmethod
    def _parse_legacy_object_header(magic, f):
        """Parse a legacy object, creating it but not reading the file."""
        bufsize = 1024
        decomp = zlib.decompressobj()
        header = decomp.decompress(magic)
        start = 0
        end = -1
        while end < 0:
            extra = f.read(bufsize)
            header += decomp.decompress(extra)
            magic += extra
            end = header.find(b'\0', start)
            start = len(header)
        header = header[:end]
        type_name, size = header.split(b' ', 1)
        size = int(size)  # sanity check
        obj_class = object_class(type_name)
        if not obj_class:
            raise ObjectFormatException("Not a known type: %s" % type_name)
        return obj_class()
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |17cd35c2a6979d10a76c105059649144#
    def _parse_legacy_object(self, map):
        """Parse a legacy object, setting the raw string."""
        text = _decompress(map)
        header_end = text.find(b'\0')
        if header_end < 0:
            raise ObjectFormatException("Invalid object header, no \\0")
        self.set_raw_string(text[header_end+1:])

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |fb14455012608b9c47c36f051ed6a5c5#
    def as_legacy_object(self):
        """Return string representing the object in the experimental format.
        """
        return b''.join(self.as_legacy_object_chunks())
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |58cf706a3a4e6f74d5a64f0cd791d178#
    def as_legacy_object_chunks(self):
        """Return chunks representing the object in the experimental format.

        :return: List of strings
        """
        compobj = zlib.compressobj()
        yield compobj.compress(self._header())
        for chunk in self.as_raw_chunks():
            yield compobj.compress(chunk)
        yield compobj.flush()

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |092abb4211df009dca2da684a5e195cf#
    def __repr__(self):
        return "<%s %s>" % (self.__class__.__name__, self.id)
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |8b8ade229d1c62f5e1f6e7682efef28b#
    def __str__(self):
        """Return raw string serialization of this object."""
        return self.as_raw_string()

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |0bffa7cc56b3432a234732e81f20b0d4#
    def __hash__(self):
        """Return unique hash for this object."""
        return hash(self.id)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |593aeccb068cc1ce523f66a51f733d77#
    def as_pretty_string(self):
        """Return a string representing this object, fit for display."""
        return self.as_raw_string()

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |dea703bb68701f6fadd7b9b7578a91bb#
    @staticmethod
    def from_raw_chunks(type_num, chunks, sha=None):
        """Creates an object of the indicated type from the raw chunks given.

        :param type_num: The numeric type of the object.
        :param chunks: An iterable of the raw uncompressed contents.
        :param sha: Optional known sha for the object
        """
        obj = object_class(type_num)()
        obj.set_raw_chunks(chunks, sha)
        return obj

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |4e39a2996ca82b8da27c655fb1fb4e91#
    @classmethod
    def from_string(cls, string):
        """Create a ShaFile from a string."""
        obj = cls()
        obj.set_raw_string(string)
        return obj

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |cf9d6c3f0d741a4b83817b3f15eaed4c#
    def raw_length(self):
        """Returns the length of the raw string of this object."""
        ret = 0
        for chunk in self.as_raw_chunks():
            ret += len(chunk)
        return ret

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |8c4e48d26173aee3c007f15cd474baad#
    def copy(self):
        """Create a new copy of this SHA1 object from its raw string"""
        obj_class = object_class(self.get_type())
        return obj_class.from_raw_string(
            self.get_type(),
            self.as_raw_string(),
            self.id)

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |639e961afb8c2c8778cc97cfa742b19e#
    def __ne__(self, other):
        return not isinstance(other, ShaFile) or self.id != other.id

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |a389162c8cdd4ced9063dc7c6ea74207#
    def __eq__(self, other):
        """Return True if the SHAs of the two objects match.

        It doesn't make sense to talk about an order on ShaFiles, so we don't
        override the rich comparison methods (__le__, etc.).
        """
        return isinstance(other, ShaFile) and self.id == other.id

    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |5dfef3f1454bb8e79a42e9803793475f#
    def __lt__(self, other):
        if not isinstance(other, ShaFile):
            raise TypeError
        return self.id < other.id
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |d97dc5e877f0f1e89b38292255583946#
    def __le__(self, other):
        if not isinstance(other, ShaFile):
            raise TypeError
        return self.id <= other.id
    # nw_e: [[ShaFile]] methods #
    # nw_s: [[ShaFile]] methods |990ce7b522e60075a898e191049e9f4d#
    def __cmp__(self, other):
        if not isinstance(other, ShaFile):
            raise TypeError
        return cmp(self.id, other.id)
    # nw_e: [[ShaFile]] methods #
# nw_e: class ShaFile #

# nw_s: class Blob |0523c37cbf78cb4d7d4b422a2f01af75#
class Blob(ShaFile):
    """A Git Blob object."""

    __slots__ = ()

    type_name = b'blob'
    type_num = 3

    # nw_s: [[Blob]] methods |01707bff2e2f5d3c322c67048511f327#
    # nw_s: method Blob._get_data |bd9ad7c4fbd4ef06dd1a8d5085a3e626#
    def _get_data(self):
        return self.as_raw_string()
    # nw_e: method Blob._get_data #
    # nw_s: method Blob._set_data |49410ecf1c53cbb30e0b2b3773468c68#
    def _set_data(self, data):
        self.set_raw_string(data)
    # nw_e: method Blob._set_data #
    data = property(_get_data, _set_data,
                    "The text contained within the blob object.")

    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |115b73a0fb02f1ea0ffb7251d55813f2#
    # nw_s: method Blob._get_chunked |cc097aa5be2aca42b859ffdca9a497ab#
    def _get_chunked(self):
        return self._chunked_text
    # nw_e: method Blob._get_chunked #
    # nw_s: method Blob._set_chunked |b65b0fa1d0f61f1a3fecd4c4f1e634fb#
    def _set_chunked(self, chunks):
        self._chunked_text = chunks
    # nw_e: method Blob._set_chunked #
    chunked = property(
        _get_chunked, _set_chunked,
        "The text within the blob object, as chunks (not necessarily lines).")
    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |d66991b05ccbb4be3984b39908e065ca#
    def __init__(self):
        super(Blob, self).__init__()
        self._chunked_text = []
        self._needs_serialization = False
    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |9b12244a572731e7e4bceb2fa5fd01a0#
    def check(self):
        """Check this object for internal consistency.

        :raise ObjectFormatException: if the object is malformed in some way
        """
        super(Blob, self).check()

    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |ab5e99145baae8a765ca72d540932f6f#
    @classmethod
    def from_path(cls, path):
        blob = ShaFile.from_path(path)
        if not isinstance(blob, cls):
            raise NotBlobError(path)
        return blob
    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |843b8826dabda61bdaaf8a841b4e56c0#
    def _deserialize(self, chunks):
        self._chunked_text = chunks
    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |f918e8915a2771b9dc08dd6729d8d2bc#
    def _serialize(self):
        return self._chunked_text

    # nw_e: [[Blob]] methods #
    # nw_s: [[Blob]] methods |3202a2e927b559d422d25d9162d9ca6a#
    def splitlines(self):
        """Return list of lines in this blob.

        This preserves the original line endings.
        """
        chunks = self.chunked
        if not chunks:
            return []
        if len(chunks) == 1:
            return chunks[0].splitlines(True)
        remaining = None
        ret = []
        for chunk in chunks:
            lines = chunk.splitlines(True)
            if len(lines) > 1:
                ret.append((remaining or b"") + lines[0])
                ret.extend(lines[1:-1])
                remaining = lines[-1]
            elif len(lines) == 1:
                if remaining is None:
                    remaining = lines.pop()
                else:
                    remaining += lines.pop()
        if remaining is not None:
            ret.append(remaining)
        return ret
    # nw_e: [[Blob]] methods #
# nw_e: class Blob #

# nw_s: function objects._parse_message |7510801ae4d62cb8aafd3dc030da9d48#
def _parse_message(chunks):
    """Parse a message with a list of fields and a body.

    :param chunks: the raw chunks of the tag or commit object.
    :return: iterator of tuples of (field, value), one per header line, in the
        order read from the text, possibly including duplicates. Includes a
        field named None for the freeform tag/commit text.
    """
    f = BytesIO(b''.join(chunks))
    k = None
    v = ""
    eof = False

    def _strip_last_newline(value):
        """Strip the last newline from value"""
        if value and value.endswith(b'\n'):
            return value[:-1]
        return value

    # Parse the headers
    #
    # Headers can contain newlines. The next line is indented with a space.
    # We store the latest key as 'k', and the accumulated value as 'v'.
    for l in f:
        if l.startswith(b' '):
            # Indented continuation of the previous line
            v += l[1:]
        else:
            if k is not None:
                # We parsed a new header, return its value
                yield (k, _strip_last_newline(v))
            if l == b'\n':
                # Empty line indicates end of headers
                break
            (k, v) = l.split(b' ', 1)

    else:
        # We reached end of file before the headers ended. We still need to
        # return the previous header, then we need to return a None field for
        # the text.
        eof = True
        if k is not None:
            yield (k, _strip_last_newline(v))
        yield (None, None)

    if not eof:
        # We didn't reach the end of file while parsing headers. We can return
        # the rest of the file as a message.
        yield (None, f.read())

    f.close()
# nw_e: function objects._parse_message #

# nw_s: class Tag |9e990bab08255d100723cd5731d625b2#
class Tag(ShaFile):
    """A Git Tag object."""

    type_name = b'tag'
    type_num = 4

    __slots__ = ('_tag_timezone_neg_utc', '_name', '_object_sha',
                 '_object_class', '_tag_time', '_tag_timezone',
                 '_tagger', '_message')
    # nw_s: [[Tag]] methods |2c5f4a720b17e14994def89838f5c222#
    @classmethod
    def from_path(cls, filename):
        tag = ShaFile.from_path(filename)
        if not isinstance(tag, cls):
            raise NotTagError(filename)
        return tag

    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |3092715d5039a061b615cfec02884d1f#
    def __init__(self):
        super(Tag, self).__init__()
        self._tagger = None
        self._tag_time = None
        self._tag_timezone = None
        self._tag_timezone_neg_utc = False
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |565251a52c0f65ca77eded2a264e4b5e#
    # nw_s: method Tag._get_object |b39eabac9b1b303d8417ca212c3116bd#
    def _get_object(self):
        """Get the object pointed to by this tag.

        :return: tuple of (object class, sha).
        """
        return (self._object_class, self._object_sha)
    # nw_e: method Tag._get_object #
    # nw_s: method Tag._set_object |fa5acaee85d8a5a62bd3e43653cd9864#
    def _set_object(self, value):
        (self._object_class, self._object_sha) = value
        self._needs_serialization = True

    # nw_e: method Tag._set_object #
    object = property(_get_object, _set_object)
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |cff265c98a082aa030c63ffb2b0c2c1f#
    name = serializable_property("name", "The name of this tag")
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |eee9aa38e290ad1b15d3dfb51e817416#
    tagger = serializable_property("tagger",
        "Returns the name of the person who created this tag")
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |873bbacaa0751231a7306b7cb7d77ad2#
    tag_time = serializable_property("tag_time",
        "The creation timestamp of the tag.  As the number of seconds "
        "since the epoch")
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |4bd9a536302a99487d06b12356a14b34#
    tag_timezone = serializable_property("tag_timezone",
        "The timezone that tag_time is in.")
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |bad36aa5bcea8d96867f70c0c8654552#
    message = serializable_property(
        "message", "The message attached to this tag")
    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |b1de4cc5a5295ce7593166cf7c995cd1#
    def check(self):
        """Check this object for internal consistency.

        :raise ObjectFormatException: if the object is malformed in some way
        """
        super(Tag, self).check()
        self._check_has_member("_object_sha", "missing object sha")
        self._check_has_member("_object_class", "missing object type")
        self._check_has_member("_name", "missing tag name")

        if not self._name:
            raise ObjectFormatException("empty tag name")

        check_hexsha(self._object_sha, "invalid object sha")

        if getattr(self, "_tagger", None):
            check_identity(self._tagger, "invalid tagger")

        last = None
        for field, _ in _parse_message(self._chunked_text):
            if field == _OBJECT_HEADER and last is not None:
                raise ObjectFormatException("unexpected object")
            elif field == _TYPE_HEADER and last != _OBJECT_HEADER:
                raise ObjectFormatException("unexpected type")
            elif field == _TAG_HEADER and last != _TYPE_HEADER:
                raise ObjectFormatException("unexpected tag name")
            elif field == _TAGGER_HEADER and last != _TAG_HEADER:
                raise ObjectFormatException("unexpected tagger")
            last = field

    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |af444e3547064f8456fcd2f7223494cf#
    def _deserialize(self, chunks):
        """Grab the metadata attached to the tag"""
        self._tagger = None
        self._tag_time = None
        self._tag_timezone = None
        self._tag_timezone_neg_utc = False
        for field, value in _parse_message(chunks):
            if field == _OBJECT_HEADER:
                self._object_sha = value
            elif field == _TYPE_HEADER:
                obj_class = object_class(value)
                if not obj_class:
                    raise ObjectFormatException("Not a known type: %s" % value)
                self._object_class = obj_class
            elif field == _TAG_HEADER:
                self._name = value
            elif field == _TAGGER_HEADER:
                try:
                    sep = value.index(b'> ')
                except ValueError:
                    self._tagger = value
                    self._tag_time = None
                    self._tag_timezone = None
                    self._tag_timezone_neg_utc = False
                else:
                    self._tagger = value[0:sep+1]
                    try:
                        (timetext, timezonetext) = value[sep+2:].rsplit(b' ', 1)
                        self._tag_time = int(timetext)
                        self._tag_timezone, self._tag_timezone_neg_utc = \
                                parse_timezone(timezonetext)
                    except ValueError as e:
                        raise ObjectFormatException(e)
            elif field is None:
                self._message = value
            else:
                raise ObjectFormatException("Unknown field %s" % field)

    # nw_e: [[Tag]] methods #
    # nw_s: [[Tag]] methods |0c23a063d9c1d9f281ef9e927b92aa09#
    def _serialize(self):
        chunks = []
        chunks.append(git_line(_OBJECT_HEADER, self._object_sha))
        chunks.append(git_line(_TYPE_HEADER, self._object_class.type_name))
        chunks.append(git_line(_TAG_HEADER, self._name))
        if self._tagger:
            if self._tag_time is None:
                chunks.append(git_line(_TAGGER_HEADER, self._tagger))
            else:
                chunks.append(git_line(
                    _TAGGER_HEADER, self._tagger,
                    str(self._tag_time).encode('ascii'),
                    format_timezone(self._tag_timezone, self._tag_timezone_neg_utc)))
        if self._message is not None:
            chunks.append(b'\n') # To close headers
            chunks.append(self._message)
        return chunks

    # nw_e: [[Tag]] methods #

# nw_e: class Tag #

class TreeEntry(namedtuple('TreeEntry', ['path', 'mode', 'sha'])):
    """Named tuple encapsulating a single tree entry."""

    def in_path(self, path):
        """Return a copy of this entry with the given path prepended."""
        if not isinstance(self.path, bytes):
            raise TypeError('Expected bytes for path, got %r' % path)
        return TreeEntry(posixpath.join(path, self.path), self.mode, self.sha)

# nw_s: function object.parse_tree |e2e52336ccad080b1bb822ac90e71756#
def parse_tree(text, strict=False):
    """Parse a tree text.

    :param text: Serialized text to parse
    :return: iterator of tuples of (name, mode, sha)
    :raise ObjectFormatException: if the object was malformed in some way
    """
    count = 0
    l = len(text)
    while count < l:
        mode_end = text.index(b' ', count)
        mode_text = text[count:mode_end]
        if strict and mode_text.startswith(b'0'):
            raise ObjectFormatException("Invalid mode '%s'" % mode_text)
        try:
            mode = int(mode_text, 8)
        except ValueError:
            raise ObjectFormatException("Invalid mode '%s'" % mode_text)
        name_end = text.index(b'\0', mode_end)
        name = text[mode_end+1:name_end]
        count = name_end+21
        sha = text[name_end+1:count]
        if len(sha) != 20:
            raise ObjectFormatException("Sha has invalid length")
        hexsha = sha_to_hex(sha)
        yield (name, mode, hexsha)
# nw_e: function object.parse_tree #

# nw_s: function object.serialize_tree |f0a63833c1a0a6ae252afa0f67d23c67#
def serialize_tree(items):
    """Serialize the items in a tree to a text.

    :param items: Sorted iterable over (name, mode, sha) tuples
    :return: Serialized tree text as chunks
    """
    for name, mode, hexsha in items:
        yield ("%04o" % mode).encode('ascii') + b' ' + name + b'\0' + hex_to_sha(hexsha)
# nw_e: function object.serialize_tree #


def sorted_tree_items(entries, name_order):
    """Iterate over a tree entries dictionary.

    :param name_order: If True, iterate entries in order of their name. If
        False, iterate entries in tree order, that is, treat subtree entries as
        having '/' appended.
    :param entries: Dictionary mapping names to (mode, sha) tuples
    :return: Iterator over (name, mode, hexsha)
    """
    key_func = name_order and key_entry_name_order or key_entry
    for name, entry in sorted(entries.items(), key=key_func):
        mode, hexsha = entry
        # Stricter type checks than normal to mirror checks in the C version.
        mode = int(mode)
        if not isinstance(hexsha, bytes):
            raise TypeError('Expected bytes for SHA, got %r' % hexsha)
        yield TreeEntry(name, mode, hexsha)


def key_entry(entry):
    """Sort key for tree entry.

    :param entry: (name, value) tuplee
    """
    (name, value) = entry
    if stat.S_ISDIR(value[0]):
        name += b'/'
    return name


def key_entry_name_order(entry):
    """Sort key for tree entry in name order."""
    return entry[0]


def pretty_format_tree_entry(name, mode, hexsha, encoding="utf-8"):
    """Pretty format tree entry.

    :param name: Name of the directory entry
    :param mode: Mode of entry
    :param hexsha: Hexsha of the referenced object
    :return: string describing the tree entry
    """
    if mode & stat.S_IFDIR:
        kind = "tree"
    else:
        kind = "blob"
    return "%04o %s %s\t%s\n" % (
            mode, kind, hexsha.decode('ascii'),
            name.decode(encoding, 'replace'))

# nw_s: class Tree |800d5fbdf7a1b8aea53513c9652e9231#
class Tree(ShaFile):
    """A Git tree object"""

    type_name = b'tree'
    type_num = 2

    __slots__ = ('_entries')

    # nw_s: [[Tree]] methods |d7fbb1e5067060696b7ca78bc04a8a77#
    def __init__(self):
        super(Tree, self).__init__()
        self._entries = {}
    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |eaef2ef44b69e34583e883fa8cbab213#
    def check(self):
        """Check this object for internal consistency.

        :raise ObjectFormatException: if the object is malformed in some way
        """
        super(Tree, self).check()
        last = None
        allowed_modes = (stat.S_IFREG | 0o755, stat.S_IFREG | 0o644,
                         stat.S_IFLNK, stat.S_IFDIR, S_IFGITLINK,
                         # TODO: optionally exclude as in git fsck --strict
                         stat.S_IFREG | 0o664)
        for name, mode, sha in parse_tree(b''.join(self._chunked_text),
                                          True):
            check_hexsha(sha, 'invalid sha %s' % sha)
            if b'/' in name or name in (b'', b'.', b'..'):
                raise ObjectFormatException('invalid name %s' % name)

            if mode not in allowed_modes:
                raise ObjectFormatException('invalid mode %06o' % mode)

            entry = (name, (mode, sha))
            if last:
                if key_entry(last) > key_entry(entry):
                    raise ObjectFormatException('entries not sorted')
                if name == last[0]:
                    raise ObjectFormatException('duplicate entry %s' % name)
            last = entry

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |455f77463f4e48490a40be5fa6c28400#
    def add(self, name, mode, hexsha):
        """Add an entry to the tree.

        :param mode: The mode of the entry as an integral type. Not all
            possible modes are supported by git; see check() for details.
        :param name: The name of the entry, as a string.
        :param hexsha: The hex SHA of the entry as a string.
        """
        self._entries[name] = mode, hexsha
        self._needs_serialization = True
    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |5d51fe69d0dd1caff9eed703042c6228#
    def __getitem__(self, name):
        return self._entries[name]

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |989b675d7177b5d349295a32cb11fa1e#
    def __setitem__(self, name, value):
        """Set a tree entry by name.

        :param name: The name of the entry, as a string.
        :param value: A tuple of (mode, hexsha), where mode is the mode of the
            entry as an integral type and hexsha is the hex SHA of the entry as
            a string.
        """
        mode, hexsha = value
        self._entries[name] = (mode, hexsha)
        self._needs_serialization = True

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |28be2af9176dda36366c5220b9dae04c#
    def __delitem__(self, name):
        del self._entries[name]
        self._needs_serialization = True

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |4178decb3a3eb0feb44bad21a6b0ffbc#
    @classmethod
    def from_path(cls, filename):
        tree = ShaFile.from_path(filename)
        if not isinstance(tree, cls):
            raise NotTreeError(filename)
        return tree
    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |0513cc45636129210378ffaebebeacd2#
    def _deserialize(self, chunks):
        """Grab the entries in the tree"""
        try:
            parsed_entries = parse_tree(b''.join(chunks))
        except ValueError as e:
            raise ObjectFormatException(e)
        # TODO: list comprehension is for efficiency in the common (small)
        # case; if memory efficiency in the large case is a concern, use a genexp.
        self._entries = dict([(n, (m, s)) for n, m, s in parsed_entries])

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |b0a3a71394dc24423407d14674cc88b6#
    def _serialize(self):
        return list(serialize_tree(self.iteritems()))
    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |2469bc0a5f793b2331ec85143c6b179f#
    def __contains__(self, name):
        return name in self._entries

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |99f0f4398a3882b9b479cce87f451909#
    def __len__(self):
        return len(self._entries)

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |2a47da5f060a6bd958ccb1719efe9ee4#
    def __iter__(self):
        return iter(self._entries)

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |f0283ec78c93fafe8ef719954c73ec1c#
    def iteritems(self, name_order=False):
        """Iterate over entries.

        :param name_order: If True, iterate in name order instead of tree
            order.
        :return: Iterator over (name, mode, sha) tuples
        """
        return sorted_tree_items(self._entries, name_order)

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |72ceeb7cb4e7b9ab0448d0c7640dd8b9#
    def items(self):
        """Return the sorted entries in this tree.

        :return: List with (name, mode, sha) tuples
        """
        return list(self.iteritems())

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |4007aff5295be783abf530961272d573#
    def as_pretty_string(self):
        text = []
        for name, mode, hexsha in self.iteritems():
            text.append(pretty_format_tree_entry(name, mode, hexsha))
        return "".join(text)

    # nw_e: [[Tree]] methods #
    # nw_s: [[Tree]] methods |b5510e04c198d200552aa0737d3b5b5a#
    def lookup_path(self, lookup_obj, path):
        """Look up an object in a Git tree.

        :param lookup_obj: Callback for retrieving object by SHA1
        :param path: Path to lookup
        :return: A tuple of (mode, SHA) of the resulting path.
        """
        parts = path.split(b'/')
        sha = self.id
        mode = None
        for p in parts:
            if not p:
                continue
            obj = lookup_obj(sha)
            if not isinstance(obj, Tree):
                raise NotTreeError(sha)
            mode, sha = obj[p]
        return mode, sha
    # nw_e: [[Tree]] methods #
# nw_e: class Tree #



def parse_timezone(text):
    """Parse a timezone text fragment (e.g. '+0100').

    :param text: Text to parse.
    :return: Tuple with timezone as seconds difference to UTC
        and a boolean indicating whether this was a UTC timezone
        prefixed with a negative sign (-0000).
    """
    # cgit parses the first character as the sign, and the rest
    #  as an integer (using strtol), which could also be negative.
    #  We do the same for compatibility. See #697828.
    if not text[0] in b'+-':
        raise ValueError("Timezone must start with + or - (%(text)s)" % vars())
    sign = text[:1]
    offset = int(text[1:])
    if sign == b'-':
        offset = -offset
    unnecessary_negative_timezone = (offset >= 0 and sign == b'-')
    signum = (offset < 0) and -1 or 1
    offset = abs(offset)
    hours = int(offset / 100)
    minutes = (offset % 100)
    return (signum * (hours * 3600 + minutes * 60),
            unnecessary_negative_timezone)


def format_timezone(offset, unnecessary_negative_timezone=False):
    """Format a timezone for Git serialization.

    :param offset: Timezone offset as seconds difference to UTC
    :param unnecessary_negative_timezone: Whether to use a minus sign for
        UTC or positive timezones (-0000 and --700 rather than +0000 / +0700).
    """
    if offset % 60 != 0:
        raise ValueError("Unable to handle non-minute offset.")
    if offset < 0 or unnecessary_negative_timezone:
        sign = '-'
        offset = -offset
    else:
        sign = '+'
    return ('%c%02d%02d' % (sign, offset / 3600, (offset / 60) % 60)).encode('ascii')

# nw_s: function objects.parse_commit |95209456761b717bbf0150a5ad04b4a7#
def parse_commit(chunks):
    """Parse a commit object from chunks.

    :param chunks: Chunks to parse
    :return: Tuple of (tree, parents, author_info, commit_info,
        encoding, mergetag, gpgsig, message, extra)
    """
    parents = []
    extra = []
    tree = None
    author_info = (None, None, (None, None))
    commit_info = (None, None, (None, None))
    encoding = None
    mergetag = []
    message = None
    gpgsig = None

    for field, value in _parse_message(chunks):
        # TODO(jelmer): Enforce ordering
        if field == _TREE_HEADER:
            tree = value
        elif field == _PARENT_HEADER:
            parents.append(value)
        elif field == _AUTHOR_HEADER:
            author, timetext, timezonetext = value.rsplit(b' ', 2)
            author_time = int(timetext)
            author_info = (author, author_time, parse_timezone(timezonetext))
        elif field == _COMMITTER_HEADER:
            committer, timetext, timezonetext = value.rsplit(b' ', 2)
            commit_time = int(timetext)
            commit_info = (committer, commit_time, parse_timezone(timezonetext))
        elif field == _ENCODING_HEADER:
            encoding = value
        elif field == _MERGETAG_HEADER:
            mergetag.append(Tag.from_string(value + b'\n'))
        elif field == _GPGSIG_HEADER:
            gpgsig = value
        elif field is None:
            message = value
        else:
            extra.append((field, value))
    return (tree, parents, author_info, commit_info, encoding, mergetag,
            gpgsig, message, extra)

# nw_e: function objects.parse_commit #

# nw_s: class Commit |1b35160b985698760b68b9b3f43b5575#
class Commit(ShaFile):
    """A git commit object"""

    type_name = b'commit'
    type_num = 1

    __slots__ = ('_parents', '_encoding', '_extra', '_author_timezone_neg_utc',
                 '_commit_timezone_neg_utc', '_commit_time',
                 '_author_time', '_author_timezone', '_commit_timezone',
                 '_author', '_committer', '_tree', '_message',
                 '_mergetag', '_gpgsig')

    # nw_s: [[Commit]] methods |cc52c896e8488d05786ce83dd8442127#
    def __init__(self):
        super(Commit, self).__init__()
        self._parents = []
        self._encoding = None
        self._mergetag = []
        self._gpgsig = None
        self._extra = []
        self._author_timezone_neg_utc = False
        self._commit_timezone_neg_utc = False
    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |d2ae1c7b98f5f15ac8ef437e183d93c4#
    tree = serializable_property(
        "tree", "Tree that is the state of this commit")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |0cd7e57b050b9d98a98626477560b702#
    # nw_s: method Common._get_parents |c78795930531579145261947c3873e08#
    def _get_parents(self):
        """Return a list of parents of this commit."""
        return self._parents
    # nw_e: method Common._get_parents #
    # nw_s: method Common._set_parents |650c5a864d79317ba0fa40abe096559c#
    def _set_parents(self, value):
        """Set a list of parents of this commit."""
        self._needs_serialization = True
        self._parents = value
    # nw_e: method Common._set_parents #
    parents = property(_get_parents, _set_parents,
                       doc="Parents of this commit, by their SHA1.")
    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |2f779cd394e846ca1f44d854e39d6955#
    author = serializable_property("author",
        "The name of the author of the commit")
    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |a97e6c5a8d34ccba69ec431fb4ba96ce#
    committer = serializable_property("committer",
        "The name of the committer of the commit")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |fdc01c2c6dcf4ad472979d11b7bbce1a#
    message = serializable_property(
        "message", "The commit message")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |303e508e81ffd17b6b66e115ec4d1467#
    commit_time = serializable_property("commit_time",
        "The timestamp of the commit. As the number of seconds since the epoch.")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |9482e9b2c731fc798b239a90c3d355c5#
    commit_timezone = serializable_property("commit_timezone",
        "The zone the commit time is in")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |87857048c178d6809c4247e6279633f5#
    author_time = serializable_property("author_time",
        "The timestamp the commit was written. As the number of "
        "seconds since the epoch.")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |cb422573069bedd296297a10c2cbf016#
    author_timezone = serializable_property(
        "author_timezone", "Returns the zone the author time is in.")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |e2a413fe5a685808ad26b275e251b9ec#
    encoding = serializable_property(
        "encoding", "Encoding of the commit message.")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |6576ff6ad51e6768a2a8de33e22206ea#
    mergetag = serializable_property(
        "mergetag", "Associated signed tag.")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |fa4ed4d471138a1c3173562e83f0c7de#
    gpgsig = serializable_property(
        "gpgsig", "GPG Signature.")
    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |ad81993d26b08a3c2b8310607de9c439#
    # nw_s: method Commit._get_extra |b978d8bfb267627f175a10ea7900125e#
    def _get_extra(self):
        """Return extra settings of this commit."""
        return self._extra

    # nw_e: method Commit._get_extra #
    extra = property(_get_extra,
        doc="Extra header fields not understood (presumably added in a "
            "newer version of git). Kept verbatim so the object can "
            "be correctly reserialized. For private commit metadata, use "
            "pseudo-headers in Commit.message, rather than this field.")

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |4014485108c50d4c00bccb1048187bfc#
    def check(self):
        """Check this object for internal consistency.

        :raise ObjectFormatException: if the object is malformed in some way
        """
        super(Commit, self).check()
        self._check_has_member("_tree", "missing tree")
        self._check_has_member("_author", "missing author")
        self._check_has_member("_committer", "missing committer")
        # times are currently checked when set

        for parent in self._parents:
            check_hexsha(parent, "invalid parent sha")
        check_hexsha(self._tree, "invalid tree sha")

        check_identity(self._author, "invalid author")
        check_identity(self._committer, "invalid committer")

        # nw_s: [[Commit.check()]] check message |ed60f0214577b9453099af342c3e1eef#
        last = None
        for field, _ in _parse_message(self._chunked_text):
            if field == _TREE_HEADER and last is not None:
                raise ObjectFormatException("unexpected tree")
            elif field == _PARENT_HEADER and last not in (_PARENT_HEADER,
                                                          _TREE_HEADER):
                raise ObjectFormatException("unexpected parent")
            elif field == _AUTHOR_HEADER and last not in (_TREE_HEADER,
                                                          _PARENT_HEADER):
                raise ObjectFormatException("unexpected author")
            elif field == _COMMITTER_HEADER and last != _AUTHOR_HEADER:
                raise ObjectFormatException("unexpected committer")
            elif field == _ENCODING_HEADER and last != _COMMITTER_HEADER:
                raise ObjectFormatException("unexpected encoding")
            last = field
        # nw_e: [[Commit.check()]] check message #
        # TODO: optionally check for duplicate parents

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |71ff000201f33d1478354f079e3de044#
    @classmethod
    def from_path(cls, path):
        commit = ShaFile.from_path(path)
        if not isinstance(commit, cls):
            raise NotCommitError(path)
        return commit

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |1519efb29111027e30548a36baabbfde#
    def _deserialize(self, chunks):
        (self._tree, self._parents, author_info, commit_info, self._encoding,
                self._mergetag, self._gpgsig, self._message, self._extra) = (
                        parse_commit(chunks))
        (self._author, self._author_time, (self._author_timezone,
             self._author_timezone_neg_utc)) = author_info
        (self._committer, self._commit_time, (self._commit_timezone,
             self._commit_timezone_neg_utc)) = commit_info

    # nw_e: [[Commit]] methods #
    # nw_s: [[Commit]] methods |98b66aff5419b9760105e805ba091a13#
    def _serialize(self):
        chunks = []
        tree_bytes = self._tree.id if isinstance(self._tree, Tree) else self._tree
        chunks.append(git_line(_TREE_HEADER, tree_bytes))
        for p in self._parents:
            chunks.append(git_line(_PARENT_HEADER, p))
        chunks.append(git_line(
            _AUTHOR_HEADER, self._author, str(self._author_time).encode('ascii'),
            format_timezone(self._author_timezone,
                            self._author_timezone_neg_utc)))
        chunks.append(git_line(
            _COMMITTER_HEADER, self._committer, str(self._commit_time).encode('ascii'),
            format_timezone(self._commit_timezone,
                            self._commit_timezone_neg_utc)))
        if self.encoding:
            chunks.append(git_line(_ENCODING_HEADER, self.encoding))
        for mergetag in self.mergetag:
            mergetag_chunks = mergetag.as_raw_string().split(b'\n')

            chunks.append(git_line(_MERGETAG_HEADER, mergetag_chunks[0]))
            # Embedded extra header needs leading space
            for chunk in mergetag_chunks[1:]:
                chunks.append(b' ' + chunk + b'\n')

            # No trailing empty line
            if chunks[-1].endswith(b' \n'):
                chunks[-1] = chunks[-1][:-2]
        for k, v in self.extra:
            if b'\n' in k or b'\n' in v:
                raise AssertionError(
                    "newline in extra data: %r -> %r" % (k, v))
            chunks.append(git_line(k, v))
        if self.gpgsig:
            sig_chunks = self.gpgsig.split(b'\n')
            chunks.append(git_line(_GPGSIG_HEADER, sig_chunks[0]))
            for chunk in sig_chunks[1:]:
                chunks.append(git_line(b'',  chunk))
        chunks.append(b'\n')  # There must be a new line after the headers
        chunks.append(self._message)
        return chunks

    # nw_e: [[Commit]] methods #
# nw_e: class Commit #


# nw_s: constant OBJECT_CLASSES |21cfbea91abd87f840f95afd0872c079#
OBJECT_CLASSES = (
    Commit,
    Tree,
    Blob,
    Tag,
    )
# nw_e: constant OBJECT_CLASSES #

# nw_s: global _TYPE_MAP |7125249470dd3867e5a339af50f99a69#
_TYPE_MAP = {}
# nw_e: global _TYPE_MAP #

# nw_s: [[objects.py]] toplevel |0952d67eda6eb74b402789e0ae10cb7f#
for cls in OBJECT_CLASSES:
    _TYPE_MAP[cls.type_name] = cls
    _TYPE_MAP[cls.type_num] = cls
# nw_e: [[objects.py]] toplevel #


# Hold on to the pure-python implementations for testing
_parse_tree_py = parse_tree
_sorted_tree_items_py = sorted_tree_items
try:
    # Try to import C versions
    from dulwich._objects import parse_tree, sorted_tree_items
except ImportError:
    pass
# nw_e: dulwich/objects.py #
