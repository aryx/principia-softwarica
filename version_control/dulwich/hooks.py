# nw_s: dulwich/hooks.py |a1eed196309df2f18a61b691deefae46#
# hooks.py -- for dealing with git hooks
# Copyright (C) 2012-2013 Jelmer Vernooij and others.
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
"""Access to hooks."""

import os
import subprocess
import sys
import tempfile

from dulwich.errors import (
    HookError,
)

# nw_s: class Hook |357c2de4e8eed2bddd08959a379a3f6d#
class Hook(object):
    """Generic hook object."""

    # nw_s: [[Hook]] methods |c81baed2f5600f006f1321d8f31a9ed8#
    def execute(self, *args):
        """Execute the hook with the given args

        :param args: argument list to hook
        :raise HookError: hook execution failure
        :return: a hook may return a useful value
        """
        raise NotImplementedError(self.execute)
    # nw_e: [[Hook]] methods #
# nw_e: class Hook #

# nw_s: class ShellHook |65b0237ac3762a588ff5b553120ed042#
class ShellHook(Hook):
    """Hook by executable file

    Implements standard githooks(5) [0]:

    [0] http://www.kernel.org/pub/software/scm/git/docs/githooks.html
    """

    # nw_s: [[ShellHook]] methods |4ae7a0b164b5edf299019316f9050fa3#
    def __init__(self, name, path, numparam,
                 pre_exec_callback=None, post_exec_callback=None):
        """Setup shell hook definition

        :param name: name of hook for error messages
        :param path: absolute path to executable file
        :param numparam: number of requirements parameters
        :param pre_exec_callback: closure for setup before execution
            Defaults to None. Takes in the variable argument list from the
            execute functions and returns a modified argument list for the
            shell hook.
        :param post_exec_callback: closure for cleanup after execution
            Defaults to None. Takes in a boolean for hook success and the
            modified argument list and returns the final hook return value
            if applicable
        """
        self.name = name
        self.filepath = path
        self.numparam = numparam

        self.pre_exec_callback = pre_exec_callback
        self.post_exec_callback = post_exec_callback
    # nw_e: [[ShellHook]] methods #
    # nw_s: [[ShellHook]] methods |f654e4808285d03c4362d3f8c1156839#
    def execute(self, *args):
        """Execute the hook with given args"""

        if len(args) != self.numparam:
            raise HookError("Hook %s executed with wrong number of args. \
                            Expected %d. Saw %d. args: %s"
                            % (self.name, self.numparam, len(args), args))

        if (self.pre_exec_callback is not None):
            args = self.pre_exec_callback(*args)

        try:
            ret = subprocess.call([self.filepath] + list(args))
            if ret != 0:
                if (self.post_exec_callback is not None):
                    self.post_exec_callback(0, *args)
                raise HookError("Hook %s exited with non-zero status"
                                % (self.name))
            if (self.post_exec_callback is not None):
                return self.post_exec_callback(1, *args)
        except OSError:  # no file. silent failure.
            if (self.post_exec_callback is not None):
                self.post_exec_callback(0, *args)
    # nw_e: [[ShellHook]] methods #
# nw_e: class ShellHook #


# nw_s: class PreCommitShellHook |504858d1215451d2433b0d64f8eb191a#
class PreCommitShellHook(ShellHook):
    """pre-commit shell hook"""

    def __init__(self, controldir):
        filepath = os.path.join(controldir, 'hooks', 'pre-commit')

        ShellHook.__init__(self, 'pre-commit', filepath, 0)
# nw_e: class PreCommitShellHook #

# nw_s: class PostCommitShellHook |be6b76485d480725f60f177d43a2429f#
class PostCommitShellHook(ShellHook):
    """post-commit shell hook"""

    def __init__(self, controldir):
        filepath = os.path.join(controldir, 'hooks', 'post-commit')

        ShellHook.__init__(self, 'post-commit', filepath, 0)
# nw_e: class PostCommitShellHook #

# nw_s: class CommitMsgShellHook |97d5c27ed849a4e01deb8ec31d4931a4#
class CommitMsgShellHook(ShellHook):
    """commit-msg shell hook

    :param args[0]: commit message
    :return: new commit message or None
    """

    def __init__(self, controldir):
        filepath = os.path.join(controldir, 'hooks', 'commit-msg')

        def prepare_msg(*args):
            (fd, path) = tempfile.mkstemp()

            with os.fdopen(fd, 'wb') as f:
                f.write(args[0])

            return (path,)

        def clean_msg(success, *args):
            if success:
                with open(args[0], 'rb') as f:
                    new_msg = f.read()
                os.unlink(args[0])
                return new_msg
            os.unlink(args[0])

        ShellHook.__init__(self, 'commit-msg', filepath, 1,
                           prepare_msg, clean_msg)
# nw_e: class CommitMsgShellHook #
# nw_e: dulwich/hooks.py #
