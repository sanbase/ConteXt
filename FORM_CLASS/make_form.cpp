/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:make_form.cpp
*/

#include "StdAfx.h" 
#include "../CX_Browser.h"

static struct tag_descriptor *tag;
static struct tag_descriptor tag_std;
static struct panel *panel;
static struct label *label;
static int size,num_tag,num_panel,num_label;
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
static int  edit_panel(int x,int y);
static int  edit_label(int x,int y);
static void show_tag(int x,int y);
void shadows(int x,int y,int l,int h,int x0,int y0,int l0,int h0);
static void Write_str(CX_BASE *DG,long page);
static void relocate(int x0,int y0);
extern Terminal *term;
extern Line *EditLine;

static void show_form(int arg)
{
	int x,y,i;
	int fg=016,bg=010;
	term->MultiColor(0,0,term->l_x(),term->l_y());
	for(i=0;i<num_panel;i++)
	{
		if(panel[i].x==-1)
		{
			bg=panel[i].bg;
			fg=3;
			break;
		}
	}
	term->box2(0,0,term->l_x(),term->l_y(),' ',arg==3?014:017,010,fg,bg);
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
		term->Set_Font(1,1);
	}
//        term->Set_Font(1,1);
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
	term->Del_All_Objects(LABEL);
	for(i=0;i<num_panel;i++)
	{
		int x,y,fnt=0;

		if(panel[i].atr&TABL)
		{
			int clr;
			if(term->scr->Color(panel[i].x+1,panel[i].y+1).bg!=15)
				clr=017;
			else    clr=0;
			int h=panel[i].h;
			if(h<1)
				h=1;
			term->Put_Object_Scr(RECT,panel[i].x+1,panel[i].y+1,panel[i].l,h,clr);
			term->Set_Color(017,016);
		}
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
				if(panel[i].atr&FONT)
				{
					term->scr->put_font(x+1,y+1,((panel[i].fg&0x7)<<4)+(panel[i].bg&0x3));
				}
				else
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
				}
/*
				else
				{
					term->scr->put_font(x+1,y+1,((panel[i].fg&0x7)<<4)+(panel[i].bg&0x3));
				}
*/
			}
		}
		if(panel[i].atr==0)
		{
			shadows(panel[i].x+1,panel[i].y+1-line,
			panel[i].l,panel[i].h,0,0,term->l_x(),term->l_y());
		}
	}
	term->Set_Font(0,0);
	if(arg==0)
		term->Del_All_Objects(ICON);

	Load_Menu(ICONSDEF,4);
	show_menu(1);

	for(i=0;i<num_label;i++)
	{
		int clr;
		term->Put_Label(label[i].x+1,label[i].y+1,label[i].l,label[i].h,label[i].text,
		label[i].fg,label[i].bg,label[i].atr,label[i].font);
		if(term->scr->Color(label[i].x+1,label[i].y+1).bg!=15)
			clr=016;
		else    clr=0;
		term->Put_Object_Scr(RECT,label[i].x+1,label[i].y+1,label[i].l,label[i].h,clr);
	}
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

		term->Set_Font(0,0);
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
		if(tag[i].sla[SLA_DEEP-1].m)
		{
			term->Set_Font(tag[i].sla[SLA_DEEP-1].m>>4,tag[i].sla[SLA_DEEP-1].m&0x3);
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
		for(int h=1;h<tag[i].h;h++)
		{
			term->dpp(tag[i].x+1,tag[i].y-line+h);
			term->dpn(tag[i].l,' ');
		}
		term->dpp(tag[i].x+1,tag[i].y-line+1);
		db->Get_Slot(1,tag[i].sla,ch);
		if(ch!=NULL)
		{
			strncpy(str,ch,sizeof str-1);
			str[tag[i].l]=0;
			term->dps(str);
			free(ch);
			ch=NULL;
		}

		term->scr->draw_frame(tag[i].x+1,tag[i].y-line+1,tag[i].l,1,1);
//                term->dpp(tag[i].x+1,tag[i].y-line+1);

		sprintf(str,"%s [%s] attr=%x",des,db->Name_Field(tag[i].sla),tag[i].atr);
		if(arg==0)
		{
			if(tag[i].sla->n>db->Num_Fields())
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

//                term->dpp(tag[i].x+1,tag[i].y-line+1);
	}
}


static void conv_form(char *form)
{
	int i,fd,name_len=0,buf_len=0,shift;
	union line
	{
		char str[16]; 
		struct field field; 
		struct header hdr; 
	} 
	line;
	char *names=NULL,*buf=NULL,*header_name;
	char *class_name;
	CX_BASE *db;

	try
	{
		db=new CX_BASE(form);
	}
	catch(...)
	{
		return;
	}
	if(db->Num_Fields()>=6)
	{
		delete db;
		return;
	}
	header_name=(char *)malloc(2*strlen(form)+20);

	class_name=form;

	bzero(&line,sizeof line);
#ifdef WIN32
	mkdir(class_name);
#else
	mkdir(class_name,0777);
#endif
	full(class_name,class_name,header_name);
	fd=open(header_name,O_RDWR);

	unlink(header_name);
	fd=creat(header_name,0644);

	struct st blank,tg,ds,lb;

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

	create_class(&blank,form,-1);

	struct st space;
	bzero(&space,sizeof (struct st));
	space.ptm=2;
	space.size=8;
	space.field=(struct field *)calloc(2,sizeof (struct field));
	space.field[0].a=X_INTEGER;
	space.field[0].l=4;
	space.field[1].a=X_INTEGER;
	space.field[1].l=4;
	char *tmp_header_name=(char *)malloc(strlen(form)+strlen(SPACEDB)+20);
	sprintf(tmp_header_name,"%s/%s",form,SPACEDB);
	if(access(tmp_header_name,R_OK))
		create_class(&space,tmp_header_name,1);
	free(tmp_header_name);
	full(class_name,ROOT,header_name);
	fd=open(header_name,O_RDONLY);

	struct root r;
	read(fd,&r,sizeof r);

	tmp_header_name=(char *)malloc(2*strlen(form)+20);
	full(class_name,"Main.tmp",tmp_header_name);
	int df=creat(tmp_header_name,0664);
	write(df,&r,sizeof r);
	buf=(char *)malloc(db->Len_Record());
	char str[8];
	bzero(str,sizeof str);
	for(;read(fd,buf,db->Len_Record())>0;)
	{
		write(df,buf,db->Len_Record());
		write(df,str,4);
		if(db->Num_Fields()==4)
			write(df,str,4);
	}
	close(fd);
	close(df);
	free(buf);
	unlink(header_name);
#ifndef WIN32
	link(tmp_header_name,header_name);
#else
	fcopy(header_name,tmp_header_name);
#endif
	unlink(tmp_header_name);
	free(header_name);
	free(tmp_header_name);
	delete db;
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

	term->Del_All_Objects(LABEL);
	term->Del_All_Objects(RECT);
	term->Del_All_Objects(ICON);
	Load_Menu(ICONSDEF,5);
	show_menu(1);
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
		int x,y;

		term->put_box(0,term->l_y()-line-2,f);
		term->BOX(0,term->l_y()-line-1,col*len+2,line+2,' ',0x8,0xf,0x8,0xf);
		for(i=0;i<size;i++)
		{
			if(i==act)
			{
				if(fb>=0)
				{
					int yb=term->l_y()/2-4;
					if(yb+8>=term->l_y()-line-1)
						yb=term->l_y()-line-1-9;
					term->box2(term->l_x()/2-10,yb,20,7,'x',
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
				sprintf(fmt,"%%%dx",len);
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
				switch(get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))
				{
					case c_Exit:
						act=-1;
						goto EXIT;
					case c_Save:
						goto EXIT;
				}
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
	clean_menu();
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
	{"Curier Plain Proportional      ", 010, 7, 0x00 },
	{"Curier Italic Proportional     ", 010, 7, 0x01 },
	{"Curier Bold Proportional       ", 010, 7, 0x02 },
	{"Curier Italic+Bold Proportional", 010, 7, 0x03 },
	{"Times Plain Proportional       ", 010, 7, 0x10 },
	{"Times Italic Proportional      ", 010, 7, 0x11 },
	{"Times Bold Proportional        ", 010, 7, 0x12 },
	{"Times Italic+Bold Proportional ", 010, 7, 0x13 },
	{"Serif Plain Proportional       ", 010, 7, 0x20 },
	{"Serif Italic Proportional      ", 010, 7, 0x21 },
	{"Serif Bold Proportional        ", 010, 7, 0x22 },
	{"Serif Italic+Bold Proportional ", 010, 7, 0x23 },
	{"Dialog Plain Proportional      ", 010, 7, 0x30 },
	{"Dialog Italic Proportional     ", 010, 7, 0x31 },
	{"Dialog Bold Proportional       ", 010, 7, 0x32 },
	{"Dialog Italic+Bold Proportional", 010, 7, 0x33 },
	{"Curier Plain Native         ", 010, 7, 0x40 },
	{"Curier Italic Native        ", 010, 7, 0x41 },
	{"Curier Bold Native          ", 010, 7, 0x42 },
	{"Curier Italic+Bold Native   ", 010, 7, 0x43 },
	{"Times Plain Native          ", 010, 7, 0x50 },
	{"Times Italic Native         ", 010, 7, 0x51 },
	{"Times Bold Native           ", 010, 7, 0x52 },
	{"Times Italic+Bold Native    ", 010, 7, 0x53 },
	{"Serif Plain Native          ", 010, 7, 0x60 },
	{"Serif Italic Native         ", 010, 7, 0x61 },
	{"Serif Bold Native           ", 010, 7, 0x62 },
	{"Serif Italic+Bold Native    ", 010, 7, 0x63 },
	{"Dialog Plain Native         ", 010, 7, 0x70 },
	{"Dialog Italic Native        ", 010, 7, 0x71 },
	{"Dialog Bold Native          ", 010, 7, 0x72 },
	{"Dialog Italic+Bold Native   ", 010, 7, 0x73 }

};

struct item *colors;

//#include <dirent.h>
int cmp(const void *a, const void *b)
{
	return(strcmp(*(char **)a,*(char **)b));
}

static int old=0;
static int edit_flg=0;

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
			if((slot=find_slot(j-1,i-1))==-1)
			{
				buf=(char *)realloc(buf,++size);
				buf[size-1]=term->get_ch(j,i);
			}
			else
			{
				buf=(char *)realloc(buf,size+tag[slot].l+1);
				memset(buf+size,' ',tag[slot].l);
				size+=tag[slot].l;
				j+=tag[slot].l-1;
			}
		}
		while(buf[size-1]==' ')
			size--;
		buf[size++]='\n';
	}
	while(buf[size-1]=='\n' || buf[size-1]==' ')
		size--;
	buf[size]=0;
}

void table_line(int y)
{
	int i;
	for(i=0;i<num_tag;i++)
	{
		if(tag[i].y==y)
			break;
	}
	if(i==num_tag)
	{
		dial(message(77),4);
		return;
	}
	for(i=0;i<num_panel;i++)
	{
		if(panel[i].atr==(TABL|NO_EDIT))
		{
			bcopy(panel+i+1,panel+i,(--num_panel-i)*sizeof (struct panel));
			break;
		}
	}
	panel=(struct panel *)realloc(panel,++num_panel*sizeof (struct panel));
	bzero(panel+num_panel-1,sizeof (struct panel));
	i=num_panel-1;
	panel[i].x=0;
	panel[i].y=y;
	panel[i].l=term->l_x()-1;
	panel[i].atr=(TABL|NO_EDIT);
	panel[i].h=0;
	panel[i].fg=0;
	panel[i].bg=0;
	edit_flg=1;
}

int Edit_Form(CX_BASE *CX_Class,long page)
{
	char str[256];
	int i,x,y;
	char name[256],*ch=NULL;
	CX_BASE *DB;
	int ff=term->get_box(0,0,term->l_x(),term->l_y());

	*str=0;

	if(page<1)
		page=1;
	term->dpp(0,0);
	term->Set_Color(0x8,0);
	term->MultiColor(0,0,term->l_x(),term->l_y());
	term->cursor_visible();

	bzero(&tag_std,sizeof tag_std);
	colors=(struct item *)malloc(256*sizeof( struct item));
	db=CX_Class;
	for(i=0;i<256;i++)
	{
		colors[i].name=(char *)malloc(4);
		sprintf(colors[i].name,"  ");
		colors[i].bg=i;
		colors[i].fg=0;
		colors[i].font=0;
	}

	sprintf(str,"%s/%s",db->Name_Base(),FORMDB);
	for(i=1;i<=db->Num_Fields();i++)
	{
		if(db->Field_Descr(i)->a==X_POINTER && strstr(db->Name_Subbase(i),HYPERFORM)!=NULL)
		{
			if(dial(message(40),1))
			{
				sprintf(str,"%s/%s/%s",db->Name_Base(),HYPERFORM,FORMDB);
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
		struct st blank,tg,ds,lb;

		if(!access(str,R_OK))
		{
			term->restore_box(ff);
			term->free_box(ff);
			return(-1);
		}
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

		create_class(&blank,str,-1);

		struct st space;
		bzero(&space,sizeof (struct st));
		space.ptm=2;
		space.size=12;
		space.field=(struct field *)calloc(2,sizeof (struct field));
		space.field[0].a=X_INTEGER;
		space.field[0].l=4;
		space.field[1].a=X_INTEGER;
		space.field[1].l=8;
		char *tmp_header_name=(char *)malloc(strlen(str)+strlen(SPACEDB)+20);
		sprintf(tmp_header_name,"%s/%s",str,SPACEDB);
		create_class(&space,tmp_header_name,1);
		free(tmp_header_name);

		DB=new CX_BASE(str);
	}
	if(!*DB->Name_Base())
	{
		term->restore_box(ff);
		term->free_box(ff);
		return(-2);
	}
	if(DB->Num_Fields()<6)
	{
		if(DB->Field_Descr(3)->l==sizeof (struct tag_descriptor))
		{
			delete DB;
			conv_form(str);
			DB=new CX_BASE(str);
		}
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
			old=1;
		}

		num_panel=DB->Read(page,4,(char *&)panel).len/(long)sizeof (struct panel);

		if(DB->Num_Fields()>4)
		{
			DB->Get_Slot(page,5,ch);
			if(ch!=NULL)
			{
				form_atr=atoi(ch);
				free(ch);
				ch=NULL;
			}
			if(DB->Num_Fields()>5)
			{
				num_label=DB->Read(page,6,(char *&)label).len/(long)sizeof (struct label);
				struct sla sla[SLA_DEEP];
				bzero(sla,sizeof sla);
				sla[0].n=6;
				sla[1].n=sizeof(struct label)/4;
				for(i=1;i<=num_label;i++)
				{
					++sla[0].m;
					label[i-1].text=NULL;
					DB->Get_Slot(page,sla,label[i-1].text);
				}
			}
		}
	}
	else
	{
		num_tag=0;
		num_panel=0;
		num_label=0;
		size=0;
		if(label)
		{
			for(i=0;i<num_label;i++)
			{
				if(label[i].text!=NULL)
				{
					free(label[i].text);
					label[i].text=NULL;
				}
			}
			free(label);
		}
		if(panel)
			free(panel);
		if(tag)
			free(tag);
		if(buf)
			free(buf);
		buf=NULL;
		panel=NULL;
		label=NULL;
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
		last_char=Xmouse(term->dpi());
M_SWITCH:
		switch(last_char)
		{
			case 0:
				switch(get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))
				{
					case c_Exit:
						if(Draw_Mode)
						{
							Draw_Mode=0;
							break;
						}
						last_char=F10;
						goto END;
					case c_Save:
						last_char=F12;
						goto END;
					case c_Struct:
						last_char=F1;
						goto M_SWITCH;
					case c_Slot:
						last_char=F2;
						goto M_SWITCH;
					case c_Panel:
						last_char=F3;
						goto M_SWITCH;
					case c_Relocate:
						last_char=F5;
						goto M_SWITCH;
					case c_Draw_S:
						Draw_Mode=1;
						break;
					case c_Draw_D:
						Draw_Mode=2;
						break;
					case c_Attr:
						last_char=F11;
						goto M_SWITCH;
					case c_Table:
						last_char=F7;
						goto M_SWITCH;
					default:
						break;
				}
				if(term->ev().x>=term->l_x()-1)
					break;
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
			case F1:
				Help(4,1);
				break;
			case F2:
				edit_tag(x,y);
				break;
			case F3:
				edit_panel(x,y);
				break;
			case F4:
				edit_label(x,y);
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
			case F7:
				table_line(y);
				show_form(2);
				break;
			case F8:
			{
				char name[128];
				int fd;
				struct stat st;

#ifndef WIN32
				term->dpp(term->l_x()-1,term->l_y()-1);
				term->scrbufout();
//                                term->Put_Screen(1,0,0,term->l_x()+1,term->l_y()+1,120,42);
				term->Put_Screen(1,5,2,term->l_x()-10,term->l_y()-4,term->l_x(),term->l_y());
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
				term->Del_All_Objects(LABEL);
				term->Del_All_Objects(RECT);

				fd=open(name,O_RDWR);
				fstat(fd,&st);

				buf=(char *)realloc(buf,size=st.st_size+1);
				bzero(buf,size);
				read(fd,buf,st.st_size);
				close(fd);
				unlink(name);
				strcat(name,".b");
				if(!access(name,R_OK))
				{
					edit_flg=1;
					unlink(name);
				}
#endif
				goto BEGIN;
			}
			case F9:
			{
SCHEMA:
				term->Del_All_Objects(LABEL);
				term->Del_All_Objects(RECT);
				term->Del_All_Objects(ICON);
				Show_Structure(db->type(),0,db->Short_Name(),0);
				term->MultiColor(0,0,term->l_x(),term->l_y());
				show_form();
				break;
			}

			case F11:
				if(DB->Num_Fields()>4)
				{
					char *ch=NULL,Atr[32];
					DB->Get_Slot(page,5,ch);
					int at=atoi(ch);
					free(ch);
					ch=NULL;
					sprintf(Atr,"0x%x",at);
					if(dial(message(21),5,NULL,-1,Atr)=='\r')
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
				show_form();
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
				if(buf)
					free(buf);
				if(tag)
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
				if(buf)
					free(buf);
				if(tag)
					free(tag);
				buf=NULL;
				tag=NULL;
				goto WRITE;
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
				else
				{
					if(Insert_Mode)
						term->scr->insertChar(x+1,y+1,last_char);
					else
						term->scr->putChar(x+1,y+1,last_char);
					x++;
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
	term->Del_All_Objects(LABEL);
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
	term->restore_box(ff);
	term->free_box(ff);
	clean_menu();

	return(0);
WRITE:
	term->Del_All_Objects(LABEL);
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

	if(dial("Attribute:",5,NULL,-1,str)=='\r')
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
	int num=256;
	int fg=-1,bg=-1;
	int ofg=tag[i].color.fg,obg=tag[i].color.bg;
	struct color color;
	for(;;)
	{
		show_form(3);
		hot_line(message(22));

		if(tag[i].color.fg || tag[i].color.bg)
		{
			fg=tag[i].color.fg;
			bg=tag[i].color.bg;
		}

		color=Type_Color(db,db->Field_Descr(tag[i].sla)->a);
		switch(last_char=Xmouse(term->dpi()))
		{
			case F3:
			{
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(23));
				int j=choise_box(colors,num,fg==-1?color.fg:fg,bg==-1?color.bg:bg,1);
				if(j!=-1 && last_char!=F10)
				{
					bg=tag[i].color.bg=j;
					if(fg==-1)
						tag[i].color.fg=color.fg;
					edit_flg=1;
				}
				break;
			}
			case F4:
			{
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(24));
				int j=choise_box(colors,num,fg==-1?color.fg:fg,bg==-1?color.bg:bg,0);
				if(j!=-1 && last_char!=F10)
				{
					fg=tag[i].color.fg=j;
					if(bg==-1)
						tag[i].color.bg=color.bg;
					edit_flg=1;
				}
				break;
			}
			case F10:
			{
				tag[i].color.bg=obg;
				tag[i].color.fg=ofg;
				return(-1);
			}
			case F12:
			{
				if(fg!=-1 || bg!=-1)
				{
					if(fg==-1)
						tag[i].color.fg=color.fg;
					if(bg==-1)
						tag[i].color.bg=color.bg;
				}
				return(0);
			}
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
		show_form(3);
		hot_line(message(28));
		term->dpp(tag[i].x+tag[i].l+1,tag[i].y-line+1);
		last_char=Xmouse(term->dpi());
M_SWITCH:
		switch(last_char)
		{
			case 0:
				switch(get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))
				{
					case c_Exit:
						last_char=F10;
						goto M_SWITCH;
					case c_Save:
						last_char=F12;
						goto M_SWITCH;
					case c_Attr:
						last_char=F6;
						goto M_SWITCH;
					case c_Table:
						last_char=F7;
						goto M_SWITCH;
					default:
						break;
				}
				if(term->ev().x>=term->l_x()-1)
					break;
				tag[i].l=term->ev().x-tag[i].x-1;

				tag[i].h=term->ev().y-tag[i].y-1;
				if(tag[i].h<0)
					tag[i].h=0;

				if(tag[i].l<0)
					tag[i].l=1;
				if(tag[i].h<=0)
					tag[i].h=1;
				break;
			case CR:
				if(tag[i].x+tag[i].l<term->l_x())
					tag[i].l++;
				break;
			case CL:
				if(tag[i].l>1)
					tag[i].l--;
				break;
			case CU:
				if(tag[i].h>1)
					tag[i].h--;
				break;
			case CD:
				tag[i].h++;
				break;
			case F5:
				if((j=choise_box(fonts,sizeof fonts/sizeof (struct item),07,010,-1))>=0)
				{
					tag[i].sla[SLA_DEEP-1].m=fonts[j].font;
				}
				break;
			case F6:
				set_atr(i);
				break;
			case F7:
				set_color(i);
				break;
			case F12:
PUT_DESCR:
				hot_line(message(75));
				j=tag[i].sla[SLA_DEEP-1].m;
				if(dial(message(74),5,NULL,-1,str)==F7)
				{
					struct sla sla[SLA_DEEP];
					bzero(sla,sizeof sla);
					term->Del_All_Objects(LABEL);
					term->Del_All_Objects(RECT);
					term->Del_All_Objects(ICON);
					Show_Structure(db->type(),0,db->Short_Name(),0,sla);
					term->MultiColor(0,0,term->l_x(),term->l_y());
					bcopy(sla,tag[i].sla,sizeof tag[i].sla);
				}
				else if(atoi(str+1)==0 && str[1]!='0')
				{
					if(dial(message(73),1))
						goto PUT_DESCR;
					if(tag[i].sla->n==0)
						bcopy(tag+i+1,tag+i,(--num_tag-i)*sizeof (struct tag_descriptor));
					goto END;
				}
				else
					str_to_sla(str,tag[i].sla);
				tag[i].sla[SLA_DEEP-1].m=j;
				sla_to_str(tag[i].sla,str);
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


static void show_tag(int x,int y)
{
	int i;

	term->Set_Color(010,017);
	term->dpp(2,term->l_y()-1);
	term->dpn(term->l_x()-4,'Í');
	term->dpp(2,term->l_y()-1);
	for(i=0;i<num_tag;i++)
	{
		if(y!=tag[i].y-line)
			continue;
		if(x>=tag[i].x && x<tag[i].x+tag[i].l)
		{
			char str[256];

			if(tag[i].l)
			{
				char str1[64];
				sla_to_str(tag[i].sla,str1);
				char *ch=db->Name_Field(tag[i].sla);
				sprintf(str," slot %s [%s] atr=0x%x font=0x%x ",str1,ch==NULL?"":ch,tag[i].atr,tag[i].sla[SLA_DEEP-1].m);

			}
			else
				sprintf(str," box(%dx%d) ",tag[i].sla[0].n,tag[i].sla[0].m);
			term->dps(str);
			break;
		}
	}
	for(i=0;i<num_panel;i++)
	{
		if(panel[i].x==x && panel[i].y==y)
		{
			char str[64];
			sprintf(str," panel l=%d atr=0x%x ",panel[i].l,panel[i].atr);
			term->dps(str);
		}
	}
	for(i=0;i<num_label;i++)
	{
		if(label[i].x==x && label[i].y==y)
		{
			char str[64];
			sprintf(str," label l=%d atr=0x%x l=%d h=%d ",label[i].l,label[i].atr,label[i].l,label[i].h);
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

static int edit_label(int x,int y)
{
	int i,j,new_e=0;
	int num=256;

	edit_flg=1;
	for(i=0;i<num_label;i++)
		if(label[i].x==x && label[i].y==y+line)
			break;
	if(i==num_label)
	{
		label=(struct label *)realloc(label,++num_label*sizeof (struct label));
		i=num_label-1;
		label[i].x=x;
		label[i].y=y+line;
		label[i].l=1;
		label[i].h=1;
		label[i].fg=016;
		label[i].bg=7;
		label[i].atr=1;
		label[i].text=0;
		label[i].font=0;
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
		last_char=Xmouse(term->dpi());
M_SWITCH:
		switch(last_char)
		{
			case 0:
				switch(get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))
				{
					case c_Exit:
						last_char=F10;
						goto M_SWITCH;
					case c_Save:
						last_char=F12;
						goto M_SWITCH;
					default:
						break;
				}
				if(term->ev().x>=term->l_x()-1)
					break;
				label[i].l=term->ev().x-label[i].x;
				label[i].h=term->ev().y-label[i].y+line;
				break;
			case CR:
				if(label[i].x+label[i].l<term->l_x())
					label[i].l++;
				break;
			case CL:
				if(label[i].l)
					label[i].l--;
				break;
			case CD:
				if(label[i].y-line+label[i].h<term->l_y()-3)
					label[i].h++;
				break;
			case CU:
				if(label[i].h)
					label[i].h--;
				break;
			case F3:
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(23));
				if((j=choise_box(colors,num,label[i].fg,label[i].bg,1))>=0)
					label[i].bg=j;
				break;
			case F4:
				term->dpp(0,0); term->Set_Color(0,7);
				term->dps(message(24));
				if((j=choise_box(colors,num,label[i].fg,label[i].bg,0))>=0)
					label[i].fg=j;
				break;
			case F5:
				if((j=choise_box(fonts,sizeof fonts/sizeof (struct item),07,010,-1))>=0)
				{
					label[i].font=fonts[j].font;
				}
				show_form(2);
				return(0);
			case F8:
				if(label[i].atr&NO_EDIT)
					label[i].atr&=(~NO_EDIT);
				else    label[i].atr|=NO_EDIT;
				break;
			case F10:
				if(!new_e)
				{
					show_form();
					return(0);
				}
			case DEL:
				term->Del_Label(label[i].x+1,label[i].y+1);
				if(label[i].text!=NULL)
					free(label[i].text);
				bcopy(label+i+1,label+i,(--num_label-i)*sizeof (struct label));
				show_form();
				return(0);
			case F12:
			{
				char name[128],str[128];
				int fd;
				struct stat st;
#ifndef WIN32
				sprintf(str,"/usr/local/bin/ned");
				if(access(str,X_OK))
					break;
				term->dpp(term->l_x()-1,term->l_y()-1);
				term->scrbufout();
				term->Put_Screen(1,10,3,term->l_x()-20,term->l_y()-7,80,25);
				term->cursor_visible();

				term->scrbufout();
				sprintf(name,"/tmp/.vb%d",getpid());
				fd=creat(name,0644);
				if(label[i].text!=NULL)
					write(fd,label[i].text,strlen(label[i].text));
				close(fd);
				sprintf(str,"/usr/local/bin/ned %s",name);
				Ttyreset();
				system(str);
				Ttyset();

				term->dpp(0,0);
				term->Set_Color(0,3);
				term->clean();
				term->scrbufout();

				term->Set_Screen(0);
				term->Del_Screen(1);

				term->dpp(0,0);
				term->Set_Color(0,3);
				term->clean();
				term->scrbufout();
				term->Del_All_Objects(LABEL);
				term->Del_All_Objects(RECT);

				fd=open(name,O_RDWR);
				fstat(fd,&st);

				label[i].text=(char *)realloc(label[i].text,st.st_size+1);
				read(fd,label[i].text,st.st_size);
				close(fd);
				unlink(name);
				strcat(name,".b");
				if(!access(name,0))
					edit_flg=1;
				unlink(name);
#endif
				show_form();
				return(0);
			}
		}
	}
}

static int edit_panel(int x,int y)
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
		last_char=Xmouse(term->dpi());
M_SWITCH:
		switch(last_char)
		{
			case 0:
				switch(get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))
				{
					case c_Exit:
						last_char=F10;
						goto M_SWITCH;
					case c_Save:
						last_char=F12;
						goto M_SWITCH;
					default:
						break;
				}
				if(term->ev().x>=term->l_x()-1)
					break;
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
			case F12:
				show_form();
				return(0);
		}

	}
}


static void Write_str(CX_BASE *DB,long page)
{
	char *buf1,*ch=NULL;
	char *name=NULL;
	int i;
	struct get_field_result res;

	if(!dial(message(47),1))
		return;
	DB->Get_Slot(page,1,name);
	if(name==NULL || !*name)
	{
		char Name[64];
		if(name!=NULL)
			strcpy(Name,name);
		else *Name=0;
		if(dial(message(20),5,NULL,-1,Name)=='\r')
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
	DB->Get_Slot(page,2,ch);
	if(buf!=NULL)
	{
		for(i=strlen(buf)-1;i;i--)
		{
			if((unsigned char)(buf[i])>' ')
			{
				break;
			}
			buf[i]=0;
		}
		char *c=buf;
		while((c=strchr(c,'\n'))!=NULL)
		{
			char *end=c-1;
			int len=0;
			while(*end==' ')
			{
				end--;
			}
			if(c!=end+1)
				bcopy(c,end+1,strlen(c));
			c=end+2;
		}

		if(strcmp(ch,buf))
			DB->Put_Slot(page,2,buf);
	}
	int len=num_tag*(old?sizeof (struct old_tag_descriptor):sizeof (struct tag_descriptor))+sizeof (long);
	buf1=(char *)calloc(len,1);
	memcpy(buf1,&len,sizeof (long));
#ifdef SPARC
	conv(buf1,4);
	for(i=0;i<num_tag;i++)
	{
		if(tag[i].l==0)
		{
			if(i<num_tag-1)
			{
				memcpy(tag+i,tag+i+1,(num_fields-i-1)*sizeof (struct tag_descriptor));
			}
			i--;
			num_tag--;
		}
	}
	for(i=0;i<num_tag;i++)
	{
		conv((char *)&tag[i].x,2);
		conv((char *)&tag[i].c,2);
		conv((char *)&tag[i].y,2);
		conv((char *)&tag[i].r,2);
		conv((char *)&tag[i].l,2);
		conv((char *)&tag[i].h,2);
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
	if(DB->Num_Elem_Array(page,3)!=num_tag)
		DB->Write(page,3,buf1);
	else
	{
		res=DB->Read(page,3,ch);
		if(bcmp(ch,tag,res.len))
			DB->Write(page,3,buf1);
	}
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
	bcopy(panel,buf1+sizeof (long),len-sizeof (long));
	if(DB->Num_Elem_Array(page,4)!=num_panel)
		DB->Write(page,4,buf1);
	else
	{
		res=DB->Read(page,4,ch);
		if(bcmp(ch,panel,res.len))
			DB->Write(page,4,buf1);
	}
	free(buf1);
	if(ch!=NULL)
		free(ch);
	struct sla sla[SLA_DEEP];
	bzero(sla,sizeof sla);
	sla[0].n=6;
	for(i=0;i<num_label;i++)
	{
		char str[64];
		sla[0].m=i+1;

		sla[1].n=1;
		sprintf(str,"%d",label[i].x);
		DB->Put_Slot(page,sla,str);
		sla[1].n=2;
		sprintf(str,"%d",label[i].y);
		DB->Put_Slot(page,sla,str);
		sla[1].n=3;
		sprintf(str,"%d",label[i].l);
		DB->Put_Slot(page,sla,str);
		sla[1].n=4;
		sprintf(str,"%d",label[i].h);
		DB->Put_Slot(page,sla,str);
		sla[1].n=5;
		sprintf(str,"%d",label[i].fg);
		DB->Put_Slot(page,sla,str);
		sla[1].n=6;
		sprintf(str,"%d",label[i].bg);
		DB->Put_Slot(page,sla,str);
		sla[1].n=7;
		sprintf(str,"%d",label[i].atr);
		DB->Put_Slot(page,sla,str);
		sla[1].n=8;
		sprintf(str,"%d",label[i].font);
		DB->Put_Slot(page,sla,str);
		sla[1].n=9;
		DB->Put_Slot(page,sla,label[i].text);
	}
	while((sla[0].m=DB->Num_Elem_Array(page,6))>num_label)
	{
		DB->Remove_Element(page,sla);
	}
	DB->Unlock(page);
}

static char str[LINESIZE+1];

static void relocate(int x0,int y0)
{
	int x1,y1;
	int ret;
	int x_shift,y_shift;

	x1=x0;
	y1=y0;

	hot_line(message(30));
	term->dpp(x0,y0);
	term->Set_Color(7,0);
	int ch=term->get_ch(x0,y0);
	term->dpo(ch);
	for(;(ret=Xmouse(term->dpi()))!=F10;)
	{
		switch(ret)
		{
			case 0:
				if(term->ev().x>=term->l_x()-1 || term->ev().y>=term->l_y()-1)
					break;
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
		term->Set_Color(7,0);
		int i,j;
		for(i=y0;i<=y1;i++)
		{
			for(j=x0;j<=x1;j++)
			{
				ch=term->get_ch(j,i);
				term->dpp(j,i);
				term->dpo(ch);
			}
		}
	}
	if(ret==F10)
		return;
NEXT:
	int f=term->get_box(x0,y0,x1-x0+1,y1-y0+1);
	x_shift=y_shift=0;
	hot_line(message(31));
	for(;(ret=Xmouse(term->dpi()))!=F10;)
	{
		switch(ret)
		{
			case 0:
				x_shift=term->ev().x-x0;
				y_shift=term->ev().y-y0;
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
		term->put_box(x0+x_shift,y0+y_shift,f);
	}
END:
	if(ret==F10)
	{
		term->free_box(f);
		return;
	}
	x0--,y0--;
	int i;
	for(i=0;i<num_tag;i++)
	{
		if(tag[i].x>=x0 && (tag[i].x+tag[i].l)<=x1 && tag[i].y>=y0)
		{
			tag[i].x+=x_shift;
			tag[i].y+=y_shift;
		}
	}
	for(i=0;i<num_panel;i++)
	{
		if(panel[i].x>=x0 && (panel[i].x+panel[i].l)<=x1+1 && (panel[i].y>=y0 && panel[i].y+panel[i].h<=y1+1))
		{
			panel[i].x+=x_shift;
			panel[i].y+=y_shift;
		}
	}
	show_form(1);

//        term->zero_box(x0,y0,x1-x0+1,y1-y0+1);
	for(i=y0+1;i<=y1+1;i++)
	{
		for(int j=x0+1;j<=x1+1;j++)
		{
			term->dpp(j,i);
			term->dpo(' ');
		}
	}

	term->put_box(x0+x_shift+1,y0+y_shift+1,f);
	screen_to_buf();
	term->free_box(f);

	term->zero_box(0,0,term->l_x(),term->l_y());

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
} os[NUMSYMB]= {
 {'³','\1','\0','\1','\0'},{'´','\1','\0','\1','\1'},{'µ','\1','\0','\1','\2'},
 {'¶','\2','\0','\2','\1'},{'·','\0','\0','\2','\1'},{'¸','\0','\0','\1','\2'},
 {'¹','\2','\0','\2','\2'},{'º','\2','\0','\2','\0'},{'»','\0','\0','\2','\2'},
 {'¼','\2','\0','\0','\2'},{'½','\2','\0','\0','\1'},{'¾','\1','\0','\0','\2'},
 {'¿','\0','\0','\1','\1'},{'À','\1','\1','\0','\0'},{'Á','\1','\1','\0','\1'},
 {'Â','\0','\1','\1','\1'},{'Ã','\1','\1','\1','\0'},{'Ä','\0','\1','\0','\1'},
 {'Å','\1','\1','\1','\1'},{'Æ','\1','\2','\1','\0'},{'Ç','\2','\1','\2','\0'},
 {'È','\2','\2','\0','\0'},{'É','\0','\2','\2','\0'},{'Ê','\2','\2','\0','\2'},
 {'Ë','\0','\2','\2','\2'},{'Ì','\2','\2','\2','\0'},{'Í','\0','\2','\0','\2'},
 {'Î','\2','\2','\2','\2'},{'Ï','\1','\2','\0','\2'},{'Ð','\2','\1','\0','\1'},
 {'Ñ','\0','\2','\1','\2'},{'Ò','\0','\1','\2','\1'},{'Ó','\2','\1','\0','\0'},
 {'Ô','\1','\2','\0','\0'},{'Õ','\0','\2','\1','\0'},{'Ö','\0','\1','\2','\0'},
 {'×','\2','\1','\2','\1'},{'Ø','\1','\2','\1','\2'},{'Ù','\1','\0','\0','\1'},
 {'Ú','\0','\1','\1','\0'},
};

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
//      int j;

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
