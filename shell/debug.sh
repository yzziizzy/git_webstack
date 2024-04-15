#!/bin/bash

gcc _build.c -lutil -o ._build -ggdb \
	&& ./._build -d
	
if [ $? -ne 0 ]; then
  exit
fi
	

old=`pwd`

cd testrepo
date >> increment
git add increment
git commit -m "test"

git push

cd $old
