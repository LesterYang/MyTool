#!/bin/bash

HOST=$1
WORKSPACE="/mnt/hgfs/Win_7/workspace-cpp2/liblist"
 
echo "Build on ${HOST:=x86}"

if [ "$HOST" == "arm" ];then

    PREFIX="/Space/ltib2/ltib/rootfs"
    BINDIR="/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin"
    LIBDIR="$PREFIX/usr/lib"
    LIBDIR2="$WORKSPACE/MX51_release"
    INCDIR="$PREFIX/usr/include"
    INCDIR2="$WORKSPACE/include"
    RPATH=""
    RPATH_LINK="$PREFIX/usr/lib" 

    CC="arm-linux-gcc"    
    CFLAGS="-Wall -O2 -std=gnu99 -march=armv7-a -mfpu=neon"
    LIBS="-llist -pthread -lncurses -lreadline"
    LDFLAGS="-L$LIBDIR -L$LIBDIR2 -I$INCDIR -I$INCDIR2 -Wl,-rpath-link=$RPATH_LINK"      

else

    PREFIX=""
    BINDIR="/usb/bin"
    LIBDIR="$WORKSPACE/x86"
    INCDIR="$WORKSPACE/include"
    RPATH="$WORKSPACE/x86"
    RPATH_LINK=""

    CC="gcc"
    CFLAGS="-Wall -O2 -g3 -std=gnu99"
    LIBS="-llist -pthread -lncurses -lreadline"
    LDFLAGS="-L$LIBDIR -I$INCDIR -Wl,-rpath=$RPATH"

fi

export PATH=$BINDIR:$PATH

: << 'END'
echo "$CC list_interactive.c -o list $CFLAGS $LIBS $LDFLAGS"
echo "$CC list_interactive.c -o list $CFLAGS $LIBS $LDFLAGS"
END

$CC list_interactive.c -o list $CFLAGS $LIBS $LDFLAGS || exit 1
$CC list.c -o list_test $CFLAGS $LIBS $LDFLAGS || exit 1

echo "Done. Copy files to Window7 and sync"
cp list list_test /mnt/hgfs/Win_7/
sync
exit 0

