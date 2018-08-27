/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:atoll.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 

#ifndef atoll 
#ifndef SPARC 
#ifndef LINUX 
dlong atoll(const char *str) 
{ 
	dlong rez=0,pow=1; 
	register int i; 
 
	if(!*str) 
		return(0); 
	for(i=1;str[i];i++) 
		if(str[i]<'0' || str[i]>'9') 
			break; 
	for(--i;i>=0;i--) 
	{ 
		if(i==0 && str[i]=='-') 
			rez=-rez; 
		else 
		{ 
			if(str[i]<'0' || str[i]>'9') 
				continue; 
		        rez+=(str[i]-'0')*pow; 
		} 
		pow*=10; 
	} 
	return(rez); 
} 
#endif 
#endif 
#endif 
char *lota(dlong rez) 
{ 
	static char str[64]; 
	register int i,j; 
	int zn=0; 
 
	if(rez<0) 
	{ 
		rez=-rez; 
		zn=1; 
	} 
	for(i=0;rez>0;i++) 
	{ 
		str[i]=rez%10+'0'; 
		rez/=10; 
	} 
	if(zn) 
		str[i++]='-'; 
	str[i]=0; 
	i--; 
	for(j=0;i>j;i--,j++) 
	{ 
		char a; 
 
		a=str[i]; 
		str[i]=str[j]; 
		str[j]=a; 
	} 
	if(!*str) 
		strcpy(str,"0"); 
	return(str); 
} 
void memcopy(register char *a, register char *b, register int len) 
{ 
	while(len--) 
		*a++=*b++; 
} 
dlong int_conv(char *ch,int len) 
{ 
	dlong a=0; 
	if(ch==NULL) 
		return(a); 
#ifndef SPARC 
	if(len==2) 
		return(*(short *)ch); 
	if(len==4) 
		return(*(long *)ch); 
#endif 
	if(len>(int)sizeof (dlong)) 
		len=sizeof (dlong); 
	char d[sizeof (dlong)]; 
	bcopy(ch,d,len); 
#ifdef SPARC 
	conv(d,len); 
	bcopy(d,d+(sizeof d) -len,len); 
	if((*(ch+len-1))&0x80) 
		memset(d,0xff,(sizeof d)-len); 
	else    memset(d,0,(sizeof d)-len); 
#else 
	if((*(ch+len-1))&0x80) 
		memset(d+len,0xff,(sizeof (dlong))-len); 
	else    memset(d+len,0,(sizeof (dlong))-len); 
#endif 
	bcopy(d,&a,sizeof (dlong)); 

	return(a); 
} 
 
dlong unsigned_conv(char *ch,int len) 
{ 
	dlong a=0; 
	if(ch==NULL) 
		return(a); 
	if(len>(int)sizeof (dlong)) 
		len=sizeof (dlong); 
	char d[sizeof (dlong)]; 
	bcopy(ch,d,len); 
#ifdef SPARC 
	conv(d,len); 
	bcopy(d,d+(sizeof d) -len,len); 
	memset(d,0,(sizeof d)-len); 
#else 
	memset(d+len,0,(sizeof (dlong))-len); 
#endif 
	bcopy(d,&a,sizeof (dlong)); 
	return(a); 
} 
 
void int_to_buf(char *ch, int len, dlong a) 
{ 
#ifdef SPARC 
	char *d=(char *)&a; 
	bcopy(d+(sizeof (dlong))-len,ch,len); 
	conv(ch,len); 
#else 
	bcopy(&a,ch,len); 
#endif 
	return; 
} 
 
#ifdef SPARC 
void conv(char *a,int len) 
{ 
	char b[sizeof (dlong)]; 
 
	bzero(b,sizeof b); 
	if(len>sizeof (dlong)) 
		len=sizeof (dlong); 
	bcopy(a,b,len); 
	for(int i=0;i<len;i++) 
		a[i]=b[len-i-1]; 
} 
#endif 
