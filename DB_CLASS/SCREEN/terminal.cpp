#include "StdAfx.h"
 
#include "screen.h"

Terminal::Terminal()
{
	screen=(Screen **)malloc(sizeof (Screen *));
	num_screen=1;
#ifndef WIN32
	Ttyset();
#endif
	io=new IO();
	screen[0]=new Screen(io->Width(),io->Height(),io);
	scr=screen[0];
	scr->charWidth(io->cW);
	scr->charHeight(io->cH);
	scr->x=scr->y=0;
	scr->num=0;
}

Terminal::~Terminal()
{
	Del_All_Objects();

	MultiColor(0,0,io->Width(),io->Height());
	JMenu(NULL);
	Set_Color(0,7);
	Set_Font(0,0);
	cursor_visible();
	clean();
	flush();
	for(int i=0;i<num_screen;i++)
		delete screen[i];
	delete io;
#ifndef WIN32
	Ttyreset();
#endif
}

struct mouse Terminal::mouse()
{
	return(io->ev);
}
void Terminal::evSetY(int y)
{
	io->ev.y=y;
}
void Terminal::evSetX(int x)
{
	io->ev.x=x;
}
struct mouse Terminal::ev()
{
	return(io->ev);
}

int Terminal::dpi()
{
	return(scr->get());
}

void Terminal::dps(char *str, int len)
{
	scr->putString(str,len);
}

void Terminal::refresh()
{
	scr->refresh();
}

void Terminal::dpsn(unsigned char *c,int n)
{
	if(c==NULL)
		return;
	while((*c) && (--n))
	{
		if(*c<' ')
		{
			if(*c=='\n' || *c=='\t' || *c=='\r')
			{
				dpo(' ');
				c++;
			}
			else
			{
				dpo('?');
				break;
			}
		}
		else
			dpo(*c++);
	}
}

void Terminal::dpo(char ch)
{
	scr->putChar(ch);
}

void Terminal::dpp(int x, int y)
{
	scr->pos(x,y);
}

void Terminal::flush()
{
	scr->flush();
	io->flush();
}

void Terminal::Set_Color(int bg, int fg)
{
	struct clr color=S_color();

	color.fg=fg;
	color.bg=bg;
	color.atr1=((fg>>8)&03);
	color.atr2=((bg>>8)&03);
	scr->Set_Color(color);
}

void Terminal::Set_Color(struct clr color)
{
	scr->Set_Color(color);
}

void Terminal::Set_Font(unsigned char font)
{
	scr->Set_Font(font);
}

void Terminal::DrawRectangle(int color, int x, int y, int l, double h, int fill)
{
	if(fill<2)
	{
		h*=font_H();
		y*=font_H();
	}
	if(h>0)
	{
		y-=(int)h;
		if(y<0)
			y=0;
	}
	else h=-h;
	scr->Put_Object(RECT3D,fill<2?x*font_W():x,y,fill<2?l*font_W():l,(int)h,color,fill);
}

void Terminal::Put_Polygon(int x[], int y[], int num, int color, int fon)
{
	scr->Put_Object(LINE,x[0],y[0],x[1],y[1],color,fon);
	for(int i=2;i<num;i++)
		scr->Put_Object(fon==0?POLYLINE:POLYGON,x[i],y[i]);
}

struct pics *Terminal::Screen_Buf()
{
	return scr->Screen_Buf();
}

int Terminal::x_c()
{
	return(scr->currX());
}

int Terminal::y_c()
{
	return(scr->currY());
}

struct clr Terminal::S_color()
{
	return(scr->currColor());
}

int Terminal::l_x(int x)
{
	return(scr->Width(x)-1);
}

int Terminal::l_y(int y)
{
	return(scr->Height(y)-1);
}

int Terminal::font_W()
{
	return(scr->charWidth());
}

int Terminal::font_H()
{
	return(scr->charHeight());
}

int Terminal::Color()
{
	return(io->Color());
}

void Terminal::dpn(int n, char ch)
{
	for(int i=0;i<n;i++)
		dpo(ch);
}

void Terminal::scrbufout()
{
	flush();
}

void Terminal::BlackWhite(int x, int y, int l, int h)
{
	scr->black_white(x,y,l,h,1);
}

void Terminal::MultiColor(int x, int y, int l, int h)
{
	scr->black_white(x,y,l,h,0);
}

void Terminal::Set_Font(int font,int atr)
{
	font&=0x7;
	Set_Font(((font<<4)+(atr&0x3))&0xff);
}

int Terminal::get_fg(int x, int y)
{
	return(scr->Color(x,y).fg);
}

int Terminal::get_bg(int x, int y)
{
	return(scr->Color(x,y).bg);
}

unsigned char Terminal::get_ch(int x, int y)
{
	return(scr->Char(x,y).ch);
}

unsigned int Terminal::get_a1(int x,int y)
{
	return(scr->Color(x,y).atr1);
}

unsigned int Terminal::get_a2(int x,int y)
{
	return(scr->Color(x,y).atr2);
}

void Terminal::put_ch(int x, int y, char ch)
{
	scr->Set_Char(x,y,ch);
}

void Terminal::put_fg(int x, int y, int fg)
{
	struct clr color=scr->Color(x,y);
	color.fg=fg;
	scr->Color(x,y,color);
}

void Terminal::put_bg(int x, int y, int bg)
{
	struct clr color=scr->Color(x,y);
	color.bg=bg;
	scr->Color(x,y,color);
}

int Terminal::get_box(int x,int y,int l,int h)
{
	return(scr->get_box(x,y,l,h));
}

void Terminal::free_box(int n)
{
	scr->free_box(n);
}

int Terminal::restore_box(int n)
{
	return(scr->restore_box(n));
}

int Terminal::put_box(int x,int y,int n)
{
	return(scr->put_box(x,y,n));
}

int Terminal::cursor_invisible()
{
	return(scr->cursor_visible(0));
}

int Terminal::cursor_visible()
{
	return(scr->cursor_visible(1));
}

void Terminal::clean()
{
	scr->erase_screen();
}

void Terminal::clean_line()
{
	scr->erase_line();
}

int Terminal::New_Color(int num, int r, int g, int b)
{
	return(scr->New_Color(num,r,g,b));
}

void Terminal::Scroll_Up(int x, int y, int w, int h, int l)
{
	scr->deleteLine(x,y,w,h,l);
}

void Terminal::Scroll_Down(int x, int y, int w, int h, int l)
{
	scr->insertLine(x,y,w,h,l);
}

int Terminal::Put_Object(int obj,int x0,int y0,int x1,int y1,int color,int fill,int r0,int r1,char *description)
{
	return(scr->Put_Object(obj,x0,y0,x1,y1,color,fill,r0,r1,description));
}

int Terminal::Put_Object_Scr(int obj,int x0,int y0,int x1,int y1,int color,int fill,int r0,int r1,char *description)
{
	return(scr->Put_Object_Scr(obj,x0,y0,x1,y1,color,fill,r0,r1,description));
}

void Terminal::Show_Image(int x, int y, char *name, char *description,int l, int h,int transparent)
{
	scr->Show_Image(x*font_W(),y*font_H(),name,description,l*font_W(),h*font_H(),transparent);
}

void Terminal::Del_Image(int x, int y)
{
	scr->Del_Object(ICON,x*font_W(),y*font_H());
}

void Terminal::ShowDocument(char *name)
{
	scr->io->ShowDocument(name);
}
void Terminal::Frame(char *file)
{
	scr->Frame(file);
}

void Terminal::Del_All_Images()
{
	scr->Del_All_Objects(ICON);
}

void Terminal::Put_Line(int x0, int y0, int x1, int y1, int color)
{
	scr->Put_Object(LINE,x0,y0,x1,y1,color);
}

void Terminal::Put_Rect(int x, int y, int l, int h, int color, int f)
{
	scr->Put_Object(RECT,x,y,l,h,color,f);
}

void Terminal::Put_Arc(int x, int y, int l, int h, int color, int f, int r1, int r2)
{
	scr->Put_Object(ARC,x,y,l,h,color,f,r1,r2);
}

void Terminal::Set_Dimension(int x, int y, int w, int h)
{
	scr->Set_Dimension(x,y,w,h);
}

void Terminal::DrawArc(int arc,int color)
{
	scr->DrawArc(arc,color);
}

void Terminal::Put_ORect(int x, int y, int l, int h, int color, int f, int r1, int r2)
{
	scr->Put_Object(ORECT,x,y,l,h,color,f,r1,r2);
}

void Terminal::Put_Rect3D(int x, int y, int l, int h, int color, int f)
{
	scr->Put_Object(RECT3D,x,y,l,h,color,f);
}

void Terminal::Del_Rectangles()
{
	scr->Del_All_Objects(RECT3D);
}

void Terminal::Put_Oval(int x, int y, int l, int h, int color, int f)
{
	scr->Put_Object(OVAL,x,y,l,h,color,f);
}

void Terminal::DrawCircle(int x,int y,int l,int h)
{
	scr->Put_Object(OVAL,x,y,l,h,S_color().fg,0);
}

void Terminal::Put_Label(int x, int y, int l, int h, char *text, int fg_color, int bg_color, int fill, int font)
{
	scr->Put_Object_Scr(LABEL,x,y,l,h,fg_color,fill,bg_color,font,text);
}

void Terminal::Del_Label(int x, int y)
{
	scr->Del_Object_Scr(LABEL,x,y);
}

void Terminal::Create_Screen(int num, int x, int y, int w, int h, int lines, int columns)
{
	scr->New_Screen(x,y,w,h,num,lines,columns);
}

void Terminal::Del_Last_Object(int obj)
{
	scr->Del_Last_Object(obj);
}

void Terminal::Del_All_Objects(int obj)
{
	scr->Del_All_Objects(obj);
}

void Terminal::zero_box(int x,int y,int l,int h)
{
	scr->erase_area(x,y,l,h);       // ???
}

void Terminal::flush_mouse()
{
	io->ev.x=-1;
	io->ev.y=-1;
	io->ev.b=0;
}

void  Terminal::Put_Screen(int num, int x, int y, int w, int h, int lines, int columns)
{
	Set_Screen(0);
	for(int i=0;i<num_screen;i++)
		if(screen[i]->num==num)
			Del_Screen(i);

	io->New_Screen(num,x,y,w,h,lines,columns);
	io->Set_Screen(num);
	io->flush();
	io->get_dimention();

	screen=(Screen **)realloc(screen,(++num_screen)*sizeof (Screen *));
	screen[num_screen-1]=new Screen(io->Width(),io->Height(),io);
	screen[num_screen-1]->num=num;
	scr=screen[num_screen-1];

	scr->charWidth(io->ev.x);
	scr->charHeight(io->ev.y);
	scr->x=x;
	scr->y=y;
}

int  Terminal::Set_Screen(int num)
{
	for(int i=0;i<num_screen;i++)
	{
		if(screen[i]->num==num)
		{
			io->Set_Screen(num);
			io->get_dimention();
			scr=screen[i];
			return(1);
		}
	}
	return(0);
}

void Terminal::Del_Screen(int num)
{
	Set_Screen(0);
	for(int i=1;i<num_screen;i++)
	{
		if(screen[i]->num==num)
		{
			io->Del_Object(SCREEN,screen[i]->x,screen[i]->y);
			delete screen[i];
			if(num_screen>1)
				bcopy(screen+i,screen+i+i,(--num_screen-i)*sizeof (Screen *));
			scr=screen[0];
			return;
		}
	}
	return;
}

void Terminal::JMenu(char *menu)
{
	io->JMenu(menu);
}
