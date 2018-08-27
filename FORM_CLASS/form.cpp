/*
			   DBMS ConteXt V.6.0
		       ConteXt library libcxform.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:form.cpp
*/
#include "StdAfx.h" 
#include "../CX_Browser.h" 
 
extern Terminal *term; 
 
static int get_atr(char *ch,long *atr); 
static int get_blank_color(char *ch,struct color *color); 
static void str_to_color(char *str,struct color *color); 
 
static int comp_d(const void *a1,const void *a2) 
{ 
	struct tag *a=(struct tag *)a1; 
	struct tag *b=(struct tag *)a2; 
	return((a->des.y>b->des.y)?1:(a->des.y<b->des.y)?-1:(a->des.x>b->des.x)?1:(a->des.x<b->des.x)?-1:0); 
} 
 
FORM::FORM(char *base) 
{ 
	bzero(this,sizeof (FORM)); 
	if(base!=NULL) 
		create_Form(base,1); 
} 
 
FORM::FORM(char *base,long record) 
{ 
	bzero(this,sizeof (FORM)); 
	if(base!=NULL) 
		create_Form(base,record); 
} 
 
FORM::FORM(char *base,char *name) 
{ 
	bzero(this,sizeof (FORM)); 
	if(base!=NULL) 
		create_Form(base,name); 
} 
 
FORM::FORM(char *base,struct x_form *x) 
{ 
	bzero(this,sizeof (FORM)); 
	if(base!=NULL) 
	{ 
		if(!(*x->blank) && x->form==0) 
		{
			create_Form(base,0);
		}
		else if(x->form) 
		{
			create_Form(base,x->form);
		}
		else    if(*x->blank) 
			{
				create_Form(base,x->blank); 
			}
	} 
} 
 
FORM::FORM(int num_field,struct tag *des) 
{ 
	int len; 
	bzero(this,sizeof (FORM)); 
 
	background=(char *)malloc(len=2); 
	background[0]='~'; 
	background[1]='│'; 
	for(int i=0;i<num_field;i++) 
	{ 
		char str[128],l; 
 
		sla_to_str(des[i].des.sla,str); 
		l=strlen(str); 
		background=(char *)realloc(background,len+l); 
		bcopy(str,background+len,l); 
		len+=l; 
		l=des[i].des.l+1; 
		background=(char *)realloc(background,len+l); 
		memset(background+len,'_',des[i].des.l); 
		len+=l; 
		background[len-1]='│'; 
	} 
	background=(char *)realloc(background,len+2); 
	background[len]='\n'; 
	background[len+1]=0; 
	strcpy(name_form.blank,"Manual"); 
	get_blank(); 
	Get_Size(); 
} 
 
/* this is a true constructor */ 
void FORM::create_Form(char *base) 
{ 
	if(name_form.form) 
	{
		create_Form(base,name_form.form); 
	}
	else
	{
		create_Form(base,name_form.blank);
	}
} 
 
 
void FORM::create_Form(char *base,char *nam) 
{ 
	if(nam!=NULL && !strcmp(nam,"Manual")) 
	{ 
		Get_Size(); 
		return; 
	} 
	CX_BASE *form=NULL; 
 
	char *str=get_user_dir(base,FORMDB); 
	char *name=nam; 
 
	if(str!=NULL && name!=NULL && if_base("",str)) 
	{ 
		try 
		{ 
			form = new CX_BASE(str); 
		} 
		catch(...) 
		{ 
			form=NULL; 
		} 
		if(form!=NULL && form->last_cadr()<1) 
		{ 
			delete(form); 
			form=NULL; 
		} 
	} 
 
	if(str!=NULL) 
		free(str); 
	if(form==NULL) 
	{ 
		name_form.form=0; 
		struct stat st; 
		char *blank_dir=get_user_dir(base,BLANKDIR); 
		char *str      =get_user_dir(base,BLANKDIR); 
		if(name==NULL || str==NULL) 
			goto MAN;
		str=(char *)realloc(str,strlen(str)+strlen(name)+2); 
		strcat(str,"/"); 
		strcat(str,name); 
READ: 
		if(!stat(str,&st) && (st.st_mode&S_IFREG)) 
		{ 
			int fd; 
			struct stat st; 
 
			Free_Form(); 
			fd=open(str,O_RDONLY); 
			fstat(fd,&st); 
 
			background=(char *)realloc(background,st.st_size+1); 
			bzero(background,st.st_size+1); 
 
			read(fd,background,st.st_size); 
			close(fd); 
			get_blank(base); 
			if(!strncmp(str,blank_dir,strlen(blank_dir))) 
				strncpy(name_form.blank,str+strlen(blank_dir)+1,(sizeof name_form.blank)-1); 
			else 
				strncpy(name_form.blank,str,(sizeof name_form.blank)-1); 
		} 
		else 
		{ 
			if(blank_dir!=NULL && !access(blank_dir,R_OK)) 
			{ 
				if(str!=NULL) 
					free(str); 
				str=Select_From_Dir(blank_dir,if_read,"Select Form"); 
				if(str!=NULL && *str)
					goto READ; 
			} 
MAN:
			background=(char *)realloc(background,64); 
			strcpy(background,"\n\t^1________________________________\n\n"); 
			get_blank(base); 
			strcpy(name_form.blank,"Default"); 
		} 
		if(blank_dir!=NULL) 
			free(blank_dir); 
		if(str!=NULL) 
			free(str); 
		Get_Size(); 
		if(name!=nam) 
			free(name); 
		return; 
	} 
 
	CX_FIND *fb = new CX_FIND(form); 
	long page; 
	if((page=fb->Find_First(1,name,0))<=0) 
		page=1; 
	while(form->Check_Del(page) && page<form->Max_Record()) 
		page++; 
	delete(fb); 
	delete(form); 
	if(name!=nam) 
		free(name); 
	bzero(this,sizeof (FORM)); 
	create_Form(base,page); 
} 

void FORM::create_Form(char *base,int record) 
{ 
	struct tag_descriptor *des=NULL; 
	char *ch=NULL; 
	char *str=get_user_dir(base,FORMDB); 
 
	CX_BASE *form=NULL; 
	if(if_base("",str)) 
	{ 
		try 
		{ 
			form = new CX_BASE(str); 
			free(str); 
			str=NULL; 
		} 
		catch(...) 
		{ 
			form=NULL; 
		} 
	} 
	if(form==NULL) 
	{ 
BLANK: 
		if(form!=NULL) 
		{ 
			delete(form); 
			form=NULL; 
		} 
		create_Form(base,""); 
		return; 
 
	} 
	if(form->Num_Fields()<4 || form->last_cadr()<1) 
	{ 
		delete(form); 
//                return(create_Form(base,(char *)NULL)); 
		create_Form(base,""); 
		return; 
	} 
	if(record<=0||record>form->Max_Record())
		record=1; 
	while(form->Check_Del(record) && record<form->Max_Record()) 
		record++; 
	Free_Form(); 
	form->Get_Slot(record,1,ch); 
	name_form.form=record; 
	if(ch!=NULL) 
	{ 
		strncpy(name_form.blank,ch,(sizeof name_form.blank)-1); 
		name_form.blank[(sizeof name_form.blank)-1]=0; 
		free(ch); 
		ch=NULL; 
	} 
	else    *name_form.blank=0; 
	if(form->Num_Fields()>4) 
	{ 
		form->Get_Slot(record,5,ch); 
		if(ch!=NULL) 
		{ 
			form_atr=atoi(ch); 
			free(ch); 
		} 
	} 
	size=form->Get_Slot(record,2,background); 
	if(background==NULL) 
		goto BLANK; 
 
	char *ch1=NULL; 
	int i,len;
 
	i=num_fields=form->Read(record,3,ch1).len;


	num_fields/=(len=form->Field_Descr(3)->l); 
 
//printf("num_fields=%d i=%d len=%d\n",num_fields,i,len); getchar();

	if(!num_fields) 
	{ 
		if(ch1!=NULL) 
			free(ch1); 
		delete(form); 
 
		char *str=get_user_dir(base,BLANKDIR); 
 
		char *name=Select_From_Dir(str,if_read,"Select Form"); 
		free(str); 
		if(name==NULL || !*name)
		{ 
			if(name!=NULL)
				free(name);
			get_blank(base); 
			return; 
		} 
		create_Form(base,name); 
		free(name); 
 
		return; 
	} 
	int old_form=0;
	if(len==sizeof (struct tag_descriptor)) 
	{ 
		des=(struct tag_descriptor *)ch1; 
	} 
	else 
	{ 
		struct old_tag_descriptor *d=(struct old_tag_descriptor *)ch1; 
		des=(struct tag_descriptor *)calloc(num_fields,sizeof (struct tag_descriptor)); 
		for(i=0;i<num_fields;i++)
		{ 
			des[i].x=d[i].x;
			des[i].y=d[i].y;
			des[i].l=d[i].l;
			des[i].atr=d[i].atr;
			des[i].color=d[i].color;
			bzero(des[i].sla,sizeof des[i].sla);
			bcopy(d[i].sla,des[i].sla,sizeof des[i].sla);
			if(des[i].sla->n==0)
				des[i].sla->n=-2;

		} 
		old_form=1;
		free(ch1); 
	} 
	ch1=NULL; 
	for(i=0;i<num_fields;i++)
	{
		if(des[i].l==0)
		{
			if(i<num_fields-1)
			{
				memcpy(des+i,des+i+1,(num_fields-i-1)*sizeof (struct tag_descriptor));
			}
			i--;
			num_fields--;
		}
	}

	num_panel=form->Read(record,4,ch1).len/(int)sizeof (struct panel);
	panel=(struct panel *)ch1;

	if(form->Num_Fields()>5)
	{
		num_label=form->Read(record,6,(char *&)label,0).len/(long)sizeof (struct label);
		struct sla sla[SLA_DEEP];
		bzero(sla,sizeof sla);
		sla[0].n=6;
		sla[1].n=sizeof(struct label)/4;
		for(i=0;i<num_label;i++)
		{
			++sla[0].m;
			label[i].text=NULL;
			form->Get_Slot(record,sla,label[i].text);
#ifdef SPARC
			conv((char *)&label[i].x,  sizeof label->x  );
			conv((char *)&label[i].y,  sizeof label->y  );
			conv((char *)&label[i].l,  sizeof label->l  );
			conv((char *)&label[i].h,  sizeof label->h  );
			conv((char *)&label[i].fg, sizeof label->fg );
			conv((char *)&label[i].bg, sizeof label->bg );
			conv((char *)&label[i].atr,sizeof label->atr);
			conv((char *)&label[i].font,sizeof label->font);
#endif
		}
	}
 
	delete(form); 
	if(tags!=NULL) 
		free(tags); 
	tags=NULL;
	if(num_fields>0)
		tags=(struct tag *)calloc(num_fields,sizeof (struct tag));

	for(i=0;i<num_fields;i++) 
	{ 
		tags[i].des=des[i]; 
#ifdef SPARC 
		conv((char *)&tags[i].des.x,       sizeof tags->des.x); 
		conv((char *)&tags[i].des.y,       sizeof tags->des.y); 
		conv((char *)&tags[i].des.l,       sizeof tags->des.l); 
		conv((char *)&tags[i].des.atr,     sizeof tags->des.atr); 
		conv((char *)&tags[i].des.color.bg,sizeof tags->des.color.bg); 
		conv((char *)&tags[i].des.color.fg,sizeof tags->des.color.fg); 
 
		for(int j=0;j<(sizeof tags->des.sla)/sizeof (struct sla);j++) 
		{ 
			conv((char *)&tags[i].des.sla[j].n,sizeof tags->des.sla->n); 
			conv((char *)&tags[i].des.sla[j].m,sizeof tags->des.sla->m); 
		} 
 
#endif 
		if(old_form==0 && tags[i].des.sla[SLA_DEEP-1].m)
		{ 
			tags[i].font.atr=tags[i].des.sla[SLA_DEEP-1].m&0x3; 
			tags[i].font.fnt=(tags[i].des.sla[SLA_DEEP-1].m>>4)&0x7; 
			tags[i].des.sla[SLA_DEEP-1].m=0; 
		} 
	} 
#ifdef SPARC 
	for(i=0;i<num_panel;i++)
	{ 
		conv((char *)&panel[i].x,  sizeof panel->x  );
		conv((char *)&panel[i].y,  sizeof panel->y  );
		conv((char *)&panel[i].l,  sizeof panel->l  );
		conv((char *)&panel[i].h,  sizeof panel->h  );
		conv((char *)&panel[i].fg, sizeof panel->fg );
		conv((char *)&panel[i].bg, sizeof panel->bg );
		conv((char *)&panel[i].atr,sizeof panel->atr);
	} 
#endif
	if(des!=NULL)
		free(des); 
	qsort(tags,num_fields,sizeof(struct tag),comp_d); 
	Get_Size(); 
//        look_for_tag_names(); 
} 
void FORM::Get_Size() 
{ 
	int x,y,i; 
	width=0; 
	if(background==NULL) 
	{ 
		num_fields=0; 
		return; 
	} 
	for(x=0,y=0,i=0;i<size;i++) 
	{ 
		if(background[i]=='\n') 
		{ 
			x=0; 
			y++; 
		} 
		else if(background[i]=='\t') 
			x=x+8-x%8; 
		else x++; 
		if(x>width) 
			width=x; 
	} 
	lines=y; 
	for(i=0;i<num_fields;i++) 
	{ 
		if(tags[i].des.x+tags[i].des.l>width) 
			width=tags[i].des.x+tags[i].des.l; 
		if(tags[i].des.y>lines) 
			lines=tags[i].des.y; 
	} 
	for(i=0;i<num_panel;i++)
	{ 
		if(!(panel[i].atr&TABL) && !(panel[i].atr&NO_SHADOW))
		{ 
			if(panel[i].x+panel[i].l+1>width)
				width=panel[i].x+panel[i].l+1;
			if(panel[i].y+panel[i].h+1>lines)
				lines=panel[i].y+panel[i].h+1;
		} 
	} 
} 
 
int FORM::if_mark_field(int i) 
{ 
	if(!f_num_mark_fields || f_num_mark_fields==NULL || !*f_num_mark_fields) 
		return(0); 
	for(int j=0;j<*f_num_mark_fields;j++) 
	{ 
		if(!slacmp((*f_mark_field)[j].des.sla,tags[i].des.sla)) 
		{ 
			return(j+1); 
		} 
	} 
	return(0); 
} 
void FORM::Free_Form() 
{ 
	if(background) 
		free(background); 
	if(panel)
		free(panel);
	int i;
	if(tags) 
	{ 
		for(i=0;i<num_fields;i++)
		{ 
			if(tags[i].str!=NULL) 
			{ 
				free(tags[i].str); 
				tags[i].str=NULL; 
			} 
			if(tags[i].mask!=NULL) 
			{ 
				free(tags[i].mask); 
				tags[i].str=NULL; 
			} 
			if(tags[i].name!=NULL) 
			{ 
				for(int j=i+1;j<num_fields;j++) 
				{ 
					if(tags[i].name==tags[j].name) 
						tags[j].name=NULL; 
				} 
				free(tags[i].name); 
				tags[i].name=NULL; 
			} 
		} 
		free(tags); 
	} 
	bzero(this,sizeof (FORM)); 
} 
 
FORM::~FORM() 
{ 
	Free_Form(); 
} 
 
void FORM::create_Manual(int num,struct tag *tgs,int num_d,struct panel *des)
{ 
	int i; 
 
	Free_Form(); 
 
	num_fields=num; 
	if(tags!=NULL) 
		free(tags); 
	tags=tgs; 
	size=0; 
	for(i=1;i<num;i++) 
		if(tags[i].des.y<tags[i-1].des.y) 
		{ 
			lines=i; 
			break; 
		} 
	lines=num; 
	num_panel=num_d;
	if(des!=NULL) 
	{ 
		panel=(struct panel *)realloc(panel,num_panel*sizeof (struct panel));
		bcopy(des,panel,num_panel*sizeof (struct panel));
	} 
	else    panel=NULL;
	bzero(&name_form,sizeof (name_form)); 
} 
static int if_space(unsigned char ch) 
{ 
	return(ch==' ' || (ch>=176 && ch<=178) || ch=='\t' || ch==196); 
} 
/* поиск и заполнение названий полей в бланке (tag'ов) */ 
void FORM::look_for_tag_names() 
{ 
	register int field; 
	int table=0; 
	int j=0; 
 
	for(field=0;field<num_fields;field++) 
	{ 
		int x,y,i,up=0; 
		register char *line,*ch; 
 
		if(!table) 
		{ 
			for(j=0;j<num_panel;j++)
			{ 
				if(panel[j].atr&TABL && (panel[j].y==tags[field].des.y))
				{ 
					table=1; 
					break; 
				} 
			} 
		} 
		if(tags[field].des.sla->n==-2) 
			continue; 
		if(table) 
			y=panel[j].y;
		else 
			y=tags[field].des.y; 
 
		x=tags[field].des.x; 
BEG: 
		if(y<0) 
			goto NO; 
		if(++up>2 && !table) 
			goto NO; 
		for(line=background,i=0;*line && i<y;i++,line++) 
			while(*line && *line!='\n') 
				line++; 
		if(*line=='\n') 
			line++; 
		if(!*line) 
			goto NO; 
		ch=line; 
		for(i=0;*ch && *ch!='\n';ch++) 
		{ 
			if(*ch=='\t') 
				i=i+8-i%8; 
			else    i++; 
			if(i>=x) 
				break; 
		} 
		if(!*ch) 
			goto NO; 
		if(look_str(field,(unsigned char *)ch,up)) 
		{       // перед полем ничего нет - надо искать вверху 
			if(up==1) 
				x+=tags[field].des.l; 
			y--; 
			goto BEG; 
		} 
		continue; 
NO: 
		tags[field].name=NULL; 
	} 
} 
/* поиск надписи слева от слота */ 
int FORM::look_str(int field,register unsigned char *ch,int atr) 
{ 
	int x=0,px=tags[field].des.x,bx; 
 
	if(field>1 && tags[field].des.y==tags[field-1].des.y) 
		x=tags[field-1].des.x+tags[field-1].des.l; 
	if(atr>1) 
	{ 
		px=tags[field].des.x+tags[field].des.l; 
		x=tags[field].des.x; 
	} 
	bx=px; 
	// пропустили пробелы 
	for(;px>x;ch--,px--) 
	{ 
		if(ch==(unsigned char *)background || *ch=='\n') 
			return(-1); 
		if(!if_space(*ch)) 
			break; 
	} 
	if((*ch>=179 && *ch<=223) || px==x) 
		return(-1);   // перед слотом псевдографика... 
	// сдвинулись до начала тэга 
	for(;;ch--,px--) 
	{ 
		register unsigned char *ch1; 
 
		if(ch==(unsigned char *)background) 
			return(-1); 
		ch1=ch-1; 
		if(px==x || (*ch1>=176 && *ch1<=223) || *ch1=='\n') 
			break; 
	} 
	while(if_space(*ch)) 
		ch++,px++; 
	x=bx-px+1; 
 
	while(x && if_space(ch[x-1])) 
		x--; 
 
	if(tags[field].name!=NULL) 
		free(tags[field].name); 
 
	tags[field].name=(char *)calloc(x+1,1); 
	bcopy(ch,tags[field].name,x); 
	return(0); 
} 
 
char *FORM::background_pos(int x,int y) 
{ 
	if(background==NULL) 
		return(NULL); 
	int x_pos=0,y_pos=0; 
	for(int i=0;background[i];i++) 
	{ 
		if(x==x_pos && y==y_pos) 
			return(background+i); 
		if(background[i]=='\n') 
		{ 
			x_pos=0; 
			y_pos++; 
		} 
		else if(background[i]=='\t') 
			x_pos=x_pos+8-x_pos%8; 
		else    x_pos++; 
	} 
	return(NULL); 
} 
int FORM::get_blank(char *base) 
{ 
	int i,x=0,y=0; 
	int cx3; 
	int len=strlen(background); 
	if(!len) 
		return(0); 
	num_fields=0; 
	if(base!=NULL && if_base("",base)<5) 
		cx3=1; 
	else cx3=0; 
	for(i=0,size=0;i<len && background[i];i++) 
	{ 
		if(background[i]=='\\' && background[i+1]=='^') 
			continue; 
		if(background[i]=='~' && (i==0 || background[i-1]=='\n')) 
		{ 
			if(num_panel==0)
			{ 
				if(panel!=NULL)
					free(panel);
				panel=(struct panel *)calloc(1,sizeof (struct panel));
				panel->y=y;
				panel->l=term->l_x();
				panel->atr=TABL|NO_SHADOW;
				num_panel=1;
			} 
			continue; 
		} 
		if(!strncmp(background+i,FMATR,strlen(FMATR))) 
		{ 
			i+=strlen(FMATR); 
			i+=get_atr(background+i,&form_atr); 
			if(background[i]=='\n') 
				i++; 
		} 
		else if(background[i]=='^' && (i==0 || background[i-1]!='\\')) 
		{ 
			char str[LINESIZE+1]; 
			int j=0; 
 
			tags=(struct tag *)realloc(tags,(++num_fields)*sizeof (struct tag)); 
			struct tag *tag=tags+num_fields-1; 
			bzero(tag,sizeof (struct tag)); 
 
			while(background[i] && background[i]!=(cx3?'[':'<') && background[i]!='_' && j<LINESIZE) 
				str[j++]=background[i++]; 
			str[j]=0; 
 
			tag->des.x=x; 
			tag->des.y=y; 
			if(atoi(str+1)) 
				str_to_sla(str,tag->des.sla); 
			else 
			{ 
				tag->name=(char *)realloc(tag->name,strlen(str)+1); 
				strcpy(tag->name,str); 
				tag->des.sla->n=-2; 
			} 
 
			if(background[i]==(cx3?'[':'<')) // есть шаблон ввода 
			{ 
				for(++i,j=0;background[i] && background[i]!=(cx3?']':'>') && background[i]!='\n' && j<LINESIZE;i++) 
				{ 
					str[j++]=background[i]; 
					background[size++]=' '; 
					x++; 
				} 
				if(j && j<LINESIZE && background[i]==(cx3?']':'>')) 
				{ 
					tag->mask=(char *)calloc(j,1); 
					bcopy(str,tag->mask,j); 
					i++; 
					goto END; 
				} 
			} 
 
			tag->mask=NULL; 
			for(j=0;background[i]=='_';x++,i++,j++) 
			{ 
				if(tag->des.sla[0].n) 
					background[size++]=' '; 
				else 
					background[size++]='_'; // ??? 
			} 
END: 
			tag->des.l=j; 
			i+=get_atr(background+i,&tag->des.atr); 
			i+=get_blank_color(background+i,&tag->color); 
		} 
		if(background[i]=='^') 
		{ 
			i--; 
			continue; 
		} 
		background[size++]=background[i]; 
		if(background[i]=='\n') 
		{ 
			x=0; 
			y++; 
		} 
		else if(background[i]=='\t') 
			x=x+8-x%8; 
		else 
			x++; 
	} 
	background=(char *)realloc(background,size+1); 
	background[size]=0; 

	look_for_tag_names(); 
	return(size); 
} 
 
static int get_atr(char *ch,long *atr) 
{ 
	int j=0; 
 
	if(*ch=='{') 
	{ 
		char str[128]; 
 
		bzero(str,8); 
		for(j=1;ch[j]!='}' && j<7;j++) 
			str[j-1]=ch[j]; 
		if(*str!='0') 
			*atr=atoi(str); 
		else if(str[1]=='x') 
			*atr=atox(str+2); 
		else 
			*atr=atoo(str+1); 
		return(j+1); 
	} 
	return(0); 
} 
static int get_blank_color(char *ch,struct color *color) 
{ 
	int j=0; 
 
	color->fg=0; 
	color->bg=0; 
	if(*ch=='(') 
	{ 
		char str[16]; 
 
		memset(str,0,16); 
		for(j=1;ch[j] && ch[j]!=')' && j<15;j++) 
			str[j-1]=ch[j]; 
		str_to_color(str,color); 
		return(j+1); 
	} 
	return(0); 
} 
static void str_to_color(char *str,struct color *color) 
{ 
	char *ch; 
 
	if((ch=strchr(str,','))!=NULL) 
	{ 
		*ch=0; 
 
		if(*str!='0') 
			color->bg=atoi(str); 
		else if(str[1]=='x') 
			color->bg=atox(str+2); 
		else 
			color->bg=atoo(str+1); 
 
		ch++; 
		if(*ch!='0') 
			color->fg=atoi(ch); 
		else if(ch[1]=='x') 
			color->fg=atox(ch+2); 
		else 
			color->fg=atoo(ch+1); 
	} 
	else 
	{ 
		int i; 
 
		if(*str!='0') 
			i=atoi(str); 
		else if(str[1]=='x') 
			i=atox(str+2); 
		else 
			i=atoo(str+1); 
		color->fg=i&0xf; 
		color->bg=(i>>4)&0xf; 
	} 
} 
