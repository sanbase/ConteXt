/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:menu.cpp
*/
#include "stdafx.h" 
#include "CX_Browser.h" 
extern Terminal *term; 
 
int CX_BROWSER::Menu() 
{ 
	struct sla sla[SLA_DEEP]; 
	int cmd=0; 
 
	bzero(sla,sizeof sla); 
	delete_menu(); 
	for(int i=0;i<num_label;i++)
		term->Del_Label(label[i].x+1+x0-colon,label[i].y+1+y0-line);


	CX_BROWSER *menu; 
	int y=5; 
	try 
	{ 
		struct x_form f; 
 
		bzero(&f,sizeof f); 
		f.form=0; 
		strcpy(f.blank,"Menu"); 
		term->scrbufout(); 
		term->BlackWhite(x0+1,y0+1,x0+l-2,y0+h-1); 
		if(y+23>term->l_y()) 
			y=term->l_y()-23; 
		if(y<0) 
			y=0; 
		menu=new CX_BROWSER(MENUDEF,1,&f,10,y,28,20); 
		menu->delete_menu(); 
	} 
	catch(...) 
	{ 
		load_menu(); 
		return(0); 
	} 
	menu->Read_Only(1); 
	for(;;) 
	{ 
		term->MultiColor(1,1,menu->l,menu->h+1); 
		Load_Menu(ICONSDEF,5);
		show_menu(1);
		int code=menu->Move(0);
		if(code>=1100)
			code-=1000;
		if(code=='\r' || code==c_Save)
		{ 
			bzero(sla,sizeof sla); 
			sla->n=2; 
			sla->m=-1; 
			char *ch=NULL; 
			if(menu->db->Read(menu->Act_Record(),sla,ch).len==0) 
			{ 
				if(ch!=NULL) 
					free(ch); 
				break; 
			} 
			if(ch!=NULL) 
				free(ch); 
			CX_BROWSER *submenu; 
			try 
			{ 
				struct x_form f; 
 
				bzero(&f,sizeof f); 
				f.form=0; 
				strcpy(f.blank,"Items"); 
				term->scrbufout(); 
				try 
				{
					submenu=new CX_BROWSER(MENUDEF,menu->Act_Record(),&f,25,y+menu->Act_Record(),34,20);
				} 
				catch(...) 
				{ 
					goto END; 
				} 
				submenu->delete_menu(); 
				submenu->Read_Only(1); 

				Load_Menu(ICONSDEF,5);
				show_menu(1);

				int i=submenu->Move(0); 
 
				clean_menu();

				term->scrbufout(); 
				term->BlackWhite(x0+1,y0+1,x0+l-2,y0+h-1); 
				if(i>=1100)
					i-=1000;
				if(i=='\r' || i==c_Save)
				{ 
					char *ch=NULL; 
					char slot[32]; 
					sprintf(slot,"^2[%d].1",submenu->tags[submenu->act_field].des.sla[0].m); 
					submenu->db->Get_Slot(submenu->Index(),slot,ch); 
					if(ch!=NULL) 
					{ 
						cmd=atoi(ch); 
						free(ch); 
					} 
					delete submenu; 
					break; 
				} 
				delete submenu; 
			} 
			catch(...) 
			{ 
				; 
			} 
		} 
		else    break; 
	} 
END: 
	clean_menu();
	delete menu; 
	term->MultiColor(x0+1,y0+1,x0+l-2,y0+h-1); 
	load_menu(); 
	return(cmd); 
} 
 
#ifdef PMENU 
int PMenu(char *menu, char **select_name) 
{ 
	char *str=(char *)malloc(strlen(menu)+1); 
	char obr[32]; 
	int max_len=0,rez; 
	int num_fields; 
	BROWSER *browser=NULL; 
	struct tag *tags=NULL; 
 
	bzero(obr,sizeof obr); 
BEGIN: 
	if(browser!=NULL) 
		delete browser; 
	strcpy(str,menu); 
	char *ch=str; 
	for(num_fields=0;*ch;num_fields++) 
	{ 
		tags=(struct tag *)realloc(tags,(num_fields+1)*sizeof (struct tag)); 
		tags[num_fields].str=ch; 
		if((ch=strchr(ch,';'))==NULL) 
			break; 
		*ch=0; 
		ch++; 
		if(*obr && strncmp(tags[num_fields].str,obr,strlen(obr))) 
		{ 
			num_fields--; 
			continue; 
		} 
		if(strlen(tags[num_fields].str)>max_len) 
			max_len=strlen(tags[num_fields].str); 
	} 
	int columns,lines; 
	if((columns=(term->l_x()-4)/(max_len+1))<=0) 
		columns=1; 
	lines=columns*(term->l_y()-6); 
	if(num_fields>lines) 
		num_fields=lines; 
	for(;;) 
	{ 
		if((columns=get_columns(num_fields,(term->l_x()-4)/(max_len+1)))==0) 
			columns=1; 
		lines = (num_fields+columns-1)/columns; 
		if(lines<term->l_y()-6) 
			break; 
		num_fields--; 
	} 
	if((columns=get_columns(num_fields,(term->l_x()-4)/(max_len+1)))==0) 
		columns=1; 
	lines = (num_fields+columns-1)/columns; 
	for(int i=0;i<num_fields;i++) 
	{ 
		bzero(&tags[i].des,sizeof tags->des); 
		tags[i].des.x=(i/lines)*(max_len+1); 
		tags[i].des.y=i%lines; 
		tags[i].des.l=max_len; 
		tags[i].des.atr=0; 
		tags[i].index=0; 
		tags[i].color.bg=06; 
		tags[i].color.fg=016; 
	} 
 
	int xx=(term->l_x()-columns*(max_len+1))/2; 
	int yy=(term->l_y()-lines)/3; 
	if(term->l_y()-yy<25) 
		yy=term->l_y()-25; 
	if(yy<0) 
		yy=0; 
 
	browser = new BROWSER(NULL,xx,yy,columns*(max_len+1),lines+2); 
	browser->create_Manual(num_fields,tags,0,NULL); 
	hot_line(message(25)); 
	switch(rez=browser->Move()) 
	{ 
		case '\r': 
		{ 
			int i=browser->Act_Field(); 
 
			*select_name=(char *)realloc(*select_name,strlen(tags[i].str)+1); 
			sprintf(*select_name,tags[i].str); 
			delete browser; 
			return(rez); 
		} 
	case F10: 
		delete browser; 
		return(F10); 
	case DE: 
		if(*obr) 
			obr[strlen(obr)-1]=0; 
		goto BEGIN; 
	default: 
		if(rez>='A' && rez<'z') 
		{ 
			obr[strlen(obr)]=rez; 
			goto BEGIN; 
		} 
	} 
	if(browser) 
		delete browser; 
	return(0); 
} 
#endif 
