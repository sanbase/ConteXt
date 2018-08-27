/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:dir.cpp
*/
#include "StdAfx.h" 
#define X11 
#include "../CX_Browser.h" 
#include <stdlib.h> 
 
extern Terminal *term; 
extern Line *EditLine; 
 
 
#ifdef WIN32 
#ifndef	__STRICT_ANSI__ 
 
#ifndef _DIRENT_H_ 
#define _DIRENT_H_ 
 
/* All the headers include this file. */ 
//#include <_mingw.h> 
 
// ugly hack for MSVC 
#if defined(_POSIX_) 
 #undef _POSIX_ 
 #include <io.h> 
 #define _POSIX_ 
#else 
 #include <io.h> 
#endif 
 
 
#ifndef RC_INVOKED 
 
#ifdef __cplusplus 
extern "C" { 
#endif 
 
struct dirent 
{ 
	long		d_ino;		/* Always zero. */ 
	unsigned short	d_reclen;	/* Always zero. */ 
	unsigned short	d_namlen;	/* Length of name in d_name. */ 
	char*		d_name;		/* File name. */ 
	/* NOTE: The name in the dirent structure points to the name in the
	 *       finddata_t structure in the DIR. */ 
}; 
 
/* 
 * This is an internal data structure. Good programmers will not use it 
 * except as an argument to one of the functions below. 
 */ 
typedef struct 
{ 
	/* disk transfer area for this dir */ 
	struct _finddata_t	dd_dta; 
 
	/* dirent struct to return from dir (NOTE: this makes this thread
	 * safe as long as only one thread uses a particular DIR struct at
	 * a time) */ 
	struct dirent		dd_dir; 
 
	/* _findnext handle */ 
	long			dd_handle; 
 
	/* 
         * Status of search: 
	 *   0 = not started yet (next entry to read is first entry) 
	 *  -1 = off the end 
	 *   positive = 0 based index of next entry 
	 */ 
	short			dd_stat; 
 
	/* given path for dir with search pattern (struct is extended) */
	char			dd_name[1]; 
} DIR; 
 
 
DIR*		opendir (const char*); 
struct dirent*	readdir (DIR*); 
int		closedir (DIR*); 
void		rewinddir (DIR*); 
long		telldir (DIR*); 
void		seekdir (DIR*, long); 
 
#ifdef	__cplusplus 
} 
#endif 
 
#endif	/* Not RC_INVOKED */ 
#endif	/* Not _DIRENT_H_ */ 
#endif	/* Not __STRICT_ANSI__ */ 
 
/* 
 * dirent.c 
 * 
 * Derived from DIRLIB.C by Matt J. Weinstein  
 * This note appears in the DIRLIB.H 
 * DIRLIB.H by M. J. Weinstein   Released to public domain 1-Jan-89 
 * 
 * Updated by Jeremy Bettis <jeremy@hksys.com> 
 * Significantly revised and rewinddir, seekdir and telldir added by Colin
 * Peters <colin@fu.is.saga-u.ac.jp> 
 * 
 * 
 */ 

#include <direct.h> 
//#include <dirent.h> 
#include <errno.h> 
#include <io.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/stat.h> 
 
#define SUFFIX	"*" 
#define	SLASH	"\\" 
 
#ifndef S_ISDIR 
#define S_ISDIR(m)	((m & S_IFMT) == S_IFDIR)	/* is a directory */
#endif

int errno; 
 
/* 
  opendir 
 
  Returns a pointer to a DIR structure appropriately filled in to begin 
  searching a directory. 
*/ 
DIR *opendir (const char *szPath) 
{ 
	DIR        *nd; 
	struct _stat statDir; 
 
	errno = 0; 
 
	if (!szPath)
	{
		errno = EFAULT; 
		return (DIR *) 0; 
	} 
 
	if (szPath[0] == '\0')
	{
		errno = ENOTDIR; 
		return (DIR *) 0; 
	} 
 
	/* Attempt to determine if the given path really is a directory. */
	if (_stat (szPath, &statDir))
	{
		/* Error, stat should have set an error value. */ 
		return (DIR *) 0; 
	} 
 
	if (!S_ISDIR (statDir.st_mode))
	{
		/* Error, stat reports not a directory. */ 
		errno = ENOTDIR; 
		return (DIR *) 0; 
	} 
 
	/* Allocate enough space to store DIR structure and the complete
	   directory path given. */ 
	nd = (DIR *) calloc (1, sizeof (DIR) + strlen (szPath) + strlen (SLASH) + strlen (SUFFIX)); 
 
	if (!nd)
	{
		/* Error, out of memory. */ 
		errno = ENOMEM; 
		return (DIR *) 0; 
	} 
 
	/* Create the search expression. */ 
	strcpy (nd->dd_name, szPath); 
 
	/* Add on a slash if the path does not end with one. */ 

	if (nd->dd_name[0] != '\0' &&
	nd->dd_name[strlen (nd->dd_name) - 1] != '/' &&
	nd->dd_name[strlen (nd->dd_name) - 1] != '\\')
	{
		strcat (nd->dd_name, SLASH); 
	} 
 
	/* Add on the search pattern */ 
	strcat (nd->dd_name, SUFFIX); 
 
	/* Initialize handle to -1 so that a premature closedir doesn't try
	   to call _findclose on it. */

	nd->dd_handle = -1; 
 
	/* Initialize the status. */ 
	nd->dd_stat = 0; 
 
	/* Initialize the dirent structure. ino and reclen are invalid under
	   Win32, and name simply points at the appropriate part of the 
	   findfirst_t structure. */ 

	nd->dd_dir.d_ino = 0; 
	nd->dd_dir.d_reclen = 0; 
	nd->dd_dir.d_namlen = 0; 
	nd->dd_dir.d_name = nd->dd_dta.name; 
 
	return nd; 
} 
 
/* 
  readdir 
 
  Return a pointer to a dirent structure filled with the information on the
  next entry in the directory. 
*/ 
struct dirent *readdir (DIR * dirp)
{ 
	errno = 0; 
 
	/* Check for valid DIR struct. */ 
	if (!dirp)
	{
		errno = EFAULT; 
		return (struct dirent *) 0; 
	} 
 
	if (dirp->dd_dir.d_name != dirp->dd_dta.name)
	{
		/* The structure does not seem to be set up correctly. */
		errno = EINVAL; 
		return (struct dirent *) 0; 
	} 
 
	if (dirp->dd_stat < 0)
	{
		/* We have already returned all files in the directory (or the
		   structure has an invalid dd_stat). */ 
		return (struct dirent *) 0; 
	}
	else if (dirp->dd_stat == 0)
	{
		/* We haven't started the search yet. */ 
		/* Start the search */ 
		dirp->dd_handle = _findfirst (dirp->dd_name,&(dirp->dd_dta));
 
		if (dirp->dd_handle == -1)
		{
			/* Whoops! Seems there are no files in that directory. */
			dirp->dd_stat = -1; 
		}
		else
		{
			dirp->dd_stat = 1; 
		} 
	}
	else
	{
		/* Get the next search entry. */ 
		if (_findnext (dirp->dd_handle, &(dirp->dd_dta)))
		{
			/* We are off the end or otherwise error. */ 
			_findclose (dirp->dd_handle); 
			dirp->dd_handle = -1; 
			dirp->dd_stat = -1; 
		}
		else
		{
			/* Update the status to indicate the correct number. */
			dirp->dd_stat++; 
		} 
	} 
 
	if (dirp->dd_stat > 0)
	{
		/* Successfully got an entry. Everything about the file is already
		   appropriately filled in except the length of the file name. */

		dirp->dd_dir.d_namlen = strlen (dirp->dd_dir.d_name); 
		return &dirp->dd_dir; 
	} 
 
	return (struct dirent *) 0; 
} 
 
/* 
  closedir 
 
  Frees up resources allocated by opendir. 
*/ 
int closedir (DIR * dirp)
{ 
	int         rc; 
 
	errno = 0; 
	rc = 0; 
 
	if (!dirp)
	{
		errno = EFAULT; 
		return -1; 
	} 
 
	if (dirp->dd_handle != -1)
	{
		rc = _findclose (dirp->dd_handle); 
	} 
 
	/* Delete the dir structure. */ 
	free (dirp); 
 
	return rc; 
} 
 
/* 
  rewinddir 
 
  Return to the beginning of the directory "stream". We simply call 
findclose 
  and then reset things like an opendir. 
*/ 
void rewinddir (DIR * dirp)
{ 
	errno = 0; 
 
	if (!dirp)
	{
		errno = EFAULT; 
		return; 
	} 
 
	if (dirp->dd_handle != -1)
	{
		_findclose (dirp->dd_handle); 
	} 
 
	dirp->dd_handle = -1; 
	dirp->dd_stat = 0; 
} 
 
/* 
  telldir 
 
  Returns the "position" in the "directory stream" which can be used with
  seekdir to go back to an old entry. We simply return the value in stat.
*/ 
long telldir (DIR * dirp)
{ 
	errno = 0; 
 
	if (!dirp)
	{
		errno = EFAULT; 
		return -1; 
	} 
	return dirp->dd_stat; 
} 
 
/* 
  seekdir 
 
  Seek to an entry previously returned by telldir. We rewind the directory
  and call readdir repeatedly until either dd_stat is the position number
  or -1 (off the end). This is not perfect, in that the directory may 
  have changed while we weren't looking. But that is probably the case with
  any such system. 
*/ 
void seekdir (DIR * dirp, long lPos)
{ 
	errno = 0; 
 
	if (!dirp)
	{
		errno = EFAULT; 
		return; 
	} 
 
	if (lPos < -1)
	{
		/* Seeking to an invalid position. */ 
		errno = EINVAL; 
		return; 
	}
	else if (lPos == -1)
	{
		/* Seek past end. */ 
		if (dirp->dd_handle != -1)
		{
			_findclose (dirp->dd_handle); 
		} 
		dirp->dd_handle = -1; 
		dirp->dd_stat = -1; 
	}
	else
	{
		/* Rewind and read forward to the appropriate index. */ 
		rewinddir (dirp); 
 
		while ((dirp->dd_stat < lPos) && readdir (dirp)); 
	} 
} 
 
#else 
#include <dirent.h> 
#endif 
 
static int comp_d(const void *a1,const void *a2) 
{ 
	struct tag *a = (struct tag *)a1; 
	struct tag *b = (struct tag *)a2; 
	return(strcmp(a->str,b->str)); 
} 
 
int get_columns(int num,int max) 
{ 
	unsigned char sq0[] = 
	{ 
		3, 6, 12, 27, 20, 35, 56 
	}; 
	int Columns; 
 
//        if(max>sizeof sq0) 
//                max=sizeof sq0; 
	for(Columns=0;Columns<max && Columns<(int)(sizeof sq0);Columns++) 
		if( num <= sq0[Columns] ) 
			return(Columns+1); 
	return(max); 
} 
 
char *Select_From_Dir(char *name, int ( *check_name)(char *,char *),int select_flag) 
{ 
	return(Select_From_Dir(name,check_name,"",select_flag)); 
} 
 
char *Select_From_Dir(char *dir, int ( *check_name)(char *,char *),char *title,int select_flag) 
{ 
	DIR *fd; 
	struct dirent *dp; 
	struct tag *tags=NULL; 
	struct panel *panel=NULL;
	int num_fields=0; 
	int num_panel=0;
	int max_len; 
	char obr[256]; 
	int rez; 
	int atr=0; 
	char *select_name=NULL; 
	char name[256]; 
	char t[256];
 
	if(dir==NULL) 
		return(NULL);
	if(*dir!='/' && title!=NULL && !*title) 
	{ 
		getcwd(name,256); 
		if(strcmp(dir,".")) 
		{ 
			strcat(name,"/"); 
			strcat(name,dir); 
		} 
	} 
	else 
		strcpy(name,dir); 
	if(!*name) 
		strcpy(name,"."); 
	bzero(obr,sizeof obr); 
	if(title!=NULL && !*title) 
		strcpy(t,name);
	else strcpy(t,title);
BEGIN:
	char cwd[256];
	getcwd(cwd,sizeof cwd);
	term->Set_Color(8,017);
	term->clean();
	term->BOX((term->l_x()-2-strlen(cwd))/2,1,strlen(cwd)+2,3,' ',0,017,0,017);
	term->dpp((term->l_x()-strlen(cwd))/2,2);
	term->dps(cwd);

	tags=NULL; 
	num_fields=0; 
	max_len=0; 
	if((fd=opendir(name))==NULL) 
	{ 
		return(NULL); 
	} 
	while((dp=readdir(fd))!=NULL) 
	{ 
		if(atr==0 && *dp->d_name=='.') 
			continue;         /// hidden file
		int len=strlen(dp->d_name); 
		if(len>2 && dp->d_name[len-1]=='b' && dp->d_name[len-2]=='.') 
			continue;         // files *.b is hiden too
		if(!if_read(name,dp->d_name)) 
			continue;         // can't read
		if(*obr)
			if(strncmp(dp->d_name,obr,strlen(obr)))
				continue; // pattern is not valid
		if(check_name!=NULL)
		{
			struct stat st;
			char *fname=(char *)malloc(strlen(name)+strlen(dp->d_name)+16);
			full(name,dp->d_name,fname);
			if(!stat(fname,&st) && !S_ISDIR(st.st_mode))
			{
				if(!(*check_name)(name,dp->d_name))
				{
					free(fname);
					continue; // type is not valid
				}
			}
			free(fname);
		}
 
		tags=(struct tag *)realloc(tags,(num_fields+1)*sizeof (struct tag)); 
		bzero(&tags[num_fields],sizeof (struct tag)); 
		tags[num_fields].str=(char *)calloc(len+1,1); 
		tags[num_fields].des.sla->n=1; 
		tags[num_fields].des.l=len; 
		if(len>max_len) 
			max_len=len; 
		strcpy(tags[num_fields].str,dp->d_name); 
 
		struct stat st; 
		char *fname=(char *)malloc(strlen(name)+strlen(dp->d_name)+16); 
		full(name,dp->d_name,fname); 
		if(stat(fname,&st)) 
		{ 
			tags[num_fields].color.fg=07; 
			tags[num_fields].color.bg=0; 
		} 
		else 
		{ 
			if(S_ISDIR(st.st_mode)) 
			{ 
				tags[num_fields].color.fg=016; 
				if(if_base(name,dp->d_name)) 
				{ 
					tags[num_fields].color.fg=013; 
					tags[num_fields].color.bg=011; 
				} 
				else 
					tags[num_fields].color.bg=01; 
			} 
			else 
			{ 
				tags[num_fields].color.bg=06; 
				tags[num_fields].color.fg=012; 
				switch(st.st_mode&0170000) 
				{ 
				case S_IFCHR: 
					tags[num_fields].color.fg=07; 
					break; 
#ifdef S_IFBLK 
				case S_IFBLK: 
					tags[num_fields].color.fg=04; 
					break; 
#endif 
#ifdef S_FIFO 
				case S_IFIFO: 
					tags[num_fields].color.fg=011; 
					break; 
#endif 
#ifdef S_IFNAM 
				case S_IFNAM: 
					tags[num_fields].color.fg=05; 
					break; 
#endif 
#ifdef S_IFLNK 
				case S_IFLNK: 
					tags[num_fields].color.fg=07; 
					break; 
#endif 
#ifdef S_IFSOCK 
				case S_IFSOCK: 
					tags[num_fields].color.fg=06; 
					break; 
#endif 
				default: 
					tags[num_fields].color.fg=012; 
					if(st.st_mode&0100) 
						tags[num_fields].color.fg=016; 
					if((len=strlen(dp->d_name))>4) 
					{ 
						char *ch1=(char *)strrchr(dp->d_name,'.'); 
						if(ch1!=NULL) 
						{ 
							if(!strcmp(ch1,".htm") || !strcmp(ch1,".html")) 
								tags[num_fields].color.fg=017; 
							if(!strcmp(ch1,".gif") || !strcmp(ch1,".jpg")) 
								tags[num_fields].color.fg=014; 
						} 
					} 
 
				} 
			} 
		} 
		free(fname); 
		num_fields++; 
	} 
	closedir(fd); 
	if(!num_fields) 
	{ 
		return(NULL); 
	} 
	max_len++; 
	if(max_len>term->l_x()-10) 
		max_len=term->l_x()-10; 
	if(num_fields==0 && *obr) 
	{ 
		if(*obr) 
			obr[strlen(obr)-1]=0; 
		if(tags!=NULL) 
		{ 
			while(num_fields) 
			{ 
				if(tags[--num_fields].str!=NULL) 
				{ 
					free(tags[num_fields].str); 
					tags[num_fields].str=NULL; 
				} 
			} 
			free(tags); 
			tags=NULL; 
		} 
		goto BEGIN; 
	} 
	if(num_fields==1 && select_flag==0) 
	{ 
		select_name=(char *)realloc(select_name,strlen(name)+strlen(tags->str)+10); 
		char *ch=name; 
		if(ch[0]=='.' && ch[1]=='/') 
			ch+=2; 
		if(name[strlen(name)-1]!='/') 
			sprintf(select_name,"%s/%s",ch,tags->str); 
		else 
			sprintf(select_name,"%s%s",ch,tags->str); 
		free(tags->str); 
		free(tags); 
		return(select_name); 
	} 
	qsort(tags,num_fields,sizeof (struct tag),comp_d); 
 
	int columns=get_columns(num_fields,(term->l_x()-4)/(max_len+1)); 
	int lines = (num_fields+columns-1)/columns; 
	for(int i=0;i<num_fields;i++) 
	{ 
		tags[i].des.x=(i/lines)*(max_len); 
		tags[i].des.y=i%lines; 
		tags[i].des.atr=0; 
	} 
	if(panel!=NULL)
	{ 
		free(panel);
		panel=NULL;
	} 
	panel=(struct panel *)calloc(num_panel=1,sizeof (struct panel));
	panel->x=-1;
	panel->bg=06;
	panel->fg=017;
 
	int l=(max_len)*columns+2; 
	if(l<(int)strlen(name)+2) 
		l=strlen(name)+2; 
	int h=lines+2; 
 
	if(h>term->l_y()-3) 
		h=term->l_y()-3; 
	if(l>term->l_x()-2) 
		l=term->l_x()-2; 
	if(l<12) 
		l=12; 

	int x=(term->l_x()-l)/2;
	int y=(term->l_y()-lines-5)/2;
	if(x<0)
		x=0;
	if(y<0)
		y=1;

	BROWSER *browser = new BROWSER(NULL,x,y,l,h,t); 
	browser->create_Manual(num_fields,tags,num_panel,panel);
	hot_line(message(70));
	if((rez=browser->Move())=='\r') 
	{ 
		int i=browser->Act_Field(); 
 
		select_name=(char *)realloc(select_name,strlen(name)+strlen(tags[i].str)+10); 

		char *ch=name; 
		if(ch[0]=='.' && ch[1]=='/') 
			ch+=2; 
 
		if(!strcmp(ch,".")) 
			strcpy(select_name,tags[i].str); 
		else if(ch[strlen(name)-1]!='/') 
			sprintf(select_name,"%s/%s",ch,tags[i].str); 
		else 
			sprintf(select_name,"%s%s",ch,tags[i].str); 

		if(if_dir("",select_name) && !if_base("",select_name))
		{ 
			delete browser; 
			chdir(select_name);

			char *dir=(char *)malloc(strlen(select_name)+1); 
			strcpy(dir,".");

			char *name=Select_From_Dir(dir,check_name,t);
			free(dir);
			if(name==NULL || !*name) 
			{ 
				free(select_name);
				return(name);
			} 
			select_name=(char *)realloc(select_name,strlen(name)+10); 
			char *ch=name; 
			if(ch[0]=='.' && ch[1]=='/') 
				ch+=2; 
			strcpy(select_name,ch); 
			free(name); 
			return(select_name); 
		} 
	} 
	else 
	{ 
		select_name=(char *)realloc(select_name,1);
		*select_name=0;
	} 
	delete browser; 
 
	switch(rez) 
	{ 
	case '/': 
		if(!*title) 
			strcpy(name,"/"); 
		goto BEGIN; 
	case '.': 
		if(!*title) 
		{ 
			char *ch=getenv("HOME"); 
			if(ch!=NULL) 
				strcpy(name,ch); 
		} 
		goto BEGIN; 
	case '\n': 
		{ 
			chdir("..");
			strcpy(name,".");
			goto BEGIN; 
		} 
	case '+': 
		atr=!atr; 
		goto BEGIN; 
	case DEL: 
		if(*obr) 
			obr[strlen(obr)-1]=0; 
		goto BEGIN; 
	case F12:
		if(check_name==if_base)
		{
			select_name=(char *)realloc(select_name,128);
			*select_name=0;
			if(getuid()==0&&dial(message(71),5,NULL,-1,select_name)=='\r')
			{
				Make_Class(select_name,NULL,1);
				return(select_name);
			}
		}
		goto BEGIN;
	default: 
		if(rez>='A' && rez<'z') 
		{ 
			obr[strlen(obr)]=rez; 
			goto BEGIN; 
		} 
	} 
	if(panel!=NULL)
		free(panel);
 
	return(select_name); 
} 
 
char *get_user_dir(char *base_name,char *dir_name) 
{ 
	char *user; 
	struct stat st; 
	int vers; 
 
	vers=if_base("",base_name); 
	char *name=(char *)malloc(strlen(base_name)+strlen(user=GetLogin())+strlen(dir_name)+3); 
	if(vers==3) 
		sprintf(name,"%s/%s/%s",base_name,dir_name,user); 
	else 
		full(base_name,user,name); 
	if(!stat(name,&st)) 
	{ 
		if(vers!=3 && S_ISDIR(st.st_mode)) 
		{ 
			strcat(name,"/"); 
			strcat(name,dir_name); 
		} 
	} 
	else 
		full(base_name,dir_name,name); 
	if(!stat(name,&st)) 
	{ 
		if(S_ISDIR(st.st_mode)) 
		{ 
			return(name); 
		} 
	} 
	free(name); 
	return(NULL); 
} 
 
static int cmp(const void *a1,const void *a2) 
{ 
	char **a=(char **)a1; 
	char **b=(char **)a2; 
	return(strcmp(*a,*b)); 
} 
 
int choise_box(struct item *names,int x0,int y0,int size) 
{ 
	int i,act=0; 
	int num,len,f; 
	int shift=0,nn; 
	char *mes=message(19); 
 
	for(num=0,len=0;*names[num].name;num++) 
	{ 
		if((int)strlen(names[num].name)>len) 
			len=strlen(names[num].name); 
	} 
	if(len<(int)strlen(mes)+2)
		len=strlen(mes)+2;
	if(num>size) 
		nn=size; 
	else    nn=num; 
	f=term->get_box(x0,y0,len+4,nn+4); 
	term->MultiColor(x0,y0,len+2,nn+2); 
	for(;;) 
	{ 
		char str[32]; 
		int y=0; 
 
		sprintf(str,"%d",act+1+shift); 
		term->BOX(x0,y0,len+2,nn+2,' ',0,017,0,0x3); 
		term->dpp(x0+1+(len-strlen(mes))/2,y0); 
		term->Set_Color(0,017); 
		term->dps(mes); 
		term->dpp(x0+len-strlen(str),y0+nn+1); 
		term->Set_Color(0,016); 
		term->dps(str); 
		for(i=0;i<num && i<size;i++) 
		{ 
			term->dpp(x0+1,y0+1+i); 
			if(i==act) 
			{ 
				term->Set_Color(0x7,0xf); 
				y=y0+1+i; 
			} 
			else 
				term->Set_Color(names[i].bg,names[shift+i].fg); 
			term->dps(names[shift+i].name); 
			term->dpn(len-strlen(names[shift+i].name),' '); 
		} 
		term->dpp(x0+1,y); 
		switch(Xmouse(term->dpi())) 
		{ 
			case 0: 
				if(term->ev().x>=x0+1 && term->ev().x<=x0+len) 
				{ 
					if(act==term->ev().y-y0-1) 
						goto EXIT; 
					if(term->ev().y>=y0+1 && term->ev().y<=y0+1+num) 
						act=term->ev().y-y0-1; 
				} 
				break; 
			case CU: 
				if(act) 
					act--; 
				else if(shift) 
					shift--; 
				break; 
			case CD: 
				if(act<i-1) 
					act++; 
				else if(act+shift<num-1) 
					shift++; 
				break; 
			case '\r': 
				goto EXIT; 
			default: 
				act=-1; 
				shift=0; 
				goto EXIT; 
		} 
	} 
EXIT: 
	term->restore_box(f); 
	term->free_box(f); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
	return(act+shift); 
} 
 
 
int choise_box(char **names,int x0,int y0,int size) 
{ 
	struct item *name; 
	int i,num; 
 
	for(num=0;*names[num];num++); 
	if(num<1) 
		return(0); 
	num++; 
	name=(struct item *)malloc(num*sizeof (struct item)); 
	for(i=0;i<num;i++) 
	{ 
		name[i].name=names[i]; 
		name[i].fg=016; 
		name[i].bg=010; 
	} 
	i=choise_box(name,x0,y0,size); 
	free(name); 
	return(i); 
} 
 
int get_filename(char *dir,char **name,int arg) 
{ 
	DIR *fd; 
	struct dirent *dp; 
	char **names=NULL; 
	int num=0,max_len=0; 
 
	if((fd=opendir(dir))==NULL) 
		return(F10); 
	while((dp=readdir(fd))!=NULL) 
	{ 
		int len; 
 
		if(*dp->d_name=='.') 
			continue; 
		len=strlen(dp->d_name); 
		if(len>2 && dp->d_name[len-1]=='b' && dp->d_name[len-2]=='.') 
			continue; 
/* 
		struct stat st; 
		stat(dp->d_name,&st); 
		if(!(st.st_mode&S_IFDIR)) 
			continue; 
*/ 
		names=(char **)realloc(names,(++num)*sizeof (char *)); 
		names[num-1]=(char *)malloc(len+1); 
		strcpy(names[num-1],dp->d_name); 
		if(len>max_len) 
			max_len=len; 
	}; 
	closedir(fd); 
	qsort(names,num,sizeof names,cmp); 
	if(num) 
	{ 
		names=(char **)realloc(names,(num+1)*sizeof (char **)); 
		names[num]=""; 
	} 
	term->dpp(0,term->l_y()); 
	term->Set_Color(06,016); 
	hot_line("Down/Up Select Enter Make Choice F12 New File F10 quit "); 
	if(num==0 || (num=choise_box(names,2,2,num>term->l_y()-5?term->l_y()-5:num))==-1) 
	{ 
		char str[256]; 
		char *mess=message(44); 
		int len=strlen(mess)+32; 
		int x=(term->l_x()-len)/2; 
		int y=term->l_y()/2-5; 
		int f=term->get_box(x,y,len+2,3); 
 
		*str=0; 
		term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
		term->MultiColor(x,y,len+2,3); 
		term->BOX(x,y,len+2,3,' ',0,017,0,017); 
		term->dpp(x+1,y+1); 
		term->Set_Color(06,016); 
		term->dps(mess); 
		term->cursor_visible(); 
		x=EditLine->edit(0,str,64,32,x+strlen(mess)+1,y+1,0); 
		term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
		term->cursor_invisible(); 
		term->restore_box(f); 
		term->free_box(f); 
		if(x==F10) 
			return(F10); 
		*name=(char *)malloc(strlen(str)+1); 
		strcpy(*name,str); 
	} 
	else 
	{ 
		*name=(char *)malloc(strlen(names[num])+1); 
		strcpy(*name,names[num]); 
		if(arg==1) 
		{ 
			if(!dial(message(39),1)) 
			{ 
				char *file=(char *)malloc(strlen(dir)+strlen(*name)+2); 
				sprintf(file,"%s/%s",dir,*name); 
				unlink(file); 
				free(file); 
			} 
		} 
	} 
	return(0); 
} 
