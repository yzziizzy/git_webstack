#!/bin/bash

gcc _build.c -lutil -o ._build -ggdb \
	&& ./._build -d \
	&& valgrind  ./git_webstack $@
	
#--leak-check=full
