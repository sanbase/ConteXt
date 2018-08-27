CC=g++ -O2 -DBSD -fhandle-exceptions -I../..
LIB= -L../.. -lcxform -lcxdb -lcxscr  -L/usr/X11R6/lib -lXpm -lX11

all:    Methods

Methods.o:        Methods.cpp
	${CC} -c $(CFLAGS) Methods.cpp

Methods:        Methods.o
	${CC} Methods.o $(LIB) -o Methods; strip Methods
