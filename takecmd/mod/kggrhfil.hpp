/* GRHFIL: convert graph format */
/* 2004 Takeaki Uno */
/* matdraw */

// #define FSTAR_INT unsigned int
// internal_params.l1 :  #edges written to the output file


#define GRHFIL_INS_ROWID 1024
#define GRHFIL_NORMALIZE 65536
#define GRHFIL_DISCRETIZE 131072

#define WEIGHT_DOUBLE

#include <math.h>
#include "stdlib2.hpp"
#include "fstar.hpp"
#include "itemset.hpp"


class KGGRHFIL{

	 LONG _ip_l1;

  // PROBLEM _PP; //problemは使わない

  // problem項目
  ITEMSET _II;
  int _problem,_problem2;
  int _dir,_root;
  
  double _ratio;
  double _th,_th2;

  char *_weight_fname, *_table_fname;
  char *_output_fname;

  VEC_ID _rows;

  FSTAR _FS;
  FSTAR _FS2;

	// _FS  
	// fs
	int _fsFlag;
	int _edge_dir;
	char *_fname;
	char *_wfname;
	int _deg_lb,_deg_ub;
	int _in_lb,_in_ub;
	int _out_lb,_out_ub;
	double _w_lb,_w_ub;
	char _sep;

	// f2
	int _fsFlag2;
	char *_fname2;
	int _edge_dir2;

 
  char *_ERROR_MES ;

	void help(void);

	int setArgs_iter (char *a, int *ff);
	int setArgs(int argc, char *argv[]);

	public:


	KGGRHFIL():
		_problem(0),_problem2(0),_fsFlag(0),_fsFlag2(0),
  	_dir(0),_root(0),_ratio(0),_th(0),_th2(0),_rows(0),
		_deg_lb(0),_deg_ub(FSTAR_INTHUGE),_in_lb(0),_in_ub(FSTAR_INTHUGE),
		_out_lb(0),_out_ub(FSTAR_INTHUGE),_w_lb(-WEIGHTHUGE),_w_ub(WEIGHTHUGE),
		_edge_dir(0),_edge_dir2(0),_sep(' '),
		_weight_fname(NULL),_table_fname(NULL),
		_output_fname(NULL),_ERROR_MES(NULL),
		_fname(NULL),_fname2(NULL),_wfname(NULL){}

	int run(int argc ,char* argv[]);
	static int mrun(int argc ,char* argv[]);

};



