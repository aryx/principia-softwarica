# nw_s: dulwich/archive.py |c93f3d1ae09340b65d39847582f05299#
# archive.py -- Creating an archive from a tarball
# Copyright (C) 2015 Jonas Haag <jonas@lophus.org>
# Copyright (C) 2015 Jelmer Vernooij <jelmer@jelmer.uk>
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
"""Generates tarballs for Git trees.

"""

import posixpath
import stat
import tarfile
from io import BytesIO
from contextlib import closing

# nw_s: class ChunkedBytesIO |5a186d80383b7cda61fb29828342db3b#
class ChunkedBytesIO(object):
    """Turn a list of bytestrings into a file-like object.

    This is similar to creating a `BytesIO` from a concatenation of the
    bytestring list, but saves memory by NOT creating one giant bytestring first::

        BytesIO(b''.join(list_of_bytestrings)) =~= ChunkedBytesIO(list_of_bytestrings)
    """
    def __init__(self, contents):
        self.contents = contents
        self.pos = (0, 0)

    def read(self, maxbytes=None):
        if maxbytes < 0:
            maxbytes = float('inf')

        buf = []
        chunk, cursor = self.pos

        while chunk < len(self.contents):
            if maxbytes < len(self.contents[chunk]) - cursor:
                buf.append(self.contents[chunk][cursor:cursor+maxbytes])
                cursor += maxbytes
                self.pos = (chunk, cursor)
                break
            else:
                buf.append(self.contents[chunk][cursor:])
                maxbytes -= len(self.contents[chunk]) - cursor
                chunk += 1
                cursor = 0
                self.pos = (chunk, cursor)
        return b''.join(buf)
# nw_e: class ChunkedBytesIO #

# nw_s: function archive.tar_stream |96c5269133ea45f5f79f304e37519081#
def tar_stream(store, tree, mtime, format=''):
    """Generate a tar stream for the contents of a Git tree.

    Returns a generator that lazily assembles a .tar.gz archive, yielding it in
    pieces (bytestrings). To obtain the complete .tar.gz binary file, simply
    concatenate these chunks.

    :param store: Object store to retrieve objects from
    :param tree: Tree object for the tree root
    :param mtime: UNIX timestamp that is assigned as the modification time for
        all files
    :param format: Optional compression format for tarball
    :return: Bytestrings
    """
    buf = BytesIO()
    with closing(tarfile.open(None, "w:%s" % format, buf)) as tar:
        for entry_abspath, entry in _walk_tree(store, tree):
            try:
                blob = store[entry.sha]
            except KeyError:
                # Entry probably refers to a submodule, which we don't yet support.
                continue
            data = ChunkedBytesIO(blob.chunked)

            info = tarfile.TarInfo()
            info.name = entry_abspath.decode('ascii') # tarfile only works with ascii.
            info.size = blob.raw_length()
            info.mode = entry.mode
            info.mtime = mtime

            tar.addfile(info, data)
            yield buf.getvalue()
            buf.truncate(0)
            buf.seek(0)
    yield buf.getvalue()
# nw_e: function archive.tar_stream #

# nw_s: function archive._walk_tree |5d73370d48087fcff09746c96f388a95#
def _walk_tree(store, tree, root=b''):
    """Recursively walk a dulwich Tree, yielding tuples of
    (absolute path, TreeEntry) along the way.
    """
    for entry in tree.iteritems():
        entry_abspath = posixpath.join(root, entry.path)
        if stat.S_ISDIR(entry.mode):
            for _ in _walk_tree(store, store[entry.sha], entry_abspath):
                yield _
        else:
            yield (entry_abspath, entry)
# nw_e: function archive._walk_tree #
# nw_e: dulwich/archive.py #
