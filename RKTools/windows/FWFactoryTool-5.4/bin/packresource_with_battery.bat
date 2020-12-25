@echo off

set base_dir=%~dp0  
%base_dir:~0,2%  

cd %base_dir%

copy ..\dts\rk-kernel.dtb 

copy ..\dts\logo.bmp

copy ..\dts\charge_anim_desc.txt

md   images\
copy ..\dts\images\ images\

resource_tool rk-kernel.dtb logo.bmp ^
	charge_anim_desc.txt images/battery_0.bmp images/battery_1.bmp ^
	images/battery_2.bmp images/battery_3.bmp images/battery_4.bmp ^
	images/battery_5.bmp images/battery_fail.bmp

if %errorlevel% EQU 0 (
	echo Info:dts to dtb success.
)else (
	echo Error:dts to dtb error.
)

del ..\Temp\Android\Image\resource.img

copy resource.img ..\Temp\Android\Image\resource.img 

del rk-kernel.dtb
del logo.bmp
del /s /q images\
rd images\
del charge_anim_desc.txt
 
del resource.img

:exit

@echo on
