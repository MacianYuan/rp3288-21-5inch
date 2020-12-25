@echo off

set base_dir=%~dp0  
%base_dir:~0,2%  

cd %base_dir%

md ..\dts\images

resource_tool --unpack  --image=..\Temp\Android\Image\resource.img ..\dts\

if %errorlevel% EQU 0 (
	echo Info:unpack resource.img success.
)else (
	echo Error:unpack resource.img error.
)

:exit

@echo on
