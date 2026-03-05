#!/bin/bash

set -e

if [ ! -f "../generate_data.rb" ]; then
    echo "Error: generate_data.rb not found in parent directory"
    exit 1
fi

cd ..
ruby generate_data.rb $ROWS
cd rust
