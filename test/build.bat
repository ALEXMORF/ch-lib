@echo off

IF NOT EXIST .\build mkdir .\build
pushd .\build

cl -nologo -Z7 -FC ..\ch_buf_test.cpp
cl -nologo -Z7 -FC ..\ch_math_test.cpp

popd