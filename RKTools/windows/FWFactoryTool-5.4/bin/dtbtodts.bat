@echo off

set base_dir=%~dp0  
%base_dir:~0,2%  

cd %base_dir%

if  not exist dtc.exe (
echo not exit dtc.exe
)
dtc -O dts -o ..\dts\Resource.dts -I dtb ..\dts\rk-kernel.dtb

if %errorlevel% EQU 0 (
	echo Info:dtb to dts success.
) else (
	echo Error:dtb to dts error.
)

:exit

@echo on
