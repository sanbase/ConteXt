#ifndef _BROWSER_H
#define _BROWSER_H

#include "DB_CLASS/CX_BASE.h"
#include "SCREEN/screen.h"
#include "CX_utils.h"
#ifndef WIN32
#include "CX_pipe.h"
#endif

int Xmouse(int i);

class CX3;

/*
 _Forms folder consist of 5 fields
1.      CHAR[32]        - form name
2.      TEXT            - background
3.      STRUCT[]        - array of tag_descriptor structures
4.      STRUCT[]        - array of panel structures
5.      INTEGER         - form attribute
*/

struct panel
{
	long x;
	long y;
	long l;
	long h;
	long fg;
	long bg;
	long atr;
};

struct label
{
	long x;
	long y;
	long l;
	long h;
	long fg;
	long bg;
	long atr;
	long font;
	char *text;
};

class FORM
{
	friend class CX_BROWSER;
private:

	void look_for_tag_names();
	int look_str(int,register unsigned char *,int);
	int get_blank(char *base=NULL);
	void Get_Size();
public:
	void create_Form(char *);
	void create_Form(char *,int);
	void create_Form(char *,char *);
	void create_Manual(int,struct tag *,int, struct panel *);
	char *background_pos(int x,int y);

	int num_fields;
	struct tag *tags;

	int num_panel;
	struct panel *panel;

	int num_label;
	struct label *label;

	int size;
	char *background;
	int lines;
	int width;
	struct x_form name_form;
	struct tag **f_mark_field;
	int *f_num_mark_fields;
	long form_atr;

	struct panel fix_area;

	FORM(char *);
	FORM(char *,char *name);
	FORM(char *,long record);
	FORM(char *,struct x_form *);
	FORM(int,struct tag *);
	int if_mark_field(int);
	void Free_Form();
	~FORM();

//        void Show();
//        int Draw_Slot(int i,int bg,int fg,struct font font);

};

class BROWSER:public FORM
{
protected:

	int f;
	int l,h;
	int line;       // horizontal shift
	int colon;      // vertical shift
	int num_colon;  // number of colons in table
	int num_lines;  // number of lines in table
	int first_line; // first field of the table
	char *title;    // title of the browser

	int find_pos(int kod);
	void create_geom(int xx, int yy, int ll, int hh);
	void restore_bg();
	int  _move(int);
	int possible(int);
	int pos_able(int);
	void Restore();
	void clean_frames();
	int Draw_Slot(int i,int bg,int fg,struct font font);
	void Del_Label();

	FORM **forms;
	int num_forms;

public:

	int x0,y0;
	int act_field;
	int frame_color;
	int form_cond;

	BROWSER(char *base,int x, int y, int l, int h);
	BROWSER(char *base,int x, int y, int l, int h,char *title);
	BROWSER(char *base,char *name, int x, int y, int l, int h);
	BROWSER(char *base,long record, int x, int y, int l, int h);
	BROWSER(char *base,struct x_form *, int x, int y, int l, int h);
	BROWSER(int num_fields,struct tag *des, int x, int y, int l, int h);
	~BROWSER();

	void Show();
	int  Move();
	int Act_Field();
	int Act_Field(int);

	void form_update(CX_BASE *db,long record);
	void form_update(CX_BASE *db,long record,int field);

};

struct query
{
	struct sla sla[SLA_DEEP];
	char *str;
};

struct t_arg
{
	char *name;
	long (*record)(long, CX_BROWSER *);
	struct sla *sla;
	long page;
	long max_index;
	long *num_rez;
	long **rez_set;
	int f;
};


// (History)
class ED
{
private:
	int num;
	struct edit_history *e;
	char *name;
public:
	ED(char *file_name);
	~ED();
	void Add(struct sla *sla,char *str);
	void Reset();
	void Write(int);
};

class CX_BROWSER:public BROWSER
{
	friend class CX3;


private:
	long record;            // current object number
	long index;             // current index
	int  shmid;             // shared mamory indentificator
	char obr[64];           // pattern for map mode
	long *mark;             // array of marked objects
	int  num_mark;          // number of marked objects
	int  level;             // recursion level
	struct s_stack *stack;  // selection steck
	int num_stack_sel;      // number of selections in the stack
	int  hyperform;         // numner of field - pointer to hyperform (if exist)
	int  prot_fd;           // log file descriptor (if exist)
	int  inherit;           // inheritage flag
	int  read_only;         // read-only flag
	char *restrict;         // name of restrict selection
	long parent_record;     // parent object number
	int hidden;
	int flag_cxerror;
	char *personal_selection;
	char *bg_orig;
	int bg_size;
	long hist_record;
	ED *ed;
	CX3 *cx3;
	struct tag *mark_field;
	int num_mark_fields;
	MString *keep_str;
	struct image *menu_cmd;
	void Form_Restruct();
	void Form_Refresh(struct tag_descriptor *td=NULL);
	void set_form();
	void New_Index();
	int  Scroll(int);
	int  Scroll_Array(int,int dp=1);
	int  del_Record(long page);
	void Set_Index();
	void Make_Indname(char *name);
	void Next_Index_File(char *tmp);
	void Open_Index_File();
	long Next_Index(long index);
	long Prev_Index(long index);
	long Sub_Index(int ind,int num_fields,long *&ind_array);
	int Find_Active();
	void Find_Place();
	void Table_Normalize();
	int if_mark(long);
//        int if_mark_field(int);
	void zero();
	void initialize(CX_BROWSER *,long);
	void initialize(char *,long,int,struct x_form *f=NULL,char *ch=NULL);
	void Mark_To_Index();
	void Write_Index(char *tmp);
	int Check_Line(char *);
	int Check_Line(struct tag *,char *);
	int Check_Cadr();
	int Event(int);
	void Ind_Update();
	int Check();
	int Check(struct tag *);
	void Modify();
	int Edit(int,struct tag *);
	int Show_Sum();
	int Command();
	int Draw_Chart(int);
	int Draw_Graph(int);
	int Intensity_Graph();
	void Calculator();
	void Add_Stack(char *,long);
	void Add_Stack(struct query *,long);
	void Del_Stack();
	int Show_Stack();
	int Menu();
	void Cadr_Read();
	void Put_Selection(int);
	void Put_Selection(int,char *);
	int find_slot(struct sla *);
	void protocol(int);
	void total(long);
	char *Remote_Browser(char *host,long page,char *passwd);
	char *Remote_Map(char *name,long page,struct sla *sla,int len);
	long Select_From_DB(char *folder,int field,int len,char *selection=NULL);
	long Select_From_DB(char *folder,struct sla *sla,int len,char *selection=NULL);
	void del_prim_selection();
	void Chart(int *value,int num,int x,int y,int l,int h,int color,int num_chart);
	void Reper_Up(int x, int y);
	void Reper_Down();
	void set_reper_coord(int x,int y,int l,int h);
public:
	long cx_cond;           // current condition
	CX_BASE *db;
	CX_BROWSER(CX_BROWSER *);
	CX_BROWSER(CX_BROWSER *STD,long REC,struct sla *sla,int max_len);
	CX_BROWSER(CX_BROWSER *,long record,int num_fields,struct tag *des);
	CX_BROWSER(char *base,long record,struct sla *sla,int len,char *ch=NULL);
	CX_BROWSER(char *base,long record,struct x_form *,char *ch=NULL);
	CX_BROWSER(CX_BROWSER *STD,int x, int y,char *name_blank);
	CX_BROWSER(char *base,long record,int blank,char *ch=NULL);
	CX_BROWSER(char *base,long record,char *blank,char *ch=NULL);
	CX_BROWSER(char *base,long record,struct x_form *,int recurs,char *ch=NULL);
	CX_BROWSER(CX_BROWSER *STD,long record,struct x_form *,int recurs);
	CX_BROWSER(char *base,long record,struct x_form *f,int x,int y,int w,int h);
	~CX_BROWSER();
	
	long Record(long page);
	void CX_Show();
	int  put_Record(long page,long ind);
	int Move(int);
	int Go(int);
	int Action();
	int Cmd_Exe(int);
	int Write();
	int Change_Form();
	int Change_Form(int);
	int Change_Form(char *);
	void Load_Env(CX_BROWSER *);
	int Query(struct query *);
	int Get_Slot(long, struct tag *,char *&);
	int cxerror();
	void Create_Map(struct sla *,int len);
	void Create_Table(int num, struct tag *);
	void form_update();
	void form_update(int field);
	long Act_Record();
	void Get_Form(char *,struct x_form *);
	void Save_Index();
	void Restore_Status(struct last_status *last);
	void Read_Index(char *tmp);
	void Go_To_Index(long);
	void Go_To_Field(struct tag_descriptor,long);
	int Arr_To_Index(long *,int);
	int  Rest_Index();
	int Export(char *name);
	int Export(int fd);
	void protocol(char *);
	void Refresh(int);
	void delete_menu();
	void load_menu(int page=1);
	long find_page(long page);
	void set_prim_selection(char *name_sel);
	int V3Show();
	long Index();
	void Read_Only(int i);
	void Set_Parent_Record(long record);
};

/* form_cond */
#define NEW      0x2      // this is a new record
#define TABLE    0x4      // table mode
#define MAP      0x8      // map mode
#define EDIT    0x10      // record was updated
#define SHOWDEL 0x20      // show deleted record
#define ARRAY   0x40      // show array in table mode
#define MARK    0x80      // mark mode
#define BW     0x100      // black/white mode
#define REL    0x200      // relatively shift
#define MANUAL 0x400      // current form has been updated by Method

// form_atr
#define NOASK    0x1      // save without prompting for confirmation
#define NODELETE 0x2      // not delete
#define NONEW    0x4      // can't create new object
#define NOMENU 0x200      // menu disable

// cx_cond
#define SORT     0x1      // selection loaded
#define HIST     0x2      // hostory mode
#define CHRT     0x4      // histogram mode
#define PUTD     0x8      // form to file
#define PROT    0x10      // log file seted up

#define EXECONT 0x100000  // mask for Action (execute and continue)

// commands of browser
#define c_Mark          100     // mark field
#define c_ChangeForm    101     // change form
#define c_RestIdx       102     // restore previous selectin
#define c_Map           103     // show map
#define c_DelRest       104     // delete or restore record (depends on current status)
#define c_GoNext        105     // go to next index
#define c_GoPrev        106     // go to previous index
#define c_ShowDel       107     // show deleted records
#define c_Recurs        108     // recursive movind to subbase
#define c_NewRec        109     // create a new record
#define c_RecForm       110     // recursive change form
#define c_Choise        111     // select from map
#define c_Refresh       112     // refrest screen
#define c_Stack         113     // show selection's stack
#define c_Save          114     // save record
#define c_Menu          115     // run menu
#define c_Modify        116     // run modify mode
#define c_SubBase       117     // ??? (probably it is a rudiment)
#define c_NoSel         118     // drop all selections
#define c_Calc          119     // Calculator
#define c_Show          120     // open separate window
#define c_PutSel        121     // save selection in file
#define c_Shell         122     // system command (or shell)
#define c_Hist          123     // set up history mode
#define c_Struct        124     // show schema of the Class
#define c_Chart         125     // show graph/chart
#define c_Pie           126     // show pie diagram
#define c_Bar           127     // show bar diagram
#define c_SortA         128     // sorting by increas
#define c_SortZ         129     // sorting by decrease
#define c_EditForm      130     // edit form
#define c_GoFirst       131     // go to first index
#define c_GoLast        132     // go to last index
#define c_MarkRec       133     // mark record
#define c_ChangeDB      134     // change folder
#define c_Request       135     // run request prompt
#define c_PutForm       136     // save form in file
#define c_ShowSel       137     // show selection in separate window
#define c_SelSave       138     // save selection as array of records
#define c_SelRest       139     // restore selection from file
#define c_Intens        140     // intensity graph
#define c_Array         141     // show graph in the bar mode (?)
#define c_Total         142     // show total value of all field in the current selection
#define c_GoUp          143     // cursor up
#define c_GoDown        144     // cursor down
#define c_GoRight       145     // cursor right
#define c_GoLeft        146     // cursor left
#define c_Exit          147     // exit
#define c_Update        148     // update browser
#define c_HTML          149     // export to HTML format
#define c_XML           150     // export to HTML file
#define c_Schema        151     // export Schema to  file
#define c_GetProperty   152     // show list of property
#define c_Create_Class  153     // create a new Class

#define c_Slot          154     // slot setup
#define c_Panel         155     // panel setup
#define c_Relocate      156     // relocate form area
#define c_Draw_S        157     // draw mode (create form) single line
#define c_Draw_D        158     // draw mode (create form) double line
#define c_Table         159     // table setup
#define c_Attr          160     // slot arrtibute setup
#define c_Dial          161     // run dialog from method
#define c_GetSel        162     // boot selection
#define c_GetLimitSel   163     // boot limited slection (only at the moment of initialization)
#define c_DelLimitSel   164     // boot limited slection (only at the moment of initialization)
#define c_ShowSum       165     //
#define c_ShowForm      166     // open the window with the form
#define c_AddField      167     // add custom field to the current form
#define c_DelField      168     // delete field from form

char *Select_From_Dir(char *name, int ( *check_name)(char *,char *),int select_flag=0);
char *Select_From_Dir(char *name, int ( *check_name)(char *,char *),char *,int select_flag=0);
struct color Type_Color(CX_BASE *,int);
void repaint();
int get_arg(char *line,char *pattern,char *&arg);

#define ICONSDB   "_CX_Icons"

#ifndef WIN32
#define MSGDEF    "/usr/local/etc/CX_Messages"
#define MENUDEF   "/usr/local/etc/CX_Menu"
#define ICONSDEF  "/usr/local/etc/CX_Icons"
#define HELPDEF   "/usr/local/etc/CX_Help"
#else
#define MSGDEF    "C:/Program Files/UnixSpace/etc/CX_Messages"
#define MENUDEF   "C:/Program Files/UnixSpace/etc/CX_Menu"
#define ICONSDEF  "C:/Program Files/UnixSpace/etc/CX_Icons"
#define HELPDEF   "C:/Program Files/UnixSpace/etc/CX_Help"
#endif

#define FMATR "\n#ATR="


void Make_Class(char *iname,struct st *st=NULL, int new_database=0);
int Edit_Form(CX_BASE *,long page=1);
void Help(int,int);

#endif
