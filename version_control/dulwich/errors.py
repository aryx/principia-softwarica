# nw_s: dulwich/errors.py |efc2996b9dea94aff4796dd3c04f2ee2#
# errors.py -- errors for dulwich
# Copyright (C) 2007 James Westby <jw+debian@jameswestby.net>
# Copyright (C) 2009-2012 Jelmer Vernooij <jelmer@samba.org>
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
"""Dulwich-related exception classes and utility functions."""

import binascii

# nw_s: errors |375d7ba181759f89e27dea50c3582134#
class ChecksumMismatch(Exception):
    """A checksum didn't match the expected contents."""

    def __init__(self, expected, got, extra=None):
        if len(expected) == 20:
            expected = binascii.hexlify(expected)
        if len(got) == 20:
            got = binascii.hexlify(got)
        self.expected = expected
        self.got = got
        self.extra = extra
        if self.extra is None:
            Exception.__init__(self,
                "Checksum mismatch: Expected %s, got %s" % (expected, got))
        else:
            Exception.__init__(self,
                "Checksum mismatch: Expected %s, got %s; %s" %
                (expected, got, extra))
# nw_e: errors #
# nw_s: errors |dcc2dd90f173478fbeeee15eedfaa89e#
class MissingCommitError(Exception):
    """Indicates that a commit was not found in the repository"""

    def __init__(self, sha, *args, **kwargs):
        self.sha = sha
        Exception.__init__(self, "%s is not in the revision store" % sha)
# nw_e: errors #
# nw_s: errors |1dbb78adb08e7caa15b44c7323056f61#
class ObjectMissing(Exception):
    """Indicates that a requested object is missing."""

    def __init__(self, sha, *args, **kwargs):
        Exception.__init__(self, "%s is not in the pack" % sha)


# nw_e: errors #
# nw_s: errors |293334b8352731090316996673c5c3c6#
class ApplyDeltaError(Exception):
    """Indicates that applying a delta failed."""

    def __init__(self, *args, **kwargs):
        Exception.__init__(self, *args, **kwargs)


# nw_e: errors #
# nw_s: errors |a3b2124234a36691d94617099b4a7924#
class NotGitRepository(Exception):
    """Indicates that no Git repository was found."""

    def __init__(self, *args, **kwargs):
        Exception.__init__(self, *args, **kwargs)
# nw_e: errors #
# nw_s: errors |32d0d813f64f671b8856139f7215700c#
class NoIndexPresent(Exception):
    """No index is present."""
# nw_e: errors #
# nw_s: errors |cad5db1f9159d18d46349cf6d2c1aa7a#
class CommitError(Exception):
    """An error occurred while performing a commit."""
# nw_e: errors #
# nw_s: errors |68682d9f7b874b481ad4cafdd49fafbc#
class RefFormatError(Exception):
    """Indicates an invalid ref name."""
# nw_e: errors #
# nw_s: errors |d18cc9585de79cdb01bac40f638a9b3b#
class HookError(Exception):
    """An error occurred while executing a hook."""
# nw_e: errors #

# nw_s: exception WrongObjectException |4b5657aeaa5b94f801a11774ef74f345#
class WrongObjectException(Exception):
    """Baseclass for all the _ is not a _ exceptions on objects.

    Do not instantiate directly.

    Subclasses should define a type_name attribute that indicates what
    was expected if they were raised.
    """

    def __init__(self, sha, *args, **kwargs):
        Exception.__init__(self, "%s is not a %s" % (sha, self.type_name))
# nw_e: exception WrongObjectException #

# nw_s: [[WrongObjectException]] errors |5663a58ae56094d6f423e300b87c9ccc#
class NotCommitError(WrongObjectException):
    """Indicates that the sha requested does not point to a commit."""

    type_name = 'commit'
# nw_e: [[WrongObjectException]] errors #
# nw_s: [[WrongObjectException]] errors |c51c3e7620278653e39a127a25409613#
class NotTreeError(WrongObjectException):
    """Indicates that the sha requested does not point to a tree."""

    type_name = 'tree'
# nw_e: [[WrongObjectException]] errors #
# nw_s: [[WrongObjectException]] errors |79c45f8eaa0e36591699c7bab6ef0531#
class NotTagError(WrongObjectException):
    """Indicates that the sha requested does not point to a tag."""

    type_name = 'tag'
# nw_e: [[WrongObjectException]] errors #
# nw_s: [[WrongObjectException]] errors |a65c10fb0a6b05c86b46c4843905af8a#
class NotBlobError(WrongObjectException):
    """Indicates that the sha requested does not point to a blob."""

    type_name = 'blob'
# nw_e: [[WrongObjectException]] errors #

# nw_s: exception GitProtocolError |b4144cfee925500797578ecfc153971a#
class GitProtocolError(Exception):
    """Git protocol exception."""

    def __init__(self, *args, **kwargs):
        Exception.__init__(self, *args, **kwargs)
# nw_e: exception GitProtocolError #

# nw_s: [[GitProtocolError]] errors |25e73a1f678d8557402014228d98ac18#
class SendPackError(GitProtocolError):
    """An error occurred during send_pack."""

    def __init__(self, *args, **kwargs):
        Exception.__init__(self, *args, **kwargs)
# nw_e: [[GitProtocolError]] errors #
# nw_s: [[GitProtocolError]] errors |03314ff56bca292abd1abb436088eb46#
class UpdateRefsError(GitProtocolError):
    """The server reported errors updating refs."""

    def __init__(self, *args, **kwargs):
        self.ref_status = kwargs.pop('ref_status')
        Exception.__init__(self, *args, **kwargs)
# nw_e: [[GitProtocolError]] errors #
# nw_s: [[GitProtocolError]] errors |87e4de8a7346815f1bb3e578c624f3e6#
class HangupException(GitProtocolError):
    """Hangup exception."""

    def __init__(self):
        Exception.__init__(self,
            "The remote server unexpectedly closed the connection.")
# nw_e: [[GitProtocolError]] errors #
# nw_s: [[GitProtocolError]] errors |d55626a4413e1e91e189fa7df97ac520#
class UnexpectedCommandError(GitProtocolError):
    """Unexpected command received in a proto line."""

    def __init__(self, command):
        if command is None:
            command = 'flush-pkt'
        else:
            command = 'command %s' % command
        GitProtocolError.__init__(self, 'Protocol got unexpected %s' % command)
# nw_e: [[GitProtocolError]] errors #

# nw_s: exception FileFormatException |9a93cab824a8d783bcd029d25e23205a#
class FileFormatException(Exception):
    """Base class for exceptions relating to reading git file formats."""
# nw_e: exception FileFormatException #

# nw_s: [[FileFormatException]] errors |b65a5daae2412bd48226985dfdc49653#
class PackedRefsException(FileFormatException):
    """Indicates an error parsing a packed-refs file."""
# nw_e: [[FileFormatException]] errors #
# nw_s: [[FileFormatException]] errors |150fee15a41a36adcefa6ea68920deb5#
class ObjectFormatException(FileFormatException):
    """Indicates an error parsing an object."""
# nw_e: [[FileFormatException]] errors #
# nw_e: dulwich/errors.py #
