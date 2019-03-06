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

class SSPC_tra2gc{

  double _th; 
  double _th2;
	int _root, _dir;
	int _problem;
	int _siz;
	char *_output_fname;
	char *_position_fname;
	char  *_vecchr;
	WEIGHT *_occ_w;
  QUEUE_INT *_itemary;
  QUEUE_INT *_buf;
  size_t _buf_end;
  QUEUE_ID _i;
	char *_ERROR_MES;


	ITEMSET _II;
	TRSACT _TT;

	void read_param(int argc, char* argv[]);
	void _error(void);

	WEIGHT *init1 ( FILE **fp);
	QUEUE_INT **init2 ();

	void output ( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id);
	void *iter (void *p);
	WEIGHT comp ( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq);
	void comp2 (QUEUE_ID i, QUEUE_ID x, WEIGHT c, WEIGHT wi, WEIGHT wx, double sq, QUEUE_INT *cnt, FILE *fp, QUEUE *itemset, int core_id);

	void SSPCCORE();
	

	void preLOAD(){

	  if ( _TT.exist_fname() ){
	  	_TT.load();
  		if (_ERROR_MES) EXIT; 
  	}

	}

	/* allocate arrays and structures */
	void preALLOC(){

		QUEUE_ID siz  = _TT.get_clms();
		QUEUE_ID siz2 = _TT.get_t();
	 	PERM *perm = _TT.get_perm();
	 	
	 	int f = PROBLEM_OCC_W +PROBLEM_VECCHR;

	  PERM *p;
		int j;

 		if ( f&(PROBLEM_OCC_W+PROBLEM_OCC_PW) ) calloc2 (_occ_w, siz+2, goto ERR);
	  if ( f&PROBLEM_ITEMARY ) calloc2(_itemary, siz+2, goto ERR);
	  if ( f&PROBLEM_VECCHR )  calloc2(_vecchr, siz2+2, goto ERR);


	  _II.alloc(_output_fname, perm, siz, 0);
	  if ( _II.get_target()<siz && _II.get_perm() )
      FLOOP (j, 0, _II.get_item_max()){ if ( _II.get_target() == _II.get_perm(j) ){ _II.set_target(j); break; } }

	  return;

  	ERR:;

  	EXIT;
}




	public :

	SSPC_tra2gc():
		_root(0),_dir(0),_problem(0),_siz(0),
		_output_fname(NULL),_position_fname(NULL),_vecchr(NULL),
		_occ_w(NULL),_itemary(NULL),_buf(NULL),
		_buf_end(0),_i(0),_ERROR_MES(NULL){}

	int run(int argc ,char* argv[]);
	
};