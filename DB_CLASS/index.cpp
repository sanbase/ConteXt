/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:index.cpp
*/

#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#define INDEXSIZE 256  /* размер индексного массива */ 
 
/* возвращает значение индексного массива */ 
long CX_BASE::Record_Index(long page)
{
	return Record(page,index_level);
}
long CX_BASE::Record(long page,int level) 
{ 
	int record=0;
	if(page>max_index)
		Flush_Index_Buf();
	if(page<0l || page>max_index) 
	{ 
		return(-1); 
	} 
	if(!(cx_cond&SORT) || page==0)
		return(page);
	if(index_buf==NULL) 
		Set_Index(level); 
	if(fd_index<=0) 
	{ 
		Open_Index_File(level); 
		if(fd_index<=0) 
			return(-4); 
	} 
	if(index_seek<0 || page-index_seek<0l || page-index_seek>=INDEXSIZE)
	{ 
		index_seek=page; 
		lseek(fd_index,(off_t)((page-1)*(sizeof (long))),SEEK_SET); 
		read(fd_index,index_buf,INDEXSIZE*(sizeof (long)));
	}
	if((record=index_buf[page-index_seek])<=0) 
	{ 
		if(!record && max_index==1) 
			return(0); 
		return(-2); 
	} 
	if(record>max_record) 
	{ 
		if(record>(max_record=last_cadr())) 
			return(-3); 
	} 
	return(record); 
} 
 
/* поместить значение ind в ячейку page */ 
int CX_BASE::put_Record(long ind,long record,int level) 
{ 
	long i; 
 
	if(ind<1 || record<1) 
		return(-1); 
	if(record>max_record)     /* что-бы не вылетали за базу */ 
	{ 
		if(record>(i=last_cadr())) 
			return(-2); 
		else  max_record=i; 
	} 
	if(ind>max_index+1) 
	{ 
		Flush_Index_Buf(); 
		if(ind>max_index+1) 
			return(-3); 
	} 
	if(index_buf==NULL) 
		Set_Index(level); 
	if(ind==max_index+1) 
		max_index++; 
	if(!(cx_cond&SORT)) 
	{ 
		if(ind!=1)      /* индексы должны идти по-порядку */ 
			return(-4); 
		max_index=1; 
		index_seek=-1; 
		cx_cond|=SORT; 
	} 
	if(fd_index<=0) 
	{ 
		Open_Index_File(level); 
		if(fd_index<=0) 
			return(-5); 
	} 
	if(index_seek<0 || record-index_seek<0l || ind-index_seek>=INDEXSIZE) 
	{ 
		lseek(fd_index,(off_t)((ind-1)*sizeof (long)),SEEK_SET); 
		write(fd_index,&record,sizeof record);
		index_seek=-1; 
		Record(ind); 
	} 
	else 
		index_buf[ind-index_seek]=record; 
	return(0); 
} 
 
/* удалить номер кадра из выборки */ 
int CX_BASE::del_Record(long page,int level) 
{ 
	long buf[256]; 
	int size; 
	size_t i; 
 
	if(page<1 || (page>max_index)) 
		return(-1); 
 
	if(!(cx_cond&SORT)) 
		return(0); 
	if(fd_index<=0) 
	{ 
		Open_Index_File(level); 
		if(fd_index<=0) 
			return(-1); 
	} 
	if((i=lseek(fd_index,(off_t)((page)*sizeof (long)),SEEK_SET))>0) 
	{ 
		while((size=read(fd_index,buf,sizeof buf))>0) 
		{ 
			lseek(fd_index,(off_t)(i-sizeof (long)),SEEK_SET); 
			write(fd_index,buf,size);
			lseek(fd_index,(off_t)(i+=size),SEEK_SET); 
		} 
	} 
	index_seek=-1;								 
#ifndef WIN32 
	ftruncate(fd_index,(--max_index)*sizeof (long)); 
#else 
#endif 
	return(0); 
} 
 
void CX_BASE::Set_Index(int level) 
{ 
	char index_name[NAMESIZE]; 
	Make_Indname(index_name,level); 
	if(*idx_name)
		if(!access(idx_name,R_OK) && strcmp(idx_name,index_name)) 
			fcopy(index_name,idx_name);
	strcpy(idx_name,index_name); 
	index_buf=(long *)realloc(index_buf,INDEXSIZE*(sizeof (long)));
	index_seek=-1; 
} 
 
/* сформировать имя индексного файла */ 
 
void CX_BASE::Make_Indname(char *name,int level) 
{ 
	char *ch,*ch1; 
 
	ch=__name; 
	index_level=level; 
	if((ch1=strrchr(ch,'/'))!=NULL) 
		ch=ch1+1; 
	sprintf(name,"%s/.vi%d_%s[%d.%d]",TEMPDIR,getpid(),ch,insert,level);
	strcpy(idx_name,name); 
} 
 
/* сохранить старую выборку в стеке и подгрузить новую из файла tmp */ 
/* при этом файл tmp удаляется, точнее переименовывается в index */ 
void CX_BASE::Next_Index_File(char *tmp,int level) 
{ 
	char index_name[NAMESIZE]; 
 
	if(access(tmp,W_OK)) 
		return; 
	Flush_Index_Buf(level); 
	cx_cond|=SORT; 
	insert++; 
	Make_Indname(index_name,level); 
	strcpy(idx_name,index_name); 
#ifndef WIN32 
	if(link(tmp,idx_name)) 
#endif 
		fcopy(idx_name,tmp); 
	unlink(tmp); 
	Set_Index(level); 
	Open_Index_File(level); 
} 
 
/* открыть индексный файл */ 
void CX_BASE::Open_Index_File(int level) 
{ 
	if(fd_index>0) 
		close(fd_index); 
	if(*idx_name && cx_cond&SORT) 
	{ 
		struct stat buf; 
		if((fd_index=open(idx_name,O_RDWR|O_CREAT|O_BINARY,0600))<0)
			return; 
		if(fstat(fd_index,&buf)) 
		{ 
			max_index=0; 
		} 
		else 
			max_index=buf.st_size/sizeof (long); 
	} 
	else 
		fd_index=-1; 
	index_seek=-1; 
} 
 
/* сбросить буфер, установить новое значение max_index */ 
void CX_BASE::Flush_Index_Buf(int level) 
{ 
	struct stat buf; 
 
	if(!(cx_cond&SORT)) 
	{ 
		max_index=max_record=last_cadr(); 
		if(max_index<=0) 
			max_index=1; 
		return; 
	} 
	if(fd_index<=0) 
		Open_Index_File(level); 
	if(fd_index<=0) 
		return; 
	if(index_seek>0) 
	{ 
		int len=0;
 
		lseek(fd_index,(off_t)((index_seek-1)*(sizeof (long))),SEEK_SET); 
		if(index_seek+INDEXSIZE>max_index)
			len=max_index-index_seek+1;
		else len=INDEXSIZE;
	       write(fd_index,index_buf,len*(sizeof (long)));
	} 
	if(fstat(fd_index,&buf)) 
	{ 
		max_index=0; 
	} 
	else 
		max_index=buf.st_size/sizeof (long); 
	if(!max_index) 
	{ 
		max_index=max_record=last_cadr(); 
		if(max_index<=0) 
			max_index=1; 
		if(fd_index>0) 
			close(fd_index); 
		fd_index=0; 
		unlink(idx_name); 
		cx_cond&=~SORT; 
	} 
	index_seek=-1; 
} 
 
long CX_BASE::find_page(long record,int level) 
{ 
	register long i; 
	if(!(cx_cond&SORT)) 
		return(record); 
	for(i=1;i<=max_index;i++) 
		if(record==Record(i,level)) 
			return(i); 
	return(-1); 
} 
 
int CX_BASE::Arr_To_Index(long *array, int num,int level) 
{ 
	int fd; 
	char tmp[64]; 
 
	if(!num) 
		return(0); 
	sprintf(tmp,"%s/.v.%d.tmp",TEMPDIR,getpid());
	if((fd=creat(tmp,0644))>0) 
	{ 
		 write(fd,array,num*sizeof (long));
		close(fd); 
		max_index=num; 
		cx_cond|=SORT; 
		Next_Index_File(tmp,level); 
	} 
	return(fd); 
} 
 
/* сохранить старую выборку в стеке и подгрузить новую из файла tmp */ 
/* все аналогично Next_Index_File, но файл tmp сохраняется */ 
void CX_BASE::Read_Index(char *tmp,int level) 
{ 
	char index_name[NAMESIZE]; 
 
	struct stat buf; 
	if(access(tmp,R_OK)) 
		return; 
	Flush_Index_Buf(level);
	insert++; 
	cx_cond|=SORT; 
	Make_Indname(index_name,level); 
	strcpy(idx_name,index_name); 
	fcopy(idx_name,tmp); 
	Open_Index_File(level); 
	if(stat(idx_name,&buf)) 
	{ 
		max_index=0; 
	} 
	else 
		max_index=buf.st_size/sizeof (long); 
} 
 
void CX_BASE::Link_Index(char *tmp,int level) 
{ 
	char index_name[NAMESIZE]; 
	struct stat buf; 
 
	if(access(tmp,R_OK)) 
	{ 
		cx_cond&=~SORT; 
		max_index=max_record=last_cadr(); 
		return; 
	} 
	Flush_Index_Buf(level);
	insert++; 
	cx_cond|=SORT; 
	Make_Indname(index_name,level); 
	strcpy(idx_name,index_name); 
#ifndef WIN32 
	if(link(tmp,idx_name)) 
#endif 
		fcopy(idx_name,tmp); 
	Open_Index_File(level); 
	if(stat(idx_name,&buf)) 
	{ 
		max_index=0; 
	} 
	else 
		max_index=buf.st_size/sizeof (long); 
} 
 
/* сохранить текущую выборку в файле tmp */ 
void CX_BASE::Write_Index(char *tmp) 
{ 
#ifndef WIN32 
	if(link(tmp,idx_name)) 
#endif 
		fcopy(tmp,idx_name); 
} 
 
/* вернуться к предыдущей выборке по стеку */ 
int CX_BASE::Rest_Index(int level) 
{ 
	if(!insert) 
		return(0); 
	if(fd_index>0) 
		close(fd_index); 
	fd_index=0; 
	unlink(idx_name); 
	--insert; 
	Set_Index(level); 
	if(access(idx_name,R_OK)) 
		cx_cond&=~SORT; 
	else 
		Open_Index_File(level); 
	return(1); 
} 
 
void CX_BASE::Erase_Index(int level) 
{ 
	while(insert) 
	{ 
		if(fd_index>0) 
			close(fd_index); 
		fd_index=0; 
		unlink(idx_name); 
		--insert; 
		Set_Index(level); 
	} 
	cx_cond&=~SORT; 
} 
