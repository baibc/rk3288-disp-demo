#!/bin/sh

cp ../../../../out/target/product/rk3288/system/bin/disp  .
adb push disp /data/hello
adb shell
