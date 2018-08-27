#include "DB_CLASS/CX_BASE.h"
#include "SCREEN/screen.h"
Terminal *term;

int main(int argc, char **argv)
{
	CX_BASE *db_in;
	char str[256];

	if(argc<2)
	{
		printf("Usage: 5to6 [dir] cx5db\n");
		exit(1);
	}
	try
	{
		db_in = new CX_BASE(argv[argc-1]);
	}
	catch(...)
	{
		printf("\nThe file: \"%s\" is not a valid CX class\n",argv[argc-1]);
		exit(-1);
	}
	if(argc==2)
	{
		sprintf(str,"%s_cx6",argv[1]);
	}
	else
	{
		if(access(argv[1],X_OK))
			mkdir(argv[1],0775);
		sprintf(str,"%s/%s",argv[1],argv[argc-1]);
	}
	char cmd[256];
	mkdir(str,0777);
	sprintf(cmd,"(cd %s && tar -cpf - * .nu*)|(cd %s && tar -xpf -) 2>/dev/null",argv[argc-1],str);
//        sprintf(cmd,"(cd %s && tar -cpf - _Forms _HyperForm _Flex)|(cd %s && tar -xpf -)",argv[argc-1],str);
	system(cmd);

	struct st *ss=db_in->type();
	create_class(ss,str,0);
	delete db_in;


	struct st space;
 
	bzero(&space,sizeof (struct st));
	space.ptm=2;
	space.size=8;
	space.field=(struct field *)calloc(2,sizeof (struct field));
	space.field[0].a=X_INTEGER;
	space.field[0].l=4;
	space.field[1].a=X_INTEGER;
	space.field[1].l=8;

	char DBNAME[256];
	sprintf(DBNAME,"%s/%s",str,SPACEDB);

	create_class(&space,DBNAME,1);

	exit(0);
}
