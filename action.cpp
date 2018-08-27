/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:action.cpp
*/
#include "stdafx.h"

#include "CX_Browser.h"
#include <time.h>

extern CX_BROWSER *current_browser;
CX_BROWSER *current_cx=NULL;
extern Terminal *term;
extern Line *EditLine;
extern int demo;
extern void serialization();
extern int comp_d(const void *a1,const void *a2);

struct Open_CX
{
	char *name;
	struct x_form name_form;
	char passwd[16];
};

static int  num_open_cx;       // number of opened subbases
static struct Open_CX *open_cx;

void intro()
{
	term->scrbufout();
	term->ShowDocument("http://unixspace.com/context/help.html");
	fflush(stdout);
}

int CX_BROWSER::Action()
{
	int ret;
	form_update();
	for(ret=0;;ret=0)
	{
BEGIN:
		ret=Move(ret);

		if(ret==27)
			continue;
		if(ret>=1100)
		{
			ret=Cmd_Exe(ret-1000);
			if(ret==F10)
				goto END;
			continue;
		}
		switch(ret)
		{
			case 1000:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_GoNext);
				else
					ret=Cmd_Exe(c_GoLast);
				break;
			case 1001:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_GoPrev);
				else
					ret=Cmd_Exe(c_GoFirst);
				break;
			case 1002:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_SortA);
				else
					ret=Cmd_Exe(c_SortZ);
				break;
			case F8:
				if(form_atr&NOMENU)
					break;
			case 1003:
				if(form_cond&MAP || cx_cond&HIST)
				{
					ret=F8;
					goto END;
				}
				if(term->ev().b==1)
					ret=Cmd_Exe(c_RestIdx);
				else
					ret=Cmd_Exe(c_NoSel);
				break;
			case '`':
				if(form_atr&NOMENU)
					break;
			case 1004:
				ret=Cmd_Exe(c_NewRec);
				break;
			case F9:
				if(form_atr&NOMENU)
					break;
			case 1005:
				ret=Cmd_Exe(c_Calc);
				break;
			case 1006:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_Chart);
				else    ret=Cmd_Exe(c_Array);
				break;
			case 1007:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_Bar);
				else    ret=Cmd_Exe(c_Pie);
				break;
			case 1008:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_Show);
				else
					ret=Cmd_Exe(c_PutSel);
				break;
			case F6:
				if(form_atr&NOMENU)
					break;
				if(form_cond&MARK)
				{
					ret=Cmd_Exe(c_Chart);
					break;
				}
			case 1009:
				if(form_atr&NOMENU)
					break;
				ret=Cmd_Exe(c_Save);
				break;
			case F4:
				if(form_atr&NOMENU)
					break;
			case 1010:
				ret=Cmd_Exe(c_ChangeForm);
				break;
			case F2:
			case F5:
				if(form_atr&NOMENU)
					break;
			case 1011:
				if((ret==1011 && term->ev().b==1) || (ret==F2 && !(form_cond&TABLE) && !(form_cond&MAP)) || ret==F5)
					form_cond|=MARK;
				Cmd_Exe(c_Mark);
				if((form_cond&TABLE || form_cond&MAP) && !(form_cond&MARK))
					ret=CD;
				else
				{
					if(form_cond&MARK && num_mark_fields>1)
					{
						int i1=find_slot(mark_field[num_mark_fields-1].des.sla);
						int i2=find_slot(mark_field[num_mark_fields-2].des.sla);

						if(i1>=0 && i2>=0 && tags[i1].des.y>tags[i2].des.y)
							ret=CD;
						else    ret=CR;
					}
					else
					{
						if(form_cond&TABLE)
							ret=CR;
						else
							ret=CD;
					}
				}
				goto BEGIN;
			case 1012:
				Cmd_Exe(c_Exit);
				goto END;
			case F7:
				if(form_atr&NOMENU)
					break;
			case 1013:
				ret=Cmd_Exe(c_Map);
				break;
			case 1014:
				if(term->ev().b==1)
					ret=Cmd_Exe(c_Struct);
				else
					ret=Cmd_Exe(c_Stack);
				break;
			case 1015:
				ret=Cmd_Exe(c_Modify);
				break;
#ifdef DEBUG
			case 1016:
				ret=Cmd_Exe(c_Shell);
				break;
#endif
			case IS:
				term->Set_Color(8,017);
				term->clean();
				form_update();
				break;
			case F1:
				protocol("Help");
				Help(1,level+1);
				current_cx=this;
				break;
			case F3:
			{
				int i;
				if(form_atr&NOMENU)
					break;
				if((i=Cmd_Exe(c_Menu))<0)
				{
					ret=F10;
					goto BEGIN;
				}
				if(i==c_ChangeDB)
					return(c_ChangeDB);
				if(i==c_Total)
				{
					ret=0;
					goto BEGIN;
				}
				break;
			}
			case 0:
				if(term->ev().b==1)
					break;
			case '\n': //
				if(x0==0)
					break;
			case F10:
				if(cx_cond&HIST)
				{
					cx_cond&=~HIST;
					Cadr_Read();
					Go_To_Index(index);
					break;
				}
				Cmd_Exe(c_Exit);
				goto END;
			case DEL:
				if(form_atr&NOMENU)
					break;
				if(form_cond&MAP)
				{
					if(*obr)
						obr[strlen(obr)-1]=0;
					goto NEW_MAP;
				}
				//fill
			if(read_only)
			{
				dial(message(15),4);
				break;
			}

				ret=Cmd_Exe(c_DelRest);
				break;
			case PD:
				ret=Cmd_Exe(c_GoNext);
				break;
			case PU:
				ret=Cmd_Exe(c_GoPrev);
				break;

			case F11:
				ret=Cmd_Exe(c_GetProperty);
//ret=Cmd_Exe(c_ShowForm);
				break;

//                                if(form_atr&NOMENU)
//                                        break;
//                                ret=Cmd_Exe(c_ShowDel);
//                                break;

			case F12:
				if(form_atr&NOMENU)
					break;
				if(form_cond&MARK)
				{
					ret=Cmd_Exe(c_Map);
					break;
				}
				if(num_mark)
				{
					if(Write())
						break;
					Mark_To_Index();
					if(form_cond&MAP)
						goto NEW_MAP;
					Go_To_Index(index);
				}
				else
				{
					if(Write())
						break;
					ret=Command();
					if(ret)
					{
						repaint();
						goto BEGIN;
					}
					if(form_cond&MAP)
						goto NEW_MAP;

				}
				repaint();
				break;
			case '\r':
				if(strstr(db->Name_Base(),"_HistDB"))
				{
					term->dpp(0,0);
					term->Set_Color(8,017);
					term->clean();

					char *str=NULL;
					db->Get_Slot(record,"^6.1",str);
					int w=atoi(str);
					db->Get_Slot(record,"^6.2",str);
					int h=atoi(str);
					db->Get_Slot(record,5,str);

					struct pics *scr=(struct pics *)str;

					for(int i=0;i<h;i++)
					{
						term->dpp(0,i);
						for(int j=0;j<w;j++)
						{
							term->Set_Color(scr[i*w+j].clr);
							term->dpo(scr[i*w+j].ch);
						}
					}
					if(str!=NULL)
						delete str;
					term->dpi();

					break;
				}
				{
					int i=0;
					int field=tags[act_field].des.sla->n;

					if(form_cond&MAP)
					{
						Go_To_Index(index=tags[act_field].index);
						goto END;
					}
					if(read_only==2 && field<=db->Num_Fields())
						goto END;
					if((i=Cmd_Exe(ret))==F10)
					{
						ret=i;
						goto END;
/*
						ret=0;
						goto BEGIN;
*/
					}
					if(i==c_ChangeDB)
						return(c_ChangeDB);
					if(i==c_Choise || field>db->Num_Fields())
					{
						ret=i;
						break;
					}
					if(db->Field_Descr(tags[act_field].des.sla)->a==X_FILENAME)
					{
						char *ch=NULL;
						int len=db->Get_Slot(Record(index),tags[act_field].des.sla,ch);
						if(len==0 || ch==NULL)
							break;
						char *str=(char *)malloc(strlen(ch)+64);
						sprintf(str,"/usr/local/bin/Window %s",ch);
						free(ch);
						system(str);
						free(str);
						break;
					}
					if(db->Field_Descr(tags[act_field].des.sla)->a==X_IMAGE)
					{
						char *ch=NULL;
						int len=db->Get_Slot(Record(index),tags[act_field].des.sla,ch);
						if(ch!=NULL && len!=0)
						{
							char name[256], *n,sl[64];

							sla_to_str(tags[act_field].des.sla,sl);
							if((n=strrchr(db->Name_Base(),'/'))==NULL)
								n=db->Name_Base();
							else    n++;
							sprintf(name,"/var/www/docs/Images/Tmp/.%d.%s.%d.%s",getpid(),n,(int)Record(index),sl);
							int fd=creat(name,0600);
							write(fd,ch,len);
							close(fd);
							free(ch);
							term->Show_Image(x0+tags[act_field].des.x-colon+1,y0+tags[act_field].des.y-line+1,name+14,NULL);

							term->dpp(0,0);
							term->scrbufout();
							ret=term->dpi();
							unlink(name);
							term->Del_Image(x0+tags[act_field].des.x-colon+1,y0+tags[act_field].des.y-line+1);
							if(ret=='\r')
								break;
							goto BEGIN;
						}
						break;
					}
				}
				if(read_only==2)
					goto END;
				if(cx_cond&HIST)
					goto END;
				if(!(form_cond&MAP) && db->is_pointer(tags[act_field].des.sla))
				{
					if(ret=='\r')
					{
						if((ret=Cmd_Exe(c_Recurs))==c_ChangeDB)
							return(ret);
						break;
					}
				}
//                                break;
			default:
				if(form_cond&MAP)
				{
					obr[strlen(obr)]=ret;
NEW_MAP:
					struct sla sla[SLA_DEEP],*sl;
					int len=tags[0].des.l;

					bcopy(tags[act_field].des.sla,sla,sizeof sla);
					sl=sla;
					if((index=find_page(Record(tags[act_field].index)))<0)
						index=1;
					record=Record(index);
					Create_Map(sl,len);
					break;
				}
				else if(!read_only && !(tags[act_field].des.atr&NO_EDIT && db->Field_Descr(tags[act_field].des.sla)->a!=X_TEXT))
				{
					if(!(form_cond&EDIT))
					{

						if (tags[act_field].des.sla->n<=db->Num_Fields())
						{
							if(db->Wlock(record,0))
							{
								strcpy(db->share->output,message(42));
								break;
							}
						}
					}
					term->cursor_visible();
					protocol("Edit");

					ret=Edit(ret,tags+act_field);

					if(!(form_cond&EDIT))
						db->Unlock(record,0);

					term->cursor_invisible();
					if(form_cond&ARRAY && tags[act_field].des.sla->m)
					{
						struct tag_descriptor td;

						bcopy(&tags[act_field].des,&td,sizeof td);
						create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
						create_Form(db->Name_Base());
						Form_Restruct();
						New_Index();
						Go_To_Field(td,Record(index));
					}
					form_update();
					if(ret=='\t')
						goto BEGIN;
//                                        if(ret<100)
						ret=0;
				}
				else
				{
					if(read_only==2)
						goto END;
					if(read_only)
						strcpy(db->share->output,message(15));
					else
						strcpy(db->share->output,message(16));
				}
				break;
		}
		if(ret==F10)
			break;
	}
END:
	if(ret==F10 && form_cond&MARK)
	{
		num_mark_fields=0;
		form_cond&=~MARK;
		ret=0;
		goto BEGIN;
	}
	Write();
	return(ret);
}

int CX_BROWSER::if_mark(long rec)
{
	for(int i=0;i<num_mark;i++)
		if(mark[i]==rec)
			return(i+1);
	return(0);
}

int CX_BROWSER::Cmd_Exe(int cmd)
{
	int edt,ret=0;
	if(!(form_cond&EDIT))
	{
		db->update();
		if(tags!=NULL)
			db->Cadr_Read(record=Record(index=tags[act_field].index));
		edt=0;
	}
	else    edt=1;
	if(db->share!=NULL)
		bcopy(&name_form,&db->share->form,sizeof name_form);

	if(!(cmd&EXECONT))
	{
		char std[sizeof db->share->output];
		if(db->share!=NULL)
			bcopy(db->share->output,std,sizeof db->share->output);
		ret=Event(cmd);
		if(ret && demo && rand()%100==1)
			serialization();
		if(db->share!=NULL && !(*db->share->output))
			bcopy(std,db->share->output,sizeof db->share->output);

		Go_To_Field(db->share->slot,tags[act_field].index);
	}
	else    cmd&=~EXECONT;
	if(edt==0 && form_cond&EDIT)
		edt=1;
	else    edt=0;
	if(ret & EXECONT)
	{
		ret=Cmd_Exe(ret);
		if(ret==F10)
			return F10;
		ret=0;
	}
	if(ret)
	{
		if(ret==c_ChangeDB || ret==c_Exit)
		{
			if(db->share!=NULL)
			{
				char std[sizeof db->share->output];
				bcopy(db->share->output,std,sizeof db->share->output);
				*db->share->output=0;
				if(Write())
					return(F10);
				bcopy(std,db->share->output,sizeof db->share->output);
			}
			else
				if(Write())
					return(F10);
			return(ret==c_Exit?F10:ret);
		}
		cmd=ret;
	}
	if(edt && db->share!=NULL)
	{
		char std[sizeof db->share->output];
		if(db->share!=NULL)
		{
			bcopy(db->share->output,std,sizeof db->share->output);
			*db->share->output=0;
		}
		CX_Show();
		form_update();
		if(db->share!=NULL)
			bcopy(std,db->share->output,sizeof db->share->output);
	}
BEGIN:
	switch(cmd)
	{
		case c_GoUp:
			Go(CU);
			break;
		case c_GoDown:
			Go(CD);
			break;
		case c_GoRight:
			Go(CR);
			break;
		case c_GoLeft:
			Go(CL);
			break;
		case c_Exit:
			Write();
			protocol("Exit");
			return F10;
		case c_SelSave:
		case c_SelRest:
		{
			char *name;
			char *dir;
			if(db->share!=NULL &&  *db->share->output)
			{
				dir=(char *)malloc(strlen(db->share->output)+1);
				strcpy(dir,db->share->output);
				*db->share->output=0;
			}
			else
			{
				dir=get_user_dir(db->Name_Base(),INDEXDIR);
				if(dir==NULL)
				{
					break;
				}
				if(get_filename(dir,&name,0)==F10)
				{
					free(dir);
					term->MultiColor(x0,y0,l,h);
					break;
				}
				term->MultiColor(x0,y0,l,h);
				strcat(dir,"/");
				strcat(dir,name);
				free(name);
			}
			record=Record(index);
			if(cmd==c_SelSave)
				Write_Index(dir);
			else
				Read_Index(dir);
			free(dir);
			index=find_page(record);
			Form_Restruct();
			New_Index();
			Go_To_Index(index);
			break;
		}
		case c_Request:
			if(Write())
			{
				cmd=0;
				break;
			}
			Command();
			repaint();
			break;
		case c_ChangeDB:
			protocol("c_ChangeDB");
			if(Write())
			{
				cmd=0;
			}
			break;
		case c_GoLast:
			if(Write())
			{
				cmd=0;
				break;
			}
			Go_To_Index(db->max_index);
			break;
		case c_GoFirst:
			if(Write())
			{
				cmd=0;
				break;
			}
			Go_To_Index(1);
			break;
		case c_Save:
			if(Write())
			{
				cmd=0;
				break;
			}
			break;
		case c_EditForm:
		{
			struct x_form std;
			protocol("c_EditForm");
			delete_menu();
			bcopy(&name_form,&std,sizeof std);
			Edit_Form(db,(int)name_form.form);
			term->BlackWhite(0,0,term->l_x()+1,term->l_y());
			term->cursor_invisible();
			Free_Form();

			create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
			Get_Form(db->Name_Base(),&std);
			Form_Restruct();
			New_Index();
			Go_To_Index(index);
			term->MultiColor(x0,y0,l,h);
			CX_Show();
			load_menu();
			break;
		}
		case c_Modify:
			protocol("c_Modify");
			if(!(cx_cond&HIST))
			{
				if(read_only)
				{
					strcpy(db->share->output,message(15));
					break;
				}
				term->BlackWhite(0,0,term->l_x()+1,term->l_y());
				term->cursor_visible();
				Modify();
				term->cursor_invisible();
				term->BlackWhite(0,0,term->l_x(),term->l_y());
				term->MultiColor(x0,y0,l,h);
			}
			break;
		case c_PutForm:
			protocol("c_PutForm");
			Put_Selection(0);
			break;
		case c_PutSel:
			protocol("c_PutSel");
			if(*db->share->output)
			{
				Put_Selection(0,db->share->output);
				bzero(db->share->output,sizeof db->share->output);
			}
			else
				Put_Selection(1);
			break;
		case c_Show:
			{
				char str[64];

				protocol("c_Show");
#ifndef WIN32
				sprintf(str,"%s/.vpf%d",TEMPDIR,getpid());
#else
				sprintf(str,"%s/%d",TEMPDIR,getpid());
#endif
				Export(str);
				term->Frame(str);
				unlink(str);
			}
			break;
		case c_HTML:
			{
				char str[64];
				int i,n;
				char *ch="<HTML><META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=KOI8-R\"><BODY BGCOLOR=#508090><TABLE BORDER=2 BGCOLOR=#C0D0C0><TR><TD>\n";
				char *table_start="<TABLE BORDER=2 BGCOLOR=#FFE0BB WIDTH=50%><TR>\n";
				char *table_end="</TR></TABLE>\n";
				char *td="<TD BGCOLOR=#b7c0b0><FONT SIZE=-1>";
#ifndef WIN32
				sprintf(str,"%s/.vpf%d.html",TEMPDIR,getpid());
#else
				sprintf(str,"%s/%d.html",TEMPDIR,getpid());
#endif
				int fd=creat(str,0666);

				write(fd,ch,strlen(ch));
				write(fd,table_start,strlen(table_start));
				ch="</TR>\n<TR>";
				if(!(form_cond&TABLE))
				{
					int tabset1[256];
					int tabset2[256];
					int num_tab1=0;
					int num_tab2=0;
					int line=tags[0].des.y;
					int start_field=0;
					int end_field=0;

					term->BlackWhite(0,0,term->l_x(),term->l_y());
					int total_form=dial(message(43),0);
					term->BlackWhite(0,0,term->l_x(),term->l_y());
					term->MultiColor(x0,y0,l,h);
					for(i=0;i<num_fields;i++)
					{
						if(i==num_fields-1)
						{
							end_field=i+1;
							tabset1[num_tab1++]=tags[i].des.x;
							goto BROWS;
						}

						if(tags[i].des.y==line)
						{
							if(num_tab1==0 && tags[i].des.x>5)
								tabset1[num_tab1++]=0;
							tabset1[num_tab1++]=tags[i].des.x;
							continue;
						}

						if(num_tab2==0)
							goto SHIFT;
						if(!bcmp(tabset1,tabset2,num_tab2*sizeof (int)))
						{
SHIFT:
							end_field=i;
							bcopy(tabset1,tabset2,sizeof tabset1);
							line=tags[i].des.y;
							num_tab2=num_tab1;
							bzero(tabset1,sizeof tabset1);
							num_tab1=0;
							if(tags[i].des.x>5)
								tabset1[num_tab1++]=0;
							tabset1[num_tab1++]=tags[i].des.x;
							continue;
						}

						int *s1,*s2;
						int *b1,*b2;
						if(num_tab1>num_tab2)
						{
							s1=&num_tab1;
							s2=&num_tab2;
							b1=tabset1;
							b2=tabset2;
						}
						else
						{
							s2=&num_tab1;
							s1=&num_tab2;
							b2=tabset1;
							b1=tabset2;
						}
						for(n=0;n<*s2;n++)
						{
							int j;
							for(j=0;j<*s1;j++)
							{
								if(b2[n]==b1[j])
									break;
							}
							if(j==*s1)
								goto BROWS;

						}
						end_field=i;
						bcopy(b1,tabset2,sizeof tabset1);
						line=tags[i].des.y;
						num_tab2=*s1;
						bzero(tabset1,sizeof tabset1);
						tabset1[0]=tags[i].des.x;
						num_tab1=1;
						continue;
BROWS:
						int current_tab=0;
						for(n=start_field;n<end_field;n++)
						{
							if(n>start_field && tags[n-1].des.y!=tags[n].des.y)
							{
								write(fd,ch,strlen(ch));
								current_tab=0;
							}

							while(current_tab<num_tab2 && tags[n].des.x!=tabset2[current_tab++])
							{
								char *ch="";

								if(background!=NULL && !total_form && current_tab>0)
								{
									char *str=(char *)calloc(strlen(background)+1,1);

									ch=(char *)background_pos(tabset2[current_tab-1],tags[n].des.y);
									if(ch!=NULL && tabset2[current_tab]>tabset2[current_tab-1])
									{
										strncpy(str,ch,tabset2[current_tab]-tabset2[current_tab-1]);
										if((ch=strchr(str,'\n'))!=NULL)
											*ch=0;
										for(ch=str;*ch;ch++)
										{
											int c=(unsigned char)*ch;
											if(c<' ' || (c>=176 && c<=223))
												*ch=' ';
										}
									}
									ch=pc_demos(str);
									free(str);
								}
								if(total_form)
									write(fd,"<TD></TD>",9);
								write(fd,td,strlen(td));
								write(fd,ch,strlen((char *)ch));
								write(fd,"</TD>",5);
							}

							char *name=db->Name_Field(tags[n].des.sla);
							if(total_form)
							{
								name=pc_demos(name);
								write(fd,td,strlen(td));
								write(fd,name,strlen(name));
								write(fd,"</TD>",5);
							}
							write(fd,"<TD>",4);
							name=pc_demos(tags[n].str);
							write(fd,name,strlen(name));
							write(fd,"</TD>",5);
						}
						write(fd,table_end,strlen(table_end));
						write(fd,table_start,strlen(table_start));

						bcopy(tabset1,tabset2,sizeof tabset1);
						num_tab2=num_tab1;
						start_field=end_field;
						end_field=i;
						tabset1[0]=tags[i].des.x;
						num_tab1=1;
						line=tags[i].des.y;
					}
				}
				else
				{
					long page;
					char *slot=NULL;

					for(i=first_line;tags[i].des.atr&TABL;i++)
					{
						char *ch=db->Name_Field(tags[i].des.sla);
						write(fd,td,strlen(td));
						ch=pc_demos(ch);
						write(fd,ch,strlen(ch));
						write(fd,"</TD>",5);
						if(tags[i].index!=tags[i+1].index)
							break;
					}
					write(fd,ch,strlen(ch));
					for(page=1;page<=db->max_index;page++)
					{
						if(db->Check_Del(Record(page)))
							continue;
						for(i=first_line;tags[i].des.atr&TABL;i++)
						{
							if(tags[i].des.sla->n==0)
							{
								slot=(char *)realloc(slot,32);
								sprintf(slot,"%d",(int)page);
							}
							else
								Get_Slot(Record(page),tags+i,slot);

							write(fd,"<TD>",4);
							char *ch=pc_demos(slot);
							write(fd,ch,strlen(ch));
							write(fd,"</TD>",5);
							if(tags[i].index!=tags[i+1].index)
								break;
						}
						write(fd,ch,strlen(ch));
					}
					if(slot!=NULL)
						free(slot);
				}
				write(fd,table_end,strlen(table_end));
				ch="</TD></TR></TABLE>\n</BODY></HTML>";
				write(fd,ch,strlen(ch));
				close(fd);
				term->Frame(str);
				unlink(str);
			}
			break;
		case c_ShowSel:
			{
				char str[64];

				protocol("c_ShowSel");
#ifndef WIN32
				sprintf(str,"%s/.vpf%d",TEMPDIR,getpid());
#else
				sprintf(str,"%s/%d",TEMPDIR,getpid());
#endif
				Put_Selection(1,str);
				term->Frame(str);
				unlink(str);
			}
			break;
		case c_Chart:
		case c_Array:
			{
				int ind=1;

				if(!num_mark_fields)
				{
					form_cond|=MARK;
					if(Cmd_Exe(c_Mark)==F10)
						return(F10);
				}
				form_cond&=~MARK;
				protocol("c_Chart");
				if(cmd==c_Chart)
				{
					if(num_mark_fields>1)
					{
						term->BlackWhite(0,0,term->l_x(),term->l_y());
						ind=dial(message(11),1);
						term->BlackWhite(0,0,term->l_x(),term->l_y());
						term->MultiColor(x0,y0,l,h);
					}
					Draw_Chart(!ind);
				}
				else
					Draw_Chart(3);
				load_menu();
				break;
			}
		case c_Pie:
			protocol("c_Pie");
			delete_menu();
			Draw_Graph(0);
			load_menu();
			break;
		case c_Bar:
			protocol("c_Bar");
			delete_menu();
			Draw_Graph(1);
			load_menu();
			break;
		case c_Intens:
			protocol("c_Intens");
			Intensity_Graph();
			load_menu();
			break;
		case c_Shell:
			{

				int f=term->get_box(0,0,term->l_x(),term->l_y());
				protocol("c_Shell");
				term->cursor_visible();
				delete_menu();
				delete term;

				if(*db->share->output)
					system(db->share->output);
				else
					system("/usr/local/bin/ss");
				bzero(db->share->output,sizeof db->share->output);
				term = new Terminal();
				term->l_x((term->l_x()-4));
				term->dpp(0,0); term->Set_Color(8,017);
				term->clean();
				term->restore_box(f);
				term->free_box(f);
				term->cursor_invisible();
				CX_Show();
				load_menu();
				break;
			}
		case c_Create_Class:
			{
/*
			if (getuid()!=0)
			 break;
*/
				char name[64];
				sprintf(name,message(71));
				if(dial(name,5)=='\r')
					Make_Class(name,NULL,1);
			}
			break;
		case c_ShowSum:
			Show_Sum();
			break;
		case c_Total:
			total(index);
			break;
		case c_Hist:
			protocol("c_Hist");
			if(Write())
				break;
			cx_cond|=HIST;
			hist_record=0;
			Cadr_Read();
			Go_To_Index(index);
			break;
		case c_Struct:
			protocol("c_Struct");
			delete_menu();
			term->scrbufout();
			term->Del_All_Objects(LABEL);
			Show_Structure(&db->ss,0,db->Short_Name(),0);
			term->BlackWhite(0,0,term->l_x(),term->l_y());
			term->MultiColor(x0,y0,l,h);
			load_menu();
			break;
		case c_Menu:
#ifdef PMENU
			if(*db->share->output && tags[act_field].des.sla[0].n>db->Num_Fields())
			{
				protocol("c_Menu");
				char *str=NULL;

				if(*db->share->output=='/')
				{
					int fd=open(db->share->output,O_RDWR|O_BINARY);
					if(fd<0)
					{
						bzero(db->share->output,sizeof db->share->output);
						break;
					}
					struct stat st;
					fstat(fd,&st);
					char *menu=(char *)malloc(st.st_size+1);
					read(fd,menu,st.st_size);
					menu[st.st_size]=0;
					close(fd);
					unlink(db->share->output);

					PMenu(menu,&str);

					free(menu);

				}
				else
					PMenu(db->share->output,&str);

				bzero(db->share->output,sizeof db->share->output);
				if(str==NULL)
					break;

				if(!Check_Line(str))
				{
					form_cond|=EDIT;
					form_update();
				}
				free(str);
				break;
			}
			else
			{
				char *str=NULL;

				PMenu(menu,&str);
				if(str!=NULL)
				{
					cmd=atoi(str);
					free(str);
				}
				if(cmd)
				{
					if(cmd<0)
						return(-1);
					cmd+=99;
					goto BEGIN;
				}
				break;
			}
#else
			if((cmd=Menu())!=0)
			{
				goto BEGIN;
			}
#endif
			break;
		case c_Mark:
			if(form_cond&MARK)
			{
				int mark;

				protocol("c_Mark");
				mark=if_mark_field(act_field);
				if(mark)
				{
					if(mark_field[mark-1].name!=NULL)
						free(mark_field[mark-1].name);
					num_mark_fields--;
					bcopy(mark_field+mark,mark_field+mark-1,(num_mark_fields-mark+1)*sizeof (struct slot));
					if(!num_mark_fields)
						form_cond&=~MARK;
					break;
				}
				mark_field=(struct tag *)realloc(mark_field,++num_mark_fields*sizeof (struct tag));
				bcopy(&tags[act_field].des,&mark_field[num_mark_fields-1].des,sizeof (struct tag_descriptor));
				char *name=db->Name_Field(tags[act_field].des.sla);
				if(name!=NULL)
				{
					mark_field[num_mark_fields-1].name=(char *)malloc(strlen(name)+1);
					strcpy(mark_field[num_mark_fields-1].name,name);
				}
				else mark_field[num_mark_fields-1].name=NULL;
				break;
			}
		case c_MarkRec:
			protocol("c_MarkRec");
			{
				int j;

				if(!(j=if_mark(Record(tags[act_field].index))))
				{
					mark=(long *)realloc(mark,++num_mark*sizeof (long));
					mark[num_mark-1]=Record(tags[act_field].index);
				}
				else
				{
					bcopy(mark+j,mark+j-1,(num_mark-j)*sizeof (long));
					num_mark--;
				}
			}
			break;
		case c_Stack:
			protocol("c_Stack");
			Show_Stack();
			break;
		case c_SortA:
		case c_SortZ:
		{
			selection sel;
			struct sla **sla;

			protocol("c_Sort");
			if(cx_cond&HIST)
				break;
			if(db->cx_cond&SORT)
			{
				sel.num_index=db->max_index;
				current_browser=this;
			}
			record=Record(index=tags[act_field].index);
			sla = (struct sla **)calloc((num_mark_fields+1),sizeof (struct sla *));
			//fill если не было никакой выборки сформируем, чтобы выкинуть удаленные
			if (sel.num_index<=0)
			{
				long tmpl=db->Select(1,"*",&sel);
				if(Arr_To_Index(sel.index,tmpl)>0)
				{
					struct query query;

					query.sla[0].n=1;
					query.str="*";
					Add_Stack(&query,tmpl);
				}
			}
			if(!num_mark_fields)
			{
				*sla=tags[act_field].des.sla;
				db->Sorting(sla,1,&sel,cmd==c_SortZ);
			}
			else
			{
				for(int i=0;i<num_mark_fields;i++)
				{
					sla[i]=mark_field[i].des.sla;
				}
				db->Sorting(sla,num_mark_fields,&sel,cmd==c_SortZ);
				form_cond&=~MARK;
				num_mark_fields=0;
			}
			if(Arr_To_Index(sel.index,db->max_index)>0)
			{
				struct query query;

				bcopy(sla,query.sla,sizeof query.sla);
				query.str="Sorting";
				Add_Stack(&query,db->max_index);
			}
			free(sla);
			Go_To_Index(index);
			break;
		 }
		case c_ChangeForm:
		{
			if(form_cond&MAP)
				break;

			protocol("c_ChangeForm");
			char std[sizeof db->share->output];
			if(db->share!=NULL)
			{
				bcopy(db->share->output,std,sizeof db->share->output);
				*db->share->output=0;
			}
			if(Write())
				break;
			if(db->share!=NULL)
				bcopy(std,db->share->output,sizeof db->share->output);
			term->BlackWhite(0,0,term->l_x(),term->l_y());
			Change_Form();
			term->BlackWhite(0,0,term->l_x(),term->l_y());
			term->MultiColor(x0,y0,l,h);

			break;
		}
		case c_RestIdx:
			protocol("c_RestIdx");
			if(db->cx_cond&SORT && !(cx_cond&HIST))
			{
				Write();
				Rest_Index();
				Go_To_Index(index);
			}
			break;
		case c_NoSel:
			protocol("c_NoSel");
			if(db->cx_cond&SORT && !(cx_cond&HIST))
			{
				while(db->insert)
				{
					int ins=db->insert;
					Rest_Index();
					if(ins==db->insert)
						break;
				}

				Go_To_Index(index);
			}
			break;
		case c_DelField:
			if(db->share!=NULL && db->share->field->des.sla->n)
			{
				int i;
				int y=0;

				if(form_cond&MANUAL)
				{
					for(i=0;i<num_fields;i++)
					{
						if(tags[i].str!=NULL)
							free(tags[i].str);
						tags[i].str=NULL;
						if(tags[i].des.atr&TABL && tags[i].des.x==db->share->field->des.x)
						{
							bcopy(tags+i+1,tags+i,(num_fields-i-1)*sizeof (struct tag));
							num_fields--;
						}
					}
				}
				else
				{
					for(i=0;i<num_fields;i++)
					{
						if(tags[i].des.x==db->share->field->des.x && tags[i].des.y==db->share->field->des.y)
							break;
					}
					if(i!=num_fields)
					{
						if(tags[i].str!=NULL)
							free(tags[i].str);
						tags[i].str=NULL;

						index=tags[act_field].index;

						bcopy(tags+i+1,tags+i,(num_fields-i-1)*sizeof (struct tag));
						num_fields--;
					}
				}

				qsort(tags,num_fields,sizeof(struct tag),comp_d);

				for(i=0;i<num_fields;i++)
				{
					if(tags[i].des.atr&TABL && panel[num_panel-1].y!=tags[i].des.y)
					{
						if(tags[i].str!=NULL)
							free(tags[i].str);
						tags[i].str=NULL;
						bcopy(tags+i+1,tags+i,(num_fields-i-1)*sizeof (struct tag));
						i--;
						num_fields--;
					}
				}

				bzero(db->share->field,sizeof db->share->field);
				if(bg_orig!=NULL)
				{
					free(background);
					background=new char[bg_size];
					memcpy(background,bg_orig,bg_size);
				}

				Form_Restruct();

				Go_To_Index(index);
				form_update();
				CX_Show();
			}
			break;
		case c_AddField:
			if(db->share!=NULL && db->share->field->des.sla->n)
			{
				struct tag *tag;

				if(form_cond&TABLE)
					break;
				for(tag=db->share->field;tag->des.sla->n;tag++)
				{
					int i;
					for(i=0;i<num_fields;i++)
					{
						if(tags[i].des.x==tag->des.x && tags[i].des.y==tag->des.y)
							break;
						if(tags[i].des.atr&TABL)
						{
							panel=(struct panel *)realloc(panel,(++num_panel)*sizeof (struct panel));
							bzero(panel+num_panel-1,sizeof (struct panel));

							panel[num_panel-1].l=term->l_x();
							panel[num_panel-1].atr=TABL|NO_SHADOW;
							form_cond|=MANUAL;

							panel[num_panel-1].y=tag->des.y;
						}

						if(tag->des.x<x0)
							x0=tag->des.x;
						if(tag->des.y<y0)
							y0=tag->des.y;
						if(tag->des.x+tag->des.l>x0+l)
							l=x0+tag->des.x+tag->des.l+1;
						if(tag->des.y+tag->des.h>y0+h)
							h=y0+tag->des.y+tag->des.h+3;
						h=term->l_y()-y0;

					}
					if(i==num_fields)
					{
						tags=(struct tag *)realloc(tags,(++num_fields)*sizeof (struct tag));
						bzero(tags+num_fields-1,sizeof (struct tag));
						memcpy((char *)&tags[num_fields-1],tag,sizeof (struct tag));
					}
				}

				qsort(tags,num_fields,sizeof(struct tag),comp_d);

				bzero(db->share->field,sizeof db->share->field);

				Form_Restruct();

				Go_To_Index(index);
				form_update();
				CX_Show();
			}
			else
			{
				for(int i=0;i<num_fields;i++)
				{
					if(tags[i].str!=NULL)
						free(tags[i].str);
					tags[i].str=NULL;
				}
				struct x_form form;
				memcpy((char *)&form,(char *)&name_form,sizeof form);

				index=tags[act_field].index;
				record=Record(index);

				Del_Label();
				Free_Form();
				create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
				restore_bg();

				if(form_cond&MANUAL && num_panel>0)
				{
					if(panel[num_panel-1].atr&TABL)
						num_panel--;
				}
				form_cond&=~MANUAL;

				Get_Form(db->Name_Base(),&form);
				Form_Restruct();
				New_Index();
				Go_To_Index(index);
			}
			break;

		case c_ShowForm:
			if(db->share!=NULL)
			{
				char form[256];
				int x=0,y=0,w=term->l_x(),h=term->l_y();
				int lin=20,col=80;


				if(sscanf(db->share->output,"Blank:%d:%d:%d:%d:%d:%d:%s",&x,&y,&w,&h,&col,&lin,form)==7)
				{

				if(Write())
					break;

					term->Put_Screen(1,x,y,w,h,lin,col);

					CX_BROWSER *dial = new CX_BROWSER(this,1,1,form);
					current_cx=dial;

					dial->form_update();
					term->MultiColor(x0,y0,l,h);
					dial->Action();

					term->Del_Screen(1);
					delete dial;

					current_cx=this;
					db->share->ret=0;
					load_menu();
					form_update();
				}
			}
			break;
		case c_Map:
			if(tags[act_field].des.atr&SECURE)
				break;
			protocol("c_Map");
			if(form_cond&MARK)
			{
				record=Record(index=tags[act_field].index);
				CX_BROWSER *table;
				try
				{
					table=new CX_BROWSER(this,record,num_mark_fields,mark_field);
				}
				catch(...)
				{
					break;
				}
				current_cx=table;
				table->db->Cadr_Read(record);
				table->form_update();

				table->Action();

				index=find_page(table->Record(table->index));
				delete table;

				Go_To_Index(index);
				current_cx=this;
				form_cond&=~MARK;
				num_mark_fields=0;
				break;

			}
			if(!(form_cond&MAP) && !(cx_cond&HIST))
			{
				int new_rec;
				struct tag_descriptor td;

				if(Write())
					break;

				db->Cadr_Read(Record(index));
				bcopy(&tags[act_field].des,&td,sizeof td);
				CX_BROWSER *map;
				Restore();
				try
				{
					map=new CX_BROWSER(this);
				}
				catch(...)
				{
					break;
				}
				current_cx=map;

				map->db->Cadr_Read(Record(tags[act_field].index));
				map->form_update();

				long ret=map->Action();

				if(ret=='\r')
					new_rec=map->Act_Record();
				else    new_rec=0;

				num_stack_sel=map->num_stack_sel;
				stack=map->stack;

				delete map;
				current_cx=this;

				if(new_rec)
					index=find_page(new_rec);
				Go_To_Index(index);
				Go_To_Field(td,record=Record(index));
				Table_Normalize();
				form_update();
				CX_Show();
				load_menu();
			}
			break;
		case c_NewRec:
			if(form_atr&NONEW)
				break;
			protocol("c_NewRec");
			if(form_cond&MAP || form_cond&MARK || cx_cond&HIST)
				break;
			if(Write())
				break;
			index=0;
			if(read_only)
			{
				dial(message(15),4);
				break;
			}
			form_cond|=NEW;
			if(form_cond&ARRAY)
			{
				struct tag_descriptor td;

				bcopy(&tags[act_field].des,&td,sizeof td);
				create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
				create_Form(db->Name_Base());
				Form_Restruct();
				if(form_cond&ARRAY)
					Go_To_Field(td,Record(index));
			}
			New_Index();
			form_cond|=EDIT;
			record=0;
			db->Cadr_Read(0);
			if(Cmd_Exe(1)==F10)
				return(F10);
			Get_Form(db->Name_Base(),&name_form);
//                        Table_Normalize();
			Form_Restruct();
			form_update();
			break;
		case c_DelRest:
			if(form_cond&MAP || form_cond&MARK || cx_cond&HIST)
				break;

//                        if(form_cond&ARRAY && db->Field_Descr(tags[act_field].des.sla->n)->m)
			if(db->Field_Descr(tags[act_field].des.sla->n)->m)
			{
				char str3[LINESIZE];

				sprintf(str3,message(58));
				term->BlackWhite(0,0,term->l_x(),term->l_y());
				if(!dial(str3,0))
				{
					term->MultiColor(x0,y0,l,h);
					break;
				}
				term->MultiColor(x0,y0,l,h);
				if(db->Remove_Element(Record(tags[act_field].index),tags[act_field].des.sla)>0)
				{
					form_cond|=EDIT;
					Go_To_Index(index);
					break;
				}
			}
			else
			{
				if(form_atr&NODELETE)
					break;
				protocol("c_DelRest");
				if(db->Check_Del(Record(tags[act_field].index)))
				{
					char str3[LINESIZE];

					strcpy(str3,message(35));
					term->BlackWhite(0,0,term->l_x(),term->l_y());
					if(!dial(str3,0))
					{
						term->MultiColor(x0,y0,l,h);
						break;
					}
					term->MultiColor(x0,y0,l,h);
					db->Restore(Record(index=tags[act_field].index));
					New_Index();
					Go_To_Index(index);
				}
				else
				{
					char str3[LINESIZE];

					strcpy(str3,message(34));
					term->BlackWhite(0,0,term->l_x(),term->l_y());
					if(!dial(str3,0))
					{
						term->MultiColor(x0,y0,l,h);
						break;
					}
					term->MultiColor(x0,y0,l,h);
					del_Record(index=tags[act_field].index);
					if(!(form_cond&SHOWDEL) && !(form_cond&NEW))
					{
						Find_Place();
						New_Index();
						Go_To_Index(index);
					}
				}
			}
			db->Cadr_Read(Record(tags[act_field].index));
			form_update();
			break;
		case c_GoPrev:
			protocol("c_GoPrev");
			if(Write())
				break;
/*
			if(form_cond&TABLE && form_cond&MANUAL)
				break;
*/
			if(form_cond&MAP)
			{
				Go_To_Index(index=tags[0].index);
				break;
			}
			else if(cx_cond&HIST)
			{
				if(hist_record>1)
					hist_record--;
				Cadr_Read();
			}
			else if(form_cond&TABLE)
				index=tags[first_line].index;
			else
				index=Prev_Index(index);
			Go_To_Index(index);
			break;
		case c_GoNext:
			protocol("c_GoNext");
			if(Write())
				break;
/*
			if(form_cond&TABLE && form_cond&MANUAL)
				break;
*/
			if(form_cond&MAP)
			{
				Go_To_Index(index=tags[num_fields-1].index);
				break;
			}
			else if(cx_cond&HIST)
			{
				hist_record++;
				Cadr_Read();
			}
			else if(form_cond&TABLE && !(form_cond&ARRAY))
				index=tags[first_line+num_colon*(num_lines-1)].index;
			else
				index=Next_Index(index);
			Go_To_Index(index);
			break;
		case c_GetProperty:
			try
			{
				char name[LINESIZE];
				char dir[LINESIZE];
				selection sel;
				CX_BROWSER *property;
				struct x_form form;

				strcpy(form.blank,"property");
				form.form=0;
				getcwd(dir,sizeof dir);

				chdir(db->Name_Base());
				property=new CX_BROWSER(PROPERTY,1,&form,1);

				sprintf(name,"#%ld",record);
				property->db->Select(1,name,&sel);
				if(sel.num_index<=0)
				{
					long page=-1;
					if(dial(message(66),1))
						page=property->db->New_Record();
					if(page<=0)
					{
						delete property;
						current_cx=this;
						chdir(dir);
						break;
					}
					property->db->Put_Slot(page,1,name);
					property->db->Unlock(page);
					sel.Add(page);
				}

				sprintf(name,"%s/sel.XXXXXX",TEMPDIR);
#ifndef WIN32
				int fd=mkstemp(name);
#else
				_mktemp(name);
				int fd=creat(name,_S_IREAD | _S_IWRITE);
#endif
				write(fd,sel.index,sel.num_index*sizeof (long));
				close(fd);

				property->set_prim_selection(name);
				property->Read_Index(name);
				property->Set_Parent_Record(record);
				property->Go_To_Index(1);
				property->Action();
				unlink(name);
				delete property;
				current_cx=this;
				chdir(dir);
			}
			catch(...)
			{
			}
			break;
		case c_ShowDel:
			if(form_atr&NODELETE)
			{
				form_cond&=~SHOWDEL;
				break;
			}
			protocol("c_ShowDel");
			index=tags[act_field].index;
			if(form_cond&SHOWDEL)
			{
				form_cond&=~SHOWDEL;
				Find_Place();
			}
			else
				form_cond|=SHOWDEL;
			New_Index();
			Table_Normalize();
			db->Cadr_Read(Record(tags[act_field].index));
			form_update();
			break;
		case c_Calc:
			protocol("c_Calc");
			term->BlackWhite(0,0,term->l_x(),term->l_y());
			Calculator();
			term->BlackWhite(0,0,term->l_x(),term->l_y());
			term->MultiColor(x0,y0,l,h);
			break;
		case c_Choise:
			if(*db->share->output)
			{
				int i;
				char *arg=NULL;
				char *dir=NULL;
				int (*check_name)(char *,char *)=NULL;

				protocol("c_Choise");
				if(get_arg(db->share->output,"-D",dir))
				{
					if(get_arg(db->share->output,"-CX",arg))
						check_name=if_base;
					if(get_arg(db->share->output,"-T",arg))
					{
						if(!strcmp(arg,"base") || !strcmp(arg,"cx"))
							check_name=if_base;
						else    if(!strcmp(arg,"dir"))
							check_name=if_dir;
						else    if(!strcmp(arg,"file"))
							check_name=if_file;
						else    if(!strcmp(arg,"exec"))
							check_name=if_exec;
					}
					*db->share->output=0;
					if(arg!=NULL)
						free(arg);
					char *ch=Select_From_Dir(dir,check_name,"",1);
					if(dir!=NULL)
						free(dir);
					if(ch!=NULL)
					{
						if(*ch)
							Check_Line(ch);
						free(ch);
					}
					Go_To_Index(index);
					form_update();
					break;
				}
				if(get_arg(db->share->output,"-F",arg) || get_arg(db->share->output,"-B",arg))
				{
					CX_BROWSER *browser;
					struct x_form x;
					long rec;

					x.form=0;
					strcpy(x.blank,arg);
					if(get_arg(db->share->output,"-R",arg))
						rec=atoi(arg);
					else    rec=1;
					if(get_arg(db->share->output,"-N",dir))
					{
						try
						{
							get_arg(db->share->output,"-I",arg);
							browser = new CX_BROWSER(dir,rec,&x,level+1,arg);
						}
						catch(...)
						{
							if(arg)
								free(arg);
							if(dir)
								free(dir);
							break;
						}
					}
					else
					{
						try
						{
							if(get_arg(db->share->output,"-I",arg))
								strcpy(db->share->io,arg);
							browser = new CX_BROWSER(this,rec,&x,level+1);
						}
						catch(...)
						{
							if(arg)
								free(arg);
							if(dir)
								free(dir);
							break;
						}
					}
					if(get_arg(db->share->output,"-S",arg))
					{
						browser->set_prim_selection(arg);
						browser->Read_Index(arg);
						browser->Refresh(0);
					}

					if(get_arg(db->share->output,"-A",arg))
					{
						struct tag_descriptor td;

						str_to_sla(arg,td.sla);
						browser->Go_To_Field(td,browser->find_page(rec));
					}
					if(get_arg(db->share->output,"-M",arg))
					{
						*db->share->output=0;
						strcpy(browser->db->share->output,arg);
					}
					else *db->share->output=0;

					if(arg)
						free(arg);
					if(dir)
						free(dir);

					browser->Read_Only(2);
//                                        browser->Go_To_Index(browser->find_page(rec));

					ret=browser->Action();
					rec=browser->Act_Record();

					delete browser;
					current_cx=this;

					if(ret!='\r')
						rec=0;

					char str[LINESIZE];
					sprintf(str,"#%ld",rec);

					Check_Line(str);

					Go_To_Index(index);
					ret=0;
					break;
				}
				if(arg)
					free(arg);
				term->cursor_visible();
				i=Edit(F7,tags+act_field);
				term->cursor_invisible();
				if(form_cond&ARRAY && tags[act_field].des.sla->m)
				{
					struct tag_descriptor td;

					bcopy(&tags[act_field].des,&td,sizeof td);
					create_geom(x0,y0,term->l_x()-level+1,term->l_y()-level);
					create_Form(db->Name_Base());
					Form_Restruct();
					New_Index();
					if(form_cond&ARRAY)
						Go_To_Field(td,Record(index));
				}
				form_update();
				CX_Show();
				Go_To_Index(index);
//                                if(i==F10)
//                                        return(F10);
				break;
			}
			return(0);
		case c_RecForm:
			if(*db->share->output)
			{
				long rec=record;
				int atr=0;
				char selection_name[NAMESIZE];
				char io[LINESIZE];
				char query[LINESIZE];
				char output[sizeof db->share->output];
				char *arg=NULL;
				struct tag newtag;

				*query=0;
				bcopy(tags+act_field,&newtag,sizeof newtag);
				if(db->share->slot.sla->n)
					bcopy(db->share->slot.sla,newtag.des.sla,sizeof newtag.des.sla);
				else
					bcopy(tags[act_field].des.sla,newtag.des.sla,sizeof newtag.des.sla);

				bzero(&db->share->slot,sizeof (db->share->slot));

				protocol("->Form");
				*selection_name=0;
				*io=0;
				bcopy(db->share->output,output,sizeof db->share->output);
				bzero(db->share->output,sizeof db->share->output);

				if(get_arg(output,"-L",arg))
				      setenv("CXUSER",arg,1);
				if(get_arg(output,"-A",arg))
					str_to_sla(arg,newtag.des.sla);

				if(Write())
					break;

				struct x_form x;
				x.form=0;
				char name[NAMESIZE];
				if(get_arg(output,"-N",arg))
				{
					strcpy(name,arg);
					if(get_arg(output,"-R",arg))
						rec=atoi(arg);
					if(get_arg(output,"-Q",arg))
						strcpy(query,arg);
					else if(get_arg(output,"-S",arg))
						strcpy(selection_name,arg);
					if(get_arg(output,"-I",arg))
						strcpy(io,arg);
					atr=strcmp(name,db->Name_Base())!=0; //fill NULL change to 0
				}
				if(get_arg(output,"-F",arg) || get_arg(output,"-B",arg))
					strcpy(x.blank,arg);
				else if(!atr)
				{
					strcpy(name,db->Name_Base());
					strcpy(x.blank,output);
				}
				if(arg!=NULL)
					free(arg);
				try
				{
					CX_BROWSER *browser;

					int insert_std=0;

					bzero(db->share->output,sizeof (db->share->output));
					bzero(&db->share->slot, sizeof (db->share->slot));
					bzero(&db->share->color,sizeof (db->share->color));
					Restore();
					if(!atr)
					{
						insert_std=db->insert;
						browser=new CX_BROWSER(this,rec,&x,level+1);
						rec=Record(index);

					}
					else
					{
						try
						{
							browser=new CX_BROWSER(name,rec,&x,level+1,io);
						}
						catch(...)
						{
							break;
						}
					}
					current_cx=browser;
					if(*query)
					{
						selection sel;
						browser->db->Select(newtag.des.sla,query,&sel);
						if(sel.num_index>0)
						{
							sprintf(selection_name,"%s/sel.XXXXXX",TEMPDIR);
#ifndef WIN32
							int fd=mkstemp(selection_name);
#else
							_mktemp(selection_name);
							int fd=creat(selection_name,_S_IREAD | _S_IWRITE);
#endif
							write(fd,sel.index,sel.num_index*sizeof (long));
							close(fd);
						}
						else    *query=0;
					}
					if(*selection_name)
					{
						browser->set_prim_selection(selection_name);
						browser->Read_Index(selection_name);
						browser->Refresh(0);
					}
					browser->Go_To_Field(newtag.des,rec);

					browser->Action();
					if(*query && *selection_name)
						unlink(selection_name);
					protocol("<-Form");
					if(form_atr&NOMENU)
						browser->delete_menu();
					if(*selection_name)
						browser->del_prim_selection();
					while(browser->db->insert>insert_std)
					{
						int ins=browser->db->insert;
						browser->Rest_Index();
						if(ins==browser->db->insert)
							break;
					}
					if(!atr)
					{

						stack = browser->stack;
						browser->stack=NULL;
						prot_fd  = browser->prot_fd;
						browser->prot_fd=0;
						keep_str = browser->keep_str;
						browser->keep_str=NULL;

						read_only= browser->read_only;
						browser->read_only=0;
						ed = browser->ed;
						browser->ed=NULL;
						cx_cond  = browser->cx_cond;
						record   = browser->record;

						browser->db = NULL;

						index=find_page(record);
						db->Cadr_Read(Record(index));
						Go_To_Index(index);
					}else{
					db->Cadr_Read(Record(tags[act_field].index));
					}
					delete browser;
					current_cx=this;

				}
				catch(...)
				{
					;
				}
				index=find_page(record);
				db->Cadr_Read(Record(index));
				Go_To_Index(index);
//                                form_update();
				CX_Show();
				load_menu();
				break;
			}
			cmd=0;
		case c_Update:
			form_update();
			break;
		case  c_GetLimitSel:
			if(db->share!=NULL && *db->share->output && !access(db->share->output,R_OK))
			{
				if(Write())
					break;
				record=Record(index);
				personal_selection=(char *)realloc(personal_selection,strlen(db->share->output)+1);
				strcpy(personal_selection,db->share->output);
				Read_Index(db->share->output);
				*db->share->output=0;
				index=find_page(record);
				db->Cadr_Read(Record(index));
				Go_To_Index(index);
			}
			break;
		case c_DelLimitSel:
			if(personal_selection!=NULL)
			{
				free(personal_selection);
				personal_selection=NULL;
			}
			break;
		case c_GetSel:
			if(db->share!=NULL && *db->share->output)
			{
				if(Write())
					break;
				record=Record(index);
				db->Link_Index(db->share->output,level);
				index=find_page(record);
				db->Cadr_Read(Record(index));
				Go_To_Index(index);
			}
			break;
		case c_Refresh:
		{
			Refresh(1);
			break;
		}
		case c_Recurs:
			if(!(tags[act_field].des.atr&NO_RECURS) && db->is_pointer(tags[act_field].des.sla))
			{
				char *ch=NULL;
				struct field *f;
				char *name;
				CX_BROWSER *cx=NULL;

				struct sla sla[SLA_DEEP];
				protocol("c_Recurs");
				bzero(sla,sizeof sla);
				bcopy(tags[act_field].des.sla,sla,sizeof tags[act_field].des.sla);

				while(((f=db->Field_Descr(sla))->a!=X_POINTER) && (f->a!=X_VARIANT))
				{
					for(int i=SLA_DEEP-1;i;i--)
					{
						if(sla[i].n)
						{
							sla[i].n=0;
							break;
						}
					}
				};
				struct get_field_result res;
				res=db->Read(record=Record(index=tags[act_field].index),sla,ch,1);
				if(res.len>0)
				{
					long page=0,spag=0;
					int field=act_field;

					if(ch==NULL)
						break;
					if(res.field.a==X_POINTER)
					{
						bcopy(ch,&page,res.field.l);
#ifdef SPARC
						conv((char *)&page,sizeof page);
#endif
						free(ch);
					}
					else
					{
						if(tags[act_field].des.sla[1].n>2)
							break;
						bcopy(ch,&spag,res.field.n);
						bcopy(ch+res.field.n,&page,res.field.l-res.field.n);
#ifdef SPARC
						conv((char *)&spag,sizeof page);
						conv((char *)&page,sizeof page);
#endif
						free(ch);
						if(tags[act_field].des.sla[1].n<2)
						{
							page=spag;
							spag=1;
						}
						if(spag<=0)
							break;
					}
					if(page>0 || (res.field.a==X_VARIANT && tags[act_field].des.sla[1].n<2))
					{
						struct tag tg;
						int i=0;

						tg=tags[act_field];
						bzero(tg.des.sla,sizeof (tg.des.sla));
						if(res.field.a==X_VARIANT && tags[act_field].des.sla[1].n==2)
							bcopy(tags[act_field].des.sla+2,tg.des.sla,3*sizeof (struct sla));
						else
							bcopy(tags[act_field].des.sla+1,tg.des.sla,4*sizeof (struct sla));
						if(tg.des.sla->n==0)
							tg.des.sla->n=1;

						if(Write())
							break;
						for(i=0;i<num_open_cx;i++)
						{
							if(!strcmp(open_cx[i].name,db->Name_Base()))
							{
								bcopy(&name_form,&open_cx[i].name_form,sizeof (struct x_form));
								break;
							}
						}
						if(res.field.a==X_VARIANT && tags[act_field].des.sla[1].n==2)
							name=db->Name_Subbase(sla,record);
//                                                        name=db->Name_Subbase(sla,spag);
						else
							name=db->Name_Subbase(sla);
						if(name==NULL || ((ch=strchr(name,':'))==NULL && !if_base("",name)))
							break;
						for(i=0;i<num_open_cx;i++)
						{
							if(!strcmp(open_cx[i].name,name))
							{
								break;
							}
						}
						if(ch!=NULL)
						{
							if(i==num_open_cx)
							{
								open_cx=(struct Open_CX *)realloc(open_cx,++num_open_cx*sizeof (struct Open_CX));
								open_cx[i].name=(char *)malloc(strlen(name)+1);
								open_cx[i].name_form.form=-1;
								*open_cx[i].name_form.blank=0;
								*open_cx[i].passwd=0;
								strcpy(open_cx[i].name,name);
							}
							if(!*open_cx[i].passwd)
							{
GOT_PSWD:
								int ret;
								int f=term->get_box(19,term->l_y()/3,30,4);
								term->BlackWhite(0,0,term->l_x(),term->l_y());
								term->MultiColor(19,term->l_y()/3,28,3);
								term->BOX(19,term->l_y()/3,28,3,' ',6,0xe,6,0xe);
								term->dpp(20,term->l_y()/3+1);
								term->dps("Password:");
								*open_cx[i].passwd=0;
								term->cursor_visible();
								ret=EditLine->edit(0,open_cx[i].passwd,16,16,30,term->l_y()/3+1,-1);
								term->cursor_invisible();
								term->restore_box(f);
								term->free_box(f);
								if(ret!='\r')
									goto NO_REMOTE;
							}
#ifndef WIN32
							ch=Remote_Browser(open_cx[i].name,page,open_cx[i].passwd);
							if(!strcmp(ch,message(36)))
								goto GOT_PSWD;
#endif
NO_REMOTE:
							db->Cadr_Read(Record(tags[act_field].index));
							term->BlackWhite(0,0,term->l_x(),term->l_y());
							term->MultiColor(x0,y0,l,h);
							form_update();
							CX_Show();
							load_menu();
							break;
						}
						Restore();
						if(i==num_open_cx)
						{
							open_cx=(struct Open_CX *)realloc(open_cx,++num_open_cx*sizeof (struct Open_CX));
							open_cx[i].name=(char *)malloc(strlen(name)+1);
							strcpy(open_cx[i].name,name);
							open_cx[i].name_form.form=-1;
							*open_cx[i].name_form.blank=0;
						}
						try
						{
							cx=new CX_BROWSER(open_cx[i].name,page,&open_cx[i].name_form,level+1);
						}
						catch(...)
						{
							free(open_cx[i].name);
							num_open_cx=i;
							break;
						}
						current_cx=cx;

						if((page=cx->find_page(page))<=0)
							page=1;
						cx->Go_To_Index(page);
						cx->Go_To_Field(tg.des,page);

						bcopy(&cx->name_form,&open_cx[i].name_form,sizeof (struct x_form));
						cx->Action();
						bcopy(&cx->name_form,&open_cx[i].name_form,sizeof (struct x_form));
						delete cx;
						current_cx=this;
						act_field=field;
					}
					index=find_page(record);
					db->Cadr_Read(Record(index));
					Go_To_Index(index);

//                                        db->Cadr_Read(Record(index=tags[act_field].index));
//                                        form_update();

					CX_Show();
					load_menu();
					break;
				}
				if(ch!=NULL)
					free(ch);
			}
		default:
			return(0);
	}
	Event(-cmd);
	return(cmd);
}

int CX_BROWSER::Check_Line(char *str)
{
	return(Check_Line(tags+act_field,str));
}

int CX_BROWSER::Check_Line(struct tag *tag,char *str)
{
#ifndef WIN32
	if(cx3!=NULL)
		return(cx3->Check_Line(Record(index),tag,str));
#endif
	if(db->share==NULL || db->p_method<0)
		return(0);
	strcpy(db->share->output,str);
	db->share->cmd=CHK_LINE;
	int ret=Check(tag);
	if(ret>=100)
	{
		if(ret & EXECONT)
		{
			ret&=~EXECONT;
			Cmd_Exe(ret);
			ret=0;
		}
		else
		{
			Cmd_Exe(ret);
			ret=1;
		}
		*db->share->output=0;
	}
	return(ret);
}

#ifndef WIN32
int wait_mess_id=-1;
static int chart_color[]={0xe,0xc,0xa,0xb,0xf,0x9,35,0x3,0x2,0x1};
void wait_mess(int sig)
{
	static int n;
	static int i;
	if(wait_mess_id<0)
	{
		int x=(term->l_x()-50)/2;
		int y=(term->l_y()/2);
		wait_mess_id=term->get_box(x-2,y-2,50+6,5);
		term->BOX(x-1,y-1,52,3,' ',41,41,41,41),
		term->Set_Color(chart_color[0],0);
		term->dpp(x,y);
		term->scrbufout();
		i=0;
		n=0;
	}
	if(n<50)
		n++;
	else
	{
		n=0;
		i++;
		term->dpp((term->l_x()-50)/2,(term->l_y()/2));
		term->Set_Color(chart_color[i%10],0);
	}
	term->dpo(' ');
	term->scrbufout();
}
#endif

int CX_BROWSER::Check_Cadr()
{
#ifndef WIN32
	if(cx3!=NULL)
		return(cx3->Check_Cadr(Record(index),tags+act_field,1));
#endif
	if(db->share==NULL || db->p_method<0)
		return(0);
	db->share->cmd=CHK_CADR;
	int ret=Check();
	if(ret>=100)
	{
		if(ret & EXECONT)
		{
			ret&=~EXECONT;
			Cmd_Exe(ret);
			ret=0;
		}
		else
		{
			Cmd_Exe(ret);
			ret=1;
		}
		*db->share->output=0;
	}
	return(ret);
}

extern void refresh(int i);

int CX_BROWSER::Event(int act)
{
	if(db->share==NULL || db->p_method<0)
		return 0;
#ifndef WIN32
	if(cx3!=NULL)
	{
		if(act==c_DelRest)
			return(cx3->Check_Cadr(Record(index),tags+act_field,0));
		if(act=='\r' && tags[act_field].des.sla->n>db->Num_Fields())
			return(cx3->Check_Line(Record(index),tags+act_field,""));
	}
#endif

	db->share->cmd=CHK_ACT;
	bcopy(&act,db->share->output,sizeof act);

#ifndef WIN32
	signal(SIGUSR2,wait_mess);
#endif
	int i=Check();

	if(act==*(int *)db->share->output)
	{
		*db->share->output=0;
	}

#ifndef WIN32

	if(wait_mess_id>=0)
	{
		term->restore_box(wait_mess_id);
		term->free_box(wait_mess_id);
		wait_mess_id=-1;
	}

	signal(SIGUSR2,refresh);
#endif
	return(i);
}

void CX_BROWSER::Ind_Update()
{
	if(db->share==NULL || db->p_method<0)
		return;
	db->Flush_Index_Buf(level);
#ifndef WIN32
	db->share->cmd=NEWINDEX;
	db->share->ret=level;
	strncpy(db->share->output,db->idx_name,(sizeof db->idx_name)-1);
	db->Get_Check();
	bzero(db->share->output,sizeof db->share->output);
#endif
}

int CX_BROWSER::Check()
{
	return(Check(tags+act_field));
}

int CX_BROWSER::Check(struct tag *tag)
{
	struct sla sla_std[SLA_DEEP];
	char *cadr_std=NULL;

	if(db->share==NULL || db->p_method<0)
	{
		bzero(db->share->output,sizeof db->share->output);
		return(0);
	}

	if(tags!=NULL)
		bcopy(&tag->des,&db->share->slot,sizeof (db->share->slot));

	db->Flush_Index_Buf(level);
	db->share->record=record;
	db->share->index=index;
	db->share->color.bg=db->share->color.fg=0;
	bcopy(db->share->slot.sla,sla_std,sizeof sla_std);
	db->share->font.fnt=-1;

	cadr_std=(char *)malloc(db->len_cadr);
	if(db->cadr!=NULL)
		bcopy(db->cadr,cadr_std,db->len_cadr);


	if(hyperform)
	{
		char *name1=NULL,*name2=NULL;
		int dif=0;
		db->Get_Slot(record,hyperform,name1);

		db->Get_Check();
		if(db->share->cmd==c_Dial)
		{
			int secure=-(db->share->ret<0);
			if(secure)
				db->share->ret=-db->share->ret;
			int i=dial(db->share->output,db->share->ret,NULL,-1,NULL,secure);
			db->share->cmd=i;
			db->Get_Check();
			db->share->cmd=0;
		}

		db->Get_Slot(record,hyperform,name2);
		if(name1!=NULL && name2!=NULL)
			dif=strcmp(name1,name2);
		else if((name1==NULL && name1!=NULL) || (name2==NULL && name1!=NULL))
			dif=1;
		if(name1!=NULL)
			free(name1);
		if(name2!=NULL)
			free(name2);
		if(dif)
			Form_Refresh(&tag->des);
	}
	else
	{
		db->Get_Check();
		if(db->share->cmd==c_Dial)
		{
			int secure=-(db->share->ret<0);
			if(secure)
				db->share->ret=-db->share->ret;
			if(strncmp(db->share->output,"Blank:",6)==0)
			{
				int x=0,y=0;
				char form[256];
				sscanf(db->share->output,"Blank:%d:%d:%s",&x,&y,form);
				Write();
				CX_BROWSER *dial = new CX_BROWSER(this,x,y,form);
				current_cx=dial;

				dial->form_update();

				term->MultiColor(x0,y0,l,h);

				dial->Action();

				delete dial;
				current_cx=this;
				db->share->ret=0;
				load_menu();
				form_update();
			}
			else
			{
				int i=dial(db->share->output,db->share->ret,NULL,-1,NULL,secure);
				db->share->cmd=i;
				db->Get_Check();
			}
		}
	}

	if(tags!=NULL)
	{
		if(slacmp(sla_std,db->share->slot.sla))
			Go_To_Field(db->share->slot,record);
	}
	if((db->cadr!=NULL && cadr_std!=NULL && memcmp(db->cadr,cadr_std,db->len_cadr)) || db->share->edit_flag)
	{
		form_cond|=EDIT;
		form_update();
	}

	if(cadr_std!=NULL)
		free(cadr_std);
	return(db->share->ret);
}

extern int x_c;
void CX_BROWSER::Modify()
{
	int length,i,flag=0,date=0;
	char str1[LINESIZE];
	char str2[LINESIZE];
	long page;
	char *ch=NULL;
	double r;
	struct sla sla[SLA_DEEP];
	if (getuid()!=0)
	 return ;

	if(Write())
		return;
	db->Cadr_Read(0);
	bcopy(tags[act_field].des.sla,sla,sizeof sla);
	if((length=tags[act_field].des.l+10)<30)
		length=30;
	if(length>term->l_x()-10)
		length=term->l_x()-10;
	int f=term->get_box(3,term->l_y()-7,length+3,7);
	term->MultiColor(3,term->l_y()-7,length,5);
	term->BOX(3,term->l_y()-7,length,5,' ',6,0xe,6,0xe);
	term->dpp(3+(length-12)/2,term->l_y()-6); term->dps("Modification");
	i=db->Field_Descr(sla)->a;
	if(db->Field_Descr(sla[0].n)->a==X_POINTER)
	{
		if((page=sla[1].n?Select_From_DB(db->Name_Subbase(sla),sla+1,tags[act_field].des.l):Select_From_DB(db->Name_Subbase(sla),1,tags[act_field].des.l))>=0)
		{
			sla[1].n=0;
			i=F5;
			goto CC;
		}
		goto END;
	}
	if(i==X_DATE)
		date=1;
	if(i==X_TIME)
		date=2;
	if(i==X_DATE || i==X_TIME || i==X_INTEGER || i==X_FLOAT || i==X_DOUBLE)
		flag=1;
	*str1=0;
	*str2=0;
AA:
	i=EditLine->edit(0,str1,LINESIZE-10,length-5,6,term->l_y()-5,0);

	if(i=='\r')
	{
		switch(*str1)
		{
			case '+':
				if(flag)
					i=F1;
				break;
			case '-':
				if(flag)
					i=F2;
				break;
			case '*':
				if(flag)
					i=F3;
				break;
			case '/':
				if(flag)
					i=F4;
				break;
			case '=':
				if(flag)
					i=0;
				break;
			default:
				i='\r';
				goto CC;
		}
		bcopy(str1+1,str1,sizeof(str1));
	}
CC:
	switch(i)
	{
		case F10:
			goto END;
		case F5:
			flag=7;
			break;
		case '\r':
			flag=0;
			break;
		case PU:
		case PD:
			db->Get_Slot(Record(index),sla,ch);
			strcat(str1,ch);
			goto AA;
		case F1:
			if(flag)
				flag=3;
			else    flag=1;
			break;
		case F2:
			if(flag)
				flag=4;
			else    flag=2;
			break;
		case F3:
			if(!flag || date)
			{
//                                dpo(bl);
				goto AA;
			}
			flag=5;
			break;
		case F4:
			if(!flag || date)
			{
//                                dpo(bl);
				goto AA;
			}
			flag=6;
			break;
		case F8:
			if(!*str1)
			{
				flag=-1;
				break;
			}
			else *str1=0;
		default:
			goto AA;
	}
	strcpy(str2,str1);
	if(!strcmp(str1,"delete"))
		flag=-1;
        for(i=1;i<=db->max_index;i++)
	{
		switch(flag)
		{
			case 0:
			case 7:
			case -1:
				break;
			case 1:
				db->Get_Slot(Record(i),sla,ch);
				strcpy(str2,ch);        // slot+str
				strcat(str2,str1);
				break;
			case 2:
			{
				db->Get_Slot(Record(i),sla,ch);
				strcpy(str2,str1);      // str+slot
				strcat(str2,ch);
				break;
			}
			case 3:         // value(slot)+value(str);
			case 4:         // value(slot)-value(str)

				db->Get_Slot(Record(i),sla,ch);
				strcpy(str2,ch);
				if(!date)
				{
					if(flag==3)
						r=db->atof(str2)+db->atof(str1);
					else
						r=db->atof(str2)-db->atof(str1);
					sprintf(str2,"%f",r);
				}
				else
				{
					if(date==1)
						r=conv_date(str2);
					else
						r=conv_time(str2);
					if(flag==3)
						r+=atoi(str1);
					else
						r-=atoi(str1);
					if(date==1)
						get_date((long)r,str2);
					else
						get_time((long)r,str2);
				}
				break;
			case 5:         // value(slot)*value(str)
			case 6:         // value(slot)/value(str)

				db->Get_Slot(Record(i),sla,ch);
				strcpy(str2,ch);
				if(flag==5)
					r=db->atof(str2)*db->atof(str1);
				else
				{
					double a=db->atof(str1);

					if(a!=0)
						r=db->atof(str2)/db->atof(str1);
					else    r=db->atof(str2);
				}
				sprintf(str2,"%f",r);
				break;
		}
		if(i==1)
		{
			char str3[LINESIZE];

			if(flag==-1)
				sprintf(str3,message(38),db->max_index);
			else
				sprintf(str3,message(37),db->max_index);
			term->BlackWhite(0,0,term->l_x(),term->l_y());
			if(!dial(str3,0))
			{
				term->MultiColor(x0,y0,l,h);
				goto END;
			}
			term->MultiColor(x0,y0,l,h);
		}
		if(flag==7)
		{
			db->Write(Record(i),sla->n,(char *)&page);
		}
		else if(flag==-1)
		{
			db->Delete(Record(i));
		}
		else
		{
			db->Put_Slot(Record(i),sla,str2);
		}
	}
END:
	if(ch!=NULL)
		free(ch);
	term->restore_box(f);
	term->free_box(f);
	db->Cadr_Read(Record(tags[act_field].index));
	form_update();
}

long CX_BROWSER::Select_From_DB(char *name_base,int field,int len,char *selection)
{
	struct sla sla[SLA_DEEP];
	bzero(sla,sizeof sla);
	sla[0].n=field;
	return(Select_From_DB(name_base,sla,len,selection));
}
long CX_BROWSER::Select_From_DB(char *name_base,struct sla *SLA,int len,char *selection)
{
	int ret=0;
	struct sla sla[SLA_DEEP];

	bcopy(SLA,sla,sizeof sla);
	if(!sla->n)
		sla->n=1;
#ifndef WIN32
	if(strchr(name_base,':')!=NULL)
	{
		char *ch;

		char *buf=Remote_Map(name_base,1,sla,len);
		if(buf==NULL || (ch=strstr(buf,"Return="))==NULL)
			return(-1);
		if(atoi(ch+7)==F8)
			return(0);
		if(atoi(ch+7)!='\r' || (ch=strstr(ch,"Record="))==NULL)
			return(-1);
		return(atoi(ch+7));
	}
#endif
	CX_BROWSER *subbase=NULL;

	try
	{
		subbase = new CX_BROWSER(name_base,1,sla,len);
	}
	catch(...)
	{
		return(-1);
	}
	if(subbase->db->last_cadr()>0)
	{
		char *sel=NULL;
		if(get_arg(db->share->output,"-S",sel) && sel!=NULL)
		{
			subbase->set_prim_selection(sel);
			subbase->Read_Index(sel);
			free(sel);
		}
		if(selection!=NULL && *selection)
		{
			subbase->set_prim_selection(selection);
			subbase->Read_Index(selection);
			subbase->Go_To_Index(1);
		}
		subbase->Form_Restruct();
		switch(subbase->Action())
		{
			case '\r':
				ret=subbase->Record(subbase->tags[subbase->Act_Field()].index);
				delete subbase;
				current_cx=this;
				return(ret);
			case F8:
				delete subbase;
				current_cx=this;
				return(0);
			default:
				delete subbase;
				current_cx=this;
				return(-1);
		}
	}
	delete subbase;
	current_cx=this;
	return(-1);
}

int CX_BROWSER::find_slot(struct sla *sla)
{
	for(int i=0;i<num_fields;i++)
	{
		if(!slacmp(tags[i].des.sla,sla))
			return(i);
	}
	return(-1);
}

static char *keys[]=
{
	"F1",  "F2",  "F3",  "F4",  "F5",  "F6",  "F7",  "F8",
	"F9",  "F10", "F11", "F12", "CL",  "CR",  "CU",  "CD",
	"PU",  "PD",  "HM",  "EN",  "IN",  "DE"
};

void CX_BROWSER::protocol(int i)
{
	char str[64];

	if(!(cx_cond&PROT) || prot_fd<0)
		return;
	if(i>300 && i<301+sizeof(keys)/sizeof (char *))
		sprintf(str,"%s",keys[i-301]);
	else
		sprintf(str,"%4d",i);
	protocol(str);
}

void CX_BROWSER::protocol(char *cmd)
{
	char str[64];
	if(!(cx_cond&PROT) || prot_fd<0)
		return;
	if(!prot_fd)
	{
		char name[64];
		char *ch;
		if((ch=getenv("SSH_CLIENT"))!=NULL || (ch=getenv("TELNET_CLIENT"))!=NULL)
		{
			strcpy(str,ch);
			char  *ch=strchr(str,' ');
			if(ch!=NULL)
				*ch=0;
		}
		else    *str=0;
		sprintf(name,"%s/%s/log.[%s].%s",db->Name_Base(),TMPDIR,str,GetLogin());
		if((prot_fd=open(name,O_RDWR|O_CREAT|O_BINARY,0600))<=0)
			return;
	}
	time_t t=time(0);
	char str1[64];
	strcpy(str1,ctime(&t)+4);
	str1[strlen(str1)-6]=0;
	sprintf(str,"%s %s\n",str1,cmd);
	lseek(prot_fd,0,SEEK_END);
	write(prot_fd,str,strlen(str));
}

void Help(int i,int level)
{
	CX_BROWSER *browser;
	struct x_form x;
	long rec;

	x.form=1;
	*x.blank=0;
	try
	{
		browser = new CX_BROWSER(HELPDEF,i,&x,level);
	}
	catch(...)
	{
		return;
	}
	browser->Read_Only(1);
	browser->Action();
	delete browser;
}
