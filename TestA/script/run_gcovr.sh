#!/bin/sh

# Test suite executable
Executable="$1"; shift

# Run executable
./${Executable}
x=$?

# Wait stdout and stderr to flush
sync

# Run gcovr if test was successful
if [ $x = 0 ]; then

    # Uncovered lines
    echo; echo
    gcovr --root=.

    # Uncovered branches
    echo; echo
    gcovr --root=. --branches --delete
fi
