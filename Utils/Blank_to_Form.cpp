/*
			 DBMS ConteXt V.5.7
			  ConteXt utilites

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: lashenko@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:Blank_to_Form.cpp
*/

#include "../CX_Browser.h"
#include <dirent.h>

static int get_atr(char *ch,long *atr);
static void str_to_color(char *str,struct color *color);
static int get_blank_color(char *ch,struct color *color);
void create_form(char *name);
int write_form(char *base,char *blank_name);
static int dpb=0;
static int yes_ans=0;

Terminal *term;
Line *EditLine;

main(int argc,char **argv)
{
	char dirname[256];
	char *base=argv[1];
	if(argc<2)
	{
		term = new Terminal();
		EditLine = new Line(term);
		term->dpp(0,0);
		term->Set_Color(0x8,0);
//              dpo(es);
		dpb=1;
		char *folder=Select_From_Dir("",if_base,"Select File",1);
		if(folder==NULL)
		{
			delete term;
			exit(0);
		}
		base=folder;
		goto BEGIN;
	}
	if(argc==2)
	{
BEGIN:
		DIR *fd;
		struct dirent *dp;

		sprintf(dirname,"%s/%s",base,BLANKDIR);
		if((fd=opendir(dirname))==NULL)
		{
			char str[265];
			sprintf(str,"Can not open %s directory\n",dirname);
			if(dpb)
			{
				dial(str,4);
				delete term;
			}
			else
				printf(str);
			exit(2);
		}
		while((dp=readdir(fd))!=NULL)
		{
			int len=strlen(dp->d_name);
			if(len>2 && dp->d_name[len-1]=='b' && dp->d_name[len-2]=='.')
				continue;         /* файлы *.b не выводятся */
			if(dp->d_name[0]=='.' && dp->d_name[1]=='v')
				continue;         /* файлы *.b не выводятся */
			if(!if_read(dirname,dp->d_name) || if_dir(dirname,dp->d_name))
				continue;         /* его и читать-то нельзя... */
			write_form(base,dp->d_name);
		}
	}
	else
	{
		for(int arg=2;arg<argc;arg++)
			write_form(base,argv[arg]);
	}
	if(dpb)
		delete term;
	exit(0);
}

int write_form(char *base,char *blank_name)
{
	char *name=(char *)malloc(strlen(base)+strlen(BLANKDIR)+strlen(blank_name)+3);
	sprintf(name,"%s/%s/%s",base,BLANKDIR,blank_name);
	if(access(name,R_OK))
	{
		if(dpb)
		{
			char str[265];
			sprintf(str,"Can't open %s\n",name);
			dial(str,4);
		}
		else
			printf("Can't open %s\n",name);
		return(-1);
	}
	CX_BASE *db;
	struct stat st;
	int fd=open(name,O_RDONLY);
	fstat(fd,&st);
	char *background=(char *)calloc(st.st_size+1,1);
	long form_atr=0;
	read(fd,background,st.st_size);
	close(fd);
	free(name);

	struct tag_descriptor *des=NULL;
	struct panel *panel=NULL;
	int num_fields=0;
	int num_panel=0;
	int size=0;

	int i,x=0,y=0;
	int len=strlen(background);
	if(!len)
		return(0);
	try
	{
		db=new CX_BASE(base);
	}
	catch(...)
	{
		return(-1);
	}
	num_fields=0;
	for(i=0,size=0;i<len && background[i];i++)
	{
		if(background[i]=='\\' && background[i+1]=='^')
			continue;
		if(background[i]=='~' && ((i>0 && background[i-1]=='\n') || i==0))
		{
			if(num_panel==0)
			{
				if(panel!=NULL)
					free(panel);
				panel=(struct panel *)calloc(1,sizeof (struct panel));
				panel->y=y;
				panel->l=100;
				panel->atr=TABL|NO_EDIT;
				num_panel=1;
			}
			continue;
		}
		if(!strncmp(background+i,FMATR,strlen(FMATR)))
		{
			i+=strlen(FMATR);
			i+=get_atr(background+i,&form_atr);
			if(background[i]=='\n');
				i++;
		}
		if(background[i]=='^' && (i==0 || background[i-1]!='\\'))
		{
			char str[LINESIZE+1];
			int j=0;

			des=(struct tag_descriptor *)realloc(des,(++num_fields)*sizeof (struct tag_descriptor));
			struct tag_descriptor *tag=des+num_fields-1;
			bzero(tag,sizeof (struct tag_descriptor));

			while(background[i] && background[i]!='_' && j<LINESIZE)
				str[j++]=background[i++];
			str[j]=0;

			tag->x=x;
			tag->y=y;
			if(atoi(str+1))
				str_to_sla(str,tag->sla);
			else
				db->Str_To_Sla(str,tag->sla);
			if(background[i]=='[') // есть шаблон ввода
			{
				for(++i,j=0;background[i] && background[i]!=']' && background[i]!='\n' && j<LINESIZE;i++)
				{
					str[j++]=background[i];
					background[size++]=' ';
					x++;
				}
				if(j && j<LINESIZE && background[i]==']')
				{
//                                        base->blank->sxy[base->blank->ptm].mask=(char *)calloc(j+1);
//                                        bcopy(str,base->blank->sxy[base->blank->ptm].mask,j);
					i++;
					goto END;
				}
			}
//                        base->blank->sxy[base->blank->ptm].mask=NULL;
			for(j=0;background[i]=='_';x++,i++,j++)
			{
				if(tag->sla[0].n)
					background[size++]=' ';
				else
					background[size++]='_'; // ???
			}
END:
			tag->l=j;
			i+=get_atr(background+i,&tag->atr);
			i+=get_blank_color(background+i,&tag->color);
		}
		if(background[i]=='^')
		{
			i--;
			continue;
		}
		background[size++]=background[i];
		if(background[i]=='\n')
		{
			x=0;
			y++;
		}
		else if(background[i]=='\t')
			x=x+8-x%8;
		else
			x++;
	}
	background=(char *)realloc(background,size);
	background[size-1]=0;

	name=(char *)malloc(strlen(base)+strlen(FORMDB)+2);
	sprintf(name,"%s/%s",base,FORMDB);
	if(!if_base(0,name))
		create_form(base);
	else if(dpb && !yes_ans)
	{
		if(!dial(message(60),0))
		{
			delete term;
			exit(0);
		}
		yes_ans=1;
	}

	CX_BASE *form;
	try
	{
		form=new CX_BASE(name);
	}
	catch(int i)
	{
		printf("Can't open %s\n",name);
		return(1);
	}
	struct selection select;
	bzero(&select,sizeof select);
	long page;
	if(form->Select(1,blank_name,&select)>0)
	{
		page=select.index[0];
	}
	else    page=form->New_Record();
	form->Put_Slot(page,1,blank_name);
	form->Put_Slot(page,2,background);
	if(form->Num_Fields()>4)
	{
		char str[64];
		sprintf(str,"%d",form_atr);
		form->Put_Slot(page,5,str);
	}
	char *buf1;

	len=num_fields*sizeof (struct tag_descriptor)+sizeof (long);
	buf1=(char *)calloc(len,1);
	bcopy(&len,buf1,sizeof (long));
#ifdef SPARC
	conv(buf1,4);
	for(i=0;i<num_fields;i++)
	{
		conv((char *)&des[i].x,4);
		conv((char *)&des[i].y,4);
		conv((char *)&des[i].l,4);
		conv((char *)&des[i].atr,4);
		for(int j=0;j<10;j++)
			conv((char *)&des[i].sla[j],2);
		conv((char *)&des[i].color.fg,4);
		conv((char *)&des[i].color.bg,4);
	}
#endif
	memcpy(buf1+sizeof (long),des,len-sizeof (long));
	form->Write(page,3,buf1);
	free(buf1);

#ifdef SPARC
	for(i=0;i<num_panel;i++)
	{
		conv((char *)&panel[i].x,4);
		conv((char *)&panel[i].y,4);
		conv((char *)&panel[i].l,4);
		conv((char *)&panel[i].h,4);
		conv((char *)&panel[i].fg,4);
		conv((char *)&panel[i].bg,4);
		conv((char *)&panel[i].atr,4);
	}
#endif
	len=num_panel*sizeof (struct panel)+sizeof (long);
	buf1=(char *)calloc(len,1);
	memcpy(buf1,&len,sizeof (long));
#ifdef SPARC
	conv(buf1,4);
#endif
	memcpy(buf1+sizeof (long),panel,len-sizeof (long));

/*

	len=num_panel*sizeof (struct panel)+sizeof (long);
	buf1=(char *)calloc(len,1);
	memcpy(buf1,&len,sizeof (long));

	memcpy(buf1+sizeof (long),panel,len-sizeof (long));
*/
	form->Write(page,4,buf1);
	free(buf1);
	free(background);
	free(panel);
	free(des);
	free(name);
	delete(form);
	return(0);
}

static int get_atr(char *ch,long *atr)
{
	int j=0;

	if(*ch=='{')
	{
		char str[128];

		bzero(str,8);
		for(j=1;ch[j]!='}' && j<7;j++)
			str[j-1]=ch[j];
		if(*str!='0')
			*atr=atoi(str);
		else if(str[1]=='x')
			*atr=atox(str+2);
		else
			*atr=atoo(str+1);
		return(j+1);
	}
	return(0);
}
static int get_blank_color(char *ch,struct color *color)
{
	int j=0;

	color->fg=0;
	color->bg=0;
	if(*ch=='(')
	{
		char str[16];

		memset(str,0,16);
		for(j=1;ch[j] && ch[j]!=')' && j<15;j++)
			str[j-1]=ch[j];
		str_to_color(str,color);
		return(j+1);
	}
	return(0);
}
static void str_to_color(char *str,struct color *color)
{
	char *ch;

	if((ch=strchr(str,','))!=NULL)
	{
		*ch=0;

		if(*str!='0')
			color->bg=atoi(str);
		else if(str[1]=='x')
			color->bg=atox(str+2);
		else
			color->bg=atoo(str+1);

		ch++;
		if(*ch!='0')
			color->fg=atoi(ch);
		else if(ch[1]=='x')
			color->fg=atox(ch+2);
		else
			color->fg=atoo(ch+1);
	}
	else
	{
		int i;

		if(*str!='0')
			i=atoi(str);
		else if(str[1]=='x')
			i=atox(str+2);
		else
			i=atoo(str+1);
		color->fg=i&0xf;
		color->bg=(i>>4)&0xf;
	}
}

void create_form(char *name)
{
	struct st blank,tg,ds,lb;
	int i;

	char *DBNAME=(char *)malloc(strlen(name)+strlen(FORMDB)+strlen(SPACEDB)+10);
	sprintf(DBNAME,"%s/%s",name,FORMDB);
	if(!access(DBNAME,R_OK))
		return;
	bzero(&blank,sizeof (struct st));
	bzero(&tg,sizeof (struct st));
	bzero(&ds,sizeof (struct st));

	tg.ptm=sizeof(struct tag_descriptor)/4+3;
	tg.size=sizeof (struct tag_descriptor);
	tg.field=(struct field *)calloc(tg.ptm,sizeof (struct field));
	for(i=0;i<tg.ptm;i++)
	{
		tg.field[i].a=X_INTEGER;
		if(i<6)
			tg.field[i].l=2;
		else
			tg.field[i].l=4;
	}

	ds.ptm=sizeof(struct panel)/4;
	ds.size=sizeof (struct panel);
	ds.field=(struct field *)calloc(ds.ptm,sizeof (struct field));
	for(i=0;i<ds.ptm;i++)
	{
		ds.field[i].a=X_INTEGER;
		ds.field[i].l=4;
	}
	lb.ptm=sizeof(struct label)/4;
	lb.size=sizeof (struct label);
	lb.field=(struct field *)calloc(lb.ptm,sizeof (struct field));
	for(i=0;i<lb.ptm;i++)
	{
		lb.field[i].a=X_INTEGER;
		lb.field[i].l=4;
	}
	lb.field[lb.ptm-1].a=X_TEXT;

	blank.ptm=6;
	blank.size=52;

	blank.field=(struct field *)calloc(blank.ptm,sizeof (struct field));
	blank.field[0].a=X_STRING;
	blank.field[0].l=32;
	blank.field[1].a=X_TEXT;
	blank.field[1].l=4;
	blank.field[2].a=X_STRUCTURE;
	blank.field[2].l=tg.size;
	blank.field[2].m=MULTISET;
	blank.field[2].st.st=&tg;
	blank.field[3].a=X_STRUCTURE;
	blank.field[3].l=ds.size;
	blank.field[3].m=MULTISET;
	blank.field[3].st.st=&ds;
	blank.field[4].a=X_INTEGER;
	blank.field[4].l=4;
	blank.field[5].a=X_STRUCTURE;
	blank.field[5].l=lb.size;
	blank.field[5].m=MULTISET;
	blank.field[5].st.st=&lb;

	create_class(&blank,DBNAME,1);

	sprintf(DBNAME,"%s/%s/%s",name,FORMDB,SPACEDB);
	bzero(&blank,sizeof (struct st));
	blank.ptm=2;
	blank.size=8;
	blank.field=(struct field *)calloc(2,sizeof (struct field));
	blank.field[0].a=X_INTEGER;
	blank.field[0].l=4;
	blank.field[1].a=X_INTEGER;
	blank.field[1].l=4;

	create_class(&blank,DBNAME,1);
}

void Help(int,int)
{
}
