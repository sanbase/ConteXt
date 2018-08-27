/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:strdif.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
static int first=1; 
static unsigned char weight[256]; 
static void get_weight(); 
 
/* сравнение строк */ 
int strcmpw(const char *s1,const char *s2,int len) 
{ 
	if(s1==NULL) 
	{ 
		if(s2==NULL) 
			return(0); 
		return(-1); 
	} 
	if(s2==NULL) 
		return(1); 
 
	register int i,j=0; 
	unsigned char *a=(unsigned char *)s1,*b=(unsigned char *)s2; 
 
	if(first) 
		get_weight(); 
	for(i=0;len==0 || i<len;i++,a++,b++) 
	{ 
		j=weight[*a]-weight[*b]; 
		if(j || !(*a) || !(*b)) 
			return(j); 
	} 
	return(j); 
} 
 
int strdif(unsigned char *a,unsigned char *b,int n) 
{ 
	register int i=0,j=0; 
	int old,oldj,next=0,flag1,flag2; 
	if(first) 
		get_weight(); 
	if(b==NULL) 
		return(strcmp((char *)a,"*")); 
NEXT: 
	flag1=flag2=0; 
	for(i=next;a[next] && a[next]!='|';next++) 
		if(a[next]=='*' && i!=next)
			flag2=1;
	if(!a[next]) next=0; 
	if(a[i]=='*') 
	{ 

		flag1=1; 
		i++; 
	} 
	j=0; 
	if(flag1 && !flag2) 
	{ 
		if(next) j=strlen((char *)b)-next+i; 
		else     j=strlen((char *)b)-strlen((char *)a+i); 
	} 
	for(old=i,oldj=j;a[i] && b[j] && i-old<n;i++,j++) 
	{ 
		if(a[i]=='*' && flag2) 
		{
			return(0); 
		}
		if(a[i]==' ' && (b[j]=='\n' || b[j]=='\t'))
			continue;
		if(a[i]=='|' || (a[i]!='?' && weight[a[i]]!=weight[b[j]])) 
		{ 
			if(flag1 && flag2 && a[i]!='|') 
			{ 
				i=old-1;  
				j=oldj++;  
				continue;  
			} 
			if(next) 
			{ 
				next++;  
				goto NEXT; 
			} 
			break; 
		} 
	} 
	if(i-old>=n || ((a[i]==0 || a[i]=='|') && flag2))
		return(0);
	next=a[i]=='|'||a[i]=='*'?0:a[i]; 
	return(weight[next]-weight[b[j]]); 
} 
 
static void get_weight() 
{ 
	register int fd; 
 
	first=0; 
	if((fd=open("/usr/local/etc/.weight",O_RDONLY))<0)
	{ 
		int i; 
		for(i=0;i<256;i++) 
		{ 
			if(i>='a' && i<='z') 
				weight[i]=i-('a'-'A'); 
			else 
				weight[i]=i; 
		} 
	} 
	else 
	{ 
		read(fd,weight,256); 
		close(fd); 
	} 
} 
