#############################
# Makefile
# Author: Lester
#############################

# cross-compile : set PREFIX and HOST
PREFIX     = 
HOST       = 

# Compiler
CC         = $(HOST)gcc
DEFINES    =
CFLAGS     = -g3 -Wall $(DEFINES)
INCPATH    = -I$(PREFIX)/usr/include -I. -I./include
LINK       = $(HOST)gcc
LIBPATH    = -L$(PREFIX)/usr/lib -L$(PREFIX)/lib
RPATH      =
RPATH_LINK = $(PREFIX)/usr/lib                   # search library for cross-compile
LFLAGS     =                                     #-Wl,-rpath=$RPATH # -Wl,-rpath-link=$RPATH_LINK" 
LIBS       = $(LIBPATH) -lasound
AR         = $(HOST)ar
SED        = sed

OBJECTS    = qplay.o qplay_lib.o

# All Target
all: qplay

qplay: $(OBJECTS)
	@echo 'Building target: $@'
	$(CC) -o qplay $(OBJECTS) $(LFLAGS) $(LIBS)  

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(INCPATH) $< -o $@
	

	
clean:
	rm -f $(OBJECTS)
	


