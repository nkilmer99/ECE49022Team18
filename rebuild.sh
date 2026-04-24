#!/usr/bin/env bash

cmake --build build --target main -j$(nproc)
