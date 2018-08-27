#include "StdAfx.h" 
 
#include "screen.h"

Screen::Screen(int x, int y, IO *s_io)
{
	bzero(this,sizeof (Screen));
	mX=x; mY=y;
	server=(struct pics *)calloc(mX*mY,sizeof (struct pics));
	client=(struct pics *)calloc(mX*mY,sizeof (struct pics));
	cW=11;
	cH=20;
	io=s_io;
	num_reg=0;
	bzero(&S_color,sizeof S_color);
	S_color.fg=7;
	num_object=0;
	ob=NULL;
}

Screen::~Screen()
{
	free(server);
	free(client);
	if(ob!=NULL)
	{
		for(int i=0;i<num_object;i++)
		{
			if(ob[i].text)
				free(ob[i].text);
		}
		free(ob);
	}
}
int Screen::Width(int x)
{
	if(x!=0)
		mX=x;
	return(mX);
}
int Screen::Height(int y)
{
	if(y!=0)
		mY=y;
	return(mY);
}
int Screen::charWidth(int w)
{
	if(w!=0)
		cW=w;
	return(cW);
}
int Screen::charHeight(int h)
{
	if(h!=0)
		cH=h;
	return(cH);
}
void Screen::Set_Color(struct clr color)
{
	S_color=color;
}
void Screen::Set_Font(unsigned char font)
{
	S_color.font=font;
}

void Screen::Set_Char(int x, int y, char ch)
{
	x = checkBounds(x, 0, mX-1);
	y = checkBounds(y, 0, mY-1);
	client[y*mX+x].ch=ch;
}

void Screen::Color(int x, int y, struct clr color)
{
	x = checkBounds(x, 0, mX-1);
	y = checkBounds(y, 0, mY-1);
	client[y*mX+x].clr=color;
}

struct clr Screen::currColor()
{
	return S_color;
}

int Screen::currX()
{
	return x_c;
}

int Screen::currY()
{
	return y_c;
}

int Screen::cursor_visible(int atr)
{
	return(io->cursor_visible(atr));
}

int Screen::New_Color(int num, int r, int g, int b)
{
	return(io->New_Color(num,r,g,b));
}

void Screen::Show_Image(int x, int y, char *name, char *description,int l, int h,int transparent)
{
	io->Show_Image(x,y,name,description,l,h,transparent);
}

void Screen::Set_Dimension(int x, int y, int w, int h)
{
	io->Set_Dimension(x,y,w,h);
}

void Screen::DrawArc(int arc,int color)
{
	io->DrawArc(arc,color);
}

void Screen::Del_Object_Scr(int obj, int x, int y)
{
	for(int i=0;i<num_object;i++)
	{
		if((obj==0?1:ob[i].obj==obj) && ob[i].x==x && ob[i].y==y)
		{
			num_object--;
			if(ob[i].text)
				free(ob[i].text);
			ob[i].text=NULL;
			if(i<num_object)
				bcopy(ob+i+1,ob+i,(num_object-i)*sizeof (struct object));
			if(!num_object)
			{
				free(ob);
				ob=NULL;
			}
			break;
		}
	}
	io->Del_Object(obj,x*cW,y*cH);
}
void Screen::Del_Object(int obj, int x, int y)
{
	io->Del_Object(obj,x,y);
}

void Screen::Del_Last_Object(int obj)
{
	io->Del_Last_Object(obj);
}

void Screen::Del_All_Objects(int obj)
{

	for(int i=0;i<num_object;i++)
	{
		if(ob[i].text)
			free(ob[i].text);
		ob[i].text=NULL;
	}
	if(ob!=NULL)
		free(ob);
	ob=NULL;
	num_object=0;

	io->Del_All_Objects(obj);
}

int Screen::Put_Object(int obj,int x0,int y0,int x1,int y1,int color,int fill,int r0,int r1,char *description)
{
	int atr[9];

	atr[0]=obj;
	atr[1]=x0;
	atr[2]=y0;
	atr[3]=x1;
	atr[4]=y1;
	atr[5]=color;
	atr[6]=fill;
	atr[7]=r0;
	atr[8]=r1;

	return(io->Put_Object(atr,description));
}

int Screen::Put_Object_Scr(int obj,int x0,int y0,int x1,int y1,int color,int fill,int r0,int r1,char *description)
{
	int atr[9];
	for(int i=0;i<num_object;i++)
	{
		if(ob[i].obj==obj && ob[i].x==x0 && ob[i].y==y0)
		{
			if(description!=NULL && ob[i].text!=NULL)
			{
				if(strcmp(description,ob[i].text))
					break;
			}
			return(0);
		}
	}
	atr[0]=obj;
	atr[1]=x0;
	atr[2]=y0;
	atr[3]=x1;
	atr[4]=y1;
	atr[5]=color;
	atr[6]=fill;
	atr[7]=r0;
	atr[8]=r1;

	ob=(struct object *)realloc(ob,(++num_object)*sizeof (struct object));
	ob[num_object-1].obj=obj;
	ob[num_object-1].x=x0;
	ob[num_object-1].y=y0;
	ob[num_object-1].l=x1;
	ob[num_object-1].h=y1;
	if(description!=NULL)
	{
		ob[num_object-1].text=(char *)malloc(strlen(description)+1);
		strcpy(ob[num_object-1].text,description);
	}
	else    ob[num_object-1].text=NULL;

	return(io->Put_Object_Scr(atr,description));
}

void Screen::Frame(char *file)
{
	io->Frame(file);
}

void Screen::New_Screen(int num, int x, int y, int w, int h, int lines, int columns)
{
	io->New_Screen(num,x,y,w,h,lines,columns);
}

struct pics Screen::Char(int x,int y)
{
	x = checkBounds(x, 0, mX-1);
	y = checkBounds(y, 0, mY-1);
	return(client[y*mX+x]);
}

struct pics *Screen::Screen_Buf()
{
	return client;
}


struct clr Screen::Color(int x,int y)
{
	x = checkBounds(x, 0, mX-1);
	y = checkBounds(y, 0, mY-1);
	return(client[y*mX+x].clr);
}

void Screen::erase_screen()
{
	erase_screen(S_color);
}

void Screen::erase_screen(struct clr color)
{
//        flush();

	for(int y = 0; y < mY; y++)
	{
		for(int x = 0; x < mX; x++)
		{
			client[y*mX+x].ch=' ';
			client[y*mX+x].clr=color;
		}
	}
	if(io->erase_area(0,0,mX,mY,color))
		bcopy(client,server,mX*mY*sizeof (struct pics));
}

void Screen::erase_line()
{
	erase_line(S_color);
}

void Screen::erase_line(struct clr color)
{
//        flush();
	for(int x = x_c; x < mX; x++)
	{
		client[y_c*mX+x].ch=' ';
		client[y_c*mX+x].clr=color;
	}
	if(io->erase_area(x_c,y_c,mX,1,color))
		bcopy(client,server,mX*mY*sizeof (struct pics));
}

void Screen::erase_area(int x,int y,int l,int h)
{
	erase_area(x,y,l,h,S_color);
}

void Screen::Erase_Area(int x,int y,int l,int h)
{
	struct clr color;
	bzero(&color,sizeof color);
	erase_area(x,y,l,h,color);
}

void Screen::erase_area(int x,int y,int l,int h,struct clr color)
{
//        flush();

	for(int j=y;j<y+h && j<mY;j++)
	{
		for(int i=x;i<x+l && i<mX;i++)
		{
			client[j*mX+i].ch=' ';
			client[j*mX+i].clr=color;
		}
	}
	if(io->erase_area(x,y,l,h,color))
		bcopy(client,server,mX*mY*sizeof (struct pics));
}

void Screen::black_white(int atr)
{
	S_color.bwlt=atr;
}
void Screen::black_white(int x,int y,int l,int h,int atr)
{
//        flush();

	for(int j=y;j<y+h && j<mY;j++)
	{
		for(int i=x;i<x+l && i<mX;i++)
		{
			server[j*mX+i].clr.bwlt=client[j*mX+i].clr.bwlt=atr>0;
		}
	}
	io->black_white(x,y,l,h,atr);
//                bcopy(client,server,mX*mY*sizeof (struct pics));
}

void Screen::putChar(int x, int y, char ch)
{
	putChar(x,y,ch,S_color);
}

void Screen::putChar(int x, int y, char ch, struct clr color)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	client[y*mX+x].ch  = ch;
	client[y*mX+x].clr = color;
}

void Screen::put_a1(int x,int y,int atr1)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	client[y*mX+x].clr.atr1=atr1&03;
}
void Screen::put_a2(int x,int y,int atr2)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	client[y*mX+x].clr.atr2=atr2&03;
}
void Screen::put_frame(int x,int y,int frame)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	client[y*mX+x].clr.frame=frame;
}

void Screen::put_font(int x,int y,int font)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	client[y*mX+x].clr.font=font;
}

void Screen::insertChar(int x, int y, char ch)
{
	insertChar(x,y,ch,S_color);
}

void Screen::insertChar(int x, int y, char ch, struct clr color)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	bcopy(client+y*mX+x,client+y*mX+x+1,(mX - x - 1)*sizeof (struct pics));
	putChar(x, y, ch, color);
}

void Screen::deleteChar(int x, int y)
{
	x = checkBounds(x, 0, mX - 1);
	y = checkBounds(y, 0, mY - 1);
	if(x < mX - 1)
		bcopy(client+y*mX+x+1,client+y*mX+x,(mX - x - 1)*sizeof (struct pics));
	putChar(mX - 1, y, ' ');
}

void Screen::insertLine(int y)
{
	insertLine(y, 1);
}

void Screen::insertLine(int y, int n)
{
	y = checkBounds(y, 0, mY-1);
	int bottom=mY;
	bcopy(client+y*mX,client+(y+n)*mX,(bottom-y-n)*mX*sizeof (struct pics));
	Erase_Area(0,y,mX,n);
}

void Screen::insertLine(int x, int y, int w, int h, int n)
{
	y = checkBounds(y, 0, mY-1);
	x = checkBounds(x, 0, mX-1);
	h = checkBounds(h, 0, mY-y);
	w = checkBounds(w, 0, mX-x);
	n = checkBounds(n, 0, h-1);
//        flush();
	for(int i=y+h-n-1;i>=y;i--)
		bcopy(client+(i)*mX+x,client+(i+1)*mX+x,w*sizeof (struct pics));
	for(int j=0;j<n;j++)
	{
		for(int i=x;i<x+w;i++)
		{
			client[(y+j)*mX+i].ch=' ';
			bzero(&client[(y+j)*mX+i].clr,sizeof (struct clr));
		}
	}
	if(io->insertLine(x,y,w,h,n))
		bcopy(client,server,mX*mY*sizeof (struct pics));
}

void Screen::deleteLine(int x, int y, int w, int h, int n)
{
	y = checkBounds(y, 0, mY-1);
	x = checkBounds(x, 0, mX-1);
	h = checkBounds(h, 0, mY-y);
	w = checkBounds(w, 0, mX-x);
	n = checkBounds(n, 0, h-1);
//        flush();
	for(int i=y;i<y+h-n;i++)
		bcopy(client+(i+n)*mX+x,client+(i)*mX+x,w*sizeof (struct pics));
	for(int j=0;j<n;j++)
	{
		for(int i=x;i<x+w;i++)
		{
			client[(y+h-j-1)*mX+i].ch=' ';
			bzero(&client[(y+h-j-1)*mX+i].clr,sizeof (struct clr));
		}
	}
	if(io->deleteLine(x,y,w,h,n))
		bcopy(client,server,mX*mY*sizeof (struct pics));
}

void Screen::deleteLine(int l)
{
	deleteLine(l,1);
}

void Screen::deleteLine(int l, int n)
{
	l = checkBounds(l, 0, mY-1);
	int bottom=mY;
	bcopy(client+(l+n)*mX,client+l*mX,(bottom-l-n)*mX*sizeof (struct pics));
	Erase_Area(0,bottom-l-1,mX,n);
}

void Screen::deleteArea(int c, int l, int w, int h)
{
	c = checkBounds(c, 0, mX - 1);
	l = checkBounds(l, 0, mY - 1);

	erase_area(c,l,w,h);
}

void Screen::pos(int x, int y)
{
	x_c=Screen::checkBounds(x,0,mX-1);
	y_c=Screen::checkBounds(y,0,mY-1);
}

void Screen::putString(char *str,int len)
{
	if(str==NULL)
		return;
	if(len==0)
		len=strlen(str);
	for(int i=0;i<len;i++)
	{
		putChar(str[i]);
	}
}

void Screen::putChar(char ch)
{
	unsigned char c=(unsigned char)ch;
	switch(c)
	{
	case '\r':
		x_c=0;
		break;
	case '\t':
		if(x_c>=mX)
			break;
		x_c=x_c+8-x_c%8;
		break;
	case '\b':
		x_c--;
		if(x_c<0)
			x_c=0;
		break;
	case '\n':
		if (y_c >= mY-1)
		{
			deleteLine(0);
			return;
		}
		else
			y_c++;
		break;
	default:
		if(x_c>=mX)
			break;
		if(io->Color()==0 && c>0xaf && c<0xe0)
		{
			if(c==0xb3 || c==0xba) c='|';
			else
				if(c==0xc4 || c==0xcd) c='-';
				else
					if(c>=0xb0 && c<=0xb2) c=' ';
					else    c='+';
		}

		if(x_c >= mX)
		{
			if(y_c < mY - 1)
				y_c++;
			else
				deleteLine(0);
			x_c = 0;
		}
		if (insertmode)
			insertChar(x_c,y_c,c);
		else
			putChar(x_c,y_c,c);
		x_c++;
	}
}



void Screen::flush()
{
	register struct pics *a;
	int x_s,y_s;
	struct clr c_s;

	x_s=x_c;
	y_s=y_c;
	c_s=S_color;

	for(int y=0;y<mY;y++)
	{
		register struct pics *b;

		b=&server[y*mX];

		for(int x=0;x<mX;x++,b++)
		{
			a=client+(y*mX)+x;
			if(bcmp(a,b,sizeof (struct pics)))
			{
/*
				if(a->ch==' ' && b->ch==' ')
				{
					if(a->clr.bg==b->clr.bg)
						continue;
				}
*/
				if(y==mX-1 && x==mX-1)
					break;
				io->setPos(x,y);
				io->setColor(a->clr);
				io->putChar(a->ch);
				bcopy(a,b,sizeof (struct pics));
			}
		}
	}
	io->setPos(x_s,y_s);
	io->setColor(c_s);
//        bcopy(client,server,mX*mY*sizeof (struct pics));
}

int Screen::get()
{
	flush();
	return(io->get());
}

int Screen::get_box(int x,int y,int l,int h)
{
	int re;

	for(re=0;re<num_reg;re++)
		if(region[re]==0) break;
	if(re==num_reg)
	{
		re=num_reg;
		region=(struct area  **)realloc(region,(++num_reg)*sizeof (struct area *));
	}
	if((region[re]=get_area(x,y,l,h))==NULL)
	{
		region[re]=0; return(-1);
	}
	return(re);
}
void Screen::free_box(int n)
{
	if(n<0 || n>=num_reg)
		return;
	if(region[n]!=0)
		free((void *)region[n]);
	region[n]=0;
}

int Screen::restore_box(int n)
{
	if(n<0 || n>=num_reg)
		return(-1);
	if(region[n]==0)
		return(-1);
	put_area(region[n]->x,region[n]->y,region[n]);
	return(0);
}
int Screen::put_box(int x,int y,int n)
{
	if(n<0 || n>=num_reg)
		return(-1);
	if(region[n]==0)
		return(-1);
	put_area(x,y,region[n]);
	return(0);
}

struct area *Screen::get_area(int x,int y,int l,int h)
{
	register int i,j;
	struct area *buf;


	l = checkBounds(l, 0, mX);
	y = checkBounds(y, 0, mY);

	if((buf=(struct area *)malloc(sizeof (struct area)+(l*h*(sizeof (struct pics)))))==NULL)
		return(NULL);
	for(i=0;i<h;i++)
		for(j=0;j<l;j++)
			buf->ar[i*l+j]=client[x+j+(y+i)*mX];
	buf->h=h; buf->l=l;
	buf->x=x; buf->y=y;
	return(buf);
}

int Screen::put_area(int x,int y,struct area *are)
{
	register int i,j;

	for(i=0;i<are->h;i++)
	{
		for(j=0;j<are->l;j++)
		{
			client[x+j+(y+i)*mX]=are->ar[i*are->l+j];
		}
	}
	return(0);
}

void Screen::refresh()
{
	bzero(server,mX*mY*sizeof (struct pics));
}

void Screen::draw_frame(int x, int y, int l, int h, int arg)
{
	x = checkBounds(x, 0, mX);
	y = checkBounds(y, 0, mY);
	l = checkBounds(l, 0, mX-x);
	h = checkBounds(h, 0, mY-y);
	for(int i=x;i<x+l;i++)
	{
		for(int j=y;j<y+h;j++)
		{
			int frame=0;
			if(i==x)
				frame|=0x4;
			if(i==x+l-1)
				frame|=0x8;
			if(j==y)
				frame|=0x1;
			if(j==y+h-1)
				frame|=0x2;
			if(frame!=0)
				put_frame(i,j,frame+(arg?16:0));
		}
	}
}
