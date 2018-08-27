/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:write.cpp
*/
/*
			 DBMS ConteXt V.5.7

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Fri Jun  4 11:45:45 2010
			Module:write.cpp
*/
/* 
			 DBMS ConteXt V.5.7 
 
		Autor: Alexander Lashenko, Toronto, Canada 
		Reply to: info@unixspace.com 
		Last modification:Fri Oct 17 12:25:26 2003 
			Module:write.cpp 
*/ 
 
#include "stdafx.h" 
#include "CX_Browser.h" 
 
extern Terminal *term; 
 
void CX_BROWSER::Put_Selection(int arg) 
{ 
	char *dir; 
 
	if((dir=get_user_dir(db->Name_Base(),FILES))!=NULL) 
	{ 
		char *name=NULL; 
		term->BlackWhite(0,0,term->l_x(),term->l_y()); 
		if(get_filename(dir,&name,1)==F10) 
		{ 
			free(dir); 
			term->MultiColor(x0,y0,l,h); 
			return; 
		} 
		term->MultiColor(x0,y0,l,h); 
		strcat(dir,"/"); 
		strcat(dir,name); 
		free(name); 
		Put_Selection(arg,dir); 
		free(dir); 
	} 
} 
 
void CX_BROWSER::Put_Selection(int arg,char *name) 
{ 
	int fd; 
	int index_std=index; 
 
	if((fd=open(name,O_RDWR|O_CREAT,0644))<0) 
	{ 
		sprintf(db->share->output,"Can't open %s!",name); 
		return; 
	} 
	lseek(fd,0,SEEK_END); 
	cx_cond|=PUTD; 
	if(arg) 
	{ 
		int n=0; 
		int x=(term->l_x()-50)/2; 
		int y=(term->l_y()/2); 
		int f=-1; 
		long page=0; 
		if(db->max_index>50) 
		{ 
			term->BOX(x-1,y-1,52,3,' ',6,0x7,6,0x7), 
			term->dpp(x,y); 
			term->scrbufout(); 
			f=term->get_box(0,0,term->l_x(),term->l_y()); 
		} 
		Go_To_Index(index=1); 
		int size=db->max_index/50; 
		for(index=1;index<=db->max_index;index=tags[first_line+num_colon*(num_lines-1)].index+1) 
		{ 
			Go_To_Index(index); 
			Export(fd); 
			page+=(tags[first_line+num_colon*(num_lines-1)].index+1-index); 
			if(f>=0 && (page>size) && n<50) 
			{ 
				page=0; 
				term->restore_box(f); 
				term->free_box(f); 
				term->Set_Color(016,0); 
				term->dpp(x+n++,y); 
				term->dpo(' '); 
				term->scrbufout(); 
				f=term->get_box(0,0,term->l_x(),term->l_y()); 
			} 
		} 
		if(f>=0) 
		{ 
			term->free_box(f); 
		} 
	} 
	else 
		Export(fd); 
	cx_cond&=~PUTD; 
	close(fd); 
	Go_To_Index(index=index_std); 
} 
