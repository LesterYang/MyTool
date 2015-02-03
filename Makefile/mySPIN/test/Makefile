#############################
# Makefile qsi-audio
# Author: Lester
#############################

VERSION    = 0.0
PROGRAM    = qsiMySpinTest

# cross-compile : set PREFIX and PATH
PREFIX     = /qsi_rootfs/rootfs_l
CC_PATH    = /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/
CROSS      = arm-linux-
DESTDIR    = ../output/

# Compiler
HOST       = $(CC_PATH)$(CROSS)
CC         = $(HOST)gcc
DEFINES    = -DQ_ASSERT -D_GNU_SOURCE -DQMYSPIN_VERSION='"$(VERSION)"'
CFLAGS     = -O2 -Wall -Werror -std=gnu99 -march=armv7-a -mfpu=neon $(DEFINES)
INCPATH    = -I../output/usr/include -I./include -I$(PREFIX)/usr/include -I$(PREFIX)/usr/local/include 
LINK       = $(HOST)gcc
LIBPATH    = -L$(PREFIX)/usr/lib -L$(PREFIX)/usr/local/lib -L$(PREFIX)/lib
RPATH      =
RPATH_LINK = $(PREFIX)/usr/lib
LFLAGS     = -rdynamic -Wl,-rpath-link=$(RPATH_LINK)
LIBS       = $(LIBPATH) -lmySPIN-Core -lpthread -lz -lqsiMySpin -lQSI-IPCLib
AR         = $(HOST)ar
OBJECTS    = src/qsiMySpinTest.o
    		 
# All Target
all: $(PROGRAM) install

$(PROGRAM): $(OBJECTS)
	@echo -e '\e[1;36mMake $@... \e[0m'
	$(CC) -o $@ $(OBJECTS) $(LFLAGS) $(LIBS)  

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INCPATH) $< -o $@
			
install:
	@test -d $(DESTDIR)/usr/bin || mkdir -p $(DESTDIR)/usr/bin
	@mv $(PROGRAM) $(DESTDIR)/usr/bin/
	@sync
	@echo -e '\e[1;33mOutput : /usr/bin/$(PROGRAM)\e[0m'
	
clean:
	@echo -e '\e[1;31mClean $(PROGRAM)...\e[0m'
	rm -f $(OBJECTS)


