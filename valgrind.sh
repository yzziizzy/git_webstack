#!/bin/bash

gcc _build.c -lutil -o ._build -ggdb \
	&& ./._build -d \
	&& valgrind  --leak-check=full  --show-leak-kinds=all ./git_webstack $@
	
#--leak-check=full
