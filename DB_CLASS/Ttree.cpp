/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:Ttree.cpp
*/
/*
			  DBMS ConteXt V.5.8
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Thu Sep  2 10:03:10 2010
			Module:Ttree.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
//extern Terminal *term;

TTree::TTree(CX_BASE *db)
{ 
	this->db=db; 
} 
 
char *TPage::MIN()
{ 
	return(buf); 
} 
char *TPage::MAX()
{ 
	char *ch=buf+pos(hdr.last-1);
	return(ch); 
} 
 
int TPage::len(int i)
{ 
	if(hdr.a==X_TEXT || hdr.a==X_BINARY) 
		return(int_conv(((leaf_data *)(buf+pos(i)))->data,hdr.n==0?4:hdr.n)+4);
	else    return(hdr.l+4); 
} 
 
int TPage::len(char *ch)
{ 
	if(hdr.a==X_TEXT) 
	{
		return(int_conv(ch,hdr.n==0?4:hdr.n)+4);
	}
	return(hdr.l+4); 
} 
 
int TPage::len_field(char *ch)
{ 
	if(ch==NULL) 
		return(0); 
	if(hdr.a==X_TEXT) 
	{ 
		int len=int_conv(ch,hdr.n==0?4:hdr.n);
		return(len-=(hdr.n==0?4:hdr.n)); 
	} 
	return(hdr.l); 
} 
 
void TPage::push_right(TPage *tmp)
{
	if(hdr.last==0)
		return;
	char *ch=MAX();
	tmp->insert(((leaf_data *)ch)->data,((leaf_data *)ch)->record);
	hdr.space+=len(((leaf_data *)ch)->data);
	hdr.last--;
//        bzero(buf+hdr.PAGESIZE-sizeof (struct key)-hdr.space,hdr.space);
	return;
}

void TPage::push_left(TPage *tmp)
{ 
	if(hdr.last==0) 
		return;
	char *ch=MIN();
	tmp->insert(((leaf_data *)ch)->data,((leaf_data *)ch)->record);
	int l=len(((leaf_data *)ch)->data);
	hdr.space+=l;
	hdr.last--;
	bcopy(ch+l,ch,hdr.PAGESIZE-sizeof hdr - hdr.space);
//        bzero(buf+hdr.PAGESIZE-sizeof (struct key)-hdr.space,hdr.space);
	return;
} 
 
char *TPage::value(char *ch)
{ 
	if(hdr.a==X_TEXT) 
		return(ch+(hdr.n==0?4:hdr.n));
	return(ch);
} 
 
char *TPage::value(long record,int n)
{ 
	register int count=0;
	char *ch=buf;
	for(register int i=0;i<hdr.last;i++)
	{ 
		if(int_conv(ch,4)==record && (count++==n))
		{ 
			return(((leaf_data *)ch)->data);
		} 
		ch+=len(((leaf_data *)ch)->data);
	} 
	return(NULL); 
} 
 
int TPage::pos(int p)
{ 
	if(hdr.a==X_TEXT) 
	{ 
		register char *ch=((leaf_data *)(buf))->data;
		register int l;
		int ln=0, shift=0;
#ifndef SPARC
		if(hdr.n==2)
			ln=2;
		if(hdr.n==4 || hdr.n==0)
			ln=4;
#endif
		for(register int i=0;i<p && i<=hdr.last;i++)
		{ 
			if(ln==2)
				l=(*(short *)(ch));
			else if(ln==4)
				l=(*(long *)(ch));
			else
				l=int_conv(ch,hdr.n);
			l+=4;
			shift+=l; 
			ch+=l; 
		} 
		return(shift); 
	} 
	else    return(p*(hdr.l+4));
} 
 
long TPage::rec(char *ch)
{ 
#ifdef SPARC
	return((long)int_conv(ch,4));
#else
	return(*(long *)ch);
#endif
} 
 
long TPage::rec(int i)
{ 
	return(rec(buf+pos(i))); 
} 
 
 
int TPage::del(char *ch)
{ 
	if(ch==NULL) 
		return(-1); 
	hdr.space+=len(((leaf_data *)ch)->data);
	int shift=(int)(ch-buf); 
	int length=len(((leaf_data *)ch)->data);
	bcopy(ch+length,ch,hdr.PAGESIZE-length-shift-sizeof (struct page_hdr)-sizeof (struct key));
//        bzero(buf+hdr.PAGESIZE-sizeof (struct key)-hdr.space,hdr.space);
	return(--hdr.last); 
} 
 
int TPage::del(int i)
{ 
	if(i<0 || i>=hdr.last) 
		return(-1); 
	return(del(buf+pos(i))); 
} 
 
int TPage::Compare(struct field *field,char *ch1,leaf_data *ch2,long record,int len)
{ 
	int j; 
	if(hdr.a==X_TEXT) 
	{ 
		int len1=len_field(ch1); 
		int len2=len_field(ch2->data);
		if(len2<=0) 
			j=len1>0; 
		else if(len1<=0) 
			j=-1; 
		else 
		{ 
			char *a1=(char *)calloc(len1+1,1); 
			char *a2=(char *)calloc(len2+1,1); 
			if(value(ch1)!=NULL) 
				bincopy(value(ch1),a1,len1);
			if(value(ch2->data)!=NULL)
				bincopy(value(ch2->data),a2,len2);
			j=strcmpw(a1,a2,len); 
			free(a1); 
			free(a2); 
		} 
	} 
	else 
	{ 
		j=::Compare(field,ch1,ch2->data,len==0?hdr.l:len);
	} 
	if(j==0 && record!=0)
#ifdef SPARC
		j=record-rec((char *)ch2);
#else
		j=record-ch2->record;
#endif
	return(j>0?1:j<0?-1:0); 
} 
 
int TPage::insert(char *slot,long record)
{ 
	struct field field; 
	bzero(&field, sizeof field); 
	field.a=hdr.a; 
	field.l=hdr.l; 
	field.n=hdr.n; 
	char *ch=buf; 
 
	int i,l=0; 
	int h=hdr.last-1; 
	while(l<=h) 
	{ 
		i = (l+h)/2; 
		ch=buf+pos(i);
		int c=Compare(&field,slot,(leaf_data *)ch,record);
		if(c==0) 
			return(0);
		if(c>0)
			l=i+1; 
		else 
			h=i-1; 
	} 
	int shift=pos(l);
	ch=buf+shift; 
	int length=len(slot);
	if(hdr.space-length<0)  // no enought space
	{ 
		if(l==hdr.last)
			return(-2);
/*
		if(l==0 || l==hdr.last)
			return(2);
		if(hdr.KEY.r<0)
			return(1);
		if(hdr.KEY.l<0)
			return(-1);
		if(l>(hdr.last>>1))
			return(1);
*/
		return(-1);
	} 

	bcopy(ch,ch+length,hdr.PAGESIZE-length-shift-sizeof (struct page_hdr)-sizeof (struct key));
	bincopy(&record,ch,4);
	bincopy(slot,ch+4,length-4);
	hdr.space-=length;
	hdr.last++; 
	return(0);
} 
 
struct key TPage::find(char *pattern, long record, int atr, int len)
{ 
	struct key key; 
	struct field field; 
 
	bzero(&field, sizeof field); 
	field.a=hdr.a; 
	field.l=hdr.l; 
	field.n=hdr.n; 
	key.l=0; 
	key.r=0; 
	int j=Compare(&field,pattern,(leaf_data *)MIN(),record,len);
	if(j<0 || (j==0 && record && rec(MIN())>record))
	{ 
		key.l=-1; 
		return key; 
	} 
	j=Compare(&field,pattern,(leaf_data *)MAX(),record);
	if(j>0 || (j==0 && record && rec(MAX())<record)) 
	{ 
		key.l=1; 
		return key; 
	} 
	if(atr) 
		return key; 
 
	int i=0,l=0; 
	int h=hdr.last-1; 
	key.r=-1; 
	while(l<=h) 
	{ 
		i = (l+h)/2; 
		leaf_data *ch2=(leaf_data *)(buf+pos(i));
		int c = Compare(&field,pattern,ch2,record,len);
		if(c==0) 
		{ 
			key.r=i; 
			break; 
		} 
		if(c > 0)
			l=i+1; 
		else 
			h=i-1; 
	} 
	if(key.r>0) 
	{
		while(i) 
		{ 
			leaf_data *ch2=(leaf_data *)(buf+pos(i));
			if(Compare(&field,pattern,ch2,record,len))
				break; 
			key.r=i; 
			i--; 
		} 
	}
	return(key); 
} 
 
struct key TTree::Pos_in_Page(long page,char *value,long record,TPage *val,int len)
{ 
	if(val==NULL) 
	{ 
		struct key key; 
		key.l=-1; key.r=-1; 
		val=(TPage *)db->Get_Buf(db->fd+2,page*db->PAGESIZE,db->PAGESIZE);
		if(val==NULL) 
			return(key); 
	} 
	return(val->find(value,record,0,len)); 
} 
 
long TTree::Find_Left(int field,long page)
{
	struct node node;
	while((node=db->Get_Node(page,field)).x[0])
		page=node.x[0];
	return(page);
}

long TTree::Find_Right(int field,long page)
{ 
	struct node node; 
	while((node=db->Get_Node(page,field)).x[1])
		page=node.x[1];
	return(page); 
} 
 
int TTree::Find_Page(int field,char *slot,long record)
{ 
	int v; 
	struct node node; 
	long page; 
 
	node=db->Get_Node(0,field); 
	if(node.x[1]==0)
	{ 
		return(-3); 
	} 
	page=node.x[1]; 
	long MAX=node.x[0]+1; 
	for(v=0;v<MAX;v++) 
	{ 
		TPage *val=(TPage *)db->Get_Buf(db->fd+2,page*db->PAGESIZE,db->PAGESIZE);
		if(val==NULL) 
			return(-1); 
		node.x[0]=val->hdr.KEY.l; 
		node.x[1]=val->hdr.KEY.r; 
		node.s=0; 
		if(val->hdr.KEY.l<0 && val->hdr.KEY.r<0) 
			return(-1);     // deleted
		if(val->hdr.KEY.l<0)
		{ 
			node.x[0]=-val->hdr.KEY.l; 
		} 
		else    if(val->hdr.KEY.r<0) 
		{
			node.x[1]=-val->hdr.KEY.r; 
		}

		long rez=val->find(slot,record,1).l; 
 
		if(rez==0) 
		{ 
			return(page); 
		} 
		if(rez<0) 
		{ 
			if(node.x[0]==0) 
				return(page); 
			page=node.x[0]; 
		} 
		else if(rez>0) 
		{ 
			if(node.x[1]==0) 
				return(page); 
/*
			if(val->hdr.space>=val->len(slot)) 
			{ 
				TTree dt(db);
				long l_page=dt.Find_Left(field,node.x[1]); 
				val=(TPage *)db->Get_Buf(db->fd+2,l_page*db->PAGESIZE,db->PAGESIZE);
				if(val->find(slot,record,1).l<0) 
				{
					return(page); 
				}
			} 
*/
			page=node.x[1]; 
		} 
	} 
	return(page); 
} 
