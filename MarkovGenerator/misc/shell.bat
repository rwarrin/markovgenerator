@echo off

subst R: /D
subst R: E:\Dev\Projects\MarkovGenerator

pushd R:

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
start D:\Development\Vim\vim80\gvim.exe
