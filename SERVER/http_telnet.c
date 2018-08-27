#include "httpd.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

extern char **environ;

static int randtbl[32];
static int fptr    = 4;
static int rptr   = 1;
static int rand_deg = 31;
static int rand_sep = 3;
static int end_ptr  = 32;

void telnet_node( char *url, char *args, int in, FILE *out)
{
	execl("/usr/libexec/telnetd","telnetd",0);
}

int good_rand (int x)
{
	int hi, lo;

	hi = x / 127773;
	lo = x % 127773;
	x = 16807 * lo - 2836 * hi;
	if (x <= 0)
		x += 0x7fffffff;
	return (x);
}

long Rand()
{
	int i;
	int f, r;

	f = fptr;
	r = rptr;
	randtbl[f]+=randtbl[r];
	i = (randtbl[f] >> 1) & 0x7fffffff;
	if (++f >= end_ptr)
	{
		f = 1;
		++r;
	}
	else if (++r >= end_ptr)
		r = 1;

	fptr = f;
	rptr = r;
	return(i);
}

void Srand(long x)
{
	int i;

	randtbl[0] = x;
	for (i = 0; i < rand_deg-1; i++)
		randtbl[i+1] = good_rand(randtbl[i]);
	fptr = rand_sep;
	rptr = 0;
	for (i = 0; i < 10 * rand_deg; i++)
		Rand();
}

void crypto_node( char *url, char *args, int input, FILE *out)
{
	int ret,pid=-1,i,fd;
	char str[256],buf[8192];
	struct sockaddr soc;
	struct sockaddr_in *to;
	struct hostent *hp;
	int s=0,status,a;
	struct timeval timeout;
	unsigned char ch;
	int size=0;
	FILE *in;

	long i_key,o_key=time(0)%10000+getpid()*81;

	sprintf(str,"%d",o_key);
	fwrite(str,strlen(str),1,out);
	fflush(out);

//        while((size+=read(0,buf+size,64-size))<64);
	size=read(0,buf,64);
	*str=0;
	for(i=0;i<size;i++)
	{
		char st[16];
		sprintf(st,"%d ",(unsigned char )buf[i]);
		strcat(str,st);
		if(strlen(str)>sizeof str -16)
			goto END;
	}
	sprintf(buf,"/usr/local/bin/decrypt %s",str);
	if((in=popen(buf,"r"))==NULL)
		goto END;
	fread(str,64,1,in);
	pclose(in);

	i_key=atoi(str);
	Srand(o_key+i_key);

	bzero((char *)&soc, sizeof(struct sockaddr));
	to = (struct sockaddr_in *)&soc;

	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr("127.0.0.1");
	to->sin_port = htons(8884);

	if (to->sin_addr.s_addr == (u_int)-1)
	{
		hp = gethostbyname("127.0.0.1");
		if (!hp)
			exit(1);
		to->sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, (caddr_t)&to->sin_addr, hp->h_length);
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1)
		exit(1);

	if((status = connect(s, (struct sockaddr*)&soc, sizeof(soc)))<0)
		goto END;
	ret=0;
	do
	{
		fd_set writefds;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;
		FD_ZERO(&writefds);
		FD_SET(s, &writefds);
		ret = select(FD_SETSIZE, NULL, &writefds, NULL, &timeout);
		if(ret<0)
		{
			status=ret;
			break;
		}
	}
	while((ret == -1) && (errno == EINTR));
	if(ret<0)
		exit(1);
	if(!(pid=fork()))
	{
		while(read(input,&ch,1)>0)
		{       // reseive FROM applet  - applet send()
			ch+=Rand()%0xff;        // decoding
			if(write(s,&ch,1)<0)
				break;
		}
		goto END;
	}

	while((status=read(s,buf,sizeof buf))>0)
	{       // send TO applet - applet receive()
		for(i=0;i<status;i++)
			buf[i]+=Rand()%0xff;    // coding
		if(fwrite(buf,status,1,out)<0)
			break;
		fflush(out);
	}
END:
	if(s>0)
		close(s);
	if(pid>0)
		kill(pid,SIGKILL);
	else    if(pid==0)
			kill(getppid(),SIGKILL);
	exit(0);
}
