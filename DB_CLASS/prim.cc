#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif
#define BLOCK_SIZE 4096
#define POOL_SIZE 10
int cmp(const void *a,const void *b)
{
	return(*(long *)a-*(long *)b);
}
main()
{
	int fd=open("BUF.file",O_RDWR|O_BINARY);
	long t=time(0);
	struct stat st;
	long buf[POOL_SIZE];
	int pool=0;
	fstat(fd,&st);

	srand(0);

	int max=(st.st_size/BLOCK_SIZE);
	for(int i=0;i<10000;i++)
	{
		char str1[BLOCK_SIZE];

		int pos1=rand()%max;
		lseek(fd,pos1*sizeof str1,SEEK_SET);
		read(fd,str1,sizeof str1);
		buf[pool++]=pos1;
		if(pool==POOL_SIZE)
		{
			qsort(buf,POOL_SIZE,sizeof (long),cmp);
			for(int j=0;j<POOL_SIZE;j++)
			{
				lseek(fd,buf[j]*sizeof str1,SEEK_SET);
				write(fd,str1,sizeof str1);
				printf("%d\n",buf[j]);
			}
			getchar();
			pool=0;
		}

		if((i%1000)==0)
		{
			printf("\r%d",i);
			fflush(stdout);
		}
	}
	printf("\ntime =%d\n",time(0)-t);
	return(0);
}
