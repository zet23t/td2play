@echo off
echo Copying header files from srclib to arduino project directories ...

cd %~dp0

FOR /D %%G IN ("*") DO (
  echo    ... into %%G
  copy ..\srclib\lib_*.* %%G >NUL
)

echo Done.