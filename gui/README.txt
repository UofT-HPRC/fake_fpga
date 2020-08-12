This Makefile depends on the GNU find command. I installed make and find
on my Windows computer via MinGW.

Brief instructions for installing make find: 
	1. Download the mingw-get package manager from 
	   https://osdn.net/projects/mingw/releases/. 
	2. From this package manager, install the mingw32-make and the 
	   msys-findutils "bin" packages.
	3. Edit your system's PATH variable to include "C:\MinGW\bin" and
	   "C:\MinGW\msys\1.0\bin". (These file locations may be slightly
	   different on your computer. You can double-check if they're right
	   by making sure the first one has "make.exe" in it and the second
	   one has "find.exe" in it).

The Makefile also invokes the java compiler and archiver. Here is the 
output of `java -version` on my computer:

	java version "14.0.2" 2020-07-14
	Java(TM) SE Runtime Environment (build 14.0.2+12-46)
	Java HotSpot(TM) 64-Bit Server VM (build 14.0.2+12-46, mixed mode, sharing)

