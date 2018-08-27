/*
			 DBMS ConteXt V.5.7
			  ConteXt utilites

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: lashenko@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:CX_repair.cpp
*/

#include "../DB_CLASS/ram_base.h"
#include "../CX_Browser.h"
#include <sys/wait.h>

int visual;
Terminal *term;
Line *EditLine;

main(int argc, char **argv)
{
	CX_BASE *cx;

	visual=1;
	int field=0;
	int field0=0;
	char *folder=NULL;

	for(int i=0;i<argc;i++)
	{
		char *arg=argv[i];

		if(*arg!='-')
			continue;
		arg++;
		switch(*arg)
		{
			case 'N':
				arg++;
				while(*arg==' ')
					arg++;
				folder=arg;
				break;
			case 'Q':
				visual=0;
				break;
			case 'F':
				arg++;
				while(*arg==' ')
					arg++;
				field0=atoi(arg);
				break;
			default:
				break;
		}
	}
	if(folder==NULL && argc==2)
	{
		folder=argv[1];
		visual=0;
	}
	if(visual)
	{
		term=new Terminal();
		EditLine = new Line(term);

		term->dpp(0,0); term->Set_Color(28,017); term->clean();
		if(folder==NULL)
		{
			folder=Select_From_Dir(".",if_base,"",1);
			term->MultiColor(0,0,term->l_x()+1,term->l_y());

			if(folder==NULL)
			{
				delete term;
				exit(0);
			}
		}
	}
	else
	{
		if(folder==NULL)
			exit(1);
	}
	try
	{
		cx=new CX_BASE(folder);
	}
	catch (int i)
	{
		printf("%d-Exception:Can't open %s database\n",i,folder);
		exit(0);
	}
	int num=cx->Num_Fields();
	cx->Wlock(0);
	for(field=1;field<=num;field++)
	{
		if(field0)
			field=field0;
		if(!cx->is_index(field))
			continue;
		if(cx->Field_Descr(field)->b)
			continue;       // B++ tree

		CXDB_REPAIR *db;
		try
		{
			db=new CXDB_REPAIR(folder,field,visual);
		}
		catch(int i)
		{
			if(visual)
				delete term;
			printf("%d Can't open %s database\n",i,folder);
			exit(1);
		}
		db->Create_Tree();
		delete db;
		if(field0)
			break;
	}
	delete cx;
	if(visual)
		delete term;
	exit(0);
}

int rebuild_tree(char *name,long record,struct sla *sla)
{
	int pid,i;
	if(!dial(message(64),1))
		return(-1);
	if((pid=vfork())==0)
	{
		CXDB_REPAIR *ram;
		char str[LINESIZE];

		for(i=1;i<SIGUSR1;i++)
			signal(i,SIG_IGN);
		sprintf(str,"%s/db.log",name);
		int fd=open(str,O_RDWR|O_CREAT,0666);
		lseek(fd,0,SEEK_END);
		sprintf(str,"%s tree_error: page=%ld n=%d\n",local_time(),record,sla->n);
		write(fd,str,strlen(str));
		close(fd);
		try
		{
			ram=new CXDB_REPAIR (name,sla->n,1);
		}
		catch(...)
		{
			exit(1);
		}
		ram->Create_Tree();
		delete ram;
		exit(0);
	}
	waitpid(pid,&i,0);
	return(0);
}

void Help(int,int)
{
}
