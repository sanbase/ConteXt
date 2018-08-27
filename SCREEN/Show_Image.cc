/*
	Copyright (c) 1990-1995 Unitex+, ltd. Kiev, Ukraine
	Copyright (c) 1996 SOFTEX PLUS inc. Toronto ON, Canada.
	Copyright (c) 1996-2000 SanBase inc. Toronto ON, Canada.
	Written by Alexander L. Lashenko (1990-2000)
	and M. Flerov (1986-1988).
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
			Module:Show_Image.c
*/
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <tty_codes.h>

int copy(char *a,char *b)
{
	int fd,df,size;
	char buf[1024];

	if((fd=open(b,O_RDONLY))<0)
		return(01);
	if((df=open(a,O_RDWR|O_CREAT,0644))<0)
	{
		close(fd);
		return(-2);
	}
	while((size=read(fd,buf,1024))>0)
		write(df,buf,size);
	close(fd);
	close(df);
	return(0);
}

main(int argc, char **argv)
{
	char name[128],cmd[256];
	char *ch=getenv("TERM");
	if(ch==NULL || strcmp(ch,"applet"))
	{
		printf("Your environment dosen't support this command.\nYou should be connected via applet\n");
		exit(0);
	}
	if(argc!=2 || access(argv[1],R_OK) || (strstr(argv[1],".jpg")==NULL && strstr(argv[1],".gif")==NULL))
	{
		exit(0);
	}
	sprintf(name,"%s/Images/Tmp/%s",HTTPDIR,argv[1]);
	copy(name,argv[1]);
	chmod(name,0644);

	dpbeg("");
	dpp(0,0); dps("Image:"); dps(argv[1]);
	Show_Image(1,1,name+strlen(HTTPDIR),NULL);
	dpp(0,0); scrbufout();
	dpi();
	unlink(name);
	Del_Image(1,1);
	dpend();
	exit(0);
}
