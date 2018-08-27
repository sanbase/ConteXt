/*
			 DBMS ConteXt V.5.7
			  ConteXt utilites

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: lashenko@unixspace.com
		Last modification:Fri Oct  2 18:53:38 2015
			Module:Create_DB.cpp
*/

#include "../CX_Browser.h"
static struct st sss;
static struct header *contex;

//extern struct ret_xy ret;

void Create_From_Schema(char *iname);

Terminal *term;
Line *EditLine;

static void exit_sig(int i)
{
	if(i==SIGHUP)
		kill(getppid(),SIGHUP);
	delete term;
	sleep(1);
	exit(0);
}

void suspend_init()
{
	for(int i=1;i<SIGUSR1;i++)
	{
#ifdef _BSDI_VERSION
		if(i==SIGSEGV)
			continue;
#endif
		if(i==SIGALRM || i==SIGCHLD)
			continue;
		signal(i,exit_sig);
	}
	return;
}

main(int argc,char **argv)
{
	char *name=NULL;
	int new_database;

	term=new Terminal();
	if(strcmp(argv[0],"CX_Edit") && argc==2 && !access(argv[1],R_OK))
	{
		Create_From_Schema(argv[1]);
		delete term;
		exit(0);
	}
	EditLine = new Line(term);

	term->dpp(0,0);
	term->Set_Color(8,0);
	term->clean();
	suspend_init();
	new_database=0;
	if(!strcmp(argv[0],"CX_Edit"))
	{
		if(argc<2)
			name=Select_From_Dir("",if_base,"Select File",1);
		else if(argc>=2)
		{
			name=(char *)malloc(2*strlen(argv[1])+1);
			full(argv[1],argv[1],name);
			strcpy(name,argv[1]);
		}
	}
	else if(argc>=2)
	{
		name=(char *)malloc(2*strlen(argv[1])+1);
		strcpy(name,argv[1]);
		new_database=1;
	}

	if(name==NULL)
	{
		char str[256];

		*str=0;
		term->Set_Color(0,03);

		term->dpp(0,term->l_y());
		term->clean_line();
		term->Set_Color(06,016);
		term->dps("File name:");
		term->Set_Color(0,14);
		if(EditLine->edit(0,str,255,64,10,term->l_y(),0)==F10)
		{
			delete term;
			exit(0);
		}
		name=(char *)malloc(strlen(str)+1);
		new_database=1;
		strcpy(name,str);
	}
	if(name!=NULL && *name)
	{
		hot_line(message(63));
		Make_Class(name,NULL,new_database);
	}
	delete term;
	exit(0);
}


void Create_From_Schema(char *iname)
{
	int fd=open(iname,O_RDONLY);
	int len=0;
	if(fd<0)
		return;
	char name[LINESIZE];
	struct stat st;
	fstat(fd,&st);
	char *buf=(char *)calloc(st.st_size+1,1);
	if(buf==NULL)
		return;
	read(fd,buf,st.st_size);
	close(fd);
	char *ch=strstr(buf,"CLASS");
	if(ch==NULL)
		return;
	ch=strchr(ch,'\"');
	strncpy(name,ch+1,sizeof name-1);
	ch=strchr(name,'\"');
	*ch=0;
	struct st *ss=NULL;
	ch=strstr(buf,"\n\n");
	ch+=2;
	Text_to_Schema(ch,ss,0,&len);

/*
	buf=(char *)realloc(buf,len+=strlen(ch)+1);
	*buf=0;
	Struct_to_XML(ss,buf,0,0);
	fd=creat("a1",0644);
	write(fd,buf,strlen(buf));
	close(fd);
*/
	free(buf);
	Make_Class(name,ss);
}

void Help(int,int)
{
}
