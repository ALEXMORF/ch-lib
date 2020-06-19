@echo off

IF NOT EXIST .\build mkdir .\build
pushd .\build

ctime -begin tests.ctm
REM cl -nologo -Z7 -FC -WX -W4 ..\ch_buf_test.cpp /link -incremental:no
REM cl -nologo -Z7 -FC -WX -W4 ..\ch_math_test.cpp /link -incremental:no
cl -nologo -Z7 -FC -WX -W4 -wd4189 -wd4505 ..\ch_win32_test.cpp User32.lib Gdi32.lib
ctime -end tests.ctm

popd