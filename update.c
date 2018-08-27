#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "ver.h"

void bcopy(const void *s1, void *s2, size_t n)
{
	memcpy(s2,s1,n);
}
int bcmp(const void *s1, const void *s2, size_t n)
{
	return(memcmp(s1,s2,n));
}
void bzero(void *s, size_t n)
{
	memset(s,0,n);
}

main(int argc, char **argv)
{
	int fd,size;
	time_t dat;
	struct stat st;
	char *buf1,*buf2,*ch,str[64];

	if(argc<2 || (fd=open(argv[1],O_RDWR))<0)
		exit(1);
	fstat(fd,&st);
	dat=st.st_mtime;
	buf1=(char *)malloc(size=st.st_size);
	read(fd,buf1,size);
	close(fd);
	if(!strcmp(argv[1],"main.cpp"))
	{
		char *ch=strstr(buf1,"\nstatic char *ver=");
		if(ch!=NULL)
		{
			char *ch1=strchr(ch+1,'\n');
			char *tmp_buf=(char *)malloc(1+(size=(int)ch-(int)buf1));
			char str[256],dat[256];
			time_t len=time(0);

			bcopy(buf1,tmp_buf,size);
			tmp_buf[size]=0;
			sprintf(dat,"%s",ctime(&len));
			dat[strlen(dat)-1]=0;
			sprintf(str,"\nstatic char *ver=\"ConteXt 6.%d release %s [%s]\\nAlexander Lashenko. Toronto, Canada\\nmailto:lashenko@unixspace.com\";",VER,mod,dat);
			size+=strlen(str);
			tmp_buf=(char *)realloc(tmp_buf,size+1);
			strcat(tmp_buf,str);
			tmp_buf[size]=0;
			size+=st.st_size-((int)ch1-(int)buf1);
			tmp_buf=(char *)realloc(tmp_buf,size+1);
			strcat(tmp_buf,ch1);
			free(buf1);
			buf1=tmp_buf;
		}
	}
	if((fd=open("title",O_RDWR))<0)
		exit(2);
	fstat(fd,&st);
	buf2=(char *)malloc(st.st_size);
	read(fd,buf2,st.st_size);
	close(fd);
	if((ch=strstr(buf2,"Last modification:"))==NULL)
		exit(3);
	ch+=18;
	strcpy(ch,ctime(&dat));
	sprintf(str,"\t\t\tModule:%s\n*/\n",argv[1]);
	strcat(buf2,str);

	fd=creat("tmp_file",0644);
	write(fd,buf2,strlen(buf2));
	if(!memcmp(buf1,buf2,24))
		write(fd,buf1+strlen(buf2),size-strlen(buf2));
	else
		write(fd,buf1,size);
	close(fd);
	unlink(argv[1]);
	link("tmp_file",argv[1]);
	unlink("tmp_file");
	exit(0);
}
