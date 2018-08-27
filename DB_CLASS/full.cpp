/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:full.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
int PathName(char *pathname, char *path, char *name) 
{ 
	char *ch; 
 
	if( !pathname ) 
	{ 
		return -1; 
	} 
	if( path && (!name || *name != '/') ) 
	{ 
 
		if( pathname != path ) 
			strcpy( pathname, path ); 
		if((ch=strchr(pathname,'\n'))!=NULL) 
			*ch=0; 
		if( *pathname && pathname[strlen(pathname)-1] != '/' ) 
			strcat( pathname, "/" ); 
	} 
	else 
		*pathname = 0; 
	if( name ) 
	{ 
		int i,j; 
 
		if((ch=strchr(name,'\n'))==NULL) 
			j=strlen(name); 
		else    j=ch-name; 
 
		i=strlen(pathname); 
 
		strncat( pathname, name, j); 
		if(ch!=NULL) 
			pathname[i+j]=0; 
	} 
	return 0; 
} 
 
int HeadName(char *headname, char *basename) 
{ 
	if( basename ) 
	{ 
		char *s = strrchr( basename, '/' ); 
		return PathName( headname, basename, s ? s + 1 : basename ); 
	} 
	return -1; 
} 
 
int full(char *a,char *b,char *l) 
{ 
	if(a==NULL || !*a) 
	{ 
		strcpy(l,b); 
		return(0); 
	} 
	if(b==NULL || !*b) 
	{ 
		strcpy(l,a); 
		return(0); 
	} 
	return (!strcmp(a, b)) ? HeadName( l, a ) : PathName( l, a, b ); 
} 
