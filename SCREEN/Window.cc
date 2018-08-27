/*
	Copyright (c) 1996-2000 SanBase inc. Toronto ON, Canada.
	Written by Alexander L. Lashenko (2000-2001)
	E-mail: lashenko@unixspace.com

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	Last modification:Fri Mar 23 10:08:37 2001
			Module:Window.c
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

char name[64];

void ext(int sig)
{
	if(*name)
		unlink(name);
	exit(0);
}

void window(char *file)
{
	char str[64];
	int fd,fin;
	long i,size;
	unsigned char *buf;
	char *ch;
	int html=0;
	struct stat st;

	static int tabl[]=
	{
		0x2502,// #BOX DRAWINGS LIGHT VERTICAL
		0x2524,// #BOX DRAWINGS LIGHT VERTICAL AND LEFT
		0x2561,// #BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
		0x2562,// #BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
		0x2556,// #BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
		0x2555,// #BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
		0x2563,// #BOX DRAWINGS DOUBLE VERTICAL AND LEFT
		0x2551,// #BOX DRAWINGS DOUBLE VERTICAL
		0x2557,// #BOX DRAWINGS DOUBLE DOWN AND LEFT
		0x255d,// #BOX DRAWINGS DOUBLE UP AND LEFT
		0x255c,// #BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
		0x255b,// #BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
		0x2510,// #BOX DRAWINGS LIGHT DOWN AND LEFT
		0x2514,// #BOX DRAWINGS LIGHT UP AND RIGHT
		0x2534,// #BOX DRAWINGS LIGHT UP AND HORIZONTAL
		0x252c,// #BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
		0x251c,// #BOX DRAWINGS LIGHT VERTICAL AND RIGHT
		0x2500,// #BOX DRAWINGS LIGHT HORIZONTAL
		0x253c,// #BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
		0x255e,// #BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
		0x255f,// #BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
		0x255a,// #BOX DRAWINGS DOUBLE UP AND RIGHT
		0x2554,// #BOX DRAWINGS DOUBLE DOWN AND RIGHT
		0x2569,// #BOX DRAWINGS DOUBLE UP AND HORIZONTAL
		0x2566,// #BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
		0x2560,// #BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
		0x2550,// #BOX DRAWINGS DOUBLE HORIZONTAL
		0x256c,// #BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
		0x2567,// #BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
		0x2568,// #BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
		0x2564,// #BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
		0x2565,// #BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
		0x2559,// #BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
		0x2558,// #BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
		0x2552,// #BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
		0x2553,// #BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
		0x256b,// #BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
		0x256a,// #BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
		0x2518,// #BOX DRAWINGS LIGHT UP AND LEFT
		0x250c,// #BOX DRAWINGS LIGHT DOWN AND RIGHT
		0x2588,// #FULL BLOCK
		0x2584,// #LOWER HALF BLOCK
		0x258c,// #LEFT HALF BLOCK
		0x2590,// #RIGHT HALF BLOCK
		0x2580 // #UPPER HALF BLOC
	};

	if((fin=open((char *)file,O_RDONLY))<0)
	{
		return;
	}
	fstat(fin,&st);
	size=st.st_size;
	if(size>1024)
		size=1024;
	if((buf=(unsigned char *)malloc(size))==NULL)
	{
		return;
	}
	read(fin,(char *)buf,size);
	ch=strrchr(file,'.');
	if(ch!=NULL && (strcmp(ch,".html") || strcmp(ch,".htm")))
	{
		html=1;
	}
	else if(ch==NULL)
	{
		for(i=0;i<size && i<128;i++)
		{
			if(buf[i]<' ' && buf[i]!='\n' && buf[i]!='\r' && buf[i]!='\t')
			{
				html=2; // binary
				break;
			}
		}
		ch=".html";
	}
	sprintf(name,"/tmp/%lld,%s",st.st_size,ch==NULL?"":ch);

	if(mkfifo(name,0666))
		return;

	signal(SIGPIPE,ext);

	sprintf(str,"%c#cgi-bin/tmp_read?%s&",27,name+4);
	printf("%s",str);
	fflush(stdout);
	ch="<HTML><HEAD><PRE>\n";
	fd=open(name,O_WRONLY);
	if(fd<0)
		return;

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
				if(c>=128 && c<=175)
					c+=1040-128;
				else if(c>=224 && c<=239)
					c+=1088-224;
				else if(c>=179 && c<=179+sizeof (tabl)/sizeof (int))
					c=tabl[c-179];
				if(c>=128 || c=='&' || c=='<' || c=='>')
				{
					sprintf(str,"&#%d;",c);
					int len=strlen(str);
					a=(char *)realloc(a,jsize+=len);
					bcopy(str,a+j,len);
					j+=len;
					continue;
				}
			}
			a=(char *)realloc(a,++jsize);
			a[j++]=buf[i];
		}
		write(fd,a,jsize);
		free(a);
		if((size=read(fin,buf,size))<=0)
			break;
	}
END:
	close(fin);
	ch="\n</PRE></HEAD></HTML>";
	if(!html)
		write(fd,ch,strlen(ch));
	close(fd);
}
main(int argc,char **argv)
{
	if(!fork())
		window(argv[1]);
	else    sleep(1);
	exit(0);
}
