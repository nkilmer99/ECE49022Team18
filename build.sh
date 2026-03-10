#!/usr/bin/env bash

rm -rf build/*
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build --target main -j$(nproc)
