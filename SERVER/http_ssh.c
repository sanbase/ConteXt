#include "httpd.h"

void ssh_node( char *url, char *args, int in, FILE *out)
{
	int pid,i;
	if((pid=fork())==0)
	{
		execl("/usr/local/bin/sshd","sshd","-i",0);
	}
	else
	{
		waitpid(pid,&i,0);
	}
}
