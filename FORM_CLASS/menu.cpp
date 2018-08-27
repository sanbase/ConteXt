/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:menu.cpp
*/
#include "StdAfx.h" 
#include "../CX_Browser.h" 
 
extern Terminal *term; 
 
struct image *images=NULL; 
static struct { char file[NAMESIZE]; long page; int atr;} ic_menu; 
 
int get_menu_cmd(int x0,int y0,int b) 
{ 
	if(images==NULL) 
		return(0); 
	for(int i=0;images[i].name!=NULL;i++) 
	{ 
		int x,shift; 
		if(images[i].y>term->l_y()-2) 
			continue; 
		if(!(*images[i].name)) 
			continue; 
		if(term->font_W()<10) 
			shift=(11-term->font_W())*4/term->font_W(); 
		else    shift=0; 
		if(images[i].x<0) 
			x=term->l_x()+1-shift; 
		else    x=images[i].x; 
		if(x0>=x && x0<=x+40/term->font_W() && y0==images[i].y) 
		{ 
			if(b==1) 
				return(images[i].left); 
			else 
				return(images[i].right); 
		} 
	} 
	return(0); 
} 
 
void clean_menu() 
{ 
	del_menu(); 
	if(images!=NULL) 
	{ 
		int i; 
		for(i=0;images[i].name!=NULL;i++) 
			free(images[i].name); 
		for(i=0;images[i].des!=NULL;i++) 
			free(images[i].des); 
	} 
	images=NULL; 
} 
 
void show_menu(int atr) 
{ 
	int i,x,shift=0; 
 
	if((images==NULL) || (atr==0 && ic_menu.atr==0)) 
		return;

	if(term->font_W()<10) 
		shift=(11-term->font_W())*4/term->font_W(); 
	else    shift=0; 
 
	int h=0,l=0;

	if((term->font_H()*2>60) || (term->font_H()*2<30))
	{
		h=2;
	}
	if((term->font_W()*4>60) || (term->font_W()*4<30))
	{
		l=4;
	}
	for(i=0;images[i].name!=NULL;i++) 
	{ 
		if(images[i].y>term->l_y()-2) 
			continue; 
		if(!(*images[i].name)) 
			continue; 
		if(images[i].x<0) 
			x=term->l_x()+1-shift; 
		else    x=images[i].x; 
		term->Show_Image(x,images[i].y,images[i].name,images[i].des,l,h,1);
	} 
	ic_menu.atr=0; 
} 
 
void del_menu() 
{ 
	int i,x,shift=0; 
	if(images==NULL) 
		return; 
	if(term->font_W()<10) 
		shift=(11-term->font_W())*4/term->font_W(); 
	else    shift=0; 
	for(i=0;images[i].name!=NULL;i++) 
	{ 
		if(images[i].y>term->l_y()-2) 
			continue; 
		if(!(*images[i].name)) 
			continue; 
		if(images[i].x<0) 
			x=term->l_x()+1-shift; 
		else    x=images[i].x; 
		term->Del_Image(x,images[i].y); 
	} 
	bzero(&ic_menu,sizeof ic_menu); 
//        term->scrbufout(); 
} 
 
 
void Load_Menu(char *icon_base,long page) 
{ 
	if(!strcmp(ic_menu.file,icon_base) && ic_menu.page==page) 
		return; 
 
	CX_BASE *icons=NULL; 
	try 
	{ 
		icons = new CX_BASE(icon_base); 
	} 
	catch(...) 
	{ 
		return; 
	} 
	if(icons!=NULL && icons->Name_Base()==NULL) 
	{ 
		delete icons; 
		return; 
	} 
 
	if(icons!=NULL) 
	{ 
		char *str=NULL; 
		int i; 
 
		if(images!=NULL) 
			clean_menu(); 
		images=(struct image *)calloc(icons->Num_Fields(),sizeof (struct image)); 
		struct sla sla[3]; 
		bzero(sla,sizeof sla); 
		for(i=0;i<icons->Num_Fields();i++) 
		{ 
			sla->n=i+2; 
			sla[1].n=1; 
			icons->Get_Slot(page,sla,images[i].name); 
			if(images[i].name==NULL || !*images[i].name) 
				break; 
			sla[1].n=2; 
			icons->Get_Slot(page,sla,images[i].des); 
			sla[1].n=3; 
			icons->Get_Slot(page,sla,str); 
			images[i].left=atoi(str); 
			sla[1].n=4; 
			icons->Get_Slot(page,sla,str); 
			images[i].right=atoi(str); 
			sla[1].n=5; 
			icons->Get_Slot(page,sla,str); 
			images[i].x=atoi(str); 
			sla[1].n=6; 
			icons->Get_Slot(page,sla,str); 
			images[i].y=atoi(str); 
		} 
		if(str!=NULL) 
			free(str); 
		images[i].x=-1; 
		images[i].y=1000; 
		ic_menu.atr=1; 
		strcpy(ic_menu.file,icon_base); 
		ic_menu.page=page; 
		delete icons; 
	} 
} 
