CC= g++ -I.. -DBSD -g -fhandle-exceptions
DST=/usr/local/bin

TARGET=Create_Form Create_DB Form_to_Blank Blank_to_Form CX_repair CX_restore
LIB= -L.. -lcxform -lcxdb -lcxscr  -L/usr/X11R6/lib -lXpm -lX11

all:    $(TARGET) CX_Adm/Methods

install:all
	cp $(TARGET) $(DST)
	rm -f CX_Adm/Methods.o
	tar -cf - CX_Adm | (cd /usr/local/lib; tar -xf - );
	ln -fs /usr/local/bin/Create_DB /usr/local/bin/CX_Edit
	cp cxadm /usr/local/bin

Create_Form: Create_Form.o ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h ../libcxdb.a ../libcxform.a
	$(CC) $(CFLAGS) Create_Form.o $(LIB) -o Create_Form

Create_DB: Create_DB.o ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h ../libcxdb.a ../libcxform.a
	$(CC) $(CFLAGS) Create_DB.o $(LIB) -o Create_DB

Form_to_Blank: Form_to_Blank.o ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h ../libcxdb.a ../libcxform.a
	$(CC) $(CFLAGS) Form_to_Blank.o $(LIB) -o Form_to_Blank

Blank_to_Form: Blank_to_Form.o ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h ../libcxdb.a ../libcxform.a
	$(CC) $(CFLAGS) Blank_to_Form.o $(LIB) -o Blank_to_Form

CX_repair: CX_repair.o ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h  ../libcxdb.a ../libcxform.a
	$(CC) CX_repair.o $(LIB) -o CX_repair

CX_restore: CX_restore.o ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h  ../libcxdb.a ../libcxform.a
	$(CC) CX_restore.o $(LIB) -o CX_restore

.cpp.o:  ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	${CC}  -c $(CFLAGS)  ${.IMPSRC}

Create_Form.o:  Create_Form.cpp ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	../update Create_Form.cpp
	${CC} -c $(CFLAGS) Create_Form.cpp

Create_DB.o:  Create_DB.cpp ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	../update Create_DB.cpp
	${CC} -c $(CFLAGS) Create_DB.cpp

Form_to_Blank.o:  Form_to_Blank.cpp ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	../update Form_to_Blank.cpp
	${CC} -c $(CFLAGS) Form_to_Blank.cpp

Blank_to_Form.o:  Blank_to_Form.cpp ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	../update Blank_to_Form.cpp
	${CC} -c $(CFLAGS) Blank_to_Form.cpp

CX_repair.o:  CX_repair.cpp ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	../update CX_repair.cpp
	${CC} -c $(CFLAGS) CX_repair.cpp

CX_restore.o:  CX_repair.cpp ../DB_CLASS/ConteXt.h ../DB_CLASS/CX_BASE.h ../CX_Browser.h
	../update CX_restore.cpp
	${CC} -c $(CFLAGS) CX_restore.cpp

CX_Adm/Methods: CX_Adm/Methods.cpp
	cd CX_Adm; make -f Makefile.freeBSD

clean:
	rm -f $(TARGET)
	rm -f CX_Adm/Methods
	rm -f *.o
