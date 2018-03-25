#!/bin/sh
bindir=$(pwd)
cd /home/dika/Documents/IF3260-Grafika/grafika_opengl/tugas_minggu01/tugas1
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/dika/Documents/IF3260-Grafika/grafika_opengl/build/tugas_01 
	else
		"/home/dika/Documents/IF3260-Grafika/grafika_opengl/build/tugas_01"  
	fi
else
	"/home/dika/Documents/IF3260-Grafika/grafika_opengl/build/tugas_01"  
fi
