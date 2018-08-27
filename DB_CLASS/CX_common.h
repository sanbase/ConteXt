#ifndef _CX_H 
#define _CX_H 
 
#define GCC 
 
#include <sys/types.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <signal.h> 
#include <string.h> 
#include <stdio.h> 
#include <memory.h> 
#include <stdlib.h> 
#ifndef WIN32 
#include <unistd.h> 
//#include <sgtty.h>
#include <termios.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <strings.h> 
#include <SCREEN/screen.h>
#else 
void bcopy(const void *s1, void *s2, size_t n); 
int  bcmp(const void *s1, const void *s2, size_t n); 
void bzero(void *s, size_t n); 
#endif 

void decode(char *);
void encode(char *);
 
int month_length(int y,int m); 
long link_date(int d,int m,int y); 
int  get_dmy(long date,int *day,int *month,int *year ); 
void get_date(long dat,char *str); 
long conv_date(char *str); 
int check_date(char *str); 
void get_time(long tim,char *str); 
void get_unix_time(long tim,char *str); 
long conv_time(char *str); 
void local_time(char *str); 
char *local_time(); 
long get_act_date(); 
long get_act_time(); 
long date_from_rec(char *ch); 
dlong int_conv(char *,int); 
dlong unsigned_conv(char *ch,int len); 
void int_to_buf(char *ch,int len,dlong i); 
dlong atoll(const char *str); 
int full(char *,char *,char *); 
void create_class(struct st *st,char *db, int serv=0);
int len_struct(struct st *st); 
int if_base(char *,char *); 
int if_read(char *); 
int if_write(char *); 
int if_exec(char *); 
int if_stat(char *,int); 
int if_read(char *,char *); 
int if_write(char *,char *); 
int if_exec(char *,char *); 
int if_stat(char *,char *,int); 
int if_dir(char *,char *); 
int if_file(char *,char *); 
int compare(struct field *des_field,char *ch1,char *ch2); 
void put_last_status(char *,struct last_status *); 
int fcopy(char *a,char *b); 
int get_columns(int num,int max); 
void get_colors(char *name_base); 
void SetColor(struct clr color); 
void B_O_X(int x,int y,int l,int h,int ch,struct clr bord,struct clr text); 
void hot_line(char *ch);
void set_hot_key_status(char *key,int bg, int fg=0xa);
int dial(char *mess,int atr,struct color *color=NULL,int font=-1,char *outmess=NULL,int secure=0);
int Button(int a,char *,char *);
int yes(int a); 
//int wait_mess(int num);
//int wait_mess(char *);
char *get_user_dir(char *,char *); 
//void free_wait_mess(int f);
//void free_wait_mess(int f, int num);
void help_line(int); 
int get_menu_cmd(int x0,int y0,int b); 
void flush_mouse(); 
int strcmpw(const char *s1,const char *s2, int len=0); 
int strdif(unsigned char *a,unsigned char *b,int n); 
char *pc_demos(char *str); 
void get_shname(char *name,int level,char *sh_name); 
void sla_to_str(struct sla *,char *); 
int str_to_sla(char *,struct sla *); 
int slacmp(struct sla *a,struct sla *b); 
void conv_money(char *); 
int string_digit(char *a,char d='.');
char *message(int); 
int compress(char *in, int len,char *&out); 
int decompress(char *in,int len,char *&out); 
int zip(int in,int out,int ilen,char *&buf,char *ibuf); 
int unzip(int in,int out,int ilen,char *&buf,char *ibuf); 
int Select_Form(char *,struct x_form *); 
#ifdef SPARC 
void conv(char *,int); 
#endif 
int atox(char *str); 
int atoo(char *str); 
int Show_Structure(struct st *struc,int rec,char *name,int field,struct sla *sla=NULL); 
int PMenu(char *menu, char **select_name); 
int get_filename(char *dir,char **name,int arg=0); 
 
int Get_Slot(char *name_base,long record,int field,char *&slot); 
int Get_Slot(char *name_base,long record,struct sla *sla,char *&slot); 
int Get_Slot(char *name_base,long record,char *descr,char *&slot); 
 
void show_menu(int atr=0); 
void clean_menu(); 
void del_menu(); 
void Load_Menu(char *icon_base,long page); 
int rebuild_tree(char *name,long record,struct sla *sla); 
void bincopy(const void *src, void *dst, int len);
char *GetLogin(); 
 
int  main_box(struct st *struc,int,char *,char *,int); 
int  len_struct(struct st *); 
void load_struct(char *,struct st *&); 
 
#endif 
