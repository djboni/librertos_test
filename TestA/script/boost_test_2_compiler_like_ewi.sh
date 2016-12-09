#!/bin/sh
# Convert Boost.Test error messages to compiler-like error messages.
# by Djones A. Boni

# The 'log level' parameter
# http://www.boost.org/doc/libs/1_34_1/libs/test/doc/components/utf/parameters/log_level.html
#
# Parameter Name:     The Unit Test Framework log level
#
# Environment variable name:  BOOST_TEST_LOG_LEVEL
#
# Command line argument name:     log_level
#
# Acceptable Values:
# all             - report all log messages including the passed test
#                   notification
# success         - the same as all
# test_suite      - show test suite messages
# message         - show user messages
# warning         - report warnings issued by user
# error           - report all error conditions
# cpp_exception   - report uncaught c++ exception
# system_error    - report system originated non-fatal errors (for example,
#                   timeout or floating point exception)
# fatal_error     - report only user or system originated fatal errors (for
#                   example, memory access violation)
# nothing         - does not report any information
#
# Description:    allows to set the Unit Test Framework log level in a range
#                 from a complete log, when all successful tests are confirmed
#                 and all test suite messages are included, to an empty log when
#                 nothing is logged a test output stream. Note that log levels
#                 are accumulating, in other words each log level includes also
#                 all the information reported by less restrictive ones.

# Boost.Test error message
#../tests/./LibreRTOS/lib/test_event.hpp(29): error in "should_init_if_valid_parameters": check xOSError == ((BaseType_t) -2) failed [0 != -2]
#../tests/LibreRTOS/lib/test_event.cpp(302): warning in "should_insert_task_in_event_list_if_valid": condition 0 == 1 is not satisfied [0 != 1]
#../tests/LibreRTOS/test_task.cpp(214): info: check uxSchedulerSuspended == 0U passed

# Compiler error message
#../tests/./LibreRTOS/lib/test_event.hpp:29:30: error: ‘pdPASSs’ was not declared in this scope

sed '
s/\(.*\)(\(.*\)): info: \(.*\)/\1:\2: info: \3/
s/\(.*\)(\(.*\)): warning in \(.*\)/\1:\2: warning: \3/
s/\(.*\)(\(.*\)): error in \(.*\)/\1:\2: error: \3/
s/\(.*\)(\(.*\)): fatal error in \(.*\)/\1:\2: fatal error: \3/
'
