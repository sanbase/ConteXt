 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#ifndef WIN32 
#include "ConteXt.h" 
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
 
UNIX_UDPServerSocket::UNIX_UDPServerSocket(const char *path) 
{ 
	if((m_sockfd=socket(AF_UNIX,SOCK_DGRAM,0))>=0) 
	{ 
		bzero(&serv_addr,sizeof serv_addr); 
		serv_addr.sun_family=AF_UNIX; 
		strncpy(serv_addr.sun_path, path,sizeof(serv_addr.sun_path)); 
 
		unlink(path); 
		if(bind( m_sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))>=0) 
			return; 
	} 
	close(m_sockfd); 
	m_sockfd = -1; 
	cmptr=NULL; 
	return; 
} 
 
UNIX_UDPServerSocket::~UNIX_UDPServerSocket() 
{ 
	if(cmptr!=NULL) 
		free(cmptr); 
	unlink(serv_addr.sun_path); 
} 
 
void *UNIX_UDPServerSocket::RcvDgram() 
{ 
	memset(dgram, 0, sizeof(dgram)); 
	size_t n = recv(m_sockfd, dgram, sizeof(dgram)-1, 0); 
	if(n <= 0) 
	{ 
		m_len = 0; 
		return NULL; 
	} 
	m_len = n; 
	dgram[n] = '\0'; 
	return((void *)dgram); 
} 
 
void *UNIX_UDPServerSocket:: RcvFrom(struct sockaddr_un *from) 
{ 
	socklen_t len= sizeof(struct sockaddr_un); 
 
	memset(dgram, 0, sizeof(dgram)); 
	size_t n = recvfrom(m_sockfd, dgram, sizeof(dgram)-1, 0, 
	(struct sockaddr *)from, &len); 
	if(n <= 0) 
	{ 
		m_len = 0; 
		return NULL; 
	} 
 
	m_len = n; 
	dgram[n] = '\0'; 
	return((void *)dgram); 
} 
 
#define CONTROLEN (sizeof(struct cmsghdr)+sizeof(int)) 
 
int UNIX_UDPServerSocket::transfd(int targetfd,int USock,char *path) 
{ 
	struct iovec    iov[1]; 
	struct msghdr   msg; 
	char            buf[2];//TODO subst :" " 
 
	iov[0].iov_base=buf; 
	iov[0].iov_len=2; 
	msg.msg_iov     =iov; 
	msg.msg_iovlen  =1; 
 
	bzero(&serv_addr,sizeof serv_addr); 
	serv_addr.sun_family      = AF_UNIX; 
	strncpy(serv_addr.sun_path, path,sizeof(serv_addr.sun_path)); 
 
	msg.msg_name    =(caddr_t)&serv_addr; 
	msg.msg_namelen =sizeof(serv_addr); 
 
	if(targetfd<0) 
	{ 
#if BSDOS //TODO 
		msg.msg_control=NULL; 
		msg.msg_controllen=0; 
		buf[1]=-targetfd; 
#endif 
	} 
	else 
	{ 
		if(cmptr==NULL && (cmptr=(cmsghdr*)malloc(CONTROLEN))==NULL) 
			return(-1); 
		cmptr->cmsg_level=SOL_SOCKET; 
		cmptr->cmsg_type=SCM_RIGHTS; 
		cmptr->cmsg_len=CONTROLEN; 
#if BSDOS 
		msg.msg_control=(caddr_t)cmptr; 
		msg.msg_controllen=CONTROLEN; 
		*(int*)CMSG_DATA(cmptr)=targetfd; 
#endif 
		buf[1]=1; 
	} 
 
	buf[0]=1; 
 
	if(sendmsg(USock,&msg,0)<0) 
		return(-1); 
 
	return(0); 
 
} 
 
 
int UNIX_UDPServerSocket::recvfd(int USock) 
{ 
	char    *ptr,buf[256/**MAXLINE*/]; 
	struct iovec    iov[1]; 
	struct msghdr   msg; 
	int targetfd,nread,status; 
 
	status=-1; 
	while(1) 
	{ 
		iov[0].iov_base =buf; 
		iov[0].iov_len  =sizeof(buf); 
		msg.msg_iov     =iov; 
		msg.msg_iovlen  =1; 
		msg.msg_name    =NULL; 
		msg.msg_namelen =0; 
 
		if(cmptr==NULL&&(cmptr=(cmsghdr*)malloc(CONTROLEN))==NULL) 
			return(-1); 
#if BSDOS //TODO 
		msg.msg_control=(caddr_t)cmptr; 
		msg.msg_controllen=CONTROLEN; 
#endif 
 
		if((nread=recvmsg(USock,&msg,0))<0) 
		{ 
		} 
		else if(nread==0) 
		{ 
			return(-1); 
		} 
#if BSDOS 
		if(msg.msg_controllen==CONTROLEN) 
			targetfd=*(int*)CMSG_DATA(cmptr); 
#endif 
 
#if BSDOS 
		if(msg.msg_controllen==CONTROLEN) 
			return(targetfd); 
		else 
			return(-1); 
#else 
		return(targetfd); 
#endif 
	} 
} 
 
// return NN of bytes sent 
int UNIX_UDPServerSocket:: SendTo(const char *dgrm, int len, struct sockaddr_un *to) 
{ 
	int tolen = sizeof(struct sockaddr_un); 
 
	size_t n = sendto(m_sockfd, dgrm, len, 0,(struct sockaddr *)to, tolen); 
	if(n <= 0) 
		return 0; 
	return(n); 
} 
 
 
UNIX_UDPClientSocket::UNIX_UDPClientSocket(const char *path) 
{ 
	bzero(&serv_addr,sizeof serv_addr); 
	serv_addr.sun_family      = AF_UNIX; 
	strncpy(serv_addr.sun_path, path,sizeof(serv_addr.sun_path)); 
 
	if((m_sockfd=socket(AF_UNIX,SOCK_DGRAM,0))>=0) 
	{ 
		memset((char *) &cli_addr, 0, sizeof(cli_addr)); 
		cli_addr.sun_family      = AF_UNIX; 
		strcpy(cli_addr.sun_path,"/tmp/cx.XXXXXX"); 
		mktemp(cli_addr.sun_path); 
 
		if(bind(m_sockfd,(struct sockaddr *)&cli_addr,sizeof(cli_addr))>=0) 
			return; 
	} 
	close(m_sockfd); 
	m_sockfd = -1; 
	return; 
} 
 
UNIX_UDPClientSocket::UNIX_UDPClientSocket(const char *path,const char *clpath) 
{ 
	bzero(&serv_addr,sizeof serv_addr); 
	serv_addr.sun_family      = AF_UNIX; 
	strncpy(serv_addr.sun_path, path,sizeof(serv_addr.sun_path)); 
 
	if((m_sockfd=socket(AF_UNIX,SOCK_DGRAM,0))>=0) 
	{ 
		memset((char *) &cli_addr, 0, sizeof(cli_addr)); 
		cli_addr.sun_family      = AF_UNIX; 
		strcpy(cli_addr.sun_path,clpath); 
		int clilen=sizeof(cli_addr.sun_family)+strlen(cli_addr.sun_path); 
 
		if(bind(m_sockfd,(struct sockaddr *)&cli_addr,clilen+1)>=0) 
			return; 
	} 
	close(m_sockfd); 
	m_sockfd = -1; 
	return; 
} 
 
UNIX_UDPClientSocket::~UNIX_UDPClientSocket() 
{ 
	if(cmptr!=NULL) 
		free(cmptr); 
	unlink(cli_addr.sun_path); 
} 
 
// return NN of bytes sent 
int UNIX_UDPClientSocket::SendDgram(const char *p_dgram, int len) 
{ 
	int tolen=sizeof(struct sockaddr_un); 
 
	size_t n=sendto(m_sockfd,p_dgram,len,0,(struct sockaddr *)&serv_addr,tolen); 
	if(n <= 0) 
		return 0; 
	return(n); 
} 
 
// return NN of bytes sent 
int UNIX_UDPClientSocket::SendMsg(const char *msg) 
{ 
	return(SendDgram(msg,strlen(msg))); 
} 
 
void *UNIX_UDPClientSocket::RcvDgram() 
{ 
	socklen_t len = sizeof(serv_addr); 
 
	memset(dgram, 0, sizeof(dgram)); 
	size_t n = recvfrom(m_sockfd, dgram, sizeof(dgram)-1, 0, 
	(struct sockaddr *)&serv_addr, &len); 
	if(n <= 0) 
	{ 
		m_len = 0; 
		return NULL; 
	} 
 
	m_len = n; 
	dgram[n] = '\0'; 
	return((void *)dgram); 
} 
 
int UNIX_UDPClientSocket::transfd(int targetfd,int USock) 
{ 
	struct iovec    iov[1]; 
	struct msghdr   msg; 
	char            buf[2];//TODO subst :" " 
 
	iov[0].iov_base=buf; 
	iov[0].iov_len=2; 
	msg.msg_iov     =iov; 
	msg.msg_iovlen  =1; 
 
	bzero(&serv_addr,sizeof serv_addr); 
	serv_addr.sun_family      = AF_UNIX; 
	strncpy(serv_addr.sun_path, "/tmp/CX.srv",sizeof(serv_addr.sun_path)); 
 
	msg.msg_name    =(caddr_t)&serv_addr;   //NULL; 
	msg.msg_namelen =sizeof(serv_addr);             //0; 
 
	if(targetfd<0) 
	{ 
#if BSDOS 
		msg.msg_control=NULL; 
		msg.msg_controllen=0; 
#endif 
		buf[1]=-targetfd; 
	} 
	else 
	{ 
		if(cmptr==NULL&&(cmptr=(cmsghdr*)malloc(CONTROLEN))==NULL) 
			return(-1); 
		cmptr->cmsg_level=SOL_SOCKET; 
		cmptr->cmsg_type=SCM_RIGHTS; 
		cmptr->cmsg_len=CONTROLEN; 
#if BSDOS 
		msg.msg_control=(caddr_t)cmptr; 
		msg.msg_controllen=CONTROLEN; 
		*(int*)CMSG_DATA(cmptr)=targetfd; 
#endif 
		buf[1]=1; 
	} 
 
	buf[0]=1; 
 
	int nread; 
	if((nread=sendmsg(USock,&msg,0))<0) 
		return(-1); 
	return(0); 
 
} 
 
int UNIX_UDPClientSocket::recvfd(int USock) 
{ 
	char    *ptr,buf[256/**MAXLINE*/]; 
	struct iovec    iov[1]; 
	struct msghdr   msg; 
	int targetfd,nread,status; 
 
	status=-1; 
	while(1) 
	{ 
		iov[0].iov_base =buf; 
		iov[0].iov_len  =sizeof(buf); 
		msg.msg_iov     =iov; 
		msg.msg_iovlen  =1; 
		msg.msg_name    =NULL; 
		msg.msg_namelen =0; 
 
		if(cmptr==NULL&&(cmptr=(cmsghdr*)malloc(CONTROLEN))==NULL) 
			return(-1); 
#if BSDOS 
		msg.msg_control=(caddr_t)cmptr; 
		msg.msg_controllen=CONTROLEN; 
#endif 
 
		if((nread=recvmsg(USock,&msg,0))<0) 
		{ 
		} 
		else if(nread==0) 
		{ 
			return(-1); 
		} 
#if BSDOS 
		if(msg.msg_controllen==CONTROLEN) 
			targetfd=*(int*)CMSG_DATA(cmptr); 
		if(msg.msg_controllen==CONTROLEN) 
			return(targetfd); 
		else 
			return(-1); 
#else 
		return(targetfd); 
#endif 
	} 
} 
#endif 
