/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:convers.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term; 
 
char *lota(dlong); 
 
int CX_BASE::Buf_To_String(char *ch, struct get_field_result *res,char *&slot) 
{ 
	char str[64]; 
	int len=res->len; 
	if(slot!=NULL) 
		free(slot); 
	slot=NULL; 
 
	if(len==0 || ch==NULL) 
	{ 
		slot=(char *)calloc(4,1); 
		return(0); 
	} 
	if(res->field.a==X_STRING) 
	{ 
		slot=(char *)malloc(len+1); 
		bcopy(ch,slot,len); 
		slot[len]=0; 
		return(len); 
	} 
	if(res->field.a>20) // User defined field
	{ 
		return(-1); 
	} 
	if(res->field.a==X_TEXT || res->field.a==X_BINARY || res->field.a==X_FILENAME
	|| res->field.a==X_EXPRESSION || res->field.a==X_IMAGE || res->field.a==X_COMPLEX)
	{ 
		short key=0; 
		bcopy(ch,&key,2); 
#ifdef SPARC 
		conv((char *)&key,2); 
#endif 
		if(key==-29922) 
		{ 
			char *chr=(char *)malloc(len); 
			bcopy(ch,chr,len); 
			len=decompress(chr,len,slot); 
			free(chr); 
			if(len<0) 
				return(0); 
		} 
		else 
		{ 
			slot=(char *)malloc(len); 
			bcopy(ch,slot,len); 
		} 
		if(res->field.a!=X_BINARY && res->field.a!=X_IMAGE) 
		{ 
			slot=(char *)realloc(slot,len+1); 
			slot[len]=0; 
		} 
		return(len); 
	} 
	if((len=res->field.l)<LINESIZE) 
		len=LINESIZE; 
	slot=(char *)calloc(len+1,1); 
	switch(res->field.a) 
	{ 
		case X_STRUCTURE: 
		{ 
			slot=(char *)realloc(slot,len+=1); 
			strcat(slot,"{"); 
			for(int i=0;i<res->field.st.st->ptm;i++) 
			{ 
				char *buf=NULL; 
				struct get_field_result fld; 
 
				bcopy(res,&fld,sizeof fld); 
				fld.field=res->field.st.st->field[i]; 
				fld.sla[1].n=i+1; 
				int l=Buf_To_String(ch+fld.field.atr.attr->wshift,&fld,buf); 
				slot=(char *)realloc(slot,len+=l+2); 
				if(i>0) 
					strcat(slot,";"); 
				strcat(slot,buf); 
				free(buf); 
			} 
			slot=(char *)realloc(slot,len+=1); 
			strcat(slot,"}"); 
			return(len); 
		} 
		case X_DATE: 
			get_date(date_from_rec(ch),slot); 
			goto END; 
		case X_TIME: 
			if(res->field.l==3) 
				get_time(date_from_rec(ch),slot); 
			else 
				get_unix_time((int)int_conv(ch,res->field.l),slot); 
			goto END; 
		case X_DOUBLE: 
		{ 
			if(res->field.n>16) 
				strcpy(str,"%.6f"); 
			else 
				sprintf(str,"%%.%df",res->field.n); 
			sprintf(slot,str,*(double *)ch); 
			goto END; 
		} 
		case X_POINTER: 
		{ 
 
			if(res->field.name!=NULL) 
			{ 
				if(strchr(res->field.name,':')!=NULL && res->sla[1].n) 
				{ 
					slot=(char *)realloc(slot,(len=res->len)+1); 
					bcopy(ch,slot,len); 
					slot[len]=0; 
					return(len); 
				} 
			} 
			long page=int_conv(ch,res->field.l); 
			sprintf(slot,"#%d",(int)page); 
			goto END; 
		} 
		case X_VARIANT: 
		{ 
			long page=0,spage=0; 
		       // bcopy(ch,&spage,res->field.n); fill
		       // bcopy(ch+res->field.n,&page,res->field.l-res->field.n);
			spage=int_conv(ch,res->field.n);
			page=int_conv(ch+res->field.n,res->field.l-res->field.n);
			sprintf(slot,"#%d:%d",(int)spage,(int)page); 
			goto END; 
		} 
		case X_FLOAT: 
		{ 
			if(res->field.n>6) 
				strcpy(str,"%.6f"); 
			else 
				sprintf(str,"%%.%df",res->field.n); 
			sprintf(slot,str,*(float *)ch); 
			goto END; 
		} 
		case X_INTEGER: 
		case X_UNSIGNED: 
		{ 
			dlong a=0; 
 
			if(res->field.a==X_UNSIGNED) 
				bcopy(ch,&a,res->field.l); 
			else 
				a=int_conv(ch,res->field.l); 
			if(res->context.pswd==CXKEY4) 
			{ 
				if(a==0) 
					return(len); 
				if(a>0) 
					a--; 
			} 
			if(res->field.n) 
			{ 
				int j; 
				if(a>0) 
					sprintf(str,"%%0%dlld",res->field.n); 
				else 
					sprintf(str,"%%0%dlld",res->field.n+1); 
				sprintf(slot,str,a);
				j=strlen(slot);
				bcopy(slot+(j-res->field.n),slot+(j-res->field.n+1),res->field.n+1); 
				slot[j-res->field.n]=int_delimiter;
				slot[j+1]=0;
			} 
			else 
#ifdef FREEBSD 
				sprintf(slot,"%s",lota(a)); 
#else 
				sprintf(slot,"%lld",a); 
#endif 
			goto END; 
		} 
		default: 
			break; 
	} 
	return(-1); 
END: 
	if(slot!=NULL) 
	{ 
		len=strlen(slot); 
		slot=(char *)realloc(slot,len+1); 
	} 
	return(len); 
} 
 
int CX_BASE::String_To_Buf(char *slot, struct field field, char *&ch,int no_compress,int field_size,long record) 
{ 
	dlong i; 
	int len=field.l; 
	int l=field.n; 
	if(l<1 || l>4) 
		l=4; 
	ch=(char *)calloc(field.l,4); 
	if(field.a>X_COMPLEX) // User defined field 
	{ 
		return(-1); 
	} 
	switch(field.a) 
	{ 
		case 0: // VIRTUAL field 
			break; 
		case X_STRING: 
			strncpy(ch,slot,field.l); 
			break; 
		case X_FLOAT: 
			if(!string_digit(slot)) 
				*(float *)ch=(float)Expression(record,slot); 
			else    *(float *)ch=(float)::atof(slot);
			break; 
		case X_DOUBLE: 
			if(!string_digit(slot)) 
				*(double *)ch=Expression(record,slot); 
			else    *(double *)ch=::atof(slot);
			break; 
		case X_POINTER: 
			if(*slot=='#') 
				i=atoi(slot+1); 
			else 
				i=atoi(slot); 
			int_to_buf(ch,field.l,i); 
			break; 
		case X_VARIANT: 
		{ 
			long page=0,spage=0; 
			char *dlm=NULL;
			if(*slot=='#') 
			{ 
				spage=atoi(slot+1); 
				dlm=strchr(slot+1,':'); 
			} 
			else 
			{ 
				spage=atoi(slot); 
				dlm=strchr(slot,':'); 
			} 
			if(dlm!=NULL) 
				page=atoi(dlm+1); 
			int_to_buf(ch,field.n,spage); 
			int_to_buf(ch+field.n,field.l-field.n,page); 
			break; 
		} 
		case X_TEXT: 
		case X_FILENAME: 
			len=strlen(slot); 
/*
			if(!no_compress && len>1024) 
			{ 
				free(ch); 
				ch=NULL; 
				if((len=compress(slot,len,ch))<0) 
					break; 
				ch=(char *)realloc(ch,len+l); 
				bcopy(ch,ch+l,len); 
				bcopy(&len,ch,l); 
				break; 
			} 
*/
			ch=(char *)realloc(ch,len+l); 
			bcopy(slot,ch+l,len); 
			len+=l;
			bcopy(&len,ch,l); 
			break; 
		case X_DATE: 
			i=conv_date(slot); 
			int_to_buf(ch,field.l,i); 
			break; 
		case X_TIME: 
			i=conv_time(slot); 
			int_to_buf(ch,field.l,i); 
			break; 
		case X_INTEGER: 
		case X_UNSIGNED: 
		{ 
			char str[LINESIZE],*a; 
 
			if(slot[0]=='0' && slot[1]=='x') 
			{ 
				sprintf(str,"%ld",strtol(slot,NULL,16)); 
			} 
			else if(!string_digit(slot,int_delimiter))
			{ 
				char form[64]; 
				sprintf(form,"%%f.%d",field.n); 
				sprintf(str,form,Expression(record,slot)); 
			} 
			else 
				strcpy(str,slot); 
			if((a=strchr(str,int_delimiter))!=NULL || (a=strchr(str,'.'))!=NULL)
			{ 
				*(a+field.n+1)=0; 
				bcopy(a+1,a,field.n+1); 
				i=atoll(str); 
				for(int j=field.n-(strlen(str)-(a-str));j>0;j--) 
					i*=10; 
 
			} 
			else 
			{ 
				i=atoll(str); 
				for(int j=field.n;j;j--) 
					i*=10; 
			} 
			if(field.a==X_UNSIGNED && i<0) 
				i=-i; 
			if(i>=0 && *slot && context->pswd==CXKEY4) 
				i++; 
			int_to_buf(ch,field.l,i); 
			break; 
		} 
		case X_STRUCTURE: 
		{ 
			int l=0; 
			for(i=0;slot[i];i++) 
			{ 
				if(slot[i]=='{') 
					l++; 
				if(slot[i]=='}') 
					l--; 
			} 
			if(l!=0) 
				return(-2); 
			char *buf=(char *)malloc(strlen(slot)); 
			if(*slot=='{') 
			{ 
				strcpy(buf,slot+1); 
				l=1; 
			} 
			else 
				strcpy(buf,slot); 
			char *n=NULL,*b=buf; 
			for(i=0;l && b[i];i++) 
			{ 
				if(b[i]=='{') 
					l++; 
				if(b[i]=='}') 
					l--; 
				if(l==0) 
				{ 
					b[i]=0; 
					break; 
				} 
			} 
			l=0; 
			for(int j=0;j<field.st.st->ptm;j++) 
			{ 
				char *chr=NULL;
				struct field fld=field.st.st->field[j]; 
 
				if((chr=strchr(b,';'))!=NULL) 
				{ 
					if(fld.a!=X_STRUCTURE) 
						*chr=0; 
				} 
				char *c=NULL; 
				int lenf=String_To_Buf(b,fld,c,no_compress,field_size,record); 
				if(lenf<0) 
				{ 
					free(buf); 
					return(lenf); 
				} 
				bcopy(c,(ch)+l,lenf); 
				l+=lenf; 
				free(c); 
				if(fld.a!=X_STRUCTURE && chr!=NULL) 
				{ 
					b=chr+1; 
				} 
				else 
				{ 
					int lv=0; 
					for(i=0;b[i];i++) 
					{ 
						if(b[i]=='{') 
							lv++; 
						if(b[i]=='}') 
							lv--; 
						if(lv==0) 
						{ 
							b[i]=0; 
							n=buf+i+1; 
							if(*n==';') 
								n++; 
							break; 
						} 
					} 
					b=n; 
				} 
				if(!*b) 
					break; 
			} 
			free(buf); 
			return(field.l); 
		} 
		default: 
			if(field_size==0) 
			{ 
				bcopy(slot,&len,sizeof len); 
				if(!no_compress && len>1024) 
				{ 
					free(ch); 
					ch=NULL; 
					len=compress(slot+sizeof (long),len-sizeof (long),ch); 
					ch=(char *)realloc(ch,len+sizeof(long)); 
					bcopy(ch,ch+sizeof(long),len); 
					bcopy(&len,ch,sizeof (long)); 
					break; 
				} 
				if((ch=(char *)realloc(ch,len+sizeof (long)))==NULL) 
					break; 
				bcopy(slot,ch,len+=sizeof (long)); 
			} 
			else 
			{ 
				len=field_size; 
				if(!no_compress && len>1024)
				{ 
					free(ch); 
					ch=NULL; 
					len=compress(slot,len,ch); 
					if((ch=(char *)realloc(ch,len+sizeof (long)))==NULL) 
						break; 
					bcopy(ch,ch+sizeof(long),len); 
					bcopy(&len,ch,sizeof (long)); 
					break; 
				} 

				if((ch=(char *)realloc(ch,len+sizeof (long)))==NULL) 
					break; 
				bcopy(slot,ch+sizeof (long),len);
				len+=sizeof (long);
				bcopy(&len,ch,sizeof (long));
			} 
			break; 
	} 
	if(field.k && context->pswd!=CXKEY3) 
	{ 
		ch=(char *)realloc(ch,len+sizeof (long)); 
		bcopy(&record,ch+len,sizeof (long)); 
	} 
	return(field.l); 
} 
 
void sla_to_str(struct sla *sla,char *str) 
{ 
	int i; 
 
	strcpy(str,"^"); 
	for(i=0;i<SLA_DEEP;i++) 
	{ 
		char str1[64]; 
 
		if(sla[i].n==0) 
		{ 
break; 
			int j; 
 
			for(j=i;j<SLA_DEEP;j++) 
				if(sla[j].n) 
					break; 
			if(j==SLA_DEEP) 
				break; 
		} 
		if(i) 
		{ 
			if(sla[i].n<0) 
				sprintf(str1,".X"); 
			else 
				sprintf(str1,".%d",sla[i].n); 
		} 
		else 
		{ 
			if(sla[i].n<0) 
				sprintf(str1,"X"); 
			else 
				sprintf(str1,"%d",sla[i].n); 
		} 
		strcat(str,str1); 
		if(sla[i].m) 
		{ 
			if(sla[i].m<0) 
				sprintf(str1,"[X]"); 
			else 
				sprintf(str1,"[%d]",sla[i].m); 
			strcat(str,str1); 
		} 
	} 
} 
 
int str_to_sla(char *str1,struct sla *sla) 
{ 
	char *str=NULL;
	char *ch=NULL,*ch1=NULL;
	int i=0; 
 
	str=(char *)malloc(strlen(str1)); 
	strcpy(str,str1+(*str1=='^')); 
	ch1=ch=str; 
	bzero(sla,SLA_DEEP*sizeof (struct sla)); 
	for(;;) 
	{ 
		int j; 
 
		if(++i>=SLA_DEEP) 
		{ 
			free(str); 
			return(i); 
		} 
 
		sla[i].n=0; 
		sla[i].m=0; 
		if(*ch1=='X' || *ch1=='x') 
			sla[i-1].n=-1; 
		else 
			sla[i-1].n=atoi(ch1); 
		sla[i-1].m=0; 
		for(j=0;ch1[j];j++) 
		{ 
			if(ch1[j]=='.') 
				break; 
			if(ch1[j]=='[') 
			{ 
				if(ch1[j+1]=='X'||ch1[j+1]=='x') 
					sla[i-1].m=-1; 
				else 
					sla[i-1].m=atoi(ch1+j+1); 
				while(ch1[j] && ch1[j]!=']') 
					j++; 
				ch1+=j; 
				break; 
			} 
			if(ch1[j]==',') // old style 
			{ 
				int n; 
				for(n=0;sla[n].m;n++); 
				if(ch1[j+1]=='X'||ch1[j+1]=='x') 
					sla[n].m=-1; 
				else 
					sla[n].m=atoi(ch1+j+1); 
				while(ch1[j] && (ch1[j]>='0' && ch1[j]<='9')) 
					j++; 
				ch1+=(j-1); 
				break; 
 
			} 
		} 
		if((ch=strchr(ch1,'.'))!=NULL) 
			*ch=0; 
		if(ch!=NULL) 
			ch1=ch+1; 
		else 
			break; 
	} 
	free(str); 
	return(i); 
} 
 
void conv_money(char *str) 
{ 
	char line[64]; 
	int i,j,flag; 
	i=strlen(str)-1; 
	if(strchr(str,'.')==NULL) 
		flag=1; 
	else flag=0; 
	for(j=0;i>=0;j++,i--) 
	{ 
		line[j]=str[i]; 
		if(str[i]=='.') 
		{ 
			flag=1; 
			continue; 
		} 
		if(flag&&i) 
		{ 
			if(flag>=3 && str[i-1]!='-') 
			{ 
				line[++j]='`'; 
				flag=0; 
			} 
			flag++; 
		} 
	} 
	line[j]=0; 
 
	j=strlen(line)-1; 
	for(i=0;line[i];i++) 
		str[i]=line[j-i]; 
	str[i]=0; 
} 
 
int slacmp(struct sla *a,struct sla *b) 
{ 
	int i; 
	for(i=0;a[i].n && b[i].n;i++) 
	{ 
		if(a[i].n!=b[i].n) 
			return(a[i].n-b[i].n); 
		if(a[i].m!=b[i].m) 
			return(a[i].m-b[i].m); 
	} 
	return(a[i].n-b[i].n); 
} 
