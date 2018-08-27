#ifndef _CONTEXT_H 
#define _CONTEXT_H 
 
#ifdef PATROL 
#include <mpatrol.h> 
#endif 
#include <sys/types.h> 
#ifdef SPARC 
typedef longlong_t  dlong; 
typedef int socklen_t; 
#else 
#ifdef WIN32 
#include <io.h> 
#define F_OK 0 
#define W_OK 2 
#define R_OK 4 
typedef __int64 dlong; 
typedef int pid_t; 
#include <io.h> 
#include <direct.h> 
#define VER 7
#else 
typedef int64_t  dlong; 
#define O_BINARY 0
#include <pthread.h> 
#endif 
 
#define LINESIZE 256 
#define NAMESIZE 256 
#define NAMELEN  64 
 
#define CXKEY3   5472564    //  ConteXt-3 
#define CXKEY4   514142603  //  Context-4 
#define CXKEY5  -514142603  //  Context-5 
#define CXKEY6   8848177    //  Context-6
 
#define SLA_DEEP    16 
 
#define X_STRING      1
#define X_DATE        2
#define X_TIME        3
#define X_POINTER     4
#define X_VARIANT     5
#define X_INTEGER     6
#define X_UNSIGNED    7
#define X_FLOAT       8
#define X_DOUBLE      9
#define X_STRUCTURE  10
#define X_TEXT       11
#define X_BINARY     12
#define X_FILENAME   13
#define X_EXPRESSION 14
#define X_IMAGE      15
#define X_COMPLEX    16
 
#define MULTISET    1 
#define SET         2 
#define LIST        3 
 
struct field 
{ 
	unsigned char  a:7;  // attribute
	unsigned char  b:1;  // b-tree
	unsigned char  n:4;  // precision(%.n) 
	unsigned char  d:1;  // direct pointer 
	unsigned char  m:2;  // array 
	unsigned char  k:1;  // key (polymorph) field 
	unsigned short   l;  // size of field - up to 64KB 
	union 
	{ 
		struct attr *attr; 
		long num_subfield; 
	} atr; 
	union 
	{ 
		struct st *st; 
		long struct_descr; 
	} st; 
	char  *name; 
}; 
 
// header of folder 
struct header 
{ 
	long pswd;           // magic number 
	long ptm;            // number of primary fields 
 
	unsigned long b0; 
	unsigned char b1; 
	unsigned char b2; 
	unsigned char b3; 
 
	unsigned char v;     // #version 
}; 
 
// b0 flags: 
#define KEEP_DELETE 0x1 
#define NO_DELETE   0x2 
#define NO_COMPRESS 0x4 
 
//  slot 
struct sla 
{ 
	short n;      // field 
	short m;      // element of array (if field is array) 
}; 
 
 
struct attr 
{ 
	long wshift; 
	long cshift; 
	struct fd *sfd; 
	long tree_fd; 
}; 
 
struct st 
{  // structure of folder or composite field 
	long ptm;            // number of primary fields 
	unsigned long size;  // physical size if the record or field 
	struct field *field; // array of field structures 
}; 
 
struct fd 
{       // descriptor 
	int Fd;               // descriptor itself 
	int n;                // number of the Bank
	int b;                // number of Tree
	long size;            // size of superblock 
	off_t seek;           // position in superbloc 
	off_t fsize;          // file size 
	char *buf;            // superblock
	char in_memory;       // in_memory flag
}; 
 
struct var      // header of the data in Bank.0 
{ 
	unsigned long length;   // occuped space 
	unsigned long size;     // size of data 
	unsigned long main;     // back pointer to Main 
	long magic;             // magic number 
}; 
 
struct var_hdr 
{ 
	long length; 
	long size; 
}; 
 
// B-tree structure 
struct key 
{ 
	long l,r; 
}; 
 
// structure of node 
struct node 
{ 
	long x[2];  
	char b,s; 
}; 
 
struct root 
{ 
	struct key Root; 
	struct key del_chain; 
	long reserv[4]; 
}; 
 
 
struct lock_record 
{ 
	long record; 
	int  bank; 
	int  num; 
	int  type; 
#ifdef THREAD 
	pthread_t pid; 
#else 
	pid_t pid; 
#endif 
}; 
 
struct f_lock
{ 
	unsigned size;  
	struct lock_record *lock_str; 
}; 
 
struct node_val 
{ 
	long record; 
	int len; 
	char *ch; 
	char Dtree; 
}; 
 
struct get_field_result 
{ 
	long len; 
	struct field field; 
	struct header context; 
	struct sla sla[SLA_DEEP]; 
}; 
 
union cmp {unsigned short n[2]; long ne; }; 
 
struct color 
{ 
	long  fg; 
	long  bg; 
}; 
 
struct tag_descriptor 
{ 

	short x;        // x coordinate 
	short c;        // columns 
	short y;        // y coordinate 
	short r;        // rows 
	short l;        // length 
	short h;        // heigth 

	long atr; 
	struct color color; 
	struct sla sla[SLA_DEEP]; 
}; 
 
// values of atr field 
#define NO_EDIT         1 
#define NO_POS          2 
#define TABL            4 
#define SECURE          8 
#define NO_RECURS    0x10 
#define NO_NEW_REC   0x20 
#define IS_IMAGE     0x40 
#define NO_NED       0x80 
 
#define NO_SHADOW       1 
#define FONT         0x80 
 
struct old_tag_descriptor 
{ 
	long x; 
	long y; 
	long l; 
	long atr; 
	struct sla sla[10]; 
	struct color color; 
}; 
 
union s_space 
{ 
	struct old_tag_descriptor slot; 
	char buf[sizeof (struct old_tag_descriptor)]; 
}; 
 
struct slot 
{ 
	struct tag_descriptor des; 
	char name[NAMESIZE];
}; 
 
struct x_form 
{ 
	long  form; 
	char blank[NAMELEN]; 
}; 
 
struct font 
{ 
	char fnt; 
	char atr; 
}; 
 
struct tag
{
	long index;
	char *str;
	char *name;
	char *mask;
	struct color color;
	struct font font;
	struct tag_descriptor des;
	int shift;
};


// shared memory 
struct share 
{ 
	long record;                // current record 
	long index;                 // current index 
	long cadr_record;           // read record 
	long cmd;                   // command 
	long ret;                   // returned value 
	struct color color;         // color of message 
	struct x_form form;         // current form 
	char output[LINESIZE];      // I/O string 
	char io[LINESIZE];          // inter process buffer 
	struct tag_descriptor slot; // current slot 
	struct font font;           // font for virtual fields 
	char edit_flag;             // record has been changed
	union
	{
		struct tag field[128];  //
		char reserve[15684];    // The total size of the shared memoty is 16K
	};
}; 
 
#define CHK_LINE  1 
#define CHK_CADR  2 
#define CHK_ACT   3 
#define VIRTUAL   4 
#define NEWINDEX  5
 
struct s_stack 
{ 
	long num_index; 
	struct sla sla[SLA_DEEP]; 
	char *query; 
	char *name; 
}; 
 
struct image 
{ 
	char *name; 
	char *des; 
	long left; 
	long right; 
	int x; 
	int y; 
}; 
 
struct last_status 
{ 
	long record; 
	struct x_form form; 
	long  field; 
	long  status; 
	char rez[24]; 
}; 
 
struct edit_history 
{ 
	struct sla sla[SLA_DEEP]; 
	char   *str; 
}; 
 
struct find_steck 
{ 
	long num; 
	struct node node; 
}; 
 
union value {double d; char *ch; dlong i; char str[sizeof (dlong)];}; 
 
struct item 
{ 
	char *name; 
	int bg; 
	int fg; 
	int font; 
}; 
 
 
#define ROOT      "Main" 
#define BANK      "Bank." 
#define TREE      "Tree"
#define SPACEDB   "_Space" 
#define EXPRDB    "_Expression" 
#define COMPLDB   "_Complex" 
#define FLEXDB    "_Flex" 
#define INDEXDIR  "_Select" 
#define LIMITDIR  "_Limit" 
#define TMPDIR    "_Tmp" 
#define FORMDB    "_Forms" 
#define PROPERTY  "_Property" 
#define EXEDIR    "_Usebin" 
#ifdef WIN32
#define MODULE    "Methods.dll"
#else
#define MODULE    "Methods" 
#endif
#define RULES     "Rules" 
#define MACRO     "_Macro" 
#define QUERY     "_Query" 
#define FILES     "_Files" 
#define LOGDB     "_LogDB" 
#define HISTORY   "_History" 
#define BLANKDIR  "_Blanks" 
#define HYPERFORM "_HyperForm" 
#define STORAGE   "_Storage" 
#define CHANGEDB  "_Change"
 
#include "CX_common.h" 
#endif 
#endif 
