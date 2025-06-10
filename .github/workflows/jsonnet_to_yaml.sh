#!/usr/bin/env bash

set -euo pipefail

# This script requires to have:
#  - jsonnet (any jsonnet should work, e.g., go-jsonnet)
#  - yq (a.k.a go-yq) from https://github.com/mikefarah/yq
#    Note that many distributions come with a default yq that is not
#    as powerful as https://github.com/mikefarah/yq so follow
#    the instructions there to install yq on your machine
#
# The sed command is because 'on' is printed with or without quotes depending
# on the version. It's a dirty hack that may break some input.

jsonnet "$@" \
| yq eval -P \
| sed -e 's/^\( *\)"on":/\1on:/' \
| sed -e 's/: "yes"$/: yes/'
