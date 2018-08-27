#include "StdAfx.h"
#include "screen.h"

#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/socket.h>

#include <sys/utsname.h>
#include <setjmp.h>
#endif

void exit_prog(int i);
int in_fd=0;

#ifdef WIN32
extern SOCKET sock_fd;
#endif

int read_bytes(unsigned char *buf, int len,int t)
{
	struct timeval timeout;
	fd_set read_set;
	int i;
BEGIN:
	if(in_fd==0)
	{
		FD_ZERO(&read_set);
#ifdef WIN32
		FD_SET(sock_fd, &read_set);
#else
		FD_SET(0, &read_set);
#endif
		bzero(&timeout,sizeof timeout);
		if(t==0)
			timeout.tv_sec=60;
		else    timeout.tv_sec=t;
		if((i=select(1,&read_set,NULL,NULL,&timeout))==0)
		{
#ifndef WIN32
			if(getppid()==1)
			{
				kill(getpid(),SIGHUP);
				sleep(2);
				exit_prog(0);
			}
#endif
			if(t!=0)
				goto BEGIN;
			else    return(0);
		}
		if(i<0)
#ifdef WIN32
			exit(0);
#else
			exit_prog(0);
#endif
	}
#ifndef WIN32
//        i=getdtablesize();
//        return(recv(in_fd,(char *)buf,i,0));
	return(read(in_fd,buf,len));
#else
	return(recv(sock_fd,(char *)buf,len,0));
#endif
}
