#include "CX_BASE.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <time.h>

#define CONTROLLEN      (sizeof(struct cmsghdr) + sizeof(int))

void test(char *str)
{
	int fd=open("BUF",O_RDWR|O_CREAT,0644);
	lseek(fd,0,SEEK_END);
	write(fd,str,strlen(str));
	close(fd);
}

static pthread_t main_thread;

void set_signals()
{
#ifdef BSD
	signal(SIGCHLD,(void (*)())ign);
#else
	signal(SIGCHLD,SIG_IGN);
#endif
}

void handler(int sig)
{
	int status;
	if(sig==SIGCHLD)
	{
		waitpid(-1, &status, WNOHANG);
		signal(SIGCHLD,handler);
	}
	else if(sig==SIGPIPE)
	{
		usleep(10000);
	}
	else
	{
		char str[256];
		int fd=open("/var/log/httpd/cxd.log",O_RDWR|O_CREAT,0644);
		lseek(fd,0,SEEK_END);
		time_t i=time(0);
		char *ch=ctime(&i);
		sprintf(str,"%d Got fatal signal:%d  %s",getpid(),sig,ch);
		write(fd,str,strlen(str));
		close(fd);
		exit(0);
	}
}

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
void *thread(void *f)
{
	int fd=*(int *)f;
	char buf[1025],*ch;


	pthread_detach(pthread_self());
	*(int *)f=0;

	bzero(buf,sizeof buf);

	for(ch=buf;read(fd,ch,1)>0;ch++)
	{
		if(*ch=='\n')
			break;
	}
	if(!strncmp(buf,"SLOT ",5))
	{
		Sock_Message *s = new Sock_Message(fd);
		ch=strchr((char *)buf+5,';');
		if(ch!=NULL)
		{
			char slot[64];
			char name_base[256];
			long record;

			*ch=0;
			strcpy(name_base,(char *)buf+5);
			record=atoi(ch+1);
			if((ch=strchr(ch+1,';'))==NULL)
				goto END;
			strcpy(slot,ch+1);

			pthread_mutex_lock(&mtx);
			RCX_BASE *db;
			ch=NULL;
			try
			{
				db=open_db(name_base);
				db->Get_Slot(record,slot,ch);
			}
			catch(...)
			{
				ch=NULL;
			}
			pthread_mutex_unlock(&mtx);
		}
END:
		if(ch==NULL)
		{
			s->WriteMsg("");
		}
		else
		{
			s->WriteMsg(ch);
			free(ch);
		}
		delete s;
	}
	else if(!strncmp(buf,"TELNET",6))
	{
		if(!fork())
		{
			close(0);
			close(1);
			dup2(fd,0);
			dup2(fd,1);
//                        close(fd);

			execl("/usr/libexec/telnetd","telnetd",0);
		}
		pthread_exit(NULL);
	}
	else
	{
		char *ch=HTTP_request("127.1",buf);
		write(fd,ch,strlen(ch));
	}
	close(fd);
	pthread_exit(NULL);
}

main()
{
	char str[256];
	int port=81;
	int f=0;

	thread(&f);

	exit(0);
}
