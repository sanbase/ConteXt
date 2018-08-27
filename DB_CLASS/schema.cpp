/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:schema.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
static struct sla sla[SLA_DEEP]; 
 
struct item field_type[]= 
{ 
	{"STRING     ", 010,016,0 },
	{"DATE       ", 010, 03,0 },
	{"TIME       ", 010, 03,0 },
	{"POINTER    ",  01,016,0 },
	{"VARIANT    ",  01,017,0 },
	{"INTEGER    ", 010,012,0 },
	{"UNSIGNED   ", 010,012,0 },
	{"FLOAT      ", 010,012,0 },
	{"DOUBLE     ", 010,012,0 },
	{"STRUCTURE  ",  03,012,0 },
	{"TEXT       ", 010,013,0 },
	{"BINARY     ", 010,014,0 },
	{"FILENAME   ", 010,017,0 },
	{"EXPRESSION ", 010,015,0 },
	{"IMAGE      ", 010,04 ,0 },
	{"COMPLEX    ", 010,013,0 },
	{"USERDEFINED",  07,  0,0 }, 
	{"",         0,  0, 0 } 
}; 
 
int size_field_types=sizeof field_type; 
 
int Schema_to_Text(struct st *struc,char *&buf,int level,int xml,int blank)
{ 
	int i,j,len; 
	char ch[1024],sla_str[64];
 
	if(struc==NULL) 
		return(-1); 
	if(buf==NULL) 
		buf=(char *)calloc(1,1); 
	len=strlen(buf); 
 
	for(i=0;i<struc->ptm;i++) 
	{ 
		char *name=struc->field[i].name; 
		if(name==NULL)
			name="";
 
		strcpy(ch,"\n"); 
		sla[level].n=i+1; 
		sla_to_str(sla,sla_str); 
		for(j=0;j<level;j++) 
			strcat(ch,"    "); 
		buf=(char *)realloc(buf,len+=strlen(ch)+1); 
		strcat(buf,ch); 
		if(xml) 
		{ 
			if(struc->field[i].a==X_POINTER) 
			{ 
				char *c=strchr(name,'\n'); 
				if(c!=NULL) 
					name=c+1; 
				else    name=NULL; 
			} 
			if(name!=NULL && *name) 
				sprintf(ch,"&lt;%s&gt;",pc_demos(name)); 
			else 
				sprintf(ch,"&lt;%s&gt;",sla_str); 
			buf=(char *)realloc(buf,len+=strlen(ch)+1); 
			strcat(buf,ch); 
			*ch=0; 
		} 
		char line[64]; 
		int n=struc->field[i].a-1; 
		if(n<0 || n>=(int)((sizeof field_type)/sizeof (struct item))) 
			n=((sizeof field_type)/sizeof (struct item))-1; 
 
		if(field_type[n].name!=NULL)
		{
			strcpy(line,field_type[n].name);
			for(j=0;line[j];j++)
			{
				if(line[j]==' ')
				{
					line[j]=0;
					break;
				}
			}
		}
		else    *line=0;
		switch(struc->field[i].a) 
		{ 
			case X_STRING: 
			case X_TIME: 
			case X_POINTER: 
			case X_VARIANT: 
				sprintf(ch,"%s[%d]",line,struc->field[i].l); 
				break; 
			case X_INTEGER: 
			case X_UNSIGNED: 
			case X_FLOAT: 
			case X_DOUBLE: 
				sprintf(ch,"%s[%d.%d]",line,struc->field[i].l,struc->field[i].n); 
				break; 
			default: 
				if(struc->field[i].a<=X_COMPLEX) 
					sprintf(ch,"%s",line); 
				else    sprintf(ch,"USERDEFINED=%d",struc->field[i].a); 
				break; 
		} 
		while(ch[strlen(ch)-1]==' ') 
			ch[strlen(ch)-1]=0; 
		buf=(char *)realloc(buf,len+=strlen(ch)+1); 
		strcat(buf,ch); 
		*ch=0; 
		if(struc->field[i].m) 
		{ 
			sprintf(ch," [%s] ",struc->field[i].m==MULTISET?"ARRAY":struc->field[i].m==SET?"SET":"LIST"); 
			buf=(char *)realloc(buf,len+=strlen(ch)+1); 
			strcat(buf,ch); 
			*ch=0; 
		} 
		if(struc->field[i].a==X_STRUCTURE) 
		{ 
			if(!xml) 
			{ 
				if(blank)
				{
					if(struc->field[i].name!=NULL && *struc->field[i].name)
						sprintf(ch," \"%s\" %s",pc_demos(struc->field[i].name),struc->field[i].d?" >>>>":"");
					else
						sprintf(ch," %s",struc->field[i].d?" >>>>":"");
				}
				else
				{
					if(struc->field[i].name!=NULL && *struc->field[i].name)
						sprintf(ch," \"%s\" %s (%s)\n",pc_demos(struc->field[i].name),struc->field[i].d?" >>>>":"",sla_str);
					else
						sprintf(ch," %s \"%s\"\n",struc->field[i].d?" >>>>":"",sla_str);
				}
				for(j=0;j<level;j++) 
					strcat(ch,"    "); 
				if(!blank)
					strcat(ch,"{");
			} 
			buf=(char *)realloc(buf,len+=strlen(ch)+1); 
			strcat(buf,ch); 
 
			Schema_to_Text(struc->field[i].st.st,buf,level+1,xml,blank);
			len=strlen(buf);
			if(!blank)
			{
				strcpy(ch,"\n");
				for(j=0;j<level;j++)
					strcat(ch,"    ");
				if(!xml)
					strcat(ch,"}");
			}
			else    *ch=0;
			sla[level+1].n=0; 
		} 
		else 
		{ 
			char name[256]; 
			*ch=0;
			if(struc->field[i].name!=NULL && *struc->field[i].name)
				strcpy(name,struc->field[i].name); 
			else
			{
				if(blank)
					*name=0;
				else
					sprintf(name,"^%d",i+1);
			}
			if(struc->field[i].a==X_POINTER) 
			{ 
				char *c=strchr(name,'\n'); 
				if(c!=NULL) 
					*c=0; 
				sprintf(ch,"->\"%s\"",pc_demos(name));
				if(c!=NULL) 
					strcpy(name,c+1); 
				else    *name=0; 
				buf=(char *)realloc(buf,len+=strlen(ch)+1); 
				strcat(buf,ch); 
				*ch=0; 
			} 
			if(*name) 
			{ 
				if(!xml) 
					sprintf(ch," \"%s\" ",pc_demos(name)); 
			} 
			if(struc->field[i].d) 
				strcat(ch," >>>>"); 
		} 
		buf=(char *)realloc(buf,len+=strlen(ch)+1); 
		strcat(buf,ch); 
		*ch=0; 
		if(xml) 
		{ 
			if(name!=NULL && *name) 
				sprintf(ch,"&lt;/%s&gt",pc_demos(name)); 
			else 
				sprintf(ch,"&lt;/%s&gt;",sla_str); 
		} 
		else if(struc->field[i].a!=X_STRUCTURE) 
		{
			if(blank)
				sprintf(ch,"\t%s________________________________",sla_str);
			else
				sprintf(ch," (%s)",sla_str);
		}
		buf=(char *)realloc(buf,len+=strlen(ch)+1); 
		strcat(buf,ch); 
	} 
	return(0); 
} 
 
int Text_to_Schema(char *buf,struct st *&ss,int i,int *len) 
{ 
	char *ch,*line=NULL; 
	int j; 
	if(ss==NULL) 
		ss=(struct st *)calloc(1,sizeof (struct st)); 
BEGIN: 
	int break_flg=0; 
	line=(char *)realloc(line,1); 
	*line=0; 
	for(j=0;buf[i];i++) 
	{ 
		if(buf[i]=='\n') 
		{ 
			i++; 
			break; 
		} 
		line=(char *)realloc(line,j+1); 
		line[j++]=buf[i]; 
	} 
	line[j]=0; 
	if(strchr(line,'}')!=NULL) 
		break_flg=1; 
	for(j=0;*field_type[j].name;j++) 
	{ 
		char type[64]; 
		strcpy(type,field_type[j].name); 
		if((ch=strchr(type,' '))!=NULL) 
			*ch=0; 
		if((ch=strstr(line,type))!=NULL) 
		{ 
			int field=ss->ptm; 
			ss->field=(struct field *)realloc(ss->field,(++ss->ptm+1)*sizeof (struct field)); 
			bzero(ss->field+field,sizeof (struct field)); 
			ss->field[field].a=j+1; 
			int len_field=4; 
			char *l=strchr(ch,'['); 
			if(l==NULL) 
			{ 
				if(j+1==X_DATE || j+1==X_TIME) 
					len_field=3; 
				if(j+1==X_DOUBLE) 
					len_field=8; 
				if(j+1==X_TEXT || j+1==X_BINARY || j+1==X_IMAGE || ss->field[field].m)
					len_field=8;
			} 
			else 
			{ 
				len_field=atoi(l+1); 
				if((l=strchr(l,'.'))!=NULL) 
					ss->field[field].n=atoi(l+1); 
			} 
			if(strstr(ch,"[ARRAY]")!=NULL) 
				ss->field[field].m=MULTISET; 
			if(strstr(ch,"[SET]")!=NULL) 
				ss->field[field].m=SET; 
			if(strstr(ch,"[LIST]")!=NULL) 
				ss->field[field].m=LIST; 
			if(strstr(ch," >>>>")!=NULL) 
				ss->field[field].d=1; 
			if((l=strchr(ch,'\"'))!=NULL) 
			{ 
				l++; 
				char *e=strchr(l,'\"'); 
				if(e!=NULL) 
					*e=0; 
				if(*l!='^') 
				{ 
					ss->field[field].name=(char *)malloc(strlen(l)+1); 
					strcpy(ss->field[field].name,l); 
				} 
			} 
			if(j+1==X_STRUCTURE) 
			{ 
				while(buf[i]!='{') 
					i++; 
				len_field=0; 
				i=Text_to_Schema(buf,ss->field[field].st.st,i,&len_field); 
			} 
			ss->field[field].l=len_field; 
			(*len)+=len_field; 
			break; 
		} 
	} 
	if(buf[i] && !break_flg) 
		goto BEGIN; 
	return(i); 
} 
