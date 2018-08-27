#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>

main(int argc,char **argv)
{
	FILE *in;
	RSA *rsa;
	int i,size;
	char str[256],buf[256];

	in=fopen("/etc/key.pem","r");
	if(in==NULL)
		goto END;
	rsa=(RSA *)PEM_read_RSAPrivateKey(in,NULL,NULL,NULL);
	if(rsa==NULL)
		goto END;
	fclose(in);

	for(size=1;size<argc;size++)
		buf[size-1]=atoi(argv[size]);
	size--;
	if((size=RSA_private_decrypt(size, (unsigned char *)buf, (unsigned char *)str, rsa, RSA_PKCS1_PADDING))<0)
		goto END;
	str[size]=0;
	printf("%s\n",str);
END:
	exit(0);
}
