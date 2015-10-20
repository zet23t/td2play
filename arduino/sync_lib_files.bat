@echo off
echo Copying header files from srclib to arduino project directories ...

cd %~dp0

FOR /D %%G IN ("*") DO (
  echo    ... into %%G
  copy ..\srclib\lib_*.h %%G >NUL
)

echo Done.