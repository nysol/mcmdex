/* forward star type graph structure -- for quite large graphs */
/* 6/Nov/2008   Takeaki Uno  */

#pragma once


#include"stdlib2.hpp"
#include"file2.hpp"

//#define FSTAR_INT_LONG

#define FSTAR_REDUCE 33554432    // output table
#define FSTAR_CNT_IN 67108864
#define FSTAR_CNT_OUT 134217728
#define FSTAR_CNT_DEG_ONLY 268435456   // in_deg means usual degree

#define FSTAR_DEG_CHK 536870912
#define FSTAR_IN_CHK 1073741824
#define FSTAR_OUT_CHK 4194304
#define FSTAR_INS_ROWID 536870912  // insert row ID to the head of each line
#define FSTAR_INS_ROWID_WEIGHT 1073741824  // insert row ID to the head of each line

// undirected graph => CNT_DEG_ONLY, edge_dir = 0
// bipartite graph => BIPARTITE, edge_dir != 0
#ifdef FSTAR_INT_LONG
  #define FSTAR_INT LONG
  #define FSTAR_INTHUGE LONGHUGE
  #define FSTAR_INTF "%llu"
#else
  #define FSTAR_INT unsigned int
  #define FSTAR_INTHUGE UINTHUGE
  #define FSTAR_INTF "%u"
#endif


class FSTAR{
//  unsigned char _type;  // type definition
  char *_fname, *_wfname;      // input file name, edge weight file name
  char _sep;         // separator for output file
  int _flag;         // flag
  FSTAR_INT *_edge;   // edge array
  FSTAR_INT *_fstar;  // starting position of edge list for each vertex
  FSTAR_INT *_in_deg, *_out_deg;    // in/out-degree of each vertex (can be NULL if not used)
  FSTAR_INT _node_num, _out_node_num, _in_node_num;  // #vertex and #1st/2nd vertices
  FSTAR_INT _edge_num, _edge_num_org, _reduced_node_num;  // #edges in file, in array
  FSTAR_INT _xmax, _ymax;  // maximum in 1st/2nd column
  FSTAR_INT *_table, *_rev_table;   // vertex permutation table and its reverse
  WEIGHT *_edge_w;   // edge weights

  int _edge_dir;
  
  FSTAR_INT _deg_lb, _in_lb, _out_lb, _deg_ub, _in_ub, _out_ub;  // bounds for degrees
  WEIGHT _w_lb, _w_ub; // bounds for edge weight



		void print (FILE *fp);
		void init2 ();
		void end ();
		
		void inc_deg(FSTAR_INT x, FSTAR_INT y);//privateでOK?
		
		void sort_adjacent_node (int flag);
		LONG alloc_deg ();
		void calc_fstar ();

		void scan_file (FILE2 *fp);

		void read_file (FILE2 *fp, FILE2 *wfp);//privateでOK?

		void extract_subgraph();


		int  eval_edge ( FSTAR_INT x, FSTAR_INT y, WEIGHT w);


		FILE *  open_write_file ( char *fname);//privateでOK?
		void write_graph_ID (FILE *fp, FILE *fp2, FSTAR_INT ID);//privateでOK?
		int  write_graph_item (FSTAR_INT x, FSTAR_INT y, WEIGHT w, FILE *fp, FILE *fp2, int *row, FSTAR_INT *prv);//privateでOK?

		



	public:
	FSTAR():
		_fname(NULL),_wfname(NULL),_sep(' '),_flag(0),
		_edge(NULL),_fstar(NULL),_in_deg(NULL),_out_deg(NULL),
		_node_num(0),_out_node_num(0),_in_node_num(0),_edge_num(0),
		_edge_num_org(0),_reduced_node_num(0),_xmax(0),_ymax(0),
		_table(NULL),_rev_table(NULL),_edge_w(NULL),_edge_dir(0),
		_deg_lb(0),_in_lb(0),_out_lb(0),
		_deg_ub(FSTAR_INTHUGE),_in_ub(FSTAR_INTHUGE),_out_ub(FSTAR_INTHUGE),
		_w_lb(-WEIGHTHUGE),_w_ub(WEIGHTHUGE){}

	char *get_fname(void)   { return _fname;}
	FSTAR_INT * get_table(void){ return _table;}
	FSTAR_INT get_table(LONG l) { return _table[l];}

	void set_fname(char* v) { _fname = v;}
	void set_wfname(char* v) { _wfname = v;}

	void load();

	void set_edge_dir(int i){ _edge_dir=i;}
	int  get_edge_dir(void){ return _edge_dir;}
	int  get_edge_num(void){ return _edge_num;}
	FSTAR_INT get_out_node_num(void){return _out_node_num;}
	FSTAR_INT get_in_node_num(void){return _in_node_num;}
	FSTAR_INT get_node_num(void){return _node_num;}

	WEIGHT *get_edge_w(void){ return _edge_w;}
	WEIGHT get_edge_w(LONG l){ return _edge_w[l];}
	void set_edge_w(LONG l,WEIGHT v){ _edge_w[l]=v;}


	FSTAR_INT get_edge(LONG l){ return _edge[l];}
	void set_edge(LONG l,FSTAR_INT v){ _edge[l]=v;}
	
	FSTAR_INT get_fstar(LONG l){ return _fstar[l];}
	void set_fstar(LONG l ,FSTAR_INT v ){ _fstar[l]=v; }
	int  get_flag(void){ return _flag;}

	int *getp_edge_dir(void){ return &_edge_dir;}


	void union_flag(int flag){ _flag|=flag;}
	void sub_flag(int flag){ _flag-=flag;}
	void set_flag(int flag){ _flag=flag;}

	void set_out_node_num(FSTAR_INT v){_out_node_num=v;}
	void set_deg_lb(FSTAR_INT v){ _deg_lb = v;}
	void set_deg_ub(FSTAR_INT v){ _deg_ub = v;}
	void set_in_lb(FSTAR_INT v) { _in_lb  = v;}
	void set_in_ub(FSTAR_INT v) { _in_ub  = v;}
	void set_out_lb(FSTAR_INT v){ _out_lb = v;}
	void set_out_ub(FSTAR_INT v){ _out_ub = v;}
	void set_w_lb(WEIGHT v)     { _w_lb   = v;}
	void set_w_ub(WEIGHT v)     { _w_ub   = v;}
	void set_sep(char v)        { _sep   = v;}

	FSTAR_INT get_deg_lb(void){ return _deg_lb;}
	FSTAR_INT get_deg_ub(void){ return _deg_ub;}
	FSTAR_INT get_in_lb(void){ return _in_lb;}
	FSTAR_INT get_in_ub(void){ return _in_ub;}
	FSTAR_INT get_out_lb(void){ return _out_lb;}
	FSTAR_INT get_out_ub(void){ return _out_ub;}


	void edge_w_pow(LONG l,double ratio){
    _edge_w[l] = pow (_edge_w[l], ratio);
	}
	void edge_w_mul_min(LONG l,double ratio,double th){
    _edge_w[l] *= ratio; 
    _edge_w[l] = (_edge_w[l]<th) ? _edge_w[l] : th ;
  }
	void edge_w_div(int x,double w){
		_edge_w[x] /= w;
	}


	double edge_w_mul(LONG l){ return _edge_w[l]* _edge_w[l]; }

	LONG array_LOAD(char *tfname){
		return FILE2::ARY_Load(_table,tfname,1);
	}



/*		cmn_FILE2.open(cmn_FILE2, tfname ,"r");
		ARY_SCAN(l,int,cmn_FILE2,1);
		malloc2(_table,(l)+1,EXIT);
		FILE2_reset(&cmn_FILE2);
		ARY_READ(_table,int,l,cmn_FILE2);
		FILE2_close(&cmn_FILE2);
	}
*/

		void write_table_file (char *fname);

		static LONG write_graph_operation (FSTAR *F1, FSTAR *F2, char *fname, char *fname2, int op, double th);

		LONG write_graph (char *fname, char *fname2);


} ;


/**************************************************************/ 
/*
void FSTAR_print (FILE *fp, FSTAR *F);
void FSTAR_init2 (FSTAR *F);
void FSTAR_end (FSTAR *F);
void FSTAR_load (FSTAR *F);

LONG FSTAR_alloc_deg (FSTAR *F);
void FSTAR_calc_fstar (FSTAR *F);
int FSTAR_eval_edge (FSTAR *F, FSTAR_INT x, FSTAR_INT y, WEIGHT w);
void FSTAR_extract_subgraph (FSTAR *F);
void FSTAR_sort_adjacent_node (FSTAR *F, int flag);

void FSTAR_scan_file (FSTAR *F, FILE2 *fp);
void FSTAR_read_file_edge (FSTAR *F, FILE2 *fp);
void FSTAR_read_file_node (FSTAR *F, FILE2 *fp, FILE2 *fp2);

void FSTAR_write_table_file (FSTAR *F, char *fname);
LONG FSTAR_write_graph (FSTAR *F, char *fname, char *fname2);
LONG FSTAR_write_graph_operation (FSTAR *F1, FSTAR *F2, char *fname, char *fname2, int op, double th);

#endif
*/

