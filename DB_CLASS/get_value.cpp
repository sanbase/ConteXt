/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:get_value.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
double CX_BASE::Get_Value(char *str,long record,char **out) 
{ 
	double value=0; 
	struct sla sla[SLA_DEEP]; 
	char *ch=NULL;
 
	char *buf=(char *)malloc(strlen(str)+1); 
	bzero(sla,sizeof sla); 
	strcpy(buf,str); 
	ch=strrchr(buf,']'); 
	if(ch==NULL)         // format error 
		goto END; 
	*ch=0; 
	ch=strrchr(buf,','); 
	if(ch==NULL) 
	{ 
		value=Get_Value(record,str,out); 
		goto END; 
	} 
	*ch=0; 
	ch++; 
	while(*ch==' ' || *ch=='"') ch++; 
	Str_To_Sla(ch,sla); 
 
	ch=strrchr(buf,','); 
	if(ch==NULL) 
	{ 
		record=atoi(buf); 
		value=Get_Value(record,sla,out); 
		goto END; 
	} 
	*ch=0; 
	ch++; 
	while(*ch==' ' || *ch=='"') ch++; 
	record=atoi(ch); 
 
	CX_BASE *db; 
	while(*buf==' ' || *buf=='"') buf++; 
	try 
	{ 
		if((ch=strrchr(buf,'"'))!=NULL) 
			*ch=0; 
		db=open_db(buf); 
	} 
	catch(...) 
	{ 
		goto END; 
	} 
	value=db->Get_Value(record,sla,out); 
	delete db; 
END: 
	free(buf); 
	return(value); 
} 
 
double CX_BASE::Get_Value(long record, char *ch, char **out) 
{ 
	struct sla sla[SLA_DEEP]; 
 
	if(Str_To_Sla(ch,sla)) 
		return(0); 
	return(Get_Value(record,sla,out)); 
} 
 
double CX_BASE::Get_Value(long record, int field, char **out) 
{ 
	struct sla sla[SLA_DEEP]; 
	bzero(sla,sizeof sla); 
	sla->n=field; 
	return(Get_Value(record,sla,out)); 
} 
 
double CX_BASE::Get_Value(long record, struct sla *sla, char **out) 
{ 
	char *ch=NULL; 
	double value=0; 
 
	Get_Slot(record,sla,ch); 
	if(ch!=NULL) 
	{ 
		switch(Field_Descr(sla)->a)
		{
			case X_DATE:
				value=conv_date(ch);
				break;
			case X_TIME:
				value=conv_time(ch);
				break;
			default:
				value=atof(ch);
		}
		if(out!=NULL) 
		{ 
			*out=(char *)realloc(*out,strlen(ch)+1); 
			strcpy(*out,ch); 
		} 
		free(ch); 
	} 
	return(value); 
} 

double CX_BASE::atof(char *s)
{
	if(int_delimiter)
	{

		char str[256],*ch;
		strcpy(str,s);
		if((ch=strchr(str,int_delimiter))!=NULL)
			*ch='.';
		return ::atof(str);
	}
	else
		return ::atof(s);

}
