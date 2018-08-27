/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:network.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#ifndef WIN32 
#include <sys/socket.h> 
#include <sys/select.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <sys/uio.h> 
#include <assert.h> 
#include <netdb.h> 
#include <errno.h> 
 
extern int errno; 
char *local_host_name(char *l_hostname) 
{ 
	if(gethostname(l_hostname,sizeof(l_hostname)) == 0) 
		return l_hostname; 
	else 
		return NULL; 
} 
 
struct in_addr local_host_addr() 
{ 
	struct in_addr local_addr; 
	local_addr.s_addr = htonl(INADDR_ANY); 
	char hostname[256]; 
	if(local_host_name(hostname)==NULL) 
		return local_addr; 
 
	struct hostent *h; 
	h = gethostbyname(hostname); 
	if(h==NULL) return local_addr ; 
	struct in_addr *ptr = (struct in_addr *) *h->h_addr_list; 
	local_addr = *ptr; 
	return local_addr; 
} 
 
char *host_name(in_addr hostaddr,char *h_hostname) 
{ 
	struct hostent *he = gethostbyaddr((char *)&hostaddr, sizeof(in_addr),AF_INET); 
	strcpy(h_hostname, he ? he->h_name :  inet_ntoa(hostaddr));
	return h_hostname; 
} 
 
char *host_name(int socket,char *hostname) 
{ 
	struct in_addr sock_in_addr; 
	struct sockaddr sa; 
	socklen_t sa_len = sizeof(sa); 
	if(getpeername(socket, &sa, &sa_len) < 0) 
		return ""; 
	sock_in_addr.s_addr = ((struct sockaddr_in *)&sa)->sin_addr.s_addr; 
	return(host_name(sock_in_addr,hostname)); 
} 
 
struct in_addr peer_inaddr(int socket) 
{ 
	struct in_addr socket_in_addr; 
	socket_in_addr.s_addr = INADDR_ANY; 
	struct sockaddr sa; 
	socklen_t sa_len = sizeof(sa); 
	if(getpeername(socket, &sa, &sa_len) == 0) 
		socket_in_addr.s_addr = ((struct sockaddr_in *)&sa)->sin_addr.s_addr; 
	return socket_in_addr; 
} 
 
char *peer_dotname(int socket) 
{ 
	return(inet_ntoa(peer_inaddr(socket))); 
} 
#endif 
