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
#include "problem.hpp"
#include "itemset.hpp"
#include"file2.hpp"
#include"filecount.hpp"

class OQueue{

	QUEUE * _oq;
	size_t _size; //必要ないかも
	
	public:

	OQueue(void):_oq(NULL),_size(0){}
	
	QUEUE_INT* start(size_t i){ return _oq[i].start(); }
	QUEUE_INT* begin(size_t i){ return _oq[i].begin(); }
	QUEUE_INT* end(size_t i)  { return _oq[i].end(); }
	QUEUE_INT * get_v(size_t i){ return _oq[i].get_v(); }
	QUEUE_INT get_v(size_t i,size_t j){ return _oq[i].get_v(j); }


	void clrMark(size_t i,char *mark)	{ _oq[i].clrMark(mark); }
	void endClr(size_t i)  { return _oq[i].endClr(); }
	void set_end(size_t i,int v){ _oq[i].set_end(v); }
	void set_sentinel(size_t i){ _oq[i].set_v( _oq[i].get_t(),INTHUGE);}
	void move(size_t i,size_t j){ return _oq[i].move(j); }

	void setfromT(SETFAMILY &T){
		QUEUE_INT M = T.get_clms();
		for (VEC_ID i=0 ; i< _oq[M].get_t() ; i++){ 
	  	VEC_ID e = _oq[M].get_v(i);
			for ( QUEUE_INT * x = T.get_vv(e) ; *x < M ; x++){ 
				_oq[*x].push_back(e);
			}
		}
	}

	// 仮
	QUEUE * getOQ(void){ return _oq; }

	void alloc(SETFAMILY &T){
		VEC_ID *p = T.counting();
		QUEUE_INT clm_max = p[0];
		size_t cmmsize = p[0];
		for(int cmm = 1 ; cmm < T.get_clms() ;cmm++ ){
			cmmsize += p[cmm];
			if( clm_max < p[cmm]){ clm_max = p[cmm]; }
		}
		
		_oq = new QUEUE[T.get_clms()+1]; 
		_size = T.get_clms()+1;

		QUEUE_INT *cmn_pnt;
		try{
			cmn_pnt = new QUEUE_INT[cmmsize+T.get_clms()+2]; //malloc2
		}catch(...){
			delete[] _oq;
			throw;
		}
		size_t posx=0;
		for(VEC_ID cmmv =0; cmmv < T.get_clms() ; cmmv++){
			_oq[cmmv].alloc( p[cmmv],cmn_pnt+posx );
			posx += p[cmmv]+1;
		}
		_oq[T.get_clms()].alloc( MAX(T.get_t(), clm_max));

		for(size_t i=0 ; i < T.get_clms()+1 ; i++ ){
			_oq[i].endClr();
		}
		_oq[T.get_clms()].initVprem(T.get_t());  //ARY_INIT_PERM
		delete [] p;
	}
	void prefin(size_t i ,QUEUE_INT * o){
	  _oq[i].add_t( _oq[i].get_v() - o);
  	_oq[i].set_v ( o);
  }

};


class KGSSPC{

	// receive parameter
	// SSPC
	int _problem;
  double _th2;
	double _th;

	char *_output_fname2;
	char *_table_fname;
	char *_outperm_fname;
	char *_output_fname;

	bool _progressFlag;
	bool _showFlag;
	bool _rowSortDecFlag; // for using -b -B  LOAD_SIZSORT+LOAD_DECROWSORT
	bool _rmDupFlag;      // for using para : 1


	LONG _itemtopk_item;
	LONG _itemtopk_item2;
	LONG _itemtopk_end;

	ItemSetParams _ipara;


	int _len_ub;
	int _len_lb;


	// filecountで必要
	LimitVal _limVal;

  FILE_COUNT _C;
  SETFAMILY _T;   // transaction
  VECARY<WEIGHT> _w;
  WEIGHT *_pw;  // weight/positive-weight of transactions
	PERM *_perm,*_trperm;
  QUEUE _jump;
  OQueue _OQ;

	char *_wfname;
	char *_iwfname;
	char *_fname2;
	int _tflag;
	char *_fname;

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

	void output ( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id);

	void *iter(void *p);

	WEIGHT comp( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq);
	void   comp2( QUEUE_ID i, QUEUE_ID x, 
								WEIGHT c, WEIGHT wi, WEIGHT wx, 
								double sq, QUEUE_INT *cnt, OFILE2 *fp, 
								QUEUE *itemset, int core_id
							);

	void _SspcCore();

	/* allocate arrays and structures */
	void _preALLOC();
	
	int setArgs(int argc, char *argv[]);

 	void help();

	public :
	KGSSPC():
		_problem(0),_siz(0),_trperm(NULL),_perm(NULL),
		_th(0),_th2(0),_progressFlag(false),_showFlag(false),
		_output_fname(NULL),_output_fname2(NULL),
		_outperm_fname(NULL),_table_fname(NULL),
		_positPERM(NULL),_vecchr(NULL),_rowSortDecFlag(false),
		_occ_w(NULL),_itemary(NULL),_pw(NULL),
		_buf_end(0),_tflag(LOAD_INCSORT),_rmDupFlag(false),
		_wfname(NULL),_iwfname(NULL),_fname2(NULL),_fname(NULL),
		_len_ub(INTHUGE),_len_lb(0),
		_itemtopk_item(0),_itemtopk_item2(0),_itemtopk_end(0)
		{}

	int run(int argc ,char* argv[]);
	
	std::vector<LONG> iparam(){ 
		std::vector<LONG> rtn(2);
		rtn[0] = _II.get_solutions(); 
		rtn[1] = _C.clms();
		return rtn;
	}
	
	static std::vector<LONG> mrun(int argc ,char* argv[]);

};
//QUEUE *_OQ;   
//  QUEUE_INT  _clm_max; // #items in original file, max size of clms, and max of (original item, internal item)
//  VEC_ID  _row_max; 	// #transactions in the original file	
// Trsact
//	VEC_ID _new_t;

//		_root(0),_dir(0),	_sep(0),
//  int _dir; 
//  int _root; 
//  int _sep;

//TRSACT _TT;


