# [LibreRTOS Test](https://github.com/djboni/librertos_test)

This repository contains [LibreRTOS](https://github.com/djboni/librertos) unit-tests, using C++ Boost.Test library.

# Cloning the repository

```sh
git clone --recurse-submodules git@github.com:djboni/librertos_test
```

# Compiling and running the tests

To compile enter the build directory, call make and run the executable.

```sh
cd Build
make
./librertos_test
```

# Show code coverage information

To see code coverage information enter the build directory, run the executable, and use the gcovr tool:

```sh
cd Build
./librertos_test
gcovr --root=.
```

If you wish to see branch coverage, instead of line coverage, use the corresponding option:

```sh
gcovr --root=. --branches
```

# Scripts

In the scripts directory there are three scripts:

* One converts the output of the test to compiler-like error messages;
* Another converts the output of gcovr to compiler-like error messages; and
* The third is a helper that calls both above.

Running the helper script as post-build allows you to easiliy find the lines where there are failures in tests or in coverage.

An example of post-build command is shown below:

```sh
# BASH    HelperScriptPath        ExecutablePath
/bin/bash ../scripts/run_tests.sh ./librertos_test
```

# Prerequisites 

Ubuntu 18.04.

```sh
sudo apt install libboost-test-dev gcovr
```

