#!/bin/bash

set -e

ROWS="${1:-100000000}"

echo "Generating $ROWS rows of test data..."

ruby generate_data.rb $ROWS
