@echo off

set base_dir=%~dp0  
%base_dir:~0,2%  

cd %base_dir%

dtc -O dtb -b 0 -o ..\dts\rk-kernel.dtb -I dts ..\dts\Resource.dts

if %errorlevel% EQU 0 (
	echo Info:dts to dtb success.
)else (
	echo Error:dts to dtb error.
)

:exit

@echo on
