#include "../CX_Browser.h"
#include <dirent.h>

struct record {long page; struct sla sla[6];};

void fill_SB(char *folder);
int Find_Pointers(CX_BASE *sb,CX_BASE *base,int num, struct sla *sla,struct record **record,int num_rec);

main(int argc, char **argv)
{
	DIR *fd;
	struct dirent *dp;
	char name[256];

	if(argc<2)
		getcwd(name,sizeof name);
	else
	{
		if(if_base(0,argv[1]))
		{
			getcwd(name,sizeof name);
			if(*argv[1]!='/')
			{
				char *folder=(char *)malloc(strlen(name)+strlen(argv[1])+2);
				sprintf(folder,"%s/%s",name,argv[1]);
				fill_SB(folder);
				free(folder);
			}
			else
				fill_SB(argv[1]);
			exit(0);
		}
		strcpy(name,argv[1]);
	}
	if((fd=opendir(name))==NULL)
		exit(1);
	while((dp=readdir(fd))!=NULL)
	{
		if(!if_base(name,dp->d_name))
			continue;
		char *folder=(char *)malloc(strlen(name)+strlen(dp->d_name)+2);
		sprintf(folder,"%s/%s",name,dp->d_name);
		fill_SB(folder);
		free(folder);
	}
	closedir(fd);
	exit(0);
}
void fill_SB(char *folder)
{
	CX_BASE *sb;
	try
	{
		sb=new CX_BASE("SuperBase");
	}
	catch(int i)
	{
		printf("Can't open SuperBase\n");
		exit(1);
	}
	CX_FIND *find=new CX_FIND(sb);
	long page=find->Find_First(1,folder,0);
	if(page<=0)
	{
		page=sb->New_Record();
		sb->Put_Slot(page,1,folder);
		sb->Unlock(page);
	}
	CX_BASE *base;
	try
	{
		base=new CX_BASE(folder);
	}
	catch(int i)
	{
		printf("Can't open %s\n",folder);
		return;
	}
	struct record *record=NULL;
	int num_records;
	struct sla sla[6];
	bzero(sla,sizeof sla);
	num_records=Find_Pointers(sb,base,0,sla,&record,0);

	int len;
	char *ch=(char *)malloc(len=num_records*sizeof(struct record)+sizeof (int));
	bcopy(&len,ch,sizeof (int));
	bcopy(record,ch+sizeof (int),len-sizeof(int));
	sb->Write(page,2,ch);
	free(ch);
	free(record);
	delete(sb);
}
int Find_Pointers(CX_BASE *sb,CX_BASE *base,int num, struct sla *SLA,struct record **record,int num_records)
{
	struct sla sla[6];

	bzero(sla,sizeof sla);
	for(int i=0;SLA[i].n;i++)
		sla[i].n=SLA[i].n;
	for(sla[num].n=1;sla[num].n<=base->Num_Fields();sla[num].n++)
	{
		char str[256];
		if(base->Field_Descr(sla)->a==POINTER)
		{
			char *ch;
			CX_FIND find(sb);

			*record=(struct record *)realloc(*record,(num_records+1)*sizeof (struct record));
			bzero(*record+num_records,sizeof (struct record));
			ch=base->Name_Subbase(sla);
			if(*ch!='/')
			{
				getcwd(str,sizeof str);
				strcat(str,"/");
				strcat(str,ch);
				ch=str;
			}
			long r_page=find.Find_First(1,ch,0);
			if(r_page<=0)
			{
				char *ns=(char *)malloc(strlen(ch)+1);
				strcpy(ns,ch);
				fill_SB(ns);
				free(ns);
				sb->update();
				r_page=find.Find_First(1,ch,0);
			}
			(*record)[num_records].page=r_page;
			bcopy(sla,(*record)[num_records].sla,(num+1)*sizeof(struct sla));
			for(int i=0;i<=num;i++)
				(*record)[num_records].sla[i].n++;
			num_records++;
		}
		else if(base->Field_Descr(sla)->a==STRUCTURE)
		{
			num_records=Find_Pointers(sb,base,num+1,sla,record,num_records);
		}
	}
	sla[num].n=0;
	return(num_records);
}
