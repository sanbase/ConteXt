/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:utils.cpp
*/
#include "StdAfx.h" 
#include "../CX_Browser.h" 
extern Terminal *term; 
 
MString::MString(char *s)
{ 
	str=(char *)malloc(strlen(s)+1); 
	strcpy(str,s); 
} 
 
MString::~MString()
{ 
	free(str); 
} 
 
Line::Line(Terminal *t) 
{ 
	insert=1; 
	term=t; 
	*mask=0; 
	line_std=NULL; 
} 
 
Line::~Line() 
{ 
	if(line_std!=NULL) 
		free(line_std); 
} 
 
int Line::edit(unsigned int i,char *line,int size,int show,int x,int y,int arg) 
{ 
	char *obr="0123456789abcdefABCDEF"; 
	int poz,ch; 
	int maxpoz,shift_poz; 
	if(i>255) 
		return(F10); 
	if(arg<0) 
		secure=1; 
	else secure=0; 
	mask_flg=mask[0]; 
	if(mask_flg) /* если = 0 - маски нет */ 
	{ 
		for(poz=0;mask[poz];poz++) 
			if(mask[poz]=='_') 
				mask[poz]=' '; 
	} 
	for(poz=strlen(line);poz<size;poz++) 
		line[poz]=0; 
	shift_poz=0; 
	term->dpp(x,y); 
	if(term->S_color().fg==0)
		term->Set_Font(0,2);
	else
		term->Set_Font(0,0);
	inds(line,show,' ',NULL);
	for(poz=0;poz<size && line[poz];poz++); 
	maxpoz=poz; 
	if(*mask) 
		for(poz=0;mask[poz] && mask[poz]!=' ';poz++); 
	line[size-1]=0; 
	if(poz>show) 
		poz=show; 
	line_std=(char *)realloc(line_std,strlen(line)+1); 
	strcpy(line_std,line); 
	term->dpp(x+poz,y); 
	if(i) 
	{ 
		ch=i; 
		goto SWITCH; 
	} 
	for(;;) 
	{ 
		indstr(0,line+shift_poz,poz-shift_poz,x,y,show);
		ch=Xmouse(term->dpi()); 
SWITCH: 
		switch(ch) 
		{ 
		case 0: 
			if(term->mouse().y!=y || term->mouse().x<x || term->mouse().x>x+show) 
				return(0); 
			poz=term->mouse().x-x; 
			term->dpp(x+poz-shift_poz,y); 
			break; 
		case 2: 
			continue; 
		case F10: 
			strcpy(line,line_std); 
			return(F10); 
		case CL:        /* - позиция */ 
Cur_left: 
			poz=seek_poz(0,poz,size); 
			while(poz-shift_poz<0) 
			{ 
				shift_poz-=show==1?1:show/2; 
			} 
			term->dpp(x+poz-shift_poz,y); 
			break; 
		case EN: 
			poz=strlen(line); 
			if(poz>=size) poz=size-1; 
			if(maxpoz>show) 
			{ 
				while(poz-shift_poz>=show) shift_poz+=show==1?1:show/2; 
				clear_line(x,y,show); 
				break; 
			} 
			term->dpp(x+poz-shift_poz,y); 
			continue; 
		case HM: 
			poz=0; 
			shift_poz=0; 
			term->dpp(x,y); 
			continue; 
		case IS:        // insert
			if(mask_flg) break; 
			insert=!insert; 
			ind_ins(x+poz-shift_poz,y); 
			break; 
		case CR:
 
			poz=seek_poz(1,poz,size); 
			while(poz-shift_poz>=show) 
			{ 
				shift_poz+=show==1?1:show/2; 
				clear_line(x,y,show); 
			} 
			if(poz>=size) poz=size-1; 
			term->dpp(x+poz-shift_poz,y); 
			break; 
		case '\b':        // backspace
			if(mask_flg) 
				goto Cur_left; 
			if(poz) poz--; 
			else break; 
		case DEL: 
			if(poz<maxpoz) 
			{ 
				while(poz-shift_poz<0) 
				{ 
					shift_poz-=show==1?1:show/2; 
				} 
				if(!insert) 
					line[poz]=' '; 
				{ 
					register int j; 
					for(j=poz;j<maxpoz-1;j++) 
						line[j]=line[j+1]; 
					line[maxpoz=j]=0; 
				} 
Delete: 
				clear_line(x,y,show); 
			} 
			else 
			{ 
				if(poz==maxpoz) 
				{ 
					line[poz]=0; 
					if(poz) 
						maxpoz=--poz; 
					goto Delete; 
				} 
			} 
			if(poz-shift_poz<0) 
			{ 
				poz++; 
				goto Cur_left; 
			} 
			continue; 
		case '\t':        /* табуляция */ 
/* 
			poz+=(8-(poz%8)); 
			if(poz>=size) poz=size-1; 
			goto CR; 
*/ 
		case '\r': 
		case '\n': 
		{ 
			int j; 
 
			for(j=strlen(line)-1;j>=0 && line[j]==' ';j--) 
				line[j]=0; 
//			line[size]=0; 
			return(ch); 
		} 
		default: 
			if(ch<' ' || ch>=256) 
			{ 
				//добавил ля располнования EAN128 fill
				if (ch==29)
				ch=126;
				else
				return(ch); 
			} 
			if(poz==size || ((maxpoz==size) && insert)) 
				break; 
			if(arg>0) 
			{ 
				int j,arg1; 
				arg1=arg; 
				if(arg=='c' || arg=='C') 
				{ 
					if(i=='*' || i=='/' || i=='%') 
						goto OK; 
					arg1=10; 
				} 
				if(ch=='-' || ch=='+' || ch=='.') goto OK; 
				for(j=0;j<arg1 && obr[j] && ch!=obr[j] ;j++); 
				if(j==arg1 || !obr[j]) 
				{ 
//                                        term->dpo(bl); 
					break; 
				} 
			} 
OK: 
			if(poz<maxpoz && insert ) 
			{ 
				register int j; 
				if(maxpoz>=size) break; 
				for(j=++maxpoz;j>poz&&j;j--) 
					line[j]=line[j-1]; 
				line[poz]=' '; 
			} 
			else 
			{ 
				for(;maxpoz<=poz;maxpoz++) 
					line[maxpoz]=' '; 
			} 
			line[poz]=ch; 
			line[maxpoz]=0; 
			poz=seek_poz(1,poz,size); 
			if(poz-shift_poz>show) 
			{ 
				shift_poz+=show==1?1:show/2; 
				clear_line(x,y,show); 
			} 
			else 
			{ 
				term->dpp(x+poz-shift_poz,y); 
			} 

			if(poz==size) 
			{ 
				return('\r');
			} 
		} 
	} 
	return(ch); 
} 
 
void Line::ind_ins(int x,int y) 
{ 
	struct clr color; 
 
	term->dpp(term->l_x()-2,term->l_y()-1); 
	color=term->S_color(); 
	term->Set_Color(0,014); 
	if(!insert) term->dpo('-'); 
	else        term->dpo('+'); 
	term->Set_Color(color); 
	term->dpp(x,y); 
} 
 
void Line::clear_line(int x,int y,int l) 
{ 
	term->dpp(x,y); 
	term->dpn(l,' ');
	term->dpp(x,y); 
} 
 
int Line::seek_poz(int znak,int poz,int size) 
{ 
	int poz_std; 
	poz_std=poz; 
	if(znak) 
	{ 
		if(poz<size) poz++; 
		if(poz<size && mask_flg) 
		{ 
			while(mask[poz]!=' ' && poz<size && mask[poz]) poz++; 
			if(poz==size) poz=seek_poz(0,poz_std,size); 
		} 
	} 
	else 
	{ 
		if(poz) poz--; 
		if(mask_flg) 
		{ 
			if(!poz && mask[poz]==' ') 
				return(poz); 
			while(mask[poz]!=' ' && poz) 
				poz--; 
			if(!poz) poz=seek_poz(1,poz_std,size); 
		} 
	} 
	return(poz); 
} 
/* показать строку line размером size в позиции (x,y) 
  если ind - выделить цветом. После - курсор установить на (x+poz,y) */ 
void Line::indstr(int ind,char *line,int poz,int x,int y,int size) 
{ 
	if(x+size>term->l_x())
		size=term->l_x()-x;
	if(ind)
		term->Set_Color(ind,0);
	term->dpp(x,y); 
	inds(line,size,' ',NULL);
	term->dpp(x+poz,y); 
} 
 
void Line::inds(char *ss,char l,int ch,struct clr *color) 
{ 
	register int i,flag; 
	unsigned char *str=(unsigned char *)ss; 
	struct clr clr; 
	int x_std=term->x_c(),y_std=term->y_c(); 
 
	clr=term->S_color(); 
	if(color!=NULL) 
		term->Set_Color(*color); 
	if(term->S_color().fg==0)
		term->Set_Font(0,2);
	else
		term->Set_Font(0,0);
	for(i=flag=0;i<l;i++) 
	{ 
		if(!str[i]) 
		{ 
			term->Set_Color(clr);
			flag=1; 
		} 
		if(flag) 
		{ 
			term->dpo(ch); 
			continue; 
		} 
		if(secure) 
			term->dpo('*'); 
		else if(str[i]=='\n' || str[i]=='\t') term->dpo(' '); 
		else  if((unsigned char)str[i]<' ') term->dpo('?'); 
		else  term->dpo(str[i]); 
	} 
 
	if(term->get_a2(x_std,y_std)) 
	{ 
		term->put_ch(x_std,y_std,term->get_ch(x_std,y_std)|0x8000); 
		term->put_ch(x_std+l-1,y_std,term->get_ch(x_std+l-1,y_std)|0x4000); 
	} 
	term->scr->draw_frame(x_std,y_std,l,1,1);
	term->Set_Color(clr);
} 
