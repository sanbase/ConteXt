/*
			 DBMS ConteXt V.5.7
			  ConteXt utilites

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: lashenko@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:CX_restore.cpp
*/

#include "../DB_CLASS/ram_base.h"
#include "../CX_Browser.h"

Terminal *term;
Line *EditLine;

int visual;

class CX_RESTORE:public RAM_BASE
{
public:
	CX_RESTORE(char *folder,int field):RAM_BASE(folder,field){};
	void Create_Tree();
};

void CX_RESTORE::Create_Tree()
{
	char *ch=NULL;
	int f=-1,x,y;

	if(visual && max_record>1000)
	{
		x=(term->l_x()-50)/2;
		y=(term->l_y()/2);
		f=term->get_box(x-2,y-2,50+6,5);
		term->BOX(x-1,y-1,52,3,' ',0x6,7,0x6,7),
		term->Set_Color(016,0);
		term->dpp(x,y);
		term->scrbufout();
	}
	for(int page=1;page<=max_record;page++)
	{
		if(visual && f>=0 && (page%(max_record/50))==0)
		{
			term->dpo(' ');
			term->scrbufout();
		}
		Read(page,v_field,ch);
		if(ch!=NULL)
			bcopy(ch,(char *)pos+page*len_rec+sizeof (struct key),len_rec-sizeof (struct key));
		Insert_Node(page,1);
	}
	free(ch);
	if(visual)
	{
		term->dpp(x,y);
		term->Set_Color(014,0);
	}
	if(v_field->n==1)
	{
		Put_Buf(fd,(off_t)0,sizeof (struct key),(char *)pos);
		for(int page=1;page<=max_record;page++)
		{
			Put_Buf(fd,root_size+(page-1)*ss.size,sizeof (struct key),(char *)pos+page*len_rec);
			if(visual && f>=0 && (page%(max_record/50))==0)
			{
				term->dpo(' ');
				term->scrbufout();
			}
		}
	}
	else
	{
		char name[LINESIZE];

		sprintf(name,"%s/Tree.%d",Name_Base(),v_field->n);
		int fd=creat(name,0666);
		write(fd,pos,sizeof(struct key));
		for(int page=1;page<=max_record;page++)
		{
			write(fd,(char *)pos+page*len_rec,sizeof(struct key));
			if(visual && f>=0 && (page%(max_record/50))==0)
			{
				term->dpo(' ');
				term->scrbufout();
			}
		}
		close(fd);
	}
	if(f>=0)
	{
		term->restore_box(f);
		term->free_box(f);
	}
}

main(int argc, char **argv)
{
	CX_RESTORE *db;
	int field=0;
	char *folder=NULL;

	visual=1;
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
			case 'F':
				arg++;
				field=atoi(arg);
				break;
			case 'Q':
				visual=0;
				break;
			default:
				break;
		}
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
		if(field==0)
		{
			char str[16];

			*str=0;
			term->Set_Color(0,15);
			term->dpp(0,term->l_y()); term->clean();
			term->dps("Filed:"); EditLine->edit(0,str,8,8,8,term->l_y(),0);
			field=atoi(str);
		}
	}
	else
	{
		if(folder==NULL)
			exit(1);
	}
	if(field==0)
		field=1;
	try
	{
		db=new CX_RESTORE(folder,field);
	}
	catch(int i)
	{
		if(visual)
			delete term;
		printf("Can't open %s database\n",folder);
		exit(0);
	}
	db->Create_Tree();
	if(visual)
		delete term;
	exit(0);
}

void Help(int,int)
{
}
