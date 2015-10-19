#!/bin/bash

echo "Copying header files from srclib to arduino project directories ..."

cd $(dirname $0)

for D in `find . -type d -name "[^.]*"`
do
  echo "  ... into to ${D:2}"
  cp ../srclib/*.h $D
done

echo "Done."