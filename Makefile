# 
WWWDIR=/var/www/docs
DST=/usr/local/bin
VER=0

#CC=g++ -O2 -I. -I/usr/local/include -DVER=$(VER)
CC=g++ -ggdb -I. -I/usr/local/include -DVER=$(VER)

#.if exists (/usr/X11R6/include/X11/Xlib.h)
#CC+=-DX11R6
#XLIB=-L/usr/X11R6/lib -lX11
#.endif

OBJ=    main.o context.o action.o query.o cmd.o edit.o chart.o menu.o write.o pipe.o index.o remote.o

all:    update cx6 /usr/local/etc/CX_Messages/Main /usr/local/etc/CX_Menu/Main /usr/local/etc/CX_Icons/Main /usr/local/etc/CX_Help/Main /usr/local/etc/Methods.cc

install: all
	./mkdir_html $(WWWDIR);
	sh ./users-groups.sh
	mkdir -p /usr/local/include/DB_CLASS
	mkdir -p /usr/local/include/SCREEN
	mkdir -p /var/log/httpd
	cd Utils; make -f Makefile.freeBSD install
	cd UnixSpace; make install
	cd SCREEN; make -f Makefile.freeBSD install
	cd SERVER; make -f Makefile.freeBSD install
	cp DB_CLASS/CX_BASE.h DB_CLASS/CX_common.h DB_CLASS/ConteXt.h DB_CLASS/StdAfx.h DB_CLASS/Ttree.h DB_CLASS/context_dll.h DB_CLASS/gzip.h DB_CLASS/ram_base.h DB_CLASS/tailor.h /usr/local/include/DB_CLASS/
	cp CX_Browser.h CX_Methods.h CX_pipe.h CX_utils.h /usr/local/include/
	cp SCREEN/screen.h /usr/local/include/SCREEN/
	cp libcxdb.a libcxform.a libcxscr.a /usr/local/lib
	mkdir -p /usr/local/man/cat1
	cp man-cx6.gz /usr/local/man/cat1/cx6.$(VER).gz
	cp -f cx6 $(DST)/cx6.$(VER)
	cp -f cx6 $(DST)/cx6

clean:
	find . -name "*.o" -exec rm -f {} \;
	find . -name "*.b" -exec rm -f {} \;
	find . -name ".*" -not -name ".git" -exec rm -f {} \;
	rm -f cx6 update
	rm -f libcxdb.a libcxform.a libcxscr.a
	cd Utils; make -f Makefile.freeBSD clean
	cd SCREEN; make -f Makefile.freeBSD clean
	cd SERVER; make -f Makefile.freeBSD clean
	cd UnixSpace; make clean

cx6:    ./update $(OBJ) DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h libcxdb.a libcxform.a libcxscr.a
	g++ -O2 $(OBJ) -L. -lcrypt -lcxform -lcxdb -lcxscr $(XLIB) -o cx6
	strip cx6
	chmod a+x cx6
	cd Utils; make -f Makefile.freeBSD
	cd SERVER; make -f Makefile.freeBSD
#        cd UnixSpace; make

#pp:     pp.o
#        g++ -g -O pp.o -L. -lcrypt -lcxform -lcxdb -lcxscr $(XLIB) -o pp

$(WWWDIR)/telnet.html: java/telnet/telnet.html
	cp java/telnet/telnet.html $(WWWDIR)/telnet.html

$(WWWDIR)/telnet.jar: java/telnet/classes/telnet.jar
	cp java/telnet/classes/telnet.jar $(WWWDIR)/telnet.jar

libcxdb.a: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h
	cd DB_CLASS; make -f Makefile.freeBSD

libcxform.a: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h
	cd FORM_CLASS; make -f Makefile.freeBSD

libcxscr.a:
	cd SCREEN; make -f Makefile.freeBSD

/usr/local/etc/CX_Messages/Main: CX_Messages/Main
	 tar -cf - CX_Messages | (cd /usr/local/etc; tar -xf - );

/usr/local/etc/CX_Menu/Main: CX_Menu/Main
	 tar -cf - CX_Menu | (cd /usr/local/etc; tar -xf - );

/usr/local/etc/CX_Icons/Main: CX_Icons/Main
	 tar -cf - CX_Icons | (cd /usr/local/etc; tar -xf - );

/usr/local/etc/CX_Help/Main: CX_Help/Main
	 tar -cf - CX_Help | (cd /usr/local/etc; tar -xf - );

.c.o:   DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h
	./update    ${.IMPSRC}
	${CC} -c  ${.IMPSRC}

.cc.o:  DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h
	./update    ${.IMPSRC}
	${CC}  -c ${.IMPSRC}

.cpp.o:  DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h
	./update    ${.IMPSRC}
	${CC}  -c ${.IMPSRC}

main.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h main.cpp
	./update main.cpp
	${CC} -c $(CFLAGS) main.cpp

context.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h context.cpp
	./update context.cpp
	${CC} -c $(CFLAGS) context.cpp

action.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h action.cpp
	./update action.cpp
	${CC} -c $(CFLAGS) action.cpp

query.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h query.cpp
	./update query.cpp
	${CC} -c $(CFLAGS) query.cpp

cmd.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h cmd.cpp
	./update cmd.cpp
	${CC} -c $(CFLAGS) cmd.cpp

edit.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h edit.cpp
	./update edit.cpp
	${CC} -c $(CFLAGS) edit.cpp

chart.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h chart.cpp
	./update chart.cpp
	${CC} -c $(CFLAGS) chart.cpp

menu.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h menu.cpp
	./update menu.cpp
	${CC} -c $(CFLAGS) menu.cpp

write.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h write.cpp
	./update write.cpp
	${CC} -c $(CFLAGS) write.cpp

pipe.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h CX_pipe.h pipe.cpp
	./update pipe.cpp
	${CC} -c $(CFLAGS) pipe.cpp

index.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h index.cpp
	./update index.cpp
	${CC} -c $(CFLAGS) index.cpp

remote.o: DB_CLASS/ConteXt.h DB_CLASS/CX_BASE.h CX_Browser.h remote.cpp
	./update remote.cpp
	${CC} -c $(CFLAGS) remote.cpp

./update: ./update.o ver.h
	${CC} ./update.o -o ./update

./update.o:  ./update.c ver.h
	gcc -DVER=$(VER) -c ./update.c

registration.o:  registration.c
	gcc -c registration.c

ucrypt.o:  ucrypt.cpp
	g++ -c ucrypt.cpp

/usr/local/etc/Methods.cc:   Methods.cc
	./update Methods.cc
	cp Methods.cc /usr/local/etc
