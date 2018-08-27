#include "CX_BASE.h" 
 
class RAM_BASE:public CX_BASE 
{ 
	friend class CXDB_REPAIR; 
private: 
	void init(struct sla *); 
protected: 
	int ram_size; 
	int len_rec; 
	void *pos; 
	struct sla v_field[10]; 
	struct field *des_field; 
	long *delete_list; 
	int del_num; 
 
	int fill_vnode(long record,struct sla *sla,struct node_val *inode); 
	int comp_vnode(long record,struct sla *sla,struct node_val *inode); 
	int Put_Node(long record,struct sla *sla,struct node node); 
	struct node Get_Node(long record,struct sla *sla); 
	int InsNode(struct sla *sla, struct node_val *inode ); 
	int Insert_Node(long record,int field); 
	int Insert_Node(long record,struct sla *sla); 
//        int balance(struct sla *sla,long *parent,struct node *pqn ,int npr); 
public: 
	RAM_BASE(char *name_base,int field,int len_field=8); 
	RAM_BASE(char *name_base,struct sla *sla,int len_field=8); 
	~RAM_BASE(); 
	void Create_Tree(); 
}; 
 
class CX_REPAIR:public RAM_BASE 
{ 
public: 
	CX_REPAIR(char *folder,int field):RAM_BASE(folder,field){}; 
	~CX_REPAIR() 
	{ 
	} 
	void Create_Tree(); 
}; 
 
class CXDB_REPAIR:public RAM_BASE 
{ 
private: 
	int visual; 
public: 
	CXDB_REPAIR(char *folder,int field,int v):RAM_BASE(folder,field) 
	{ 
		visual=v; 
	}; 
	~CXDB_REPAIR() 
	{ 
	} 
	void Create_Tree(); 
}; 
