src_files = src\display.cpp src\emulate.cpp src\init.cpp src\main.cpp
libs = extern\SDL2\bin\SDL2.dll
incl = -Iinclude -Iextern\SDL2\include


build:
	cp $(libs) bin
	g++ -o bin\main.exe $(incl) $(src_files) $(libs)	
.PHONY: build

run: build
	.\bin\main.exe
.PHONY: run

debug:
	cp $(libs) bin
	g++ -o bin\main.exe $(incl) $(src_files) $(libs) -g
	gdb .\bin\main.exe
.PHONY: debug