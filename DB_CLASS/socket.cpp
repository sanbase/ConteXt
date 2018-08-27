/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:socket.cpp
*/
 
#include "StdAfx.h"  
#include "CX_BASE.h" 
 
#ifndef WIN32 
#include <sys/socket.h> 
#include <sys/select.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <sys/uio.h> 
#include <assert.h> 
#include <netdb.h> 
#include <errno.h> 
#include <sys/time.h> 
#include <sys/types.h> 
 
extern int errno; 
#else
#include <Winsock2.h>
#define socklen_t int
#endif

IPsocket::IPsocket() 
{ 
	m_sockfd = -1; 
} 
IPsocket::~IPsocket() 
{ 
	if(m_sockfd != -1)  
	{ 
		close(m_sockfd); 
	} 
} 
 
int  IPsocket::fd() 
{ 
	return m_sockfd; 
} 
 
int  IPsocket::isValid() 
{ 
	return (m_sockfd != -1); 
} 
 
int IPsocket::ReuseAddress() 
{ 
	if(m_sockfd == -1) return 0; 
 
	int optval=1; 
	size_t optlen = sizeof(optval); 
	setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&optval, optlen); 
	return (optval); 
} 
 
int IPsocket::Port() 
{ 
	if(m_sockfd == -1) return 0; 

	struct sockaddr addr; 
	struct sockaddr_in *in = (sockaddr_in *)&addr; 
	socklen_t len = sizeof(addr); 
	getsockname(m_sockfd, &addr, &len); 
	return(ntohs(in->sin_port)); 
} 
 
// return 0 if specified timeout expires 
// 
int IPsocket::Select(struct timeval *tv) 
{ 
	int nfds = m_sockfd+1; 
	fd_set read_set; 
	FD_ZERO(&read_set); 
	FD_SET(m_sockfd, &read_set); 
 
	int nSel; 
 
	do  
	{ 
		nSel = select(nfds, &read_set, NULL, NULL, tv); 
	}  
	while(nSel == -1 && errno == EINTR); 
 
	return nSel; 
} 
int IPsocket::Select(int timeout) 
{ 
	struct timeval tv; 
	tv.tv_sec = timeout; 
	tv.tv_usec = 0; 
	return(Select(&tv)); 
} 
int IPsocket::Select() 
{ 
	return(Select(0)); 
} 
 
 
ClientIPsocket::ClientIPsocket(const char *host, unsigned short port) 
{ 
	struct hostent *h; 
	h = gethostbyname(host); 
	if(h==NULL) 
	{ 
		throw 4; 
		return; 
	} 
	struct in_addr *host_addr = (struct in_addr *) *h->h_addr_list; 
	if(host_addr == NULL) 
	{ 
		throw 5; 
		return; 
	} 
	if( (m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		throw 6; 
		return; 
	} 
	bzero(&serv_addr,sizeof serv_addr); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr  = *host_addr;  
	serv_addr.sin_port   = htons(port); 
 
	if(connect(m_sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	{ 
		close(m_sockfd); 
		m_sockfd = -1; 
		throw 7; 
		return; 
	} 
} 
 
ClientIPsocket::~ClientIPsocket() 
{ 
} 
 
ServerIPsocket::ServerIPsocket() 
{ 
	Init(0); 
} 
ServerIPsocket::ServerIPsocket(unsigned short port) 
{ 
	Init(port); 
} 
void ServerIPsocket::Init(unsigned short port) 
{ 
	int res = 0; 
 
	for(;;) 
	{ 
		if( (m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
			break; 
 
		ReuseAddress(); 
 
		bzero(&serv_addr,sizeof serv_addr); 
		serv_addr.sin_family      = AF_INET; 
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
		serv_addr.sin_port        = htons(port); 
 
		if( bind( m_sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
			break; 
 
		listen(m_sockfd, 512); 
 
		return; 
	} 
 
	if(!res) 
	{ 
		close(m_sockfd); 
		m_sockfd = -1; 
		return; 
	} 
} 
 
ServerIPsocket::~ServerIPsocket() 
{ 
} 
 
int ServerIPsocket::Accept() 
{ 
	socklen_t clilen = sizeof(cli_addr); 
	int fd; 
	do 
	{ 
		fd = accept(m_sockfd,(struct sockaddr *)&cli_addr,&clilen); 
	} 
	while(fd==-1 && errno==EINTR); 
 
	return(fd); 
} 
 
// return -1 if error / -2 if time limit expires 
// 
int ServerIPsocket::Accept(int timeout) 
{ 
	int nSel = Select(timeout); 
 
	if(nSel < 0)                 return nSel; 
	else if(nSel == 0)           return -2;         // timeout 
	else                         return(Accept()); 
} 
 
Sock_Message::Sock_Message() 
{ 
	m_sock = NULL; 
	m_sock = NULL; 
	LocalInit(-1,-1); 
} 
 
Sock_Message::Sock_Message(const char *host, unsigned short port) 
{ 
	m_sock = NULL; 
	m_sock = new ClientIPsocket(host,port); 
	int fd = m_sock->m_sockfd; 
	LocalInit(fd,fd); 
} 
 
Sock_Message::Sock_Message(int sockfd) 
{ 
	m_sock = NULL; 
	m_sock = NULL; 
	LocalInit(sockfd, sockfd); 
} 
 
Sock_Message::Sock_Message(int readfd, int writefd) 
{ 
	m_sock = NULL; 
	LocalInit(readfd,writefd); 
} 
 
void Sock_Message::LocalInit(int readfd, int writefd) 
{ 
	m_len = 0; 
	fdread = readfd; 
	fdwrite = writefd; 
	m_timeout = NULL;               // infinite default timeout 
 
} 
 
 
// duplex channel 
void Sock_Message::Init(int sockfd) 
{ 
	if(fdread >=0)  close(fdread); 
	if(fdwrite>=0)  close(fdwrite); 
	fdread = sockfd; 
	fdwrite = sockfd; 
} 
// 
void Sock_Message::Init(int readfd, int writefd) 
{ 
	if(fdread >=0) 
		close(fdread); 
	if(fdwrite>=0) 
		close(fdwrite); 
	fdread = readfd; 
	fdwrite = writefd; 
} 
 
Sock_Message::~Sock_Message() 
{ 
	if(fdread >=0) 
		close(fdread); 
	if(fdwrite>=0) 
		close(fdwrite); 
	if(m_timeout) 
		delete m_timeout; 
	if(m_sock) 
		delete m_sock; 
} 
 
// `ReadMsg' method separates stream of bytes into messages 
// each message is prefixed by length (two octets in MSB first order) 
// These two octets are not included in the length of messsage 
// Return pointer to the message terminated with '\0' or NULL if socket 
// is closed 
 
// read message with specified timeout 
int Sock_Message::ReadMsg(int timeout,char *&InBuf) 
{ 
	struct timeval tv; 
	tv.tv_sec = timeout; 
	tv.tv_usec = 0; 
	return(ReadMsg(&tv,InBuf)); 
} 
// read message with default timeout 
int Sock_Message::ReadMsg(char *&InBuf) 
{ 
	return(ReadMsg(m_timeout,InBuf)); 
} 
int Sock_Message::ReadMsg(struct timeval *tv,char *&InBuf) 
{ 
	if(!isValid()) 
		return(-1); 
 
	if(fdread<0) 
		return(-2); 
	m_len =0; 
 
// select fdread with specified timeout 
// 
 
	int nfds = fdread+1; 
	fd_set read_set; 
	FD_ZERO(&read_set); 
	FD_SET(fdread, &read_set); 
	int nSel; 
 
	do 
	{ 
		nSel = select(nfds, &read_set, NULL, NULL, tv); 
	} 
	while(nSel == -1 && errno == EINTR); 
 
	if(nSel <= 0) 
		return(-3); 
 
//        assert(nSel==1); 
 
	int len; 
	int n = readn((char *)&len, sizeof len); // read packet header
	if(n < (int)sizeof len) 
		return(-4); 
 
	if(len==0) 
		return(0); 
 
	InBuf=(char *)realloc(InBuf,len+1); 
 
	n = readn(InBuf,len); 
	if(n != len) 
		return(-5); 
 
	m_len = len; 
 
	InBuf[len] = '\0'; 
	return (len); 
} 
 
// read exactly 'len' bytes from socket 
// if error occurs then returns actual NN of bytes (< len) 
// 
int Sock_Message::readn(char *buf, int len) 
{ 
	int nleft = len, nread; 
	char *ptr = buf; 
 
	while ( nleft > 0) 
	{ 
		do 
		{ 
			nread = recv(fdread, ptr, nleft,0); 
		} 
		while(nread < 0 && errno == EINTR); 
 
		if(nread <= 0) 
		{ 
			break; 
		} 
 
		nleft -= nread; 
		ptr   += nread; 
	} 
	return (len - nleft); 
} 
 
int Sock_Message::WriteMsg(const void *msg, int len) 
{ 
 
	if(fdwrite<0) 
	{ 
		return -1; 
	} 
	char *Msg = (char *)msg; 
#ifndef WIN32
	struct iovec iov[2]; 
	iov[0].iov_base = (char *)&len; 
	iov[0].iov_len  = sizeof len; 
	iov[1].iov_base = Msg; 
	iov[1].iov_len = len; 
 
	int res = writev(fdwrite,iov,2)!=len+(int)sizeof len; 
#else
	char *buf=(char *)malloc(len+sizeof (len));
	memcpy(buf,(char *)&len,sizeof (len));
	memcpy(buf+sizeof (len),msg,len);
	int res=write(fdwrite,buf,len+sizeof(len));
	free(buf);
#endif
	return res; 
} 
 
int Sock_Message::WriteMsg(const char *msg) 
{ 
	return(WriteMsg((void *)msg, msg==NULL?0:strlen(msg))); 
} 
 
void Sock_Message::Close() 
{ 
	close(fdread); 
	fdread = -1; 
	close(fdwrite); 
	fdwrite = -1; 
} 
 
void Sock_Message::DefaultTimeout(int seconds) 
{ 
	if(!m_timeout) 
		m_timeout = new timeval; 
	m_timeout->tv_sec = seconds; 
	m_timeout->tv_usec = 0; 
} 
 
int Sock_Message::isValid() 
{ 
	return (fdwrite != -1); 
} 
 
ClientIPsocket *Sock_Message::getsock() 
{ 
	return(m_sock); 
} 
 
int Sock_Message::MsgLength() 
{ 
	return(m_len); 
} 
 
int Sock_Message::Getfd(int flag) 
{ 
	return(flag ? fdwrite : fdread); 
} 
#ifndef WIN32 
UNIXServerSocket::UNIXServerSocket(const char *path) 
{ 
	struct sockaddr_un serv_addr; 
 
	if((m_sockfd=socket(AF_UNIX,SOCK_STREAM,0))>=0) 
	{ 
		bzero(&serv_addr,sizeof serv_addr); 
		serv_addr.sun_family=AF_UNIX; 
		strncpy(serv_addr.sun_path, path,sizeof(serv_addr.sun_path)); 
 
		unlink(path); 
		if(bind( m_sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr))>=0) 
		{ 
			listen(m_sockfd, 512); 
			return; 
		} 
	} 
	close(m_sockfd); 
	m_sockfd = -1; 
	return; 
} 
 
UNIXServerSocket::~UNIXServerSocket() 
{ 
	if(m_sockfd>0) 
		close(m_sockfd); 
} 
 
int UNIXServerSocket::Accept() 
{ 
	struct sockaddr_un cli_addr; 
	socklen_t clilen = sizeof(cli_addr); 
	int fd; 
	do 
	{ 
		fd = accept(m_sockfd,(struct sockaddr *)&cli_addr, &clilen); 
	} 
	while(fd==-1 && errno==EINTR); 
 
	return(fd); 
} 
 
UNIXClientSocket::UNIXClientSocket(const char *path) 
{ 
	if( (m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) 
		return; 
 
	struct sockaddr_un serv_addr; 
	bzero(&serv_addr,sizeof serv_addr); 
	serv_addr.sun_family = AF_UNIX; 
 
	strncpy(serv_addr.sun_path, path,sizeof(serv_addr.sun_path)); 
 
	if(connect(m_sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	{ 
		close(m_sockfd); 
		m_sockfd = -1; 
		return; 
	} 
} 
 
UNIXClientSocket::~UNIXClientSocket() 
{ 
	if(m_sockfd>0) 
		close(m_sockfd); 
} 
#endif 
static int HTTP_Connect (char *host, int *s) 
{ 
	struct sockaddr soc; 
	struct sockaddr_in *to; 
	struct hostent *hp; 
	int status,ret; 
	struct timeval timeout; 
 
	bzero((char *)&soc, sizeof(struct sockaddr)); 
	to = (struct sockaddr_in *)&soc; 
 
	to->sin_family = AF_INET; 
	to->sin_addr.s_addr = inet_addr(host); 
	to->sin_port = htons(80); 
 
	timeout.tv_sec = 10; 
	timeout.tv_usec = 0; 
 
	if (to->sin_addr.s_addr == (u_int)-1) 
	{ 
		hp = gethostbyname(host); 
		if (!hp) 
			return(-1); 
		to->sin_family = hp->h_addrtype; 
		bcopy(hp->h_addr, (char *)&to->sin_addr, hp->h_length); 
	} 
 
	*s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (*s == -1) 
		return(-1); 
#ifndef SPARC 
#ifndef WIN32
	setsockopt(*s,SOL_SOCKET,SO_SNDTIMEO,(struct timeval *)&timeout,sizeof timeout);
#else
	setsockopt(*s,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof timeout);
#endif
#endif 
	if((status = connect(*s, (struct sockaddr*)&soc, sizeof(soc)))<0) 
	{ 
		return(-1); 
	} 
	ret=0; 
 
	do 
	{ 
		fd_set writefds; 
		FD_ZERO(&writefds); 
		FD_SET(*s, &writefds); 
		ret = select(FD_SETSIZE, NULL, &writefds, NULL, &timeout); 
		if(ret<=0) 
			break; 
	} 
	while((ret == -1) && (errno == EINTR)); 
	return(ret); 
} 
 
char *HTTP_request(char *host,char *rq) 
{ 
	int s,i=0; 
	char str[LINESIZE]; 
	static char *ch=NULL; 
 
	sprintf(str,"%s HTTP/1.0\r\n\r\n",rq); 
	if(HTTP_Connect (host, &s)<0) 
		return(NULL); 
	write(s, str, strlen(str)); 
	struct timeval timeout; 
	fd_set read_set; 
 
	FD_ZERO(&read_set); 
	FD_SET(s, &read_set); 
	bzero(&timeout,sizeof timeout); 
	timeout.tv_sec=10;      // timeout 10 sec. 
	int nSel; 
 
	if((nSel=select(s+1,&read_set,NULL,NULL,&timeout))<=0) 
	{ 
		close(s); 
		if(nSel==0) 
			return(0); 
		return(NULL); 
	} 
	while(read(s, str ,1)==1) 
	{ 
		ch=(char *)realloc(ch,i+2); 
		ch[i++]=*str; 
		ch[i]=0; 
	} 
	close(s); 
	return(ch); 
} 
