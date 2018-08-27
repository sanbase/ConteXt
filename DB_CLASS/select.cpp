/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:select.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#include "../CX_Browser.h" 
#ifdef WIN32 
#define SERVER 
#else 
#ifdef THREAD 
#include <pthread.h> 
#endif 
#endif 
 
extern CX_BROWSER *current_browser; 
 
long CX_BASE::Select(char *str,char *query,selection *select) 
{ 
	struct sla sla[SLA_DEEP]; 
	str_to_sla(str,sla); 
	return(Select(sla,query,select)); 
} 
 
long CX_BASE::Select(int field,char *query,selection *select) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	bzero(sla,sizeof sla); 
	sla->n=field; 
	return(Select(sla,query,select)); 
} 
 
long CX_BASE::Select(struct sla *sla,char *query,selection *select) 
{ 
	current_browser=NULL; 
	return(Select(sla,query,select,NULL)); 
} 
/* 
long CX_BASE::Select(struct sla *sla,char *query,selection *select, long (*record)(long, CX_BROWSER *)) 
{ 
	return Select(sla,query,select,record); 
} 
*/ 
int CX_BASE::Sorting(int field,selection *select,int nap) 
{ 
	struct sla sla[SLA_DEEP],*s; 
 
	bzero(sla,sizeof sla); 
	sla->n=field; 
	s=sla; 
	return(Sorting(&s,1,select,nap)); 
} 
 
int CX_BASE::Sorting(struct sla *sla,selection *select,int nap) 
{ 
	current_browser=NULL; 
	return(Sorting(&sla,1,select,nap)); 
} 
 
int CX_BASE::Sorting(struct sla **sla,int num_fields,selection *select,int nap) 
{ 
	Query q(this); 
	return q.Sorting(sla,num_fields,select,nap); 
} 
 
long CX_BASE::join(selection *b, selection *a) 
{ 
	if(b==NULL || b->num_index==0) 
		return(a->num_index); 
	int max=a->num_index; 
	for(int i=0;i<b->num_index;i++) 
	{ 
		for(int j=0;j<max;j++) 
		{ 
			if(a->index[j]==b->index[i]) 
				goto CONT; 
		} 
		a->index=(long *)realloc(a->index,(++a->num_index)*sizeof (long)); 
		a->index[a->num_index-1]=b->index[i]; 
CONT: 
		continue; 
	} 
	return(a->num_index); 
} 
 
int CX_BASE::check_tree(char *query) 
{ 
	int i=0; 
	char *q=(char *)malloc(strlen(query)+1); 
	strcpy(q,query); 
	char *ch; 
	do 
	{ 
		if((ch=strrchr(q,'|'))!=NULL) 
		{ 
			*ch=0; 
			ch++; 
		} 
		else    ch=q; 
 
		if(*ch=='!' || *ch=='<' || strcmp(ch,"-") || !strcmp(ch,"NULL") || !strcmp(ch,"null") || strchr(ch,'*')!=NULL) 
		{ 
			i=1; 
			break; 
		} 
	} 
	while(ch!=q); 
	free(q); 
	return(i); 
} 
 
long CX_BASE::Select(struct sla *sla,char *query,selection *select, long (*record)(long, CX_BROWSER *)) 
{ 
	Query qu(this); 
	if(open_Tree(sla)<=0 || !check_tree(query)) 
		return(qu.Select(sla,query,select,record)); 
 
	char *q=(char *)malloc(strlen(query)+1); 
 
	selection *s1 = new selection(select); 
	selection *s2 = new selection(); 
	strcpy(q,query); 
	char *ch; 
 
	do 
	{ 
		if((ch=strrchr(q,'|'))!=NULL) 
		{ 
			*ch=0; 
			ch++; 
		} 
		else    ch=q; 
		if(qu.Select(sla,ch,s1,record)) 
		{ 
			join(s1,s2); 
		} 
			delete s1;
			s1 = new selection(select);
	} 
	while(ch!=q); 
 
	if(select->index!=NULL) 
	{ 
		free(select->index); 
		select->index=NULL; 
	} 
	delete s1; 
	select->index=s2->index; 
	select->num_index=s2->num_index; 
	free(q); 
	return(s2->num_index); 
 
} 
 
static int cmp(const void *a, const void *b) 
{ 
	return(*(long *)a-*(long *)b); 
} 
 
// select records having property "attr"=="value". 
// if value==NULL,select records where property "attr" exist. 
int CX_BASE::Select_Property(selection *sel, char *attr, char *value) 
{ 
	char *str=NULL; 
	selection psel; 
 
	sel->clean(); 
	chdir(__name); 
	CX_BASE *property=NULL,*attribute=NULL; 
	try 
	{ 
		property=open_db(PROPERTY); 
 
		str=(char *)malloc(strlen(attr)+32); 
		sprintf(str,"%s/%s",PROPERTY,attr); 
		attribute=open_db(str); 
	} 
	catch(...) 
	{ 
		goto END; 
	} 
	if(value==NULL) 
	{ 
		str=(char *)realloc(str,strlen(attr)+32); 
		sprintf(str,"%s/%s",PROPERTY,attr); 
		property->Select("^2.1",str,&psel); 
	} 
	else 
	{ 
		CX_BASE *flex=NULL; 
 
		long atr; 
		str=(char *)realloc(str,64); 
		sprintf(str,"%s/%s",PROPERTY,FLEXDB); 
 
		try 
		{ 
			flex=open_db(str); 
		} 
		catch(...) 
		{ 
			goto END; 
		} 
		str=(char *)realloc(str,strlen(attr)+32); 
		sprintf(str,"%s/%s",PROPERTY,attr); 
		if(flex->Select("^1",str,&psel)<=0) 
			 goto END; 
		atr=psel.index[0]; 
		psel.clean(); 
		if(attribute->Select(1,value,&psel)<=0) 
			goto END; 
		sprintf(str,"#%ld:%ld",atr,psel.index[0]); 
		psel.clean(); 
		property->Select("^2",str,&psel); 
	} 
	if(psel.num_index>0) 
	{ 
		char *ch=NULL; 
		int i; 
		for(i=1;i<=psel.length();i++) 
		{ 
			property->Get_Slot(psel.Index(i),1,ch); 
			if(ch!=NULL && *ch=='#') 
				sel->Add(atoi(ch+1)); 
		} 
		if(ch!=NULL) 
			free(ch); 
		qsort(sel->index,sel->length(),sizeof sel->index[0],cmp); 
		int num=sel->length(); 
		sel->num_index=1; 
		for(i=1;i<num;i++) 
		{ 
			if(sel->index[i]==sel->index[sel->num_index-1]) 
				continue; 
			sel->index[sel->num_index++]=sel->index[i]; 
		} 
	} 
END: 
	if(str!=NULL) 
		free(str); 
	chdir(".."); 
	psel.clean(); 
	return(sel->num_index); 
} 
 
// create selection of property belongs to the record 
int CX_BASE::Get_Property(long record, selection *sel) 
{ 
	char *str=(char *)malloc(strlen(__name)+32); 
	sprintf(str,"%s/%s",__name,PROPERTY); 
	CX_BASE *property; 
	try 
	{ 
		property=open_db(str); 
	} 
	catch(...) 
	{ 
		return(0); 
	} 
	sprintf(str,"#%ld",record); 
	property->Select(1,str,sel); 
	free(str); 
	return(sel->num_index); 
} 
/* 
int CX_BASE::Select_Property(long record,char *name) 
{ 
	char *str=NULL; 
	CX_BASE *property,*flex; 
	if(name==NULL || !*name) 
		return(-1); 
	try 
	{ 
		str=(char *)malloc(strlen(__name)+32); 
		sprintf(str,"%s/%s",__name,PROPERTY); 
		property=open_db(str); 
		strcat(str,"/"); 
		strcat(str,FLEXDB); 
		flex=open_db(str); 
	} 
	catch(...) 
	{ 
		return(-1); 
	} 
	str=(char *)realloc(str,strlen(__name)+strlen(name)+32); 
	sprintf(str,"%s/%s",PROPERTY,name); 
	char *item_db=(char *)malloc(strlen(str)+32); 
	sprintf(item_db,"%s/%s",__name,str); 
 
	CX_FIND f=CX_FIND(flex); 
	long page=f.Find_First(1,str,0); 
	if(page<=0)     // such attribute is not exist 
		return(0); 
 
	CX_BASE *item=open_db(item_db); 
	f=CX_FIND(item); 
	page=f.Find_First(1,value,0); 
} 
*/ 
long selection::Index(int i) 
{ 
	if(i>0 && i<=num_index) 
		return(index[i-1]); 
	else    return(0); 
} 
 
long selection::length() 
{ 
	return(num_index); 
} 
 
void selection::clean() 
{ 
	if(index) 
		free(index); 
	num_index=0; 
	index=NULL; 
} 
 
int selection::Add(long i) 
{ 
	index=(long *)realloc(index,++num_index*sizeof (long)); 
	index[num_index-1]=i; 
	return(num_index); 
} 
 
selection::selection(selection *sel) 
{ 
	num_index=sel->num_index; 
	if(num_index>0 && sel!=NULL && sel->index!=NULL) 
	{ 
		index=(long *)malloc(num_index*sizeof (long)); 
		bcopy(sel->index,index,num_index*sizeof (long)); 
	} 
	else 
	{ 
		index=NULL; 
	} 
} 
 
selection::selection() 
{ 
	num_index=0; 
	index=NULL; 
} 
 
selection::~selection() 
{ 
	if(index!=NULL) 
		free(index); 
} 
