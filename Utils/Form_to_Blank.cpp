/*
			 DBMS ConteXt V.5.7
			  ConteXt utilites

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: lashenko@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:Form_to_Blank.cpp
*/

#include "../CX_Browser.h"

Terminal *term;
Line *EditLine;

main(int argc,char **argv)
{
	CX_BASE *form;
	char *name;
	char *folder=argv[1];
	if(argc!=2)
	{
		term = new Terminal();
		EditLine = new Line(term);

		term->dpp(0,0);
		term->Set_Color(0x8,0);
//                dpo(es);

		folder=Select_From_Dir("",if_base,"Select File",1);
		delete term;
		if(folder==NULL)
			exit(0);
		name=(char *)malloc(strlen(folder)+strlen(FORMDB)+2);
		sprintf(name,"%s/%s",folder,FORMDB);
	}
	else
	{
		name=(char *)malloc(strlen(folder)+strlen(FORMDB)+2);
		sprintf(name,"%s/%s",folder,FORMDB);
	}
	try
	{
		form = new CX_BASE(name);
	}
	catch(int i)
	{
		printf("Can't open %s\n",name);
		exit(0);
	}
	char *background=NULL;
	struct tag_descriptor *td=NULL;
	struct panel *panel=NULL;
	for(int page=1;page<=form->last_cadr();page++)
	{
		form->Get_Slot(page,1,name);
		int size=form->Get_Slot(page,2,background);
		if(size==0)
		{
			background=(char *)realloc(background,size=100*40);
			memset(background,' ',size);
			for(int i=0;i<size;i+=100)
				background[i]='\n';
		}
		int len;
		char *ch1=NULL;
		int num_fields=form->Read(page,3,ch1).len;
		num_fields/=(len=form->Field_Descr(3)->l);

		if(td!=NULL)
			free(td);
		if(len==sizeof (struct tag_descriptor))
		{
			td=(struct tag_descriptor *)ch1;
		}
		else
		{
			struct old_tag_descriptor *d=(struct old_tag_descriptor *)ch1;
			td=(struct tag_descriptor *)calloc(num_fields,sizeof (struct tag_descriptor));
			for(int i=0;i<num_fields;i++)
			{
				td[i].x=d[i].x;
				td[i].y=d[i].y;
				td[i].l=d[i].l;
				td[i].atr=d[i].atr;
				td[i].color=d[i].color;
				bzero(td[i].sla,sizeof (td[i].sla));
				bcopy(d[i].sla,td[i].sla,sizeof d[i].sla);
			}
			free(ch1);
		}

		ch1=NULL;
		int num_panel=form->Read(page,4,ch1).len/sizeof (struct panel);
		if(panel!=NULL)
			free(panel);
		panel=(struct panel *)ch1;

		int x=0,y=0;
		char *blank=NULL;
		len=0;
		for(int i=0;i<=size;i++)
		{
			int j;
			for(j=0;j<num_panel;j++)
			{
				if(x==0 && panel[j].y==y && (panel[j].atr&TABL))
				{
					blank=(char *)realloc(blank,(++len));
					blank[len-1]='~';
					break;
				}
			}
			for(j=0;j<num_fields;j++)
			{
				if(td[j].x==x && td[j].y==y)
				{
					char str[1024];

					sla_to_str(td[j].sla,str);
					int old_len=len;
					blank=(char *)realloc(blank,(len+=strlen(str))+1);
					strcat(blank+old_len-1,str);
					for(int k=0;k<td[j].l;k++)
						background[i+k]='_';
					break;
				}
			}
			blank=(char *)realloc(blank,++len+1);
			blank[len-1]=background[i];
			blank[len]=0;
			if(background[i]=='\n')
			{
				x=0;
				y++;
			}
			else if(background[i]=='\t')
				x=x+8-x%8;
			else
			x++;
			if(!background[i])
				break;
		}
		if(form->Num_Fields()>4)
		{
			char *ch=NULL;
			form->Get_Slot(page,5,ch);
			if(ch!=NULL && atoi(ch))
			{
				char str[64];
				sprintf(str,"%s{%d}\n",FMATR,atoi(ch));
				blank=(char *)realloc(blank,(len+=strlen(str))+1);
				strcat(blank,str);
				free(ch);
			}
		}
		int fd;
		char blank_name[128];
		sprintf(blank_name,"%s/%s",folder,BLANKDIR);
		if(access(blank_name,X_OK))
			mkdir(blank_name,0775);
		if(*name)
			sprintf(blank_name,"%s/%s/%s",folder,BLANKDIR,name);
		else
			sprintf(blank_name,"%s/%s/Blank.%d",folder,BLANKDIR,page);
		if((fd=creat(blank_name,0644))>0)
		{
			write(fd,blank,strlen(blank));
			close(fd);
		}
		free(blank);
	}
	delete (form);
	exit(0);
}

void Help(int,int)
{
}
