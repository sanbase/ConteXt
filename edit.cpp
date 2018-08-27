/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:edit.cpp
*/
#include "stdafx.h" 
#include "CX_Browser.h" 
#ifndef WIN32 
#include <sys/file.h> 
#endif 
#include <time.h> 
 
extern CX_BROWSER *current_browser; 
extern CX_BROWSER *current_cx; 
extern Terminal *term; 
extern Line *EditLine; 
 
int CX_BROWSER::Edit(int ret,struct tag *tag) 
{ 
	char *str=NULL; 
	char *str_std=NULL; 
	long spage=0; 
	field field; 
	struct sla sla[SLA_DEEP]; 
	int n=0,tip; 
	char *selection=NULL; 
 
	if(!num_fields) 
		return(0); 
	str=(char *)malloc(LINESIZE+1); 
	str_std=(char *)malloc(16); 
	if(*db->share->output && ret==F7) 
	{ 
		char *arg=NULL; 
		int i; 
 
		if(get_arg(db->share->output,"-A",arg)) 
			str_to_sla(arg,sla); 
		else 
			str_to_sla(db->share->output,sla); 
		for(i=0;i<num_fields;i++) 
		{ 
			if(tags[i].index!=index) 
				continue; 
			if(!slacmp(tags[i].des.sla,sla)) 
			{ 
				tag=tags+i; 
				break; 
			} 
		} 
		if(i==num_fields)       // slot not found 
		{ 
			static struct tag tag_new; 
 
			bzero(&tag_new,sizeof tag_new); 
			bcopy(sla,tag_new.des.sla,sizeof tag_new.des.sla); 
			tag_new.des.l=32; 
			tag=&tag_new; 
		} 
		if(get_arg(db->share->output,"-S",arg)) 
			selection=arg; 
		*db->share->output=0; 
	} 
	bcopy(db->Field_Descr(tag->des.sla[0].n),&field,sizeof field); 
	if(ret==F7) 
	{ 
		if(field.a==X_VARIANT && tag->des.sla[1].n==1) 
			goto CHOISE1; 
		goto CHOISE2; 
	} 
	if(ret<' ')
		ret=' '; 
	if(form_cond&EDIT && record!=Record(tag->index)) 
		Write(); 
 
	form_update(); 
	if((tag->des.atr & NO_NED)==0 && ((field.a==X_STRING && field.l>255)||(field.a==X_TEXT && field.k==0)))
	{ 
#ifndef WIN32
		int f=term->get_box(0,0,term->l_x(),term->l_y()); 
		char name[64]; 
 
		sprintf(name,"%s/.vedit.%d",TEMPDIR,getpid());
		int fd=creat(name,0600); 
		char *ch=NULL; 
		struct sla sla[SLA_DEEP];
		bcopy(tag->des.sla,sla,sizeof sla);
		if(field.a==X_TEXT)
		{
			int i;
			for(i=0;sla[i].n;i++);
			if(i>0)
				sla[i-1].m=0;

		}
		int len=db->Get_Slot(Record(index),sla,ch);
		write(fd,ch,len); 
		free(ch); 
		char cmd[64]; 
		sprintf(cmd,"/usr/local/bin/ned %s %s",tag->des.atr&NO_EDIT?"-m":"",name); 
		delete_menu(); 
		term->Del_All_Images(); term->scrbufout();

		term->Put_Screen(1,2,2,term->l_x()/2,term->l_y()/2,25,20);

//                term->Put_Screen(1,2,2,40,20,20,20);
//                term->Set_Screen(1);

		term->Set_Color(0,2); term->clean();

		system(cmd);

		term->Del_Screen(1);

		load_menu(); 
		term->restore_box(f); 
		term->free_box(f); 
		sprintf(cmd,"%s.b",name); 
		if(tag->des.atr&NO_EDIT || access(cmd,R_OK) || (fd=open(name,O_RDWR))<0) 
		{ 
			unlink(name); 
			free(str); 
			free(str_std); 
			return(0); 
		} 
		unlink(cmd); 
		struct stat st; 
		fstat(fd,&st); 
		ch=(char *)malloc(st.st_size+1); 
		read(fd,ch,st.st_size); 
		ch[st.st_size]=0; 
		close(fd); 
		if(!Check_Line(tag,name)) 
		{ 
			form_cond|=EDIT; 
			db->Put_Slot(record=Record(index),tag->des.sla,ch); 
			if(ed!=NULL) 
				ed->Add(tag->des.sla,ch); 
		} 
		unlink(name); 
		free(ch); 
#else
		dial(message(78),4);
#endif
		goto END; 
	} 
	record=Record(index); 
 
	if(field.a==X_VARIANT) 
	{ 
CHOISE1: 
		char *name=db->Name_Subbase(tag->des.sla[0].n); 
		char *ch=NULL; 
		if(name==NULL) 
		{ 
			free(str); 
			free(str_std); 
			return(0); 
		} 
		if(tag->des.sla[1].n==1) 
		{ 
 
			if(ret==F7 || ret==0 || parent_record==0) 
			{ 
				char str1[64]; 
				struct sla sla[SLA_DEEP]; 
 
				bzero(sla,sizeof sla); 
				sla->n=1; 
				if((spage=Select_From_DB(name,sla,tag->des.l))<0) 
				{ 
					load_menu(); 
					ret=F10; 
					goto END; 
				} 
				sprintf(str1,"#%ld:0",spage); 
				sla->n=tag->des.sla->n;
				sla->m=tag->des.sla->m;
				db->Put_Slot(record=Record(index),sla,str1);
				form_cond|=EDIT; 
				load_menu(); 
				goto END; 
			} 
		} 
		else if(tag->des.sla[1].n==2) 
		{ 
			struct sla sla[SLA_DEEP];
 
			bzero(sla,sizeof sla);
			sla->n=tag->des.sla->n;
			sla->m=tag->des.sla->m;
			db->Get_Slot(record,tag->des.sla[0].n,ch); 
			spage=atoi(ch+1); 
			free(ch); 
		} 
	} 
	help_line(2); 
	if(tag->str!=NULL) 
	{ 
		str=(char *)realloc(str,strlen(tag->str)+LINESIZE+1); 
		strcpy(str,tag->str); 
	} 
	else 
		*str=0;
	if(ret==' ') 
		ret=0; 
	else    *str=0; 
	str_std=(char *)realloc(str_std,strlen(str)+1); 
	strcpy(str_std,str); 
	if(tag->mask!=NULL) 
	{ 
		strncpy(EditLine->mask,tag->mask,(sizeof EditLine->mask)-1); 
		if(!(*str))
			strncpy(str,EditLine->mask,LINESIZE); 
		EditLine->insert=0; 
	} 
	else    *EditLine->mask=0; 
	if(ret && *str)
		EditLine->insert=0; // if come with symbol - will be insert mode
EDT:
/*
	term->Set_Color((tag->des.color.bg||tag->des.color.fg)?tag->des.color.bg:tag->color.bg,
	(tag->des.color.bg||tag->des.color.fg)?tag->des.color.fg:tag->color.fg);
*/
	term->Set_Color(120,0);

	if((ret=EditLine->edit(ret,str,strlen(str)+LINESIZE,tag->des.l,x0+tag->des.x+1,y0+tag->des.y+1,(tag->des.atr&SECURE)?-1:0))=='\r' || ret=='\t') 
	{ 
		if(!strcmp(str,str_std))
		{ 
			ret=F10; 
			goto END; 
		} 
		if(field.a==X_VARIANT && tag->des.sla[1].n==1) 
		{ 
			char *name=db->Name_Subbase(sla);
			if(name==NULL) 
			{ 
				ret=F10; 
				goto END; 
			} 
			CX_BASE *flex=new CX_BASE(name); 
			CX_FIND fnd(flex); 
			char str1[LINESIZE]; 
 
			sprintf(str1,"%s/%s",db->Name_Base(),str); 
			long spage=fnd.Find_First(1,str1,0); 
			if(spage>0) 
			{ 
				delete flex; 
				sprintf(str1,"#%ld",spage); 
				db->Put_Slot(record=Record(index),tag->des.sla->n,str1); 
				form_cond|=EDIT; 
				load_menu(); 
				goto END; 
			} 
 
			term->Put_Screen(1,3,3,term->l_x()-6,term->l_y()-6,term->l_x(),term->l_y());
			term->clean(); 
			hot_line(message(63)); 
			sprintf(str1,"%s/%s",PROPERTY,str); 
			Make_Class(str1,NULL,2);
 
			term->Del_Screen(1); 
			spage=flex->New_Record(); 
			sprintf(str1,"%s/%s",PROPERTY,str); 
			flex->Put_Slot(spage,1,str1); 
			flex->Unlock(spage); 
			delete flex; 
 
			sprintf(str1,"#%ld",spage); 
			db->Put_Slot(record=Record(index),tag->des.sla->n,str1); 
			form_cond|=EDIT; 
			load_menu(); 
			goto END; 
		} 
		bzero(sla,sizeof (sla)); 
		for(n=0;tag->des.sla[n].n;n++) 
		{ 
			for(int j=0;j<=n;j++) 
			{ 
				sla[j].n=tag->des.sla[j].n; 
				sla[j].m=tag->des.sla[j].m; 
			} 
			if((tip=db->Field_Descr(sla)->a)==X_POINTER || tip==X_VARIANT) 
			{ 
				bcopy(db->Field_Descr(sla),&field,sizeof field); 
				sla[n+1].n=tag->des.sla[n+1].n; 
				if(tip==X_VARIANT) 
					sla[n+2].n=tag->des.sla[n+2].n; 
				break; 
			} 
		} 
		if(!tag->des.sla[n].n) 
		{ 
			n=0; 
			sla[1].n=0; 
			sla[1].m=0; 
			tip=db->Field_Descr(tag->des.sla)->a; 
		} 
/* 
		if(tip>=X_INTEGER && tip<=X_DOUBLE && *str) 
		{ 
			char str1[64]; 
 
			double rez=db->Expression(Record(index),str); 
			sprintf(str1,"%%.%df",db->Field_Descr(tag->des.sla)->n); 
			sprintf(str,str1,rez); 
		} 
*/ 
		if(!Check_Line(tag,str)) 
		{ 
			if((field.a==X_POINTER || field.a==X_VARIANT) && sla[1+(field.a==X_VARIANT)].n && *str!='#') 
			{ 
				if(!*str) 
				{ 
					bzero(str,sizeof str); 
					sla[n+1].n=0; 
					db->Put_Slot(record,sla,str); 
					if(ed!=NULL) 
						ed->Add(tag->des.sla,str); 
					form_cond|=EDIT; 
				} 
				else 
				{ 
					int r=0; 
					struct query query; 
					long page; 
					CX_BROWSER *subbase=NULL; 
 
					if(field.a==X_POINTER) 
					{ 
						struct sla sla1[SLA_DEEP]; 
 
						bzero(sla1,sizeof sla1); 
						bcopy(sla+n+1,sla1,(sizeof sla1)-(n+1)*sizeof (struct sla)); 
						try 
						{ 
							subbase = new CX_BROWSER(db->Name_Subbase(sla),1,sla1,tag->des.l+1); 
						} 
						catch(...) 
						{ 
							free(str); 
							free(str_std); 
							return(0); 
						} 
					} 
					else 
					{ 
						char *name=NULL; 
						CX_BASE soft(db->Name_Subbase(sla)); 
						soft.Get_Slot(spage,1,name); 
						try 
						{ 
							subbase = new CX_BROWSER(name,1,sla,tag->des.l); 
						} 
						catch(...) 
						{ 
							if(name!=NULL) 
								free(name); 
							free(str); 
							free(str_std); 
							return(0); 
						} 
						if(name!=NULL) 
							free(name); 
					} 
 
					current_browser=subbase; 
					current_cx=subbase; 
					if(subbase->db->last_cadr()<=0) 
					{ 
						goto NEW_REC; 
					} 
					bzero(&query,sizeof query); 
					query.str=str; 
					bcopy(sla+n+1+(field.a==X_VARIANT),query.sla,(SLA_DEEP-n-1)*sizeof (struct sla));
					if((r=subbase->Query(&query))==1) 
					{ 
						page=subbase->Record(1); 
						goto WRITE_DOWN; 
					} 
					else if(r<=0) 
					{ 
NEW_REC: 
						long *rec; 
						if(tag->des.atr & NO_NEW_REC) 
						{ 
							delete(subbase); 
							free(str); 
							free(str_std); 
							strcpy(db->share->output,message(41)); 
							return(0); 
						} 
						term->BlackWhite(0,0,term->l_x(),term->l_y()); 
						if(!dial(message(4),1)) 
						{ 
							delete(subbase); 
							free(str); 
							free(str_std); 
							term->MultiColor(x0,y0,l,h); 
							return(0); 
						} 
						term->MultiColor(x0,y0,l,h); 
						page=subbase->db->New_Record(); 
						subbase->db->Put_Slot(page,sla+n+1+(field.a==X_VARIANT),str); 
						if(subbase->db->Num_Fields()==1) 
							goto WRITE_DOWN; 
						subbase->x0=x0+1; 
						subbase->y0=y0+1; 
						subbase->initialize(NULL,page,0); 
						subbase->read_only=0; 
 
						rec=(long *)malloc(sizeof (long)); 
						*rec=page; 
						subbase->Arr_To_Index(rec,1); 
						free(rec); 
						subbase->Change_Form(); 
						strcpy(subbase->db->share->output,message(5)); 
						if(subbase->Action()=='\n') 
							goto WRITE_DOWN; 
						else 
						{ 
							delete(subbase); 
							current_cx=this; 
 
							free(str); 
							free(str_std); 
							return(0); 
						} 
					} 
					else 
					{ 
						subbase->x0=x0+1; 
						subbase->y0=y0+1; 
						subbase->initialize(NULL,1,1); 
						subbase->Change_Form(); 
						strcpy(subbase->db->share->output,message(7)); 
						if(subbase->Action()=='\n') 
						{ 
							page=subbase->Record(subbase->index); 
WRITE_DOWN: 
							form_cond|=EDIT; 
							sla[n+1].n=0; 
							if(field.a==X_POINTER) 
							{ 
								char str2[64]; 
								sprintf(str2,"#%ld",page); 
								db->Put_Slot(record=Record(index),sla,str2); 
							} 
							else 
							{ 

								char *str2=NULL; 
								db->Get_Slot(record=Record(index),sla,str2);
								if(str2!=NULL)
								{
									char str[64];
									long spage;
									sscanf(str2,"#%d:",&spage);
									sprintf(str,"#%d:%d",spage,page);
									db->Put_Slot(record,sla,str);
									free(str2);
								}
							} 
							if(ed!=NULL) 
							{ 
								char *ch=NULL; 
								db->Get_Slot(record,tag->des.sla,ch); 
								ed->Add(tag->des.sla,ch); 
								free(ch); 
							} 
						} 
					} 
					if(subbase!=NULL) 
						delete(subbase); 
					current_browser=this; 
					current_cx=this; 
				} 
			} 
			else 
			{ 
				int i=db->Put_Slot(record=Record(index),tag->des.sla,str); 
				if(i<0) 
				{ 
					db->share->color.bg=014; 
					db->share->color.fg=0; 
					if(i==-5) 
						sprintf(db->share->output,"Format error"); 
					else 
						sprintf(db->share->output,"Write Error:%d",i); 
				} 
				else 
				{ 
					form_cond|=EDIT; 
					if(ed!=NULL) 
						ed->Add(tag->des.sla,str); 
				} 
			} 
		} 
	} 
	else if(ret==F5) 
	{ 
		if(keep_str!=NULL && keep_str->str!=NULL) 
			strncpy(str,keep_str->str,LINESIZE-1); 
		ret=0; 
		goto EDT; 
	} 
	else if(ret==F9 && *str) 
	{ 
		if(keep_str!=NULL) 
			delete keep_str; 
		keep_str = new MString(str); 
		ret=0; 
		goto EDT; 
	} 
	else if(ret==F8) 
	{ 
		if(!strcmp(str,EditLine->mask))
			*str=0; 
		else strcpy(str,EditLine->mask); 
		ret=0; 
		goto EDT; 
	} 
	else if(ret==F3) 
	{ 
		if(keep_str!=NULL) 
			delete keep_str; 
		keep_str=NULL; 
		goto EDT; 
	} 
	else if(ret==F7) 
	{ 
CHOISE2: 
		if(field.a==X_IMAGE || field.a==X_FILENAME) 
		{ 
			char *ch=Select_From_Dir(".",if_file,"",1); 
			if(ch==NULL || !*ch)
			{ 
				if(ch!=NULL)
					free(ch);
				free(str); 
				free(str_std); 
				return(0); 
			} 
			str=(char *)realloc(str,strlen(ch)+1); 
			strcpy(str,ch); 
			free(ch); 
			if(form_cond&EDIT && record!=Record(tag->index)) 
				Write(); 
			form_cond|=EDIT; 
			db->Put_Slot(record=Record(index),tag->des.sla,str); 
			goto END; 
		} 
 
		struct sla sla[SLA_DEEP]; 
		int n=0; 
 
		bzero(sla,sizeof (sla)); 
		for(n=0;tag->des.sla[n].n;n++) 
		{ 
			sla[n]=tag->des.sla[n]; 
			if(db->Field_Descr(sla)->a==X_POINTER || db->Field_Descr(sla)->a==X_VARIANT) 
			{ 
				bcopy(db->Field_Descr(sla),&field,sizeof field); 
				break; 
			} 
		} 
		if(!tag->des.sla[n].n)   // вообще нет ссылок. 
		{ 
			n=0; 
			sla[1].n=0; 
			sla[1].m=0; 
		} 
		if(field.a==X_POINTER || field.a==X_VARIANT) 
		{ 
			long page; 
 
			if(field.a==X_POINTER) 
			{ 
				if((page=Select_From_DB(db->Name_Subbase(sla),tag->des.sla+n+1,tag->des.l+1,selection))<0) 
				{ 
					load_menu(); 
					ret=F10; 
					goto END; 
				} 
				load_menu(); 
			} 
			else 
			{ 
				char *name=NULL; 
 
				CX_BASE soft(db->Name_Subbase(sla)); 
				if(tag->des.sla[n+1].n==2) 
				{ 
					db->Get_Slot(record,sla,name); 
					spage=atoi(name+1); 
					soft.Get_Slot(spage,1,name); 
				} 
				else name=db->Name_Subbase(sla); 
 
				if((page=Select_From_DB(name,tag->des.sla+n+(tag->des.sla[n+1].n),tag->des.l,selection))<0) 
				{ 
					ret=F10; 
					goto END; 
				} 
			} 
			if(page>=0) 
			{ 
				str=(char *)realloc(str,LINESIZE); 
				if(field.a==X_POINTER)
				      sprintf(str,"#%ld",page);
				else
				{
					if(tag->des.sla[n+1].n==1)
						sprintf(str,"#%ld:0",page);
					else
						sprintf(str,"#%ld:%ld",spage,page);
				}
				if(!Check_Line(tag,str)) 
				{ 
					char str2[64]; 
 
					form_cond|=EDIT; 
					if(field.a==X_POINTER) 
					{ 
						sprintf(str2,"#%ld",page); 
						db->Put_Slot(record=Record(index),sla,str2); 
					} 
					else 
					{ 
						char *ch=NULL; 
 
						db->Get_Slot(record,sla,ch); 
						if(ch!=NULL) 
						{ 
							spage=atoi(ch+1); 
							free(ch); 
						} 
						else    spage=0; 
 
						if(tag->des.sla[n+1].n==1) 
							sprintf(str,"#%ld:0",page); 
						else 
							sprintf(str,"#%ld:%ld",spage,page); 
						db->Put_Slot(record=Record(index),sla,str); 
					} 
					if(ed!=NULL) 
					{ 
						char *ch=NULL; 
						db->Get_Slot(record,tag->des.sla,ch); 
						ed->Add(tag->des.sla,ch); 
						free(ch); 
					} 
				} 
			} 
		} 
		else 
		{ 
			CX_BROWSER base(db->Name_Base(),1,tag->des.sla,tag->des.l+1); 
			if(selection!=NULL && *selection) 
			{ 
				base.set_prim_selection(selection); 
				base.Read_Index(selection); 
				base.Go_To_Index(1); 
			} 
 
			term->BlackWhite(1,1,term->l_x()-2,term->l_y()-1); 
			current_browser=&base; 
			if((ret=base.Action())=='\r') 
			{ 
				char *str=NULL; 
 
				base.db->Get_Slot(base.record,base.tags[base.act_field].des.sla,str); 
				if(str!=NULL && !Check_Line(tag,str))
				{ 
					form_cond|=EDIT; 
					db->Put_Slot(record,tag->des.sla,str); 
					if(ed!=NULL) 
						ed->Add(tag->des.sla,str); 
					free(str); 
				} 
			} 
			term->MultiColor(1,1,term->l_x()-2,term->l_y()-1); 
			current_browser=this; 
		} 
	} 
END: 
	form_update(act_field+1); 
	if(db->share->cmd==c_Refresh) 
		form_update(); 
	else for(int i=0;db->share!=NULL && i<num_fields;i++) 
	{ 
		if(tags[i].des.sla->n>db->Num_Fields() || tags[i].des.sla->n==tag->des.sla->n) 
			form_update(i+1); 
		else if(form_cond&TABLE && tags[i].index==tag->index) 
			form_update(i+1); 
	} 
	if(selection!=NULL) 
		free(selection);
	free(str); 
	free(str_std); 
	return(ret); 
} 
 
ED::ED(char *file_name) 
{ 
	num=0; 
	e=NULL; 
	name=(char *)malloc(strlen(file_name)+1); 
	strcpy(name,file_name); 
} 
 
ED::~ED() 
{ 
	while(num) 
		free(e[--num].str); 
	if(e!=NULL) 
		free(e); 
	if(name!=NULL) 
		free(name); 
} 
 
void ED::Add(struct sla *sla,char *str) 
{ 
	for(int i=0;i<num;i++) 
	{ 
		if(!slacmp(sla,e[i].sla)) 
		{ 
			e[i].str=(char *)realloc(e[i].str,strlen(str)+1);
			strcpy(e[i].str,str); 
			return; 
		} 
	} 
	e=(struct edit_history *)realloc(e,++num*sizeof (struct edit_history)); 
	bcopy(sla,e[num-1].sla,sizeof e->sla); 
	e[num-1].str=(char *)malloc(strlen(str)+1);
	strcpy(e[num-1].str,str); 
} 
 
void ED::Reset() 
{ 
	while(num) 
		free(e[--num].str); 
	if(e!=NULL) 
		free(e); 
	num=0; 
	e=NULL; 
} 
 
void ED::Write(int page) 
{ 
	if(num) 
	{ 
		int fd=-1; 
		char str[128]; 
 
		*str=0; 
		if((fd=open(name,O_RDWR))>0) 
		{ 
//                        flock(fd,LOCK_EX); 
			time_t i=time(0); 
			sprintf(str,"\n\nRecord:%d changed by %s. %s",page,GetLogin(),ctime(&i)); 
			lseek(fd,0,SEEK_END); 
			write(fd,str,strlen(str)); 
		} 
		free(name); 
		name=NULL; 
		for(int i=0;i<num;i++) 
		{ 
			if(fd>0) 
			{ 
				sla_to_str(e[i].sla,str); 
				write(fd,str,strlen(str)); 
				write(fd,"\t->",3); 
				write(fd,e[i].str,strlen(e[i].str)); 
				write(fd,"\n",1); 
			} 
			free(e[i].str); 
		} 
		free(e); 
		e=NULL; 
		if(fd>=0) 
			close(fd); 
	} 
	num=0; 
} 
