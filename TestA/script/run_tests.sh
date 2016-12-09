#!/bin/sh

# Test suite executable
Executable="$1"; shift

# Test suite timeout
Timeout=0.5

set -o pipefail


# Run executable
echo; echo
timeout ${Timeout} \
        ./${Executable} --log_level=warning 2>&1 |
        sh ../script/boost_test_2_compiler_like_ewi.sh
ExecutableSuccess=$?

case $ExecutableSuccess in
0)      ;;
124)    echo "*** TIMEOUT"
        ;;
*)      ;;
esac

# Wait stdout and stderr to flush
sync



# Run gcovr if test was successful
if [ $ExecutableSuccess = 0 ]; then

    # Uncovered branches (IDE hilight)
    Level=1 # 0 (err), 1 (err+warn), 2 (err+warn+info)
    echo; echo
    python ../script/gcovr.py . --xml --root=`(cd ../librertos; pwd)` |
            python ../script/gcovr_xml_2_compiler_like_ewi.py $Level ../librertos

    # Uncovered statements (human friendly)
    echo; echo
    python ../script/gcovr.py . --root=`(cd ../librertos; pwd)`

    # Uncovered branches (human friendly)
    echo; echo
    python ../script/gcovr.py . --root=`(cd ../librertos; pwd)` --branch --delete
fi
