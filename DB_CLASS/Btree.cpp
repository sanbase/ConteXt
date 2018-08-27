/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:Btree.cpp
*/
#include "StdAfx.h"

#include "CX_BASE.h"
#ifdef WIN32
#include <sys/locking.h>
#define F_WRLCK	_LK_NBLCK
#define F_RDLCK	_LK_RLCK
#define F_UNLCK	_LK_UNLCK
#define F_SETLKW _LK_LOCK
struct flock
{
	off_t	l_start;	/* starting offset */
	off_t	l_len;		/* len = 0 means until end of file */
	pid_t	l_pid;		/* lock owner */
	short	l_type;		/* lock type: read/write, etc. */
	short	l_whence;	/* type of l_start */
};
#endif

#define PAGESIZE 4096

struct Data
{
	long record;
	char dat[1];
};

#define SHIFT(i) *(unsigned short *)((char *)this+PAGESIZE-(i+1)*2)

/**
leaf: {header, {long record; char data[]}, [short shift[]]}
node: {header, {long record; char data[], long pointer},.... [short shift[]]}
*/

int BPage::len(int i)
{
	if(i<0 || i>=hdr.h.last)
		return(0);
	register int l;
	if(hdr.f.a==X_TEXT)
	{
		if(i==0)
			l=SHIFT(i)-4;
		else
			l=SHIFT(i)-SHIFT(i-1)-4;
	}
	else
		l=hdr.f.l;
	return(l);
}

char *BPage::pos(register int i)
{
	if(i==0)
		return(hdr.h.data);
	if(i>hdr.h.last || i<0)
		return(NULL);
	if(hdr.f.a==X_TEXT)
		return(hdr.h.data+SHIFT(i-1));
	return(hdr.h.data+i*(hdr.f.l+4+(hdr.h.left?4:0)));
}

int BPage::find_first(char *pattern)
{
	int len=hdr.f.l;
	if(hdr.f.a==X_TEXT)
		len=strlen(pattern);
	for(register int i=0;i<=hdr.h.last;i++)
	{
		char *ch=pos(i);
//printf("find_first=%s (%s)\n",ch,ch+4);
		if(ch==NULL)
			return -1;
		if(::Compare(&hdr.f,pattern,ch+4,len))
			return i;
	}
	return -1;
}
int BPage::get_pos(register long record)
{
	for(register int i=0;i<hdr.h.last;i++)
	{
		char *ch=pos(i);
		if(ch==NULL)
			return -1;
		if(*(long *)ch==record)
			return i;
	}
	return -1;
}

int BPage::cmp(char *pattern, int p_len, char *slot, int s_len, long record)
{
	register int j=0;

	if(hdr.f.a==X_TEXT)
	{
		register char *p=pattern;
		register char *s=slot;

		for(int i=0;i<p_len && i<s_len;i++)
		{
			j=(*p++)-(*s++);
			if(j<0)
			{
				return(-1);
			}
			if(j>0)
			{
				return(1);
			}
		}
		if(p_len>s_len)
			j=1;
		else if(p_len<s_len)
			j=-1;
	}
	else
		j=::Compare(&hdr.f,pattern,slot,s_len);
	if(j==0 && record>0)
		j=record-*(long *)(slot-4);

//printf("patt=%s slot=%s rez=%d\n",pattern,slot,j);

	return(j>0?2:j<0?-2:0);
}

int BPage::find_pos(char *pattern,int length,long record,int *l)
{
	register int i=0;
	int h=hdr.h.last-1;
	*l=0;
	while(*l<=h)
	{
		i = (*l+h)/2;
		int c = cmp(pattern,length,pos(i)+4,len(i),record);
		if(c==0)
		{
			*l=i;
			return(0);
		}
		if(c > 0)
			*l=i+1;
		else
			h=i-1;
	}
	return(1);
}

int BPage::del(char *slot,int l,long record)
{
	int i;
	if(find_pos(slot,l,record,&i)!=0)
		return(-1);
	return(del(i));
}

int BPage::del(int i)
{
	if(i<0 || i>=hdr.h.last)
		return(-1);
	if(hdr.h.last==0)
		return(1);
	int l;
	if(hdr.f.a==X_TEXT)
	{
		int last=hdr.h.last-1;
		l=SHIFT(i)-(i==0?0:SHIFT(i-1));
		memmove(hdr.h.data+(i==0?0:SHIFT(i-1)),hdr.h.data+SHIFT(i),SHIFT(last)-SHIFT(i));
		hdr.h.space+=(l+2);

		for(int j=i;j<last;j++)
			SHIFT(j)=SHIFT(j+1)-l;
		memset(hdr.h.data+(last==0?0:SHIFT(last-1)),0,l);
		SHIFT(last)=0;
	}
	else
	{
		l=hdr.f.l+4+(hdr.h.left?4:0);
		register char *ch=hdr.h.data+i*l;
		memmove(ch,ch+l,(hdr.h.last-i)*l);
		memset(hdr.h.data+(hdr.h.last-1)*l,0,l);
		hdr.h.space+=l;
	}
	hdr.h.last--;
	return(0);
}

int BPage::split(BPage *tmp)
{
	memset(tmp,0,PAGESIZE);
	memcpy(&tmp->hdr,&hdr.h,32);
	tmp->hdr.h.space=PAGESIZE-32;
	tmp->hdr.h.fdes=hdr.h.fdes;
	if(hdr.f.a==X_TEXT)
	{
		int i;

		for(i=0;i<hdr.h.last;i++)
			if(SHIFT(i)>(PAGESIZE-32)/2-(i+1)*2)
				break;
		if(i==0 || i==hdr.h.last)
			return(-1);
		i--;

		int sh=SHIFT(i);
		int len=SHIFT(hdr.h.last-1)-sh;

		memmove(tmp->hdr.h.data,hdr.h.data+sh,len);
		memset(hdr.h.data+sh,0,len);
		hdr.h.space+=len;
		tmp->hdr.h.space-=len;

		tmp->hdr.h.last=hdr.h.last-i-1;
		len=(tmp->hdr.h.last)*2;
		memmove((char *)tmp+PAGESIZE-(tmp->hdr.h.last)*2,(char *)this+PAGESIZE-(hdr.h.last)*2,len);
		memset((char *)this+PAGESIZE-(hdr.h.last)*2,0,len);
		hdr.h.space+=len;
		tmp->hdr.h.space-=len;

		hdr.h.last=i+1;
		for(i=0;i<tmp->hdr.h.last;i++)
			*(unsigned short *)((char *)tmp+PAGESIZE-(i+1)*2)-=sh;
	}
	else
	{
		tmp->hdr.h.last=hdr.h.last/2;
		int l=(tmp->hdr.h.last)*(hdr.f.l+4+(hdr.h.left?4:0));
		tmp->hdr.h.space-=l;
		hdr.h.last-=tmp->hdr.h.last;
		memmove(tmp->hdr.h.data,hdr.h.data+hdr.h.last*(hdr.f.l+4+(hdr.h.left?4:0)),l);
//                memmove(tmp->hdr.h.data,hdr.h.data+(tmp->hdr.h.last+1)*(hdr.f.l+4+(hdr.h.left?4:0)),l);
		hdr.h.space+=l;
		memset(hdr.h.data+hdr.h.last*(hdr.f.l+4+(hdr.h.left?4:0)),0,hdr.h.space);
	}
	return(0);
}

int BPage::insert(char *slot,int l,long record,int i,long page)
{
	register char *ch;
	l+=4;   // space for record
	if(hdr.h.left)  // it is a node
		l+=4; // the space for the number of leaf
	if(hdr.h.space<l+(hdr.f.a==X_TEXT?2:0))
		return(-1);
	if(hdr.f.a==X_TEXT)
	{
		if(hdr.h.last==0)
		{
			memcpy(hdr.h.data+4,slot,l-4);
			SHIFT(0)=l;
			i=0;
		}
		else
		{
			int last=hdr.h.last-1;
			ch=pos(i);

			if(last>=0 && i<=last)
				memmove(ch+l,ch,i==0?SHIFT(last):SHIFT(last)-SHIFT(i-1));
			memcpy(ch+4,slot,l-4);
			if(i>last)
				SHIFT(i)=SHIFT(i-1)+l;
			else for(register int j=last;j>=i;j--)
				SHIFT(j+1)=SHIFT(j)+l;
			if(i==0)
				SHIFT(0)=l;
			else
				SHIFT(i)=SHIFT(i-1)+l;
		}
		*(long *)(hdr.h.data+(i==0?0:SHIFT(i-1)))=record;
		hdr.h.space-=(l+2);
		if(hdr.h.left)
		{
			*(long *)(hdr.h.data+SHIFT(i)-4)=page;
		}
	}
	else
	{
		l=hdr.f.l+4+(hdr.h.left?4:0);
		register char *ch=hdr.h.data+i*l;
		if(hdr.h.last>0)
			memmove(ch+l,ch,(hdr.h.last-i)*l);
		memcpy(ch+4,slot,l-4);
		*(long *)ch=record;
		hdr.h.space-=l;
		if(hdr.h.left)
		{
			*(long *)(ch+l-4)=page;
		}
	}
	hdr.h.last++;
	return(0);
}

BTree::BTree(CX_BASE *base,char *name,struct field f,int field)
{
	db=base;
	this->f=f;
	this->field=field;
	memset(pool,0,sizeof pool);
	for(int i=0;i<POOL_SIZE;i++)
	{
		pool_pages[i]=pool+i;
		BPage *tmp=(BPage *)(pool_pages[i]->buf);
		tmp->hdr.h.page=-1;
	}
	level=0;
	char d_name[256];
	strncpy(d_name,name,255);
	decode(d_name);
	if((fd=open(d_name,O_RDWR|O_BINARY))<0)
	{
		if((fd=open(d_name,O_RDWR|O_BINARY|O_CREAT,0644))<0)
			return;
		New_Page();
	}
}

BTree::~BTree()
{
	for(int i=0;i<POOL_SIZE;i++)
	{
		if(pool[i].status!=0)
			Flush((BPage *)pool[i].buf);
	}
	close(fd);
}

//F_RDLCK
//F_WRLCK

int BTree::Lock(int type)
{
	struct flock arg;

	if(type==F_WRLCK && wr_lock)
	{
		wr_lock++;
		return 0;
	}
	if(type==F_RDLCK && (rd_lock || wr_lock))
	{
		rd_lock++;
		return 0;
	}

	arg.l_whence=0;
	arg.l_type=type;
	arg.l_start=0;
	arg.l_len=PAGESIZE;
#ifdef WIN32
	lseek(fd,arg.l_start,SEEK_SET);
	_locking(fd,F_SETLKW,arg.l_len);
#else
	fcntl(fd,F_SETLKW,&arg);
#endif
	if(type==F_WRLCK)
		wr_lock=1;
	if(type==F_RDLCK)
		rd_lock=1;
	return(0);
}

void BTree::Unlock(int type)
{
	struct flock arg;

	if(type==F_WRLCK && wr_lock)
		wr_lock--;
	if(type==F_RDLCK && rd_lock)
		rd_lock--;
	if(rd_lock==0 && wr_lock==0)
	{
		arg.l_whence=0;
		arg.l_type=F_UNLCK;
		arg.l_start=0;
		arg.l_len=PAGESIZE;
#ifdef WIN32
		lseek(fd,arg.l_start,SEEK_SET);
		_locking(fd,F_SETLKW,arg.l_len);
#else
		fcntl(fd,F_SETLKW,&arg);
#endif
	}
}

BPage *BTree::New_Page()
{
	if(pool_pages[POOL_SIZE-1]->status>0)
		Flush((BPage *)pool_pages[POOL_SIZE-1]->buf);
	Pool *tmp=pool_pages[POOL_SIZE-1];
	memmove(pool_pages+1,pool_pages,(POOL_SIZE-1)*sizeof (BPage *));
	pool_pages[0]=tmp;
	memset(pool_pages[0]->buf,0,PAGESIZE);
	BPage *bp=(BPage *)pool_pages[0]->buf;

	memcpy(&bp->hdr.f,&f,sizeof bp->hdr.h.fdes);

	bp->hdr.h.space=PAGESIZE-32;

	off_t seek=lseek(fd,0,SEEK_END);
	bp->hdr.h.page=seek/PAGESIZE;

	write(fd,bp,PAGESIZE);

// field att in the first page is a maximum page of the B-tree.

	lseek(fd,28,SEEK_SET);
	write(fd,&bp->hdr.h.page,sizeof bp->hdr.h.attr);
	for(register int i=0;i<POOL_SIZE;i++)
	{
		BPage *tmp=(BPage *)pool_pages[i]->buf;
		if(tmp->hdr.h.page==0)
		{
			tmp->hdr.h.attr=bp->hdr.h.page;
			break;
		}
	}
	return(bp);
}

BPage *BTree::Read_Page(int page)
{
	register BPage *tmp;
	if(page==0)
		page=Root_Page();
	for(register int i=0;i<POOL_SIZE;i++)
	{
		tmp=(BPage *)pool_pages[i]->buf;
		if(tmp->hdr.h.page==page)
		{
			if(i>0)
			{
				Pool *first=pool_pages[i];
				for(;i>0;i--)
					pool_pages[i]=pool_pages[i-1];
				pool_pages[0]=first;
			}
			return((BPage *)pool_pages[0]->buf);
		}
	}

	Pool *last=pool_pages[POOL_SIZE-1];

	if(last->status>0)
		Flush((BPage *)last->buf);

	for(register int i=POOL_SIZE-1;i>0;i--)
		pool_pages[i]=pool_pages[i-1];

	pool_pages[0]=last;
	tmp=(BPage *)pool_pages[0]->buf;

	lseek(fd,page*PAGESIZE,SEEK_SET);
	if(read(fd,tmp,PAGESIZE)!=PAGESIZE)
	{
		memset(tmp,0,PAGESIZE);
		tmp->hdr.h.page=-1;
	}
	return(tmp);
}

int BTree::Write_Page(BPage *bp)
{
	for(register int i=0;i<POOL_SIZE;i++)
	{
		register BPage *tmp=(BPage *)(pool_pages[i]->buf);
		if(tmp->hdr.h.page==bp->hdr.h.page)
		{
			pool_pages[i]->status=1;
			return(0);
		}
	}
	return(Flush(bp));
}

void BTree::Flush()
{
	for(int i=0;i<POOL_SIZE;i++)
	{
		if(pool[i].status!=0)
			Flush((BPage *)pool[i].buf);
	}
}

int BTree::Flush(BPage *bp)
{
	for(register int i=0;i<POOL_SIZE;i++)
	{
		register BPage *tmp=(BPage *)(pool_pages[i]->buf);
		if(tmp->hdr.h.page==bp->hdr.h.page)
		{
			pool_pages[i]->status=0;
			break;
		}
	}
	if(fd<0)
		return(-1);
	if(bp->hdr.h.page==0)
		lseek(fd,Root_Page()*PAGESIZE,SEEK_SET);
	else
		lseek(fd,bp->hdr.h.page*PAGESIZE,SEEK_SET);
	return(write(fd,bp,PAGESIZE));
}

long BTree::Find(char *slot,long record)
{
	int i;
	return(Find_Page(slot,record,&i,1));
}

int BTree::Insert(char *slot,long record)
{
	int i;

	Lock(F_WRLCK);

	long page=Find_Page(slot,record,&i);
	if(page<0)
	{
		BPage *current=New_Page();
		page=current->hdr.h.page;
		Write_Page(current);
	}

	i=Insert(slot,record,page,i);
	Unlock(F_WRLCK);
	return(i);
}

int BTree::Delete(char *slot,long record)
{
	int i;
//        Lock(F_WRLCK);

	long page=Find_Page(slot,record,&i);
	if(page<0)
	{
		Unlock(F_WRLCK);
		return(-1);
	}
	i=Delete(slot,record,page,i);
//        Unlock(F_WRLCK);
	return(i);
}

int BTree::Delete(char *slot,long record,long page,int i)
{
	BPage *current=Read_Page(page);
	if(current->hdr.h.last>1)
	{
		i=current->del(i);
		Write_Page(current);
		return(i);
	}
	if(level<=2 && current->hdr.h.page==0) // root page
	{
		memset(current,0,PAGESIZE);
		current->hdr.f.a=f.a;
		current->hdr.f.l=f.l;
		current->hdr.h.space=PAGESIZE-32;
		Write_Page(current);
		return(0);
	}
	if(level>2)
	{
		long parent_page=path[level-2];
		current = Read_Page(parent_page);
		int l=current->hdr.f.l;
		if(current->hdr.f.a==X_TEXT)
			l=strlen(slot);
		current->find_pos(slot,l,record,&i);

		if(i>=0)
		{
			level--;
			i=Delete(slot,record,parent_page,i);
		}
	}
	return(i);
}

int BTree::Insert(char *slot,long record,long page,int i,long point)
{
	db->Wlock(0);
	BPage *current=Read_Page(page);
	int len=current->hdr.f.l;
	if(current->hdr.f.a==X_TEXT)
		len=strlen(slot);
	long p=page+1;

	if(current->hdr.h.space<len+4+(current->hdr.f.a==X_TEXT?2:0)+(current->hdr.h.left?4:0)) // not enough space
	{
		BPage *r=New_Page();    // create the new right node
		long r_page=r->hdr.h.page;

		current->split(r);      // split nodes and link pointers

		r->hdr.h.left=current->hdr.h.left;
		r->hdr.h.fdes=current->hdr.h.fdes;
		r->hdr.h.page=r_page;

		r->hdr.h.KEY.r=current->hdr.h.KEY.r;
		r->hdr.h.KEY.l=page;
		if(current->hdr.h.KEY.r)
		{
			BPage *tmp=Read_Page(current->hdr.h.KEY.r);
			tmp->hdr.h.KEY.l=r_page;
			Write_Page(tmp);
		}

		current->hdr.h.KEY.r=r_page;

		if(current->hdr.h.last<i) // insert the slot value in the right node
		{
			r->insert(slot,len,record,i-current->hdr.h.last,point);
			p=r_page+1;
		}
		else                     // ... or in the left one
		{
			current->insert(slot,len,record,i,point);
		}
		if(level<=2 && current->hdr.h.page==0) // it is a root node
		{
			BPage *l=New_Page();    // left node
			long l_page=l->hdr.h.page;
			memcpy(l,current,PAGESIZE);
			l->hdr.h.page=l_page;
			r->hdr.h.KEY.l=l_page;
			l->hdr.h.KEY.r=r_page;
			current->hdr.h.space=PAGESIZE-32;
			current->hdr.h.last=0;
			memset(current->hdr.h.data,0,PAGESIZE-32);
			current->hdr.h.left=l_page;
			current->insert(r->hdr.h.data+4,r->len(0),*((long *)r->hdr.h.data),0,r_page);
			Write_Page(l);
			Write_Page(r);
			Write_Page(current); // new root node

			// update new left page
			for(int j=0;j<l->hdr.h.last;j++)
			{
				long record=*(long *)l->pos(j);
				long seek =(db->root_size+(off_t)(record-1)*db->ss.size+sizeof (struct key));
				seek+=db->ss.field[field-1].atr.attr->wshift;
				if(record>db->max_record)
					throw -4; // BTree is currupted
				long p=l->hdr.h.page+1;
				db->Put_Buf(db->fd,seek,4,(char *)&p);
			}

			// update new right page
			for(int j=0;j<r->hdr.h.last;j++)
			{
				long record=*(long *)r->pos(j);
				long seek =(db->root_size+(off_t)(record-1)*db->ss.size+sizeof (struct key));
				seek+=db->ss.field[field-1].atr.attr->wshift;
				if(record>db->max_record)
					throw -4; // BTree is currupted
				long p=r->hdr.h.page+1;
				db->Put_Buf(db->fd,seek,4,(char *)&p);
			}

			// ... and root too
			for(int j=0;j<current->hdr.h.last;j++)
			{
				long record=*(long *)current->pos(j);
				long seek =(db->root_size+(off_t)(record-1)*db->ss.size+sizeof (struct key));
				seek+=db->ss.field[field-1].atr.attr->wshift;
				if(record>db->max_record)
					throw -4; // BTree is currupted
				long p=current->hdr.h.page+1;
				db->Put_Buf(db->fd,seek,4,(char *)&p);
			}
		}
		else
		{
			Write_Page(current);    // write down left node
			Write_Page(r);          // write down right one

// insert a new value in the parent node.
// it should be the a value of the right node

			char buf[PAGESIZE];
			long parent_page=path[level-2];
			memcpy(buf,r->hdr.h.data+4,r->len(0));
			current = Read_Page(parent_page);
			current->find_pos(buf,r->len(0),*((long *)r->hdr.h.data),&i);

			// update pointers in the Main for right page
			for(int j=0;j<r->hdr.h.last;j++)
			{
				long record=*(long *)r->pos(j);
				long seek =(db->root_size+(off_t)(record-1)*db->ss.size+sizeof (struct key));
				seek+=db->ss.field[field-1].atr.attr->wshift;
				if(record>db->max_record)
					throw -4; // BTree is currupted
				long p=r->hdr.h.page+1;
				db->Put_Buf(db->fd,seek,4,(char *)&p);
			}
			if(p!=r_page+1)
			{
				// update pointers in the Main to new value
				long seek =(db->root_size+(off_t)(record-1)*db->ss.size+sizeof (struct key));
				seek+=db->ss.field[field-1].atr.attr->wshift;
				db->Put_Buf(db->fd,seek,4,(char *)&p);
			}
			if(i>=0)
			{
				level--;
				Insert(buf,*((long *)r->hdr.h.data),parent_page,i,r_page);
			}
		}
	}
	else
	{
		current->insert(slot,len,record,i,point);
		Write_Page(current);    // write down the current node

		long seek =(db->root_size+(off_t)(record-1)*db->ss.size+sizeof (struct key));
		seek+=db->ss.field[field-1].atr.attr->wshift;
		db->Put_Buf(db->fd,seek,4,(char *)&p);
	}
	db->Unlock(0);
	return(page);
}

long BTree::Root_Page()
{
	return(0);
}

int BTree::Find_Left()
{
	register int page;
	register BPage *current;

	level=0;
	memset(path,0,sizeof path);
	Lock(F_RDLCK);
	page=Root_Page();
	int max_page=0;
	for(;;)
	{
		current=Read_Page(page);
	       // int len=current->hdr.f.l;
		if(page==0)
			max_page=current->hdr.h.attr;
		path[level++]=page;
		if(current->hdr.h.left) // it is a node
		{
			page=current->hdr.h.left;
			if(page<0 || page>max_page)     // D-tree is currupted !!!
			{
				Unlock(F_RDLCK);
				throw -3;
			}
			continue;
		}
		Unlock(F_RDLCK);
		return(page);
	}

}
int BTree::Next(int page)
{
	register BPage *current;
	current=Read_Page(page);
	return current->hdr.h.KEY.r;
}

long BTree::Find_Page(char *key,long record,int *i,int arg)
{
	int eq=0;
	register int page;
	register BPage *current;

	*i=0;
	level=0;
	memset(path,0,sizeof path);
	Lock(F_RDLCK);
	page=Root_Page();
	int max_page=0;
	for(;;)
	{
		current=Read_Page(page);
		int len=current->hdr.f.l;
		if(page==0)
		{
			max_page=current->hdr.h.attr;
			if(current->hdr.f.a==X_TEXT)
				len=strlen(key);
		}
		path[level++]=page;
		eq=current->find_pos(key,len,record,i);
		if(current->hdr.h.left) // it is a node
		{
			if(*i==0 && eq) // less than the left key
			{
				page=current->hdr.h.left;
			}
			else
			{
				register char *ch=current->pos(*i+1-(eq!=0));
				if(ch!=NULL)
					page=*((long *)(ch-4));
				else    page=-1;
			}
			if(page<0 || page>max_page)     // D-tree is currupted !!!
			{
				Unlock(F_RDLCK);
				throw -3;
			}
			continue;
		}
		Unlock(F_RDLCK);
		if(arg==0)
			return(page);
		else
			return(eq?-1:page);
	}
}

int  CX_BASE::Insert_Tree(long record,int field,char *slot)
{
	int i;
	for(i=0;i<num_btree;i++)
	{
		if(btree[i]->field==field)
			break;
	}
	if(i==num_btree)
		return -1;
	BTree *bt=btree[i];
	int page=bt->Insert(slot,record);
	bt->Flush();
	return page+1;
}

int  CX_BASE::Delete_Tree(long record,int field)
{
	int key_len=sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
	char *ch=Get_Buf(fd,seek,4);
	if(ch==NULL)
		return(-1);
	long page=int_conv(ch,4);
	if(page==0)
		return(-1); // can't delete empty value
	int i;
	for(i=0;i<num_btree;i++)
	{
		if(btree[i]->field==field)
			break;
	}
	if(i==num_btree)
		return -1;
	BTree *bt=btree[i];
	BPage *current=bt->Read_Page(page-1);
	int p=current->get_pos(record);
	if(p<0)
		return -1;
	ch=current->pos(p);
	bt->Delete(ch+4,record);
	page=0;
	return Put_Buf(fd,seek,4,(char *)&page);

}

int  CX_BASE::Put_Tree(long record,int field,char *slot)
{
	if(field==0 || field>context->ptm || ss.field[field-1].b==0)
		return(-1);

	char *ch=Get_Tree(record,field);
	int len=ss.field[field-1].l;
	if(ss.field[field-1].a==X_TEXT)
		len=strlen(slot);
	if(ch!=NULL && !::Compare(&ss.field[field-1],ch,slot,len))
	{
		return 0;  // nothing changed
	}
	if(ch!=NULL)
	{
		Delete_Tree(record,field); // delete old value
	}
	long page=Insert_Tree(record,field,slot);
	if(page<0)
		return -2;
/*
	int key_len=sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
	return Put_Buf(fd,seek,4,(char *)&page);
*/
	return 0;
}
int  CX_BASE::Put_Tree(long record,struct sla *sla,char *slot)
{
	return Put_Tree(record,sla->n,slot);
}
char *CX_BASE::Get_Tree(long record,int field)
{
	if(field==0 || field>context->ptm || ss.field[field-1].b==0)
		return(NULL);
	int key_len=sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
	char *ch=Get_Buf(fd,seek,4);
	if(ch==NULL)
		return(NULL);
	long page=int_conv(ch,4);
	if(page==0)
		return(NULL);
	int i;
	page-=1;
	for(i=0;i<num_btree;i++)
	{
		if(btree[i]->field==field)
			break;
	}
	if(i==num_btree)
		return NULL;
	BTree *bt=btree[i];

	BPage *current=bt->Read_Page(page);
	int p=current->get_pos(record);
	if(p<0)
		return NULL;
	ch=current->pos(p);
	if(ch==NULL)
		return NULL;
	return ch+4;
}
int  CX_BASE::Get_Tree(long record,int field,char *&slot)
{
	if(slot!=NULL)
		free(slot);
	slot=NULL;
	if(field==0 || field>context->ptm || ss.field[field-1].b==0)
		return(-1);
	int key_len=sizeof (struct key);
	off_t seek=root_size+(off_t)(record-1)*ss.size+key_len+ss.field[field-1].atr.attr->wshift;
	char *ch=Get_Buf(fd,seek,4);
	if(ch==NULL)
		return(-1);
	long page=int_conv(ch,4);
	if(page==0)
		return(-1);
	page-=1;
	int i;
	for(i=0;i<num_btree;i++)
	{
		if(btree[i]->field==field)
			break;
	}
	if(i==num_btree)
		return -1;
	BTree *bt=btree[i];
	BPage *current=bt->Read_Page(page);
	int p=current->get_pos(record);
	if(p<0)
		return -1;
	ch=current->pos(p);
	if(ch==NULL)
		return -1;
	int len=current->len(p);
	if(len<=0 || len>PAGESIZE)
		return -1;
	slot=(char *)malloc(len);
	memcpy(slot,ch+4,len);
	return len;
}

long BTree::Find_First(char *key)
{
	int i;
	register BPage *current;
	int page=Find_Page(key,0,&i);
	if(page<0)
		return -1;
	current=Read_Page(page);
	if(i<0 || i>current->hdr.h.last)
		return -1;
	return *(long *)current->pos(i);
}

long CX_BASE::Find_First(int field, char *pattern)
{
	if(field==0 || field>context->ptm)
		return(-1);
	if(ss.field[field-1].b)
	{
		int i;
		for(i=0;i<num_btree;i++)
		{
			if(btree[i]->field==field)
				break;
		}
		if(i==num_btree)
			return -1;
		BTree *bt=btree[i];
		return bt->Find_First(pattern);
	}
	CX_FIND find(this);
	return find.Find_First(field,pattern,strchr(pattern,'*')!=NULL);
}

long CX_BASE::SortBP(int field,selection *select,int nap)
{
	if(field==0 || field>context->ptm)
		return(-1);
	if(ss.field[field-1].b==0)
		return -1;
	int i;
	for(i=0;i<num_btree;i++)
	{
		if(btree[i]->field==field)
			break;
	}
	if(i==num_btree)
		return -1;

	BTree *bt=btree[i];
	i= bt->Find_Left();

	select->num_index=0; //last_cadr();
	select->index=(long *)realloc(select->index,last_cadr()*sizeof (long));

	register BPage *current;
	current=bt->Read_Page(i);

	for(int j=0;j<current->hdr.h.last;j++)
		select->index[select->num_index++]=*(long *)current->pos(j);
	for(;;)
	{
		i=bt->Next(i);

		if(i==0)
			break;
		current=bt->Read_Page(i);

		for(int j=0;j<current->hdr.h.last;j++)
			select->index[select->num_index++]=*(long *)current->pos(j);
	}
	if(nap>0)
	{
		for(int i=0;i<select->num_index/2;i++)
		{
			long tmp=select->index[i];
			select->index[i]=select->index[select->num_index-i-1];
			select->index[select->num_index-i-1]=tmp;
		}
	}
	return select->num_index;
}

void field_to_str(struct field f,char *ch,char *slot)
{
	char str[256];

	switch(f.a)
	{
		case X_STRING:
			strncpy(slot,ch,f.l);
			break;
		case X_TEXT:
			strncpy(slot,ch,256);
			break;
		case X_DATE:
			get_date(date_from_rec(ch),slot);
			break;
		case X_TIME:
			if(f.l==3)
				get_time(date_from_rec(ch),slot);
			else
				get_unix_time((int)int_conv(ch,f.l),slot);
			break;
		case X_DOUBLE:
		{
			if(f.n>16)
				strcpy(str,"%.6f");
			else
				sprintf(str,"%%.%df",f.n);
			sprintf(slot,str,*(double *)ch);
			break;
		}
		case X_FLOAT:
		{
			if(f.n>6)
				strcpy(str,"%.6f");
			else
				sprintf(str,"%%.%df",f.n);
			sprintf(slot,str,*(float *)ch);
			break;
		}
		case X_INTEGER:
		case X_UNSIGNED:
		{
			dlong a=0;
 
			if(f.a==X_UNSIGNED)
				bcopy(ch,&a,f.l);
			else
				a=int_conv(ch,f.l);
			if(f.n)
			{
				int j;
				if(a>0)
					sprintf(str,"%%0%dlld",f.n);
				else
					sprintf(str,"%%0%dlld",f.n+1);
				sprintf(slot,str,a);
				j=strlen(slot);
				bcopy(slot+(j-f.n),slot+(j-f.n+1),f.n+1);
				slot[j-f.n]='.';
				slot[j+1]=0;
			}
			else
#ifdef FREEBSD
				sprintf(slot,"%s",lota(a));
#else
				sprintf(slot,"%lld",a);
#endif
			break;
		}
		case X_POINTER:
		{
			long page=int_conv(ch,f.l);
			sprintf(slot,"#%d",(int)page);
			break;
		}
		case X_VARIANT:
		{
			long page=0,spage=0;
			bcopy(ch,&spage,f.n);
			bcopy(ch+f.n,&page,f.l-f.n);
			sprintf(slot,"#%d:%d",(int)spage,(int)page);
			break;
		}
		default:
			memcpy(slot,ch,f.l);
	}
}

int CX_BASE::FindBP(int field,char *pattern,long * &select,int obr)
{
	int i;
	int num_rez=0;

	if(field==0 || field>context->ptm)
		return 0;
	if(ss.field[field-1].b==0)
		return 0;
	for(i=0;i<num_btree;i++)
	{
		if(btree[i]->field==field)
			break;
	}
	if(i==num_btree)
		return 0;

	BTree *bt=btree[i];

	register BPage *current;
	int page=bt->Find_Page(pattern,0,&i);

	if(page<0)
		return 0;
	current=bt->Read_Page(page);
	if(i<0 || i>current->hdr.h.last)
		return 0;

	int j;
	for(j=0;j<current->hdr.h.last;j++) // first block
	{
		int r=current->cmp(pattern,current->hdr.f.l,current->pos(j)+4,current->len(j),0);
		if(r>0 && obr==0 || (r>=0 && obr==1))
			continue;
		if(r && obr==0)
			return num_rez;
		select=(long *)realloc(select,++num_rez*sizeof (long));
		select[num_rez-1]=*(long *)current->pos(j);
	}
	if(!num_rez)
		return 0;
	for(;current->hdr.h.KEY.r;)
	{
		page=bt->Next(page);

		if(page==0)
			break;
		current=bt->Read_Page(page);

		for(j=0;j<current->hdr.h.last;j++)
		{
/*
			field_to_str(current->hdr.f,current->pos(j)+4,key);
			if(strdif((unsigned char *)pattern,(unsigned char *)key,strlen(key)))
*/
			int r;
			if((r=current->cmp(pattern,current->hdr.f.l,current->pos(j)+4,current->len(j),0)) && obr==0)
			{
				return num_rez;
			}
			select=(long *)realloc(select,++num_rez*sizeof (long));
			select[num_rez-1]=*(long *)current->pos(j);
		}
	}
	return num_rez;
}
