#!/usr/bin/env bash

# ############################################################# #
#                                                               #
#  This is the reedb build script that compiles Reedb for       #
#  travis on Linux systems. Ubuntu sucks!                       #
#                                                               #
# ############################################################# #

set -e  # exit on error.
set -x  # echo commands.

# Preparation
if [ ! -f $PWD/my_cmake/bin/cmake ]; then
	wget --no-check-certificate https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.tar.gz -O cmake.tar.gz
	mkdir my_cmake && tar xf cmake.tar.gz -C my_cmake --strip 1
	alias cmake=$PWD/my_cmake/bin/cmake
fi

# Remove old build folders that might still exist
rm -rf $PWD/_build

# Now we need to define our own cmake command because travis sucks!
REE_CMAKE=$PWD/my_cmake/bin/cmake

# Set up g++ 4.8 because Ubuntu LTS is a piece of shit
if [ "$CXX" = "g++" ]; then export CXX="g++-4.8" CC="gcc-4.8"; fi

# I hate you travis! :(
# CFLAGS="-fsanitize=undefined"
mkdir $PWD/_build && cd $PWD/_build
rm -rf ./CMakeCache.txt # To avoid cache errors
$REE_CMAKE ..
$REE_CMAKE --build .


# Clean up
cd $PWD/.. && rm -rf $PWD/_build
