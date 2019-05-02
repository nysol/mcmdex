#pragma once

#define SSPC_INCLUSION 1
#define SSPC_SIMILARITY 2
#define SSPC_INTERSECTION 4
#define SSPC_RESEMBLANCE 8
#define SSPC_INNERPRODUCT 16
#define SSPC_MININT 32
#define SSPC_MAXINT 64
#define SSPC_PMI 128
#define SSPC_FVALUE 256
#define SSPC_COUNT 2048
#define SSPC_MATRIX 4096
#define SSPC_NO_NEIB 16384
#define SSPC_POLISH 32768
#define SSPC_POLISH2 65536
#define SSPC_OUTPUT_INTERSECT 131072
#define SSPC_COMP_ITSELF 262144

#include "trsact.hpp"
#include "problem.hpp"
#include "itemset.hpp"

class KGSSPC{

	// receive parameter
	// SSPC
  int _dir; 
  int _root; 
	int _problem;
  double _th2;
	double _th;
	char *_output_fname2;
	char *_table_fname;
	char *_outperm_fname;
	char *_output_fname;

	// _II
  int _iFlag;
	int _topk_k;
	LONG _itemtopk_item;
	LONG _itemtopk_item2;
	LONG _itemtopk_end;
	int _len_lb;
	int _len_ub;
	int _multi_core;
	int _max_solutions;
	char _separator;
	double _frq_lb;

	// _TT
  int _tFlag;
  int _tFlag2;
	double _row_lb_;
	int _row_lb;
	double _row_ub_;
	int _row_ub;
	double _clm_lb_;
	double _w_lb;
	double _clm_ub_;
	double _w_ub;

  int _sep;
	char *_wfname;
	char *_item_wfname;
	char *_fname;
  char *_fname2;

	// gloval value in org
	// LONG _ip_l2;
	 LONG _ip_l3;
	 LONG _ip_l1;

	int _siz;

	char *_position_fname;
	char  *_vecchr;
	WEIGHT *_occ_w;
  QUEUE_INT *_itemary;
	VECARY <QUEUE_INT> _buf;

  size_t _buf_end;
  QUEUE_ID _i;
	char *_ERROR_MES;


	ITEMSET _II;
	TRSACT _TT;

	void output ( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id);
	void *iter (void *p);

	WEIGHT comp ( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq);
	void comp2 (QUEUE_ID i, QUEUE_ID x, WEIGHT c, WEIGHT wi, WEIGHT wx, double sq, QUEUE_INT *cnt, OFILE2 *fp, QUEUE *itemset, int core_id);

	void SSPCCORE();
	void list_comp();

	/* allocate arrays and structures */
	void preALLOC(){

		QUEUE_ID siz  = _TT.get_clms();
		QUEUE_ID siz2 = _TT.get_t();
	 	PERM *perm    = _TT.get_perm();
	  PERM *p;

		//calloc2(_occ_w, siz+2, goto ERR);
		_occ_w = calloc2(_occ_w, siz+2);
		//calloc2(_vecchr, siz2+2, goto ERR);
		_vecchr = calloc2(_vecchr, siz2+2);

		if(_problem&SSPC_POLISH2) {
			_itemary = calloc2(_itemary, siz+2);
		}

    // set outperm
	  if ( _outperm_fname ){

			int j = FILE2::ARY_Load(p,_outperm_fname,1);

	  	if ( perm ){
      	for(int j =0 ;j < siz ; j++){
      		perm[j] = p[perm[j]];
      	}
	      delete [] p;
      }
	    else {
    		perm = p;
    	}
		}

	  _II.alloc(_output_fname, perm, siz, 0);

		_TT.set_perm(NULL); // これなに？
		
		_TT.reallocW();

	  return;

  	ERR:;

  	EXIT;
	}
	int setArgs(int argc, char *argv[]);
 	void help();

	public :

	KGSSPC():
	  _iFlag(0),_tFlag(0),
		_root(0),_dir(0),_problem(0),_siz(0),
		_th(0),_th2(0),
		_output_fname(NULL),_output_fname2(NULL),
		_outperm_fname(NULL),_table_fname(NULL),
		_position_fname(NULL),_vecchr(NULL),
		_occ_w(NULL),_itemary(NULL),
		_buf_end(0),_i(0),_ERROR_MES(NULL),
		_row_lb_(0.0),_row_lb(0),
		_row_ub_(0.0),_row_ub(QUEUE_IDHUGE),
		_clm_lb_(0.0),_w_lb(-WEIGHTHUGE),
		_clm_ub_(0.0),_w_ub(WEIGHTHUGE),
		// _II
		_itemtopk_item(0),_itemtopk_item2(0),_itemtopk_end(0),
		_len_ub(INTHUGE),_len_lb(0),
		_multi_core(1),_max_solutions(0),_separator(' '),
		_fname(NULL),_wfname(NULL),_item_wfname(NULL),
		_fname2(NULL)

		{}



	int run(int argc ,char* argv[]);
	
	static int mrun(int argc ,char* argv[]);


};