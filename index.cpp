/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:index.cpp
*/
#include "stdafx.h" 
#include "CX_Browser.h" 
extern Terminal *term; 
 
long CX_BROWSER::Index() 
{ 
	return index; 
} 
 
long CX_BROWSER::Record(long page) 
{ 
	if(personal_selection!=NULL && (!(db->cx_cond&SORT) || db->max_index==0))
	{ 
		if(page==1) 
			return(0); 
		return(-1); 
	} 
	return(db->Record(page,level)); 
} 
 
#ifdef WIN32 
char *getlogin() 
{ 
		return("user"); 
} 
#endif 
int CX_BROWSER::put_Record(long ind,long page) 
{ 
	int i=db->put_Record(ind,page,level); 
	if(personal_selection!=NULL)
	{ 
		if(!access(personal_selection,R_OK))
		{ 
			long buf[256]; 
			int i,size,fd, flag_ok=0;
 
			if((fd=open(personal_selection,O_RDWR))<0)
			{ 
				free(personal_selection);
				personal_selection=NULL;
				return(0); 
			} 
			while((size=read(fd,buf,sizeof buf))>0) 
			{ 
				for(i=0;i<(int)(size/sizeof (long));i++) 
					if(buf[i]==page) 
					{
						flag_ok=1;
						break; 
					}
			} 
			if (flag_ok==0)
			write(fd,&page,4);
			close(fd); 
		} 
		else
		{
			free(personal_selection);
			personal_selection=NULL;
		}
	} 
	return(i); 
} 
 
int CX_BROWSER::del_Record(long page) 
{ 
	long rec=Record(page); 
	if(rec<0 || db->Delete(rec)<0) 
		return(-1); 
	int i=db->del_Record(page,level); 
	if(db->max_index==0)
		Rest_Index();
	if(personal_selection!=NULL)
	{ 
		if(access(personal_selection,R_OK))
		{
			free(personal_selection);
			personal_selection=NULL;
		}
		else 
			db->Write_Index(personal_selection);
	} 
	return(i); 
} 
 
void CX_BROWSER::Set_Index() 
{ 
	db->Set_Index(level); 
} 
 
void CX_BROWSER::Make_Indname(char *name) 
{ 
	db->Make_Indname(name,level); 
} 
 
void CX_BROWSER::Open_Index_File() 
{ 
	db->Open_Index_File(level); 
} 
 
long CX_BROWSER::find_page(long page) 
{ 
	return(db->find_page(page,level)); 
} 
 
void CX_BROWSER::Write_Index(char *tmp) 
{ 
	db->Write_Index(tmp); 
} 
 
void CX_BROWSER::Mark_To_Index() 
{ 
	if(mark==NULL) 
		return; 
	if(Arr_To_Index(mark,num_mark)>0) 
	{ 
		Add_Stack("Manual",num_mark); 
	} 
	free(mark); 
	mark=NULL; 
	num_mark=0; 
} 
 
void CX_BROWSER::Add_Stack(char *str,long num) 
{ 
	char *ch; 

	int n=num_stack_sel; 
	stack=(struct s_stack *)realloc(stack,(++num_stack_sel)*sizeof(struct s_stack)); 
	stack[n].num_index=num; 
	bcopy(tags[act_field].des.sla,stack[n].sla,SLA_DEEP*sizeof (struct sla));
	stack[n].query=(char *)malloc(strlen(str)+1); 
	strcpy(stack[n].query,str); 
	if(tags[act_field].name==NULL) 
		ch=db->Name_Field(tags[act_field].des.sla); 
	else    ch=tags[act_field].name; 
	stack[n].name=(char *)malloc(strlen(ch)+1); 
	strcpy(stack[n].name,ch); 
} 
 
void CX_BROWSER::Add_Stack(struct query *q,long num) 
{ 
	char *ch; 
 
	int n=num_stack_sel; 
	stack=(struct s_stack *)realloc(stack,(++num_stack_sel)*sizeof(struct s_stack)); 
	stack[n].num_index=num; 
	bcopy(q->sla,stack[n].sla,SLA_DEEP*sizeof (struct sla));
	stack[n].query=(char *)malloc(strlen(q->str)+1); 
	strcpy(stack[n].query,q->str); 
	if(tags[act_field].name==NULL) 
		ch=db->Name_Field(q->sla); 
	else    ch=tags[act_field].name; 
	stack[n].name=(char *)malloc(strlen(ch)+1); 
	strcpy(stack[n].name,ch); 
} 
 
void CX_BROWSER::Del_Stack() 
{ 
	if(stack!=NULL) 
	{ 
		if(num_stack_sel) 
		{ 
			num_stack_sel--; 
			free(stack[num_stack_sel].query); 
			free(stack[num_stack_sel].name); 
		} 
		if(num_stack_sel>0)
			stack=(struct s_stack *)realloc(stack,num_stack_sel*sizeof(struct s_stack)); 
		else 
		{ 
			free(stack); 
			stack=NULL; 
			num_stack_sel=0;
		} 
	} 
} 
 
int CX_BROWSER::Arr_To_Index(long *array, int num) 
{ 
	if(array==NULL) 
		return(-1); 
	record=Record(index); 
	int fd=db->Arr_To_Index(array,num,level); 
	if((index=db->find_page(record,level))<0) 
		index=db->max_index; 
	db->Cadr_Read(record=Record(index)); 
	Ind_Update(); 
	return(fd); 
} 
 
void CX_BROWSER::Next_Index_File(char *tmp) 
{ 
	long page; 
	page=db->Record(index,level); 
	db->Next_Index_File(tmp,level); 
	if((index=db->find_page(page,level))<0) 
		index=db->max_index; 
	Ind_Update(); 
} 
 
void CX_BROWSER::Read_Index(char *tmp) 
{ 
	long page; 
	page=db->Record(index,level); 
	db->Read_Index(tmp); 
	if((index=db->find_page(page,level))<0) 
		index=db->max_index; 
	Ind_Update(); 
} 
 
int CX_BROWSER::Rest_Index() 
{ 
	long page; 
	page=Record(index); 
	if(db->Rest_Index(level)) 
	{ 
		if(db->insert==0) 
		{ 
			char *name; 
			if(restrict!=NULL && *restrict) 
				name=restrict; 
			else 
				name=personal_selection;
			if(name!=NULL && !access(name,R_OK))
				Read_Index(name); 
		} 
		if((index=db->find_page(page,level))<0) 
			index=db->max_index; 
		Del_Stack(); 
		db->Cadr_Read(record=Record(index)); 
		Ind_Update(); 
		return(1); 
	} 
	Ind_Update(); 
	return(0); 
} 
 
void CX_BROWSER::set_prim_selection(char *name_sel) 
{ 
	del_prim_selection();
	if(db!=NULL&&db->cx_cond&SORT && !(cx_cond&HIST))
	{ 
		while(db->insert) 
		{ 
			int ins=db->insert; 
			Rest_Index(); 
			if(ins==db->insert) 
				break; 
		} 
		Go_To_Index(index); 
	}
	restrict=(char *)realloc(restrict,strlen(name_sel)+1);
	strcpy(restrict,name_sel);
} 
 
void CX_BROWSER::del_prim_selection() 
{ 
	if(restrict!=NULL)
		free(restrict);
	restrict=NULL; 
} 
