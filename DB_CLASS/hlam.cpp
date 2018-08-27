/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:hlam.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
int if_base(char *dir,char *name) 
{ 
	char *header_name=NULL;
	int fd; 
	int key; 
 
	if(name==NULL || !*name) 
		return(0); 
 
	if(dir==NULL || !*dir || !strcmp(dir,".")) 
	{ 
		header_name=(char *)malloc(2*strlen(name)+2); 
		full(name,name,header_name); 
	} 
	else 
	{ 
		header_name=(char *)malloc(strlen(dir)+2*strlen(name)+3); 
		char *ch=strrchr(name,'/'); 
		sprintf(header_name,"%s/%s/%s",dir,name,ch==NULL?name:ch); 
	} 
	decode(header_name);
	if((fd=open(header_name,O_RDONLY|O_BINARY))<0)
	{ 
		free(header_name); 
		return(0); 
	} 
	free(header_name); 
	read(fd,&key,sizeof key); 
	close(fd); 
#ifdef SPARC 
	conv((char *)&key,4); 
#endif 
	if(key==CXKEY6)
		return(6);
	if(key==CXKEY5) 
		return(5); 
	if(key==CXKEY4) 
		return(4); 
	if(key==CXKEY3 || key==5472566 || key==(-CXKEY3)) 
		return(3); 
	return(0); 
} 
 
int if_dir(char *dir,char *name) 
{ 
	char *full_name=NULL;
	int i; 
	struct stat st; 
/* 
	if(dir==NULL || !*dir || !strcmp(dir,".")) 
		return(!access(name,R_OK)); 
*/ 
	full_name=(char *)malloc(strlen(dir)+strlen(name)+2); 
	full(dir,name,full_name); 
#ifdef WIN32 
	i=!stat(full_name,&st) && (S_IFDIR&st.st_mode); 
#else 
	i=!stat(full_name,&st) && S_ISDIR(st.st_mode); 
#endif 
	free(full_name); 
	return(i); 
} 
int if_file(char *dir,char *name) 
{ 
	char *full_name; 
	int i; 
	struct stat st; 
 
	full_name=(char *)malloc(strlen(dir)+strlen(name)+2); 
	full(dir,name,full_name); 
#ifdef WIN32 
	i=!stat(full_name,&st) && (S_IFDIR&st.st_mode) || !access(full_name,R_OK); 
#else 
	i=!stat(full_name,&st) && S_ISDIR(st.st_mode) || !access(full_name,R_OK); 
#endif 
	free(full_name); 
	return(i); 
} 
 
 
int if_read(char *name) 
{ 
	return(if_stat(name,4)); 
} 
int if_write(char *name) 
{ 
	return(if_stat(name,2)); 
} 
int if_exec(char *name) 
{ 
	return(if_stat(name,1)); 
} 
int if_stat(char *name,int status) 
{ 
	return(if_stat("",name,status)); 
} 
int if_read(char *dir,char *name) 
{ 
	return(if_stat(dir,name,4)); 
} 
int if_write(char *dir,char *name) 
{ 
	return(if_stat(dir,name,2)); 
} 
int if_exec(char *dir,char *name) 
{ 
	return(if_stat(dir,name,1)); 
} 
int if_stat(char *dir,char *name,int status) 
{ 
 
	char *full_name=NULL;
	int i; 
	struct stat st; 
 
	status&=7; 
	full_name=(char *)malloc(strlen(dir)+strlen(name)+2); 
	full(dir,name,full_name); 
 
	i=stat(full_name,&st); 
	if(i!=0) 
		goto END; 
#ifndef WIN32 
	if(getuid()==st.st_uid) 
		st.st_mode>>=6; 
	else if(getgid()==st.st_gid) 
		st.st_mode>>=3; 
#endif 
	i=(st.st_mode&status)==0; 
END: 
	free(full_name); 
	return(!i); 
} 
 
int atoo(char *str) 
{ 
	register int n=0; 
 
	while(*str >= '0' && *str <= '9') 
		n = n*8 + *str++ - '0'; 
	return(n); 
} 
 
int atox(char *str) 
{ 
	register int n=0; 
 
	while(*str >= '0' && *str <= '9' || *str>='a' && *str<='f' || *str>='A' && *str<='F') 
	{ 
		if(*str <='9') 
			n = n*16 + *str++ - '0'; 
		else if(*str>='a' && *str<='f') 
			n = n*16 + 10 + *str++ - 'a'; 
		else 
			n = n*16 + 10 + *str++ - 'A'; 
	} 
	return(n); 
} 
 
/* áª®¯¨à®¢ âì ä ©« b ¢ ä ©« a. …á«¨ ¥£® ¥é¥ ­¥â - á®§¤ âì */ 
int fcopy(char *a,char *b) 
{ 
	int fd,df,size; 
	char buf[1024]; 
	off_t length=0; 
 
	if((fd=open(b,O_RDONLY|O_BINARY))<0)
		return(-1); 
	if((df=open(a,O_RDWR|O_CREAT|O_BINARY,S_IREAD|S_IWRITE))<0)
	{ 
		close(fd); 
		return(-2); 
	} 
	while((size=read(fd,buf,1024))>0) 
	{ 
		write(df,buf,size); 
		length+=size; 
	} 
#ifndef WIN32 
	ftruncate(df,length); 
#endif 
	close(fd); 
	close(df); 
	return(0); 
} 
 
int string_digit(char *str,char delimiter)
{ 
	register char *ch=str; 
	for(;*ch;ch++) 
	{ 
		if((*ch<'0' || *ch>'9') && (*ch!=delimiter && *ch!='.'))
		{ 
			  if(ch==str && (*ch=='-' || *ch=='+')) 
				continue; 
			  return(0); 
		} 
	} 
	return(1); 
} 
 
char *GetLogin() 
{ 
#ifndef WIN32 
	char *ret=NULL;
	ret=getenv("CXUSER");
	if(ret==NULL&&(ret=getlogin())==NULL)
		ret=getenv("USER"); 
	if(ret==NULL) 
		return("nobody"); 
	return(ret); 
#else 
	return("user"); 
#endif 
} 
 
#ifdef SPARC 
void bcopy(const void *s1, void *s2, size_t n) 
{ 
	memcpy(s2,s1,n); 
} 
 
int bcmp(const void *s1, const void *s2, size_t n) 
{ 
	return(memcmp(s1,s2,n)); 
} 
 
void bzero(void *s, size_t n) 
{ 
	memset(s,0,n); 
} 
 
#endif 
 
/* 
void get_user_dir(char **name, char *base, char *dir) 
{ 
	struct stat st; 
	char *user; 
	*name = (char *)realloc(*name,strlen(base)+strlen(dir)+strlen(user=GetLogin())+3); 
	sprintf(*name,"%s/%s",base,user); 
	if(stat(*name,&st) || !(st.st_mode&S_IFDIR)) 
	{ 
		sprintf(*name,"%s/%s",base,dir); 
		return; 
	} 
	sprintf(*name,"%s/%s/%s",base,user,dir); 
} 
*/ 
 
char *pc_demos(char *str) 
{ 
	static char *buf=NULL; 
	const char *tabl="áâ÷çäåöúéêëìíîïðòóôõæ ãþûý ”èüàñÁÂ×ÇÄÅÖÚÉÊËÌÍÎÏÐ‘’‡²´§¦µ¡¨®­¬ƒ„‰ˆ†€Š¯°«¥»¸± ¾¹º¶ ª©¢¤½¼…‚ŒŽ‹ÒÓÔÕÆÈÃÞÛÝßÙØÜÀÑ•–—˜™š›œžŸ£³ “¿"; 
 
	buf=(char *)realloc(buf,strlen(str)+1); 
	strcpy((char *)buf,str); 
	for(int j=0;buf[j];j++) 
	{ 
		int i=(unsigned char)buf[j]; 
		if(i>127) 
			buf[j]=tabl[i-128]; 
	} 
	return(buf); 
} 
 
int cmp_type(struct field *des_field,char *ch1,char *ch2) 
{ 
	int j=0; 
	switch(des_field->a) 
	{ 
		case X_DATE: 
			j=(conv_date(ch1)-conv_date(ch2)); 
			break; 
		case X_TIME: 
			j=(conv_time(ch1)-conv_time(ch2)); 
			break; 
		case X_POINTER:
			j=(atoi(ch1+1)-atoi(ch2+1)); 
			break; 
		case X_INTEGER: 
		case X_UNSIGNED: 
			if(des_field->n==0) 
			{ 
				j=(atoll(ch1)-atoll(ch2)); 
				break; 
			} 
		case X_DOUBLE: 
		case X_FLOAT: 
			j=(dlong)(::atof(ch1)-::atof(ch2));
			break; 
		case X_STRING: 
			j=strcmpw(ch1,ch2,des_field->l); 
			break; 
		default: 
			j=strcmpw(ch1,ch1); 
			break; 
	} 
	return(j); 
} 

void bincopy(const void *src, void *dst, int len)
{
	register char *a=(char *)src;
	register char *b=(char *)dst;
	if(src==NULL || dst==NULL || len==0)
		return;
	for(;len>0;len--,b++, a++)
		*b=*a;
}

void decode(char *buf)
{
	for(int i=0;buf[i];i++)
	{
		buf[i]=buf[i];  // áî¤  ¢áâ ¢¨âì ¯¥à¥ª®¤¨à®¢ªã
	}
}

#ifdef WIN32 
void bcopy(const void *a,void *b,size_t len) 
{ 
		memmove(b,a,len);
} 
int  bcmp(const void *a, const void *b,size_t len) 
{ 
		return(memcmp(a,b,len)); 
} 
void bzero(void *b, size_t len) 
{ 
		memset(b,0,len); 
}
/*
int getpid() 
{ 
		return(0); 
}
*/
#endif 
