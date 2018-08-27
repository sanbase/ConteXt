#include "StdAfx.h"

#include "screen.h"

#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/ioctl.h>
#endif


#ifdef WIN32
void exit_prog(int i)
{
	exit(0);
}
#else
void exit_prog(int i);
#endif
static char *cvtin_[]=
{
	"k1",  "k2",  "k3",  "k4",  "k5",  "k6",  "k7",  "k8",
	"k9",  "k0",  "F1",  "F2",  "kl",  "kr",  "ku",  "kd",
	"kP",  "kN",  "kh",  "@7",  "kI",  "kD"
};

static char *cvtout_[]=
{
	"cm",  "kl",  "kr",  "ku",  "kd",  "ce",  "cd",  "sr", //7
	"ic",  "dc",  "al",  "dl",  "im",  "ei",  "se",  "so", //15
	"Ic",  "oc",  "ve",  "vi",  "Sa",  "Ft",  "AF",  "AB", //23
	"Ea",  "Ia",  "Da",  "AL",  "DL",  "Gg",  "Po",  "Pc", //31
	"Pi",  "Ro",  "ro",  "Si",  "Sd",  "Ns",  "Cs",  "Pv", //39
	"Rl",  "Bw",  "Pl",  "DA",  "Fr",  "Ca",  "Cb",  "Cn", //47
	"Cf",  "SD",  "JM",  "Mf",  "Mb"
};

IO::IO()
{
	int i;

	bzero(this,sizeof (IO));
	Termcap t;
	cvtin =(char **)calloc(sizeof cvtin_,1);
	cvtout=(char **)calloc(sizeof cvtout_,1);
	buf=(char *)malloc(4096+32);

	memcpy(tabl," !ó#$%&Á()*+,-.Ó0123456789ùÌ<=>ûÍÄÅñÑÖîÉïàâäãåçéèüêëíìÜÇúÎáËÌÈ^Í`†°Ê§•‰£Â®©™´¨≠ÆØÔ‡·‚„¶¢ÏÎßòùô~",96);
	memcpy(tabl2,tabl,96);
#ifdef WIN32
	sprintf(buf,"C:/Program Files/UnixSpace/.%s.t","applet");
#else
	sprintf(buf,"/usr/local/etc/.%s.t",getenv("TERM"));
#endif
	if((i=open(buf,O_RDONLY))>0)
	{
		read(i,tabl,96);
		close(i);
	}
	buf[strlen(buf)-1]='u';
	if((i=open(buf,0))>0)
	{
		read(i,tabl2,96);
		close(i);
	}
	current_pos=0;
	for(i=0;i<(int)(sizeof(cvtin_)/sizeof (char *));i++)
	{
		if(!cvtin_[i])
		{
			cvtin[i]=NULL;
			continue;
		}
		cvtin[i] = t.tgetstr(cvtin_[i]);
	}
	for(i=0;i<(int)(sizeof(cvtout_)/sizeof (char *));i++)
	{
		if(!*cvtout_[i])
		{
			cvtout[i]=NULL;
			continue;
		}
		cvtout[i] = t.tgetstr(cvtout_[i]);
		if (cvtout[i]==NULL && i==0)
			exit_prog(0);
	}
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData; 
	int err;

	wVersionRequested = MAKEWORD( 1, 1);

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 )
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return;
	}

	struct sockaddr_in serv_addr;
	for(;;)
	{
		if( (m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			char *mess=NULL;
			switch(WSAGetLastError ())
			{
			case WSANOTINITIALISED:
				mess="A successful WSAStartup must occur before using this function.";
				break;
			case WSAENETDOWN:
				mess="The network subsystem or the associated service provider has failed.";
				break;
			case WSAEAFNOSUPPORT:
				mess="The specified address family is not supported. ";
				break;
			case WSAEINPROGRESS:
				mess="A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. ";
				break;
			case WSAEMFILE:
				mess="No more socket descriptors are available.";
				break;
			case WSAENOBUFS:
				mess="No buffer space is available. The socket cannot be created. ";
				break;
			case WSAEPROTONOSUPPORT:
				mess="The specified protocol is not supported. ";
				break;
			case WSAEPROTOTYPE:
				mess="The specified protocol is the wrong type for this socket. ";
				break;
			case WSAESOCKTNOSUPPORT:
				mess="The specified socket type is not supported in this address family. ";
				break;
			default:
				mess="Unknown error";
				break;
			}
			exit(0);
		}

//		ReuseAddress();

		bzero(&serv_addr,sizeof serv_addr);
		serv_addr.sin_family      = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port        = htons(8177);

		if( bind( m_sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
			break;

		listen(m_sockfd, 512);

		break;
	}

#ifdef _DEBUG
	STARTUPINFO StartInfo; 
	PROCESS_INFORMATION PInfo;
	memset(&StartInfo,0,sizeof StartInfo);

	CreateProcess("C:/CX5/bin/ConteXt.exe", NULL,NULL,NULL, false, NULL, NULL, NULL, &StartInfo, &PInfo);
	Terminal_PID=PInfo.hProcess;
	if(Terminal_PID==NULL)
	{

		MessageBox(NULL,"Can't run Browser!","Error",MB_OK|MB_ICONSTOP);
	}
#endif
	struct sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);

	sock_fd = accept(m_sockfd,(struct sockaddr *)&cli_addr,&clilen);
#endif
	if((num_colors=t.tgetnum("Co"))<=0)
		num_colors=t.tgetnum("Nf");
	if(num_colors<0)
		num_colors=0;
	am=t.tgetflag("am");
	get_dimention();
	if(l_x==0 || l_y==0)
	{
#ifndef WIN32
		struct winsize win;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != -1 && win.ws_col > 0)
		{
			l_x = win.ws_col;
			l_y = win.ws_row;
		}
		else
#endif
		{
			l_x = t.tgetnum("co");
			l_y = t.tgetnum("li");
		}

	}

	struct clr color;
	bzero(&color,sizeof color);
	color.fg=7;
	setColor(color);
	setPos(0,0);
	sdps(cvtout[6]);
	rus_lat=0;
}

IO::~IO()
{
	setColor(0,7);
	flush();
#ifdef WIN32
#ifdef _DEBUG
	TerminateProcess(Terminal_PID,0);
#endif
#endif
	free(cvtin);
	free(cvtout);
	free(buf);
}

void IO::get_dimention()
{
	l_x=0;
	l_y=0;
	get_dimention(&l_x,&l_y);
	x_s=-100;
	y_s=-100;
	bzero(&color_s,sizeof color_s);
	memset(&color_s,0xff,sizeof color_s);
}

void IO::get_dimention(int *w0,int *h0)
{
	if(cvtout[29]!=NULL)    // receive actual dimention
	{
		char buf[512];
		int w,h;
		bzero(buf,sizeof buf);
#ifndef WIN32
		write(1,cvtout[29],strlen(cvtout[29]));
		flush();
#else
		int num=send(sock_fd,cvtout[29],strlen(cvtout[29]),0);
		char *msg=NULL;
		if(num<0)
		{
				switch(WSAGetLastError())
				{
				case WSANOTINITIALISED:
					msg="cessful WSAStartup must occur before using this function. ";
					break;
				case WSAENETDOWN:
					msg="The network subsystem has failed. ";
					break;
				case WSAEACCES:
					msg="The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address. ";
					break;
				case WSAEINTR:
					msg="A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall. ";
					break;
				case WSAEINPROGRESS:
					msg="A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. ";
					break;
				case WSAEFAULT:
					msg="The buf parameter is not completely contained in a valid part of the user address space. ";
					break;
				case WSAENETRESET:
					msg="The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress. ";
					break;
				case WSAENOBUFS:
					msg="No buffer space is available. ";
					break;
				case WSAENOTCONN:
					msg="The socket is not connected. ";
					break;
				case WSAENOTSOCK:
					msg="The descriptor is not a socket. ";
					break;
				case WSAEOPNOTSUPP:
					msg="MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations. ";
					break;
				case WSAESHUTDOWN:
					msg="The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH. ";
					break;
				case WSAEWOULDBLOCK:
					msg="The socket is marked as nonblocking and the requested operation would block. ";
					break;
				case WSAEMSGSIZE:
					msg="The socket is message oriented, and the message is larger than the maximum supported by the underlying transport. ";
					break;
				case WSAEHOSTUNREACH:
					msg="The remote host cannot be reached from this host at this time. ";
					break;
				case WSAEINVAL:
					msg="The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled. ";
					break;
				case WSAECONNABORTED:
					msg="The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. ";
					break;
				case WSAECONNRESET:
					msg="The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable. ";
					break;
				case WSAETIMEDOUT:
					msg="The connection has been dropped, because of a network failure or because the system on the other end went down without notice. ";
					break;

				}
		}
#endif
		bzero(buf,sizeof buf);
		char *b=buf;
		for(int i=0;;b+=i)
		{
			if((i=read_bytes((unsigned char *)b,sizeof buf-(b-buf),2))==0)
				break;
			if((sscanf((char *)buf,"%d;%d;%d;%dH",&ev.y,&ev.x,&w,&h))==4)
			{
				*w0=w;
				*h0=h;
				break;
			}
		}
		cW=ev.x;
		cH=ev.y;
	}
}

int IO::Width()
{
	return(l_x);
}

int IO::Height()
{
	return(l_y);
}

int IO::Color()
{
	return(num_colors);
}

static int difstr(char *a,char *b)
{
	register int i;
	for(i=0;a[i] && b[i];i++)
		if(a[i]!=b[i]) break;
	if(!a[i] && !b[i]) return(0);  /* equval */
	if(!b[i]) return(1);           /* pattern > string */
	return(-1);
}

int IO::get()
{
	unsigned char buf[512];
	unsigned int c;

	bzero(buf,sizeof buf);
	flush();

	read_bytes(buf);
	c=(unsigned int)*buf;
	ev.x=-1;
	ev.y=-1;
	if(c==27)       // Esc
	{
		read_bytes(buf+1,1);
		if(buf[1]=='%' || buf[1]=='!')
		{
			read_bytes(buf+2,sizeof buf);
			if(sscanf((char *)(buf+1),"%%%d;%d;%dH",&ev.x,&ev.y,&ev.b)==3) //mouse
			{
				if(ev.x)
					ev.x--;
				if(ev.y)
					ev.y--;
				return(0);
			}
			if(sscanf((char *)(buf+1),"!%d;%d;%d~",&ev.x,&ev.y,&ev.b)==3)  //menu
			{
				return(300);
			}
		}
		else
		{
			for(unsigned char *ch=buf+1;;read_bytes(++ch,1))
			{
				int j=0;
				for(int i=0;i<(int)(sizeof(cvtin_)/sizeof (char *));i++)
				{
					int ret;
					if(cvtin[i]==NULL)
						continue;
					if(!(ret=difstr(cvtin[i],(char *)buf)))
					{
						if(i==21)
							c=DEL;
						else
							c=i+301;
						goto OK;
					}
					if(ret>0)
						j=1;
				}
				if(j==0)
					break;
			}
		}
		return(27);

	}
	if(c==14)       // Ctrl/N
	{
		if(rus_lat!=1) rus_lat=1;
		else    rus_lat=0;

		ind_lang();
		return(get());
	}
	if(c==21)       // Ctrl/U
	{
		if(rus_lat!=2) rus_lat=2;
		else    rus_lat=0;

		ind_lang();
		return(get());
	}
OK:
     /*   if (c>=(224+16) && c<256)        // russian keyboard
		c-=16;
	else
	{
		if (c>=(128+64) && c<=(175+64) && c!=219)
			c-=64;
	} */
	if(rus_lat==1)
	{
		if(c>=' ' && c<='~')
			c=tabl[c-' '];
	}
	if(rus_lat==2)
	{
		if(c>=' ' && c<='~')
			c=tabl2[c-' '];
	}

	return( c );
}

void IO::flush()
{
#ifndef WIN32
	write(1,buf,current_pos);
#else
	send(sock_fd,buf,current_pos,0);
#endif
	current_pos=0;
}


void IO::sdps(char *str)
{
	if(str==NULL)
		return;
	strcpy(buf+current_pos,str);
	current_pos+=strlen(str);
	if(current_pos>=2048)
		flush();
}

void IO::putChar(char ch)
{
	buf[current_pos++]=ch;
	if(current_pos>=4096)
		flush();
	switch(ch)
	{
	case '\r':
		x_s=0;
		break;
	case '\t':
		x_s=x_s+8-x_s%8;
		break;
	case '\b':
		x_s--;
		if(x_s<0)
			x_s=0;
		break;
	case '\n':
		if (y_s >= l_y-1)
			return;
		else
			y_s++;
		break;
	default:
		if(x_s >= l_x)
		{
			if(y_s < l_y - 1)
				y_s++;
			x_s = 0;
		}
		x_s++;
	}
}

void IO::setPos(int x, int y)
{
	x=Screen::checkBounds(x,0,l_x-1);
	y=Screen::checkBounds(y,0,l_y-1);

	if(x==x_s && y==y_s)
		return;

	if(x==0 && y==y_s+1)
		sdps("\n\r");
	else if(x==0 && y==y_s)
		sdps("\r");
	else if(x==x_s && y==y_s+1)
		sdps("\n");
	else if(x==x_s+1 && y==y_s && cvtout[2]!=NULL)
		sdps(cvtout[2]);
	else if(x==x_s-1 && y==y_s && cvtout[1]!=NULL)
		sdps(cvtout[1]);
	else if(x==x_s && y==y_s+1 && cvtout[4]!=NULL)
		sdps(cvtout[4]);
	else if(x==x_s && y==y_s-1 && cvtout[3]!=NULL)
		sdps(cvtout[3]);
	else
		sdps(tgoto(cvtout[0],x,y));
	x_s=x;
	y_s=y;
}

void IO::setColor(int bg, int fg)
{
	struct clr color=color_s;

	color.fg=fg;
	color.bg=bg;
	setColor(color);
}

void IO::setColor(struct clr color)
{
	if(Color()==0)
		return;

	if(color.frame!=color_s.frame && cvtout[44]!=NULL)
	{
		char str[64];
		sprintf(str,cvtout[44],color.frame);
		sdps(str);
		color_s.frame=color.frame;
	}

	if(color.atr2!=color_s.atr2)
	{
		if(color_s.atr2 && color.atr2==0 && cvtout[47]!=NULL)
			sdps(cvtout[47]);
		else if(color.atr2==1 && cvtout[45]!=NULL)
			sdps(cvtout[45]);
		else if(color.atr2==2 && cvtout[46]!=NULL)
			sdps(cvtout[46]);
		color_s.atr2=color.atr2;
	}

	if(color.font!=color_s.font && cvtout[21]!=NULL)
	{
		char str[64];
		sprintf(str,cvtout[21],(color.font>>4),color.font%4);
		sdps(str);
		color_s.font=color.font;
	}

	if(color.bwlt!=color_s.bwlt && cvtout[20]!=NULL)
	{
		char str[64];
		sprintf(str,cvtout[20],0,0,0,0,color.bwlt);
		sdps(str);
		color_s.bwlt=color.bwlt;
	}

	if(color.bg==color_s.bg && color.fg==color_s.fg)
		return;

	color.bg%=num_colors;
	color.fg%=num_colors;

	if(num_colors<=16)
	{
		if(color.fg==2 && color.bg==3)
			color.fg=0;
		if(cvtout[51]!=NULL)
		{
			if(cvtout[51][color.fg]>='A')
				color.fg=cvtout[51][color.fg]-'A'+10;
			else color.fg=cvtout[51][color.fg]-'0';
		}
		if(cvtout[52]!=NULL)
		{
			if(cvtout[52][color.bg]>='A')
				color.bg=cvtout[52][color.bg]-'A'+10;
			else color.bg=cvtout[52][color.bg]-'0';
		}
		if(color.fg==color.bg)
		{
			color.fg=0;
			if(color.bg==0)
				color.fg=7;
		}
	}

	if(cvtout[48]!=NULL)
	{
		sdps(tgoto(cvtout[48],color.bg,color.fg));
		color_s.bg=color.bg;
		color_s.fg=color.fg;
	}
	else
	{
		if(color.bg!=color_s.bg)
		{
			if(cvtout[23]==NULL)
				return;
			sdps(tgoto(cvtout[23],0,color.bg));
			color_s.bg=color.bg;
		}
		if(color.fg!=color_s.fg)
		{
			if(cvtout[22]==NULL)
				return;
			sdps(tgoto(cvtout[22],0,color.fg));
			color_s.fg=color.fg;
		}
	}
}

int IO::black_white(int x,int y,int l,int h,int atr)
{
	if(cvtout[20]==NULL)
		return(0);
	char str[64];

	sprintf(str,cvtout[20],x,y,l,h,atr%2);
	sdps(str);
	return(1);
}

int IO::cursor_visible(int i)
{
	if(i==0)
	{
		if(cvtout[19]==NULL)
			return(0);
		sdps(cvtout[19]);
	}
	else
	{
		if(cvtout[18]==NULL)
			return(0);
		sdps(cvtout[18]);
	}
	return(1);

}

int IO::erase_area(int x,int y,int l,int h,struct clr color)
{
	struct clr color_std=color_s;
	if(x+l>=l_x && h==1)
	{
		if(cvtout[5]!=NULL)
		{
			setColor(color);
			setPos(x,y);
			sdps(cvtout[5]);
			setColor(color_std);
			return(1);
		}
	}
	if(x==0 && l>=l_x && y+h>=l_y)
	{
		if(cvtout[6]!=NULL)
		{
			setColor(color);
			setPos(x,y);
			sdps(cvtout[6]);
			setColor(color_std);
			return(1);
		}
	}
	if(cvtout[24]==NULL)
		return(0);
	char str[64];
	setColor(color);
	sprintf(str,cvtout[24],x,y,l,h);
	sdps(str);
	setColor(color_std);
	return(1);
}

int IO::insertLine(int x, int y, int w, int h, int l)
{
	char str[64];

	if(x==0 && w>=l_x-1)
	{
		setPos(x,y);
		if(h==1)
		{
			if(cvtout[10]==NULL)
				return(0);
			sdps(cvtout[10]);
			return(1);
		}
		if(cvtout[27]==NULL)
			return(0);
		sprintf(str,cvtout[27],h);
		sdps(str);
		return(1);
	}
	if(cvtout[25]==NULL)
		return(0);
	sprintf(str,cvtout[25],x,y,w,h,l);
	sdps(str);
	return(1);
}

int IO::deleteLine(int x, int y, int w, int h, int l)
{
	char str[64];

	if(x==0 && w>=l_x-1)
	{
		setPos(x,y);
		if(h==1)
		{
			if(cvtout[11]==NULL)
				return(0);
			sdps(cvtout[11]);
			return(1);
		}
		if(cvtout[28]==NULL)
			return(0);
		sprintf(str,cvtout[28],h);
		sdps(str);
		return(1);
	}
	if(cvtout[26]==NULL)
		return(0);
	sprintf(str,cvtout[26],x,y,w,h,l);
	sdps(str);
	return(1);
}

int IO::New_Color(int num, int r, int g, int b)
{
	char str[64];

	if(cvtout[16]==NULL)
		return(0);
	sprintf(str,cvtout[16],num,r,g,b);
	sdps(str);
	return(1);
}

int IO::Show_Image(int x, int y,char *name, char *description,int l, int h,int transparent)
{
	if(cvtout[32]==NULL)
		return(0);
	char str[256];
	sprintf(str,cvtout[32],x,y,l,h,name,description==NULL?"":description,transparent);
	sdps(str);
	return(1);
}

static char name[64];
static void ext(int sig)
{
	if(*name)
		unlink(name);
	exit(0);
}

int IO::ShowDocument(char *url)
{
	if(cvtout[33]==NULL)
		return(0);
	char str[256];
	sprintf(str,cvtout[36],url);
	sdps(str);
	flush();
	return(1);
}

int IO::Frame(char *file)
{
	char str[256];
	int fd,fin;
	long i,size;
	unsigned char *buf;
	char *ch;
	int html=0;
	struct stat st;

	if(cvtout[33]==NULL)
		return(0);
	if((fin=open((char *)file,O_RDONLY))<0)
	{
		return(0);
	}
	ch=strrchr(file,'.');
	fstat(fin,&st);
	size=st.st_size;
	if(size>2048)
		size=2048;
	if((buf=(unsigned char *)malloc(size))==NULL)
		return(0);
	read(fin,(char *)buf,size);

	if(ch!=NULL && (!strcmp(ch,".html") || !strcmp(ch,".htm")))
	{
		html=1;
	}
	else if(ch==NULL)
	{
		for(i=0;i<size && i<128;i++)
		{
			if(buf[i]>0 && buf[i]<' ' && buf[i]!='\n' && buf[i]!='\r' && buf[i]!='\t')
			{
				html=2; // binary
				break;
			}
		}
		ch=".html";
	}
	char str1[256];
#ifndef WIN32
	sprintf(name,"/tmp/%ld%s",(long)st.st_size,ch==NULL?"":ch);
	if(mkfifo(name,0666))
		return(0);

	signal(SIGPIPE,ext);
	sprintf(str1,"cgi-bin/tmp_read?%s",name+4);
#else
	sprintf(name,"C:/Program Files/UnixSpace/tmp/%ld%s",(long)st.st_size,ch==NULL?"":ch);
	fd=creat(name, _S_IREAD | _S_IWRITE);
	close(fd);
	sprintf(str1,"%s",name);
#endif
	sprintf(str,cvtout[36],str1);
	sdps(str);
	flush();
	fd=open(name,O_WRONLY);
	if(fd<0)
		return(0);
	ch="<HTML><META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=KOI8-R\"><PRE>\n";
	if(!html)
		write(fd,ch,strlen(ch));
	for(;;)
	{
		char *a=NULL;
		int j=0,jsize=0;
		for(i=0;i<size;i++)
		{
			int c=buf[i];

			if(!html)
			{
				char *tabl="·‚˜Á‰Âˆ˙ÈÍÎÏÌÓÔÚÛÙıÊ „˛˚˝ îË¸‡Ò¡¬◊«ƒ≈÷⁄… ÀÃÕŒœ–êëíÅá≤¥ß¶µ°®Æ≠¨ÉÑâàÜÄäØ∞´•ª∏±†æπ∫∂ ™©¢§ΩºÖÇçåéèã“”‘’∆»√ﬁ€›ﬂŸÿ‹¿—ïñóòôöõúùûü£≥ ìø";
				if(c>=176 && c<=223)
				{
					if(c==176 || c==177 || c==178)
						buf[i]=' ';
					else if(c==179 || c==186)
						buf[i]='|';
					else if(c==196 || c==205)
						buf[i]='-';
					else    buf[i]='+';
					c=' ';
				}
				if(c>127)
					buf[i]=tabl[c-128];
			}

			a=(char *)realloc(a,++jsize);
			a[j++]=buf[i];
		}
		write(fd,a,jsize);
		free(a);
		if((size=read(fin,buf,size))<=0)
			break;
	}

	close(fin);
	ch="\n</PRE></HTML>";
	if(!html)
		write(fd,ch,strlen(ch));
	close(fd);
#ifdef WIN32
	get();
#endif
	return(1);
}


int IO::Put_Object(int *atrlist,char *description)
{
	char str[256];
	if(cvtout[30]==NULL)
		return(0);
	char atr[256];
	int i;

	*atr=0;
	if(description!=NULL)
		sprintf(atr,"\"%s\"",description);
	for(i=9;i>0;i--)
	{
		if(atrlist[i-1]!=0)
			break;
	}
	for(int j=0;j<i;j++)
	{
		sprintf(str,"%d;",atrlist[j]);
		strcat(atr,str);
	}
	sprintf(str,cvtout[30],atr);
	sdps(str);
	return(1);
}

int IO::Put_Object_Scr(int *atrlist,char *description)
{
	if(cvtout[31]==NULL)
		return(0);

	char str[2048];
	char atr[2048];
	int i;

	*atr=0;
	if(description!=NULL)
		sprintf(atr,"\"%s\"",description);
	for(i=9;i>0;i--)
	{
		if(atrlist[i-1]!=0)
			break;
	}
	for(int j=0;j<i;j++)
	{
		sprintf(str,"%d;",atrlist[j]);
		strcat(atr,str);
	}
	sprintf(str,cvtout[31],atr);
	sdps(str);
	return(1);
}


int IO::Del_Object(int obj,int x, int y)
{
	if(cvtout[33]==NULL)
		return(0);
	char str[256];
	sprintf(str,cvtout[33],obj,x,y);
	sdps(str);
	return(1);
}

int IO::Del_All_Objects(int obj)
{
	char str[64];

	if(cvtout[34]==NULL)
		return(0);
	sprintf(str,cvtout[34],obj);
	sdps(str);
	return(1);
}

int IO::New_Screen(int num, int x, int y, int w, int h, int lines, int columns)
{
	char str[64];

	if(cvtout[37]==NULL)
		return(0);
	sprintf(str,cvtout[37],x,y,columns,lines,num,w,h);
	sdps(str);
	return(1);
}

void IO::Set_Dimension(int x, int y, int w, int h)
{
	char str[64];

	if(cvtout[49]==NULL)
		return;
	sprintf(str,cvtout[49],x,y,w,h);
	sdps(str);
	return;
}

int IO::DrawArc(int arc,int color)
{
	char str[64];

	if(cvtout[43]==NULL)
		return(0);
	sprintf(str,cvtout[43],arc,color);
	sdps(str);
	return(1);
}

int IO::Del_Last_Object(int obj)
{
	char str[64];

	if(cvtout[40]==NULL)
		return(0);
	sprintf(str,cvtout[40],obj);
	sdps(str);
	return(1);
}

int IO::Set_Screen(int num)
{

	char str[64];

	if(cvtout[38]==NULL)
		return(0);
	sprintf(str,cvtout[38],num);
	sdps(str);
	flush();
	return(1);
}

void IO::ind_lang()
{
	int x=x_s,y=y_s;

	if(cvtout[32]!=NULL)
	{
		if(rus_lat==1)
			Show_Image((l_x-3)*cW, (l_y-1)*cH, "Images/CX/rus.gif", "Russian");
		else    if(rus_lat==2)
			Show_Image((l_x-3)*cW, (l_y-1)*cH, "Images/CX/ukr.gif", "Ukrainian");
		else
			Del_Object(ICON,(l_x-3)*cW, (l_y-1)*cH);
	}
	else
	{
		setPos(l_x,0);
		if(rus_lat==1) putChar('R');
		else    if(rus_lat==2) putChar('U');
		else    putChar('L');
	}
	setPos(x,y);
}

void IO::JMenu(char *menu)
{
	flush();
	if(cvtout[50]!=NULL)
	{
		if(menu!=NULL)
		{
			char *str=(char *)malloc(strlen(menu)+10);
			sprintf(str,cvtout[50],menu);
			sdps(str);
			free(str);
		}
		else
		{
			char str[32];
			sprintf(str,cvtout[50],"");
			sdps(str);
		}
	}
}

#ifdef WIN32
extern SOCKET sock_fd;
#endif

int IO::read_bytes(unsigned char *buf, int len,int t)
{
#ifdef WIN32
	::sock_fd=sock_fd;
#endif
	return(::read_bytes(buf,len,t));
}
