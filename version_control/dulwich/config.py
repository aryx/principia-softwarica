# nw_s: dulwich/config.py |ff99d5c193771c127a729a8059be1e02#
# config.py - Reading and writing Git config files
# Copyright (C) 2011-2013 Jelmer Vernooij <jelmer@samba.org>
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
"""Reading and writing Git configuration files.

TODO:
 * preserve formatting when updating configuration files
 * treat subsection names as case-insensitive for [branch.foo] style
   subsections
"""

import errno
import os

from collections import (
    OrderedDict,
    MutableMapping,
    )

from dulwich.file import GitFile

# nw_s: class Config |188e719876b0fc9d6e490844d0243a49#
class Config(object):
    """A Git configuration."""

    # nw_s: [[Config]] methods |867b815edad10be4414adca44d613e8a#
    def get(self, section, name):
        """Retrieve the contents of a configuration setting.

        :param section: Tuple with section name and optional subsection namee
        :param subsection: Subsection name
        :return: Contents of the setting
        :raise KeyError: if the value is not set
        """
        raise NotImplementedError(self.get)
    # nw_e: [[Config]] methods #
    # nw_s: [[Config]] methods |d01e2d008ff1e38b11887d7b8014a8a9#
    def get_boolean(self, section, name, default=None):
        """Retrieve a configuration setting as boolean.

        :param section: Tuple with section name and optional subsection namee
        :param name: Name of the setting, including section and possible
            subsection.
        :return: Contents of the setting
        :raise KeyError: if the value is not set
        """
        try:
            value = self.get(section, name)
        except KeyError:
            return default
        if value.lower() == b"true":
            return True
        elif value.lower() == b"false":
            return False
        raise ValueError("not a valid boolean string: %r" % value)
    # nw_e: [[Config]] methods #
    # nw_s: [[Config]] methods |f11931e6017c5d6ac4c6d157e28d77fb#
    def set(self, section, name, value):
        """Set a configuration value.

        :param section: Tuple with section name and optional subsection namee
        :param name: Name of the configuration value, including section
            and optional subsection
        :param: Value of the setting
        """
        raise NotImplementedError(self.set)
    # nw_e: [[Config]] methods #
    # nw_s: [[Config]] methods |bf659da3bdd2c82dd0c3577057f77db9#
    def iteritems(self, section):
        """Iterate over the configuration pairs for a specific section.

        :param section: Tuple with section name and optional subsection namee
        :return: Iterator over (name, value) pairs
        """
        raise NotImplementedError(self.iteritems)
    # nw_e: [[Config]] methods #
    # nw_s: [[Config]] methods |b3a96e66d83f13dc0063a250432be4d9#
    def itersections(self):
        """Iterate over the sections.

        :return: Iterator over section tuples
        """
        raise NotImplementedError(self.itersections)
    # nw_e: [[Config]] methods #
    # nw_s: [[Config]] methods |b42122b2931f9fe824e5a5515b159f8d#
    def has_section(self, name):
        """Check if a specified section exists.

        :param name: Name of section to check for
        :return: boolean indicating whether the section exists
        """
        return (name in self.itersections())
    # nw_e: [[Config]] methods #
# nw_e: class Config #

# nw_s: class ConfigDict |197b3e7d14ee760c38f79b123a907b4d#
class ConfigDict(Config, MutableMapping):
    """Git configuration stored in a dictionary."""

    # nw_s: [[ConfigDict]] methods |e4a55df4f1f5ace3e2554ecf496db98e#
    def __init__(self, values=None):
        """Create a new ConfigDict."""
        if values is None:
            values = OrderedDict()
        self._values = values
    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |bbe822c03934a2e767d1db373e699f70#
    def __eq__(self, other):
        return (
            isinstance(other, self.__class__) and
            other._values == self._values)

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |aec6d9f7c35570bc65652b52843fc8e5#
    def __getitem__(self, key):
        return self._values.__getitem__(key)

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |318469838d58096fd1ffb1b7e5d23508#
    def __setitem__(self, key, value):
        return self._values.__setitem__(key, value)

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |3354ca5fe36f7f8bceb7b83d712ea779#
    def __delitem__(self, key):
        return self._values.__delitem__(key)

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |6c8f282a1d728ae55ddd51fb615a651c#
    def __iter__(self):
        return self._values.__iter__()

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |fdae1b67d68a4bfcf23dadfbf5432828#
    def __len__(self):
        return self._values.__len__()

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |f71a2459ff2d944673b9cf122164b0e0#
    @classmethod
    def _parse_setting(cls, name):
        parts = name.split(".")
        if len(parts) == 3:
            return (parts[0], parts[1], parts[2])
        else:
            return (parts[0], None, parts[1])

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |7a1d12e515c66e925ec3006a55b20a3f#
    def get(self, section, name):
        if not isinstance(section, tuple):
            section = (section, )
        if len(section) > 1:
            try:
                return self._values[section][name]
            except KeyError:
                pass
        return self._values[(section[0],)][name]

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |c1f68add77d56d8a085e34b148165b90#
    def set(self, section, name, value):
        if not isinstance(section, tuple):
            section = (section, )
        if not isinstance(name, bytes):
            raise TypeError(name)
        if type(value) not in (bool, bytes):
            raise TypeError(value)
        self._values.setdefault(section, OrderedDict())[name] = value

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |3dac3a0030cdd5f4935c427ce83af8f0#
    def iteritems(self, section):
        return self._values.get(section, OrderedDict()).items()

    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |a92c3caf7ab83ddf8c883aa6904ead1e#
    def itersections(self):
        return self._values.keys()
    # nw_e: [[ConfigDict]] methods #
    # nw_s: [[ConfigDict]] methods |ded3952917132477b48ff22543d61f15#
    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self._values)
    # nw_e: [[ConfigDict]] methods #
# nw_e: class ConfigDict #

# nw_s: function config._format_string |db75440e7010d4905989b462937548bf#
def _format_string(value):
    if (value.startswith(b" ") or
            value.startswith(b"\t") or
            value.endswith(b" ") or
            b'#' in value or
            value.endswith(b"\t")):
        return b'"' + _escape_value(value) + b'"'
    else:
        return _escape_value(value)
# nw_e: function config._format_string #

# nw_s: constant config._ESCAPE_TABLE |88622e2b064cf7ea30f83ee98a74e6d6#
_ESCAPE_TABLE = {
    ord(b"\\"): ord(b"\\"),
    ord(b"\""): ord(b"\""),
    ord(b"n"): ord(b"\n"),
    ord(b"t"): ord(b"\t"),
    ord(b"b"): ord(b"\b"),
    }
# nw_e: constant config._ESCAPE_TABLE #
# nw_s: constant config._COMMENT_CHARS |6adbf8581ac3543aa1075841af01d7c7#
_COMMENT_CHARS = [ord(b"#"), ord(b";")]
# nw_e: constant config._COMMENT_CHARS #
# nw_s: constant config._WHITESPACE_CHARS |21f9251b4987173ca26cfd8238c1f76a#
_WHITESPACE_CHARS = [ord(b"\t"), ord(b" ")]
# nw_e: constant config._WHITESPACE_CHARS #

# nw_s: function config._parse_string |02e184f12c5185713f7d07aa047dec0d#
def _parse_string(value):
    value = bytearray(value.strip())
    ret = bytearray()
    whitespace = bytearray()
    in_quotes = False
    i = 0
    while i < len(value):
        c = value[i]
        if c == ord(b"\\"):
            i += 1
            try:
                v = _ESCAPE_TABLE[value[i]]
            except IndexError:
                raise ValueError(
                    "escape character in %r at %d before end of string" %
                    (value, i))
            except KeyError:
                raise ValueError(
                    "escape character followed by unknown character "
                    "%s at %d in %r" % (value[i], i, value))
            if whitespace:
                ret.extend(whitespace)
                whitespace = bytearray()
            ret.append(v)
        elif c == ord(b"\""):
            in_quotes = (not in_quotes)
        elif c in _COMMENT_CHARS and not in_quotes:
            # the rest of the line is a comment
            break
        elif c in _WHITESPACE_CHARS:
            whitespace.append(c)
        else:
            if whitespace:
                ret.extend(whitespace)
                whitespace = bytearray()
            ret.append(c)
        i += 1

    if in_quotes:
        raise ValueError("missing end quote")

    return bytes(ret)
# nw_e: function config._parse_string #

# nw_s: function config._escape_value |cdfe220c72d52de6e78994afa71a7b44#
def _escape_value(value):
    """Escape a value."""
    value = value.replace(b"\\", b"\\\\")
    value = value.replace(b"\n", b"\\n")
    value = value.replace(b"\t", b"\\t")
    value = value.replace(b"\"", b"\\\"")
    return value
# nw_e: function config._escape_value #

# nw_s: function config._check_variable_name |17c1ac8581bc5b5077660b96506000aa#
def _check_variable_name(name):
    for i in range(len(name)):
        c = name[i:i+1]
        if not c.isalnum() and c != b'-':
            return False
    return True
# nw_e: function config._check_variable_name #

# nw_s: function config._check_section_name |212f1eec93e17a4076a7f6f6cdbd4664#
def _check_section_name(name):
    for i in range(len(name)):
        c = name[i:i+1]
        if not c.isalnum() and c not in (b'-', b'.'):
            return False
    return True
# nw_e: function config._check_section_name #

# nw_s: function config._strip_comments |2474126c70ff27063bf2edef9d75ff66#
def _strip_comments(line):
    line = line.split(b"#")[0]
    line = line.split(b";")[0]
    return line
# nw_e: function config._strip_comments #


# nw_s: class ConfigFile |a3152b98627d497ea5b0dd56c415943c#
class ConfigFile(ConfigDict):
    """A Git configuration file, like .git/config or ~/.gitconfig.
    """
    # nw_s: [[ConfigFile]] methods |db9bdb1a1950538869dbd479043d2212#
    @classmethod
    def from_file(cls, f):
        """Read configuration from a file-like object."""
        ret = cls()
        section = None
        setting = None
        for lineno, line in enumerate(f.readlines()):
            line = line.lstrip()
            if setting is None:
                # Parse section header ("[bla]")
                if len(line) > 0 and line[:1] == b"[":
                    line = _strip_comments(line).rstrip()
                    last = line.index(b"]")
                    if last == -1:
                        raise ValueError("expected trailing ]")
                    pts = line[1:last].split(b" ", 1)
                    line = line[last+1:]
                    pts[0] = pts[0].lower()
                    if len(pts) == 2:
                        if pts[1][:1] != b"\"" or pts[1][-1:] != b"\"":
                            raise ValueError(
                                "Invalid subsection %r" % pts[1])
                        else:
                            pts[1] = pts[1][1:-1]
                        if not _check_section_name(pts[0]):
                            raise ValueError("invalid section name %r" %
                                             pts[0])
                        section = (pts[0], pts[1])
                    else:
                        if not _check_section_name(pts[0]):
                            raise ValueError(
                                "invalid section name %r" % pts[0])
                        pts = pts[0].split(b".", 1)
                        if len(pts) == 2:
                            section = (pts[0], pts[1])
                        else:
                            section = (pts[0], )
                    ret._values[section] = OrderedDict()
                if _strip_comments(line).strip() == b"":
                    continue
                if section is None:
                    raise ValueError("setting %r without section" % line)
                try:
                    setting, value = line.split(b"=", 1)
                except ValueError:
                    setting = line
                    value = b"true"
                setting = setting.strip().lower()
                if not _check_variable_name(setting):
                    raise ValueError("invalid variable name %s" % setting)
                if value.endswith(b"\\\n"):
                    continuation = value[:-2]
                else:
                    continuation = None
                    value = _parse_string(value)
                    ret._values[section][setting] = value
                    setting = None
            else:  # continuation line
                if line.endswith(b"\\\n"):
                    continuation += line[:-2]
                else:
                    continuation += line
                    value = _parse_string(continuation)
                    ret._values[section][setting] = value
                    continuation = None
                    setting = None
        return ret
    # nw_e: [[ConfigFile]] methods #
    # nw_s: [[ConfigFile]] methods |e2372c9a6bf5c54f9ccb7ba72fcf26a2#
    @classmethod
    def from_path(cls, path):
        """Read configuration from a file on disk."""
        with GitFile(path, 'rb') as f:
            ret = cls.from_file(f)
            ret.path = path
            return ret
    # nw_e: [[ConfigFile]] methods #
    # nw_s: [[ConfigFile]] methods |5a017d2275ba464d20639f8357b01bef#
    def write_to_path(self, path=None):
        """Write configuration to a file on disk."""
        if path is None:
            path = self.path
        with GitFile(path, 'wb') as f:
            self.write_to_file(f)
    # nw_e: [[ConfigFile]] methods #
    # nw_s: [[ConfigFile]] methods |70e0756f20461aafeab7264fff9537a3#
    def write_to_file(self, f):
        """Write configuration to a file-like object."""
        for section, values in self._values.items():
            try:
                section_name, subsection_name = section
            except ValueError:
                (section_name, ) = section
                subsection_name = None
            if subsection_name is None:
                f.write(b"[" + section_name + b"]\n")
            else:
                f.write(b"[" + section_name +
                        b" \"" + subsection_name + b"\"]\n")
            for key, value in values.items():
                if value is True:
                    value = b"true"
                elif value is False:
                    value = b"false"
                else:
                    value = _format_string(value)
                f.write(b"\t" + key + b" = " + value + b"\n")
    # nw_e: [[ConfigFile]] methods #
# nw_e: class ConfigFile #

# nw_s: class StackedConfig |3393974269e165e64e54ab82fae85e6f#
class StackedConfig(Config):
    """Configuration which reads from multiple config files.."""

    # nw_s: [[StackedConfig]] methods |8d48ca1a50d9a83e99f0942b4fd2db92#
    def __init__(self, backends, writable=None):
        self.backends = backends
        self.writable = writable
    # nw_e: [[StackedConfig]] methods #
    # nw_s: [[StackedConfig]] methods |8799271260266be27d903b147ffc4e75#
    @classmethod
    def default_backends(cls):
        """Retrieve the default configuration.

        See git-config(1) for details on the files searched.
        """
        paths = []
        paths.append(os.path.expanduser("~/.gitconfig"))

        xdg_config_home = os.environ.get(
            "XDG_CONFIG_HOME", os.path.expanduser("~/.config/"),
        )
        paths.append(os.path.join(xdg_config_home, "git", "config"))

        if "GIT_CONFIG_NOSYSTEM" not in os.environ:
            paths.append("/etc/gitconfig")

        backends = []
        for path in paths:
            try:
                cf = ConfigFile.from_path(path)
            except (IOError, OSError) as e:
                if e.errno != errno.ENOENT:
                    raise
                else:
                    continue
            backends.append(cf)
        return backends

    # nw_e: [[StackedConfig]] methods #
    # nw_s: [[StackedConfig]] methods |e63220965f011112c8509e799c070d28#
    def get(self, section, name):
        for backend in self.backends:
            try:
                return backend.get(section, name)
            except KeyError:
                pass
        raise KeyError(name)

    # nw_e: [[StackedConfig]] methods #
    # nw_s: [[StackedConfig]] methods |3a6ffa6fb0eb2bdb31a5980001b05241#
    def set(self, section, name, value):
        if self.writable is None:
            raise NotImplementedError(self.set)
        return self.writable.set(section, name, value)
    # nw_e: [[StackedConfig]] methods #
    # nw_s: [[StackedConfig]] methods |3b59a762d928f9f2f478521c89de656e#
    def __repr__(self):
        return "<%s for %r>" % (self.__class__.__name__, self.backends)
    # nw_e: [[StackedConfig]] methods #
# nw_e: class StackedConfig #

# nw_s: function config.parse_submodules |fec74395f8b426afe37e1e671752a525#
def parse_submodules(config):
    """Parse a gitmodules GitConfig file, returning submodules.

   :param config: A `ConfigFile`
   :return: list of tuples (submodule path, url, name),
       where name is quoted part of the section's name.
    """
    for section in config.keys():
        section_kind, section_name = section
        if section_kind == b'submodule':
            sm_path = config.get(section, b'path')
            sm_url = config.get(section, b'url')
            yield (sm_path, sm_url, section_name)
# nw_e: function config.parse_submodules #

# nw_e: dulwich/config.py #
