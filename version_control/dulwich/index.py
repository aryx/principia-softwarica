# nw_s: dulwich/index.py |3583ca0031119bce624b9b0f99199b4d#
# index.py -- File parser/writer for the git index file
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
"""Parser for the git index file format."""

import collections
import errno
import os
import stat
import struct
import sys

from dulwich.file import GitFile
from dulwich.objects import (
    Blob,
    S_IFGITLINK,
    S_ISGITLINK,
    Tree,
    hex_to_sha,
    sha_to_hex,
    )
from dulwich.pack import (
    SHA1Reader,
    SHA1Writer,
    )

# nw_s: type IndexEntry |5cf897f2e6af0306b0ca3ad990a04e30#
IndexEntry = collections.namedtuple(
    'IndexEntry', [
        'ctime', 'mtime', 'dev', 'ino', 'mode', 'uid', 'gid', 'size', 'sha',
        'flags'])
# nw_e: type IndexEntry #

# nw_s: function index.pathsplit |b73cae439589607497d9bf0ba22c7fbf#
def pathsplit(path):
    """Split a /-delimited path into a directory part and a basename.

    :param path: The path to split.
    :return: Tuple with directory name and basename
    """
    try:
        (dirname, basename) = path.rsplit(b"/", 1)
    except ValueError:
        return (b"", path)
    else:
        return (dirname, basename)
# nw_e: function index.pathsplit #

# nw_s: function index.pathjoin |fd1fb149887eb4fd9c58ed9443c3dfe8#
def pathjoin(*args):
    """Join a /-delimited path.

    """
    return b"/".join([p for p in args if p])
# nw_e: function index.pathjoin #

# nw_s: function index.read_cache_time |052479542dac7ea48e962342991fe4c1#
def read_cache_time(f):
    """Read a cache time.

    :param f: File-like object to read from
    :return: Tuple with seconds and nanoseconds
    """
    return struct.unpack(">LL", f.read(8))
# nw_e: function index.read_cache_time #

# nw_s: function index.write_cache_time |0e936e007ebf93667a2c35c71df2875d#
def write_cache_time(f, t):
    """Write a cache time.

    :param f: File-like object to write to
    :param t: Time to write (as int, float or tuple with secs and nsecs)
    """
    if isinstance(t, int):
        t = (t, 0)
    elif isinstance(t, float):
        (secs, nsecs) = divmod(t, 1.0)
        t = (int(secs), int(nsecs * 1000000000))
    elif not isinstance(t, tuple):
        raise TypeError(t)
    f.write(struct.pack(">LL", *t))
# nw_e: function index.write_cache_time #

# nw_s: function index.read_cache_entry |a9f93d1414698c2b248ebf1db904d7cf#
def read_cache_entry(f):
    """Read an entry from a cache file.

    :param f: File-like object to read from
    :return: tuple with: device, inode, mode, uid, gid, size, sha, flags
    """
    beginoffset = f.tell()
    ctime = read_cache_time(f)
    mtime = read_cache_time(f)
    (dev, ino, mode, uid, gid, size, sha, flags, ) = \
        struct.unpack(">LLLLLL20sH", f.read(20 + 4 * 6 + 2))
    name = f.read((flags & 0x0fff))
    # Padding:
    real_size = ((f.tell() - beginoffset + 8) & ~7)
    f.read((beginoffset + real_size) - f.tell())
    return (name, ctime, mtime, dev, ino, mode, uid, gid, size,
            sha_to_hex(sha), flags & ~0x0fff)
# nw_e: function index.read_cache_entry #

# nw_s: function index.write_cache_entry |1875df1eb07867a5eb18aa77525096fc#
def write_cache_entry(f, entry):
    """Write an index entry to a file.

    :param f: File object
    :param entry: Entry to write, tuple with:
        (name, ctime, mtime, dev, ino, mode, uid, gid, size, sha, flags)
    """
    beginoffset = f.tell()
    (name, ctime, mtime, dev, ino, mode, uid, gid, size, sha, flags) = entry
    write_cache_time(f, ctime)
    write_cache_time(f, mtime)
    flags = len(name) | (flags &~ 0x0fff)
    f.write(struct.pack(b'>LLLLLL20sH', dev & 0xFFFFFFFF, ino & 0xFFFFFFFF, mode, uid, gid, size, hex_to_sha(sha), flags))
    f.write(name)
    real_size = ((f.tell() - beginoffset + 8) & ~7)
    f.write(b'\0' * ((beginoffset + real_size) - f.tell()))
# nw_e: function index.write_cache_entry #

# nw_s: function read_index |149a7f19ffdcf586005c57dc74c51807#
def read_index(f):
    """Read an index file, yielding the individual entries."""
    header = f.read(4)
    if header != b'DIRC':
        raise AssertionError("Invalid index file header: %r" % header)
    (version, num_entries) = struct.unpack(b'>LL', f.read(4 * 2))
    assert version in (1, 2)
    for i in range(num_entries):
        yield read_cache_entry(f)
# nw_e: function read_index #

# nw_s: function read_index_dict |9a3fcf0c75aeb79428da929d4c5632c2#
def read_index_dict(f):
    """Read an index file and return it as a dictionary.

    :param f: File object to read from
    """
    ret = {}
    for x in read_index(f):
        ret[x[0]] = IndexEntry(*x[1:])
    return ret
# nw_e: function read_index_dict #

# nw_s: function write_index |b286166ed44d4ebd48b4ae55a35b4f74#
def write_index(f, entries):
    """Write an index file.

    :param f: File-like object to write to
    :param entries: Iterable over the entries to write
    """
    f.write(b'DIRC')
    f.write(struct.pack(b'>LL', 2, len(entries)))
    for x in entries:
        write_cache_entry(f, x)
# nw_e: function write_index #

# nw_s: function write_index_dict |d41dfdeec66d8758c61da10b32f956bc#
def write_index_dict(f, entries):
    """Write an index file based on the contents of a dictionary.

    """
    entries_list = []
    for name in sorted(entries):
        entries_list.append((name,) + tuple(entries[name]))
    write_index(f, entries_list)
# nw_e: function write_index_dict #

# nw_s: function index.cleanup_mode |9b2c554fd4aaec8dd54ce406af136a4e#
def cleanup_mode(mode):
    """Cleanup a mode value.

    This will return a mode that can be stored in a tree object.

    :param mode: Mode to clean up.
    """
    if stat.S_ISLNK(mode):
        return stat.S_IFLNK
    elif stat.S_ISDIR(mode):
        return stat.S_IFDIR
    elif S_ISGITLINK(mode):
        return S_IFGITLINK
    ret = stat.S_IFREG | 0o644
    ret |= (mode & 0o111)
    return ret
# nw_e: function index.cleanup_mode #

# nw_s: class Index |d08138f8e8637be60f5c884594bfacd1#
class Index(object):
    """A Git Index file."""

    # nw_s: [[Index]] methods |fb0edc9d830d930e0e5846794e45e0cb#
    def __init__(self, filename):
        """Open an index file.

        :param filename: Path to the index file
        """
        self._filename = filename
        self.clear()
        self.read()
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |45abfccf8ea8259e70fe870f0d80f43a#
    def clear(self):
        """Remove all contents from this index."""
        self._byname = {}

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |7b358f64f0a02369c28d892816eb9826#
    def __getitem__(self, name):
        """Retrieve entry by relative path.

        :return: tuple with (ctime, mtime, dev, ino, mode, uid, gid, size, sha, flags)
        """
        return self._byname[name]

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |44b841238acac08a28240d14b804878a#
    def __setitem__(self, name, x):
        assert isinstance(name, bytes)
        assert len(x) == 10
        # Remove the old entry if any
        self._byname[name] = x

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |69e4de202774a6f447aa21752815b9c6#
    def __delitem__(self, name):
        assert isinstance(name, bytes)
        del self._byname[name]

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |8917b84fb6d53065d94434a3975bd0f3#
    def read(self):
        """Read current contents of index from disk."""
        if not os.path.exists(self._filename):
            return
        f = GitFile(self._filename, 'rb')
        try:
            f = SHA1Reader(f)
            for x in read_index(f):
                self[x[0]] = IndexEntry(*x[1:])
            # FIXME: Additional data?
            f.read(os.path.getsize(self._filename)-f.tell()-20)
            f.check_sha()
        finally:
            f.close()

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |d917996e93af4a2a910b2eb6b8f2c0cb#
    def write(self):
        """Write current contents of index to disk."""
        f = GitFile(self._filename, 'wb')
        try:
            f = SHA1Writer(f)
            write_index_dict(f, self._byname)
        finally:
            f.close()

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |dfa11cbe2b3f769df08e474103a99fb3#
    def commit(self, object_store):
        """Create a new tree from an index.

        :param object_store: Object store to save the tree in
        :return: Root tree SHA
        """
        return commit_tree(object_store, self.iterblobs())
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |5ff0a1b157867f13405d93308f631c30#
    def changes_from_tree(self, object_store, tree, want_unchanged=False):
        """Find the differences between the contents of this index and a tree.

        :param object_store: Object store to use for retrieving tree contents
        :param tree: SHA1 of the root tree
        :param want_unchanged: Whether unchanged files should be reported
        :return: Iterator over tuples with (oldpath, newpath), (oldmode, newmode), (oldsha, newsha)
        """
        def lookup_entry(path):
            entry = self[path]
            return entry.sha, entry.mode
        for (name, mode, sha) in changes_from_tree(self._byname.keys(),
                lookup_entry, object_store, tree,
                want_unchanged=want_unchanged):
            yield (name, mode, sha)
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |9e5681f0bbcbd5460a22f18f2904ec82#
    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self._filename)
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |82d2bb732739ef3b42b598f991a3f581#
    @property
    def path(self):
        return self._filename

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |fe374675acbe36754052efdf1488ae69#
    def __len__(self):
        """Number of entries in this index file."""
        return len(self._byname)
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |6cd809fd6ed911d060b62a575f48b802#
    def __iter__(self):
        """Iterate over the paths in this index."""
        return iter(self._byname)
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |4b8b39dea8c049f515bdec596c9860a2#
    def get_sha1(self, path):
        """Return the (git object) SHA1 for the object at a path."""
        return self[path].sha
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |4a571aa619db74a36fc757394e1fab1d#
    def get_mode(self, path):
        """Return the POSIX file mode for the object at a path."""
        return self[path].mode

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |879316496682ceed5c29d5706a5082e6#
    def iterblobs(self):
        """Iterate over path, sha, mode tuples for use with commit_tree."""
        for path in self:
            entry = self[path]
            yield path, entry.sha, cleanup_mode(entry.mode)
    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |43a374dc67b48049b40d54e2e4c495b7#
    def iteritems(self):
        return self._byname.items()

    # nw_e: [[Index]] methods #
    # nw_s: [[Index]] methods |6d5a56795a88fd186d0a892eb0c4bfb5#
    def update(self, entries):
        for name, value in entries.items():
            self[name] = value

    # nw_e: [[Index]] methods #
# nw_e: class Index #

# nw_s: function index.commit_tree |07555e38e6249e930ab9f187d448986c#
def commit_tree(object_store, blobs):
    """Commit a new tree.

    :param object_store: Object store to add trees to
    :param blobs: Iterable over blob path, sha, mode entries
    :return: SHA1 of the created tree.
    """

    trees = {b'': {}}

    def add_tree(path):
        if path in trees:
            return trees[path]
        dirname, basename = pathsplit(path)
        t = add_tree(dirname)
        assert isinstance(basename, bytes)
        newtree = {}
        t[basename] = newtree
        trees[path] = newtree
        return newtree

    for path, sha, mode in blobs:
        tree_path, basename = pathsplit(path)
        tree = add_tree(tree_path)
        tree[basename] = (mode, sha)

    def build_tree(path):
        tree = Tree()
        for basename, entry in trees[path].items():
            if isinstance(entry, dict):
                mode = stat.S_IFDIR
                sha = build_tree(pathjoin(path, basename))
            else:
                (mode, sha) = entry
            tree.add(basename, mode, sha)
        object_store.add_object(tree)
        return tree.id
    return build_tree(b'')
# nw_e: function index.commit_tree #

# nw_s: function index.changes_from_tree |fb181269a9f9299ce04a4b9434bf962e#
def changes_from_tree(names, lookup_entry, object_store, tree,
        want_unchanged=False):
    """Find the differences between the contents of a tree and
    a working copy.

    :param names: Iterable of names in the working copy
    :param lookup_entry: Function to lookup an entry in the working copy
    :param object_store: Object store to use for retrieving tree contents
    :param tree: SHA1 of the root tree, or None for an empty tree
    :param want_unchanged: Whether unchanged files should be reported
    :return: Iterator over tuples with (oldpath, newpath), (oldmode, newmode),
        (oldsha, newsha)
    """
    other_names = set(names)

    if tree is not None:
        for (name, mode, sha) in object_store.iter_tree_contents(tree):
            try:
                (other_sha, other_mode) = lookup_entry(name)
            except KeyError:
                # Was removed
                yield ((name, None), (mode, None), (sha, None))
            else:
                other_names.remove(name)
                if (want_unchanged or other_sha != sha or other_mode != mode):
                    yield ((name, name), (mode, other_mode), (sha, other_sha))

    # Mention added files
    for name in other_names:
        try:
            (other_sha, other_mode) = lookup_entry(name)
        except KeyError:
            pass
        else:
            yield ((None, name), (None, other_mode), (None, other_sha))
# nw_e: function index.changes_from_tree #

# nw_s: function index.index_entry_from_stat |ede195789312cd664fe77cb530b82893#
def index_entry_from_stat(stat_val, hex_sha, flags, mode=None):
    """Create a new index entry from a stat value.

    :param stat_val: POSIX stat_result instance
    :param hex_sha: Hex sha of the object
    :param flags: Index flags
    """
    if mode is None:
        mode = cleanup_mode(stat_val.st_mode)
    return (stat_val.st_ctime, stat_val.st_mtime, stat_val.st_dev,
            stat_val.st_ino, mode, stat_val.st_uid,
            stat_val.st_gid, stat_val.st_size, hex_sha, flags)
# nw_e: function index.index_entry_from_stat #


# nw_s: function build_file_from_blob |8bcd88e93657e2237be4a543c7cc88d2#
def build_file_from_blob(blob, mode, target_path, honor_filemode=True):
    """Build a file or symlink on disk based on a Git object.

    :param obj: The git object
    :param mode: File mode
    :param target_path: Path to write to
    :param honor_filemode: An optional flag to honor core.filemode setting in
        config file, default is core.filemode=True, change executable bit
    :return: stat object for the file
    """
    try:
        oldstat = os.lstat(target_path)
    except OSError as e:
        if e.errno == errno.ENOENT:
            oldstat = None
        else:
            raise
    contents = blob.as_raw_string()
    if stat.S_ISLNK(mode):
        # FIXME: This will fail on Windows. What should we do instead?
        if oldstat:
            os.unlink(target_path)
        os.symlink(contents, target_path)
    else:
        if oldstat is not None and oldstat.st_size == len(contents):
            with open(target_path, 'rb') as f:
                if f.read() == contents:
                    return oldstat

        with open(target_path, 'wb') as f:
            # Write out file
            f.write(contents)

        if honor_filemode:
            os.chmod(target_path, mode)

    return os.lstat(target_path)
# nw_e: function build_file_from_blob #

# nw_s: constant index.INVALID_DOTNAMES |11794855ba5a07a25ea8a1ebedb635c4#
INVALID_DOTNAMES = (b".git", b".", b"..", b"")
# nw_e: constant index.INVALID_DOTNAMES #


# nw_s: function index.validate_path_element_default |cb4cbda72ea9f5073a07cc913064981a#
def validate_path_element_default(element):
    return element.lower() not in INVALID_DOTNAMES
# nw_e: function index.validate_path_element_default #

# nw_s: function index.validate_path |00a75c4fc7b7085002b7a68a7bafae7f#
def validate_path(path, element_validator=validate_path_element_default):
    """Default path validator that just checks for .git/."""
    parts = path.split(b"/")
    for p in parts:
        if not element_validator(p):
            return False
    else:
        return True
# nw_e: function index.validate_path #


# nw_s: function build_index_from_tree |8244added04a2e3658b84c6083716598#
def build_index_from_tree(root_path, index_path, object_store, tree_id,
                          honor_filemode=True,
                          validate_path_element=validate_path_element_default):
    """Generate and materialize index from a tree

    :param tree_id: Tree to materialize
    :param root_path: Target dir for materialized index files
    :param index_path: Target path for generated index
    :param object_store: Non-empty object store holding tree contents
    :param honor_filemode: An optional flag to honor core.filemode setting in
        config file, default is core.filemode=True, change executable bit
    :param validate_path_element: Function to validate path elements to check out;
        default just refuses .git and .. directories.

    :note:: existing index is wiped and contents are not merged
        in a working dir. Suitable only for fresh clones.
    """

    index = Index(index_path)
    if not isinstance(root_path, bytes):
        root_path = root_path.encode(sys.getfilesystemencoding())

    for entry in object_store.iter_tree_contents(tree_id):
        if not validate_path(entry.path, validate_path_element):
            continue
        full_path = _tree_to_fs_path(root_path, entry.path)

        if not os.path.exists(os.path.dirname(full_path)):
            os.makedirs(os.path.dirname(full_path))

        # TODO(jelmer): Merge new index into working tree
        if S_ISGITLINK(entry.mode):
            if not os.path.isdir(full_path):
                os.mkdir(full_path)
            st = os.lstat(full_path)
            # TODO(jelmer): record and return submodule paths
        else:
            obj = object_store[entry.sha]
            st = build_file_from_blob(
                obj, entry.mode, full_path, honor_filemode=honor_filemode)
        # Add file to index
        if not honor_filemode or S_ISGITLINK(entry.mode):
            # we can not use tuple slicing to build a new tuple,
            # because on windows that will convert the times to
            # longs, which causes errors further along
            st_tuple = (entry.mode, st.st_ino, st.st_dev, st.st_nlink,
                        st.st_uid, st.st_gid, st.st_size, st.st_atime,
                        st.st_mtime, st.st_ctime)
            st = st.__class__(st_tuple)
        index[entry.path] = index_entry_from_stat(st, entry.sha, 0)

    index.write()
# nw_e: function build_index_from_tree #

# nw_s: function index.blob_from_path_and_stat |e12db89c500c12a8b70e5a1996db61c6#
def blob_from_path_and_stat(fs_path, st):
    """Create a blob from a path and a stat object.

    :param fs_path: Full file system path to file
    :param st: A stat object
    :return: A `Blob` object
    """
    assert isinstance(fs_path, bytes)
    blob = Blob()
    if not stat.S_ISLNK(st.st_mode):
        with open(fs_path, 'rb') as f:
            blob.data = f.read()
    else:
            blob.data = os.readlink(fs_path)
    return blob
# nw_e: function index.blob_from_path_and_stat #

# nw_s: function index.get_unstaged_changes |680fc036907e68d4a88745e1e2581f0a#
def get_unstaged_changes(index, root_path):
    """Walk through an index and check for differences against working tree.

    :param index: index to check
    :param root_path: path in which to find files
    :return: iterator over paths with unstaged changes
    """
    # For each entry in the index check the sha1 & ensure not staged
    if not isinstance(root_path, bytes):
        root_path = root_path.encode(sys.getfilesystemencoding())

    for tree_path, entry in index.iteritems():
        full_path = _tree_to_fs_path(root_path, tree_path)
        # TODO(jelmer): handle S_ISGITLINK(entry.mode) here
        try:
            blob = blob_from_path_and_stat(full_path, os.lstat(full_path))
        except OSError as e:
            if e.errno != errno.ENOENT:
                raise
            # The file was removed, so we assume that counts as
            # different from whatever file used to exist.
            yield tree_path
        except IOError as e:
            if e.errno != errno.EISDIR:
                raise
            # The file was changed to a directory, so consider it removed.
            yield tree_path
        else:
            if blob.id != entry.sha:
                yield tree_path
# nw_e: function index.get_unstaged_changes #

# nw_s: constant index.os_sep_bytes |09f4625ac796b98e787789b04e2c11fd#
os_sep_bytes = os.sep.encode('ascii')
# nw_e: constant index.os_sep_bytes #

# nw_s: function index._tree_to_fs_path |68d191366ce09fe305c26e3b1b86fec3#
def _tree_to_fs_path(root_path, tree_path):
    """Convert a git tree path to a file system path.

    :param root_path: Root filesystem path
    :param tree_path: Git tree path as bytes

    :return: File system path.
    """
    assert isinstance(tree_path, bytes)
    if os_sep_bytes != b'/':
        sep_corrected_path = tree_path.replace(b'/', os_sep_bytes)
    else:
        sep_corrected_path = tree_path
    return os.path.join(root_path, sep_corrected_path)
# nw_e: function index._tree_to_fs_path #

# nw_s: function index._fs_to_tree_path |16de446362345824e5200d6a288a0ad5#
def _fs_to_tree_path(fs_path, fs_encoding=None):
    """Convert a file system path to a git tree path.

    :param fs_path: File system path.
    :param fs_encoding: File system encoding

    :return:  Git tree path as bytes
    """
    if fs_encoding is None:
        fs_encoding = sys.getfilesystemencoding()
    if not isinstance(fs_path, bytes):
        fs_path_bytes = fs_path.encode(fs_encoding)
    else:
        fs_path_bytes = fs_path
    if os_sep_bytes != b'/':
        tree_path = fs_path_bytes.replace(os_sep_bytes, b'/')
    else:
        tree_path = fs_path_bytes
    return tree_path
# nw_e: function index._fs_to_tree_path #
# nw_e: dulwich/index.py #
