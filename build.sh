#!/bin/bash
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
  rm -rf "$BUILD_DIR"/*
else
  mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"
cmake ..
make -j5