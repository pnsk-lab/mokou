# $Id$

PREFIX = /usr/local
PLATFORM = generic
PWD = `pwd`
FLAGS = PLATFORM=$(PLATFORM) PWD=$(PWD) PREFIX=$(PREFIX)

.PHONY: all clean ./Mokou ./Control

all: ./Mokou ./Control

./Mokou::
	$(MAKE) -C $@ $(FLAGS)

./Control::
	$(MAKE) -C $@ $(FLAGS)

clean:
	$(MAKE) -C ./Mokou $(FLAGS) clean
	$(MAKE) -C ./Control $(FLAGS) clean
