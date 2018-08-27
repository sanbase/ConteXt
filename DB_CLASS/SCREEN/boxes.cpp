#include "StdAfx.h" 
#include "screen.h"

void Terminal::box(int x,int y,int l,int h,char ch)
{
	box2(x,y,l,h,ch,0,0,0,0);
}

void Terminal::box1(int x,int y,int l,int h,char ch,int bord_fg,int bord_bg,int text_fg,int text_bg)
{
	register int i,j;

	h--;
	dpp(x,y);
	if(bord_fg || bord_bg) Set_Color(bord_bg,bord_fg);
	dpo('Ú');
	dpn(l-2,'Ä');
	dpo('¿');
	for(j=y+1;j<y+h;j++)
	{
		if(ch) dpp(x,j);
		for(i=x;i<x+l;i++)
			if(i==x || i==x+l-1)
			{
				dpp(i,j);
				if(bord_fg || bord_bg) Set_Color(bord_bg,bord_fg);
				dpo('³');
			}
			else if(ch)
			{
				if(text_fg || text_bg) Set_Color(text_bg,text_fg);
				dpo(ch);
			}
	}
	dpp(x,y+h);
	if(bord_fg || bord_bg) Set_Color(bord_bg,bord_fg);
	dpo('À');
	dpn(l-2,'Ä');
	dpo('Ù');
}

void Terminal::box2(int x,int y,int l,int h,char ch,int bord_fg,int bord_bg,int text_fg,int text_bg)
{
	register int i,j;

	h--;
	dpp(x,y);
	if(bord_fg || bord_bg)
		Set_Color(bord_bg,bord_fg);
	if(bord_bg!=bord_fg)
	{
		dpo('É');
		dpn(l-2,'Í');
		dpo('»');
	}
	else
		dpn(l,' ');
	for(j=y+1;j<y+h;j++)
	{
		if(ch) dpp(x,j);
		for(i=x;i<x+l;i++)
		{
			if(i==x || i==x+l-1)
			{
				dpp(i,j);
				if(bord_fg || bord_bg)
					Set_Color(bord_bg,bord_fg);
				if(bord_bg!=bord_fg)
					dpo('º');
				else    dpo(' ');
			}
			else if(ch)
			{
				if(text_fg || text_bg)
					Set_Color(text_bg,text_fg);
				dpo(ch);
			}
		}
	}
	dpp(x,y+h);
	if(bord_fg || bord_bg)
		Set_Color(bord_bg,bord_fg);
	if(bord_bg!=bord_fg)
	{
		dpo('È');
		dpn(l-2,'Í');
		dpo('¼');
	}
	else
	{
		dpn(l,' ');
	}
}

void Terminal::BOX(int x,int y,int l,int h,char ch,int bord_bg,int bord_fg,int text_bg,int text_fg)
{
	struct clr bord;
	struct clr text;

	bzero(&bord,sizeof bord);
	bzero(&text,sizeof text);
	bord.bg=bord_bg;
	bord.fg=bord_fg;
	text.bg=text_bg;
	text.fg=text_fg;
	BOX(x,y,l,h,ch,&bord,&text);
	scr->draw_frame(x,y,l,h);
}

void Terminal::BOX(int x,int y,int l,int h,char ch,struct clr *bord,struct clr *text)
{
	if(bord!=NULL)
		Set_Color(*bord);
	box2(x,y,l,h,ch,bord!=NULL?bord->fg:0,bord!=NULL?bord->bg:0,text!=NULL?text->fg:0,text!=NULL?text->bg:0);
	shadow(x,y,l,h);
	if(text!=NULL)
		Set_Color(*text);
	scr->draw_frame(x,y,l,h);
}

void Terminal::shadow(int x,int y,int l,int h)
{
	register int i,j;

	if(io->Color()==0) return;
	i=x+l;
	j=y+1;
	if(h<2)
	{
		revers(i,y);
	        revers(i+1,y);
	}
	for(;j<y+h+1;j++)
	{
		revers(i,j);
		revers(i+1,j);
	}
	x+=2;
	y+=h;
	for(i=0;i<l-2;i++)
		revers(x+i,y);
}

int Terminal::shad(int bg)
{
	if(io->Color()<256)
	{
		if(bg<8)
			return(8);
		if(bg==8)
			return(0);
		if(bg<16)
			return(bg-8);
		return(8);
	}
	if(bg<64)
		return(bg+192);
	if(bg<96)
		return(0);
	if(bg<128)
		return(8);
	if(bg<192)
		return(bg+64);
	return(0);
}

void Terminal::revers(int x,int y)
{
	struct clr color;

	color=scr->Color(x,y);
	color.fg=shad(color.fg);
	color.bg=shad(color.bg);
	scr->Color(x,y,color);
}


void Terminal::vert(int x,int y,int h)
{
	register int i;
	int ch;
	if(io->Color()>32) ch='³';
	else     ch='|';
	dpp(x,y);
	for(i=0;i<h;i++)
	{
		dpp(x,y+i);
		dpo(ch);
	}
}

void Terminal::goriz(int x,int y,int l)
{
	dpp(x,y);
	if(io->Color()>32)
		dpn(l,'Ä');
	else
		dpn(l,'-');
}

void Terminal::goriz_s(int x,int y,int l)
{
	dpp(x,y);
	if(io->Color()>32)
		dpo('Ç');
	else    dpo('+');
	goriz(x+1,y,l-1);
	dpp(x+l,y);
	if(io->Color()>32)
		dpo('¶');
	else    dpo('+');
}

void Terminal::vert_s(int x,int y,int h)
{
	dpp(x,y);
	if(io->Color()>32)
		dpo('Ñ');
	else    dpo('+');
	vert(x,y+1,h-1);
	dpp(x,y+h);
	if(io->Color()>32)
		dpo('Ï');
	else    dpo('+');
}

void Terminal::krest(int x,int y)
{
	dpp(x,y);
	if(io->Color()>32)
		dpo('Å');
	else    dpo('+');
}
