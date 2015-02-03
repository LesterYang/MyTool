#############################
# Makefile
# Author: Lester
#############################

MAJOR_VER  = 0
VERSION    = $(MAJOR_VER).0.0
PROGRAM    = libqsiMySpin

# cross-compile : set PREFIX and PATH
PREFIX     = /qsi_rootfs/rootfs_l
CC_PATH    = /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/
CROSS      = arm-linux-
DESTDIR    = ../output/

# Compiler
HOST       = $(CC_PATH)$(CROSS)
CC         = $(HOST)gcc
DEFINES    = -D_GNU_SOURCE -DQSIMYSPIN_VER='"$(VERSION)"'
CFLAGS     = -O2 -Wall -Werror -std=gnu99 -fPIC -march=armv7-a -mfpu=neon $(DEFINES)
INCPATH    = -I./include -I$(PREFIX)/usr/include -I$(PREFIX)/usr/local/include
LINK       = $(HOST)gcc
LIBPATH    = -L$(PREFIX)/usr/lib -L$(PREFIX)/usr/local/lib -L$(PREFIX)/lib
RPATH      =
RPATH_LINK = $(PREFIX)/usr/lib
LFLAGS     = -rdynamic -shared -Wl,-rpath-link=$(RPATH_LINK),-soname=$(PROGRAM).so.$(MAJOR_VER)
LIBS       = $(LIBPATH) -lmySPIN-Core -lpthread -lz -lasound -lts
AR         = $(HOST)ar


OBJECTS    =  $(shell ls ./src/adapter/*.c      | sed 's/\.c/.o/g') \
	          $(shell ls ./src/connectivity/*.c | sed 's/\.c/.o/g') \
              $(shell ls ./src/IVI/*.c     | sed 's/\.c/.o/g')
		
# All Target
all: $(PROGRAM).so.$(VERSION) install

$(PROGRAM).so.$(VERSION): $(OBJECTS)
	@echo -e '\e[1;36mMake $@... \e[0m'
	$(CC) -o $@ $(OBJECTS) $(LFLAGS) $(LIBS)  

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INCPATH) $< -o $@

install:
	@test -d $(DESTDIR)/usr/lib || mkdir -p $(DESTDIR)/usr/lib
	@test -d $(DESTDIR)/usr/include || mkdir -p $(DESTDIR)/usr/include
	@mv $(PROGRAM).so.$(VERSION) $(DESTDIR)/usr/lib/
	@cp include/qsiAdapter.h $(DESTDIR)/usr/include/
	@sync
	@echo -e '\e[1;33mOutput : $(DESTDIR)/usr/lib/$(PROGRAM).so.$(VERSION)\e[0m'

clean:
	@echo -e '\e[1;31mClean $(PROGRAM)...\e[0m'
	rm -f $(OBJECTS)
