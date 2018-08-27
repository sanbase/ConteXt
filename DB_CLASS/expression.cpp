/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:expression.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
void formtrans(char **str); 
 
double CX_BASE::calculation(long record,char *str) 
{ 
	double rez,value; 
	char *beg=NULL;
	char *out=(char *)malloc(strlen(str)+1); 
	strcpy(out,str); 
B1: 
	beg=NULL; 
	for(register int i=0;out[i];i++) 
	{ 
		if(out[i]=='*' || out[i]=='/') 
		{ 
			beg=out+i; 
			char op=*beg; 
 
			for(i=beg-out-1;i>=0;i--) 
				if(out[i]=='+' || out[i]=='-' || out[i]=='*' || out[i]=='/') 
					break; 
			if(!bcmp(out+i+1,"CX[",3)) 
				rez=Get_Value(out+i+4,record); 
			else if(*(out+i+1)=='^') 
				rez=Get_Value(record,out+i+1); 
			else 
				rez=atof(out+i+1);
			out[i+1]=0; 
			if(*(beg+1)=='^') 
				value=Get_Value(record,beg+1); 
			else    value=atof(beg+1);
			if(op=='*') 
				rez*=value; 
			else    rez/=value; 
			for(i=beg-out+1;out[i];i++) 
				if(out[i]=='+' || out[i]=='-' || out[i]=='*' || out[i]=='/') 
					break; 
			char *out1=(char *)malloc(strlen(out)+32+strlen(out+i)); 
			sprintf(out1,"%s%f%s",out,rez,out+i); 
			out=(char *)realloc(out,strlen(out1)+1); 
			strcpy(out,out1); 
			free(out1); 
			goto B1; 
		} 
	} 
	if(strlen(out)>=3 && !bcmp(out,"CX[",3)) 
		rez=Get_Value(out+3,record); 
	else if(*out=='^') 
		rez=Get_Value(record,out); 
	else    rez=atof(out);
	beg=out; 
	while(*beg=='-' || *beg=='+') 
		beg++; 
BEG: 
	while(*beg && *beg!='+' && *beg!='-') 
		beg++; 
	if(!*beg) 
	{ 
		free(out); 
		return(rez); 
	} 
	if(*(beg+1)=='^') 
		value=Get_Value(record,beg+1); 
	else    value=atof(beg+1);
	switch(*beg) 
	{ 
		case '+': 
			rez+=value; 
			break; 
		case '-': 
			rez-=value; 
			break; 
	} 
	beg++; 
	goto BEG; 
} 
 
 
double CX_BASE::Expression(long record, char *str) 
{ 
	char *beg=NULL,*end=NULL;
	double rez=0; 
	int i; 
	char *out=(char *)malloc(strlen(str)+1); 
	strcpy(out,str); 
 
	beg=out; 
	while((beg=strchr(beg,' '))!=NULL) 
	{ 
		bcopy(beg+1,beg,strlen(beg)); 
		beg++; 
	} 
BEG: 
	if((beg=strrchr(out,'('))!=NULL) 
	{ 
		if((end=strchr(beg,')'))==NULL) 
		{ 
			free(out); 
			return(0);      /* нарушен балланс скобок */ 
		} 
		*end=0; 
		*beg=0; 
		for(i=1;beg[i];i++) 
			if(beg[i]=='+' || beg[i]=='-' || beg[i]=='*' || beg[i]=='/') 
				break; 
		if(beg[i]==0)   /* что-то типа ((3.5)) */ 
		{ 
			bcopy(beg+1,beg,strlen(beg+1)); 
			strcat(out,end+1); 
			goto BEG; 
		} 
		rez=calculation(record,beg+1); 
		char *out1=(char *)malloc(strlen(out)+strlen(end+1)+32); 
		sprintf(out1,"%s%f%s",out,rez,end+1); 
		out=(char *)realloc(out,strlen(out1)+1); 
		strcpy(out,out1); 
		free(out1); 
		goto BEG; 
	} 
	rez=calculation(record,out); 
	free(out); 
	return(rez); 
} 
