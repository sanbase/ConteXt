/*
			 DBMS ConteXt V.5.7
			  ConteXt utilites

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: lashenko@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:Create_Form.cpp
*/

#include "../CX_Browser.h"

static struct tag_descriptor *tag;
static struct tag_descriptor tag_std;
static struct panel *panel;
static int num_tag,size,num_panel;
static int line=0;
static char *buf;
static long page;
static char *name_form;
static int last_char;
static CX_BASE *db;

#define nis_graph(c)  ((unsigned char)c>=179 && (unsigned char)c<=218)

static long form_atr=0;
static unsigned char Draw_Mode=0;
static int Insert_Mode=0;

static int WhatDraw(int x, int y, int c);
static void show_form(int arg=0);
static int  set_atr(int x,int y);
static int  set_color(int x,int y);
static int  edit_tag(int x,int y);
static int  edit_box(int x,int y);
static void show_tag(int x,int y);
void shadows(int x,int y,int l,int h,int x0,int y0,int l0,int h0);
static void Write_str(CX_BASE *DG,long page);
static void relocate(int x0,int y0);

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

static void suspend_init()
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

static int choise_box(struct item *names,int size,int fg,int bg,int fb)
{
	int i,act;
	int len,f,num;

	term->cursor_invisible();
	for(num=0,len=0;num<size && *names[num].name;num++)
	{
		if(strlen(names[num].name)>len)
			len=strlen(names[num].name);
	}

	int col=(term->l_x()-2)/len;
	if(col>=32)
		col=32;
	else if(col>=16)
		col=16;
	else    if(col>=8)
		col=8;
	if(fb<0)
		act=0;
	else if(fb)
		act=bg;
	else    act=fg;
	int line=size/col+((size%col)!=0);
	f=term->get_box(0,term->l_y()-line-2,term->l_x(),line+4);
	for(;;)
	{
		char str[32];
		int x,y;

		term->put_box(0,term->l_y()-line-2,f);
		term->BOX(0,term->l_y()-line-1,col*len+2,line+2,' ',0x8,0xf,0x8,0xf);
		for(i=0;i<size;i++)
		{
			if(i==act)
			{
				if(fb>=0)
				{
					term->box2(term->l_x()/2-10,term->l_y()/2-4,20,7,'x',
					!fb?names[i].bg:fg,
					fb?names[i].bg:bg,
					!fb?names[i].bg:fg,
					fb?names[i].bg:bg);
					term->Set_Color(names[i].bg+0x100,0xf);
				}
				else
					term->Set_Color(0x10f,0);
				x=1+(i%col)*len;
				y=term->l_y()-line+i/col;
			}
			else
				term->Set_Color(names[i].bg,names[i].fg);
			term->dpp(1+(i%col)*len,term->l_y()-line+i/col);
			term->Set_Font(names[i].font);
			if(i==act && fb>=0)
			{
				char str[64],fmt[16];
				sprintf(fmt,"%%%dd",len);
				sprintf(str,fmt,names[i].bg);
				term->dps(str);
			}
			else
			{
				term->dps(names[i].name);
				term->dpn(len-strlen(names[i].name),' ');
			}
			term->Set_Font(0);
		}
		term->dpp(x,y);
		switch(Xmouse(term->dpi()))
		{
			case 0:
				if(term->ev().y>=term->l_y()-line && term->ev().y<term->l_y())
				{
					x=term->ev().x-1;
					y=term->ev().y-(term->l_y()-line);
					if(act==x/len+y*col)
						goto EXIT;
					act=x/len+y*col;
				}
				break;
			case '\t':
			case CR:
				act++;
				act%=size;
				break;
			case CU:
				if(act>=col)
					act-=col;
				act%=size;
				break;
			case CD:
				act+=col;
				act%=size;
				break;
			case CL:
				act--;
				if(act<0)
					act=size-1;
				break;
			case F10:
			case F12:
				act=-1;
				goto EXIT;
			case '\r':
				goto EXIT;
		}
	}
EXIT:
	term->put_box(0,term->l_y()-line-2,f);
	term->free_box(f);
	term->cursor_visible();
	return(act);
}

static int choise_box(char **names,int size,int fg,int bg,int fb)
{
	struct item *name;
	int i,num;

	for(num=0;*names[num];num++);
	if(num<1)
		return(0);
	num++;
	name=(struct item *)malloc(num*sizeof (struct item));
	for(i=0;i<num;i++)
	{
		name[i].name=names[i];
		name[i].fg=016;
		name[i].bg=010;
	}
	i=choise_box(name,size,fg,bg,fb);
	free(name);
	return(i);
}

static struct item fonts[]=
{
	{"Curier Plain Proportional         ", 010, 7, 0x00 },
	{"Curier Italic Proportional        ", 010, 7, 0x01 },
	{"Curier Bold Proportional          ", 010, 7, 0x02 },
	{"Curier Italic+Bold Proportional   ", 010, 7, 0x03 },
	{"Serif Plain Proportional          ", 010, 7, 0x10 },
	{"Serif Italic Proportional         ", 010, 7, 0x11 },
	{"Serif Bold Proportional           ", 010, 7, 0x12 },
	{"Serif Italic+Bold Proportional    ", 010, 7, 0x13 },
	{"SansSerif Plain Proportional      ", 010, 7, 0x20 },
	{"SansSerif Italic Proportional     ", 010, 7, 0x21 },
	{"SansSerif Bold Proportional       ", 010, 7, 0x22 },
	{"SansSerif Italic+Bold Proportional", 010, 7, 0x23 },
	{"Dialog Plain Proportional         ", 010, 7, 0x30 },
	{"Dialog Italic Proportional        ", 010, 7, 0x31 },
	{"Dialog Bold Proportional          ", 010, 7, 0x32 },
	{"Dialog Italic+Bold Proportional   ", 010, 7, 0x33 },
	{"Curier Plain Native         ", 010, 7, 0x40 },
	{"Curier Italic Native        ", 010, 7, 0x41 },
	{"Curier Bold Native          ", 010, 7, 0x42 },
	{"Curier Italic+Bold Native   ", 010, 7, 0x43 },
	{"Serif Plain Native          ", 010, 7, 0x50 },
	{"Serif Italic Native         ", 010, 7, 0x51 },
	{"Serif Bold Native           ", 010, 7, 0x52 },
	{"Serif Italic+Bold Native    ", 010, 7, 0x53 },
	{"SansSerif Plain Native      ", 010, 7, 0x60 },
	{"SansSerif Italic Native     ", 010, 7, 0x61 },
	{"SansSerif Bold Native       ", 010, 7, 0x62 },
	{"SansSerif Italic+Bold Native", 010, 7, 0x63 },
	{"Dialog Plain Native         ", 010, 7, 0x70 },
	{"Dialog Italic Native        ", 010, 7, 0x71 },
	{"Dialog Bold Native          ", 010, 7, 0x72 },
	{"Dialog Italic+Bold Native   ", 010, 7, 0x73 }

};

extern struct item *colors;

#include <dirent.h>
static int old=0;
static int edit_flg=0;
static char *folder;

static int find_slot(int x, int y)
{
	for(int i=0;i<num_tag;i++)
	{
		if(tag[i].l==0)
			continue;
		if(tag[i].x==x && tag[i].y==y)
			return(i);
	}
	return(-1);
}

static void screen_to_buf()
{
	size=0;
	int x=0;
	for(int i=1;i<term->l_y()-1;i++)
	{
		for(int j=1;j<term->l_x()-1;j++)
		{
			int slot;
//                        if((slot=find_slot(j-1,i-1))==-1)
			{
				buf=(char *)realloc(buf,++size);
				buf[size-1]=term->get_ch(j,i);
			}
/*
			else
			{
				buf=(char *)realloc(buf,size+tag[slot].l+1);
				memset(buf+size,' ',tag[slot].l);
				size+=tag[slot].l;
				j+=tag[slot].l-1;
			}
*/
		}
		while(buf[size-1]==' ')
			size--;
		buf[size++]='\n';
	}
	while(buf[size-1]=='\n')
		size--;
}

main(int argc, char **argv)
{
	char str[256];
	int fd,i,j,x,y;
	char name[256],*ch=NULL;
	CX_BASE *DB;

	*str=0;
	if(argc>=2)
		folder=argv[1];
	if(argc>2)
		page=atoi(argv[2]);
	else page=1;
	term= new Terminal();
	EditLine = new Line(term);
	term->dpp(0,0);
	term->Set_Color(0x8,0);
	suspend_init();
	bzero(&tag_std,sizeof tag_std);
	colors=(struct item *)malloc(256*sizeof( struct item));
	for(i=0;i<256;i++)
	{
		colors[i].name=(char *)malloc(4);
		sprintf(colors[i].name,"   ");
		colors[i].bg=i;
		colors[i].fg=0;
		colors[i].font=0;
	}
	if(folder==NULL || !*folder)
	{
		folder=Select_From_Dir("",if_base,"Select File",1);
		term->MultiColor(0,0,term->l_x(),term->l_y());
		if(folder==NULL)
		{
			delete term;
			exit(0);
		}
	}
	try
	{
		db=new CX_BASE(folder);
	}
	catch(...)
	{
		delete term;
		exit(-1);
	}

	sprintf(str,"%s/%s",folder,FORMDB);
	for(i=1;i<=db->Num_Fields();i++)
	{
		if(db->Field_Descr(i)->a==X_POINTER && strstr(db->Name_Subbase(i),HYPERFORM)!=NULL)
		{
			if(dial(message(40),1))
			{
				sprintf(str,"%s/%s/%s",folder,HYPERFORM,FORMDB);
				break;
			}
		}
	}
	try
	{
		DB=new CX_BASE(str);
	}
	catch(...)
	{
		struct st blank,tg,ds;

		if(!access(str,R_OK))
		{
			delete term;
			printf("Cant't open %s\n",str);
			exit(1);
		}
		bzero(&blank,sizeof (struct st));
		bzero(&tg,sizeof (struct st));
		bzero(&ds,sizeof (struct st));
		tg.ptm=sizeof(struct tag_descriptor)/4;
		tg.size=sizeof (struct tag_descriptor);
		tg.field=(struct field *)calloc(tg.ptm,sizeof (struct field));
		for(i=0;i<tg.ptm;i++)
		{
			tg.field[i].a=X_INTEGER;
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
		blank.ptm=5;
		blank.size=48;
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

		create_class(&blank,str,1);
		DB=new CX_BASE(str);
	}
	if(!*DB->Name_Base())
	{
		delete term;
		exit(-1);
	}
START:
	edit_flg=0;
	if(buf)
		free(buf);
	buf=NULL;
	if(tag)
		free(tag);
	tag=NULL;
	if(ch)
		free(ch);
	ch=NULL;
	if(!DB->Check_Del(page))
	{
		DB->Get_Slot(page,1,ch);
		strcpy(name,ch);
		name_form=name;
		free(ch);
		ch=NULL;
		size=DB->Get_Slot(page,2,buf);

		char *ch1=NULL;
		int len;

		num_tag=DB->Read(page,3,ch1).len;
		num_tag/=(len=DB->Field_Descr(3)->l);

		if(len==sizeof (struct tag_descriptor))
		{
			tag=(struct tag_descriptor *)ch1;
		}
		else
		{
			struct old_tag_descriptor *d=(struct old_tag_descriptor *)ch1;
			tag=(struct tag_descriptor *)calloc(num_tag,sizeof (struct tag_descriptor));
			for(i=0;i<num_tag;i++)
			{
				tag[i].x=d[i].x;
				tag[i].y=d[i].y;
				tag[i].l=d[i].l;
				tag[i].atr=d[i].atr;
				tag[i].color=d[i].color;
				bzero(tag[i].sla,sizeof (tag[i].sla));
				bcopy(d[i].sla,tag[i].sla,sizeof d[i].sla);
			}
			free(ch1);
			old=1;
		}


		ch1=(char *)panel;
		num_panel=DB->Read(page,4,ch1).len/(long)sizeof (struct panel);
		panel=(struct panel *)ch1;
		if(DB->Num_Fields()>4)
		{
			DB->Get_Slot(page,5,ch);

			form_atr=atoi(ch);
			free(ch);
			ch=NULL;
		}
	}
	else
	{
		num_tag=0;
		num_panel=0;
		size=0;
		if(panel)
			free(panel);
		if(tag)
			free(tag);
		if(buf)
			free(buf);
		buf=NULL;
		panel=NULL;
		tag=NULL;
		*name=0;
		name_form=name;
	}
BEGIN:
	x=y=0;
	show_form();
	for(;;)
	{
		hot_line(message(26));
		if(Draw_Mode)
			set_hot_key_status("F6",Draw_Mode==1?0x10e:0x10c,Draw_Mode==1?017:016);
		show_tag(x,y);
		term->dpp(x+1,y+1);
		switch(last_char=Xmouse(term->dpi()))
		{
			case 0:
				if(term->ev().x==term->l_x()-1)
				{
					if((term->ev().y==1) && line)
						line--;
					else if(term->ev().y==term->l_y()-2)
						line++;
					show_form(2);
					break;
				}
				x=term->ev().x-1;
				y=term->ev().y-1;
				break;

			case CU:
				if(y>0)
				{
					if(WhatDraw(x+1,y+1,last_char))
						y--;
				}
				break;
			case CD:
				if(y<term->l_y()-3)
				{
					if(WhatDraw(x+1,y+1,last_char))
						y++;
				}
				break;
			case CL:
				if(x>0)
				{
					if(WhatDraw(x+1,y+1,last_char))
						x--;
				}
				break;
			case CR:
				if(x<term->l_x()-3)
				{
					if(WhatDraw(x+1,y+1,last_char))
						x++;
				}
				break;
			case IS:
				Insert_Mode=!Insert_Mode;
				break;
			case F12:
			case F10:
				goto END;
/*
			case '\r':
			{
				char name[128];
				int fd;
				struct stat st;

				term->dpp(term->l_x()-1,term->l_y()-1);
				term->scrbufout();
//                                term->Put_Screen(1,0,0,term->l_x()+1,term->l_y()+1,120,42);
				term->Put_Screen(1,5,2,term->l_x()-20,term->l_y()-4,120,42);
				term->cursor_visible();

				for(i=0;i<num_tag;i++)
				{
					char str[64];

					if(tag[i].l==0)
						continue;
					if(tag[i].x>term->l_x() || tag[i].y-line>term->l_y() || tag[i].y<line)
						continue;
					term->Put_Object_Scr(RECT,tag[i].x+1,tag[i].y-line+1,tag[i].l,1,7);
				}
				term->scrbufout();
				sprintf(name,"/tmp/.vb%d",getpid());
				fd=creat(name,0644);
				if(buf!=NULL)
					write(fd,buf,strlen(buf));
				close(fd);
				sprintf(str,"/usr/local/bin/ned %s",name);
				Ttyreset();
				system(str);
				Ttyset();

				term->dpp(0,0); term->Set_Color(0,3); term->clean(); term->scrbufout();

				term->Set_Screen(0);
				term->Del_Screen(1);

				term->dpp(0,0);
				term->Set_Color(0,3);
				term->clean();
				term->scrbufout();
				term->Del_All_Objects(RECT);

				fd=open(name,O_RDWR);
				fstat(fd,&st);

				buf=(char *)realloc(buf,size=st.st_size+1);
				bzero(buf,size);
				read(fd,buf,st.st_size);
				close(fd);
				unlink(name);
				strcat(name,".b");
				if(!access(name,0))
					edit_flg=1;
				unlink(name);

				edit_flg=1;
				goto BEGIN;
			}
*/
			case F1:
			{
//                                if(term->Color())
//                                        term->l_x(term->l_x()-3);
				term->Del_All_Objects(RECT);
				term->Del_All_Objects(ICON);
				Show_Structure(db->type(),0,db->Short_Name(),0);
//                                if(term->Color())
//                                        term->l_x(term->l_x()+3);
				term->MultiColor(0,0,term->l_x(),term->l_y());
				show_form();
				break;
			}
			case F2:
				edit_tag(x,y);
				break;
			case F3:
				edit_box(x,y);
				break;
			case F5:
				relocate(x+1,y+1);
				show_form(2);
				break;
			case F6:
				Draw_Mode++;
				if((Draw_Mode%=3)==0)
					show_form(2);
				break;
			case EN:
				line+=term->l_y();
				show_form();
				break;
			case HM:
				if(line)
					line-=term->l_y();
				show_form();
				break;
			case PD:
				if(edit_flg)
					Write_str(DB,page);
				edit_flg=0;
				if(page<=DB->last_cadr())
					page++;
				free(buf);
				free(tag);
				buf=NULL;
				tag=NULL;
				goto WRITE;
			case PU:
				if(edit_flg)
					Write_str(DB,page);
				edit_flg=0;
				if(page>1)
					page--;
				free(buf);
				free(tag);
				buf=NULL;
				tag=NULL;
				goto WRITE;
			case F11:
				if(DB->Num_Fields()>4)
				{
					char *ch=NULL,Atr[32];
					term->dpp(0,term->l_y());
					term->Set_Color(0,03); //dpo(es);
					term->dps(message(21));
					DB->Get_Slot(page,5,ch);
					int at=atoi(ch);
					free(ch);
					sprintf(Atr,"0x%x",at);
					if(EditLine->edit(0,Atr,32,32,strlen(message(21)),term->l_y(),0)=='\r')
					{
						if(*Atr!='0')
							form_atr=atoi(Atr);
						else if(Atr[1]=='x')
							form_atr=atox(Atr+2);
						else
							form_atr=atoo(Atr+1);
						edit_flg=1;
					}
				}
			default:
				if(last_char>256)
					break;
				x=term->x_c()-1;
				y=term->y_c()-1;


				if(last_char==DEL || last_char=='\b')
				{
					show_form(1);
					if(x==0)
					{
						term->scr->deleteLine(1,y+1,term->l_x()-2,term->l_y()-y-2,1);
					}
					else
					{
						term->dpp(x+1,y+1);
						term->scr->deleteChar(term->x_c()-(last_char=='\b'),term->y_c());
						term->scr->putChar(term->l_x()-2,term->y_c(),' ');
						if(last_char=='\b')
							x--;
					}
					screen_to_buf();
					show_form(2);
				}
				else if(last_char=='\r')
				{
					if(Insert_Mode)
					{
						show_form(1);
						term->scr->insertLine(1,y+1,term->l_x()-2,term->l_y()-y-2,1);
						screen_to_buf();
						show_form(2);
					}
					else    y++;
				}
				else if(Insert_Mode)
				{
					term->scr->insertChar(x+1,y+1,last_char);
					x++;
					screen_to_buf();
					show_form(2);
				}
				else
				{
					term->scr->putChar(x+1,y+1,last_char);
					screen_to_buf();
					show_form(2);
				}
				if(x<0)
					x=0;
				if(y>=term->l_y()-3)
					y=term->l_y()-3;
				if(x>term->l_x()-3)
					x=term->l_x()-3;
				edit_flg=1;
		}
	}
END:
	term->Del_All_Objects(RECT);
	term->Del_All_Objects(ICON);
	if(edit_flg && last_char!=F10)
	{
		Write_str(DB,page);
		edit_flg=0;
	}
	if(!dial(message(59),0))
		goto BEGIN;
	delete DB;
	delete term;
	exit(0);
WRITE:
	term->Del_All_Objects(RECT);
	term->Del_All_Objects(ICON);
	if(edit_flg)
	{
		Write_str(DB,page);
		edit_flg=0;
	}
	goto START;
}

static int set_atr(int i)
{
	char str[256];

	sprintf(str,"0x%x",tag[i].atr);
	term->Set_Color(0,014);

	if(EditLine->edit(0,str,term->l_x(),term->l_x(),0,term->l_y(),0)=='\r')
	{
		if(*str!='0')
			tag[i].atr=atoi(str);
		else if(str[1]=='x')
			tag[i].atr=atox(str+2);
		else
			tag[i].atr=atoo(str+1);
		edit_flg=1;
	}
	return(0);
}

static int set_color(int i)
{
	int atr=0;
	char str[256];
	int num=256;
/*
	if(i==num_tag)  // new tag
	{
		for(i=0;i<num_panel;i++)
			if(panel[i].x==x && panel[i].y==y+line)
				break;
		if(i==num_panel)
			for(i=0;i<num_panel;i++)
				if(panel[i].x==-1)
					break;
		if(i==num_panel)
		{
			panel=(struct panel *)realloc(panel,++num_panel*sizeof (struct panel));
			bzero(panel+num_panel-1,sizeof (struct panel));
			i=num_panel-1;
			panel[i].x=-1;
			panel[i].y=y+line;
			panel[i].bg=7;
		}
		atr=1;
	}
*/
	for(;;)
	{
		show_form(2);
		hot_line(message(22));
		switch(last_char=Xmouse(term->dpi()))
		{
			case F3:
			{
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(23));
				int j=choise_box(colors,num,atr?panel[i].fg:tag[i].color.fg,atr?panel[i].bg:tag[i].color.bg,1);
				if(j!=-1 && last_char!=F10)
				{
					if(atr)
						panel[i].bg=j;
					else
						tag[i].color.bg=j;
					edit_flg=1;
				}
				break;
			}
			case F4:
			{
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(24));
				int j=choise_box(colors,num,atr?panel[i].fg:tag[i].color.fg,atr?panel[i].bg:tag[i].color.bg,0);
				if(j!=-1 && last_char!=F10)
				{
					if(atr)
						panel[i].fg=j;
					else
						tag[i].color.fg=j;
					edit_flg=1;
				}
				break;
			}
			case F10:
				return(-1);
			case '\r':
				return(0);
		}
	}
	return(0);
}

static int edit_tag(int x,int y)
{
	int i,j,new_e=0;
	char str[256];

	edit_flg=1;
	for(i=0;i<num_tag;i++)
	{
		if(y!=tag[i].y-line)
			continue;
		if(x>=tag[i].x && x<tag[i].x+tag[i].l)
			break;
	}
	if(i==num_tag)  /* new tag */
	{
		tag=(struct tag_descriptor *)realloc(tag,++num_tag*sizeof (struct tag_descriptor));
		bzero(tag+num_tag-1,sizeof (struct tag_descriptor));
		tag[i].x=x;
		tag[i].y=y+line;
		tag[i].l=1;
		new_e=1;
	}
	sla_to_str(tag[i].sla,str);
	term->dpp(0,term->l_y());
	term->dps(str);
	for(;;)
	{
		show_form(2);
		hot_line(message(28));
		term->dpp(tag[i].x+tag[i].l+1,tag[i].y-line+1);
		switch(last_char=Xmouse(term->dpi()))
		{
			case 0:
				tag[i].l=term->ev().x-tag[i].x-1;
				if(tag[i].l<0)
					tag[i].l=1;
				break;
			case CR:
				if(tag[i].x+tag[i].l<term->l_x())
					tag[i].l++;
				break;
			case CL:
				if(tag[i].l>1)
					tag[i].l--;
				break;
			case F6:
				set_atr(i);
				break;
			case F7:
				set_color(i);
				break;
			case '\r':
				term->Set_Color(0,014);

				if((j=EditLine->edit(0,str,term->l_x(),term->l_x(),0,term->l_y(),0))==F7)
				{
					struct sla sla[SLA_DEEP];

					bzero(sla,sizeof sla);
					Show_Structure(db->type(),0,db->Short_Name(),0,sla);
					bcopy(sla,tag[i].sla,sizeof tag[i].sla);
					term->MultiColor(0,0,term->l_x(),term->l_y());
				}
				else
					str_to_sla(str,tag[i].sla);

				sla_to_str(tag[i].sla,str);

				term->Set_Color(0,013);
				term->dpp(0,term->l_y()); // dpo(el);
				term->dps(str);
				bcopy(tag+i,&tag_std,sizeof tag_std);
				goto END;
			case IS:
				if(tag_std.l)
				{
					bcopy(tag_std.sla,tag[i].sla,sizeof tag_std.sla);
					tag[i].atr=tag_std.atr;
					tag[i].l=tag_std.l;
					tag[i].color=tag_std.color;
					goto END;
				}
				break;
			case F10:
				if(!new_e)
					goto END;
			case DEL:
				bcopy(tag+i+1,tag+i,(--num_tag-i)*sizeof (struct tag_descriptor));
				goto END;
		}
	}
END:
	show_form();
	return(i);
}

static void show_form(int arg)
{
	int x,y,i;
	int fg=016,bg=010;

	for(i=0;i<num_panel;i++)
	{
		if(panel[i].x==-1)
		{
			bg=panel[i].bg;
			fg=3;
			break;
		}
	}
	term->box2(0,0,term->l_x(),term->l_y(),' ',017,010,fg,bg);
	if(arg!=1)
	{
		char str[64];
		sprintf(str,"%d ",page);
		term->dpp(2,0);
		term->Set_Color(010,017);
		term->Set_Font(1,3);
		term->dps(str);
		term->dps(name_form);
		term->Set_Color(bg,fg);
		term->Set_Font(0,1);
	}
	for(y=0,x=0,i=0;i<size;i++)
	{
		if(buf[i]=='\n')
		{
			x=0;
			y++;
		}
		else if(buf[i]=='\t')
			x=x+8-x%8;
		else
		{
			if(x<term->l_x() && y<term->l_y()+line-2 && y>=line)
			{
				term->dpp(x+1,y+1-line);
				term->dpo(buf[i]);
			}
			x++;
		}
	}
	if(arg==1)
		return;
	term->Del_All_Objects(RECT);
	for(i=0;i<num_panel;i++)
	{
		int x,y,fnt=0;

		if(panel[i].atr&TABL)
			term->Set_Color(017,016);
		else
			term->Set_Color(panel[i].bg,panel[i].fg);

		if(panel[i].atr&FONT)
		{
			int clr;
			if(term->scr->Color(panel[i].x+1,panel[i].y+1).bg<3)
				clr=016;
			else    clr=0;
			term->Put_Object_Scr(RECT,panel[i].x+1,panel[i].y+1,panel[i].l,panel[i].h,clr);
			fnt=1;
		}

		for(y=panel[i].y;y<panel[i].y+panel[i].h;y++)
		{
			if(y-line<0)
				continue;
			if(y-line>term->l_y())
				break;

			for(x=panel[i].x;x<panel[i].x+panel[i].l;x++)
			{
				if(x<0 || x>=term->l_x())
					continue;
				if(!fnt)
				{
					int frame=0;
					if(x==panel[i].x)
						frame|=0x4;
					if(y==panel[i].y)
						frame|=0x1;
					if(x==panel[i].x+panel[i].l-1)
						frame|=0x8;
					if(y==panel[i].y+panel[i].h-1)
						frame|=0x2;
					term->scr->put_frame(x+1,y+1-line,frame);

					if(panel[i].atr&TABL)
					{
						term->put_bg(x+1,y+1-line,017);
						term->put_fg(x+1,y+1-line,016);
					}
					else
					{
						term->put_fg(x+1,y+1-line,panel[i].fg);
						term->put_bg(x+1,y+1-line,panel[i].bg);
					}

//                                        term->dpp(x+1,y+1-line); term->dpo(ch);
				}
				else
				{
					term->scr->put_font(x+1,y+1,(panel[i].fg<<4)+(panel[i].bg&0x3));
				}
			}
		}
		if(panel[i].atr==0)
			shadows(panel[i].x+1,panel[i].y+1-line,
			panel[i].l,panel[i].h,
			0,0,term->l_x(),term->l_y());
	}
	term->Set_Font(0,0);
	if(arg==0)
		term->Del_All_Objects(ICON);
	for(i=0;arg!=1 && i<num_tag;i++)
	{
		char str[256],*ch=NULL;
		char des[64];
		int fg=0,bg=0;

		if(tag[i].l==0)
			continue;
		if(tag[i].x>term->l_x() || tag[i].y-line>term->l_y() || tag[i].y<line)
			continue;
		sla_to_str(tag[i].sla,des);

		db->Get_Slot(1,tag[i].sla,ch);
		if(tag[i].sla->n>=db->Num_Fields())
		{
			if(db->share->color.bg || db->share->color.fg)
			{
				bg=db->share->color.bg;
				fg=db->share->color.fg;
			}
			if(db->share->font.fnt!=-1)
			{
				term->Set_Font(db->share->font.fnt,db->share->font.atr);
			}
		}
		if(tag[i].color.fg || tag[i].color.bg)
		{
			fg=tag[i].color.fg;
			bg=tag[i].color.bg;
		}
		else if(fg==0 && bg==0)
		{
			struct color color=Type_Color(db,db->Field_Descr(tag[i].sla)->a);
			fg=color.fg;
			bg=color.bg;
		}
		bg|=0x200;
		term->Set_Color(bg,fg);

		term->dpp(tag[i].x+1,tag[i].y-line+1);
		term->dpn(tag[i].l,' ');
		sprintf(str,"%s [%s] attr=%x",des,db->Name_Field(tag[i].sla),tag[i].atr);
		if(arg==0)
		{
			if(tag[i].sla->n>=db->Num_Fields())
				term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.magenta.gif",str,2,1);
			else switch(db->Field_Descr(tag[i].sla)->a)
			{
				case X_DATE:
				case X_TIME:
					term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.white.gif",str,2,1);
					break;
				case X_STRING:
					term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.yellow.gif",str,2,1);
					break;
				case X_POINTER:
				case X_VARIANT:
					term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.blue.gif",str,2,1);
					break;
				case X_INTEGER:
				case X_UNSIGNED:
				case X_FLOAT:
				case X_DOUBLE:
					term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.green.gif",str,2,1);
					break;
				case X_STRUCTURE:
					term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.purple.gif",str,2,1);
					break;
				default:
					term->Show_Image(tag[i].x-1+tag[i].l,tag[i].y-line+1,"Images/CX/dt.red.gif",str,2,1);
					break;
			}
		}

		term->dpp(tag[i].x+1,tag[i].y-line+1);

		if(ch!=NULL)
		{
			strncpy(str,ch,sizeof str-1);
			str[tag[i].l]=0;
			term->dps(str);
			free(ch);
		}
		term->dpp(tag[i].x+1,tag[i].y-line+1);
	}
}

static void show_tag(int x,int y)
{
	int i;

	term->Set_Color(010,017);
	term->dpp(2,term->l_y()-1);
	term->dpn(40,'Í');
	term->dpp(2,term->l_y()-1);
	for(i=0;i<num_tag;i++)
	{
		if(y!=tag[i].y-line)
			continue;
		if(x>=tag[i].x && x<tag[i].x+tag[i].l)
		{
			char *str=(char *)malloc(256);

			if(tag[i].l)
			{
				sla_to_str(tag[i].sla,str);
				char *ch=db->Name_Field(tag[i].sla);
				if(ch!=NULL)
				{
					int len=strlen(str)+strlen(ch)+16;
					str=(char *)realloc(str,len);
					strcat(str," ");
					strcat(str,ch);
				}
			}
			else
				sprintf(str,"box(%dx%d)",tag[i].sla[0].n,tag[i].sla[0].m);
			term->dps(str);
			free(str);
			break;
		}
	}
	for(i=0;i<num_panel;i++)
	{
		if(panel[i].x==x && panel[i].y==y)
		{
			char str[64];
			sprintf(str," panel l=%d atr=%d",panel[i].l,panel[i].atr);
			term->dps(str);
		}
	}
}

static void clean_frame(int i)
{
	int x,y;
	for(y=panel[i].y;y<panel[i].y+panel[i].h;y++)
	{
		if(y-line<0)
			continue;
		if(y-line>term->l_y())
			break;
		for(x=panel[i].x;x<panel[i].x+panel[i].l;x++)
			term->scr->put_frame(x+1,y+1-line,0);
	}
}

static int edit_box(int x,int y)
{
	int i,j,new_e=0;
	int num=256;

	edit_flg=1;
	for(i=0;i<num_panel;i++)
		if(panel[i].x==x && panel[i].y==y+line)
			break;
	if(i==num_panel)
	{
		panel=(struct panel *)realloc(panel,++num_panel*sizeof (struct panel));
		bzero(panel+num_panel-1,sizeof (struct panel));
		i=num_panel-1;
		panel[i].x=x;
		panel[i].y=y+line;
		panel[i].bg=7;
		new_e=1;
	}
	for(;;)
	{
		show_form(2);
		hot_line(message(29));
		term->dpp(x+1,y+1);
		term->Set_Color(15,0);
		term->dpo('*');
		clean_frame(i);
		switch(last_char=Xmouse(term->dpi()))
		{
			case 0:
				panel[i].l=term->ev().x-panel[i].x;
				panel[i].h=term->ev().y-panel[i].y+line;
				break;
			case CR:
				if(panel[i].x+panel[i].l<term->l_x())
					panel[i].l++;
				break;
			case CL:
				if(panel[i].l)
					panel[i].l--;
				break;
			case CD:
				if(panel[i].y-line+panel[i].h<term->l_y()-3)
					panel[i].h++;
				break;
			case CU:
				if(panel[i].h)
					panel[i].h--;
				break;
			case F2:
				panel[i].x=0;
				panel[i].l=term->l_x();
				panel[i].atr=TABL|NO_EDIT;
				show_form(2);
				return(0);
			case F3:
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(23));
				if((j=choise_box(colors,num,panel[i].fg,panel[i].bg,1))>=0)
					panel[i].bg=j;
				break;
			case F4:
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(24));
				if((j=choise_box(colors,num,panel[i].fg,panel[i].bg,0))>=0)
					panel[i].fg=j;
				break;
			case F5:
				if((j=choise_box(fonts,sizeof fonts/sizeof (struct item),07,010,-1))>=0)
				{
					panel[i].atr|=FONT;
					panel[i].fg=fonts[j].font>>4;
					panel[i].bg=(fonts[j].font&0x3);
				}
				show_form(2);
				return(0);
			case F8:
				if(panel[i].atr&NO_EDIT)
					panel[i].atr&=(~NO_EDIT);
				else    panel[i].atr|=NO_EDIT;
				break;
			case F10:
				if(!new_e)
				{
					show_form();
					return(0);
				}
			case DEL:
				bcopy(panel+i+1,panel+i,(--num_panel-i)*sizeof (struct panel));
				show_form();
				return(0);
			case '\r':
				show_form();
				return(0);
		}

	}
}


static void Write_str(CX_BASE *DB,long page)
{
	char *buf1;
	char *name=NULL;
	int i;

	if(!dial(message(47),1))
		return;
	DB->Read(page,1,name);
	if(name==NULL || !*name)
	{
		term->dpp(0,term->l_y()); term->Set_Color(0,03);
		//dpo(es);
		term->dps(message(20));
		char Name[64];
		if(name!=NULL)
			strcpy(Name,name);
		else *Name=0;
		if(EditLine->edit(0,Name,32,32,12,term->l_y(),0)=='\r')
		{
			if((i=DB->Put_Slot(page,1,Name))<0)
			{
				page=DB->New_Record();
				i=DB->Put_Slot(page,1,Name);
			}
		}
		free(name);
	}
	if(DB->Num_Fields()>4)
	{
		char Name[64];

		sprintf(Name,"%d",form_atr);
		DB->Put_Slot(page,5,Name);
	}

	i=DB->Put_Slot(page,2,buf);

	int len=num_tag*(old?sizeof (struct old_tag_descriptor):sizeof (struct tag_descriptor))+sizeof (long);
	buf1=(char *)calloc(len,1);
	memcpy(buf1,&len,sizeof (long));
#ifdef SPARC
	conv(buf1,4);
	for(i=0;i<num_tag;i++)
	{
		conv((char *)&tag[i].x,4);
		conv((char *)&tag[i].y,4);
		conv((char *)&tag[i].l,4);
		conv((char *)&tag[i].atr,4);
		conv((char *)&tag[i].color.fg,4);
		conv((char *)&tag[i].color.bg,4);
		for(int j=0;j<(old?10:SLA_DEEP);j++)
		{
			conv((char *)&tag[i].sla[j].n,2);
			conv((char *)&tag[i].sla[j].m,2);
		}
	}
#endif
	if(old)
	{
		struct old_tag_descriptor *od;
		int shift=sizeof (long);
		for(i=0;i<num_tag;i++)
		{
			struct old_tag_descriptor d;

			d.x=tag[i].x;
			d.y=tag[i].y;
			d.l=tag[i].l;
			d.atr=tag[i].atr;
			d.color=tag[i].color;
			bcopy(tag[i].sla,d.sla,sizeof d.sla);
			bcopy(&d,buf1+shift,sizeof d);
			shift+=sizeof d;
		}
	}
	else
		bcopy(tag,buf1+sizeof (long),len-sizeof (long));
	i=DB->Write(page,3,buf1);
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
	i=DB->Write(page,4,buf1);
	free(buf1);
}

static char str[LINESIZE+1];

static void relocate(int x0,int y0)
{
	int x1,y1;
	int ret;
	int x_shift,y_shift;

	x1=x0;
	y1=y0;

	term->dpp(x0,y0);
	term->Set_Color(0,15);
	hot_line(message(30));
	for(;(ret=Xmouse(term->dpi()))!=F10;)
	{
		switch(ret)
		{
			case 0:
				x1=term->ev().x;
				y1=term->ev().y;
				break;
			case CU:
				if(y1>y0)
					y1--;
				break;
			case CD:
				if(y1<term->l_y())
					++y1;
				break;
			case CR:
				if(x1<term->l_x())
					++x1;
				break;
			case CL:
				if(x1>x0)
					--x1;
				break;
			case F9:
				goto NEXT;
		}
		show_form(2);
		term->Set_Color(016,017);
		int i,j;
		for(i=y0;i<=y1;i++)
		{
			for(j=x0;j<=x1;j++)
			{
				int ch=term->get_ch(j,i);
				term->dpp(j,i); term->dpo(ch);
			}
		}

	}
	if(ret==F10)
		return;
NEXT:
	x_shift=y_shift=0;
	hot_line(message(31));
	for(;(ret=Xmouse(term->dpi()))!=F10;)
	{
		switch(ret)
		{
			case 0:
				x0=term->ev().x;
				y0=term->ev().y;
				break;
			case CU:
				if(y0+y_shift>1)
					y_shift--;
				break;
			case CD:
				if(y1+y_shift<term->l_y())
					y_shift++;
				break;
			case CR:
				if(x1+x_shift<term->l_x())
					x_shift++;
				break;
			case CL:
				if(x0+x_shift>1)
					x_shift--;
				break;
			case F5:
			case IS:
				goto END;
		}
		show_form(2);
		term->Set_Color(016,017);
		int i,j;
		for(i=y0;i<=y1;i++)
		{
			for(j=x0;j<=x1;j++)
			{
				int ch=term->get_ch(j+x_shift,i+y_shift);
				term->dpp(j+x_shift,i+y_shift); term->dpo(ch);
			}
		}
	}
END:
	if(ret==F10)
		return;
	x0--,y0--;
	for(int i=0;i<num_tag;i++)
	{
		if(tag[i].x>=x0 && (tag[i].x+tag[i].l)<=x1 && tag[i].y>=y0)
		{
			tag[i].x+=x_shift;
			tag[i].y+=y_shift;
		}
	}
	for(int i=0;i<num_panel;i++)
	{
		if(panel[i].x>=x0 && (panel[i].x+panel[i].l)<=x1 && (panel[i].y>=y0 && panel[i].y+panel[i].h<y1))
		{
			panel[i].x+=x_shift;
			panel[i].y+=y_shift;
		}
	}
	show_form();
	edit_flg=1;
}

static char mas2char(unsigned char *,unsigned char);
static int char2mas(unsigned char, unsigned char *);
static void FillNext(int i,int j,unsigned char *);

static int WhatDraw(int PozX, int PozY, int c)
{
	unsigned char CurC[4],NextC[4];
	int x=PozX;
	int y=PozY;
	int i=1;
	if(Draw_Mode==0)
	{
//                show_form(2);
		return(1);
	}
	switch(c)
	{
		case CU:
			y--; break;
		case CD:
			y++; break;
		case CR:
			x++; break;
		case CL:
			x--; break;
		default:
			return(1);
	}
	bzero(CurC,4);
	bzero(NextC,4);
	char2mas((unsigned char)term->get_ch(PozX,PozY),CurC);
	switch(c)
	{
	case CR:
		if(CurC[1]==Draw_Mode)
		{
			PozX++;
			NextC[3]=Draw_Mode;
		}
		else
		{
			i=0;
			NextC[1]=Draw_Mode;
		}
		FillNext(PozX,PozY,NextC);
		if(!NextC[0] && !NextC[2])
			NextC[(NextC[3]==Draw_Mode)?1:3]=Draw_Mode;
		if(NextC[1] && NextC[3] && NextC[1]!=NextC[3])
			NextC[1]=NextC[3]=Draw_Mode;
		break;
	case CL:
		if(CurC[3]==Draw_Mode)
		{
			PozX--;
			NextC[1]=Draw_Mode;
		}
		else
		{
			i=0;
			NextC[3]=Draw_Mode;
		}
		FillNext(PozX,PozY,NextC);
		if(!NextC[0] && !NextC[2])
			NextC[(NextC[1]==Draw_Mode)?3:1]=Draw_Mode;
		if(NextC[1] && NextC[3] && NextC[1]!=NextC[3])
			NextC[1]=NextC[3]=Draw_Mode;
		break;
	case CU:
		if(CurC[0]==Draw_Mode)
		{
			PozY--;
			NextC[2]=Draw_Mode;
		}
		else
		{
			i=0;
			NextC[0]=Draw_Mode;
		}
		FillNext(PozX,PozY,NextC);
		if(!NextC[1] && !NextC[3])
			NextC[(NextC[2]==Draw_Mode)?0:2]=Draw_Mode;
		if(NextC[0] && NextC[2] && NextC[0]!=NextC[2])
			NextC[0]=NextC[2]=Draw_Mode;
		break;
	case CD:
		if(CurC[2]==Draw_Mode)
		{
			PozY++;
			NextC[0]=Draw_Mode;
		}
		else
		{
			i=0;
			NextC[2]=Draw_Mode;
		}
		FillNext(PozX,PozY,NextC);
		if(!NextC[1] && !NextC[3])
			NextC[(NextC[0]==Draw_Mode)?2:0]=Draw_Mode;
		if(NextC[0] && NextC[2] && NextC[0]!=NextC[2])
			NextC[0]=NextC[2]=Draw_Mode;
	}
	c=(unsigned char)mas2char(NextC,Draw_Mode);
END:
	term->dpp(PozX,PozY);
	term->scr->putChar(c);

	screen_to_buf();

	show_form(2);

	if(term->get_ch(PozX,PozY)!=c)
		i=1;
	term->dpp(PozX,PozY);
	edit_flg=1;

	return(i);
}

static void FillNext(int x,int y,unsigned char *NextC)
{
	unsigned char CurC[4];
	unsigned char c;

	c=term->get_ch(x+1,y);
	char2mas(c,CurC);
	if(CurC[3] && !NextC[1] )
		NextC[1]=CurC[3];
	c=term->get_ch(x-1,y);
	char2mas(c,CurC);
	if(CurC[1] && !NextC[3])
		NextC[3]=CurC[1];
	c=term->get_ch(x,y-1);
	char2mas(c,CurC);
	if(CurC[2] && !NextC[0])
		NextC[0]=CurC[2];
	c=term->get_ch(x,y+1);
	char2mas(c,CurC);
	if(CurC[0] && !NextC[2])
		NextC[2]=CurC[0];
}

#define NUMSYMB 40

struct symb_cod
{
	unsigned char c;
	unsigned char s[4];
};
extern struct symb_cod *os;

static char mas2char(unsigned char *s,unsigned char st)
{
	int i;

	for(i=0;i<NUMSYMB;i++)
	{
		if(!memcmp(os[i].s,s,4))
			return(os[i].c);
	}
	for(i=0;i<4;i++)
		if(s[i])
			s[i]=st;
	return(mas2char(s,st));
}

static int char2mas(unsigned char c, unsigned char *s)
{
	int j;

	if(!nis_graph(c))
	{
		bzero(s,4);
		return(-1);
	}
	if(c-179 > NUMSYMB)
		return(-1);
	memcpy(s,os[c-179].s,4);
	return(0);
}

void Help(int,int)
{
}
