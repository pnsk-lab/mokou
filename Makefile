# $Id$

PREFIX = /usr/local
PLATFORM = generic
PWD = `pwd`
FLAGS = PLATFORM=$(PLATFORM) PWD=$(PWD) PREFIX=$(PREFIX)

.PHONY: all clean ./Mokou ./Control

all: ./Mokou ./Control ./Manpage/mokouctl.8

./Mokou::
	$(MAKE) -C $@ $(FLAGS)

./Control::
	$(MAKE) -C $@ $(FLAGS)

./Manpage/mokouctl.8: ./Manpage/mokouctl.8.tmp
	sed "s%@PREFIX@%$(PREFIX)%g" ./Manpage/mokouctl.8.tmp > $@

clean:
	$(MAKE) -C ./Mokou $(FLAGS) clean
	$(MAKE) -C ./Control $(FLAGS) clean
	rm -f ./Manpage/mokouctl.8
