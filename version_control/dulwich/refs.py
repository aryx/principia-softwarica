# nw_s: refs.py |87dd03d77019ab6532f6ec49f2d6f898#
# refs.py -- For dealing with git refs
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

"""Ref handling.

"""
import errno
import os
import sys

from dulwich.errors import (
    PackedRefsException,
    RefFormatError,
    )
from dulwich.objects import (
    git_line,
    valid_hexsha,
    ZERO_SHA,
    )
from dulwich.file import (
    GitFile,
    ensure_dir_exists,
    )

# nw_s: constant refs.SYMREF |c5e7859f5b53ca84a0cdcfc2f90f8f24#
SYMREF = b'ref: '
# nw_e: constant refs.SYMREF #
# nw_s: constant refs.LOCAL_BRANCH_PREFIX |5862203e9828d3b781772e57d05e528c#
LOCAL_BRANCH_PREFIX = b'refs/heads/'
# nw_e: constant refs.LOCAL_BRANCH_PREFIX #
# nw_s: constant refs.BAD_REF_CHARS |ef0a1d4818f3712e12d2f9a575ad3146#
BAD_REF_CHARS = set(b'\177 ~^:?*[')
# nw_e: constant refs.BAD_REF_CHARS #
# nw_s: constant refs.ANNOTATED_TAG_SUFFIX |33bef4b0b2620043e4c92c383f0d508c#
ANNOTATED_TAG_SUFFIX = b'^{}'
# nw_e: constant refs.ANNOTATED_TAG_SUFFIX #

# nw_s: function refs.check_ref_format |02160bb4ddc0f0fe52666fb076896362#
def check_ref_format(refname):
    """Check if a refname is correctly formatted.

    Implements all the same rules as git-check-ref-format[1].

    [1] http://www.kernel.org/pub/software/scm/git/docs/git-check-ref-format.html

    :param refname: The refname to check
    :return: True if refname is valid, False otherwise
    """
    # These could be combined into one big expression, but are listed separately
    # to parallel [1].
    if b'/.' in refname or refname.startswith(b'.'):
        return False
    if b'/' not in refname:
        return False
    if b'..' in refname:
        return False
    for i, c in enumerate(refname):
        if ord(refname[i:i+1]) < 0o40 or c in BAD_REF_CHARS:
            return False
    if refname[-1] in b'/.':
        return False
    if refname.endswith(b'.lock'):
        return False
    if b'@{' in refname:
        return False
    if b'\\' in refname:
        return False
    return True
# nw_e: function refs.check_ref_format #

# nw_s: class RefsContainer |20ed8827faf07d0dcee6ce87c80fd11d#
class RefsContainer(object):
    """A container for refs."""

    # nw_s: [[RefsContainer]] methods |8a454ca8288be48cbb2ff5ca10a20122#
    def __getitem__(self, name):
        """Get the SHA1 for a reference name.

        This method follows all symbolic references.
        """
        _, sha = self.follow(name)
        if sha is None:
            raise KeyError(name)
        return sha

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |ecb75ed7fed6324d133e128570f9b651#
    def __setitem__(self, name, ref):
        """Set a reference name to point to the given SHA1.

        This method follows all symbolic references if applicable for the
        subclass.

        :note: This method unconditionally overwrites the contents of a
            reference. To update atomically only if the reference has not
            changed, use set_if_equals().
        :param name: The refname to set.
        :param ref: The new sha the refname will refer to.
        """
        self.set_if_equals(name, None, ref)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |7e735ee2caf94d1d556c3a999217f0d9#
    def set_if_equals(self, name, old_ref, new_ref):
        """Set a refname to new_ref only if it currently equals old_ref.

        This method follows all symbolic references if applicable for the
        subclass, and can be used to perform an atomic compare-and-swap
        operation.

        :param name: The refname to set.
        :param old_ref: The old sha the refname must refer to, or None to set
            unconditionally.
        :param new_ref: The new sha the refname will refer to.
        :return: True if the set was successful, False otherwise.
        """
        raise NotImplementedError(self.set_if_equals)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |b4cb912ec8b467e3bbda0aa5f625338b#
    def __delitem__(self, name):
        """Remove a refname.

        This method does not follow symbolic references, even if applicable for
        the subclass.

        :note: This method unconditionally deletes the contents of a reference.
            To delete atomically only if the reference has not changed, use
            remove_if_equals().

        :param name: The refname to delete.
        """
        self.remove_if_equals(name, None)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |281755ec8115161f2fd17d128cd5da77#
    def remove_if_equals(self, name, old_ref):
        """Remove a refname only if it currently equals old_ref.

        This method does not follow symbolic references, even if applicable for
        the subclass. It can be used to perform an atomic compare-and-delete
        operation.

        :param name: The refname to delete.
        :param old_ref: The old sha the refname must refer to, or None to delete
            unconditionally.
        :return: True if the delete was successful, False otherwise.
        """
        raise NotImplementedError(self.remove_if_equals)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |4bae65d91c63ff1021501626482dc6d9#
    def set_symbolic_ref(self, name, other):
        """Make a ref point at another ref.

        :param name: Name of the ref to set
        :param other: Name of the ref to point at
        """
        raise NotImplementedError(self.set_symbolic_ref)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |b34ecfb678b193c035a2b01419b204a0#
    def _check_refname(self, name):
        """Ensure a refname is valid and lives in refs or is HEAD.

        HEAD is not a valid refname according to git-check-ref-format, but this
        class needs to be able to touch HEAD. Also, check_ref_format expects
        refnames without the leading 'refs/', but this class requires that
        so it cannot touch anything outside the refs dir (or HEAD).

        :param name: The name of the reference.
        :raises KeyError: if a refname is not HEAD or is otherwise not valid.
        """
        if name in (b'HEAD', b'refs/stash'):
            return
        if not name.startswith(b'refs/') or not check_ref_format(name[5:]):
            raise RefFormatError(name)

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |597e56d1d48da6690474a9699d3e2815#
    def import_refs(self, base, other):
        for name, value in other.items():
            self[b'/'.join((base, name))] = value
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |d052fec904a0ebc683760638c209e14e#
    def follow(self, name):
        """Follow a reference name.

        :return: a tuple of (refnames, sha), wheres refnames are the names of
            references in the chain
        """
        contents = SYMREF + name
        depth = 0
        refnames = []
        while contents.startswith(SYMREF):
            refname = contents[len(SYMREF):]
            refnames.append(refname)
            contents = self.read_ref(refname)
            if not contents:
                break
            depth += 1
            if depth > 5:
                raise KeyError(name)
        return refnames, contents

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |3cbf032af529decc9c5a50006c9450c5#
    def read_ref(self, refname):
        """Read a reference without following any references.

        :param refname: The name of the reference
        :return: The contents of the ref file, or None if it does
            not exist.
        """
        contents = self.read_loose_ref(refname)
        if not contents:
            contents = self.get_packed_refs().get(refname, None)
        return contents

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |4078fb6b8ff489133cf4ce1b452ca681#
    def read_loose_ref(self, name):
        """Read a loose reference and return its contents.

        :param name: the refname to read
        :return: The contents of the ref file, or None if it does
            not exist.
        """
        raise NotImplementedError(self.read_loose_ref)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |555ba187cdb2792dcc3b918f0fab539b#
    def allkeys(self):
        """All refs present in this container."""
        raise NotImplementedError(self.allkeys)

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |f37055994b125c137e36c4423f43bf40#
    def as_dict(self, base=None):
        """Return the contents of this container as a dictionary.

        """
        ret = {}
        keys = self.keys(base)
        if base is None:
            base = b''
        else:
            base = base.rstrip(b'/')
        for key in keys:
            try:
                ret[key] = self[(base + b'/' + key).strip(b'/')]
            except KeyError:
                continue  # Unable to resolve

        return ret
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |03bdce0d72dfd118407a06ecdb181b36#
    def add_if_new(self, name, ref):
        """Add a new reference only if it does not already exist."""
        raise NotImplementedError(self.add_if_new)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |64f5deff428a545a1052942a01ebc9ea#
    def keys(self, base=None):
        """Refs present in this container.

        :param base: An optional base to return refs under.
        :return: An unsorted set of valid refs in this container, including
            packed refs.
        """
        if base is not None:
            return self.subkeys(base)
        else:
            return self.allkeys()

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |774e7d5c1609ded87affdad9c2b35eac#
    def subkeys(self, base):
        """Refs present in this container under a base.

        :param base: The base to return refs under.
        :return: A set of valid refs in this container under the base; the base
            prefix is stripped from the ref names returned.
        """
        keys = set()
        base_len = len(base) + 1
        for refname in self.allkeys():
            if refname.startswith(base):
                keys.add(refname[base_len:])
        return keys

    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |f372f3961e8d39e0036f8650b968e842#
    def get_packed_refs(self):
        """Get contents of the packed-refs file.

        :return: Dictionary mapping ref names to SHA1s

        :note: Will return an empty dictionary when no packed-refs file is
            present.
        """
        raise NotImplementedError(self.get_packed_refs)
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |24dc564b127acef0466cd9c6868eb5b6#
    def get_peeled(self, name):
        """Return the cached peeled value of a ref, if available.

        :param name: Name of the ref to peel
        :return: The peeled value of the ref. If the ref is known not point to a
            tag, this will be the SHA the ref refers to. If the ref may point to
            a tag, but no cached information is available, None is returned.
        """
        return None
    # nw_e: [[RefsContainer]] methods #
    # nw_s: [[RefsContainer]] methods |8613c95915f588c5cc574a862cd6a2ff#
    def __contains__(self, refname):
        if self.read_ref(refname):
            return True
        return False

    # nw_e: [[RefsContainer]] methods #
# nw_e: class RefsContainer #

# nw_s: class DictRefsContainer |dad89e87c55ca89a3463ace7c1ffa77f#
class DictRefsContainer(RefsContainer):
    """RefsContainer backed by a simple dict.

    This container does not support symbolic or packed references and is not
    threadsafe.
    """

    def __init__(self, refs):
        self._refs = refs
        self._peeled = {}

    def allkeys(self):
        return self._refs.keys()

    def read_loose_ref(self, name):
        return self._refs.get(name, None)

    def get_packed_refs(self):
        return {}

    def set_symbolic_ref(self, name, other):
        self._refs[name] = SYMREF + other

    def set_if_equals(self, name, old_ref, new_ref):
        if old_ref is not None and self._refs.get(name, ZERO_SHA) != old_ref:
            return False
        realnames, _ = self.follow(name)
        for realname in realnames:
            self._check_refname(realname)
            self._refs[realname] = new_ref
        return True

    def add_if_new(self, name, ref):
        if name in self._refs:
            return False
        self._refs[name] = ref
        return True

    def remove_if_equals(self, name, old_ref):
        if old_ref is not None and self._refs.get(name, ZERO_SHA) != old_ref:
            return False
        try:
            del self._refs[name]
        except KeyError:
            pass
        return True

    def get_peeled(self, name):
        return self._peeled.get(name)

    def _update(self, refs):
        """Update multiple refs; intended only for testing."""
        # TODO(dborowitz): replace this with a public function that uses
        # set_if_equal.
        self._refs.update(refs)

    def _update_peeled(self, peeled):
        """Update cached peeled refs; intended only for testing."""
        self._peeled.update(peeled)
# nw_e: class DictRefsContainer #

# nw_s: class InfoRefsContainer |ea9e554ceb9e4d8d31999187bb51abf8#
class InfoRefsContainer(RefsContainer):
    """Refs container that reads refs from a info/refs file."""

    def __init__(self, f):
        self._refs = {}
        self._peeled = {}
        for l in f.readlines():
            sha, name = l.rstrip(b'\n').split(b'\t')
            if name.endswith(ANNOTATED_TAG_SUFFIX):
                name = name[:-3]
                if not check_ref_format(name):
                    raise ValueError("invalid ref name %r" % name)
                self._peeled[name] = sha
            else:
                if not check_ref_format(name):
                    raise ValueError("invalid ref name %r" % name)
                self._refs[name] = sha

    def allkeys(self):
        return self._refs.keys()

    def read_loose_ref(self, name):
        return self._refs.get(name, None)

    def get_packed_refs(self):
        return {}

    def get_peeled(self, name):
        try:
            return self._peeled[name]
        except KeyError:
            return self._refs[name]
# nw_e: class InfoRefsContainer #

# nw_s: class DiskRefsContainer |19f7b5ddcbc5317b164b02e6da4105a3#
class DiskRefsContainer(RefsContainer):
    """Refs container that reads refs from disk."""

    # nw_s: [[DiskRefsContainer]] methods |62c5e0a0373c43c60b11583c6e3a3f19#
    def __init__(self, path, worktree_path=None):
        self.path = path
        self.worktree_path = worktree_path or path
        # nw_s: [[DiskRefsContainer.__init__()]] set pack fields |1fb450b77fd5ff3f8d949de118771808#
        self._packed_refs = None
        self._peeled_refs = None
        # nw_e: [[DiskRefsContainer.__init__()]] set pack fields #
    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |258bcca2fa4db754b9f5fa25842b74f9#
    def set_symbolic_ref(self, name, other):
        """Make a ref point at another ref.

        :param name: Name of the ref to set
        :param other: Name of the ref to point at
        """
        self._check_refname(name)
        self._check_refname(other)
        filename = self.refpath(name)
        try:
            f = GitFile(filename, 'wb')
            try:
                f.write(SYMREF + other + b'\n')
            except (IOError, OSError):
                f.abort()
                raise
        finally:
            f.close()

    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |069ca35765e79087b65c55cc65e19ba5#
    def refpath(self, name):
        """Return the disk path of a ref.

        """
        if getattr(self.path, "encode", None) and getattr(name, "decode", None):
            name = name.decode(sys.getfilesystemencoding())
        if os.path.sep != "/":
            name = name.replace("/", os.path.sep)
        # TODO: as the 'HEAD' reference is working tree specific, it
        # should actually not be a part of RefsContainer
        if name == 'HEAD':
            return os.path.join(self.worktree_path, name)
        else:
            return os.path.join(self.path, name)

    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |ba03a47e60bfb75cb84c3120de34287d#
    def read_loose_ref(self, name):
        """Read a reference file and return its contents.

        If the reference file a symbolic reference, only read the first line of
        the file. Otherwise, only read the first 40 bytes.

        :param name: the refname to read, relative to refpath
        :return: The contents of the ref file, or None if the file does not
            exist.
        :raises IOError: if any other error occurs
        """
        filename = self.refpath(name)
        try:
            with GitFile(filename, 'rb') as f:
                header = f.read(len(SYMREF))
                if header == SYMREF:
                    # Read only the first line
                    return header + next(iter(f)).rstrip(b'\r\n')
                else:
                    # Read only the first 40 bytes
                    return header + f.read(40 - len(SYMREF))
        except IOError as e:
            if e.errno == errno.ENOENT:
                return None
            raise
    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |43cbc07d9d8367c2cfadca3142bd7922#
    def allkeys(self):
        allkeys = set()
        if os.path.exists(self.refpath(b'HEAD')):
            allkeys.add(b'HEAD')
        path = self.refpath(b'')
        for root, dirs, files in os.walk(self.refpath(b'refs')):
            dir = root[len(path):].strip(os.path.sep).replace(os.path.sep, "/")
            for filename in files:
                refname = ("%s/%s" % (dir, filename)).encode(sys.getfilesystemencoding())
                if check_ref_format(refname):
                    allkeys.add(refname)
        # nw_s: [[DiskRefsContainer.allkeys()]] look in packed refs |d3d1d7711f13cafe97d41fb30d10e0ca#
        allkeys.update(self.get_packed_refs())
        # nw_e: [[DiskRefsContainer.allkeys()]] look in packed refs #
        return allkeys

    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |b158863828dd464ae0ee39df89a2d344#
    def set_if_equals(self, name, old_ref, new_ref):
        """Set a refname to new_ref only if it currently equals old_ref.

        This method follows all symbolic references, and can be used to perform
        an atomic compare-and-swap operation.

        :param name: The refname to set.
        :param old_ref: The old sha the refname must refer to, or None to set
            unconditionally.
        :param new_ref: The new sha the refname will refer to.
        :return: True if the set was successful, False otherwise.
        """
        self._check_refname(name)
        try:
            realnames, _ = self.follow(name)
            realname = realnames[-1]
        except (KeyError, IndexError):
            realname = name
        filename = self.refpath(realname)
        ensure_dir_exists(os.path.dirname(filename))
        with GitFile(filename, 'wb') as f:
            if old_ref is not None:
                try:
                    # read again while holding the lock
                    orig_ref = self.read_loose_ref(realname)
                    if orig_ref is None:
                        orig_ref = self.get_packed_refs().get(realname, ZERO_SHA)
                    if orig_ref != old_ref:
                        f.abort()
                        return False
                except (OSError, IOError):
                    f.abort()
                    raise
            try:
                f.write(new_ref + b'\n')
            except (OSError, IOError):
                f.abort()
                raise
        return True
    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |84a759165be8e4dfed789384303d7910#
    def add_if_new(self, name, ref):
        """Add a new reference only if it does not already exist.

        This method follows symrefs, and only ensures that the last ref in the
        chain does not exist.

        :param name: The refname to set.
        :param ref: The new sha the refname will refer to.
        :return: True if the add was successful, False otherwise.
        """
        try:
            realnames, contents = self.follow(name)
            if contents is not None:
                return False
            realname = realnames[-1]
        except (KeyError, IndexError):
            realname = name
        self._check_refname(realname)
        filename = self.refpath(realname)
        ensure_dir_exists(os.path.dirname(filename))
        with GitFile(filename, 'wb') as f:
            if os.path.exists(filename) or name in self.get_packed_refs():
                f.abort()
                return False
            try:
                f.write(ref + b'\n')
            except (OSError, IOError):
                f.abort()
                raise
        return True

    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |d9a42897ba12c5166088226960d074a1#
    def remove_if_equals(self, name, old_ref):
        """Remove a refname only if it currently equals old_ref.

        This method does not follow symbolic references. It can be used to
        perform an atomic compare-and-delete operation.

        :param name: The refname to delete.
        :param old_ref: The old sha the refname must refer to, or None to delete
            unconditionally.
        :return: True if the delete was successful, False otherwise.
        """
        self._check_refname(name)
        filename = self.refpath(name)
        ensure_dir_exists(os.path.dirname(filename))
        f = GitFile(filename, 'wb')
        try:
            if old_ref is not None:
                orig_ref = self.read_loose_ref(name)
                if orig_ref is None:
                    orig_ref = self.get_packed_refs().get(name, ZERO_SHA)
                if orig_ref != old_ref:
                    return False
            # may only be packed
            try:
                os.remove(filename)
            except OSError as e:
                if e.errno != errno.ENOENT:
                    raise
            self._remove_packed_ref(name)
        finally:
            # never write, we just wanted the lock
            f.abort()
        return True
    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |bf159aeb21dee7611b0c5112b8d33bb3#
    def subkeys(self, base):
        subkeys = set()
        path = self.refpath(base)
        for root, dirs, files in os.walk(path):
            dir = root[len(path):].strip(os.path.sep).replace(os.path.sep, "/")
            for filename in files:
                refname = (("%s/%s" % (dir, filename))
                           .strip("/").encode(sys.getfilesystemencoding()))
                # check_ref_format requires at least one /, so we prepend the
                # base before calling it.
                if check_ref_format(base + b'/' + refname):
                    subkeys.add(refname)
        # nw_s: [[DiskRefsContainer.subkeys()]] look in packed refs |68c6a4f5942b6383b6a9a8746774dec1#
        for key in self.get_packed_refs():
            if key.startswith(base):
                subkeys.add(key[len(base):].strip(b'/'))
        # nw_e: [[DiskRefsContainer.subkeys()]] look in packed refs #
        return subkeys
    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |5151b7d4eda7e7b22ea29c554b0d0cb8#
    def get_packed_refs(self):
        """Get contents of the packed-refs file.

        :return: Dictionary mapping ref names to SHA1s

        :note: Will return an empty dictionary when no packed-refs file is
            present.
        """
        # TODO: invalidate the cache on repacking
        if self._packed_refs is None:
            # set both to empty because we want _peeled_refs to be
            # None if and only if _packed_refs is also None.
            self._packed_refs = {}
            self._peeled_refs = {}
            path = os.path.join(self.path, 'packed-refs')
            try:
                f = GitFile(path, 'rb')
            except IOError as e:
                if e.errno == errno.ENOENT:
                    return {}
                raise
            with f:
                first_line = next(iter(f)).rstrip()
                if (first_line.startswith(b'# pack-refs') and b' peeled' in
                        first_line):
                    for sha, name, peeled in read_packed_refs_with_peeled(f):
                        self._packed_refs[name] = sha
                        if peeled:
                            self._peeled_refs[name] = peeled
                else:
                    f.seek(0)
                    for sha, name in read_packed_refs(f):
                        self._packed_refs[name] = sha
        return self._packed_refs

    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |7dacce98add98658c6d532f75b3d86d7#
    def _remove_packed_ref(self, name):
        if self._packed_refs is None:
            return
        filename = os.path.join(self.path, 'packed-refs')
        # reread cached refs from disk, while holding the lock
        f = GitFile(filename, 'wb')
        try:
            self._packed_refs = None
            self.get_packed_refs()

            if name not in self._packed_refs:
                return

            del self._packed_refs[name]
            if name in self._peeled_refs:
                del self._peeled_refs[name]
            write_packed_refs(f, self._packed_refs, self._peeled_refs)
            f.close()
        finally:
            f.abort()
    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |af2dcafc48608d69c6af61570ff42099#
    def get_peeled(self, name):
        """Return the cached peeled value of a ref, if available.

        :param name: Name of the ref to peel
        :return: The peeled value of the ref. If the ref is known not point to a
            tag, this will be the SHA the ref refers to. If the ref may point to
            a tag, but no cached information is available, None is returned.
        """
        self.get_packed_refs()
        if self._peeled_refs is None or name not in self._packed_refs:
            # No cache: no peeled refs were read, or this ref is loose
            return None
        if name in self._peeled_refs:
            return self._peeled_refs[name]
        else:
            # Known not peelable
            return self[name]

    # nw_e: [[DiskRefsContainer]] methods #
    # nw_s: [[DiskRefsContainer]] methods |902a417959676b3de52c62da519f6a4b#
    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self.path)
    # nw_e: [[DiskRefsContainer]] methods #
# nw_e: class DiskRefsContainer #

# nw_s: function refs._split_ref_line |9905c4eabbbfe21a2c545fe24afad392#
def _split_ref_line(line):
    """Split a single ref line into a tuple of SHA1 and name."""
    fields = line.rstrip(b'\n\r').split(b' ')
    if len(fields) != 2:
        raise PackedRefsException("invalid ref line %r" % line)
    sha, name = fields
    if not valid_hexsha(sha):
        raise PackedRefsException("Invalid hex sha %r" % sha)
    if not check_ref_format(name):
        raise PackedRefsException("invalid ref name %r" % name)
    return (sha, name)
# nw_e: function refs._split_ref_line #

# nw_s: function refs.read_packed_refs |9e5e8eb6ceff44d77166433e5ebd4601#
def read_packed_refs(f):
    """Read a packed refs file.

    :param f: file-like object to read from
    :return: Iterator over tuples with SHA1s and ref names.
    """
    for l in f:
        if l.startswith(b'#'):
            # Comment
            continue
        if l.startswith(b'^'):
            raise PackedRefsException(
              "found peeled ref in packed-refs without peeled")
        yield _split_ref_line(l)
# nw_e: function refs.read_packed_refs #

# nw_s: function refs.read_packed_refs_with_peeled |88463a02433d219a1029842d4d45ed66#
def read_packed_refs_with_peeled(f):
    """Read a packed refs file including peeled refs.

    Assumes the "# pack-refs with: peeled" line was already read. Yields tuples
    with ref names, SHA1s, and peeled SHA1s (or None).

    :param f: file-like object to read from, seek'ed to the second line
    """
    last = None
    for l in f:
        if l[0] == b'#':
            continue
        l = l.rstrip(b'\r\n')
        if l.startswith(b'^'):
            if not last:
                raise PackedRefsException("unexpected peeled ref line")
            if not valid_hexsha(l[1:]):
                raise PackedRefsException("Invalid hex sha %r" % l[1:])
            sha, name = _split_ref_line(last)
            last = None
            yield (sha, name, l[1:])
        else:
            if last:
                sha, name = _split_ref_line(last)
                yield (sha, name, None)
            last = l
    if last:
        sha, name = _split_ref_line(last)
        yield (sha, name, None)
# nw_e: function refs.read_packed_refs_with_peeled #

# nw_s: function refs.write_packed_refs |59a4824a1d0eeecdd90cee80a212df9a#
def write_packed_refs(f, packed_refs, peeled_refs=None):
    """Write a packed refs file.

    :param f: empty file-like object to write to
    :param packed_refs: dict of refname to sha of packed refs to write
    :param peeled_refs: dict of refname to peeled value of sha
    """
    if peeled_refs is None:
        peeled_refs = {}
    else:
        f.write(b'# pack-refs with: peeled\n')
    for refname in sorted(packed_refs.keys()):
        f.write(git_line(packed_refs[refname], refname))
        if refname in peeled_refs:
            f.write(b'^' + peeled_refs[refname] + b'\n')
# nw_e: function refs.write_packed_refs #

# nw_s: function refs.read_info_refs |40957dfd2266d638331fb308d4d44501#
def read_info_refs(f):
    ret = {}
    for l in f.readlines():
        (sha, name) = l.rstrip(b"\r\n").split(b"\t", 1)
        ret[name] = sha
    return ret
# nw_e: function refs.read_info_refs #

# nw_s: function refs.write_info_refs |53afbae485185b96c7f52805e123d0cd#
def write_info_refs(refs, store):
    """Generate info refs."""
    for name, sha in sorted(refs.items()):
        # get_refs() includes HEAD as a special case, but we don't want to
        # advertise it
        if name == b'HEAD':
            continue
        try:
            o = store[sha]
        except KeyError:
            continue
        peeled = store.peel_sha(sha)
        yield o.id + b'\t' + name + b'\n'
        if o.id != peeled.id:
            yield peeled.id + b'\t' + name + ANNOTATED_TAG_SUFFIX + b'\n'
# nw_e: function refs.write_info_refs #

# nw_s: function is_local_branch |295b3cb5455b81c07c8899da936c57bf#
is_local_branch = lambda x: x.startswith(b'refs/heads/')
# nw_e: function is_local_branch #
# nw_e: refs.py #
