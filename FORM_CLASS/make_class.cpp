/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:make_class.cpp
*/

#include "StdAfx.h" 
#include "../CX_Browser.h" 
 
static struct st sss; 
static struct header *contex; 
 
int  main_box(struct st *struc,int,char *,char *,int); 
int  len_struct(struct st *); 
void load_struct(char *,struct st *&); 
void Create_Folder(char *iname,struct st *st, int new_database); 
 
static int num_soft; 
static int num_complex; 
static int num_expr; 
 
static char *prop_blank="旼컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커\n\
~�^2.1_____________________�^2.2.1_________________________________________ �\n\
읕컴컴컴컴컴컴컴컴컴컴좔컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸"; 
 
extern Terminal *term; 
extern Line *EditLine; 
 
void Make_Class(char *iname,struct st *st0, int new_database)
{ 
	struct st *struc; 
	struct st space,blank,tg,ds; 
	char *name; 
 
	bzero(&space,sizeof (struct st)); 
	space.ptm=2; 
	space.size=12;
	space.field=(struct field *)calloc(2,sizeof (struct field)); 
	space.field[0].a=X_INTEGER; 
	space.field[0].l=4; 
	space.field[1].a=X_INTEGER; 
	space.field[1].l=8;
 
	int read_only=(new_database==0 && access(iname,W_OK)); 
 
	hot_line(message(63));
	if(st0==NULL) 
	{ 
		struc=(struct st *)calloc(sizeof (struct st),1); 
		struc[0].ptm=1; 
		struc[0].field=(struct field *)calloc(sizeof (struct field),1); 
 
		struc[0].field[0].a=X_STRING; 
		struc[0].field[0].l=16; 
 
		if(if_base("",iname)) 
			load_struct(iname,struc); 
 
		if(main_box(struc,0,iname,iname,0)==F10) 
		{ 
			term->MultiColor(0,0,term->l_x(),term->l_y()); 
			return; 
		} 
		term->MultiColor(0,0,term->l_x(),term->l_y()); 
		if(read_only) 
		{ 
			dial(message(15),4); 
			return; 
		} 
		if(!new_database && !dial(message(60),0)) 
		{ 
			return; 
		} 
	} 
	else    struc=st0; 
	int f=term->get_box(0,0,term->l_x(),term->l_y());
	term->BOX((term->l_x()-strlen(message(67)))/2-1,term->l_y()/2-2,strlen(message(67))+2,3,' ',41,41,41,41);
	term->Set_Color(6,15); 
	term->dpp((term->l_x()-strlen(message(67)))/2,term->l_y()/2-1); 
	term->dps(message(67)); 
	term->scrbufout(); 
 
	create_class(struc,iname,new_database>1);
 
	char *DBNAME=(char *)malloc(strlen(iname)+1); 
	name=(char *)malloc(strlen(iname)+2); 
	strcpy(name,iname); 
 
	if(num_soft && !if_base(name,FLEXDB)) 
	{ 
		struct st st; 
		st.ptm=1; 
		st.field=(struct field *)calloc(1,sizeof (struct field)); 
		st.field->a=X_STRING; 
		st.field->l=64; 
		DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(FLEXDB)+2); 
		sprintf(DBNAME,"%s/%s",name,FLEXDB); 
		create_class(&st,DBNAME,1);
		free(st.field); 
	} 
 
	if(new_database<2 && !if_base(name,PROPERTY)) 
	{ 
		struct st st; 
		st.ptm=2; 
		st.field=(struct field *)calloc(2,sizeof (struct field)); 
		st.field[0].a=X_POINTER; 
		st.field[0].l=4; 
		st.field[0].name=(char *)malloc(strlen(name)+16); 
		sprintf(st.field[0].name,"../%s",name); 
		st.field[1].a=X_VARIANT; 
		st.field[1].l=6; 
		st.field[1].n=2; 
		st.field[1].name=(char *)malloc(64); 
		sprintf(st.field[1].name,"%s/%s",PROPERTY,FLEXDB); 
		DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(PROPERTY)+2); 
		sprintf(DBNAME,"%s/%s",name,PROPERTY); 
		create_class(&st,DBNAME,1);
		free(st.field); 
		DBNAME=(char *)realloc(DBNAME,strlen(DBNAME)+64); 
		sprintf(DBNAME,"%s/%s/%s/property",name,PROPERTY,BLANKDIR); 
		int fd=creat(DBNAME,0644); 
		write(fd,prop_blank,strlen(prop_blank)); 
		close(fd); 
		st.field=(struct field *)calloc(1,sizeof (struct field));  
		st.ptm=1; 
		st.field->a=X_STRING; 
		st.field->l=64; 
		DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(PROPERTY)+strlen(FLEXDB)+16); 
		sprintf(DBNAME,"%s/%s/%s",name,PROPERTY,FLEXDB); 
		create_class(&st,DBNAME,1);
		free(st.field); 
	} 
	if(num_expr && !if_base(name,EXPRDB)) 
	{ 
		struct st st; 
		st.ptm=1; 
		st.field=(struct field *)calloc(1,sizeof (struct field)); 
		st.field->a=X_TEXT; 
		st.field->l=4; 
		DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(EXPRDB)+2); 
		sprintf(DBNAME,"%s/%s",name,EXPRDB); 
		create_class(&st,DBNAME,1);
		free(st.field); 
	} 
 
	if(num_complex && !if_base(name,COMPLDB)) 
	{ 
		struct st st; 
		st.ptm=2; 
		st.field=(struct field *)calloc(2,sizeof (struct field)); 
		st.field[0].a=X_STRING; 
		st.field[0].l=16; 
		st.field[1].a=X_BINARY; 
		st.field[1].l=8;
		DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(COMPLDB)+2); 
		sprintf(DBNAME,"%s/%s",name,COMPLDB); 
		create_class(&st,DBNAME,1);
		free(st.field); 
	} 

	DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(SPACEDB)+10); 
	sprintf(DBNAME,"%s/%s",name,SPACEDB); 

	if(!access(DBNAME,R_OK)) 
		goto A1; 
 
	create_class(&space,DBNAME,1);

A1: 
	DBNAME=(char *)realloc(DBNAME,strlen(name)+strlen(FORMDB)+strlen(SPACEDB)+strlen(HYPERFORM)+32); 
	sprintf(DBNAME,"%s/%s/Blank",name,BLANKDIR); 
	if(access(DBNAME,W_OK)) 
	{ 
		int fd=creat(DBNAME,0644); 
		CX_BASE *db; 
		try 
		{ 
			db=new CX_BASE(iname); 
		} 
		catch(...) 
		{ 
			free(DBNAME); 
			return; 
		} 
		char *buf=NULL;
		Schema_to_Text(struc,buf,0,0,1);
		write(fd,buf,strlen(buf));
		delete db; 
		close(fd); 
	} 
 
	bzero(&blank,sizeof (struct st)); 
	bzero(&tg,sizeof (struct st)); 
	bzero(&ds,sizeof (struct st)); 
	tg.ptm=sizeof(struct tag_descriptor)/4+3;
	tg.size=sizeof (struct tag_descriptor); 
	tg.field=(struct field *)calloc(tg.ptm,sizeof (struct field)); 
	int i; 
	for(i=0;i<tg.ptm;i++) 
	{ 
		tg.field[i].a=X_INTEGER; 
		if(i<6)
			tg.field[i].l=2;
		else
			tg.field[i].l=4;
	} 
	ds.ptm=sizeof(struct panel)/4;
	ds.size=sizeof (struct panel);
	ds.field=(struct field *)calloc(ds.ptm,sizeof (struct field)); 
	for(i=0;i<ds.ptm;i++) 
	{ 
		ds.field[i].a=X_INTEGER; 
		ds.field[i].l=4; 
	} 
	blank.ptm=5; 
	blank.size=48; 
	blank.field=(struct field *)calloc(blank.ptm,sizeof (struct field)); 
	blank.field[0].a=X_STRING; 
	blank.field[0].l=32; 
	blank.field[1].a=X_TEXT; 
	blank.field[1].l=8;
	blank.field[2].a=X_STRUCTURE; 
	blank.field[2].l=88; 
	blank.field[2].m=MULTISET; 
	blank.field[2].st.st=&tg; 
	blank.field[3].a=X_STRUCTURE; 
	blank.field[3].l=28; 
	blank.field[3].m=MULTISET; 
	blank.field[3].st.st=&ds; 
	blank.field[4].a=X_INTEGER; 
	blank.field[4].l=4; 
 
	sprintf(DBNAME,"%s/%s",name,FORMDB); 
	if(new_database<2 && access(DBNAME,R_OK)) 
		create_class(&blank,DBNAME,1);

	sprintf(DBNAME,"%s/%s/%s",name,FORMDB,SPACEDB); 
	if(new_database<2 && access(DBNAME,R_OK)) 
		create_class(&space,DBNAME,1);

	sprintf(DBNAME,"%s/%s",name,HYPERFORM); 
	if(new_database<2 && access(DBNAME,R_OK)) 
	{ 
#ifdef WIN32 
		mkdir(DBNAME); 
#else 
		mkdir(DBNAME,0777); 
#endif 
	} 
 
	sprintf(DBNAME,"%s/%s/%s",name,HYPERFORM,FORMDB); 
	if(new_database<2 && access(DBNAME,R_OK)) 
		create_class(&blank,DBNAME,1);

	sprintf(DBNAME,"%s/%s/%s/%s",name,HYPERFORM,FORMDB,SPACEDB); 
	if(new_database<2 && access(DBNAME,R_OK)) 
		create_class(&space,DBNAME,1);

	sprintf(DBNAME,"%s/Methods.cc",name); 
	if(new_database<2 && access(DBNAME,R_OK)) 
	{ 
		fcopy(name,"/usr/local/etc/Methods.cc");
	} 
	free(DBNAME); 

	term->restore_box(f);
	term->free_box(f);

	if(dial(message(72),1))
	{
		CX_BASE *db;
		try
		{
			db=new CX_BASE(name);
		}
		catch(...)
		{
			return;
		}
		Edit_Form(db,1);
		delete db;
	}
} 
 
int last_char; 
int Choise_Box(struct item *names,int x0,int y0,int size,int start) 
{ 
	int i,act=start; 
	int num,len,f; 
	int shift=0,nn; 
 
	for(num=0,len=0;*names[num].name;num++) 
	{ 
		if((int)strlen(names[num].name)>len) 
			len=strlen(names[num].name); 
	} 
	if(num>size) 
		nn=size; 
	else    nn=num; 
	f=term->get_box(x0,y0,len+5,nn+4);
	term->Set_Font(0,0); 
	term->BOX(x0,y0,len+3,nn+2,' ',41,41,41,41);
	for(;;) 
	{ 
		char str[32]; 
		int y=0; 
 
		sprintf(str,"%d",act+1+shift); 
		term->MultiColor(x0,y0,len+3,nn+2);
		term->dpp(x0+len-strlen(str)+1,y0+nn+1);
		term->Set_Color(41,0);
		term->dps(str); 
		term->scr->draw_frame(x0,y0,len+3,nn+2);
		term->Set_Font(0,2);
		for(i=0;i<num && i<size;i++) 
		{ 
			term->Set_Color(0x200+names[shift+i].bg,names[shift+1].fg);
			term->dpp(x0+1,y0+1+i);
			term->dps(" ");
			term->dps(names[shift+i].name); 
			term->dpn(len-strlen(names[shift+i].name)-1,' ');
		} 
		term->scr->draw_frame(x0+1,y0+1,len+1,nn,1);

		term->Set_Font(0,0);
		term->dpp(x0+1,y0+1+act);
		term->Set_Color(0x10f,0xe);
		term->dps(" ");
		term->dps(names[shift+act].name);
		term->dpn(len-strlen(names[shift+act].name)-1,' ');

		term->dpp(x0+2,y);
		switch(last_char=Xmouse(term->dpi())) 
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
			case F10: 
			case F12: 
				act=-1; 
				shift=0; 
				goto EXIT; 
			case '\r': 
				goto EXIT; 
		} 
	} 
EXIT: 
	term->restore_box(f); 
	term->free_box(f); 
	term->BlackWhite(0,0,term->l_x(),term->l_y()); 
	return(act+shift); 
} 
 
 
int Choise_Box(char **names,int x0,int y0,int size,int start) 
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
		name[i].font=0; 
	} 
	i=Choise_Box(name,x0,y0,size,start); 
	free(name); 
	return(i); 
} 

struct item ATR[]=
{
	{"String     ",142,0,0},
	{"Date       ",135,0,0},
	{"Time       ",7,  0,0},
	{"Pointer    ",176,0,0},
	{"Variant    ",175,0,0},
	{"Integer    ",170,0,0},
	{"Unsigned   ",171,0,0},
	{"Float      ",138,0,0},
	{"Double     ",44, 0,0},
	{"Structure  ",137,0,0},
	{"Text       ",139,0,0},
	{"Binary     ",140,0,0},
	{"FileName   ",167,0,0},
	{"Expression ",46, 0,0},
	{"Image      ",38, 0,0},
	{"Complex    ",15, 0,0},
	{"Hyperform  ", 0, 7,0},
	{"",          0,  0, 0}
};
int get_pos(int x) 
{ 
	if(x<5) 
		return(-1); 
	return(x>37?5:x>31?4:x>25?3:x>23?2:x>17?1:x>5?0:-1); 
} 
int get_len(int i) 
{ 
	switch(i) 
	{ 
		case X_STRING: 
			return(16); 
		case X_DATE: 
		case X_TIME: 
			return(3); 
		case X_VARIANT: 
		case X_COMPLEX: 
			return(6); 
		case X_DOUBLE: 
		case X_TEXT:
		case X_BINARY:
		case X_EXPRESSION:
		case X_FILENAME:
		case X_IMAGE:
			return(8); 
		default: 
			return(4);
	} 
} 
struct st *copy_struct(struct st *struc) 
{ 
	int i; 
	struct st *new_struc; 
 
	new_struc=(struct st *)malloc(sizeof (struct st)); 
	new_struc->ptm=struc->ptm; 
	new_struc->size=struc->size; 
 
	new_struc->field=(struct field *)malloc(struc->ptm*sizeof (struct field)); 
 
	for(i=0;i<struc->ptm;i++)
	{ 
		memcpy(new_struc->field+i,struc->field+i,sizeof (struct field)); 
 
		if(struc->field[i].name!=NULL) 
		{ 
			new_struc->field[i].name=(char *)malloc(strlen(struc->field[i].name)+1); 
			strcpy(new_struc->field[i].name,struc->field[i].name); 
		} 
		if(struc->field[i].st.st!=NULL) 
			new_struc->field[i].st.st=copy_struct(struc->field[i].st.st); 
	} 
	return(new_struc); 
} 
void free_struct(struct st *struc) 
{ 
	int i; 
 
	for(i=0;i<struc->ptm;i++) 
	{ 
		if(struc->field[i].name!=NULL) 
			free(struc->field[i].name); 
		if(struc->field[i].st.st!=NULL) 
		{ 
			free_struct(struc->field[i].st.st); 
			free(struc->field[i].st.st); 
		} 
	} 
} 
 
int main_box(struct st *struc,int rec,char *iname,char *name,int field) 
{ 
	int f,x0=7,y0=2; 
	int len,i; 
	int act=0,shift=0,pos=0; 
	int y,kod=0; 
	int nn,num=20; 
 
	x0+=rec; 
	y0+=rec; 
//        len-=rec;
	num-=rec; 
	len=term->l_x()-x0-12; 
	term->BlackWhite(0,0,term->l_x()+1,term->l_y());
	f=term->get_box(0,0,term->l_x(),term->l_y()); 
BEGIN: 
	if(struc->ptm<num) 
		nn=struc->ptm; 
	else    nn=num; 
	term->MultiColor(x0,y0,len+2,nn+2);
	term->BOX(x0,y0,len+2,nn+2,' ',41,41,41,41);
	term->Set_Font(1,0);
	term->Set_Color(41,0);
	term->dpp(x0+9,y0);  term->dps("Type");
	term->dpp(x0+19,y0); term->dps("Len");
	term->dpp(x0+24,y0); term->dps("K"); 
	term->dpp(x0+27,y0); term->dps("Mod"); 
	term->dpp(x0+33,y0); term->dps("Dir"); 
	term->dpp(x0+39,y0); term->dps("Name");
	if(name!=NULL && *name) 
	{ 
		term->dpp(x0+39+(len-39-strlen(name))/2,y0); 
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
			char str[64]; 
 
			term->Set_Font(0,2);
			term->dpp(x0+1,y0+1+i); 
			term->Set_Color(41,0);
			sprintf(str,"%4d",shift+i+1); 
			term->dps(str); 
 
			term->dpp(x0+6,y0+1+i); 
			if(i==act && pos==0) 
			{ 
				term->Set_Font(0,0);
				term->Set_Color(0x10f,0xe);
				y=y0+1+i; 
			} 
			else term->Set_Color(0x200+ATR[struc->field[i+shift].a-1].bg,ATR[struc->field[i+shift].a-1].fg);
			term->dps(ATR[struc->field[i+shift].a-1].name); 
			term->dpp(x0+18,y0+1+i); 
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
			term->dpp(x0+24,y0+1+i); 
 
			if(struc->field[i+shift].k)
				term->dps("T");
			else if(struc->field[i+shift].b)
				term->dps("B");
			else    term->dps(" "); 
 
			term->dpp(x0+26,y0+1+i); 
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

			term->dpp(x0+32,y0+1+i); 
			if(i==act && pos==4) 
			{ 
				term->Set_Color(0x10f,0x0);
				y=y0+1+i; 
			} 
			else term->Set_Color(0x200+41,0);
			if(struc->field[i+shift].d) 
				term->dps(" --> "); 
			else    term->dps("     "); 
 
			term->dpp(x0+38,y0+1+i); 
			if(i==act && pos==5) 
			{ 
				term->Set_Color(0x10f,0x0);
				y=y0+1+i; 
			} 
			else term->Set_Color(0x200+41,0);
			term->dpn(len-37,' '); 
			term->dpp(x0+38,y0+1+i); 
			if(struc->field[i+shift].name!=NULL) 
			{ 
				strncpy(str,struc->field[i+shift].name,len-37); 
				str[len-37]=0; 
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
		term->scr->draw_frame(x0,y0,len+2,nn+2);
		switch(last_char=kod=Xmouse(term->dpi())) 
		{ 
 
			case 0: 
				if(term->ev().x>=x0+1 && term->ev().x<=x0+len) 
				{ 
					if(term->ev().b!=1) 
					{ 
						term->restore_box(f); 
						term->free_box(f); 
						goto EXIT; 
					} 
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
					} 
				} 
				break; 
			case F1:
				Help(3,1);
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
			case F10: 
//                                term->BlackWhite(0,0,term->l_x()+1,term->l_y());
				if(!dial(message(61),0)) 
				{
//                                        term->BlackWhite(0,0,term->l_x()+1,term->l_y());
//                                        term->MultiColor(x0,y0,len+2,nn+2);
					break; 
				}
			case F12: 
			case '\n': 
				term->restore_box(f); 
				term->free_box(f); 
				goto EXIT; 
			case DEL: 
				if(struc->ptm<2) 
					break; 
				if(struc->field[act+shift].a==X_EXPRESSION) 
					num_expr--; 
				if(struc->field[act+shift].a==X_COMPLEX) 
					num_complex--; 
				if(struc->field[act+shift].a==X_VARIANT) 
					num_soft--; 
				if(struc->field[act+shift].name!=NULL) 
					free(struc->field[act+shift].name); 
				if(struc->field[act+shift].st.st!=NULL) 
					free_struct(struc->field[act+shift].st.st); 
				if(act+shift<struc->ptm) 
					memcpy(struc->field+act+shift,struc->field+act+shift+1,(struc->ptm-act-shift)*sizeof (struct field)); 
				struc->field=(struct field *)realloc(struc->field,--struc->ptm*sizeof (struct field)); 
				if(act+shift>=struc->ptm) 
				{ 
					if(shift) 
						shift--; 
					else    act--; 
				} 
				term->restore_box(f); 
				goto BEGIN; 
			case IS: 
			{ 
				struc->field=(struct field *)realloc(struc->field,(struc->ptm+1)*sizeof (struct field)); 
				memcpy(struc->field+struc->ptm,struc->field+act+shift,sizeof (struct field)); 
 
				struc->field[struc->ptm].st.st=NULL; 
				if(struc->field[act+shift].name!=NULL) 
				{ 
					struc->field[struc->ptm].name=(char *)malloc(strlen(struc->field[act+shift].name)+1); 
					strcpy(struc->field[struc->ptm].name,struc->field[act+shift].name); 
				} 
				else 
					struc->field[struc->ptm].name=NULL; 
 
				if(struc->field[struc->ptm].a==X_STRUCTURE) 
					struc->field[struc->ptm].st.st=copy_struct(struc->field[act+shift].st.st); 
				struc->ptm++; 
				term->restore_box(f); 
				goto BEGIN;; 
			} 
			case '\r': 
				kod=0; 
				goto EDIT0; 
			case ' ': 
				kod=0; 
				goto EDIT1; 
			default: 
				if(kod>' ') 
					goto EDIT1; 
		} 
	} 
EDIT0: 
	if(struc->field[act+shift].st.st!=NULL) 
	{ 
		if(rec<SLA_DEEP-1) 
		{ 
			term->Set_Color(41,0);
			term->dpp(x0+1,y0); term->dpn(len,' ');
			if(field) 
			{ 
				char str[16]; 
				sprintf(str,"%d",field); 
				term->dpp(x0+len,y0); 
				term->dps(str); 
			} 
			if(name!=NULL && *name)
			{ 
				if(rec==0) 
				{ 
					term->Set_Color(41,15);
					term->dpp(x0+(len-strlen(name))/2,y0); 
				} 
				else 
					term->dpp(x0+2,y0); 
				term->dps(name); 
			} 
			term->scr->draw_frame(x0,y0,len+2,nn+2);
			main_box(struc->field[act+shift].st.st,rec+1,iname,struc->field[act+shift].name,act+shift+1); 
			struc->field[act+shift].l=len_struct(struc->field[act+shift].st.st); 
		} 
		term->BlackWhite(0,0,term->l_x()+1,term->l_y()); 
		goto END; 
	} 
EDIT1: 
	switch(pos) 
	{ 
		case 0: 
			if((i=Choise_Box(ATR,1,1,20,struc->field[act+shift].a-1)+1)>0) 
			{ 
				if(i==(sizeof ATR)/sizeof (struct item)-1) 
				{ 
					char str[256]; 
					sprintf(str,"%s/%s/%s",iname,HYPERFORM,FORMDB); 
 
					for(int j=0;j<struc->ptm;j++) 
					{ 
						if(struc->field[j].name!=NULL && !strcmp(struc->field[j].name,str) && struc->field[j].a==X_POINTER) 
							goto END; 
					} 
					bzero(struc->field+act+shift,sizeof (struct field)); 
					struc->field[act+shift].name=(char *)malloc(strlen(str)+1); 
					strcpy(struc->field[act+shift].name,str); 
					i=X_POINTER; 
				} 
				else 
					bzero(struc->field+act+shift,sizeof (struct field)); 
				struc->field[act+shift].a=i; 
				struc->field[act+shift].l=get_len(i); 
				if(i==X_EXPRESSION) 
					num_expr++; 
				if(i==X_COMPLEX) 
					num_complex++; 
				if(i==X_VARIANT) 
				{ 
					char str[256]; 
					sprintf(str,"%s/%s",iname,FLEXDB); 
					struc->field[act+shift].name=(char *)malloc(strlen(str)+1); 
					strcpy(struc->field[act+shift].name,str); 
					num_soft++; 
				} 
				if(i==X_VARIANT || i==X_COMPLEX) 
					struc->field[act+shift].n=2; 
				if(i==X_STRUCTURE) 
				{ 
					struct st *new_struc; 
 
					new_struc=(struct st *)malloc(sizeof (struct st)); 
					new_struc->ptm=1; 
					new_struc->field=(struct field *)malloc(sizeof (struct field)); 
					bzero(new_struc->field,sizeof (struct field)); 
					new_struc->field[0].a=X_STRING; 
					new_struc->field[0].l=16; 
 
					struc->field[act+shift].st.st=new_struc; 
					struc->field[act+shift].l=len_struct(struc->field[act+shift].st.st); 
				} 
			} 
			last_char=0; 
			break; 
		case 1: 
		{ 
			char *ch,str[16]; 
 
			if(struc->field[act+shift].a==X_DATE || struc->field[act+shift].a==X_STRUCTURE) 
				break; 
			sprintf(str,"%d",struc->field[act+shift].l); 
			if(kod) 
				*str=0; 
DPEDIT:
			term->Set_Color(120,0);
			if(EditLine->edit(kod,str,5,5,x0+18,y0+act+1,10)!=F10 && atoi(str)>0) 
			{ 
				if(struc->field[act+shift].a!=X_FLOAT && struc->field[act+shift].a!=X_DOUBLE && struc->field[act+shift].a!=X_DATE) 
					struc->field[act+shift].l=atoi(str); 
			} 
			if(struc->field[act+shift].a>=X_TEXT && (atoi(str)<4 || atoi(str)>8)) 
			{ 
				*str=0; 
				kod=0; 
				goto DPEDIT; 
			} 
			if(struc->field[act+shift].a==X_TIME) 
			{ 
				if(struc->field[act+shift].l>3) 
					struc->field[act+shift].l=4; 
				else struc->field[act+shift].l=3; 
			} 
			if((ch=strchr(str,'.'))!=NULL) 
				struc->field[act+shift].n=atoi(ch+1); 
			break; 
		} 
		case 2: 
			if(struc->field[act+shift].a!=X_STRUCTURE && struc->field[act+shift].a!=X_BINARY && struc->field[act+shift].a!=X_COMPLEX)
			{
				if(struc->field[act+shift].k)
				{
					struc->field[act+shift].k=0;
					struc->field[act+shift].b=1;
				}
				else if(struc->field[act+shift].b)
				{
					struc->field[act+shift].k=0;
					struc->field[act+shift].b=0;
				}
				else
				{
					struc->field[act+shift].k=1;
					struc->field[act+shift].b=0;
				}

			}
			else
			{
				struc->field[act+shift].k=0;
				struc->field[act+shift].b=0;
			}
			break; 
		case 3: 
			if(struc->field[act+shift].k || struc->field[act+shift].b)
				struc->field[act+shift].m=0; 
			else    struc->field[act+shift].m+=1; 
			break; 
		case 4: 
			if(struc->field[act+shift].k || struc->field[act+shift].b)
				struc->field[act+shift].d=0; 
			else if(!rec) 
				struc->field[act+shift].d=!struc->field[act+shift].d; 
			break; 
		case 5: 
		{ 
			char str[256]; 
			int code; 
 
			if(struc->field[act+shift].name!=NULL) 
				sprintf(str,"%s",struc->field[act+shift].name); 
			else    *str=0; 
			if(kod) 
				*str=0; 
			term->Set_Color(120,0);
			code=EditLine->edit(kod,str,255,len-37,x0+38,y0+act+1,0);
			if(struc->field[act+shift].a==X_POINTER || struc->field[act+shift].a==X_VARIANT) 
			{ 
				if(code==F7) 
				{ 
					char *name=Select_From_Dir(".",if_base); 
					term->MultiColor(x0,y0,len+2,nn+2); 
					term->MultiColor(x0,term->l_y(),len+2,1); // help line 
					if(name==NULL || !*name)
					{
						if(name!=NULL)
							free(name);
						break; 
					}
					strcpy(str,name); 
					free(name); 
				} 
				else 
				{ 
					if(!if_base(0,str) && strcmp(str,iname))
					{ 
						if(dial(message(62),1)) 
							Make_Class(str,NULL,1);
					} 
				} 
			} 
			if(code!=F10) 
			{ 
				if(struc->field[act+shift].name!=NULL) 
					free(struc->field[act+shift].name); 
				struc->field[act+shift].name=(char *)malloc(strlen(str)+1); 
				strcpy(struc->field[act+shift].name,str); 
			} 
			break; 
		} 
		default: 
			break; 
 
	} 
END: 
	term->restore_box(f); 
	if(last_char==F10 || last_char==F12) 
		return(last_char); 
	goto BEGIN; 
EXIT: 
	return(kod); 
} 
 
int create_struct(struct st *ss,struct header *context) 
{ 
	register int i; 
 
	for(i=0;i<ss->ptm;i++) 
	{ 
		int st_ptm=ss->field[i].atr.num_subfield; 
		int length; 
		char *ch=(char *)context+(long)ss->field[i].name; 
 
		ss->field[i].name=(char *)calloc(strlen(ch)+1,1); 
		strcpy(ss->field[i].name,ch); 
		if(ss->field[i].a==X_STRUCTURE) 
		{ 
			struct field *fi; 
 
			int shift=ss->field[i].st.struct_descr; 
 
			ss->field[i].st.st=(struct st *)calloc(1,sizeof (struct st)); 
 
			fi=(struct field *)(context+1+shift); 
 
			ss->field[i].st.st->field=(struct field *)calloc(st_ptm,sizeof (struct field)); 
			bcopy(fi,ss->field[i].st.st->field,st_ptm*sizeof (struct field)); 
 
			ss->field[i].st.st->ptm=st_ptm; 
			ss->field[i].st.st->size=0; 
 
			create_struct(ss->field[i].st.st,context); 
		} 
		if(ss->field[i].m) 
			length=8;
		else 
			length=ss->field[i].l; 
		ss->size+=(ss->field[i].d?0:length); 
	} 
	return(ss->size); 
} 
 
void  load_struct(char *folder,struct st *&s) 
{ 
	int df; 
	char *header_name; 
	struct stat st; 
 
	bzero(&sss,sizeof sss); 
	if(!if_base(0,folder)) 
	{ 
		return; 
	} 
	header_name=(char *)malloc(2*strlen(folder)+2); 
	full(folder,folder,header_name); 
 
	if(header_name==NULL || (df=open(header_name,O_RDONLY))<0) 
	{ 
		return; 
	} 
	free(header_name); 
	fstat(df,&st); 
	contex = (struct header *)malloc(st.st_size); 
	read(df,contex,st.st_size); 
	close(df); 
	if(contex->pswd!=CXKEY4 && contex->pswd!=CXKEY5 && contex->pswd!=CXKEY6)
	{ 
		free(contex); 
		return; 
	} 
 
	sss.ptm=contex->ptm; 
	sss.size=sizeof (struct key); 
	sss.field=(struct field *)(contex+1); 
 
	struct field *fi; 
	fi=(struct field *)calloc(sss.ptm,sizeof (struct field)); 
	bcopy(sss.field,fi,sss.ptm*sizeof (struct field)); 
	sss.field=fi; 
 
	create_struct(&sss,contex); 
	s=&sss; 
} 
