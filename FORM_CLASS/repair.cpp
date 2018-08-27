/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:repair.cpp
*/
#include "StdAfx.h" 
#include "../CX_Browser.h" 
#include "../DB_CLASS/ram_base.h" 
extern Terminal *term; 
 
#define MAXMIN -2147483647l 
 
void CXDB_REPAIR::Create_Tree() 
{ 
	char *ch=NULL; 
	int f=-1,x=0,y=0; 
 
	if(visual && max_record>1000) 
	{ 
		x=(term->l_x()-50)/2; 
		y=(term->l_y()/2); 
		f=term->get_box(x-2,y-2,50+6,5); 
		term->BOX(x-1,y-1,52,3,' ',6,0x7,6,0x7), 
		term->Set_Color(016,0); 
		term->dpp(x,y); 
		term->scrbufout(); 
	} 
 
	Wlock(0); 
	for(int page=1;page<=max_record;page++) 
	{ 
		if(visual && (page%1000)==0) 
		{ 
			char str[64]; 
			int x0=term->x_c(); 
			int y0=term->y_c(); 
			term->dpp(x+44,y-1); 
			sprintf(str,"%ld",page); 
			term->Set_Color(6,7); 
			term->dps(str); 
			term->dpp(x0,y0); 
			term->scrbufout(); 
		} 
		if(visual && f>=0 && (page%(max_record/50))==0) 
		{ 
			term->Set_Color(016,0); 
			term->dpo(' '); 
			term->scrbufout(); 
		} 
		if(Check_Del(page)) 
		{ 
			delete_list=(long *)realloc(delete_list,++del_num*sizeof(long)); 
			delete_list[del_num-1]=page; 
			continue; 
		} 
		if(des_field->a==X_TEXT) 
			Get_Slot(page,v_field,ch); 
		else Read(page,v_field,ch); 
		if(ch!=NULL) 
			bcopy(ch,(char *)pos+page*len_rec+sizeof (struct key),len_rec-sizeof (struct key)); 
		if((des_field->a==X_TEXT || des_field->a==X_STRING) && page!=max_record)
			*(char *)((char *)pos+page*len_rec+sizeof (struct key)+len_rec-1)=0; 
		Insert_Node(page,1); 
	} 
	free(ch); 
	if(visual) 
	{ 
		term->dpp(x,y); 
		term->Set_Color(014,0); 
	} 
	if(v_field->n==1) 
	{ 
		int del,cur_del=0; 
 
		Put_Buf(fd,(off_t)0,sizeof (struct key),(char *)pos); 
		if(del_num) 
		{ 
			del=-delete_list[0]; 
			Put_Buf(fd,sizeof (struct key),sizeof (long),(char *)&del); 
		} 
		else 
		{ 
			long a[2]; 
			bzero(a,sizeof a); 
			Put_Buf(fd,sizeof (struct key),2*sizeof (long),(char *)a); 
		} 
		for(int page=1;page<=max_record;page++) 
		{ 
			if(cur_del<del_num && page==delete_list[cur_del]) 
			{ 
				struct key key; 
 
				if(cur_del) 
					key.l=-delete_list[cur_del-1]; 
				else    key.l=MAXMIN; 
				if(cur_del<del_num-1) 
					key.r=-delete_list[cur_del+1]; 
				else    key.r=MAXMIN; 
				bcopy(&key,(char *)pos+page*len_rec,sizeof key); 
				if((++cur_del)==del_num) 
				{ 
					del=-delete_list[cur_del-1]; 
					Put_Buf(fd,sizeof (struct key)+sizeof (long),sizeof (long),(char *)&del); 
				} 
			} 
			Put_Buf(fd,root_size+(page-1)*ss.size,sizeof (struct key),(char *)pos+page*len_rec); 
			if(visual && f>=0 && (page%(max_record/50))==0) 
			{ 
				term->dpo(' '); 
				term->scrbufout(); 
			} 
		} 
	} 
	else 
	{ 
		char name[LINESIZE]; 
 
		sprintf(name,"%s/Tree.%d",Name_Base(),v_field->n); 
		int fd=creat(name,0666); 
		write(fd,pos,sizeof(struct key)); 
		for(int page=1;page<=max_record;page++) 
		{ 
			write(fd,(char *)pos+page*len_rec,sizeof(struct key)); 
			if(visual && f>=0 && (page%(max_record/50))==0) 
			{ 
				term->dpo(' '); 
				term->scrbufout(); 
			} 
		} 
		close(fd); 
	} 
	Unlock(0); 
	if(visual && f>=0) 
	{ 
		term->restore_box(f); 
		term->free_box(f); 
		term->scrbufout(); 
	} 
} 
