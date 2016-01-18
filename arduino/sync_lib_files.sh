#!/bin/bash

echo "Copying header files from srclib to arduino project directories ..."

cd $(dirname $0)

for D in `find . -type d -name "[^.]*" -maxdepth 1`
do
  echo "  ... into ${D:2}"
  cp ../srclib/lib_*.* $D
done

echo "Done."