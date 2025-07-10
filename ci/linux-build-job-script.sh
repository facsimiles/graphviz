#!/usr/bin/env bash

if [ -z ${CI+x} ]; then
  echo "this script is only intended to run in CI" >&2
  exit 1
fi

set -e
set -o pipefail
set -u
set -x

logfile=$(mktemp)
ci/build.sh |& tee $logfile
echo "$CI_JOB_NAME-warnings `grep -c 'warning:' $logfile`" | tee metrics.txt
