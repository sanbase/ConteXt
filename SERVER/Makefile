#
VER=0
LDADD=  -L.. -L/usr/local/lib -lcrypto

CFLAGS= -DSERVER -DBSD

CC=g++ -O2 -I.. -I/usr/local/include -DVER=$(VER)

all:    httpd decrypt CX_Server


SRCS=   http_config.c httpd.c http_request.c util.c http_dir.c \
	http_alias.c http_log.c http_mime.c http_access.c http_auth.c \
	http_get.c http_post.c http_script.c http_include.c rfc931.c \
	http_put.c http_delete.c http_ssh.c http_telnet.c ucrypt.c

OBJ=    http_config.o httpd.o http_request.o util.o http_dir.o \
	http_alias.o http_log.o http_mime.o http_access.o http_auth.o \
	http_get.o http_post.o http_script.o http_include.o rfc931.o \
	http_ssh.o http_telnet.o ucrypt.o \
	http_put.o http_delete.o

.c.o:
	${CC} -c  ${.IMPSRC}

#httpd:  $(OBJ)  $(LIB)
httpd:  $(OBJ)
	${CC} $(OBJ) -L.. -lcxdb -lcxscr -o httpd
	strip httpd;

decrypt:  decrypt.o
	${CC} decrypt.o $(LDADD) -o decrypt
	strip decrypt;

install: httpd decrypt
	cp httpd /usr/local/bin
	cp decrypt /usr/local/bin
clean:
	rm -f httpd decrypt
	rm -f *.o

http_config.o:   http_config.c
		${CC} -c http_config.c
httpd.o:         httpd.c
		${CC} -c httpd.c
http_request.o:  http_request.c
		${CC} -c http_request.c
util.o:          util.c
		${CC} -c util.c
http_dir.o:      http_dir.c
		${CC} -c http_dir.c
http_alias.o:    http_alias.c
		${CC} -c http_alias.c
http_log.o:      http_log.c
		${CC} -c http_log.c
http_mime.o:     http_mime.c
		${CC} -c http_mime.c
http_access.o:   http_access.c
		${CC} -c http_access.c
http_auth.o:     http_auth.c
		${CC} -c http_auth.c
http_get.o:      http_get.c
		${CC} -c http_get.c
http_post.o:     http_post.c
		${CC} -c http_post.c
http_script.o:   http_script.c
		${CC} -c http_script.c
http_include.o:  http_include.c
		${CC} -c http_include.c
rfc931.o:        rfc931.c
		${CC} -c rfc931.c
http_put.o:      http_put.c
		${CC} -c http_put.c
http_delete.o:   http_delete.c
		${CC} -c http_delete.c
http_ssh.o:      http_ssh.c
		${CC} -c http_ssh.c
http_telnet.o:   http_telnet.c
		${CC} -c http_telnet.c
ucrypt.o:         ucrypt.c
		${CC} -c ucrypt.c

decrypt.o:         decrypt.c
		${CC} -I/usr/local/ssl/include -c $(CFLAGS)  decrypt.c

CX_Server.o:      CX_Server.cc
	${CC} $(CFLAGS) -c CX_Server.cc
	#strip CX_Server

CX_Server:  CX_Server.o
	${CC} $(CFLAGS) CX_Server.o -L.. -lcxdb -lcxscr -pthread -o CX_Server
