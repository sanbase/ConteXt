/*
		      Defined variables:

#define OUTPUT  base->share->output     // output message
#define BG      base->share->color.bg   // background color
#define FG      base->share->color.fg   // foreground color
#define RECORD  base->share->record     // current Object number
#define RETURN  base->share->ret        // returned value
#define TAG     base->share->slot       // descripton of the current slot
#define LEN     base->share->slot.l     // length of the current slot
#define SLA     base->share->slot.sla   // structute of the current slot
#define CMD     base->share->cmd        // returned command
#define BLANK   base->share->form.blank // current form/blank name
*/

#include <CX_Methods.h>
#include <dirent.h>

CX_BASE *db=NULL;
static int num_links;
static char **links;

static void Fill_Links(struct st *s)
{
	for(int i=0;i<s->ptm;i++)
	{
		if(s->field[i].a==X_POINTER)
		{
			links=(char **)realloc(links,(++num_links)*sizeof (char *));
			links[num_links-1]=s->field[i].name;
		}
		if(s->field[i].a==X_STRUCTURE)
			Fill_Links(s->field[i].st.st);
	}
}

static void center(char *out,char *ch,int len)
{
	char form[32];

	sprintf(form,"%% %ds",(strlen(ch)+len)/2);
	sprintf(out,form,ch);

}
void Methods::Virtual(long record,struct sla *slot)     // request of virtual field
{
	char str[LINESIZE];
	int i;

	if(db==NULL && *base->share->io)
	{
		try
		{
			db=new CX_BASE(base->share->io);
			num_links=0;
			Fill_Links(&db->ss);
		}
		catch(...)
		{
		}
	}
	if(db==NULL && slot->n>200)
		return;
	FNT=1;
	FATR=2;
	BG=016;
	switch(slot->n)
	{
		case 100:
		{
			char *ch;
			if(!*base->share->io)
				ch="Select ConteXt File";
			else
			{
				if((ch=strrchr(base->share->io,'/'))==NULL)
					ch=base->share->io;
				else    ch++;
			}
			center(OUTPUT,ch,LEN);
			break;
		}
		case 200:
			if(db==NULL)
				center(OUTPUT,"Create",LEN);
			else
				center(OUTPUT,"Edit",LEN);
			break;
		case 201:
			center(OUTPUT,"Browse",LEN);
			break;
		case 202:
			center(OUTPUT,"Repair",LEN);
			break;
		case 203:
			sprintf(str,"%s/Methods.cc",db->Name_Base());
			if(access(str,W_OK))
				center(OUTPUT,"Create",LEN);
			else
				center(OUTPUT,"Edit",LEN);
			break;
		case 204:
			sprintf(str,"%s/%s",db->Name_Base(),FORMDB);
			try
			{
				CX_BASE *forms=new CX_BASE(str);
				if(forms->Max_Record()>0)
					center(OUTPUT,"Edit",LEN);
				else
					center(OUTPUT,"Create",LEN);
				delete forms;
			}
			catch(...)
			{
				center(OUTPUT,"Create",LEN);
			}

			break;
		case 205:
			center(OUTPUT,"Create",LEN);
			break;
		case 206:
			center(OUTPUT,"Blank to Form",LEN);
			break;
		case 207:
			sprintf(str,"%s/%s",db->Name_Base(),FORMDB);
			try
			{
				CX_BASE *forms=new CX_BASE(str);
				if(forms->Max_Record()>0)
					center(OUTPUT,"Form to Blank",LEN);
				delete forms;
			}
			catch(...)
			{
			}

			break;
		case 208:
			sprintf(str,"%s/Methods.cc",db->Name_Base());
			if(!access(str,R_OK))
				center(OUTPUT,"Compile",LEN);
			break;
		case 209:
			sprintf(str,"%s/%s",db->Name_Base(),FORMDB);
			try
			{
				CX_BASE *forms=new CX_BASE(str);
				for(i=0,record=1;record<=forms->Max_Record();record++)
				{
					if(forms->Check_Del(record))
						continue;
					i++;
				}
				sprintf(str,"%d",i);
				center(OUTPUT,str,LEN);
				delete forms;
			}
			catch(...)
			{
			}
			break;
		case 210:
			{
				char name[256];
				DIR *fd;
				struct dirent *dp;

				FATR=0;
				sprintf(name,"%s/%s",db->Name_Base(),BLANKDIR);
				if((fd=opendir(name))==NULL)
				{
					strcpy(OUTPUT,"-");
					break;
				}
				i=0;
				while((dp=readdir(fd))!=NULL)
				{
					if(*dp->d_name=='.')
						continue;
					int len=strlen(dp->d_name);
					if(len>2 && dp->d_name[len-1]=='b' && dp->d_name[len-2]=='.')
						continue;
					if(!if_read(name,dp->d_name))
						continue;
					i++;
				}
				closedir(fd);
				sprintf(name,"%d",i);
				center(OUTPUT,name,LEN);
			}
			break;
		case 300:
			sprintf(OUTPUT,"%d",db->Max_Record());
			break;
		case 301:
			sprintf(OUTPUT,"%d",db->Num_Fields());
			break;
		case 302:
			sprintf(OUTPUT,"%d",db->Total_Num_Fields());
			break;
		case 303:
		{
			size_t size;

			size=db->Size();
			if(size>1024*1024*100)
				sprintf(OUTPUT,"%d MB",size/(1024*1024));
			else if(size>1024*100)
				sprintf(OUTPUT,"%d KB",size/1024);
			else
				sprintf(OUTPUT,"%d",size);
			break;
		}
		case 400:
		{
			FNT=0;
			FATR=0;
			if(SLA->m==-1)
			{
				sprintf(OUTPUT,"%d",num_links-1);
				break;
			}
			if(SLA->m<=num_links)
			{
				switch(SLA[1].n)
				{
					case 0:
						sprintf(OUTPUT,"%d",SLA->m);
						break;
					case 1:
						sprintf(OUTPUT,"%s",links[SLA->m-1]);
						break;
					case 2:
					case 4:
					case 5:
					{
						try
						{
							CX_BASE *sb=new CX_BASE(links[SLA->m-1]);
							if(SLA[1].n==2)
								sprintf(OUTPUT,"%d",sb->Max_Record());
							else if(SLA[1].n==4)
								sprintf(OUTPUT,"%d",sb->Num_Fields());
							else if(SLA[1].n==5)
								sprintf(OUTPUT,"%d",sb->Total_Num_Fields());

							delete sb;
						}
						catch(...)
						{
							strcpy(OUTPUT,"-");
						}
						break;
					}
					case 3:
					{
						try
						{
							size_t size;
							CX_BASE *sb=new CX_BASE(links[SLA->m-1]);

							size=sb->Size();
							if(size>1024*1024*100)
								sprintf(OUTPUT,"%d MB",size/(1024*1024));
							else if(size>1024*100)
								sprintf(OUTPUT,"%d KB",size/1024);
							else
								sprintf(OUTPUT,"%d",size);
							delete sb;
						}
						catch(...)
						{
							strcpy(OUTPUT,"-");
						}
						break;
					}
				}
			}
			break;
		}
		case 401:
			sprintf(OUTPUT,"%d",num_links);
		default:
			break;
	}
}

// abort of the process
void Methods::Abort()
{
	if(db!=NULL)
		delete db;
}

/* the next methods are using only by DB-browser.  */

// user's action.
int Methods::Action(int act)
{
	char str[256];

	if(act!='\r')
		return(1);
	if(db==NULL && SLA->n>200)
		return(0);
	switch(SLA->n)
	{
		case 100:
			strcpy(OUTPUT,"-CX -D.");
			return(c_Choise);
		case 200:
			if(db==NULL)
				strcpy(OUTPUT,"Create_DB");
			else
				sprintf(OUTPUT,"CX_Edit %s",db->Name_Base());
			return(c_Shell);
		case 201:
			sprintf(OUTPUT,"cx5 -N%s",db->Name_Base());
			return(c_Shell);
		case 202:
			sprintf(OUTPUT,"CX_repair -N%s",db->Name_Base());
			return(c_Shell);
		case 203:

			sprintf(str,"%s/Methods.cc",db->Name_Base());
			if(access(str,W_OK))
				fcopy(str,"/usr/local/lib/Methods.cc");
			sprintf(OUTPUT,"ned %s",str);
			return(c_Shell);
		case 204:
			sprintf(OUTPUT,"Create_Form %s",db->Name_Base());
			return(c_Shell);
		case 206:
			sprintf(OUTPUT,"Blank_to_Form %s",db->Name_Base());
			return(c_Shell);
		case 207:
			sprintf(OUTPUT,"Form_to_Blank %s",db->Name_Base());
			return(c_Shell);
		case 208:
			sprintf(str,"%s/Methods.cc",db->Name_Base());
			if(!access(str,R_OK))
			{
				sprintf(OUTPUT,"g++ -fhandle-exceptions -I/usr/local/include %s -L/usr/local/lib -lcx -lcxform -lcxdb -o %s/Methods",str,db->Name_Base());
				return(c_Shell);
			}
			break;
		case 400:
			if(SLA->m<=num_links)
			{
				sprintf(OUTPUT,"-N/usr/local/lib/CX_Adm -I%s",links[SLA->m-1]);
				return(c_RecForm);
			}
			break;
	}
	return(0);
}

// attemption to write the string (str) to the current slot
int Methods::Write_Slot(char *str)
{
	if(SLA->n==100)
	{
		if(db!=NULL)
			delete db;
		db=NULL;
		strcpy(base->share->io,str);
	}
	return(0);
}

// attemption to write current Object to the File
int Methods::Write_Cadr()
{
	return(0);
}
