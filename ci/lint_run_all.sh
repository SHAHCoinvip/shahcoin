#!/usr/bin/env bash
#
# Copyright (C) 2025 The SHAHCOIN Core Developers# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

set -o errexit; source ./ci/test/00_setup_env.sh
set -o errexit; source ./ci/lint/04_install.sh
set -o errexit
./ci/lint/06_script.sh
