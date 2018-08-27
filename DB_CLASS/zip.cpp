/*
			  DBMS ConteXt V.6.0
		       ConteXt library libcxdb.a

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:zip.cpp
*/
 
#include "StdAfx.h" 
#include "CX_BASE.h" 
 
#ifdef PATROL 
#include <mpatrol.h> 
#endif 
 
#include "tailor.h" 
#include "gzip.h" 
 
//void bcopy(const void *s1, void *s2, size_t n); 
 
/* gzip flag byte */ 
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */ 
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */ 
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */ 
#define COMMENT      0x10 /* bit 4 set: file comment present */ 
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */ 
#define RESERVED     0xC0 /* bit 6,7:   reserved */ 
 
 
int file_read(char *buf,unsigned size); 
void ct_init(unsigned short  *attr,int *methodp); 
int ct_tally (int dist,int lc); 
int huft_build(unsigned *b,unsigned n,unsigned  s,unsigned short *d,unsigned short *e, struct huft **t, int *m); 
int huft_free(struct huft *t); 
int inflate_codes(struct huft *tl,struct huft *td,int bl,int bd); 
int inflate_stored(); 
int inflate_fixed(); 
int inflate_dynamic(); 
int inflate_block(int *e); 
int inflate(); 
void lm_init (int pack_level,unsigned short *flags); 
int longest_match(IPos cur_match); 
void clear_bufs(); 
void flush_outbuf(); 
void flush_window(); 
void write_buf(int fd,voidp  buf,unsigned  cnt); 
void bi_init (); 
void send_bits(int value,int length); 
unsigned bi_reverse(unsigned code,int len); 
void bi_windup(); 
void copy_block(char *buf,unsigned len,int header); 
int fill_inbuf(int eof_ok); 
 
 
static unsigned outbuf_seek; 
static unsigned inbuf_seek; 
static char *in_buf; 
static char **out_buf; 
static unsigned t_read; 
static int part_nb=0;          /* number of parts in .gz file */ 
 
int compress(char *in, int len,char *&out) 
{ 
	return(zip(0,0,len,out,in)); 
} 
int decompress(char *in,int len,char *&out) 
{ 
	return(unzip(0,0,len,out,in)); 
} 
 
local int get_method() 
{ 
	char magic[2]; 
 
	part_nb++;                   /* number of parts in gzip file */ 
 
	magic[0] = (char)get_byte(); 
	magic[1] = (char)get_byte(); 
 
	method = -1;   /* unknown yet */ 
	header_bytes = 0; 
 
	if(memcmp(magic,MY_GZIP_MAGIC,2)==0) 
	{ 
		method=DEFLATED; 
		work = unzip; 
 
		(void)get_byte();   /* Ignore decompress size */ 
		(void)get_byte();   /* Ignore decompress size */ 
		(void)get_byte();   /* Ignore decompress size */ 
		(void)get_byte();   /* Ignore decompress size */ 
 
	} 
	else if(memcmp(magic,GZIP_MAGIC,2)==0) 
	{ 
 
		method = (int)get_byte(); 
		if (method != DEFLATED) 
		    return -1; 
		work = unzip; 
		flags  = (uch)get_byte(); 
 
		if ((flags & ENCRYPTED) != 0) 
		    return -1; 
		if ((flags & CONTINUATION) != 0) 
		    return -1; 
		if ((flags & RESERVED) != 0) 
		    return -1; 
 
		get_byte();     /* Ignore timestamp */ 
		get_byte(); 
		get_byte(); 
		get_byte(); 
 
		(void)get_byte();  /* Ignore extra flags for the moment */ 
		(void)get_byte();  /* Ignore OS type for the moment */ 
 
		if ((flags & CONTINUATION) != 0) 
		{ 
		    unsigned part = (unsigned)get_byte(); 
		    part |= ((unsigned)get_byte())<<8; 
		} 
		if ((flags & EXTRA_FIELD) != 0) 
		{ 
		    unsigned len = (unsigned)get_byte(); 
		    len |= ((unsigned)get_byte())<<8; 
		    while (len--) (void)get_byte(); 
		} 
 
		if ((flags & ORIG_NAME) != 0) 
		{ 
			char c; 
			do {c=get_byte();} while (c != 0); 
		} 
 
		if ((flags & COMMENT) != 0) 
			while (get_char() != 0); 
		if (part_nb == 1) 
			header_bytes = inptr + 2*sizeof(long); /* include crc and size */ 
	} 
	return method; 
} 
 
local ulg crc;       /* crc on uncompressed file data */ 
 
int zip(int in,int out,int  ilen,char *&buf,char *ibuf) 
{ 
//        uch  flags = 0; 
	ush  attr = 0; 
	ush  deflate_flags = 0; 
 
	ifd = in; 
	ofd = out; 
	outcnt = 0; 
	ifile_size = ilen; 
 
#ifdef DYN_ALLOC 
	ALLOC(uch, inbuf,  INBUFSIZ +INBUF_EXTRA); 
	ALLOC(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA); 
	ALLOC(ush, d_buf,  DIST_BUFSIZE); 
	ALLOC(uch, window, 2L*WSIZE); 
	ALLOC(ush, tab_prefix, 1L<<BITS); 
#endif 
	clear_bufs(); /* clear input and output buffers */ 
	if(!ifd) 
		in_buf=ibuf; 
	if(!ofd) 
		out_buf=&buf; 
 
	method = DEFLATED; 
	put_byte(MY_GZIP_MAGIC[0]); /* magic header */ 
	put_byte(MY_GZIP_MAGIC[1]); 
	put_long(ifile_size);    /* input file size */ 
 
	crc = updcrc(0, 0); 
	bi_init(); 
	ct_init(&attr, &method); 
	lm_init(level, &deflate_flags); 
 
	header_bytes = (long)outcnt; 
 
	(void)deflate(); 
 
	put_long(crc); 
	put_long(isize); 
	header_bytes += 2*sizeof(long); 
	flush_outbuf(); 
 
#ifdef DYN_ALLOC 
	FREE(inbuf); 
	FREE(outbuf); 
	FREE(d_buf); 
	FREE(window); 
	FREE(tab_prefix); 
#endif 
	return(bytes_out); 
} 
 
int file_read(char *buf,unsigned size) 
{ 
	unsigned len; 
 
	if(ifile_size && t_read+size>(unsigned)ifile_size) 
		if(!(size=ifile_size-t_read)) 
			return(0); 
	if(!ifd) 
	{ 
		memcpy(buf,in_buf+inbuf_seek,size); 
		inbuf_seek+=size; 
		len=size; 
	} 
	else 
#ifdef WIN32 
	        len = _read(ifd, buf, size); 
#else 
	        len = read(ifd, buf, size); 
#endif 
	t_read+=len; 
 
	if (len == (unsigned)(-1) || len == 0) 
	        return((int)len); 
 
	crc = updcrc((uch*)buf, len); 
	isize += (ulg)len; 
	return((int)len); 
} 
 
 
int unzip(int in,int out,int ilen,char *&buf,char *ibuf) 
{ 
	ulg orig_crc = 0;       /* original crc */ 
	ulg orig_len = 0;       /* original uncompressed length */ 
	int n; 
	uch ubuf[EXTHDR];        /* extended local header */ 
 
	ifd = in; 
	ofd = out; 
 
	ifile_size = ilen; 
#ifdef DYN_ALLOC 
	ALLOC(uch, inbuf,  INBUFSIZ +INBUF_EXTRA); 
	ALLOC(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA); 
	ALLOC(ush, d_buf,  DIST_BUFSIZE); 
	ALLOC(uch, window, 2L*WSIZE); 
	ALLOC(ush, tab_prefix, 1L<<BITS); 
	updcrc(NULL, 0); 
#endif 
	clear_bufs(); /* clear input and output buffers */ 
	if(!in) 
		in_buf=ibuf; 
	if(!out) 
		out_buf=&buf; 
	bi_init(); 
	method = get_method(); 
 
	/* Decompress */ 
	if (method == DEFLATED) 
	{ 
		int res = inflate(); 
 
		if (res == 3) 
			error("out of memory"); 
		else if (res != 0) 
			error("invalid compressed data--format violated"); 
	}  
	else 
		error("internal error, invalid method"); 
 
	for (n = 0; n < 8; n++) 
		ubuf[n] = (uch)get_byte(); 
	orig_crc = LG(ubuf); 
	orig_len = LG(ubuf+4); 
 
/* 
	if (orig_crc != updcrc(outbuf, 0)) 
		error("invalid compressed data--crc error"); 
	if (orig_len != (ulg)bytes_out) 
		error("invalid compressed data--length error"); 
*/ 
#ifdef DYN_ALLOC 
	FREE(inbuf); 
	FREE(outbuf); 
	FREE(d_buf); 
	FREE(window); 
	FREE(tab_prefix); 
#endif 
	return(bytes_out); 
} 
 
void ct_init(ush  *attr,int *methodp) 
{ 
	int n; 
	int bits; 
	int length; 
	int code; 
	int dist; 
 
	file_type = attr; 
	file_method = methodp; 
	compressed_len = input_len = 0L; 
 
	if (static_dtree[0].Len != 0) return; /* ct_init already called */ 
 
	length = 0; 
	for (code = 0; code < LENGTH_CODES-1; code++) 
	{ 
		base_length[code] = length; 
		for (n = 0; n < (1<<extra_lbits[code]); n++) 
			length_code[length++] = (uch)code; 
	} 
	length_code[length-1] = (uch)code; 
 
	dist = 0; 
	for (code = 0 ; code < 16; code++) 
	{ 
		base_dist[code] = dist; 
		for (n = 0; n < (1<<extra_dbits[code]); n++) 
			dist_code[dist++] = (uch)code; 
	} 
	dist >>= 7; 
	for ( ; code < D_CODES; code++) 
	{ 
		base_dist[code] = dist << 7; 
		for (n = 0; n < (1<<(extra_dbits[code]-7)); n++) 
			dist_code[256 + dist++] = (uch)code; 
	} 
	for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0; 
	n = 0; 
	while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++; 
	while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++; 
	while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++; 
	while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++; 
	gen_codes((ct_data near *)static_ltree, L_CODES+1); 
 
	for (n = 0; n < D_CODES; n++) 
	{ 
		static_dtree[n].Len = 5; 
		static_dtree[n].Code = bi_reverse(n, 5); 
	} 
	init_block(); 
} 
 
local void init_block() 
{ 
	int n; 
 
	for (n = 0; n < L_CODES;  n++) dyn_ltree[n].Freq = 0; 
	for (n = 0; n < D_CODES;  n++) dyn_dtree[n].Freq = 0; 
	for (n = 0; n < BL_CODES; n++) bl_tree[n].Freq = 0; 
 
	dyn_ltree[END_BLOCK].Freq = 1; 
	opt_len = static_len = 0L; 
	last_lit = last_dist = last_flags = 0; 
	flags = 0;  
	flag_bit = 1; 
} 
 
local void pqdownheap(ct_data near *tree,int k) 
{ 
	int v = heap[k]; 
	int j = k << 1; 
	while (j <= heap_len) 
	{ 
		if (j < heap_len && smaller(tree, heap[j+1], heap[j])) j++; 
		if (smaller(tree, v, heap[j])) break; 
		heap[k] = heap[j];   
		k = j; 
		j <<= 1; 
	} 
	heap[k] = v; 
} 
 
local void gen_bitlen(tree_desc near *desc) 
{ 
	ct_data near *tree  = desc->dyn_tree; 
	int near *extra     = desc->extra_bits; 
	int base            = desc->extra_base; 
	int max_code        = desc->max_code; 
	int max_length      = desc->max_length; 
	ct_data near *stree = desc->static_tree; 
	int h; 
	int n, m; 
	int bits; 
	int xbits; 
	ush f; 
	int overflow = 0; 
 
	for (bits = 0; bits <= MAX_BITS; bits++) 
		bl_count[bits] = 0; 
	tree[heap[heap_max]].Len = 0; /* root of the heap */ 
 
	for (h = heap_max+1; h < HEAP_SIZE; h++) 
	{ 
		n = heap[h]; 
		bits = tree[tree[n].Dad].Len + 1; 
		if (bits > max_length) 
		        bits = max_length, overflow++; 
		tree[n].Len = (ush)bits; 
		if (n > max_code) 
		        continue; /* not a leaf node */ 
		bl_count[bits]++; 
		xbits = 0; 
		if (n >= base) 
			xbits = extra[n-base]; 
		f = tree[n].Freq; 
		opt_len += (ulg)f * (bits + xbits); 
		if (stree) 
		        static_len += (ulg)f * (stree[n].Len + xbits); 
	} 
	if (overflow == 0) return; 
 
	do 
	{ 
		bits = max_length-1; 
		while (bl_count[bits] == 0) bits--; 
		bl_count[bits]--; 
		bl_count[bits+1] += 2; 
		bl_count[max_length]--; 
		overflow -= 2; 
	}  
	while (overflow > 0); 
 
	for (bits = max_length; bits != 0; bits--) 
	{ 
		n = bl_count[bits]; 
		while (n != 0) 
		{ 
			m = heap[--h]; 
			if (m > max_code) continue; 
			if (tree[m].Len != (unsigned) bits) 
			{ 
				opt_len += ((long)bits-(long)tree[m].Len)*(long)tree[m].Freq; 
				tree[m].Len = (ush)bits; 
			} 
			n--; 
		} 
	} 
} 
 
local void gen_codes (ct_data near *tree,int max_code) 
{ 
	ush next_code[MAX_BITS+1]; 
	ush code = 0; 
	int bits; 
	int n; 
 
	for (bits = 1; bits <= MAX_BITS; bits++) 
		next_code[bits] = code = (code + bl_count[bits-1]) << 1; 
	for (n = 0;  n <= max_code; n++) 
	{ 
		int len = tree[n].Len; 
		if (len == 0) continue; 
		tree[n].Code = bi_reverse(next_code[len]++, len); 
	} 
} 
 
local void build_tree(tree_desc near *desc) 
{ 
	ct_data near *tree   = desc->dyn_tree; 
	ct_data near *stree  = desc->static_tree; 
	int elems            = desc->elems; 
	int n, m; 
	int max_code = -1; 
	int node = elems; 
 
	heap_len = 0, heap_max = HEAP_SIZE; 
 
	for (n = 0; n < elems; n++) 
	{ 
		if (tree[n].Freq != 0) 
		{ 
			heap[++heap_len] = max_code = n; 
			depth[n] = 0; 
		}  
		else 
			tree[n].Len = 0; 
	} 
 
	while (heap_len < 2) 
	{ 
		int j = heap[++heap_len] = (max_code < 2 ? ++max_code : 0); 
		tree[j].Freq = 1; 
		depth[j] = 0; 
		opt_len--;  
		if (stree) 
			static_len -= stree[j].Len; 
	} 
	desc->max_code = max_code; 
 
	for (n = heap_len/2; n >= 1; n--) 
	        pqdownheap(tree, n); 
	do { 
		pqremove(tree, n); 
		m = heap[SMALLEST]; 
 
		heap[--heap_max] = n; 
		heap[--heap_max] = m; 
 
		tree[node].Freq = tree[n].Freq + tree[m].Freq; 
		depth[node] = (uch) (MAX(depth[n], depth[m]) + 1); 
		tree[n].Dad = tree[m].Dad = (ush)node; 
		heap[SMALLEST] = node++; 
		pqdownheap(tree, SMALLEST); 
 
	}  
	while (heap_len >= 2); 
	heap[--heap_max] = heap[SMALLEST]; 
	gen_bitlen((tree_desc near *)desc); 
	gen_codes ((ct_data near *)tree, max_code); 
} 
 
local void scan_tree (ct_data near *tree,int max_code) 
{ 
	int n; 
	int prevlen = -1; 
	int curlen; 
	int nextlen = tree[0].Len; 
	int count = 0; 
	int max_count = 7; 
	int min_count = 4; 
 
	if (nextlen == 0) max_count = 138, min_count = 3; 
	tree[max_code+1].Len = (ush)0xffff; /* guard */ 
 
	for (n = 0; n <= max_code; n++) 
	{ 
		curlen = nextlen;  
		nextlen = tree[n+1].Len; 
		if (++count < max_count && curlen == nextlen) 
			continue; 
		else if (count < min_count) 
			bl_tree[curlen].Freq += count; 
		else if (curlen != 0) 
		{ 
			if (curlen != prevlen) bl_tree[curlen].Freq++; 
			bl_tree[REP_3_6].Freq++; 
		}  
		else if (count <= 10) 
			bl_tree[REPZ_3_10].Freq++; 
		else 
			bl_tree[REPZ_11_138].Freq++; 
		count = 0;  
		prevlen = curlen; 
		if (nextlen == 0) 
			max_count = 138, min_count = 3; 
		else if (curlen == nextlen) 
			max_count = 6, min_count = 3; 
		else 
			max_count = 7, min_count = 4; 
	} 
} 
 
local void send_tree (ct_data near *tree,int max_code) 
{ 
	int n; 
	int prevlen = -1; 
	int curlen; 
	int nextlen = tree[0].Len; 
	int count = 0; 
	int max_count = 7; 
	int min_count = 4; 
 
	if (nextlen == 0) max_count = 138, min_count = 3; 
 
	for (n = 0; n <= max_code; n++) 
	{ 
		curlen = nextlen;  
		nextlen = tree[n+1].Len; 
		if (++count < max_count && curlen == nextlen) 
			continue; 
		else if (count < min_count) 
		{ 
			do 
			{ 
				send_code(curlen, bl_tree); 
			} 
			while (--count != 0); 
 
		}  
		else if (curlen != 0) 
		{ 
			if (curlen != prevlen) 
			{ 
				send_code(curlen, bl_tree);  
				count--; 
			} 
			send_code(REP_3_6, bl_tree);  
			send_bits(count-3, 2); 
 
		}  
		else if (count <= 10) 
		{ 
			send_code(REPZ_3_10, bl_tree);  
			send_bits(count-3, 3); 
		}  
		else 
		{ 
			send_code(REPZ_11_138, bl_tree);  
			send_bits(count-11, 7); 
		} 
		count = 0;  
		prevlen = curlen; 
		if (nextlen == 0) 
			max_count = 138, min_count = 3; 
		else if (curlen == nextlen) 
			max_count = 6, min_count = 3; 
		else 
			max_count = 7, min_count = 4; 
	} 
} 
 
local int build_bl_tree() 
{ 
	int max_blindex; 
 
	scan_tree((ct_data near *)dyn_ltree, l_desc.max_code); 
	scan_tree((ct_data near *)dyn_dtree, d_desc.max_code); 
 
	build_tree((tree_desc near *)(&bl_desc)); 
	for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) 
		if (bl_tree[bl_order[max_blindex]].Len != 0) break; 
	opt_len += 3*(max_blindex+1) + 5+5+4; 
	return(max_blindex); 
} 
 
local void send_all_trees(int lcodes,int dcodes,int blcodes) 
{ 
	int rank; 
 
	send_bits(lcodes-257, 5); 
	send_bits(dcodes-1,   5); 
	send_bits(blcodes-4,  4); 
	for (rank = 0; rank < blcodes; rank++) 
		send_bits(bl_tree[bl_order[rank]].Len, 3); 
	send_tree((ct_data near *)dyn_ltree, lcodes-1); 
 
	send_tree((ct_data near *)dyn_dtree, dcodes-1); 
} 
 
ulg flush_block(char *buf,ulg stored_len,int eof) 
{ 
	ulg opt_lenb, static_lenb; 
	int max_blindex; 
 
	flag_buf[last_flags] = flags; 
 
	if (*file_type == (ush)UNKNOWN) set_file_type(); 
 
	build_tree((tree_desc near *)(&l_desc)); 
	build_tree((tree_desc near *)(&d_desc)); 
	max_blindex = build_bl_tree(); 
 
	opt_lenb = (opt_len+3+7)>>3; 
	static_lenb = (static_len+3+7)>>3; 
	input_len += stored_len; 
 
	if (static_lenb <= opt_lenb) opt_lenb = static_lenb; 
 
	if (stored_len <= opt_lenb && eof && compressed_len == 0L && seekable()) 
	{ 
		if (buf == (char*)0) error ("block vanished"); 
 
		copy_block(buf, (unsigned)stored_len, 0); 
		compressed_len = stored_len << 3; 
		*file_method = STORED; 
 
	}  
	else if (stored_len+4 <= opt_lenb && buf != (char*)0) 
	{ 
		send_bits((STORED_BLOCK<<1)+eof, 3); 
		compressed_len = (compressed_len + 3 + 7) & ~7L; 
		compressed_len += (stored_len + 4) << 3; 
		copy_block(buf, (unsigned)stored_len, 1); /* with header */ 
 
	} 
	else if (static_lenb == opt_lenb) 
	{ 
		send_bits((STATIC_TREES<<1)+eof, 3); 
		compress_block((ct_data near *)static_ltree, (ct_data near *)static_dtree); 
		compressed_len += 3 + static_len; 
	} 
	else 
	{ 
		send_bits((DYN_TREES<<1)+eof, 3); 
		send_all_trees(l_desc.max_code+1, d_desc.max_code+1, max_blindex+1); 
		compress_block((ct_data near *)dyn_ltree, (ct_data near *)dyn_dtree); 
		compressed_len += 3 + opt_len; 
	} 
	init_block(); 
 
	if (eof) 
	{ 
		bi_windup(); 
		compressed_len += 7; 
	} 
	return compressed_len >> 3; 
} 
int ct_tally (int dist,int lc) 
{ 
	l_buf[last_lit++] = (uch)lc; 
	if (dist == 0) 
		dyn_ltree[lc].Freq++; 
	else 
	{ 
		dist--; 
		dyn_ltree[length_code[lc]+LITERALS+1].Freq++; 
		dyn_dtree[d_code(dist)].Freq++; 
 
		d_buf[last_dist++] = (ush)dist; 
		flags |= flag_bit; 
	} 
	flag_bit <<= 1; 
 
	if ((last_lit & 7) == 0) 
	{ 
		flag_buf[last_flags++] = flags; 
		flags = 0, flag_bit = 1; 
	} 
	if ((last_lit & 0xfff) == 0) 
	{ 
		ulg out_length = (ulg)last_lit*8L; 
		ulg in_length = (ulg)strstart-block_start; 
		int dcode; 
		for (dcode = 0; dcode < D_CODES; dcode++) 
			out_length += (ulg)dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]); 
		out_length >>= 3; 
		if (last_dist < last_lit/2 && out_length < in_length/2) return 1; 
	} 
	return (last_lit == LIT_BUFSIZE-1 || last_dist == DIST_BUFSIZE); 
} 
 
local void compress_block(ct_data near *ltree,ct_data near *dtree) 
{ 
	unsigned dist; 
	int lc; 
	unsigned lx = 0; 
	unsigned dx = 0; 
	unsigned fx = 0; 
	uch flag = 0; 
	unsigned code; 
	int extra; 
 
	if (last_lit != 0) do 
	{ 
		if ((lx & 7) == 0) flag = flag_buf[fx++]; 
		lc = l_buf[lx++]; 
		if ((flag & 1) == 0) 
			send_code(lc, ltree); 
		else 
		{ 
			code = length_code[lc]; 
			send_code(code+LITERALS+1, ltree); 
			extra = extra_lbits[code]; 
			if (extra != 0) 
			{ 
				lc -= base_length[code]; 
				send_bits(lc, extra); 
			} 
			dist = d_buf[dx++]; 
			code = d_code(dist); 
			send_code(code, dtree); 
			extra = extra_dbits[code]; 
			if (extra != 0) 
			{ 
				dist -= base_dist[code]; 
				send_bits(dist, extra); 
			} 
		} 
		flag >>= 1; 
	}  
	while (lx < last_lit); 
 
	send_code(END_BLOCK, ltree); 
} 
 
local void set_file_type() 
{ 
	int n = 0; 
	unsigned ascii_freq = 0; 
	unsigned bin_freq = 0; 
	while (n < 7)        bin_freq += dyn_ltree[n++].Freq; 
	while (n < 128)    ascii_freq += dyn_ltree[n++].Freq; 
	while (n < LITERALS) bin_freq += dyn_ltree[n++].Freq; 
	*file_type = bin_freq > (ascii_freq >> 2) ? BIN : ASCII; 
} 
 
int huft_build(unsigned *b,unsigned n,unsigned  s,ush *d,ush *e, struct huft **t, int *m) 
{ 
	unsigned a; 
	unsigned c[BMAX+1]; 
	unsigned f; 
	int g; 
	int h; 
	register unsigned i; 
	register unsigned j; 
	register int k; 
	int l; 
	register unsigned *p; 
	register struct huft *q; 
	struct huft r; 
	struct huft *u[BMAX]; 
	unsigned v[N_MAX]; 
	register int w; 
	unsigned x[BMAX+1]; 
	unsigned *xp; 
	int y; 
	unsigned z; 
 
	memzero(c, sizeof(c)); 
	p = b; 
	i = n; 
	do 
	{ 
		c[*p]++; 
		p++; 
	} 
	while (--i); 
	if (c[0] == n) 
	{ 
		*t = (struct huft *)NULL; 
		*m = 0; 
		return 0; 
	} 
	l = *m; 
	for (j = 1; j <= BMAX; j++) 
		if (c[j]) 
			break; 
	k = j; 
	if ((unsigned)l < j) 
		l = j; 
	for (i = BMAX; i; i--) 
		if (c[i]) 
			break; 
	g = i; 
	if ((unsigned)l > i) 
		l = i; 
	*m = l; 
 
	for (y = 1 << j; j < i; j++, y <<= 1) 
		if ((y -= c[j]) < 0) 
			return 2;                 /* bad input: more codes than bits */ 
	if ((y -= c[i]) < 0) 
		return 2; 
	c[i] += y; 
 
	x[1] = j = 0; 
	p = c + 1; 
	xp = x + 2; 
	while (--i) 
		*xp++ = (j += *p++); 
 
	p = b; 
	i = 0; 
	do 
	{ 
		if ((j = *p++) != 0) 
			v[x[j]++] = i; 
	} 
	while (++i < n); 
 
 
	x[0] = i = 0; 
	p = v; 
	h = -1; 
	w = -l; 
	u[0] = (struct huft *)NULL; 
	q = (struct huft *)NULL; 
	z = 0; 
 
	for (; k <= g; k++) 
	{ 
		a = c[k]; 
		while (a--) 
		{ 
			while (k > w + l) 
			{ 
				h++; 
				w += l; 
 
				z = (z = g - w) > (unsigned)l ? l : z; 
				if ((f = 1 << (j = k - w)) > a + 1) 
				{ 
					f -= a + 1; 
					xp = c + k; 
					while (++j < z) 
					{ 
						if ((f <<= 1) <= *++xp) 
							break; 
						f -= *xp; 
					} 
				} 
				z = 1 << j; 
 
				if ((q = (struct huft *)malloc((z + 1)*sizeof(struct huft))) == (struct huft *)NULL) 
				{ 
					if (h) 
						huft_free(u[0]); 
					return 3; 
				} 
				hufts += z + 1; 
				*t = q + 1; 
				*(t = &(q->v.t)) = (struct huft *)NULL; 
				u[h] = ++q; 
 
				if (h) 
				{ 
					x[h] = i; 
					r.b = (uch)l; 
					r.e = (uch)(16 + j); 
					r.v.t = q; 
					j = i >> (w - l); 
					u[h-1][j] = r; 
				} 
			} 
 
			r.b = (uch)(k - w); 
			if (p >= v + n) 
				r.e = 99; 
			else if (*p < s) 
			{ 
				r.e = (uch)(*p < 256 ? 16 : 15); 
				r.v.n = (ush)(*p); 
				p++; 
			} 
			else 
			{ 
				r.e = (uch)e[*p - s]; 
				r.v.n = d[*p++ - s]; 
			} 
 
			f = 1 << (k - w); 
			for (j = i >> w; j < z; j += f) 
				q[j] = r; 
 
			for (j = 1 << (k - 1); i & j; j >>= 1) 
				i ^= j; 
			i ^= j; 
 
			while ((i & ((1 << w) - 1)) != x[h]) 
			{ 
				h--;                    /* don't need to update q */ 
				w -= l; 
			} 
		} 
	} 
 
	return y != 0 && g != 1; 
} 
 
int huft_free(struct huft *t) 
{ 
	register struct huft *p, *q; 
 
	p = t; 
	while (p != (struct huft *)NULL) 
	{ 
		q = (--p)->v.t; 
		free((char*)p); 
		p = q; 
	} 
	return 0; 
} 
int inflate_codes(struct huft *tl,struct huft *td,int bl,int bd) 
{ 
	register unsigned e; 
	unsigned n, d; 
	unsigned w; 
	struct huft *t; 
	unsigned ml, md; 
	register ulg b; 
	register unsigned k; 
 
	b = bb; 
	k = bk; 
	w = wp; 
 
	ml = mask_bits[bl]; 
	md = mask_bits[bd]; 
	for (;;) 
	{ 
		NEEDBITS((unsigned)bl) 
		if ((e = (t = tl + ((unsigned)b & ml))->e) > 16) 
		do 
		{ 
			if (e == 99) 
				return 1; 
			DUMPBITS(t->b) 
			e -= 16; 
			NEEDBITS(e) 
		} 
		while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16); 
		DUMPBITS(t->b) 
		if (e == 16) 
		{ 
			slide[w++] = (uch)t->v.n; 
			if (w == WSIZE) 
			{ 
				flush_output(w); 
				w = 0; 
			} 
		} 
		else 
		{ 
			if (e == 15) 
				break; 
 
			NEEDBITS(e) 
			n = t->v.n + ((unsigned)b & mask_bits[e]); 
			DUMPBITS(e); 
 
			NEEDBITS((unsigned)bd) 
			if ((e = (t = td + ((unsigned)b & md))->e) > 16) 
			do { 
				if (e == 99) 
					return 1; 
				DUMPBITS(t->b) 
				e -= 16; 
				NEEDBITS(e) 
			} 
			while ((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16); 
			DUMPBITS(t->b) 
			NEEDBITS(e) 
			d = w - t->v.n - ((unsigned)b & mask_bits[e]); 
			DUMPBITS(e) 
			do { 
				n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e); 
#if !defined(NOMEMCPY) && !defined(DEBUG) 
				if (w - d >= e) 
				{ 
					memcpy(slide + w, slide + d, e); 
					w += e; 
					d += e; 
				} 
				else 
#endif /* !NOMEMCPY */ 
				do 
				{ 
					slide[w++] = slide[d++]; 
				} 
				while (--e); 
				if (w == WSIZE) 
				{ 
					flush_output(w); 
					w = 0; 
				} 
			} 
			while (n); 
		} 
	} 
 
	wp = w; 
	bb = b; 
	bk = k; 
 
	return 0; 
} 
 
int inflate_stored() 
{ 
	unsigned n; 
	unsigned w; 
	register ulg b; 
	register unsigned k; 
 
 
	b = bb; 
	k = bk; 
	w = wp; 
 
	n = k & 7; 
	DUMPBITS(n); 
	NEEDBITS(16) 
	n = ((unsigned)b & 0xffff); 
	DUMPBITS(16) 
	NEEDBITS(16) 
	if (n != (unsigned)((~b) & 0xffff)) 
		return 1; 
	DUMPBITS(16) 
	while (n--) 
	{ 
		NEEDBITS(8) 
		slide[w++] = (uch)b; 
		if (w == WSIZE) 
		{ 
			flush_output(w); 
			w = 0; 
		} 
		DUMPBITS(8) 
	} 
	wp = w; 
	bb = b; 
	bk = k; 
	return 0; 
} 
 
int inflate_fixed() 
{ 
	int i; 
	struct huft *tl; 
	struct huft *td; 
	int bl; 
	int bd; 
	unsigned l[288]; 
 
	for (i = 0; i < 144; i++) 
		l[i] = 8; 
	for (; i < 256; i++) 
		l[i] = 9; 
	for (; i < 280; i++) 
		l[i] = 7; 
	for (; i < 288; i++) 
		l[i] = 8; 
	bl = 7; 
	if ((i = huft_build(l, 288, 257, cplens, cplext, &tl, &bl)) != 0) 
		return i; 
 
	for (i = 0; i < 30; i++) 
		l[i] = 5; 
	bd = 5; 
	if ((i = huft_build(l, 30, 0, cpdist, cpdext, &td, &bd)) > 1) 
	{ 
		huft_free(tl); 
		return i; 
	} 
	if (inflate_codes(tl, td, bl, bd)) 
		return 1; 
 
	huft_free(tl); 
	huft_free(td); 
	return 0; 
} 
 
int inflate_dynamic() 
{ 
	int i; 
	unsigned j; 
	unsigned l; 
	unsigned m; 
	unsigned n; 
	struct huft *tl; 
	struct huft *td; 
	int bl; 
	int bd; 
	unsigned nb; 
	unsigned nl; 
	unsigned nd; 
#ifdef PKZIP_BUG_WORKAROUND 
	unsigned ll[288+32]; 
#else 
	unsigned ll[286+30]; 
#endif 
	register ulg b; 
	register unsigned k; 
 
	b = bb; 
	k = bk; 
 
	NEEDBITS(5) 
		nl = 257 + ((unsigned)b & 0x1f); 
	DUMPBITS(5) 
		NEEDBITS(5) 
			nd = 1 + ((unsigned)b & 0x1f); 
	DUMPBITS(5) 
		NEEDBITS(4) 
			nb = 4 + ((unsigned)b & 0xf); 
	DUMPBITS(4) 
#ifdef PKZIP_BUG_WORKAROUND 
		if (nl > 288 || nd > 32) 
#else 
			if (nl > 286 || nd > 30) 
#endif 
				return 1;                   /* bad lengths */ 
 
	for (j = 0; j < nb; j++) 
	{ 
		NEEDBITS(3) 
			ll[border[j]] = (unsigned)b & 7; 
		DUMPBITS(3) 
	} 
	for (; j < 19; j++) 
		ll[border[j]] = 0; 
 
	bl = 7; 
	if ((i = huft_build(ll, 19, 19, NULL, NULL, &tl, &bl)) != 0) 
	{ 
		if (i == 1) 
			huft_free(tl); 
		return i; 
	} 
 
	n = nl + nd; 
	m = mask_bits[bl]; 
	i = l = 0; 
	while ((unsigned)i < n) 
	{ 
		NEEDBITS((unsigned)bl) 
			j = (td = tl + ((unsigned)b & m))->b; 
		DUMPBITS(j) 
			j = td->v.n; 
		if (j < 16) 
			ll[i++] = l = j; 
		else if (j == 16) 
		{ 
			NEEDBITS(2) 
				j = 3 + ((unsigned)b & 3); 
			DUMPBITS(2) 
				if ((unsigned)i + j > n) 
					return 1; 
			while (j--) 
				ll[i++] = l; 
		} 
		else if (j == 17) 
		{ 
			NEEDBITS(3) 
				j = 3 + ((unsigned)b & 7); 
			DUMPBITS(3) 
				if ((unsigned)i + j > n) 
					return 1; 
			while (j--) 
				ll[i++] = 0; 
			l = 0; 
		} 
		else 
		{ 
			NEEDBITS(7) 
				j = 11 + ((unsigned)b & 0x7f); 
			DUMPBITS(7) 
				if ((unsigned)i + j > n) 
					return 1; 
			while (j--) 
				ll[i++] = 0; 
			l = 0; 
		} 
	} 
	huft_free(tl); 
	bb = b; 
	bk = k; 
 
	bl = lbits; 
	if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != 0) 
	{ 
		if (i == 1) 
			huft_free(tl); 
		return i; 
	} 
	bd = dbits; 
	if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0) 
	{ 
		if (i == 1) 
		{ 
			huft_free(td); 
		} 
		huft_free(tl); 
		return i;                   /* incomplete code set */ 
	} 
	if (inflate_codes(tl, td, bl, bd)) 
		return 1; 
	huft_free(tl); 
	huft_free(td); 
	return 0; 
} 
 
int inflate_block(int *e) 
{ 
	unsigned t; 
	register ulg b; 
	register unsigned k; 
 
	b = bb; 
	k = bk; 
 
	NEEDBITS(1) 
	*e = (int)b & 1; 
	DUMPBITS(1) 
	NEEDBITS(2) 
	t = (unsigned)b & 3; 
	DUMPBITS(2) 
	bb = b; 
	bk = k; 
 
	if (t == 2) 
		return inflate_dynamic(); 
	if (t == 0) 
		return inflate_stored(); 
	if (t == 1) 
		return inflate_fixed(); 
	return 2; 
 
} 
 
int inflate() 
{ 
	int e; 
	int r; 
	unsigned h; 
 
	wp = 0; 
	bk = 0; 
	bb = 0; 
 
	h = 0; 
	do 
	{ 
		hufts = 0; 
		if ((r = inflate_block(&e)) != 0) 
			return r; 
		if (hufts > h) 
			h = hufts; 
	} 
	while (!e); 
 
	while (bk >= 8) 
	{ 
		bk -= 8; 
		inptr--; 
	} 
	flush_output(wp); 
 
	return 0; 
} 
 
void lm_init (int pack_level,ush *flags) 
{ 
	register unsigned j; 
 
	if (pack_level < 1 || pack_level > 9) 
	        error("bad pack level"); 
	compr_level = pack_level; 
 
	memzero((char*)head, HASH_SIZE*sizeof(*head)); 
	max_lazy_match   = configuration_table[pack_level].max_lazy; 
	good_match       = configuration_table[pack_level].good_length; 
#ifndef FULL_SEARCH 
	nice_match       = configuration_table[pack_level].nice_length; 
#endif 
	max_chain_length = configuration_table[pack_level].max_chain; 
	if (pack_level == 1) 
		*flags |= FAST; 
	else if (pack_level == 9) 
		*flags |= SLOW; 
	strstart = 0; 
	block_start = 0L; 
 
	lookahead = read_buf((char*)window,sizeof(int)<=2?(unsigned)WSIZE:2*WSIZE); 
 
	if (lookahead == 0 || lookahead == (unsigned)EOF) 
	{ 
		eofile = 1, lookahead = 0; 
		return; 
	} 
	eofile = 0; 
	while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window(); 
 
	ins_h = 0; 
	for (j=0; j<MIN_MATCH-1; j++) UPDATE_HASH(ins_h, window[j]);
} 
 
int longest_match(IPos cur_match) 
{ 
	unsigned chain_length = max_chain_length; 
	register uch *scan = window + strstart; 
	register uch *match; 
	register int len; 
	int best_len = prev_length; 
	IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL; 
 
#ifdef UNALIGNED_OK 
	register uch *strend = window + strstart + MAX_MATCH - 1; 
	register ush scan_start = *(ush*)scan; 
	register ush scan_end   = *(ush*)(scan+best_len-1); 
#else 
	register uch *strend = window + strstart + MAX_MATCH; 
	register uch scan_end1  = scan[best_len-1]; 
	register uch scan_end   = scan[best_len]; 
#endif 
 
	if (prev_length >= good_match) 
		chain_length >>= 2; 
 
	do { 
		match = window + cur_match; 
 
#if (defined(UNALIGNED_OK) && MAX_MATCH == 258) 
		if (*(ush*)(match+best_len-1) != scan_end || *(ush*)match != scan_start) 
		        continue; 
		scan++, match++; 
		do 
		{ 
		}  
		while (*(ush*)(scan+=2) == *(ush*)(match+=2) && 
		    *(ush*)(scan+=2) == *(ush*)(match+=2) && 
		    *(ush*)(scan+=2) == *(ush*)(match+=2) && 
		    *(ush*)(scan+=2) == *(ush*)(match+=2) && 
		    scan < strend); 
		if (*scan == *match) scan++; 
 
		len = (MAX_MATCH - 1) - (int)(strend-scan); 
		scan = strend - (MAX_MATCH-1); 
 
#else /* UNALIGNED_OK */ 
 
		if (match[best_len]   != scan_end  || 
		    match[best_len-1] != scan_end1 || 
		    *match            != *scan     || 
		    *++match          != scan[1])      continue; 
 
		scan += 2, match++; 
 
		do 
		{ 
		}  
		while (*++scan == *++match && *++scan == *++match && 
		    *++scan == *++match && *++scan == *++match && 
		    *++scan == *++match && *++scan == *++match && 
		    *++scan == *++match && *++scan == *++match && 
		    scan < strend); 
 
		len = MAX_MATCH - (int)(strend - scan); 
		scan = strend - MAX_MATCH; 
 
#endif /* UNALIGNED_OK */ 
 
		if (len > best_len) { 
			match_start = cur_match; 
			best_len = len; 
			if (len >= nice_match) break; 
#ifdef UNALIGNED_OK 
			scan_end = *(ush*)(scan+best_len-1); 
#else 
			scan_end1  = scan[best_len-1]; 
			scan_end   = scan[best_len]; 
#endif 
		} 
	}  
	while ((cur_match = prev[cur_match & WMASK]) > limit 
	    && --chain_length != 0); 
 
	return best_len; 
} 
 
 
local void fill_window() 
{ 
	register unsigned n, m; 
 
	unsigned more = (unsigned)(window_size - (ulg)lookahead - (ulg)strstart); 
	if (more == (unsigned)EOF) 
		more--; 
	else if (strstart >= WSIZE+MAX_DIST) 
	{ 
		memcpy((char*)window, (char*)window+WSIZE, (unsigned)WSIZE); 
		match_start -= WSIZE; 
		strstart    -= WSIZE; 
 
		block_start -= (long) WSIZE; 
 
		for (n = 0; n < HASH_SIZE; n++) 
		{ 
			m = head[n]; 
			head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL); 
		} 
		for (n = 0; n < WSIZE; n++) 
		{ 
			m = prev[n]; 
			prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL); 
		} 
		more += WSIZE; 
	} 
	if (!eofile) 
	{ 
		n = read_buf((char*)window+strstart+lookahead, more); 
		if (n == 0 || n == (unsigned)EOF) 
			eofile = 1; 
		else 
			lookahead += n; 
	} 
} 
 
local ulg deflate_fast() 
{ 
	IPos hash_head; 
	int flush=0; 
	unsigned match_length = 0; 
 
	prev_length = MIN_MATCH-1; 
	while (lookahead != 0) 
	{ 
		INSERT_STRING(strstart, hash_head);
		if (hash_head != NIL && strstart - hash_head <= MAX_DIST) 
		{ 
			match_length = longest_match (hash_head); 
			if (match_length > lookahead) 
			        match_length = lookahead; 
		} 
		if (match_length >= MIN_MATCH) 
		{ 
			flush = ct_tally(strstart-match_start, match_length - MIN_MATCH); 
			lookahead -= match_length; 
			if (match_length <= max_insert_length) 
			{ 
				match_length--; 
				do 
				{ 
					strstart++; 
					INSERT_STRING(strstart, hash_head);
				}  
				while (--match_length != 0); 
				strstart++; 
			}  
			else 
			{ 
				strstart += match_length; 
				match_length = 0; 
				ins_h = window[strstart]; 
				UPDATE_HASH(ins_h, window[strstart+1]);
			} 
		}  
		else 
		{ 
			lookahead--; 
			strstart++; 
		} 
		if (flush) FLUSH_BLOCK(0), block_start = strstart; 
		while (lookahead < MIN_LOOKAHEAD && !eofile) 
		        fill_window(); 
 
	} 
	return FLUSH_BLOCK(1); /* eof */ 
} 
 
ulg deflate() 
{ 
	IPos hash_head; 
	IPos prev_match; 
	int flush; 
	int match_available = 0; 
	register unsigned match_length = MIN_MATCH-1; 
 
	if (compr_level <= 3) 
		return deflate_fast(); 
 
	while (lookahead != 0) 
	{ 
		INSERT_STRING(strstart, hash_head);
		prev_length = match_length, prev_match = match_start; 
		match_length = MIN_MATCH-1; 
 
		if (hash_head != NIL && prev_length < max_lazy_match && strstart - hash_head <= MAX_DIST) 
		{ 
			match_length = longest_match (hash_head); 
			if (match_length > lookahead) match_length = lookahead; 
			if (match_length == MIN_MATCH && strstart-match_start > TOO_FAR) 
				match_length--; 
		} 
		if (prev_length >= MIN_MATCH && match_length <= prev_length) 
		{ 
			flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH); 
			lookahead -= prev_length-1; 
			prev_length -= 2; 
			do 
			{ 
				strstart++; 
				INSERT_STRING(strstart, hash_head);
			}  
			while (--prev_length != 0); 
			match_available = 0; 
			match_length = MIN_MATCH-1; 
			strstart++; 
			if (flush) FLUSH_BLOCK(0), block_start = strstart; 
		}  
		else if (match_available) 
		{ 
			if (ct_tally (0, window[strstart-1])) 
				FLUSH_BLOCK(0), block_start = strstart; 
			strstart++; 
			lookahead--; 
		}  
		else 
		{ 
			match_available = 1; 
			strstart++; 
			lookahead--; 
		} 
		while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window(); 
	} 
	if (match_available) ct_tally (0, window[strstart-1]); 
	return FLUSH_BLOCK(1); /* eof */ 
} 
ulg updcrc(uch *s,unsigned n) 
{ 
	register ulg c; 
 
	static ulg crc = (ulg)0xffffffffL; 
 
	if (s == NULL) 
		c = 0xffffffffL; 
	else 
	{ 
		c = crc; 
		if (n) do 
		{ 
			c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8); 
		}  
		while (--n); 
	} 
	crc = c; 
	return c ^ 0xffffffffL; 
} 
 
void clear_bufs() 
{ 
	outcnt = 0; 
	insize = inptr = 0; 
	bytes_in = bytes_out = 0L; 
	t_read=0; 
	outbuf_seek=0; 
	inbuf_seek=0; 
} 
 
void flush_outbuf() 
{ 
	if (outcnt == 0) return; 
 
	write_buf(ofd, (char *)outbuf, outcnt); 
	bytes_out += (ulg)outcnt; 
	outcnt = 0; 
} 
void flush_window() 
{ 
	if (outcnt == 0) return; 
	updcrc(window, outcnt); 
 
	write_buf(ofd, (char *)window, outcnt); 
	bytes_out += (ulg)outcnt; 
 
	outcnt = 0; 
} 
 
void write_buf(int fd,voidp  buf,unsigned  cnt) 
{ 
	unsigned  n; 
 
	if(fd<0) 
		return; 
	if(fd) 
	{ 
	        while ((n = write(fd, buf, cnt)) != cnt) 
	        { 
		        if (n == (unsigned)(-1)) 
			{ 
			        error("write_error"); 
				return; 
			} 
		        cnt -= n; 
		        buf = (voidp)((char*)buf+n); 
	        } 
	} 
	else 
	{ 
		if((*out_buf=(char *)realloc(*out_buf,outbuf_seek+cnt))==NULL) 
		{ 
			error("out of memory"); 
			return; 
		} 
		memcpy(*out_buf+outbuf_seek,(char *)buf,cnt); 
		outbuf_seek+=outcnt; 
	} 
} 
 
#if defined(NO_STRING_H) && !defined(STDC_HEADERS)
#  ifndef __STDC__ 
#    define const 
#  endif 
 
 
#endif /* NO_STRING_H */
 
/*extern short l_y,l_x;*/ 
 
void error(char *m) 
{ 
	fprintf(stderr,"%s\n",m); 
	fflush(stderr); 
} 
/* 
void write_error() 
{ 
} 
*/ 
 
/* ======================================================================== 
 * Semi-safe malloc -- never returns NULL. 
 */ 
 /* 
static voidp xmalloc (unsigned size) 
{ 
	voidp cp = (voidp)malloc (size); 
 
	if (cp == NULL) error("out of memory"); 
	return cp; 
} 
*/ 
void bi_init () 
{ 
	bi_buf = 0; 
	bi_valid = 0; 
 
	read_buf  = file_read; 
 
} 
 
void send_bits(int value,int length) 
{ 
	if (bi_valid > (int)Buf_size - length) 
	{ 
		bi_buf |= (value << bi_valid); 
		put_short(bi_buf); 
		bi_buf = (ush)value >> (Buf_size - bi_valid); 
		bi_valid += length - Buf_size; 
	}  
	else 
	{ 
		bi_buf |= value << bi_valid; 
		bi_valid += length; 
	} 
} 
 
unsigned bi_reverse(unsigned code,int len) 
{ 
	register unsigned res = 0; 
	do 
	{ 
		res |= code & 1; 
		code >>= 1, res <<= 1; 
	}  
	while (--len > 0); 
	return res >> 1; 
} 
 
void bi_windup() 
{ 
	if (bi_valid > 8) 
	{ 
		put_short(bi_buf); 
	} 
	else if (bi_valid > 0) 
	{ 
		put_byte(bi_buf); 
	} 
	bi_buf = 0; 
	bi_valid = 0; 
} 
 
void copy_block(char *buf,unsigned len,int header) 
{ 
	bi_windup();              /* align on byte boundary */ 
 
	if (header) 
	{ 
		put_short((ush)len); 
		put_short((ush)~len); 
	} 
	while (len--) 
		put_byte(*buf++); 
} 
 
int fill_inbuf(int eof_ok) 
{ 
	int len; 
 
	insize = 0; 
	do 
	{ 
		int size=INBUFSIZ-insize; 
 
		if(ifile_size && t_read+size>(unsigned)ifile_size) 
		        size=ifile_size-t_read; 
		if(!ifd) 
		{ 
			len=size; 
			if(t_read+size>(unsigned)ifile_size) 
				len=t_read+size-ifile_size; 
			if(len>0) 
				bcopy(in_buf+t_read,(char*)inbuf+insize,len); 
		} 
		else 
			len = read(ifd,(char*)inbuf+insize, size); 
		if (len == 0 || len == EOF) break; 
		t_read+=len; 
		insize += len; 
	} 
	while (insize < INBUFSIZ); 
	if (insize == 0 && eof_ok) 
		return EOF; 
	bytes_in += (ulg)insize; 
	inptr = 1; 
	return inbuf[0]; 
} 
