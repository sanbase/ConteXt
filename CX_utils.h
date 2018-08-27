
class MString
{
public:
	char *str;
	MString(char *s);
	~MString();
};

class Line
{
private:
	int mask_flg;
	int secure;
	char *line_std;
	Terminal *term;

	void indstr(int ind,char *line,int poz,int x,int y,int size);
	void inds(char *ss,char l,int ch,struct clr *color);
	void clear_line(int x,int y,int l);
	int seek_poz(int znak,int poz,int size);
	void ind_ins(int x,int y);
public:
	char insert;
	char mask[256];         //  input mask

	Line(Terminal *t);
	~Line();
	int edit(unsigned int i,char *line,int size,int show,int x,int y,int arg);
};
