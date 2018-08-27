
#include "ConteXt.h"
#include "Ttree.h"
 
#ifndef _CX_BASE_H 
#define _CX_BASE_H 
 
#ifdef WIN32
#include <process.h>
/*
#ifdef CONTEXT_EXPORTS
#define CONTEXT_API __declspec(dllexport)
#else
#define CONTEXT_API __declspec(dllimport)
#endif
#else
#define CONTEXT_API
*/
#endif 
 
class CX_BROWSER; 
class Transaction; 
class Methods; 
 
#define MAX_OPEN_BASE 256
#define MAXPATHLEN    512
 
struct rq 
{ 
	double a;       // double value
	char *cmd; 
	int ttip;       // field.a
	int emp; 
	char *obr; 
	int no_sel; 
	int no; 
	int short_flex; 
}; 
 
struct mem_buf 
{ 
	off_t seek;     // position in Bank.0 
	off_t shift;    // position in Main 
	int   bank;     // Bank number 
	int   len;      // size of buffer 
	char  *ch;      // data 
}; 
 
class Mem_Buf 
{ 
	friend class CX_BASE; 
private: 
	struct mem_buf *e; 
	int num; 
public: 
	Mem_Buf(); 
	~Mem_Buf(); 
	void  Free(int bank,off_t seek0, int len); 
	char *Get(int bank, off_t seek0, int len); 
	int  Open(int bank, off_t seek0, int len, char *ch); 
	int  Put (int bank, off_t seek0, int len, char *ch); 
	int length(); 
}; 
 
// cx_cond 
#define SORT     0x1      // selection was done 
 
class selection 
{ 
public: 
	long num_index; 
	long *index; 
	long Index(int i); 
	long length();
	void clean();
	int Add(long i);
	selection(selection *sel); 
	selection(); 
	~selection(); 
}; 
 
struct phdr
{
	long fdes;      // it's first 4 bytes of the field struct
	long page;      // number of page
	struct key KEY; // left and right pointers (the same level)
	long space;     // free space
	long last;      // number of fields
	long left;      // left pointer (down)
	long attr;      // it is last page in the first page
	char data[1];
};

union Phead
{
	struct field f;
	struct phdr  h;
};

class BPage
{
public:
	union Phead hdr;

	int insert(char *slot,int len,long record,int i,long page=0);
	int del(char *slot,int length,long record);
	int del(int i);
	int find_pos(char *pattern,int length,long record,int *i);
	int find_first(char *pattern);
	int get_pos(long record);
	char *pos(register int pos);
	int cmp(register char *pattern,int p_len,register char *slot,int s_len,long record);
	int len(int pos);
	int split(BPage *tmp);
};

struct Pool
{
	int  status;
	char buf[4096];
};

#define POOL_SIZE 4

class BTree
{
	friend class CX_BASE;
private:
	Pool pool[POOL_SIZE];
	Pool *pool_pages[POOL_SIZE];

	long path[32];
	int level;
	int rd_lock;
	int wr_lock;

	int  Insert(char *slot,long record,long page, int index,long point=0);
	int  Delete(char *slot,long record,long page,int i);
	BPage *New_Page();
	BPage *Read_Page(int page);
	int Lock(int type);
	void Unlock(int type);
	int  Write_Page(BPage *bp);
	long Root_Page();
	long Find_Page(char *key,long record,int *i,int arg=0);
	long Find_First(char *key);
	int  Flush(BPage *bp);
	void  Flush();
	int Find_Left();
	int Next(int page);
public:
	struct field f;
	int field;
	CX_BASE *db;
	int fd;

	BTree(CX_BASE *db,char *name,struct field f,int field);
	~BTree();
	int  Insert(char *slot,long record);
	int Delete(char *slot,long record);
	long Find(char *key,long record);
};

class CX_BASE 
{ 
	friend class CX_BROWSER; 
	friend class Transaction; 
	friend class RAM_BASE; 
	friend class CX_REPAIR; 
	friend class CXDB_REPAIR; 
	friend class CX_RESTORE; 
	friend class RCX_BASE; 
	friend class CX3; 
	friend class Methods; 
	friend class CX_FIND; 
	friend class Query; 
	friend class TTree;
	friend class BTree;
private: 
	char *__name;           // name of the class
	struct header *context; // header of the class
	struct st ss;           // structure of class
	long max_record;        // number of objects 
	int fdnum;              // number of descriptors 
	struct fd *fd;          // descriptors list 
	struct f_lock lock;     // lock structure
	int rdonly;             // read only 
	char  *hist;            // support of History (or NULL) 
	Transaction *ts;        // current transaction 
	BTree **btree;          // array of BTree
	int num_btree;          // number of BTree
 
	char idx_name[NAMESIZE];// name of the index file 
	long max_index;         // number of objects in the current selection 
	long *index_buf;        // index buffer 
	long index_seek;        // shift in the index buffer 
	int index_level;        // level of recursion (for browser only) 
	int  fd_index;          // index file descriptor 
	int  insert;            // selection level 
	int num_keys;           // number of key fields
	int *key_field;         // key fields

	int encoded;            // __name has been encoded
	CX_BASE *change_db;     // change database
	char int_delimiter;     // delimiter not int.int

#ifndef SERVER 
	CX_BASE **open_base; 
	int num_open_bases; 

#endif 
	char *cadr;             // working buffer (cadr) 
	int len_cadr;           // size of cadr 
	int len_record;         // size of object 
	char sh_name[NAMESIZE]; // shm indentificator
	int owner;              // shm owner 
#ifdef WIN32
	Methods *method;
	int Ask_Method();
	HANDLE shmid;           // shared memory descriptor
#else
	int shmid;              // shared memory descriptor
#endif
	pid_t p_method;         // pid of Methods process 
	int method_io[2];       // method pipe 
	int root_size;          // size of root record 
	int PAGESIZE;           // size of page in storage
 
	unsigned long key39;    // version V.3.9 
	int no_superblock;      // do not use superblock 
	Mem_Buf *var_point;     // will be used in version V.5.7 
	int in_memory;
 
	int Get_Virtual(long, int, char *&); 
	int Get_Virtual(long,struct sla *,char *&); 
	char root_dir[MAXPATHLEN+1];
 
	CX_BASE *get_subbase(struct sla *); 
	CX_BASE *get_subbase(struct st *,struct sla *,int *f=NULL);
#ifndef SERVER 
	CX_BASE *open_db(char *); 
#endif 
	int write_change(long record,int n);
	char *Get_Main(long,int); 
	struct get_field_result Read(long,struct sla *,off_t,struct fd *,struct st *,char *&,int atr=0); 
	int Write(long,struct sla *,off_t,struct fd *,struct st *,char *,char *,int); 
	off_t Get_Page(long record,struct sla *sla); 
	int Put_Page(long record,struct sla *sla,char *slot,int len); 
 
	int create_struct(struct st *,struct fd *,char *); 
	void free_substruct(struct st *); 
	void close_tree(struct st *); 
	int open_FD(struct fd *); 
	int variable(struct st *,int); 
	char *Get_Buf(struct fd *, off_t, int); 
 
	int   Put_Buf(struct fd *, off_t, int, char *); 
	off_t Write_Text(char *,off_t seek=0); 
	off_t Write_Text(char *,int,int); 
	off_t Find_Space(int *); 
	int Free_Space(off_t,int); 
	double calculation(long, char *); 
	double Total(long record,struct sla *sla); 
 
	struct field *Get_Field_Descr(register int,long r=0);
	struct field *Get_Field_Descr(register struct sla *,long r=0);
	struct field *Get_Field_Descr(register struct st *,register int,long r=0);
	struct field *Get_Field_Descr(register struct st *,register struct sla *,long r=0);
 
	int Put_Node(long record,struct node node); 
	int Put_Node(long record,int field,struct node node); 
	int Put_Node(long record,struct sla *sla,struct node node); 
	int Delete_Record(long record,int field=0); 
	int Restore_Record(long record,int field=0); 
	int Get_Remote_Slot(char *name,struct sla *sla,long record,char *&slot); 
	int fill_vnode(long record,struct sla *sla,struct node_val *inode); 
	int comp_vnode(struct field *des_field,long record,struct sla *sla,struct node_val *inode,int len=0,int arg=0); 
	int balance(struct sla *sla,long *parent,struct node *pqn ,int npr); 
	int _del(struct sla *sla,long *parent,struct node qn,struct node *na,int max,int npr ); 
	int InsNode(struct field *des_field,struct sla *sla, struct node_val *inode ); 
	int DelNode(struct field *des_field,struct sla *sla,struct node_val *inode); 
	int Insert_Node(long record,int field, int ttree=0);
	int Insert_Node(long record,struct sla *sla, int ttree=0);
	int Delete_Node(long record,int, int ttree=0);
	int Delete_Node(long record,struct sla *sla,int ttree=0);
 
	int open_Tree(int field); 
	int open_Tree(struct sla *sla); 
 
	int Map(); 
	void Run_Method(); 
	void Inherit(CX_BASE *); 
 
	int _Check_Lock(long record,int bank,int *p=NULL); 
	int _Lock(long record,int type,int bank=0); 
	int add_lock(long record,int bank,int tip); 
	int del_lock(long record,int bank); 
 
	void UnMap(); 
	int Run(char *,int *); 
	int Buf_To_String(char *ch,struct get_field_result *,char *&slot); 
	int String_To_Buf(char *slot,struct field field,char *&ch,int no_compress=0,int field_size=0,long record=0); 
 
	void conv_39node(int fd,struct key *KEY); 
	long join(selection *b, selection *a); 
	int check_tree(char *query); 
//        long Select(struct sla *,char *,selection *, long (*)(long,CX_BROWSER *)); 
	void Make_Indname(char *name,int level=0); 
	void Next_Index_File(char *tmp,int level=0); 
	void Open_Index_File(int level=0); 
	void Flush_Index_Buf(int level=0); 
	void Write_Index(char *tmp); 
	int  del_Record(long page,int level=0); 
	void Set_Index(int level=0); 

	int  Insert_Storage(long record,struct sla *sla,char *slot, long page, int rebuild);
	int  Delete_Storage(long record,int field); 
	int  Put_Storage(long record,int field,char *slot); 
	int  Put_Storage(long record,struct sla *sla,char *slot); 
	char *Get_Storage(long record,int field); 
	int  Get_Storage(long record,int field,char *&slot); 
	long New_Storage_Page(int field); 
 
	int  Insert_Tree(long record,int field,char *slot);
	int  Delete_Tree(long record,int field);
	int  Put_Tree(long record,int field,char *slot);
	int  Put_Tree(long record,struct sla *sla,char *slot);
	char *Get_Tree(long record,int field);
	int  Get_Tree(long record,int field,char *&slot);

public: 
	struct share *share;    // shared memory (shm) structure 
	long cadr_record;       // current record 
	long cx_cond;           // current condition 
 
	CX_BASE(char *name,char *dir=NULL);
	~CX_BASE(); 
	void update(); 
	void flush_sb(); 
	void close_FD(); 
	int Map(char *); 
 
	struct field *Field_Descr(int,long r=0);
	struct field *Field_Descr(struct sla *,long r=0);
 
	int  Num_Fields(); 
	size_t  Size(); 
	int  Total_Num_Fields(); 
	char *Short_Name(); 
	char *Name_Base(); 
	char *Name_Field(int); 
	char *Name_Field(struct sla *); 
	char *Name_Subbase(int,long =0);
	char *Name_Subbase(struct sla *); 
	char *Name_Subbase(struct sla *,long); 
 
	int Read(char *&); 
	int Read(long, char *&); 
	struct get_field_result Read(long, int, char *&,int atr=0); 
	struct get_field_result Read(long, char *, char *&,int atr=0); 
	struct get_field_result Read(long, struct sla *,char *&,int atr=0); 
	struct get_field_result Read(int, char *&,int atr=0); 
	struct get_field_result Read(char *, char *&,int atr=0); 
	struct get_field_result Read(struct sla *,char *&,int atr=0); 
 
	int Write(long, int, char *); 
	int Write(long, char *, char *); 
	int Write(long, struct sla *, char *); 
	int Write(int, char *); 
	int Write(char *, char *); 
	int Write(struct sla *, char *); 
 
	int Get_Slot(int, char *&); 
	int Get_Slot(char *, char *&); 
	int Get_Slot(struct sla *,char *&); 
	int Get_Slot(long, int, char *&); 
	int Get_Slot(long, char *, char *&); 
	int Get_Slot(long, struct sla *,char *&); 

	long Back_Pointer(long, int);
	long Back_Pointer(long, char *);
	long Back_Pointer(long, struct sla *);
 
	double Get_Value(char *,long,char **out=NULL); 
	double Get_Value(long,int,   char **out=NULL); 
	double Get_Value(long,struct sla *,char **out=NULL); 
	double Get_Value(long,char *,char **out=NULL); 
 
	int Put_Slot(long, int, char *, int size=0); 
	int Put_Slot(long, char *, char *, int size=0); 
	int Put_Slot(long, struct sla *, char *, int size=0); 
	int Put_Slot(int, char *,int size=0); 
	int Put_Slot(char *, char *, int size=0); 
	int Put_Slot(struct sla *, char *, int size=0); 
 
	int Put_Property(long record, char *name, char *value, int atr=0);
	int Insert_Storage(long record,struct sla *sla,char *slot);
 
	int Str_To_Sla(char *, struct sla *); 
	int Sla_To_Str(struct sla *, char *); 
 
	long Select(char *,char *,selection *); 
	long Select(int,char *,selection *); 
	long Select(struct sla *,char *,selection *); 
	long Select(struct sla *,char *,selection *, long (*)(long,CX_BROWSER *)); 
 
	int Get_Property(long record, selection *sel); 
	int Select_Property(selection *sel, char  *attr, char *value=NULL); 
	int Get_Property_Slot(long record,char *attr,char * &slot,int flag=0);
	long Get_Property_Value(long record,char *attr);
 
	int Sorting(int, selection *,int nap=0); 
	int Sorting(struct sla *, selection *,int nap=0); 
	int Sorting(struct sla **, int, selection *,int nap=0); 
 
	long Find_First(int field, char *pattern);
	long SortBP(int field,selection *select,int nap);
	int  FindBP(int field,char *pattern,long *&select,int obr=0);

	void Get_Check(); 
	double Expression(long, char *); 
	int Access(); 
 
	int Check_Lock(long record,int bank=0); 
	int Rlock(long record,int bank=0);        // shared lock 
	int Wlock(long record,int bank=0);        // exclusive lock 
	int Unlock(long record,int bank=0); 
 
	long New_Record(int arg=0); 
	int  Delete(long record); 
	int  Restore(long record); 
	int  Check_Del(long record); 
 
	struct node Get_Node(long record); 
	struct node Get_Node(long record,int field); 
	struct node Get_Node(long record,struct sla *sla); 
 
	int Num_Elem_Array(long,int);
	int Num_Elem_Array(long,struct sla *);

	struct key Num_Virtual_Elem(long, int);
	struct key Num_Virtual_Elem(long, struct sla *);

	int Insert_Element(long record,int field,char *); 
	int Insert_Element(long record,char *,char *); 
	int Insert_Element(long record,struct sla *,char *); 
	int Remove_Element(long record, struct sla *sla); 
 
	long Record_Index(long page);
	long Record(long page,int level=0); 
	int  put_Record(long page,long ind,int level=0); 
	int  Cadr_Read(long); 
	void Cadr_Read(long,char *); 
	int  Cadr_Write(); 
	int  Cadr_Write(char *buf); 
	int  Cadr_Write(long record,char *buf); 
 
	void Roll_Back(); 
	int  Cadr_Change(); 
	int  Field_Change(int field); 
	int  Field_Change(struct sla *); 
	long current_record(); 
 
	int is_digit(int); 
	int is_digit(struct sla *); 
	int is_pointer(int); 
	int is_pointer(struct sla *);
	int is_pointer_recurs(struct sla *);
	int is_index(int); 
	int is_index(struct sla *); 
 
	int writable(); 
	struct st *type(); 
	int In_Memory(int);
 
	int Len_Cadr();
	int Len_Record();
	int version(); 
	void Create_Tree(); 
 
	void Read_Index(char *tmp,int level=0); 
	void Link_Index(char *tmp,int level=0); 
	int  Arr_To_Index(long *,int, int level=0); 
	int  Rest_Index(int level=0); 
	void Erase_Index(int level=0); 
	long find_page(long page,int level=0); 
 
	long last_cadr(); 
	long Max_Index(); 
	long Max_Record(); 

	int open(char *path, int flags, int mode=0);
	double atof(char *nptr);
	void make_changedb();

	long Write_Empty_Record(long record);

// for compatability with previos version. 
	struct get_field_result Read(long, int, char **); 
	struct get_field_result Read(long, char *, char **); 
	struct get_field_result Read(long, struct sla *,char **); 
	int Get_Slot(long, int, char **); 
	int Get_Slot(long, char *, char **); 
	int Get_Slot(long, struct sla *,char **); 

	void free(char *&ptr)
	{
		if(ptr!=NULL)
			::free(ptr);
		ptr=NULL;
	}

	void free(void *ptr)
	{
		::free(ptr);
	}

}; 
 
class CX_FIND 
{ 
private: 
	CX_BASE *db; 
	struct find_steck *f_steck; 
	char *find_str; 
	int   find_atr; 
	int direction; 
	int page; 
	int position; 
	long t_record; 
	struct sla sla[SLA_DEEP]; 
	int ttree;
	int bptree;
public: 
	CX_FIND(CX_BASE *db,int dir=1); 
	~CX_FIND(); 
 
	long Find_First(char *des,char *str1,int atr); 
	long Find_First(int field,char *str1,int atr); 
	long Find_First(struct sla *sla,char *str1,int atr); 
	long Find_Left(struct sla *);
	long Find_Left(int field);
	long Find_Left(char *des);
	long Find_Right(struct sla *);
	long Find_Right(int field);
	long Find_Right(char *des);
	long Next(); 
	long Find_Next(); 
	void Set_Direction(int i); 
}; 
 
class RCX_BASE: public CX_BASE 
{ 
private: 
	RCX_BASE *open_db(char *name); 
	int _Check_Lock(long record,int bank,int *p=NULL); 
	int add_lock(long record,int bank,int tip); 
	int del_lock(long record,int bank); 
public: 
	RCX_BASE(char *name); 
	~RCX_BASE(); 
}; 
 
RCX_BASE *open_db(char *name); 
 
struct bufer 
{ 
	int len; 
	char *buf; 
}; 
 
struct elem 
{ 
	int bank; 
	off_t seek_abs; 
	struct bufer old_buf; 
	struct bufer new_buf; 
}; 
 
class Transaction 
{ 
private: 
	CX_BASE *db; 
	char *name; 
	int num_elem; 
	struct elem *elem; 
public: 
	Transaction(CX_BASE *base); 
	~Transaction(); 
	char *Read(struct fd *fd,off_t seek_abs, int len); 
	int  Write(struct fd *fd, off_t seek_abs,int len, char *buf); 
	void Roll_Back(); 
}; 
 
struct val {long record; union value v; char r;}; 
 
class Query 
{ 
private: 
	CX_BASE *db; 
	int exactly; 
	int n; 
	int NAP; 
	struct sla **SLA; 
	int *tip; 
	int *num; 
	int NUM; 
	int curr_page; 
	int num_records; 
	int ff; 
	int xx,yy; 
	int curr_color; 
	int Compare(char *str); 
	int Compare(const void *a,const void *b); 
	union value get_value(int tip,char *str); 
	struct rq *rq; 
	int different(struct val *v1,struct val *v2,int i); 
	void swapfunc(char *a, char *b, int n, int swaptype); 
	char * med3(char *a, char *b, char *c); 
	void qsort(void *a, size_t n, size_t es); 
 
//        long Select(struct sla *sla,char *query,selection *select, long (*record)(long, CX_BROWSER *)); 
public: 
	Query(CX_BASE *db); 
	long Select(struct sla *sla,char *query,selection *select, long (*record)(long, CX_BROWSER *)); 
	int Sorting(struct sla **sla,int num_fields,selection *select,int nap); 
}; 
 
 
#ifndef WIN32 
 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <sys/un.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/time.h> 
#include <sys/uio.h> 
#else
#include <Winsock2.h>
#endif
class IPsocket 
{ 
public: 
	IPsocket(); 
	~IPsocket(); 
	int  fd(); 
	int  isValid(); 
	int  ReuseAddress(); 
	int  Port(); 
	int  Select(); 
	int  Select(int timeout); 
	int  Select(struct timeval *tv); 
 
	int m_sockfd; 
}; 
 
class ServerIPsocket :public IPsocket 
{ 
public: 
	ServerIPsocket(); 
	ServerIPsocket(unsigned short port); 
	~ServerIPsocket(); 
	int Accept(); 
	int Accept(int timeout);        // return -2 if time limit expires 
 
private: 
	void Init(unsigned short port); 
	struct sockaddr_in cli_addr, serv_addr; 
}; 
 
class ClientIPsocket:public IPsocket 
{ 
public: 
	ClientIPsocket(const char *host, unsigned short port); 
	~ClientIPsocket(); 
 
private: 
	struct sockaddr_in cli_addr, serv_addr; 
}; 
 
 
class Sock_Message 
{ 
public: 
	Sock_Message(); 
	Sock_Message(const char *host, unsigned short port); 
	Sock_Message(int sockfd); 
	Sock_Message(int readfd, int writefd); 
	~Sock_Message(); 
	void Init(int sockfd); 
	void Init(int readfd, int writefd); 
	int  isValid(); 
	int  ReadMsg(char *&InBuf);             // read message with default timeout 
	int  ReadMsg(int timeout,char *&InBuf); // timeout in seconds 
	int  MsgLength(); 
	int  WriteMsg(const char *msg); 
	int  WriteMsg(const void *msg, int length); 
	void DefaultTimeout(int seconds); 
	int  Getfd(int flag); 
	void Close(); 
	ClientIPsocket *getsock(); 
 
	int fdwrite; 
protected: 
	void    LocalInit(int readfd, int writefd); 
 
private: 
	int m_len;                  // actual length of received message 
	int fdread; 
	ClientIPsocket *m_sock;       // used only with Sock_Message(host,port) constructor 
 
	struct  timeval *m_timeout; 
	int ReadMsg(struct timeval *tv,char *&InBuf); 
	int readn(char *buf,int len); 
}; 

#ifndef WIN32 
class UNIXServerSocket:public IPsocket 
{ 
public: 
	UNIXServerSocket(const char *path); 
	~UNIXServerSocket(); 
	int Accept(); 
}; 
 
class UNIXClientSocket:public IPsocket 
{ 
public: 
	UNIXClientSocket(const char *path); 
	~UNIXClientSocket(); 
}; 
 
char *local_host_name(char *); 
char *host_name(in_addr addr,char *); 
char *host_name(int socket,char *); 
struct in_addr peer_inaddr(int socket); 
char *peer_dotname(int socket); 
struct in_addr local_host_addr(); 
char *HTTP_request(char *host,char *rq); 

#endif 

int cmp_type(struct field *des_field,char *ch1,char *ch2);
int Compare(struct field *des_field,char *ch1,char *ch2,int len);
int Schema_to_Text(struct st *struc,char *&buf,int level=0,int xml=0,int blank=0);
int Text_to_Schema(char *buf,struct st *&ss,int i,int *len);

#ifndef WIN32
#define TEMPDIR "/tmp"
#else
#define TEMPDIR "C:/Program Files/UnixSpace/tmp"
#endif

#endif
