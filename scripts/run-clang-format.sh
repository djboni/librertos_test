#!/bin/sh

find -name '*.[cChH]'         -exec clang-format -i '{}' ';'
find -name '*.[cChH][pP][pP]' -exec clang-format -i '{}' ';'
