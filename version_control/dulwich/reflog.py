# nw_s: dulwich/reflog.py |6ef2e63c4e9a65b867835d576e3de5e7#
# reflog.py -- Parsing and writing reflog files
# Copyright (C) 2015 Jelmer Vernooij and others.
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
"""Utilities for reading and generating reflogs.
"""

import collections

from dulwich.objects import (
    format_timezone,
    parse_timezone,
    ZERO_SHA,
    )

# nw_s: type reflog.Entry |757f1befd1ea41a9e3dd51d9056c6705#
Entry = collections.namedtuple(
    'Entry', ['old_sha', 'new_sha', 'committer', 'timestamp', 'timezone',
              'message'])

# nw_e: type reflog.Entry #

# nw_s: function format_reflog_line |aa9399e57a8eda0c6a1f94702a9de76b#
def format_reflog_line(old_sha, new_sha, committer, timestamp, timezone,
                       message):
    """Generate a single reflog line.

    :param old_sha: Old Commit SHA
    :param new_sha: New Commit SHA
    :param committer: Committer name and e-mail
    :param timestamp: Timestamp
    :param timezone: Timezone
    :param message: Message
    """
    if old_sha is None:
        old_sha = ZERO_SHA
    return (old_sha + b' ' + new_sha + b' ' + committer + b' ' +
            str(timestamp).encode('ascii') + b' ' +
            format_timezone(timezone) + b'\t' + message)

# nw_e: function format_reflog_line #

# nw_s: function parse_reflog_line |d9a52b68da00fb083793d90aef7a7ed1#
def parse_reflog_line(line):
    """Parse a reflog line.

    :param line: Line to parse
    :return: Tuple of (old_sha, new_sha, committer, timestamp, timezone,
        message)
    """
    (begin, message) = line.split(b'\t', 1)
    (old_sha, new_sha, rest) = begin.split(b' ', 2)
    (committer, timestamp_str, timezone_str) = rest.rsplit(b' ', 2)
    return Entry(old_sha, new_sha, committer, int(timestamp_str),
                 parse_timezone(timezone_str)[0], message)
# nw_e: function parse_reflog_line #

# nw_s: function read_reflog |fb28f67cc1d39d33f24f33c42c87075d#
def read_reflog(f):
    """Read reflog.

    :param f: File-like object
    :returns: Iterator over Entry objects
    """
    for l in f:
        yield parse_reflog_line(l)
# nw_e: function read_reflog #
# nw_e: dulwich/reflog.py #
