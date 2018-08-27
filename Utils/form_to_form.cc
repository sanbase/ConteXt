#include "../CX_Browser.h"

main(int argc,char **argv)
{
	CX_BASE *form;

	if(argc!=2)
	{
		printf("usage: %s database_name\n",argv[0]);
		exit(1);
	}
	char *name=(char *)malloc(strlen(argv[1])+strlen(FORMDB)+2);
	sprintf(name,"%s/%s",argv[1],FORMDB);
	try
	{
		form = new CX_BASE(name);
	}
	catch(int i)
	{
		printf("Cant't open %s",name);
		exit(0);
	}
	struct tag_descriptor *td=NULL;
	for(int page=1;page<=form->last_cadr();page++)
	{
		int num_fields=form->Read(page,3,(char **)&td).len/sizeof (struct tag_descriptor);
		for(int field=0;field<num_fields;field++)
		{
			struct old { int n; int m; } old[5];

			bzero(old,sizeof old);
			bcopy(td[field].sla,old,sizeof old);
			bzero(td[field].sla,sizeof td->sla);
			for(int i=0;i<5;i++)
			{
				td[field].sla[i].n=old[i].n;
				td[field].sla[i].m=old[i].m;
			}
		}
		long len=num_fields*sizeof (struct tag_descriptor)+sizeof (long);
		char *buf1=(char *)calloc(len,1);
		memcpy(buf1,&len,sizeof (long));
		memcpy(buf1+sizeof (long),td,len-sizeof (long));
		form->Write(page,3,buf1);
		free(buf1);
	}
	exit(0);
}
