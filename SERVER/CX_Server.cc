#include <DB_CLASS/CX_BASE.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <time.h>
Terminal *term=NULL;
#define CONTROLLEN      (sizeof(struct cmsghdr) + sizeof(int))
void test(char *str)
{
	int fd=open("/var/log/httpd/cxd.log",O_RDWR|O_CREAT,0644);
	lseek(fd,0,SEEK_END);
	write(fd,str,strlen(str));
	close(fd);
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
//                test("Error\n");
		return;
	}
	else
	{
		char str[256];
		time_t i=time(0);
		char *ch=ctime(&i);
		sprintf(str,"%d Got fatal signal:%d  %s",getpid(),sig,ch);
		test(str);
		exit(0);
	}
}

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
void *thread(void *f)
{
	int fd=*(int *)f;

	char   *ch;
	Sock_Message *s;

	pthread_detach(pthread_self());
	*(int *)f=0;

#ifdef FDRECIVE
	int    newfd;
	struct iovec    iov[1];
	struct msghdr   msg;
	struct cmsghdr *cmptr=NULL;
	char   buf[512];
	bzero(buf,sizeof buf);
	for (;;)
	{
		iov[0].iov_base = buf;
		iov[0].iov_len  = sizeof(buf);
		msg.msg_iov     = iov;
		msg.msg_iovlen  = 1;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		if((cmptr = (struct cmsghdr *)malloc(CONTROLLEN)) == NULL)
		{
			close(fd);
			return(NULL);
		}
		msg.msg_control    = (caddr_t) cmptr;
		msg.msg_controllen = CONTROLLEN;

		if(recvmsg(fd, &msg, 0)<=0)
		{
			close(fd);
			return(NULL);
		}
		if(msg.msg_controllen==CONTROLLEN)
		{
			newfd=*(int*)CMSG_DATA(cmptr);
			free(cmptr);
			break;
		}
	}
	close(fd);
#else
	s = new Sock_Message(fd);
	char *buf=NULL;
	s->ReadMsg(buf);
#endif
	if(buf!=NULL)
		ch=strchr((char *)buf,';');
	if(ch!=NULL)
	{
		char slot[64];
		char name_base[256];
		long record;
		RCX_BASE *db;

		*ch=0;
		strcpy(name_base,(char *)buf);
		record=atoi(ch+1);
		if((ch=strchr(ch+1,';'))==NULL)
			pthread_exit(NULL);
		strcpy(slot,ch+1);

		pthread_mutex_lock(&mtx);
		try
		{
			ch=NULL;
			db=open_db(name_base);
			db->Get_Slot(record,slot,ch);
		}
		catch(...)
		{
			ch=NULL;
		}
	      pthread_mutex_unlock(&mtx);
	}
#ifdef FDRECIVE
	s = new Sock_Message(newfd);
#endif
	if(ch==NULL)
	{
		s->WriteMsg("");
	}
	else
	{
		s->WriteMsg(ch);
		free(ch);
	}
#ifdef FDRECIVE
	close(newfd);
#endif
	delete s;
	pthread_exit(NULL);
}

main()
{
	char str[256];
/*
	for(int i=1;i<=SIGUSR1;i++)
	{
		if(i!=SIGTERM)
			signal(i,handler);
	}
*/
	if(!fork())
	{
		UNIXServerSocket *s = new UNIXServerSocket("/tmp/CXsocket");
		if(!s->isValid())
		{
			int fd=open("/var/log/httpd/cxd.log",O_RDWR|O_CREAT,0644);
			lseek(fd,0,SEEK_END);
			time_t i=time(0);
			char *ch=ctime(&i);

			sprintf(str,"%d can't open socket %s",getpid(),ch);
			write(fd,str,strlen(str));
			close(fd);
			exit(0);
		}
		for(;;)
		{
			int fd=s->Accept();

			pthread_t a;
			pthread_create(&a,NULL,thread,(void *)&fd);
			while(fd);
		}
	}
	exit(0);
}
