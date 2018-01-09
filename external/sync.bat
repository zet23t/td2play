copy %~dp0HxCModPlayer\hxcmod.c %~dp0..\srclib\lib_hxcmod.c
copy %~dp0HxCModPlayer\hxcmod.h %~dp0..\srclib\lib_hxcmod.h
lua %~dp0replace.lua hxcmod.h lib_hxcmod.h %~dp0..\srclib\lib_hxcmod.c