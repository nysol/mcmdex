/* SSPC: Similar Set Pair Comparison */
/* 2007/11/30 Takeaki Uno,   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about this code for the users.
   For the commercial use, please make a contact to Takeaki Uno. */


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

#include <vector>
//#include "trsact.hpp"
#include"vec.hpp"
#include"oqueue.hpp"
#include "problem.hpp"
#include "itemset.hpp"
#include"file2.hpp"
#include"filecount.hpp"

//-k -Kは別にする？
struct kgSspcParams {

	bool _showFlag;
	bool _progressFlag;

	int _problem;
  double _th2;
	double _th;

	char *_output_fname2;
	char *_table_fname;
	char *_outperm_fname;
	char *_output_fname;

	bool _rowSortDecFlag; // for using -b -B  LOAD_SIZSORT+LOAD_DECROWSORT
	bool _rmDupFlag;      // for using para : 1


	LONG _itemtopk_item;
	LONG _itemtopk_item2;
	LONG _itemtopk_end;

	ItemSetParams _ipara;

	LimitVal _limVal;
	char *_wfname;
	char *_iwfname;
	char *_fname2;
	int _tflag;
	char *_fname;

	int _len_ub;
	int _len_lb;

	// -c 用 パラメータ
  int _dir; 
  int _root; 
  int _sep;

	// -P 用 パラメータ
	char *_table_name;

	kgSspcParams():
		_problem(0),_th(0),_th2(0),_progressFlag(false),_showFlag(false),
		_output_fname(NULL),_output_fname2(NULL),
		_outperm_fname(NULL),_table_fname(NULL),
		_rowSortDecFlag(false),_tflag(LOAD_INCSORT),_rmDupFlag(false),
		_wfname(NULL),_iwfname(NULL),_fname2(NULL),_fname(NULL),_table_name(NULL),
		_len_ub(INTHUGE),_len_lb(0),
		_itemtopk_item(0),_itemtopk_item2(0),_itemtopk_end(0),
		_root(0),_dir(0),	_sep(0){}


	int setArgs(int argc, char *argv[]){

		int iflag = 0;
		int c=1;

		if ( argc < c+3 ){ return 1; }

		if ( !strchr (argv[c], '_') ){ iflag |= SHOW_MESSAGE;  _showFlag= true; }
		if ( strchr (argv[c], '%') ) { iflag |= SHOW_PROGRESS; _progressFlag= true;}
		if ( strchr (argv[c], '+') ) iflag |= ITEMSET_APPEND;
		if ( strchr (argv[c], 'f') ) iflag |= ITEMSET_FREQ;
		if ( strchr (argv[c], 'Q') ) iflag |= ITEMSET_PRE_FREQ;

		if ( strchr (argv[c], 'i') )      _problem = SSPC_INCLUSION;
		else if ( strchr (argv[c], 'I') ) _problem = SSPC_SIMILARITY;
		else if ( strchr (argv[c], 'T') ) _problem = SSPC_INTERSECTION;
		else if ( strchr (argv[c], 's') ) _problem = SSPC_MININT;
		else if ( strchr (argv[c], 'S') ) _problem = SSPC_MAXINT;
		else if ( strchr (argv[c], 'R') ) _problem = SSPC_RESEMBLANCE;
		else if ( strchr (argv[c], 'P') ) _problem = SSPC_PMI;
		else if ( strchr (argv[c], 'F') ) _problem = SSPC_FVALUE;
		else if ( strchr (argv[c], 'C') ) _problem = SSPC_INNERPRODUCT;
		else {
			fprintf(stderr,"i, I, s, S, R, T or C command has to be specified\n");
			return 1; 
		}

		if ( strchr (argv[c], '#') ) _problem |= SSPC_COUNT;
		if ( strchr (argv[c], 'N') ) _problem |= PROBLEM_NORMALIZE;
		if ( strchr (argv[c], 'n') ) _problem |= SSPC_NO_NEIB;
		if ( strchr (argv[c], 'Y') ) _problem |= SSPC_POLISH;
		if ( strchr (argv[c], 'y') ) _problem |= SSPC_POLISH2;

		if ( !strchr (argv[c], 't') ) _tflag |= LOAD_TPOSE;
		if ( strchr (argv[c], 'E') )  _tflag |= LOAD_ELE;
		if ( strchr (argv[c], 'w') )  _tflag |= LOAD_EDGEW;
		if ( strchr (argv[c], '1') ){ _rmDupFlag = true ; }// _tflag |= LOAD_RM_DUP;
		if ( strchr (argv[c], '0') ) _problem |= SSPC_COMP_ITSELF;
	
		c++;
  
		while ( argv[c][0] == '-' ){
			switch (argv[c][1]){
				case 'K': 
					_ipara._topk_k = atoi(argv[c+1]);
				break; case 'k': 
					_itemtopk_item  = atoi(argv[c+1]); 
					_itemtopk_item2 = 1;

				break; case 'L': 
					if ( argv[c][2] == 'L' ) _limVal._row_lb_ = atof(argv[c+1]);
					else 										 _limVal._row_lb  = atoi(argv[c+1]); 

				break; case 'U':  
					if ( argv[c][2] == 'U' ) _limVal._row_ub_ = atof(argv[c+1]);
					else									   _limVal._row_ub  = atoi(argv[c+1]);

				break; case 'l':  
					if ( argv[c][2] == 'l' ) _limVal._clm_lb_ = atof(argv[c+1]);
					else                     _limVal._w_lb    = atof(argv[c+1]);

				break; case 'u': 
					if ( argv[c][2] == 'u' ) _limVal._clm_ub_ = atof(argv[c+1]);
					else                     _limVal._w_ub    = atof(argv[c+1]);

				break; case 'w': 
					_wfname = argv[c+1];

				break; case 'W':
					_iwfname = argv[c+1];
 
	      break; case 'c':  //別途考える
					_dir = 1; 
					_sep = _root = atoi(argv[c+1]) ;

				break; case '2': 
					_fname2 = argv[c+1];

				break; case '9': 
					_th2 = atof(argv[c+1]);
					c++; 
					_output_fname2 = argv[c+1];

	      break; case 'P': 
	      	_table_fname = argv[c+1];
          if ( _tflag & LOAD_TPOSE ){ _tflag -= LOAD_TPOSE;}
          else{                       _tflag |= LOAD_TPOSE;}

				break; case 'b': 
					_len_lb = atoi(argv[c+1]);

				break; case 'B': 
					_len_ub = atoi(argv[c+1]);

				break; case 'T': 
					_th = atoi(argv[c+1]);
					if ( argv[c][2] == 'T' ){ _problem |= SSPC_OUTPUT_INTERSECT; }

				break; case 'M': 
					if ( atoi(argv[c+1]) <= 0 || atoi(argv[c+1])>CORE_MAX){
						fprintf(stderr,"number of cores has to be in 1 to: %d\n",atoi(argv[c+1]));
						return 1;
					}
					_ipara._multi_core = atoi(argv[c+1]);
          	
				break; case '#': 
					_ipara._max_solutions = atoi(argv[c+1]);

				break; case ',': 
					_ipara._separator = argv[c+1][0];

				break; case 'Q': 
					_outperm_fname = argv[c+1];

				break; default: 
					goto NEXT;
			}
   		c += 2;
			if ( argc<c+2 ){ 
				return 1; 
			}
		}

		NEXT:;
	
		_fname = argv[c];
		_ipara._frq_lb = atof(argv[c+1]);
	
		if ( argc>c+2 ) _output_fname = argv[c+2];

		_ipara._flag = iflag;

		// using -b -B  :ignore pairs whose maximum/minimum common item
		if ( _len_ub < INTHUGE || _len_lb > 0 ){ 
			_rowSortDecFlag = true;
		}
	
		return 0;	
	}
};




class KGSSPC{

	// パラメータ
	kgSspcParams _P;


  FILE_COUNT _C;
  SETFAMILY _T;   // transaction
  VECARY<WEIGHT> _w;
  WEIGHT *_pw;    // weight/positive-weight of transactions

	PERM *_perm,*_trperm;
  QUEUE _jump;
  OQueue _OQ;

	int _siz; // intersection size

	PERM *_positPERM; // ( org _position_fname)
	WEIGHT *_occ_w;

	// POLISH2のときのみ利用
	//（y:output elements of each set that 
	// contribute to no similarity (fast with much memory use)）
  QUEUE_INT *_itemary;
	VECARY<QUEUE_INT> _buf;
  size_t _buf_end;

	// POLISH or POLISH2の時のみ
	char  *_vecchr;  

	ITEMSET _II;

	void _output( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id);

	void *_iter(void *p);

	WEIGHT _comp( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq);
	void   _comp2( QUEUE_ID i, QUEUE_ID x, 
								WEIGHT c, WEIGHT wi, WEIGHT wx, 
								double sq, QUEUE_INT *cnt, OFILE2 *fp, 
								QUEUE *itemset, int core_id
							);

	void _SspcCore();

	/* allocate arrays and structures */
	void _preALLOC();
	
	int _runMain(void);

	public :

	KGSSPC():
		_siz(0),_trperm(NULL),_perm(NULL),
		_positPERM(NULL),_vecchr(NULL),
		_occ_w(NULL),_itemary(NULL),_pw(NULL),
		_buf_end(0){}

	int run(kgSspcParams para);
	int run(int argc ,char* argv[]);

	std::vector<LONG> iparam(){ 
		std::vector<LONG> rtn(2);
		rtn[0] = _II.get_solutions(); 
		rtn[1] = _C.clms();
		return rtn;
	}
	
	static std::vector<LONG> mrun(int argc ,char* argv[]);

};


