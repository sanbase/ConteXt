/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:header.cpp
*/

#include "StdAfx.h" 
#include "CX_BASE.h" 

int CX_BASE::writable() 
{ 
	return(rdonly); 
} 

void CX_BASE::update() 
{ 
	struct stat st; 
	off_t size;

	if(fd==NULL) 
		return; 
	if(fd->in_memory>1)
	{
		if(fd->fsize==0 || fd->fsize<(int)sizeof (struct root))
			max_record=0;
		else
			max_record=(fd->fsize-sizeof (struct root))/ss.size;
		return;
	}
	fstat(fd[0].Fd,&st); 
	if((size=st.st_size)==0 || size<(int)sizeof (struct root)) 
		max_record=0; 
	else 
		max_record=(size-sizeof (struct root))/ss.size; 
	fd->fsize=st.st_size;
#ifndef SERVER 
	for(int i=0;i<num_open_bases;i++) 
	{
		if(open_base[i]==NULL)
			break;
		open_base[i]->flush_sb(); 
	}
#endif 
	flush_sb(); 
} 

void CX_BASE::flush_sb() 
{ 
	for(int i=0;i<fdnum;i++) 
		fd[i].seek=-1; 
} 

long CX_BASE::last_cadr() 
{ 
	update(); 
	return(max_record); 
} 

int CX_BASE::variable(struct st *s,int n) 
{ 
	if(s->field[n].a==X_TEXT && s->field[n].k && context->pswd!=CXKEY3) 
		return(0); 
	return(s->field[n].m || s->field[n].a>=X_TEXT); 
} 

struct field *CX_BASE::Field_Descr(struct sla *sla,long record)
{ 
	return(Get_Field_Descr(&ss,sla,record));
} 

struct field *CX_BASE::Field_Descr(int field,long record)
{ 
	struct sla sla[SLA_DEEP]; 

	bzero(sla,sizeof sla); 
	sla[0].n=field; 
	return(Get_Field_Descr(&ss,sla,record));
} 

int CX_BASE::Num_Fields() 
{ 
	if(context==NULL || context==0) 
		return(0); 
	return(context->ptm); 
} 

static int Num_Field_Struct(struct st *s) 
{ 
	int total=0; 
	for(int i=0;i<s->ptm;i++) 
	{ 
		if(s->field[i].a==X_STRUCTURE) 
			total+=Num_Field_Struct(s->field[i].st.st); 
		else    total++; 
	} 
	return(total); 
} 

int CX_BASE::Total_Num_Fields() 
{ 
	if(context==NULL || context==0) 
		return(0); 
	return(Num_Field_Struct(&ss)); 
} 

char *CX_BASE::Name_Base() 
{ 
	return(__name); 
} 

char *CX_BASE::Short_Name() 
{ 
	char *ch=strrchr(__name,'/'); 
	if(ch!=NULL) 
		return(ch+1); 
	return(__name); 
} 

char *CX_BASE::Name_Subbase(int field,long page)
{ 
	struct sla sla[SLA_DEEP]; 

	bzero(sla,sizeof sla); 
	sla->n=field; 
	return(Name_Subbase(sla,page));
} 

char *CX_BASE::Name_Subbase(struct sla *sla) 
{ 
	return(Name_Subbase(sla,0)); 
} 
char *CX_BASE::Name_Subbase(struct sla *sla,long page) 
{ 
	char *ch=NULL;
	if(!is_pointer(sla)) 
		return(NULL); 
	int i; 
	char *name=NULL;

	for(i=0;sla[i].n;i++);
	struct sla *SLA = (struct sla *)calloc(i+1,SLA_DEEP*sizeof (struct sla));
	for(int j=0;j<i;j++) 
	{ 
		SLA[j]=sla[j]; 
		if(sla[j].n==0) 
			break; 
	} 

	struct field *field; 
	while((field=Get_Field_Descr(&ss,SLA,page))->a!=X_POINTER && field->a!=X_VARIANT)
	{ 
		for(int j=i;j;j--) 
			if(SLA[j].n) 
			{ 
				SLA[j].n=0; 
				break; 
			} 
	} 
	if(field->name==NULL || *field->name==0) 
	{ 
		char *tmp_name=(char *)malloc(strlen(__name)+1); 
		strcpy(tmp_name,__name); 
		char *ch=strrchr(tmp_name,'/'); 
		if(ch!=NULL) 
			*ch=0; 
		field->name=tmp_name; 
	} 
	//name=(char *)realloc(name,strlen(field->name)+1);
	name=(char *)malloc(strlen(field->name)+1);
	strcpy(name,field->name); 
	if((ch=strchr(name,'\n'))!=NULL)
		*ch=0;
	if(field->a==X_VARIANT) 
	{ 

		  if(page<=0)
		  {
			free(SLA);
			return(name);
		      //  page=1;
		  }
		   char *ch=NULL;
		   Read(page,SLA,ch);
		   if (ch!=NULL)
		   {
			if ((page=int_conv(ch,field->n))>0)
			{
				CX_BASE *subbase=open_db(name);
				free(name);
				name=NULL;
				subbase->Read(page,1,name);
			}
			free(ch);
		   }

	} 
	free(SLA);
	return(name); 
} 

char *CX_BASE::Name_Field(int field) 
{ 
	struct sla sla[SLA_DEEP]; 

	bzero(sla,sizeof sla); 
	sla->n=field; 
	return(Name_Field(sla)); 
} 

char *CX_BASE::Name_Field(struct sla *sla) 
{ 
	struct sla sla1[SLA_DEEP]; 
	struct sla SLA[SLA_DEEP]; 
	static char str[LINESIZE]; 
	char *ch=NULL,*ch1=NULL;

	bzero(sla1,sizeof sla1); 
	bcopy(sla,SLA,sizeof SLA); 
	*str=0; 
	if(SLA->n>ss.ptm) 
	{ 
		ch=NULL; 
		Get_Virtual(-1,SLA,ch); 
		if(ch!=NULL) 
		{ 
			strcpy(str,ch); 
			free(ch); 
		} 
		return(str); 
	} 
	for(int i=0;SLA[i].n;i++) 
	{ 
		int tot=0; 
		if(SLA[i].n==-1) 
		{ 
			SLA[i].n=1; 
			tot=1; 
		} 
		sla1[i].n=SLA[i].n; 
		struct field *f=Get_Field_Descr(&ss,sla1); 
		ch=f->name; 

		if(f->a==X_POINTER || f->a==X_VARIANT)
			strcat(str,"->"); 
		else if(i) 
			strcat(str,"."); 
		if(ch==NULL) 
		{ 
			sla_to_str(sla1,str); 
			return(str); 
		} 
		if((ch1=strchr(ch,'\n'))!=NULL) 
			ch=ch1+1; 
		if(tot) 
			strcat(str,"Total"); 
		else 
			strcat(str,ch); 
	} 
	return(str); 
} 

int CX_BASE::Access() 
{ 
	if(context->ptm==1) 
		return(0); 
	if(rdonly) 
		return(-1); 
	return(1); 
} 

int CX_BASE::is_digit(int field) 
{ 
	if(field<1 || field>ss.ptm) 
		return(0); 
	field-=1; 
	return(ss.field[field].a==X_INTEGER || ss.field[field].a==X_UNSIGNED || 
	    ss.field[field].a==X_FLOAT   || ss.field[field].a==X_DOUBLE); 
} 
int CX_BASE::is_digit(struct sla *sla) 
{ 
	int tip=Field_Descr(sla)->a; 
	return(tip==X_INTEGER || tip==X_UNSIGNED || tip==X_FLOAT || tip==X_DOUBLE); 
} 

int CX_BASE::is_pointer(int field) 
{ 
	if(field<1 || field>ss.ptm) 
		return(0); 
	field-=1; 
	return(ss.field[field].a==X_POINTER || ss.field[field].a==X_VARIANT);
} 
int CX_BASE::is_pointer(struct sla *sla)
{
	struct st *s;
	s=&ss;
	int i=1;
BEGIN:
	register int n=sla->n;

	n--;

	if(sla->n<1 || sla->n>s->ptm)
		return(0);
	if(s->field[n].a==X_POINTER || s->field[n].a==X_VARIANT)
		return(i);
	if(s->field[n].a==X_STRUCTURE)
	{
		s=s->field[n].st.st;
		sla++;
		i++;
		goto BEGIN;
	}
	return(0);
}
int CX_BASE::is_pointer_recurs(struct sla *sla)
{
	struct st *s;
	s=&ss;
	int i=1;
BEGIN:
	register int n=sla->n;

	n--;
	if(sla->n<1 || sla->n>s->ptm)
		return(0);
	if(s->field[n].a==X_POINTER || s->field[n].a==X_VARIANT)
	{
		if(sla[1].n==0)
			return(i);
		CX_BASE *subbase=NULL;
		struct sla sla_n[SLA_DEEP];
		bzero(sla_n,sizeof sla_n);
		sla_n->n=n+1;
		if((subbase=get_subbase(s,sla_n))==NULL)
			return(0);
		bcopy(sla+1,sla_n,sizeof sla_n);
		return(subbase->is_pointer_recurs(sla_n));
	}
	if(s->field[n].a==X_STRUCTURE)
	{
		s=s->field[n].st.st;
		sla++;
		i++;
		goto BEGIN;
	}
	return(0);
}
int CX_BASE::Str_To_Sla(char *str,struct sla *sla) 
{ 
	if(str==NULL) 
		return(0); 
	if(*str=='^') 
	{ 
		if(atoi(str+1)) 
		{ 
			str_to_sla(str,sla); 
			return(sla->n==0); 
		} 
		str++; 
	} 
	char *line=(char *)malloc(strlen(str)+1); 
	strcpy(line,str); 
	char *begin=line,*end; 
	bzero(sla,sizeof (struct sla)*SLA_DEEP); 
	struct st *s=&ss; 
	for(int i=0;i<SLA_DEEP;i++) 
	{ 
		if((end=strchr(begin,'.'))!=NULL) 
			*end=0; 
		for(sla[i].n=1;sla[i].n<=s->ptm;sla[i].n++) 
		{ 
			if(!strcmp(Get_Field_Descr(s,sla[i].n)->name,begin)) 
				break; 
		} 
		if(sla[i].n>s->ptm) 
			break; 
		if(end==NULL) 
			return(0); 

		int n=sla[i].n-1; 
		if(s->field[n].a!=X_STRUCTURE) 
			break; 
		s=s->field[n].st.st; 
		begin=end+1; 
	} 
	bzero(sla,sizeof (struct sla)*SLA_DEEP); 
	free(line); 
	return(1); 
} 

int CX_BASE::Sla_To_Str(struct sla *sla,char *str) 
{ 
	sla_to_str(sla,str); 
	return(0); 
} 


size_t CX_BASE::Size() 
{ 
	struct stat st; 
	fstat(fd->Fd,&st); 
	return(st.st_size); 
} 

int CX_BASE::Num_Elem_Array(long record,int field) 
{ 
	struct sla sla[SLA_DEEP]; 

	sla[0].n=field; 
	sla[0].m=-1; 
	sla[1].n=0; 
	sla[1].m=0; 

	return(Num_Elem_Array(record,sla)); 

} 

int CX_BASE::Num_Elem_Array(long record,struct sla *SLA) 
{ 
	struct sla sla[SLA_DEEP]; 
	struct field *field=Field_Descr(SLA,record);
	int i=0;
	char *ch=NULL;

	bcopy(SLA,sla,sizeof sla); 
	int array=0; 
	if(field->a) 
	{ 
		for(array=0;array<SLA_DEEP && sla[array].n;array++); 
		for(--array;;array--) 
		{ 
			sla[array].m=0; 
			if(Field_Descr(sla,record)->m)
			{ 
				sla[array].m=-1; 
				array++; 
				break; 
			} 
			if(!array) 
				break; 
			sla[array].n=0; 
		} 
	} 
	if(array || field->a==0) 
	{ 
		struct get_field_result res=Read(record,sla,ch); 
		if(!res.field.a) // Virtual 
		{ 
			if(ch!=NULL) 
				i=atoi(ch); 
			else    i=0; 
		} 
		else    i=res.len; 

		if(ch!=NULL) 
			free(ch); 
		return(i); 
	} 
	else if(array==0 && field->a==X_TEXT)
	{
		Get_Slot(record,sla,ch);
		if(ch!=NULL)
		{
			int len=strlen(ch);
			for(int j=0;j<len;j++)
				if(ch[j]=='\n')
					i++;
			free(ch);
			return(i);
		}
	}
	return(0); 
} 
