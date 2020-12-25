@echo off

set base_dir=%~dp0  
%base_dir:~0,2%  

cd %base_dir%

resource_tool ../dts/rk_kernel.img ../dts/logo.img ../dts/logo_kernel.img 

copy resource.img ../Temp/Android/Image/resource.img 

del resource.img 

if %errorlevel% EQU 0 (
	echo Info:dts to dtb success.
)else (
	echo Error:dts to dtb error.
)

:exit

@echo on
