/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:structure.cpp
*/
#include "StdAfx.h" 
#include "../CX_Browser.h" 
extern Terminal *term; 
 
extern struct item field_type[]; 
extern int size_field_types; 
static int get_pos(int x) 
{ 
	if(x<5) 
		return(-1); 
	return(x>30?4:x>24?3:x>18?2:x>12?1:x>5?0:-1); 
} 
#define S1 8
#define S2 18
#define S3 23
#define S4 26
#define S5 32
#define S6 38

extern struct item ATR[];
int Show_Structure(struct st *struc,int rec,char *name,int field,struct sla *sla) 
{ 
	int f,x0=7,y0=2; 
	int len,i,j; 
	int act=0,shift=0,pos=0; 
	int y,kod=0; 
	int nn,num=20; 
 
	x0+=rec; 
	y0+=rec; 
	len=term->l_x()-x0-4; 
//	len-=rec; 
	num-=rec; 
	f=term->get_box(0,0,term->l_x(),term->l_y()); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
//        term->Set_Font(0,0); 
	if(rec==0) 
	{ 
		Load_Menu(ICONSDEF,2); 
		show_menu(); 
	} 
BEGIN: 
	if(struc->ptm<num) 
		nn=struc->ptm; 
	else    nn=num; 
	term->MultiColor(x0,y0,len+2,nn+2); 
	term->BOX(x0,y0,len+2,nn+2,' ',41,41,41,41);
	term->Set_Font(1,0);
	term->Set_Color(41,0);
	term->dpp(x0+S1+1,y0); term->dps("Type");
	term->dpp(x0+S2+1,y0); term->dps("Len");
	term->dpp(x0+S3+1,y0); term->dps("K");
	term->dpp(x0+S4+1,y0); term->dps("Mod");
	term->dpp(x0+S5+1,y0); term->dps("Dir");
	term->dpp(x0+S6+1,y0); term->dps("Name");
	if(name!=NULL && *name)
	{
		term->dpp(x0+S6+1+(len-39-strlen(name))/2,y0);
		term->dps(name);
	}

	if(field) 
	{ 
		char str[16]; 
		sprintf(str,"%d",field); 
		term->dpp(x0+len,y0); 
		term->dps(str); 
	} 
	for(;;) 
	{ 
		for(i=0;i<num && i<struc->ptm;i++) 
		{ 
			char str[256];

			term->Set_Font(0,2);
			term->dpp(x0+1,y0+1+i);
			term->Set_Color(41,0);
			sprintf(str,"%4d",shift+i+1);
			term->dps(str);
 
			term->dpp(x0+S1-2,y0+1+i);
			if(i==act && pos==0)
			{
				term->Set_Font(0,0);
				term->Set_Color(0x10f,0xe);
				y=y0+1+i;
			}
			else term->Set_Color(0x200+ATR[struc->field[i+shift].a-1].bg,ATR[struc->field[i+shift].a-1].fg);
			term->dps(ATR[struc->field[i+shift].a-1].name);
			term->dpp(x0+S2,y0+1+i);
			term->Set_Font(0,2);
			term->Set_Color(0x200+41,0);
			if(i==act && pos==1)
			{
				term->Set_Color(0x10f,0x0);
				y=y0+1+i;
			}
			else term->Set_Color(0x200+41,0);
			if(struc->field[i+shift].a==X_STRUCTURE && struc->field[i+shift].st.st!=NULL)
				struc->field[i+shift].l=len_struct(struc->field[i+shift].st.st);
 
			if(struc->field[i+shift].n==0)
				sprintf(str,"%5d",struc->field[i+shift].l);
			else
				sprintf(str,"%3d.%1d",struc->field[i+shift].l,struc->field[i+shift].n);
			term->dps(str);
 
			if(i==act && pos==2)
			{
				term->Set_Color(0x10f,0x0);
				y=y0+1+i;
			}
			else term->Set_Color(0x200+41,0);
			term->dpp(x0+S3+1,y0+1+i);
 
			if(struc->field[i+shift].k)
				term->dps("T");
			else if(struc->field[i+shift].b)
				term->dps("B");
			else    term->dps(" ");
 
			term->dpp(x0+S4,y0+1+i);
			if(i==act && pos==3)
			{
				term->Set_Color(0x10f,0x0);
				y=y0+1+i;
			}
			else term->Set_Color(0x200+41,0);
			switch(struc->field[i+shift].m)
			{
				case MULTISET:
					term->dps("ARRAY");
					break;
				case SET:
					term->dps(" SET ");
					break;
				case LIST:
					term->dps("LIST ");
					break;
				default:
					term->dps("     ");
			}

			term->dpp(x0+S5,y0+1+i);
			if(i==act && pos==4)
			{
				term->Set_Color(0x10f,0x0);
				y=y0+1+i;
			}
			else term->Set_Color(0x200+41,0);
			if(struc->field[i+shift].d)
				term->dps(" --> ");
			else    term->dps("     ");
 
			term->dpp(x0+S6,y0+1+i);
			if(i==act && pos==5)
			{
				term->Set_Color(0x10f,0x0);
				y=y0+1+i;
			}
			else term->Set_Color(0x200+41,0);
			term->dpn(len-S6-1,' ');
			term->dpp(x0+S6,y0+1+i);
			if(struc->field[i+shift].name!=NULL)
			{
				int size=len-37;
				if(size>sizeof str)
					size=(sizeof str)-1;
				strncpy(str,struc->field[i+shift].name,size);
				str[size]=0;
				term->dps(str);
			}
 
			term->Set_Color(41,0);
			sprintf(str,"%ld",struc->ptm);
			term->dpp(x0+len-strlen(str),y0+nn+1);
			term->dps(str);
			term->dpp(x0+5,y0+1+act);
		} 
		if(kod==IS) 
		{ 
			act=i-1; 
			while(act+shift<struc->ptm-1) 
				shift++; 
			kod=0; 
			continue; 
		} 
		switch(kod=Xmouse(term->dpi())) 
		{ 
 
			case 0: 
				if((j=get_menu_cmd(term->ev().x,term->ev().y,term->ev().b))==c_Exit) 
				{ 
					term->restore_box(f); 
					term->free_box(f); 
					kod=F10; 
					goto EXIT; 
				} 
				else if(j==c_HTML || j==c_Show) 
				{ 
					char *buf; 
					char *ch="<HTML><META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=KOI8-R\"><BODY><PRE>\n"; 
					char str[256]; 
 
					sprintf(str,"CLASS \"%s\"\n",pc_demos(name));
					buf=(char *)malloc(strlen(str)+1); 
					strcpy(buf,str); 
 
					Schema_to_Text(struc,buf,0,j==c_HTML); 
#ifndef WIN32 
					sprintf(str,"%s/.vpf%d.html",TEMPDIR,getpid());
#else
					sprintf(str,"%s/%d.html",TEMPDIR,getpid());
#endif
					int fd=creat(str,0666); 
 
					write(fd,ch,strlen(ch)); 
					write(fd,buf,strlen(buf)); 
					ch="\n</PRE></BODY></HTML>"; 
					write(fd,ch,strlen(ch)); 
 
					free(buf); 
					close(fd); 
					term->Frame(str); 
					unlink(str); 
				} 
				else if(j==c_XML || j==c_Schema)
				{ 
					char *buf; 
					char str[256]; 
 
					sprintf(str,"CLASS \"%s\"\n",pc_demos(name));
					buf=(char *)malloc(strlen(str)+1); 
					strcpy(buf,str); 
 
					char *dir=get_user_dir(name,FILES); 
					if(dir==NULL) 
					{ 
						dir=(char *)malloc(3); 
						strcpy(dir,"./"); 
					} 
					char *name=NULL; 
					if(get_filename(dir,&name,1)==F10) 
					{ 
						free(dir); 
						free(buf); 
						break; 
					} 
					strcat(dir,"/"); 
					strcat(dir,name); 
					free(name); 
					Schema_to_Text(struc,buf,0,j==c_XML); 
					int fd=creat(dir,0666); 
 
					if(j==c_XML) 
					{ 
						char *ch="<HTML><META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=KOI8-R\"><BODY><PRE>\n"; 
						write(fd,ch,strlen(ch)); 
					} 
					write(fd,buf,strlen(buf)); 
 
					if(j==c_XML) 
					{ 
						char *ch="\n</PRE></BODY></HTML>"; 
						write(fd,ch,strlen(ch)); 
					} 
					free(dir); 
					free(buf); 
					close(fd); 
					term->restore_box(f); 
					term->free_box(f); 
					goto EXIT; 
				} 
				else if(term->ev().x>=x0+1 && term->ev().x<=x0+len && term->ev().b==1) 
				{ 
					if((pos==get_pos(term->ev().x-x0)) && (term->ev().y==act+y0+1)) 
						goto EDIT0; 
					if(term->ev().y>=y0+1 && term->ev().y<=y0+1+num) 
					{ 
						int i; 
 
						act=term->ev().y-y0-1; 
						i=get_pos(term->ev().x-x0); 
						if(i<0) 
							break; 
						pos=i; 
						if(sla!=NULL)
							sla->n=act+shift+1;
					} 
				} 
				else if(term->ev().b!=1) 
				{ 
					term->restore_box(f); 
					term->free_box(f); 
					goto EXIT; 
				} 
				break; 
			case PU: 
				if(act+shift-nn>1 && shift>nn) 
				{ 
					shift-=nn; 
				} 
				else 
				{ 
					act=0; 
					shift=0; 
				} 
				break; 
			case PD: 
				if(shift+2*nn<struc->ptm-1) 
				{ 
					shift+=nn; 
					break; 
				} 
			case EN: 
				act=i-1; 
				shift=struc->ptm-1-act; 
				break; 
			case HM: 
				act=0; 
				shift=0; 
				break; 
			case CR: 
				if(pos<5)
					pos++; 
				break; 
			case CL: 
				if(pos) 
					pos--; 
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
				else if(act+shift<struc->ptm-1) 
					shift++; 
				break; 
			case F12: 
			case F10: 
			case DEL: 
			case '\n': 
				term->restore_box(f); 
				term->free_box(f); 
				goto EXIT; 
			case '\r': 
				kod=0; 
				if(sla!=NULL) 
					sla->n=act+shift+1; 
				goto EDIT0; 
		} 
	} 
EDIT0: 
	if(struc->field[act+shift].st.st!=NULL) 
	{ 
		term->BOX(x0,y0,len+2,nn+2,' ',0,017,0,0x3); 
		term->Set_Color(017,0); 
		if(field) 
		{ 
			char str[16]; 
			sprintf(str,"%d",field); 
			term->dpp(x0+len,y0); 
			term->dps(str); 
		} 
		if(*name) 
		{ 
			if(rec==0) 
				term->dpp(x0+(len-strlen(name))/2,y0); 
			else 
				term->dpp(x0+2,y0); 
			term->dps(name); 
		} 
		kod=Show_Structure(struc->field[act+shift].st.st,rec+1,struc->field[act+shift].name,act+shift+1,sla==NULL?NULL:sla+1); 
		if(kod==F10 || kod==F12) 
			goto EXIT; 
		struc->field[act+shift].l=len_struct(struc->field[act+shift].st.st); 
		goto END; 
	} 
END: 
	term->restore_box(f); 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
	if(sla==NULL) 
		goto BEGIN; 
EXIT: 
	term->restore_box(f); 
	term->free_box(f); 
	if(rec==0) 
		clean_menu(); 
	return(kod); 
} 
