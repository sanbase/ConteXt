/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:main.cpp
*/

#include "stdafx.h"  
#include "CX_Browser.h" 
#ifndef WIN32 
#include <sys/wait.h> 
#endif 
#include "DB_CLASS/ram_base.h" 
#ifdef LINUX 
#include <shadow.h> 
#ifndef __USE_XOPEN 
#define __USE_XOPEN 
#endif 
#include <crypt.h> 
#include <unistd.h> 
#endif 
#include <time.h> 
#define DEFAULT_REFRESH_DELAY 30
 
const static char *ver="ConteXt 6.0 release 1 [Mon Oct  5 08:53:41 2015]\nAlexander Lashenko. Toronto, Canada\nmailto:lashenko@unixspace.com";
const char *DESTDIR="/usr/local"; 
static CX_BROWSER *cx=NULL; 
extern CX_BROWSER *current_browser; 
extern CX_BROWSER *current_cx;
extern struct s_tdescr t; 
static struct last_status get_last_status(char *); 
int refresh_delay=DEFAULT_REFRESH_DELAY; 
int demo=0; 

#ifndef WIN32 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netinet/in_systm.h> 
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h> 
#include <netdb.h> 
#include <sys/socket.h> 
#include <sys/param.h> 
#include <pwd.h> 
#endif 
#include <errno.h> 
 
Terminal *term; 
Line *EditLine; 

#ifdef WIN32
SOCKET sock_fd;
#endif
 
#ifdef  REPORT_USAGE 
#include <pthread.h> 
 
static void *report(void *a) 
{ 
	char hostname[NAMESIZE]; 
	char str[LINESIZE]; 
	pthread_detach(pthread_self()); 
	gethostname(hostname,sizeof hostname); 
	sprintf(str,"GET /tmp/cx6.count?%s",hostname);
	HTTP_request("mhp.com.ua",str); 
	return(NULL); 
} 
#endif 
 
int refresh_flag=0; 
 
void refresh(int i) 
{ 
	if(current_cx!=NULL) 
	{ 
#ifndef WIN32 
		if(!(current_cx->cx_cond&(CHRT)) && (refresh_flag || i==SIGUSR2)) 
		{ 
			if(i==SIGUSR2) 
			{ 
				int f=term->get_box(0,0,term->l_x(),term->l_y()); 
				current_cx->delete_menu(); 
				term->clean(); 
				term->flush(); 
				term->restore_box(f); 
				term->free_box(f); 
				term->flush(); 
				term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
				term->cursor_invisible(); 
				current_cx->load_menu(); 
			} 
			else 
				current_cx->Refresh(i); 
			term->flush(); 
		} 
#else 
		current_cx->Refresh(i);
		term->flush();
#endif 
	} 
} 
 
static void exit_sig(int i) 
{ 
	if(cx!=NULL) 
	{ 
		if(cx->form_cond&EDIT) 
			cx->db->Roll_Back(); 
		delete cx; 
	} 
#ifndef WIN32 
	if(i==SIGHUP) 
		kill(getppid(),SIGHUP); 
	if(i!=SIGINT) 
	{ 
		struct color color; 
		color.bg=04; 
		color.fg=016; 
		if(i==SIGSEGV || i==SIGBUS)
			dial("Fatal error in CX-browser.",4,&color); 
		else 
		{ 
			char str[256]; 
			sprintf(str,"Got signal %d. The program will be terminated",i); 
			dial(str,4,&color); 
		} 
	} 
#endif 
	delete term; 
	exit(0); 
} 
 
#ifndef WIN32 
void suspend_init() 
{ 
	for(int i=1;i<SIGUSR1;i++) 
	{ 
//#ifdef _BSDI_VERSION 
		if(i==SIGSEGV || i==SIGABRT)
			continue; 
//#endif 
		if(i==SIGALRM || i==SIGCHLD) 
			continue; 
		signal(i,exit_sig); 
	} 
	return; 
}
#endif 

int get_arg(char *line,char *pattern,char *&arg) 
{ 
	char *ch=strstr(line,pattern); 
	int flag=0; 
	if(ch==NULL) 
	{ 
		if(arg!=NULL) 
			free(arg); 
		arg=NULL; 
		return(0); 
	} 
	ch+=strlen(pattern); 
	while(*ch==' ' || *ch=='\t') 
		ch++; 
	if(*ch=='"') 
	{ 
		ch++; 
		flag=1; 
	} 
	for(int i=0;ch[i]!=0;i++) 
	{ 
		if(flag==0 && (ch[i]==' ' || ch[i]==',' || ch[i]=='\t')) 
			break; 
		if(flag && ch[i]=='"') 
			break; 
		arg=(char *)realloc(arg,i+2); 
		arg[i]=ch[i]; 
		arg[i+1]=0; 
	} 
	return(1); 
} 
 
void serialization() 
{ 
	int fd; 
	struct stat st; 
	char buf[96]; 
	int key,x; 
	int hi, lo;
	unsigned long k;
	char *str1="This is an unregistered copy only for evaluation purposes."; 
	char *str2="For registration visit web site: www.UnixSpace.com/registration.html";
	char *str3="If you have the activation key, print it here:"; 

#ifndef WIN32
	if(stat("/usr/local/bin/cx6",&st)<0)
	{
		char *str="The file cx6 should be in the /usr/local/bin directory.";
		term->BOX((term->l_x()-strlen(str))/2-1,term->l_y()/3,strlen(str)+2,3,' ',4,0xe,4,0xe);
		term->dpp((term->l_x()-strlen(str))/2,term->l_y()/3+1); 
		term->dps(str); 
		term->dpi(); 
		delete term; 
		exit(0); 
	}
	k=st.st_mtime;
	if((fd=open("/usr/local/etc/CX.key",O_RDONLY))<0)
		goto ASK;
#else
	return;
	LONG Result;
	char tmp[256];
	memset(tmp,0,sizeof tmp);
	LONG len;
	DWORD type;
	HKEY Key;
	char *path="C:\\Program Files\\UnixSpace\\";
	Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\UnixSpace\\ConteXt\\" ,0, KEY_READ, &Key);
	if (Result == ERROR_SUCCESS)
	{
		len=sizeof tmp -1;
		Result = RegQueryValueEx(Key,"InstallPath",NULL,&type,(BYTE *)tmp, (LPDWORD)&len);
		if (Result == ERROR_SUCCESS)
		{
			path=tmp;
		}
		RegCloseKey(Key);
	}
	char str[256];
	sprintf(str,"%sbin\\cx6.exe",path);
	if(stat(str,&st)<0)
	{
		sprintf(str,"The file cx5.exe should be in the %s directory.",path);
		term->BOX((term->l_x()-strlen(str))/2-1,term->l_y()/3,strlen(str)+2,3,' ',4,0xe,4,0xe);
		term->dpp((term->l_x()-strlen(str))/2,term->l_y()/3+1);
		term->dps(str);
		term->dpi();
		delete term;
		exit(0);
	}
	k=st.st_mtime;
	sprintf(str,"%setc\\CX.key",path);
	if((fd=open(str,O_RDONLY))<0)
		goto ASK;

#endif
	bzero(buf,sizeof buf); 
	read(fd,buf,sizeof buf); 
	close(fd); 
	key=atoi(buf);

	k=st.st_mtime;
	x=k%673452;

	hi = x / 127773; 
	lo = x % 127773; 
	x = 16807 * lo - 2836 * hi; 
	if (x <= 0) 
		x += 0x7fffffff; 
	x%=100000; 
	if(key==x) 
		return; 
ASK: 
	int f=term->get_box(0,0,term->l_x(),term->l_y()); 
	//term->BOX((term->l_x()-strlen(str2))/2-1,term->l_y()/4,strlen(str2)+2,5,' ',1,0xf,1,0xf);
	//term->dpp((term->l_x()-strlen(str1))/2,term->l_y()/4+1); 
	//term->dps(str1); 
	//term->dpp((term->l_x()-strlen(str2))/2,term->l_y()/4+2); 
	//term->dps(str2); 
	//sprintf(buf,"The registration number of your copy is %ld",k%673452);
	//term->dpp((term->l_x()-strlen(buf))/2,term->l_y()/4+3); 
	//term->dps(buf);
	char *name=NULL;
#ifndef WIN32
//	if(getuid()==0 && (fd=open(name="/usr/local/etc/CX.key",O_RDWR|O_CREAT,0644))>0)
//	{ 
//		term->BOX((int)(term->l_x()-strlen(str3)-6)/2-1,term->l_y()/4+7,(int)strlen(str3)+8,3,' ',1,0xf,1,0xf);
//		term->dpp((term->l_x()-strlen(str3)-6)/2,term->l_y()/4+8); term->dps(str3); 
//		*buf=0;
//		term->Set_Color(120,0);
//		//EditLine->edit(0,buf,6,6,(term->l_x()-strlen(str3)-6)/2+strlen(str3),term->l_y()/4+8,10); 
//		if(atoi(buf)>0) 
//		{
//			close(fd);
//			if((fd=creat(name,0644))>0)
//				write(fd,buf,strlen(buf)); 
//		} 
//		close(fd); 
//	} 
//	else
#endif
//		term->dpi(); 
	term->restore_box(f); 
	term->free_box(f); 
	demo=1; 
} 
#ifdef WIN32
int main_loop(int argc,char **argv)
#else
main(int argc,char **argv) 
#endif
{ 
	int i,s=0; 
	int start=1; 
	char *folder=NULL; 
	char *name_blank=NULL; 

	char *name_sel=NULL; 
	char *cmd_line=NULL; 
	int record; 
	int remote=0; 
	struct sla sla[SLA_DEEP]; 
	int sla_len=0; 
	struct last_status last; 

	if(argc==2 && (!strcmp(argv[1],"-V") || !strcmp(argv[1],"-v"))) 
	{ 
		printf("%s\n",ver); 
		exit(0); 
	}
#ifdef WIN32
	_fmode=_O_BINARY;
#endif
#ifdef  REPORT_USAGE 
	pthread_t a; 
	pthread_create(&a,NULL,report,NULL); 
#endif 
 

	cmd_line=(char *)calloc(1,1); 
	bzero(sla,sizeof sla); 
	for(i=1;i<argc;i++) 
	{ 
		cmd_line=(char *)realloc(cmd_line,strlen(cmd_line)+strlen(argv[i])+2); 
		strcat(cmd_line," "); 
		strcat(cmd_line,argv[i]); 
	} 

#ifndef WIN32 
	if(strstr(cmd_line,"->CX5")!=NULL) 
	{ 
#ifndef SPARC 
		remote=1; 
		if(get_arg(cmd_line,"-A",folder)) 
		{ 
			char *ch=strchr(folder,':'); 
			if(ch==NULL) 
				exit(1); 
			*ch=0; 
			sla_len=atoi(ch+1); 
			str_to_sla(folder,sla); 
		} 
		else if(get_arg(cmd_line,"-U",folder) && folder!=NULL) 
		{ 
			struct passwd *pwd; 
			if((pwd = getpwnam(folder))!=NULL) 
				setenv("USER",folder,1); 
			char passwd_buf[256];
			bzero(passwd_buf,sizeof passwd_buf);
			for(i=0;i<sizeof passwd_buf;i++)
			{
				read(0,passwd_buf+i,1);
				if(passwd_buf[i]=='\n' || passwd_buf[i]=='\r' || passwd_buf[i]==0)
				{
					passwd_buf[i]=0;
					break;
				}
			}
#ifdef LINUX
			struct spwd *shadow=getspnam(folder);
			if(strcmp(crypt((const char *)passwd_buf, (const char *)shadow->sp_pwdp), shadow->sp_pwdp))
#else
			if(strcmp(crypt((const char *)passwd_buf, (const char *)pwd->pw_passwd), pwd->pw_passwd))
#endif
			{ 
				printf("%s",message(36)); fflush(stdout); 
				exit(1); 
			} 
			if(setuid(pwd->pw_uid)) 
			{ 
				printf("%s",message(36)); fflush(stdout); 
				exit(1); 
			} 
		} 
		else 
		{ 
			printf("%s",message(36)); fflush(stdout); 
			exit(2); 
		} 
		setenv("TERM","applet",1); 
#else 
		exit(0); 
#endif 
	} 
	else 
	{ 
		setuid(getuid()); 
	} 
#endif 
	if(term==NULL)
		term = new Terminal();
	if(EditLine==NULL)
		EditLine = new Line(term);
 
	if(term->Color()) 
		term->l_x(term->l_x()-3); 
 
#ifndef WIN32 
	suspend_init();
#endif 
	term->dpp(0,0); 
	term->Set_Color(8,017); 
	term->clean(); 
	serialization(); 
	term->New_Color(63,0xf8,0xa0,0x30); 
START: 
	record=0; 
	for(i=0;i<(int)strlen(cmd_line);i++) 
		if(cmd_line[i]<' ' && cmd_line[i]>0) 
		{ 
			cmd_line[i]=0; 
			break; 
		} 
#ifdef WIN32
	chdir("C:\\Program Files\\UnixSpace\\DataBase");
#endif
	if (argc<2 && start )
	{

			int fd=open("start_up",O_RDONLY);
			if(fd>0)
			{
				struct stat st;
 
				fstat(fd,&st);
				cmd_line=(char *)realloc(cmd_line,st.st_size+1);
				read(fd,cmd_line,st.st_size);
				close(fd);
				cmd_line[st.st_size]=0;
				if(!get_arg(cmd_line,"-N",folder))
				{
					char *tmp=(char *)malloc(strlen(cmd_line)+3);
					sprintf(tmp,"-N%s",cmd_line);
					free(cmd_line);
					cmd_line=tmp;
				}
				remote=0;
				start=0;
				goto START;
			}
	}

	if(get_arg(cmd_line,"-D",folder)) 
	{ 
		if(folder!=NULL) 
			chdir(folder); 
	} 

	if(get_arg(cmd_line,"-L",folder))
		setenv("CXUSER",folder,1);

	if(get_arg(cmd_line,"-N",folder)) 
		s=1; 
	else if(remote) 
	{ 
		delete term;
		exit(1); 
	} 
	if(sla->n==0 && get_arg(cmd_line,"-A",name_sel)) 
		str_to_sla(name_sel,sla); 
	if(get_arg(cmd_line,"-R",name_sel)) 
		record=atoi(name_sel); 
	get_arg(cmd_line,"-S",name_sel); 
	if(!get_arg(cmd_line,"-F",name_blank)) 
		get_arg(cmd_line,"-B",name_blank);      // old version 
	term->cursor_invisible(); 
	for(;;) 
	{ 
		start=0; 
		if(folder==NULL) 
		{ 
SELECT_FOLDER:
			term->Set_Color(8,017);
			term->clean();
			folder=Select_From_Dir(".",if_base,message(19),1); 
			if(folder==NULL || !*folder)
			{ 

				if((folder==NULL && dial(message(69),1))
				|| (folder!=NULL && dial(message(76),0)))
				{
					char name[64];
					*name=0;
//                                        if(getuid()==0&&dial(message(71),5,NULL,-1,name)=='\r')
					if(dial(message(71),5,NULL,-1,name)=='\r')
					{
						Make_Class(name,NULL,1);
						goto SELECT_FOLDER;
					}
				}
				delete term;
				exit(0);
			} 
		} 
		term->Set_Color(8,017); 
		term->clean(); 
		last=get_last_status(folder); 
		if(s) 
		{ 
			if(name_blank!=NULL) 
			{ 
				char *form_base=get_user_dir(folder,FORMDB); 
				if(if_base("",form_base)) 
				{ 
					CX_BASE form(form_base); 
					CX_FIND f(&form); 
					last.form.form=f.Find_First(1,name_blank,0); 
					if(last.form.form<=0) 
					{ 
						last.form.form=0; 
						strcpy(last.form.blank,name_blank); 
					} 
				} 
				else 
					strcpy(last.form.blank,name_blank); 
				free(form_base); 
				free(name_blank); 
				form_base=NULL;
				name_blank=NULL; 
			} 
			if(record>0) 
				last.record=record; 
		} 
		char *ch=NULL;
		get_arg(cmd_line,"-I",ch);
		
		try {
		    if(sla_len && sla->n) {
			cx=new CX_BROWSER(folder,last.record,sla,sla_len,ch);
		    }
		    else { 
			cx=new CX_BROWSER(folder,last.record,&last.form,ch);
		    }
		}
		catch (...) {
			//exit error
			if(cx->cxerror() != 77){
			    char *str=(char *)malloc(strlen(folder)+strlen(message(13))+2); 
			    sprintf(str,message(13),folder); 
			    dial(str,4); 
			}
			delete term;
			exit(0);
		}
		//Exit 
		if(cx->cxerror() == 77){
			delete term;
			exit(0);
		}
		if(ch!=NULL)
			free(ch);
		ch=NULL;
		cx->Restore_Status(&last); 
		if(s && sla->n) 
		{ 
			struct tag_descriptor td;
			bcopy(sla,td.sla,sizeof td.sla); 
			cx->Go_To_Field(td,last.record); 
		} 
		if(s && name_sel!=NULL) 
		{ 
			char *name=(char *)malloc(strlen(folder)+strlen(name_sel)+32); 
			sprintf(name,"%s/%s/%s",folder,INDEXDIR,name_sel); 
			cx->set_prim_selection(name); 
			cx->Read_Index(name); 
			long page=cx->find_page(last.record); 
			if(page<0) 
				page=cx->db->Max_Index(); 
			cx->Go_To_Index(page); 
			free(name); 
			free(name_sel); 
			name=NULL;
			name_sel=NULL; 
		} 
		char *name=(char *)malloc(strlen(folder)+10); 
		sprintf(name,"%s/.prot",folder); 
		if(!access(name,F_OK)) 
			cx->cx_cond|=PROT; 
		free(name); 
		name=NULL;
		current_browser=cx; 
 
		cx->frame_color=15; 
		cx->protocol("Start"); 
		current_cx=cx; 
#ifndef WIN32 
		if(cx->V3Show()) 
			signal(SIGUSR2,SIG_IGN); 
		else 
			signal(SIGUSR2,refresh); 
#endif 
		if(sla_len && sla->n) 
			cx->Read_Only(1); 
		i=0; 
//                cx->Refresh(0); 
		try 
		{ 
			i=cx->Action(); 
			cx->delete_menu(); 
			if(i==c_ChangeDB && *cx->db->share->output) 
			{ 
				cmd_line=(char *)realloc(cmd_line,strlen(cx->db->share->output)+1); 
				strcpy(cmd_line,cx->db->share->output); 
			} 
			else    *cmd_line=0; 
			cx->protocol("Finish\n"); 
			bcopy(&cx->name_form,&last.form,sizeof last.form); 
			last.record=cx->Record(cx->Index()); 
			last.field=cx->Act_Field(); 
			cx->Write(); 
			cx->Save_Index(); 
			put_last_status(cx->db->Name_Base(),&last); 
 
			if(folder!=NULL) 
				free(folder); 
			folder=NULL; 
			record=last.record; 
			delete(cx); 
			cx=NULL;
			current_cx=NULL;
		} 
		catch(int a) 
		{ 
			int i=open(".",O_RDONLY); 
			cx->delete_menu(); 
			delete(cx); 
			delete term;
			if(i>0) 
				printf("Got exception #%d\n",a); 
			else    printf("Too many open files\n"); 
			exit(1); 
		} 
		if(i==c_ChangeDB) 
		{ 
			if(*cmd_line) 
				goto START; 
			continue; 
		} 
		if(s) 
			break; 
	} 
	term->clean(); 
	delete term;
	if(sla_len && sla->n) 
	{ 
#ifndef WIN32 
		sleep(1); 
#endif 
		printf("Return=%d Record=%d\n",i,record); 
		fflush(stdout); 
	} 
	return 0;
} 
 
void put_last_status(char *folder,struct last_status *last) 
{ 
	int fd; 
	char *ch,str[64]; 
 
	if((ch=getenv("SSH_CLIENT"))!=NULL || (ch=getenv("TELNET_CLIENT"))!=NULL) 
	{ 
		strcpy(str,ch); 
		char  *ch=strchr(str,' '); 
		if(ch!=NULL) 
			*ch=0; 
	} 
	else    *str=0; 
 
	char *name=(char *)malloc(strlen(folder)+strlen(TMPDIR)+strlen(GetLogin())+strlen(str)+19); 
	sprintf(name,"%s/%s/.vl.%s[%s]",folder,TMPDIR,GetLogin(),str); 
	if((fd=creat(name,0600))>0) 
		write(fd,last,sizeof (struct last_status)); 
	free(name); 
} 
 
static struct last_status get_last_status(char *folder) 
{ 
	struct last_status last; 
	int fd; 
	char *ch,str[64]; 
 
	if((ch=getenv("SSH_CLIENT"))!=NULL || (ch=getenv("TELNET_CLIENT"))!=NULL) 
	{ 
		strcpy(str,ch); 
		char  *ch=strchr(str,' '); 
		if(ch!=NULL) 
			*ch=0; 
	} 
	else    *str=0; 
	bzero(&last, sizeof last); 
 
	char *name=(char *)malloc(strlen(folder)+strlen(TMPDIR)+strlen(GetLogin())+strlen(str)+16); 
	sprintf(name,"%s/%s/.vl.%s[%s]",folder,TMPDIR,GetLogin(),str); 
	if((fd=open(name,O_RDWR))>0) 
		read(fd,&last,sizeof last); 
	else 
	{ 
		sprintf(name,"%s/%s/.vl.%s",folder,TMPDIR,GetLogin()); 
		if((fd=open(name,O_RDONLY))>0) 
			read(fd,&last,sizeof last); 
	} 
	free(name); 
	return(last); 
} 
 
int Select_Form(char *folder,struct x_form *form) 
{ 
	struct sla *sla; 
	int page=1; 
	char *title="Select Form"; 
	char *form_base=NULL; 
	bzero(form,sizeof form); 
	form_base=get_user_dir(folder,FORMDB); 
	if(!if_base("",form_base)) 
	{ 
BLANK: 
		if(form_base!=NULL) 
			free(form_base); 
		form_base=get_user_dir(folder,BLANKDIR); 
		char *name=Select_From_Dir(form_base,if_read,title); 
		if(name==NULL || !*name)
		{ 
			if(name!=NULL)
				free(name);
			free(form_base); 
			return(-1); 
		} 
		if(!strncmp(name,form_base,strlen(form_base))) 
			strncpy(form->blank,name+strlen(form_base)+1,(sizeof form->blank)-1); 
		else 
			strncpy(form->blank,name,(sizeof form->blank)-1); 
		free(name); 
		free(form_base); 
		return(0); 
	} 
	sla=(struct sla *)calloc(SLA_DEEP,sizeof (struct sla)); 
	sla->n=1; 
 
	CX_BROWSER *bform; 
	try 
	{ 
		struct x_form f; 
		bzero(&f,sizeof f); 
		bform = new CX_BROWSER(form_base,1,&f); 
	} 
	catch(...) 
	{ 
		goto BLANK; 
	} 
	free(sla); 
	free(form_base); 
	form_base=NULL; 
	if(bform->db->last_cadr()<1) 
	{ 
		delete bform; 
		goto BLANK; 
	} 
	if(bform->db->last_cadr()==1) 
		page=1; 
	else 
	{ 
		char *ch=NULL; 
		int ind; 
		int f=term->get_box(0,0,term->l_x(),term->l_y()); 
		for(page=1,ind=1;page<=bform->db->last_cadr();page++) 
		{ 
			bform->db->Get_Slot(page,1,ch); 
			if(ch!=NULL && *ch!='.') 
				bform->put_Record(ind++,page); 
		} 
		if(ch!=NULL) 
			free(ch); 
		ch=NULL; 
		selection sel; 
		sel.num_index=ind-1; 
		CX_BROWSER *tmp=current_browser; 
		current_browser=bform; 
		bform->db->Sorting(1,&sel); 
		current_browser=tmp; 
		bform->Arr_To_Index(sel.index,ind-1); 
		struct sla sla[SLA_DEEP]; 
		bzero(sla,sizeof sla); 
		sla->n=1; 
		bform->Create_Map(sla,32); 
		if(ch!=NULL) 
			free(ch); 
		bform->act_field=1; 
		term->dpp(1+(term->l_x()-strlen(title))/2,bform->y0>0?bform->y0-1:0); 
		term->Set_Color(0,017); 
		term->dps(title); 
		bform->Go_To_Index(1); 
		if(bform->Move(0)=='\r') 
			page=bform->Act_Record(); 
		else 
		{ 
			delete bform; 
			term->restore_box(f); 
			term->free_box(f); 
			return(-1); 
		} 
		term->restore_box(f); 
		term->free_box(f); 
	} 
	delete bform; 
	form->form=page; 
	return(page); 
} 
 
int read_bytes(unsigned char *buf, int len,int t)
{ 
	struct timeval timeout;
	fd_set read_set;
	int i;
BEGIN:
	FD_ZERO(&read_set);
#ifdef WIN32
	FD_SET(sock_fd, &read_set);
#else
	FD_SET(0, &read_set);
#endif
	bzero(&timeout,sizeof timeout);
	if(t==0)
		timeout.tv_sec=refresh_delay;
	else    timeout.tv_sec=t;

	if((i=select(1,&read_set,NULL,NULL,&timeout))==0) 
	{ 
#ifndef WIN32
		if(getppid()<=1 || write(1,"\E\E",2)<=0)  // some kind of ping... 
		{ 
			if(cx!=NULL) 
				delete(cx); 
			delete term; 
			exit(0); 
		} 
#endif
		if(refresh_flag)
		{ 
			time_t t,tim=time(0); 
 
			refresh(0); 
 
			if((t=time(0)-tim)>DEFAULT_REFRESH_DELAY/10) 
				refresh_delay=t*10; 
			else    refresh_delay=DEFAULT_REFRESH_DELAY; 
		} 
		goto BEGIN; 
	} 
	if(i<0) 
	{ 
		if(cx!=NULL) 
			delete(cx); 
		delete term; 
		exit(0); 
	} 
#ifndef WIN32
	return(read(0,buf,len));
#else
	return(recv(sock_fd,(char *)buf,len,0));
#endif
} 
 
#ifndef WIN32 
int rebuild_tree(char *name,long record,struct sla *sla) 
{ 
	int pid,i; 
	if(!dial(message(64),1)) 
		return(-1); 
	if((pid=vfork())==0) 
	{ 
		CXDB_REPAIR *ram; 
		char str[LINESIZE]; 
 
		for(i=1;i<SIGUSR1;i++) 
			signal(i,SIG_IGN); 
		sprintf(str,"%s/db.log",name); 
		int fd=open(str,O_RDWR|O_CREAT,0666); 
		lseek(fd,0,SEEK_END); 
		sprintf(str,"%s tree_error: page=%ld n=%d\n",local_time(),record,sla->n); 
		write(fd,str,strlen(str)); 
		close(fd); 
		try 
		{ 
			ram=new CXDB_REPAIR (name,sla->n,1); 
		} 
		catch(...) 
		{ 
			exit(1); 
		} 
		ram->Create_Tree();
		delete ram; 
		exit(0); 
	} 
	waitpid(pid,&i,0); 
	return(0); 
} 
#endif
