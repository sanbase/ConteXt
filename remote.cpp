/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:remote.cpp
*/
#include "stdafx.h" 
#include "CX_Browser.h" 
 
 
#include <unistd.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
 
#include <netinet/in_systm.h> 
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h> 
#include <netdb.h> 
#include <sys/socket.h> 
#include <errno.h> 
#include <sys/param.h> 
#include <sys/time.h> 
#include <sys/types.h> 
#include <string.h> 
#include <stdlib.h> 
#include <signal.h> 
 
extern Terminal *term; 
 
#ifdef THREAD 
#include <pthread.h> 
static void *thread(void *a) 
{ 
	unsigned char ch; 
	int s=*(int *)a; 
	while(read(0,&ch,1)>0) 
	{ 
		if(write(s,&ch,1)<0) 
			break; 
	} 
	return(NULL); 
} 
#endif 
 
static void sig(int sign) 
{ 
	Ttyreset(); 
	exit(0); 
} 
 
static char *Remote_CX(char *str,char *host,char *passwd=NULL)
{ 
#ifndef SPARC 
	int s; 
	struct sockaddr soc; 
	struct sockaddr_in *to; 
	struct hostent *hp; 
	int status,ret; 
	struct timeval timeout; 
	static char buf[8196]; 
 
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
			return("Host unknown"); 
		to->sin_family = hp->h_addrtype; 
		bcopy(hp->h_addr, (caddr_t)&to->sin_addr, hp->h_length); 
	} 
 
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (s == -1) 
		return("Can't open socket"); 
	setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,(struct timeval *)&timeout,sizeof timeout);
	if((status = connect(s, (struct sockaddr*)&soc, sizeof(soc)))<0) 
	{ 
		return("Connection error"); 
	} 
	ret=0; 
	do 
	{ 
		fd_set writefds; 
		FD_ZERO(&writefds); 
		FD_SET(s, &writefds); 
		ret = select(FD_SETSIZE, NULL, &writefds, NULL, &timeout); 
		if(ret<=0) 
			break; 
	} 
	while((ret == -1) && (errno == EINTR)); 
 
	write(s,str,strlen(str)); 
 
	signal(SIGINT,sig); 
 
	int f=term->get_box(0,0,term->l_x(),term->l_y()); 
	term->cursor_visible(); 
	term->scrbufout(); 
 
#ifdef THREAD 
	pthread_t a; 
	pthread_create(&a,NULL,thread,&s); 
#else 
	unsigned char ch; 
	int pid; 
	if(!(pid=fork())) 
	{ 
		while(read(0,&ch,1)>0) 
		{ 
			if(write(s,&ch,1)<0) 
				break; 
		} 
		goto END; 
	} 
#endif 
	bzero(buf,sizeof buf); 
	if(passwd!=NULL)
	{
		write(s,passwd,strlen(passwd));
		write(s,"\r",1);
	}
	while((status=read(s,buf,sizeof buf))>0) 
	{ 
		if(fwrite(buf,status,1,stdout)==0) 
			break; 
		fflush(stdout); 
	} 
#ifndef THREAD 
END: 
#endif 
	if(s>0) 
		close(s); 
 
#ifndef THREAD 
	if(pid>0) 
		kill(pid,SIGKILL); 
#endif 
	term->dpp(0,0); term->Set_Color(8,017); term->clean();
	term->restore_box(f); 
	term->free_box(f); 
	term->cursor_invisible(); 
	return(buf); 
#else 
	return(""); 
#endif 
} 
 
char *CX_BROWSER::Remote_Browser(char *name,long page,char *passwd) 
{ 
	char *host=(char *)malloc(strlen(name)+1); 
	strcpy(host,name); 
 
	char *name_base=strchr(host,':'); 
	if(name_base==NULL) 
		return(NULL); 
	*name_base=0; 
	name_base++; 
	char *str=(char *)malloc(strlen(name_base)+strlen(GetLogin())+strlen(passwd)+64); 
//        sprintf(str,"->CX5 -N%s -R%d -U%s -P\"%s\"\r\n\r\n",name_base,(int)page,GetLogin(),passwd);
	sprintf(str,"->CX5 -N%s -R%d -U%s\r\n\r\n",name_base,(int)page,GetLogin());
	delete_menu(); 
	char *ch=Remote_CX(str,host,passwd);

	free(host); 
	free(str); 
	return(ch); 
} 
 
char *CX_BROWSER::Remote_Map(char *name,long page,struct sla *sla,int len) 
{ 
	char *host=(char *)malloc(strlen(name)+1); 
	strcpy(host,name); 
 
	char *name_base=strchr(host,':'); 
	if(name_base==NULL) 
		return(NULL); 
	*name_base=0; 
	name_base++; 
	char *str=(char *)malloc(strlen(name_base)+64); 
	char sla_str[64]; 
	sla_to_str(sla,sla_str); 
	sprintf(str,"->CX5 -N%s -R%ld -A%s:%d\r\n\r\n",name_base,page,sla_str,len); 
	delete_menu(); 
	char *ch=Remote_CX(str,host); 
	free(host); 
	free(str); 
	return(ch); 
} 
