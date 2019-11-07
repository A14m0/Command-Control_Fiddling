#!/bin/bash

if [ -d "ui" ]; then
    echo "Do not run this directly! Only do it through the Makefile provided!"
    exit 1
fi

echo "Updating UI files"

echo "" > ./python/design.py

for file in ./python/ui/*.ui; do
    pyuic4 $file >> ./python/design.py
done
