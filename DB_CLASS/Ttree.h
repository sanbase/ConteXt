#ifndef _PAGE_H 
#define _PAGE_H 
 
class CX_BASE; 

typedef struct node_data
{
	long record;
	long point;
	char data[1];
};

typedef struct leaf_data
{
	long record;
	char data[1];
};

 
struct page_hdr 
{ 
	struct key KEY;   // left and right pointers 
	long parent;      // parent node 
	long space;       // free space 

	short field;      // field number
	short last;       // number of fields (the last field in the page)
	unsigned short l; // size of field - up to 64KB 
	unsigned char  a; // attribute 
	unsigned char  n; // precision(%.n) 
	long PAGESIZE;    // size of the page
	unsigned long  z; // next page for large field
}; 
 
class TPage
{ 
public: 
	struct page_hdr hdr; 
	char buf[1]; 
 
	char *MIN(); 
	char *MAX(); 
	int len(int); 
	int len(char *); 
	int len_field(char *); 
	int pos(int); 
	long rec(char *); 
	long rec(int); 
	void push_right(TPage *);
	void push_left(TPage *);
	int Compare(struct field *field,char *ch1,leaf_data *ch2,long record=0,int len=0);
 
	int del(int i); 
	int del(char *ch); 
 
	int insert(char *slot,long record);
	struct key find(char *value, long record, int atr=0,int len=0); 
	char *value(char *); 
	char *value(long record,int i=0); 
}; 
 
class TTree
{ 
private: 
	CX_BASE *db; 
public: 
	TTree(CX_BASE *);
	int Find_Page(int field,char *slot,long record); 
	struct key Pos_in_Page(long page,char *value,long record,TPage *val=NULL,int len=0);
	long Find_Left(int field,long page);
	long Find_Right(int field,long page);
}; 
 
#endif 
