#!/bin/bash

set -e

ROWS="${1:-100000000}"

echo "Generating $ROWS rows of test data..."

if [ ! -f "../generate_data.rb" ]; then
    echo "Error: generate_data.rb not found in parent directory"
    exit 1
fi

cd ..
ruby generate_data.rb $ROWS
cd python
