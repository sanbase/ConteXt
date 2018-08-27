#ifndef _SCREEN_H
#define _SCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
//#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define W_OK 2
#define R_OK 4
typedef __int64 dlong;
typedef int pid_t;
#include <io.h>
#include <direct.h>
#include <winsock2.h>
#include <process.h>
void bcopy(const void *s1, void *s2, size_t n);
int  bcmp(const void *s1, const void *s2, size_t n);
void bzero(void *s, size_t n);
#endif
int read_bytes(unsigned char *buf,int len=1,int t=0);

// input codes

#define F1   301
#define F2   302
#define F3   303
#define F4   304
#define F5   305
#define F6   306
#define F7   307
#define F8   308
#define F9   309
#define F10  310
#define F11  311
#define F12  312

#define CL   313       // cursor left
#define CR   314       // cursor right
#define CU   315       // cursor up
#define CD   316       // cursor down

#define PU   317       // page up
#define PD   318       // page down
#define HM   319       // home
#define EN   320       // end
#define IS   321       // insert

#define DEL    0177

#define LINE     1
#define RECT     2
#define ARC      3
#define OVAL     4
#define ORECT    5
#define RECT3D   6
#define ICON     7
#define LABEL    8
#define POLYLINE 9
#define POLYGON  10
#define SCREEN   11

void Ttyset ();
void Ttyreset();
void TtyFlush();
char *tgoto(char *CM,int x,int y);

struct clr   { unsigned char bg, fg, font, frame; unsigned char atr1:4,atr2:4,bwlt:1,x1:7;};
//struct clr   { unsigned char bg, fg, font, frame, atr1, atr2; };
struct pics  { unsigned char ch; struct clr clr;};
struct mouse { int b,x,y;};
struct object {int obj, x,y,l,h; char *text;};

struct Keys
{
	char *name; int code;
};

class Termcap
{
private:
	char *tbuf;
	char *bufc;
	int hopcount;   /* detect infinite loops in termcap, init 0 */
	char *cbuf;

	char *tdecode(char *str,char **area);
	char *tskip(char *bp);
	int tgetent(char *bufc, char *name);
	int tnchktc();
	int tnamatch(char *np);
public:
	Termcap();
	~Termcap();
	int tgetnum(char *id);
	int tgetflag(char *id);
	char *tgetstr(char *id);

};

class IO
{
	friend class Terminal;
private:
	char **cvtin;
	char **cvtout;
	unsigned char tabl[96];
	unsigned char tabl2[96];
	char *buf;
	int current_pos;
	int am;
	int read_bytes(unsigned char *buf,int len=1,int t=0);
	int l_x,l_y;
	int x_s,y_s;
	struct clr color_s;
	int num_colors;
	int rus_lat;
	int insertmode;
	int cW,cH;
	void ind_lang();
#ifdef WIN32
	SOCKET m_sockfd;
	SOCKET sock_fd;
	HANDLE Terminal_PID;
#endif
public:
	struct mouse ev;

	IO();
	~IO();
	int get();
	void get_dimention();
	void get_dimention(int *,int *);
	int Width();
	int Height();
	int Color();
	void flush();
	void sdps(char *str);
	void setPos(int x, int y);
	void setColor(int bg, int fg);
	void setColor(struct clr color);
	void putChar(char ch);

	int black_white(int x,int y,int l,int h,int atr);
	int erase_area(int x,int y,int l,int h,struct clr color);
	int insertLine(int x, int y, int w, int h, int l);
	int deleteLine(int x, int y, int w, int h, int l);
	int cursor_visible(int);
	int New_Color(int num, int r, int g, int b);
	int Show_Image(int x, int y, char *name,char *description=NULL,int l=0, int h=0,int transparent=0);
	void Set_Dimension(int x, int y, int w, int h);
	int DrawArc(int arc,int color);
	int ShowDocument(char *url);
	int Frame(char *file);
	int Put_Object(int *atrlist,char *description);
	int Put_Object_Scr(int *atrlist,char *description);
	int Del_Object(int obj,int x, int y);
	int Del_Last_Object(int obj);
	int Del_All_Objects(int obj);
	int New_Screen(int num, int x, int y, int w, int h, int lines, int columns);
	int Set_Screen(int num);
	void JMenu(char *menu=NULL);

};

struct area {unsigned char x,y,h,l; struct pics ar[1];};

class Screen
{
	friend class Terminal;
private:
	struct pics *server;
	struct pics *client;
	struct object *ob;
	int num_object;
	int x,y;
	int mX,mY;
	int cW,cH;
	struct clr S_color;
	int x_c,y_c;
	IO *io;
	int insertmode;
	struct area **region;
	int num_reg;
	int num;

	struct area *get_area(int x,int y,int l,int h);
	int put_area(int x,int y,struct area *are);

public:
	Screen(int x, int y, IO *s_io);
	~Screen();
	static int checkBounds(int value, int lower, int upper)
	{
		if(value < lower)
			return lower;
		if(value > upper)
			return upper;
		return value;
	}
	int Width(int x=0);
	int Height(int y=0);
	int charWidth(int w=0);
	int charHeight(int h=0);
	void Set_Color(struct clr color);
	void Set_Font(unsigned char font);
	void Color(int x, int y, struct clr color);
	void Set_Char(int x, int y, char ch);
	struct pics *Screen_Buf();
	struct pics Char(int x,int y);
	struct clr Color(int x,int y);
	struct clr currColor();
	int currX();
	int currY();
	int  Attr(int x,int y);
	void erase_screen();
	void erase_screen(struct clr color);
	void erase_line();
	void erase_line(struct clr color);
	void erase_area(int x,int y,int l,int h);
	void Erase_Area(int x,int y,int l,int h);
	void erase_area(int x,int y,int l,int h,struct clr color);
	void black_white(int atr);
	void black_white(int x,int y,int l,int h,int atr);
	void putChar(char ch);
	void putChar(int x, int y, char ch);
	void putChar(int x, int y, char ch, struct clr color);
	void pos(int x, int y);
	void putString(char *str,int len);
	void insertChar(int x, int y, char ch);
	void insertChar(int x, int y, char ch, struct clr color);
	void deleteChar(int x, int y);
	void insertLine(int y);
	void insertLine(int l, int n);
	void insertLine(int x, int y, int w, int h, int l);
	void deleteLine(int x, int y, int w, int h, int l);
	void deleteLine(int y);
	void deleteLine(int y, int n);
	void deleteArea(int x, int y, int w, int h);
	void refresh();

	void put_frame(int x,int y,int frame);
	void put_font(int x,int y,int font);
	void put_a1(int x,int y,int atr1);
	void put_a2(int x,int y,int atr2);

	int  get();
	void flush();
	int cursor_visible(int atr);
	int get_box(int x,int y,int l,int h);
	void free_box(int n);
	int restore_box(int n);
	int put_box(int x,int y,int n);
	int New_Color(int num, int r, int g, int b);
	void Show_Image(int x, int y, char *name, char *description=NULL,int l=0, int h=0,int transparent=0);
	void Set_Dimension(int x, int y, int w, int h);
	void DrawArc(int arc,int color);
	void Del_Object(int obj, int x, int y);
	void Del_Object_Scr(int obj, int x, int y);
	void Del_Last_Object(int obj);
	void Del_All_Objects(int obj);
	int  Put_Object(int obj,int x0,int y0,int x1=0,int y1=0,int color=0,int fill=0,int r0=0,int r1=0,char *description=NULL);
	int  Put_Object_Scr(int obj,int x0,int y0,int x1=0,int y1=0,int color=0,int fill=0,int r0=0,int r1=0,char *description=NULL);
	void Frame(char *file);
	void New_Screen(int num, int x, int y, int w, int h, int lines, int columns);
	void draw_frame(int x, int y, int l, int h,int arg=0);
};


class Terminal
{
private:
	IO *io;
	int shad(int bg);
	int num_screen;
	int S_screen;
	Screen **screen;
public:
	Screen *scr;

	Terminal();
	~Terminal();

	int x_c();
	int y_c();
	struct clr S_color();
	int l_x(int x=0);
	int l_y(int y=0);
	int font_W();
	int font_H();
	int Color();

	struct pics *Screen_Buf();

	struct mouse mouse();
	struct mouse ev();
	 void evSetX(int x);
		void evSetY(int y);
	int dpi();
	int read_bytes(unsigned char *buf,int len=1,int t=0)
	{
		return(io->read_bytes(buf,len,t));
	}
	void dps(char *str, int len=0);
	void dpsn(unsigned char *c,int n);
	void dpo(char ch);
	void dpp(int x, int y);
	void dpn(int n, char ch);
	void flush();
	void scrbufout();
	void Set_Color(int bg, int fg);
	void Set_Color(struct clr color);
	void Set_Font(unsigned char font);
	void BlackWhite(int x, int y, int l, int h);
	void MultiColor(int x, int y, int l, int h);
	void Set_Font(int font,int atr);
	void box(int x,int y,int l,int h,char ch);
	void box1(int x,int y,int l,int h,char ch,int bord_fg,int bord_bg,int text_fg,int text_bg);
	void box2(int x,int y,int l,int h,char ch,int bord_fg,int bord_bg,int text_fg,int text_bg);
	void BOX(int x,int y,int l,int h,char ch,struct clr *bord,struct clr *text);
	void BOX(int x,int y,int l,int h,char ch,int bord_fg,int bord_bg,int text_fg,int text_bg);
	void revers(int x,int y);
	void shadow(int x,int y,int l,int h);
	void vert(int x,int y,int h);
	void goriz(int x,int y,int l);
	void goriz_s(int x,int y,int l);
	void vert_s(int x,int y,int h);
	void krest(int x,int y);

	int get_fg(int x, int y);
	int get_bg(int x, int y);
	unsigned char get_ch(int x, int y);
	unsigned int get_a1(int x,int y);
	unsigned int get_a2(int x,int y);
	void put_ch(int x, int y, char ch);
	void put_fg(int x, int y, int fg);
	void put_bg(int x, int y, int bg);
	int get_box(int x,int y,int l,int h);
	void free_box(int n);
	int restore_box(int n);
	int put_box(int x,int y,int n);
	int cursor_invisible();
	int cursor_visible();
	void clean();
	void clean_line();
	void refresh();
	int New_Color(int num, int r, int g, int b);
	void Scroll_Up(int x, int y, int w, int h, int l);
	void Scroll_Down(int x, int y, int w, int h, int l);
	int Put_Object(int obj,int x0,int y0,int x1=0,int y1=0,int color=0,int fill=0,int r0=0,int r1=0,char *description=NULL);
	int Put_Object_Scr(int obj,int x0,int y0,int x1=0,int y1=0,int color=0,int fill=0,int r0=0,int r1=0,char *description=NULL);
	void Show_Image(int x, int y, char *name, char *description=NULL,int l=0, int h=0,int transparent=0);
	void Del_Image(int x, int y);
	void ShowDocument(char *url);
	void Frame(char *file);
	void Del_All_Images();
	void Put_Line(int x0, int y0, int x1, int y1, int color);
	void Put_Rect(int x, int y, int l, int h, int color, int f);
	void Put_Arc(int x, int y, int l, int h, int color, int f, int r1, int r2);
	void Set_Dimension(int x, int y, int w, int h);
	void DrawArc(int arc,int color);
	void Put_ORect(int x, int y, int l, int h, int color, int f, int r1, int r2);
	void Put_Rect3D(int x, int y, int l, int h, int color, int f);
	void DrawRectangle(int color, int x, int y, int l, double h, int fill=0);
	void Del_Rectangles();
	void Put_Oval(int x, int y, int l, int h, int color, int f);
	void DrawCircle(int x,int y,int l,int h);
	void Put_Label(int x, int y, int l, int h, char *text, int fg_color=0, int bg_color=0, int fill=0, int font=0);
	void Del_Label(int x, int y);
	void Put_Polygon(int x[], int y[], int num, int color, int fon=0);
	void Create_Screen(int num, int x, int y, int w, int h, int lines, int columns);
	void Del_Last_Object(int obj);
	void Del_All_Objects(int obj=0);
	void zero_box(int x,int y,int l,int h);
	void flush_mouse();

	void Put_Screen(int num, int x, int y, int w, int h, int lines, int columns);
	int  Set_Screen(int num);
	void Del_Screen(int num);
	void JMenu(char *menu);
};

#endif
