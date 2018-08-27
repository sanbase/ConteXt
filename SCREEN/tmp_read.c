/*
	Copyright (c) 2000-2002 ConteXt Technology inc. Toronto ON, Canada.
	Written by Alexander L. Lashenko (2000-2002)
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

		Last modification:Tue Jul 30 11:08:00 2002
			Module:Window.cc
*/
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

char name[1024];

void ext(int sig)
{
	if(*name)
		unlink(name);
	exit(0);
}
void get_word(char *word, char *line)
{
	int x=0,y;

	for(x=0;line[x] && isspace(line[x]);x++);
	y=0;
	while(1)
	{
		if(!(word[y] = line[x]))
			break;
		if(isspace(line[x]))
			if((!x) || (line[x-1] != '\\'))
				break;
		if(line[x] != '\\') ++y;
		++x;
	}
	word[y] = '\0';
	while(line[x] && isspace(line[x])) ++x;
	for(y=0;line[y] = line[x];++x,++y);
}


char *find_type(char *buf,char *type)
{
	char line[256];
	int len=strlen(buf);
	int i;

	for(i=0;i<len;i++)
	{
		int j=0;
		static char cont[256];
		char typ[256];
#ifdef SPARC
		memset(line,0,sizeof line);
#else
		bzero(line,sizeof line);
#endif
		while(buf[i]!='\n')
			line[j++]=buf[i++];
		line[j]=0;
		if(*line=='#' || !*line)
			continue;
		get_word(cont,line);
		for(;;)
		{
			get_word(typ,line);
			if(!*typ)
				break;
			if(!strcmp(type,typ))
				return(cont);
		}
	}
	return(NULL);

}

int main(int argc,char **argv)
{
	char buf[32];
	int  size,fd;
	char hdr[1024];
	char *ch;

	signal(SIGPIPE,ext);
	strcpy(hdr,"Content-type: application/octet-stream\n");
	strcpy(name,"");
	strncat(name,argv[1],sizeof name - 16);
	ch=strrchr(name,'.');

	if(ch!=NULL)
	{
		if((fd=open("/var/www/conf/mime.types",O_RDONLY))>0)
		{
			struct stat st;
			char *b,*type;
			fstat(fd,&st);
			b=(char *)calloc(st.st_size+1,1);
			read(fd,b,st.st_size);
			close(fd);
			type=find_type(b,ch+1);
			if(type!=NULL)
				sprintf(hdr,"Content-type: %s\n",type);
			free(b);
		}
	}

	chroot("/tmp");
	fd=open(name,O_RDONLY);
	if(fd<=0)
		exit(0);
	ch="HTTP/1.0 200 OK\nMIME-version: 1.0\n";
	write(1,ch,strlen(ch));
	write(1,hdr,strlen(hdr));
	sprintf(hdr,"Content-Length:%d\n\n",atoi(argv[1]+1));
	write(1,hdr,strlen(hdr));

	while((size=read(fd,buf,sizeof buf))>0)
		if(write(1,buf,size)<0)
		       break;
	unlink(name);
	exit(0);
}
