#include "StdAfx.h"
#include "CX_BASE.h"

int main()
{
	int i=0;
	struct field f;

	memset(&f,0,sizeof f);
	f.a=X_STRING;
	f.l=6;
	struct pool
	{
		long i, r;
	}
	pool[1000000];
	long t=time(0);
	int size=1000000;
	{
		BTree Tree("BUF.file",f);
		srand(10000);

		for(i=0;i<size;i++)
		{
			char str[32];
			memset(str,0,sizeof str);

			pool[i].i=rand()%size;
			pool[i].r=i+1;

			sprintf(str,"%06d",pool[i].i);
			if(f.a==X_TEXT)
				Tree.Insert(str,strlen(str),i+1);
			else if(f.a==X_STRING)
				Tree.Insert(str,f.l,i+1);
			else
				Tree.Insert((char *)&pool[i].i,f.l,i+1);
			if((i%10000)==0)
			{
				printf("\r%d",i);
				fflush(stdout);
			}
		}

		printf("\nbuild time =%ld\n",time(0)-t);
		t=time(0);
		for(i=0;i<size/3;i++)
		{
			int j=rand()%size;
			if(pool[j].r)
			{
				char str[32];
				sprintf(str,"%06d",pool[j].i);
				if(f.a==X_TEXT)
					Tree.Delete(str,strlen(str),pool[j].r);
				else if(f.a==X_STRING)
					Tree.Delete(str,f.l,pool[j].r);
				else
					Tree.Delete((char *)&pool[j].i,f.l,pool[j].r);
				pool[j].r=0;
			}
			if((i%10000)==0)
			{
				printf("\r%d",i);
				fflush(stdout);
			}
		}
		printf("\ndelete time =%d\n",time(0)-t);

		t=time(0);
		for(i=0;i<size/4;i++)
		{
			char str[32];
			memset(str,0,sizeof str);

			int j=rand()%size;
			if(pool[j].r)
				continue;
			pool[j].i=rand()%size;
			pool[j].r=j+1;

			sprintf(str,"%06d",pool[j].i);
			if(f.a==X_TEXT)
				Tree.Insert(str,strlen(str),j+1);
			else if(f.a==X_STRING)
				Tree.Insert(str,f.l,i+1);
			else
				Tree.Insert((char *)&pool[i].i,f.l,j+1);
			if((i%10000)==0)
			{
				printf("\r%d",i);
				fflush(stdout);
			}
		}
		printf("\ninsert time =%ld\n",time(0)-t);
		t=time(0);
		for(i=0;i<size;i++)
		{
			if(pool[i].r)
			{
				char str[32];
				sprintf(str,"%06d",pool[i].i);
				long page;

				if(f.a==X_TEXT)
					page=Tree.Find(str,strlen(str),pool[i].r);
				else if(f.a==X_STRING)
					page=Tree.Find(str,f.l,pool[i].r);
				else
					page=Tree.Find((char *)&pool[i].i,f.l,pool[i].r);
				if(page<0)
				{
					printf("Can't find %s record=%d page=%ld i=%d\n",str,pool[i].r,page,i);
					getchar();
					break;
				}
			}
			if((i%10000)==0)
			{
				printf("\r%d",i);
				fflush(stdout);
			}
		}
	}

	printf("find time =%ld\n",time(0)-t);

	exit(0);
}
