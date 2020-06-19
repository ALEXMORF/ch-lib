@echo off

IF NOT EXIST .\build mkdir .\build
pushd .\build

ctime -begin tests.ctm
REM cl -nologo -Z7 -FC -WX -W4 ..\ch_buf_test.cpp /link -incremental:no
REM cl -nologo -Z7 -FC -WX -W4 ..\ch_math_test.cpp /link -incremental:no
REM cl -nologo -Z7 -FC -WX -W4 -wd4189 -wd4505 ..\ch_win32_test.cpp User32.lib Gdi32.lib
cl -nologo -Z7 -FC -WX -W4 -wd4189 -wd4505 -wd4100 ..\ch_d3d12_test.cpp /link -incremental:no User32.lib Gdi32.lib d3d12.lib dxgi.lib d3dcompiler.lib
ctime -end tests.ctm

popd