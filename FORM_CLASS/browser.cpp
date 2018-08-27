/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:browser.cpp
*/
#include "StdAfx.h"
#include "../CX_Browser.h" 
extern Terminal *term; 
 
void shadows(int x,int y,int l,int h, int x0,int y0,int l0,int h0); 
 
BROWSER::BROWSER(int num, struct tag *des,int xx, int yy, int ll, int hh):FORM(num,des) 
{ 
	f=-1; 
	form_cond=0; 
	title=NULL; 
	num_forms=0;
	forms=NULL;
	create_geom(xx,yy,ll,hh); 
	restore_bg(); 
} 
BROWSER::BROWSER(char *base,struct x_form *x,int xx, int yy, int ll, int hh):FORM(base,x) 
{ 
	f=-1; 
	form_cond=0; 
	title=NULL; 
	num_forms=0;
	forms=NULL;
	create_geom(xx,yy,ll,hh); 
	restore_bg(); 
} 
BROWSER::BROWSER(char *base,long record,int xx, int yy, int ll, int hh):FORM(base,record) 
{ 
	f=-1; 
	form_cond=0; 
	title=NULL; 
	num_forms=0;
	forms=NULL;
	create_geom(xx,yy,ll,hh); 
	restore_bg(); 
} 
BROWSER::BROWSER(char *base,char *name,int xx, int yy, int ll, int hh):FORM(base,name) 
{ 
	f=-1; 
	form_cond=0; 
	title=NULL; 
	num_forms=0;
	forms=NULL;
	create_geom(xx,yy,ll,hh); 
	restore_bg(); 
} 
BROWSER::BROWSER(char *base,int xx, int yy, int ll, int hh):FORM(base) 
{ 
	f=-1; 
	form_cond=0; 
	title=NULL; 
	num_forms=0;
	forms=NULL;
	create_geom(xx,yy,ll,hh); 
	restore_bg(); 
} 
BROWSER::BROWSER(char *base,int xx, int yy, int ll, int hh,char *t):FORM(base) 
{ 
	f=-1; 
	form_cond=0; 
	title=t; 
	if(t!=NULL && ll < strlen(t))
		ll=strlen(t);
	num_forms=0;
	forms=NULL;
	create_geom(xx,yy,ll,hh); 
	restore_bg(); 
} 
 
void BROWSER::restore_bg() 
{ 
	if(f>=0) 
	{ 
		term->restore_box(f); 
		term->free_box(f); 
	} 
	f=-1; 
	if(l>0 && h>0) 
		f=term->get_box(0,0,term->l_x(),term->l_y()); 
 
} 
void BROWSER::create_geom(int xx, int yy, int ll, int hh) 
{ 
	x0=xx; 
	y0=yy; 

	if(ll<0)
	{
		if(xx==0)
			x0=(term->l_x()-width)/2;
		ll=-ll;
		l=width+2;
	}
	else
	if(width>0 && width+4<ll)
		l=width+4;
	else
		l=ll;
	if(hh<0)
	{
		if(yy==0)
			y0=(term->l_y()-lines)/2-4;
		hh=-hh;
		h=lines+2;
	}
	else
	if(lines>0 && lines+3<hh)
		h=lines+2;
	else 
		h=hh;
	for(int i=0;i<num_panel;i++)
	{ 
		if(panel[i].atr&TABL)
		{ 
			h=hh; 
			break; 
		} 
	} 
	if(l+x0>term->l_x()) 
		l=term->l_x()-x0; 
	if(h+y0>=term->l_y()) 
		h=term->l_y()-y0; 
	act_field=0; 
	line=0; 
	colon=0; 
	num_colon=0; 
	num_lines=0; 
	frame_color=15; 
} 
 
void BROWSER::clean_frames() 
{ 
	int x,y; 
	for(int i=0;i<num_panel && !(form_cond&BW);i++)
	{ 
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
} 
 
void BROWSER::Restore() 
{ 
	clean_frames(); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y());
} 
 
BROWSER::~BROWSER() 
{ 
	Restore(); 
	if(f>=0) 
	{ 
		term->restore_box(f); 
		term->free_box(f); 
	} 
} 
 
struct stat_STD 
{ 
	int x,y,l,h; 
}; 
 
static struct stat_STD STD; 
 
void repaint() 
{ 
	STD.x=-100; 
} 

void BROWSER::Show()
{ 
	int x,y,i; 
	int bg=8,fg=3; 
	if(f>=0) 
	{ 
		term->restore_box(f); 
//                term->BlackWhite(0,0,term->l_x()+1,term->l_y());
	} 
	term->MultiColor(x0,y0,l,h); 

	for(i=0;i<num_panel;i++)
	{ 
		if(panel[i].x==-1)
		{ 
			term->Set_Color(bg=panel[i].bg,fg=panel[i].fg);
			break; 
		} 
	} 
	if(i==num_panel)
	{ 
		bg=8,fg=017; 
	} 
//        if(form_cond&BW)
		bg=7,fg=0; 
 
	if(name_form.form==0 && title==NULL) 
		term->Set_Color(0,fg); 
	else 
		term->Set_Color(bg,fg); 
	term->Set_Font(0,0); 
	for(i=x0;i<x0+l;i++) 
		for(int j=y0;j<y0+h;j++) 
			term->scr->put_frame(i,j,0); 
	term->box2(x0,y0,l,h,' ',0,0,0,0); 
	term->Set_Color(bg,fg); 
	term->box2(x0,y0,l,h,0,frame_color,0,0,0); 
	if(y0+h<term->l_y()-2) 
		term->shadow(x0,y0,l,h); 

	if(title!=NULL && *title) 
	{ 
		term->Set_Color(0,017); 
		term->dpp((term->l_x()-strlen(title))/2,y0>0?y0-1:0);
		term->dps(title); 
	} 

	term->Set_Color(bg,fg); 
	term->Set_Font(1,1);
	fix_area.x=0;
	fix_area.l=30;
	for(y=0,x=0,i=0;i<size;i++) 
	{ 
		if(background[i]=='\r') 
			continue; 
		if(background[i]=='\n') 
		{ 
			x=0; 
			y++; 
		} 
		else if(background[i]=='\t') 
			x=x+8-x%8; 
		else 
		{ 
			if((x>=colon && x<colon+l-2) && y<h+line-2 && y>=line)
			{ 
				if((unsigned char)(background[i])>=' ')
				{ 
					term->dpp(x0+x-colon+1,y0+y-line+1);
					term->dpo(background[i]); 
				} 
			} 
			x++; 
		} 
	} 
	if(fix_area.l && 0)
	{
		for(y=0,x=0,i=0;i<size;i++)
		{
			if(background[i]=='\r')
				continue;
			if(background[i]=='\n')
			{
				x=0;
				y++;
			}
			else if(background[i]=='\t')
				x=x+8-x%8;
			else
			{
				if((x>=fix_area.x && x<fix_area.l) && y<h+line-2 && y>=line)
				{
					if((unsigned char)(background[i])>=' ')
					{
						term->dpp(x0+x+1,y0+y-line+1);
						term->dpo(background[i]);
					}
				}
				x++;
			}
		}
	}
	term->Set_Font(0,0); 
	for(i=0;i<num_panel && !(form_cond&BW);i++)
	{ 
		int x,y; 
 
		for(y=panel[i].y;y<panel[i].y+panel[i].h;y++)
		{ 
			if(y-line<0) 
				continue; 
			if(y-line>=h-2) 
				break; 

			for(x=panel[i].x;x<panel[i].x+panel[i].l;x++)
			{ 
				if(x<colon || x>=colon+l-2) 
				{
					continue; 
				}
				if(panel[i].atr&FONT)
				{ 
					term->scr->put_font(x0+x+1-colon,y0+y+1-line,((panel[i].fg&0x7)<<4)+(panel[i].bg&03));
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

					term->scr->put_frame(x0+x+1-colon,y0+y+1-line,frame);

					term->put_fg(x0+x+1-colon,y0+y+1-line,panel[i].fg);
					term->put_bg(x0+x+1-colon,y0+y+1-line,panel[i].bg);
				} 
			} 
		} 
/*
		if(!(panel[i].atr&FONT))
		{
			term->scr->draw_frame(panel[i].x+1+x0-colon,panel[i].y+1+y0-line,panel[i].l,panel[i].h);
		}
*/
		if(!(panel[i].atr & NO_SHADOW) && !(panel[i].atr &FONT))
			shadows(x0+panel[i].x+1-colon,y0+panel[i].y+1-line,
			panel[i].l,panel[i].h,
			x0,y0,l-1,h-1); 
	} 
	for(i=0;i<num_label;i++)
	{
/*
		term->Put_Label(label[i].x+1+x0-colon,label[i].y+1+y0-line,label[i].l,label[i].h,label[i].text,
		label[i].fg,label[i].bg,label[i].atr,label[i].font);
*/
	}

	for(i=0;i<num_fields;i++) 
	{ 
		int fg,bg; 
 
		if(tags[i].des.l==0) 
			continue; 
		if(tags[i].des.x-colon>=l || tags[i].des.y-line>=h-2 || tags[i].des.y<line) 
			continue; 
		if(colon>tags[i].des.x+tags[i].des.l) 
			continue; 
 
		bg=(tags[i].des.color.bg||tags[i].des.color.fg)?tags[i].des.color.bg:tags[i].color.bg; 
		fg=(tags[i].des.color.bg||tags[i].des.color.fg)?tags[i].des.color.fg:tags[i].color.fg; 
 
		if(i==act_field) 
			bg=15|0x100,fg=0;
		else if(form_cond&TABLE && tags[i].des.y==tags[act_field].des.y)
			bg=0x115,fg=15; 
		else if(!(bg&0x300)) 
		{ 
			if(form_cond&MAP) 
				bg+=0x100; 
			else 
				bg+=0x200; 
		} 
		Draw_Slot(i,bg,fg,tags[i].font);
	} 

	if(tags==NULL || act_field>=num_fields || tags[act_field].des.x<colon || tags[act_field].des.x-colon>=l || tags[act_field].des.y-line>=h-2 || tags[act_field].des.y<line) 
		return; 
//        int xx=get_reg(f)->x; 
//        int yy=get_reg(f)->y; 
//        int ll=get_reg(f)->l; 
//        int hh=get_reg(f)->h; 
 
	if((STD.x!=x0 || STD.y!=y0 || STD.l!=l  || STD.h!=h)) 
	{ 
//                term->BlackWhite(0,0,term->l_x()+1,term->l_y());
		term->MultiColor(STD.x=x0,STD.y=y0,STD.l=l,STD.h=h); 
		term->MultiColor(x0,term->l_y(),l,1); // help line 
		if(title!=NULL && *title) 
			term->MultiColor((term->l_x()-strlen(title))/2,y0>0?y0-1:0,strlen(title),1); // title line 
	} 
	term->dpp(x0+tags[act_field].des.x-colon+1,y0+tags[act_field].des.y-line+1); 
} 
 
int BROWSER::Move() 
{ 
	int kod; 
 
	for(Show();;Show()) 
	{ 
		kod=_move(0); 
		if(kod) 
			return(kod); 
	} 
} 

int BROWSER::_move(int kod) 
{ 
	int i; 
 
	if(kod==0) 
		kod=Xmouse(term->dpi()); 
	switch(kod) 
	{ 
		case 0: 

		case -3:
			switch(get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))
			{
				case c_Exit:
					return(F10);
			}
			if(term->ev().b!=1) 
				return('\n'); 
			if(term->ev().x==x0+l-1) 
			{ 
				if((term->ev().y==y0+1) && line) 
				{ 
					line--; 
					act_field=find_pos(-1); 
				} 
				else if(term->ev().y==y0+h-2) 
				{ 
					line++; 
					act_field=find_pos(1); 
				} 
				break; 
			} 
			if(term->ev().y==y0+h-1) 
			{ 
				if((term->ev().x==x0+1) && colon) 
				{ 
					colon--; 
					act_field=find_pos(-2); 
				} 
				else if(term->ev().x==x0+l-2) 
				{ 
					colon++; 
					act_field=find_pos(2); 
				} 
				break; 
			} 
			for(i=0;i<num_fields;i++) 
			{ 
				if(term->ev().y==y0+tags[i].des.y-line+1 && term->ev().x>x0+tags[i].des.x-colon && term->ev().x<=x0+tags[i].des.x-colon+tags[i].des.l && pos_able(i))
				{ 
					if(form_cond&EDIT && form_cond&TABLE && tags[i].des.y!=tags[act_field].des.y) 
					{ 
						return(-3); 
					} 
					if(i==act_field)        // double click 
						return('\r'); 
					act_field=i; 
					break; 
				} 
			} 
			break; 
		case CU: 
		case CD: 
			if(form_cond&EDIT && form_cond&TABLE) 
				return(-kod); 
		case '\t': 
		case CR: 
		case CL: 
			if(!num_fields) 
				break; 
			i=find_pos(kod); 
			if(i<0) 
				return(i); 
			act_field=i; 
			if(tags[act_field].des.y<line || tags[act_field].des.y>h+line-3) 
				line=(h-2)*(tags[act_field].des.y/(h-2)); 
			if(tags[act_field].des.x<colon || tags[act_field].des.x+tags[act_field].des.l/2>l+colon-2) 
			{ 
				if((colon=tags[act_field].des.x-1)<5) 
					colon=0; 
			} 
			break; 
		case EN: 
			line+=h-1;
			act_field=find_pos(-1);
			break; 
		case HM: 
			if(line>=h-1) 
				line-=h-1; 
			else    line=0; 
			act_field=find_pos(1);
			break; 
		default: 
			return(kod); 
	} 
	return(0); 
} 
 
int BROWSER::find_pos(int kod) 
{ 
	int i=act_field; 
	int min=1000; 
	int j=-1,n; 
	struct tag *tag; 
	tag=tags; 
	switch(kod) 
	{ 
		case '\t': 
			for(j=i+1;!pos_able(j);j++) 
				if(j==num_fields) 
					j=0; 
			if(tag[j].des.y-line>=h-2 || tag[j].des.y<line) 
				return(act_field); 
			return(j); 
		case CR: 
			for(i=0;i<num_fields;i++) 
			{ 
				if(tag[act_field].des.y==tag[i].des.y && tag[i].des.l && pos_able(i))
				{ 
					int dif; 
 
					if((dif=tag[i].des.x-tag[act_field].des.x)>0 && dif<min) 
					{ 
						min=dif; 
						j=i; 
					} 
				} 
			} 
			if(j<0 || tag[j].des.y-line>=h-2 || tag[j].des.y<line) 
			{ 
				return(act_field); 
			} 
			return(j); 
		case CL: 
			for(i=0;i<num_fields;i++) 
			{ 
				if(tag[act_field].des.y==tag[i].des.y && tag[i].des.l && pos_able(i)) 
				{ 
					int dif; 
 
					if((dif=tag[act_field].des.x-tag[i].des.x)>0 && dif<min) 
					{ 
						min=dif; 
						j=i; 
					} 
				} 
			} 
			if(j<0 || tag[j].des.y-line>=h-2 || tag[j].des.y<line) 
			{ 
				if(j<0 && colon) 
					colon=0; 
				return(act_field); 
			} 
			return(j); 
		case CD: 
			for(++i;i<num_fields;i++) 
			{ 
				if(tag[i].des.y>tag[act_field].des.y && tag[i].des.l && pos_able(i))
					break; 
			} 
			if(i==num_fields && num_fields) 
			{ 
				if(tag[act_field].des.atr&TABL) 
				{ 
					return(-1); 
				} 
				return(act_field); 
			} 
			n=i; 
			while(i<num_fields && tag[i].des.y==tag[n].des.y) 
			{ 
				if(abs(tag[act_field].des.x-tag[i].des.x)<min && tag[i].des.l && pos_able(i))
				{ 
					min=abs(tag[act_field].des.x-tag[i].des.x); 
					j=i; 
				} 
				i++; 
			} 
			if(j<0) 
				break; 
			return(j); 
		case CU: 
			for(--i;i>=0;i--) 
			{ 
				if(tag[i].des.y<tag[act_field].des.y && tag[i].des.l && pos_able(i))
					break; 
			} 
			if(i<0) 
			{ 
				if(tag[act_field].des.atr&TABL) 
					return(-2); 
				return(act_field); 
			} 
			n=i; 
			while(i>=0 && tag[i].des.y==tag[n].des.y ) 
			{ 
				if(abs(tag[act_field].des.x-tag[i].des.x)<min && tag[i].des.l && pos_able(i))
				{ 
					min=abs(tag[act_field].des.x-tag[i].des.x); 
					j=i; 
				} 
				i--; 
			} 
			if(j<0) 
				break; 
			return(j); 
		case 1:
			if(tag[act_field].des.y-line>=h-2 || tag[act_field].des.y<line)
			{
				for(i=num_fields-1;i>=0;i--) 
					if(possible(i))
						return(i); 
			}
			break; 
		case -1:
			if(tag[act_field].des.y-line>=h-2 || tag[act_field].des.y<line)
			{
				for(i=0;i<num_fields;i++) 
					if(possible(i))
						return(i); 
			}
			break; 
		case -2: 
			if(tag[act_field].des.x-colon>=l-2 || (tag[act_field].des.x+tag[act_field].des.l)<=colon)
				for(i=act_field;i>=0;i--) 
					if(possible(i)) 
						return(i); 
			break; 
		case 2: 
			if(tag[act_field].des.x-colon>=l-2 || (tag[act_field].des.x+tag[act_field].des.l)<=colon) 
				for(i=act_field;i<num_fields;i++) 
					if(possible(i)) 
						return(i); 
		case 0: 
			if(!possible(act_field))
				for(i=0;i<num_fields;i++) 
					if(possible(i))
						return(i); 
			break; 
	} 
	return(act_field); 
} 
 
void shadows(int x,int y,int l,int h,int x0,int y0,int l0,int h0) 
{ 
	register int i,j; 
 
	i=x+l; 
	j=y+1; 
	if(h<2) 
	{ 
		if(i>x0 && i<x0+l0 && y>y0 && y<y0+h0) 
			term->revers(i,y); 
		if(i+1>x0 && i+1<x0+l0 && y>y0 && y<y0+h0) 
			term->revers(i+1,y); 
	} 
 
	for(;j<y+h+1;j++) 
	{ 
		if(i>x0 && i<x0+l0 && j>y0 && j<y0+h0) 
			term->revers(i,j); 
		if(i+1>x0 && i+1<x0+l0 && j>y0 && j<y0+h0) 
			term->revers(i+1,j); 
	} 
 
	x+=2; 
	y+=h; 
	for(i=0;i<l-2;i++) 
		if(i+x>x0 && i+x<x0+l0 && y>y0 && y<y0+h0) 
			term->revers(x+i,y); 
} 
 
int BROWSER::possible(int i) 
{ 
	if(i>=num_fields || i<0 || tags[i].des.sla[0].n==0) 
		return(0); 
	if(pos_able(i)==0)
		return(0); 
//         return(1);
	return(tags[i].des.x-colon<l-2 && tags[i].des.y-line<h-2 && tags[i].des.y>=line && (tags[i].des.x+tags[i].des.l)>colon && tags[i].des.l);
} 
 
int BROWSER::pos_able(int i) 
{ 
	int j; 
 
	j=(tags[i].des.atr&TABL)==0; 
	if(form_cond&TABLE) 
		j=!j; 
	if(form_cond&ARRAY) 
		j=1; 
	return(j && !(tags[i].des.atr&NO_POS)); 
} 
 
int BROWSER::Act_Field() 
{ 
	return(act_field); 
} 
 
int BROWSER::Act_Field(int i) 
{ 
	if( i>=0 && i<num_fields || i==-1)
		act_field=i; 
      while(act_field<num_fields && (tags[act_field].des.atr & NO_POS))
			act_field++;
	return(act_field); 
} 
 
int BROWSER::Draw_Slot(int i,int bg,int fg,struct font font)
{ 
	int len,x1,mark=0; 
 
	x1=tags[i].des.x-colon+1; 
	if(x1<1) 
		x1=1; 
	if(x1>=l-1) 
		return(0); 
	x1+=x0; 
	if(colon>tags[i].des.x) 
		len=tags[i].des.l-(colon-tags[i].des.x); 
	else    len=tags[i].des.l; 
 
	if(x1+len>=x0+l) 
		len=x0-x1+l-1; 

	if(font.fnt!=-1) 
		term->Set_Font(font.fnt,font.atr); 

	if(form_cond&MARK && (mark=if_mark_field(i))) 
	{ 
		bg=03; 
		fg=017; 
	} 
	if(bg==0 && fg==0) 
		fg=7; 
	if(tags[i].des.h<=1)
	{
		term->Set_Color(bg,fg);
		term->dpp(x1,y0+tags[i].des.y-line+1);
		term->dpn(len,' ');
	}
	else
	{
		term->Set_Color(bg&0xff,fg);
		for(int h=1;h<tags[i].des.h;h++)
		{
			term->dpp(x1,y0+tags[i].des.y-line+h);
			term->dpn(len,' ');
		}
		term->scr->draw_frame(x1,y0+tags[i].des.y-line+1,len,tags[i].des.h-1,1);
	}
	term->dpp(x1,y0+tags[i].des.y-line+1);
	if(tags[i].des.sla[0].n) 
	{ 
		for(int j=0;j<len;j++) 
		{ 
			term->put_fg(x1+j,y0+tags[i].des.y-line+1,fg); 
			term->put_bg(x1+j,y0+tags[i].des.y-line+1,bg); 
			if(bg&0x300) 
				term->scr->put_a2(x1+j,y0+tags[i].des.y-line+1,(bg>>8)&3); 
		} 
	} 
	if(mark) 
	{ 
		char str[32]; 
 
		sprintf(str,"#%d",mark); 
		term->dpsn((unsigned char *)str,len+1); 
	} 
	else if(tags[i].str!=NULL) 
	{ 
		if(string_digit(tags[i].str)) 
		{ 
			int length=strlen(tags[i].str); 
			if(length<len) 
				term->dpp(x1+(len-length),y0+tags[i].des.y-line+1); 
		} 
		char *ch=tags[i].str; 
		if(tags[i].des.atr&IS_IMAGE)
		{ 
			ch=strrchr(ch,'/'); 
			if(ch==NULL) 
				ch=tags[i].str; 
			else    ch++; 
		} 
		if(tags[i].des.h<=1)
			term->dpsn((unsigned char *)ch,len+1);
		else
		{
			for(int h=1;h<tags[i].des.h;h++)
			{
				term->dpp(x1,y0+tags[i].des.y-line+h);
				int j;
				for(j=0;j<len && ch[j];j++)
				{
					if(ch[j]=='\n')
					{
						ch++;
						break;
					}
					term->dpo(ch[j]);
				}
				if(strlen(ch)<=len)
					break;
				ch+=j;
			}
			term->scr->draw_frame(x1,y0+tags[i].des.y-line+1,len,tags[i].des.h-1,1);
		}
	} 
	term->dpp(x1,y0+tags[i].des.y-line+1); 
	term->Set_Font(0); 
	return(len); 
} 

void BROWSER::Del_Label()
{
	if(label)
	{
		for(int i=0;i<num_label;i++)
		{
			term->Del_Label(label[i].x+1+x0-colon,label[i].y+1+y0-line);
			if(label[i].text!=NULL)
			{
				free(label[i].text);
				label[i].text=NULL;
			}
		}
		free(label);
		num_label=0;
		label=NULL;
	}
}

void BROWSER::form_update(CX_BASE *db, long record)
{
	for(int i=1;i<=num_fields;i++)
		form_update(db,record,i);
}

void BROWSER::form_update(CX_BASE *db, long record, int field)
{
	if(field<1 || field>num_fields)
		return;
	if(tags[field-1].des.sla->n==-2)
		db->Str_To_Sla(tags[field-1].name,tags[field-1].des.sla);
	if(tags[field-1].des.sla->n<=0)
	{
		char str[32],fmt[32];

		sprintf(fmt,"%%%dd",(int)tags[field-1].des.l);
		sprintf(str,fmt,tags[field-1].index);
		tags[field-1].str=(char *)realloc(tags[field-1].str,strlen(str)+1);
		strcpy(tags[field-1].str,str);
		tags[field-1].des.atr|=NO_POS;
	}
	else
	{
		if(db->share!=NULL)
			db->share->font.fnt=-1;
		db->Get_Slot(record,tags[field-1].des.sla,tags[field-1].str);

		if(db->Field_Descr(tags[field-1].des.sla->n)->a==X_TEXT && tags[field-1].des.sla[1].n)
		{
			char *ch=tags[field-1].str;
			char *buf=ch;

			for(int i=0;i<tags[field-1].des.sla[1].n;i++)
			{
				if(i)
					buf=ch+1;
				if((ch=strchr(buf,'\n'))==NULL)
					break;
			}
			if(ch!=NULL)
			{
				int len=ch-buf;
				if(len>term->l_x())
					len=term->l_x();
				char *tmp=(char *)malloc(len+2);
				bcopy(buf,tmp,len+1);
				tmp[len+1]=0;
				free(tags[field-1].str);
				tags[field-1].str=tmp;
			}
		}
		if(db->Field_Descr(tags[field-1].des.sla->n)->a==X_VARIANT && tags[field-1].des.sla[1].n==1)
		{
			char *ch=strrchr(tags[field-1].str,'/');
			if(ch!=NULL)
				bcopy(ch+1,tags[field-1].str,strlen(ch));
		}
		if(tags[field-1].des.sla->n>db->Num_Fields() && db->share!=NULL)
		{
			if(db->share->font.fnt!=-1)
			{
				tags[field-1].font=db->share->font;
			}
			if(db->share->color.bg || db->share->color.fg)
			{
				tags[field-1].color.bg=db->share->color.bg;
				tags[field-1].color.fg=db->share->color.fg;
				if(form_cond&BW)
				{
					tags[field-1].color.fg=0;
					if(tags[field-1].color.bg!=010)
						tags[field-1].color.bg=017;
					else
						tags[field-1].color.bg=7;
				}
			}
		}
	}
}
