/*  QUEUE based Transaction library, including database reduction.
            25/Nov/2007   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#pragma once
#define WEIGHT_DOUBLE

#include <iostream>
using namespace std;

#include"vec.hpp"
#include"base.hpp"
#include"file2.hpp"
#include"filecount.hpp"

#define TRSACT_FRQSORT 65536  // sort transactions in decreasing order
#define TRSACT_ITEMWEIGHT 131072  // initialize itemweight by transaction weights
#define TRSACT_SHRINK 262144  // do not allocate memory for shrink, but do for mining
#define TRSACT_MULTI_STREAM 524288  // separate the datasets at each empty transaction
#define TRSACT_UNION 1048576  // take union of transactions, at the database reduction
#define TRSACT_INTSEC 2097152 // take intersection of transactions, at the database reduction
#define TRSACT_MAKE_NEW 4194304  // make new transaction for each 
#define TRSACT_DELIV_SC 16777216  // look T->sc when delivery
#define TRSACT_WRITE_PERM 67108864  // write item-order to file
#define TRSACT_1ST_SHRINK 134217728  // write item-order to file


// default weight of the transaction, for missing weights in weight file
#ifndef TRSACT_DEFAULT_WEIGHT
 #define TRSACT_DEFAULT_WEIGHT 0  
#endif

#ifndef TRSACT_MAXNUM 
 #define TRSACT_MAXNUM 20000000LL
#endif

struct TrsactParams {

	//ALL
	int _flag;
	char *_fname;
	char *_wfname;
	char *_iwfname;
	char *_fname2;
	
	//lcm , lcmseq
	int _flag2;
	
	//lcmseq
	bool _eleflg;

	// filecountで必要
	LimitVal _limVal;

	TrsactParams():
		_flag(0),_flag2(0),
  	_fname(NULL),_fname2(NULL),_wfname(NULL),_iwfname(NULL),
  	_eleflg(false){}

};

struct BaseStatusWithWeight{ //for lcm
	int _bnum;
  int _bblock;
  int _wnum;
  int _wblock;
  VEC_ID _new_t;
};

struct BaseStatus{//for lcmseq
	int _bnum;
  int _bblock;
  VEC_ID _new_t;
};



// LCM LCMseqで分ける
class TRSACT {

	TrsactParams _params;

  FILE_COUNT _C;
	
  SETFAMILY _T;   // transaction

  VECARY<WEIGHT> _w;
  WEIGHT *_pw;  // weight/positive-weight of transactions

	// #items in original file, max size of clms, and max of (original item, internal item)
  QUEUE_INT  _clm_max; 

	// #transactions in the original file
  VEC_ID  _row_max; 

  WEIGHT _total_pw; 
  WEIGHT _th;  // threshold for frequency of items
  PERM *_perm, *_trperm; // original item permutation loaded from permutation file (and inverse)


  // for finding same transactions
  // queue of non-empty buckets, used in find_same_transactions  
  QUEUE _jump, *_OQ;   
  KGLCMSEQ_QUE _jumpELE, *_OQELE;   // queue of non-empty buckets, used in find_same_transactions  


  VEC_ID *_mark;  // marks for transactions
  QUEUE_INT **_shift;  // memory for shift positions of each transaction
  char *_sc;   // flag for non-active (in-frequent) items 
  
	// for extra transactions
	VEC_ID _new_t;     // the start ID of un-used transactions
	BASE _buf;   // buffer for transaction
	BASE _wbuf;   // buffer for itemweights


	/*   print transactions */
	void prop_print (void);


	// allocate memory, set permutation, and free C.clmt,rowt,rw,cw 
	void _alloc(void);

	// load the file to allocated memory according 
	// to permutation, and free C.rw, C.cw 
	void _file_read( IFILE2 &fp,  IFILE2 &fp2 , int flag);

	/*  copy transaction t to tt (only items i s.t. pw[i]>=th)                 **/
	void copy ( VEC_ID tt, VEC_ID t, QUEUE_INT end);

	void _sort(void);
	void _sortELE(void);
	void _OccAlloc(void);


	/*  intersection of transaction t and tt (only items i s.t. pw[i]>=th)     **/
	/*  shift is the array of pointers indicates the start of each transaction **/
	void suffix_and ( VEC_ID tt, VEC_ID t);

	/*  take union of transaction t to tt (only items i s.t. pw[i]>=th)        */
	/* CAUSION: t has to be placed at the last of trsact_buf2.                 */ 
	/*   if the size of t inclreases, the following memory will be overwrited  */
	/* if memory (T->buf) is short, do nothing and return 1               */
	void itemweight_union (VEC_ID tt, VEC_ID t);
	
	/*****/
	/* remove duplicated transactions from occ, and add the weight of the removed trsacts to the representative one */
	/* duplicated trsacts are in occ[item_max]. Clear the queue when return */
	/* T->flag&TRSACT_MAKE_NEW: make new trsact for representative
   T->flag&TRSACT_INTSEC: take suffix intersection of the same trsacts
   T->flag&TRSACT_UNION: take union of the same trsacts */
	//void merge_trsact (QUEUE *o, QUEUE_INT end);
	void merge_trsact(QUEUE_INT end);

	void removeDupTrsacts();
	void removeDupTrsacts(KGLCMSEQ_QUE *o);

	/**************************************************************/
	/* Find identical transactions in a subset of transactions, by radix-sort like method */
	/* infrequent items (refer LCM_occ) and items larger than item_max are ignored */
	/* INPUT: T:transactions, occ:subset of T represented by indices, result:array for output, item_max:largest item not to be ignored */
	/* OUTPUT: if transactions i1, i2,..., ik are the same, they have same value in T->mark[i]
	 (not all) isolated transaction may have mark 1 */
	/* use 0 to end-1 of QQ temporary, and QQ[i].t and QQ[i].s have to be 0. */
	/*************************************************************************/
	void find_same ( QUEUE *occ, QUEUE_INT end);
	
	/* remove the unified transactions from occ (consider T->occ_unit) */
	void reduce_occ (QUEUE *occ);

	void Mque_allocELE(VECARY<VEC_ID> &p){
		size_t cmn_size_t = 0;

		for(VEC_ID cmm_vecid=0; cmm_vecid < _T.get_clms() ; cmm_vecid++){
			cmn_size_t += p[cmm_vecid];
		}

		_OQELE = new KGLCMSEQ_QUE[_T.get_clms()+1];

		char *cmn_pnt;
		try{ //malloc2
			if(!( cmn_pnt = (char *)malloc(sizeof(char)* ((cmn_size_t+(_T.get_clms())+2)*(sizeof(KGLCMSEQ_ELM)))  ))){
				throw("memory allocation error : malloc2");
			}
		}catch(...){
			delete[] _OQELE;
			throw;
		}

		for(VEC_ID cmm_vecid=0; cmm_vecid < _T.get_clms() ; cmm_vecid++){
			_OQELE[cmm_vecid].set_end(p[cmm_vecid]);
			_OQELE[cmm_vecid].set_v((KGLCMSEQ_ELM *)cmn_pnt);
			cmn_pnt += sizeof(KGLCMSEQ_ELM) * (p[cmm_vecid]+(1));
		}
	}


	public:

	TRSACT(void):
  	_clm_max(0), _row_max(0),_total_pw(0),
  	_perm(NULL), _trperm(NULL),_pw(NULL),
		_th(1),_mark(NULL),_shift(NULL),_OQ(NULL),_sc(NULL),_new_t(0)
		{}
		
		
	~TRSACT(){}


	// use by sspc 
	VEC_ID adjust_sep(VEC_ID sep){
		if(_params._flag&LOAD_TPOSE){ return _C.adjust_ClmSep(sep);}
		else                { return _C.adjust_RowSep(sep);}
	}

	int addjust_lenlb(int ub,int lb){

	  for (VEC_ID i=0 ; i<_T.get_t() ; i++){
    	if ( _T.get_vt(i) <= ub ){ lb = i; break; }
    }
    return lb;
	}

	
	void setParams(TrsactParams tpara)
	{
		_params = tpara;
	  _C.setLimit(_params._limVal);
	}
	
	int load(void);

	// using only sspc 
	void initialQueSet(){

		VEC_ID i, e;
 		QUEUE_INT *x;

		QUEUE *occ = &_OQ[_T.get_clms()];
		QUEUE_INT M= _T.get_clms();

		for (i=0 ; i< occ->get_t() ; (i)++){ // occ? occ->get_t(): t
   		e = occ->get_v(i);
			for ( x=_T.get_vv(e) ; *x < M ; x++){ 
				_OQ[*x].push_back(e);
			}
		}


	}


	// using only sspc 
	QUEUE_INT **skipLaegeFreqItems(int len_lb)
  {
	  QUEUE_INT **o=NULL;

		// skipping items of large frequencies
		if ( _params._flag & LOAD_SIZSORT ){

			o = new QUEUE_INT *[_T.get_clms()];

			for ( QUEUE_ID i =0 ; i < _T.get_clms() ; i++){
				o[i] = _OQ[i].get_v();
				_OQ[i].set_v( _OQ[i].get_t(),INTHUGE);// 別メソッドにできる

				QUEUE_ID j;
	      for (j=0 ; _OQ[i].get_v(j) < len_lb ; j++); //skip
	      _OQ[i].move(j);
			}
	  }
  	return o;
  }



	
  void multipule_w(){
		for(QUEUE_INT i =0 ; i < _T.get_clms(); i++){
			_w[i] *= _w[i];
		}
	}

	void setOQend(){
		for(QUEUE_INT i =0 ;i < _T.get_clms();i++){
			_OQ[i].set_end(0);
		}
	}

	void clrMark_T(int i,char *mark){
		_T.clrMark(i,mark);
	}
	
	void clrMark_Q(int i,char *mark){
		_OQ[i].clrMark(mark);
	}

	void clrOQend(int i){ _OQ[i].endClr(); }
	void clrOQt(int i)  { _OQ[i].tClr(); }

	void clrOQendByJump(){ 

		for(QUEUE_INT *x = _jump.begin(); x<_jump.end() ;x++ ){ 
  	 _OQ[*x].endClr();
		}

	}

	//再考
	QUEUE_INT * beginJump(){ return _jump.begin();} 
	QUEUE_INT * endJump(){ return _jump.end();} 

	void clrOQendELE(int i){ _OQELE[i].endClr(); }
	void clrOQtELE(int i){ _OQELE[i].tClr(); }

	PERM * convPerm(char* fname){ 

		if ( fname ){ 

			PERM *p = NULL;

			IFILE2::ARY_Load( p ,fname);

			if ( _perm ){
				for(int j =0 ;j < _T.get_clms() ; j++){
					_perm[j] = p[_perm[j]];
      	}
	    	delete [] p;
	    }
		  else {
    		_perm = p;
    	}
		}
		return _perm;
	}


	//アクセッサ

	void set_perm(PERM * perm){ _perm = perm;}

	PERM * get_perm(void){ return _perm;}


	VEC_ID get_clms(void){ return _T.get_clms();}
	size_t get_eles(void){ return _T.get_eles();}
	QUEUE_INT get_clms_org(void){ return _C.clms();}// _clms_org;}
	VEC_ID get_rows_org(void){ return _C.rows();}//_rows_org;}


	PERM* get_trperm(void) { return _trperm; } //かり

	char * get_sc(){return _sc; }
	WEIGHT get_total_w_org(void){ return _C.get_total_w_org();}
	WEIGHT get_total_pw_org(void){ return _C.get_total_pw_org();}


	bool incNega(){ return _C.existNegative(); }

	bool isShrink(){ return (_params._flag2 & TRSACT_SHRINK ); }

	VEC_ID get_t(void){ return _T.get_t();}
	WEIGHT get_w(int i){ return _w[i];}

	WEIGHT * beginTw(int i){ return _T.get_w(i);}


	QUEUE_INT * beginTv(int i){ return _T.get_vv(i);}
	QUEUE_INT * endTv(int i)  { return _T.get_vv(i) + _T.get_vt(i);}


	QUEUE_INT * beginOQv(int i){ return _OQ[i].get_v();}
	QUEUE_INT * endOQv  (int i){ return _OQ[i].get_v()+ _OQ[i].get_t();}
	QUEUE_INT * startOQv(int i){ return _OQ[i].get_v()+_OQ[i].get_s();}


	void resizeOQ(int i,int v){ _OQ[i].resize(v); }

	//	==== use in seq	====
	QUEUE_ID get_OQ_tELE(int i){ return _OQELE[i].get_t(); }
	KGLCMSEQ_ELM * beginOQvELE(int i){return _OQELE[i].get_v();}
	KGLCMSEQ_ELM * endOQvELE(int i){  return _OQELE[i].get_v()+ _OQELE[i].get_t();}

	QUEUE_INT get_Tvv(int i,int j) { return _T.get_vv(i,j);}
	void set_Tvv(int i,int j,QUEUE_INT v) { _T.set_vv(i,j,v);}
	char  get_sc(int i){return _sc[i]; }
	void set_sc(int i , char v){ _sc[i]=v; }

  void inc_OQtELE(int e){	_OQELE[e].inc_t(); }
  void inc_OQendELE(int e){	_OQELE[e].inc_end(); }
	int get_OQ_endELE(int i){ return _OQELE[i].get_end(); }

	VEC_ID get_mark(int i){ return _mark[i]; }

	QUEUE_ID get_Tvt(int i) { return _T.get_vt(i);}

	KGLCMSEQ_ELM * get_OQ_vELE(int i){ return _OQELE[i].get_v(); }
	KGLCMSEQ_QUE *  get_OQELE(){ return  _OQELE;} 

	VEC_ID get_row_max(void){ return _row_max;}
	QUEUE_INT get_clm_max(void){ return _clm_max;}

	void swap_Tvv(int i,int j){ _T.swap_vv(i,j); }

	//	==== use in seq	====



	//再考
	bool exist_Tw(void)	{ return _T.exist_w();} 

	bool exist_perm(void) { return _perm!=NULL; }
	bool exist_sc(void) { return _sc!=NULL; }
	bool exist_wfname(void) { return _params._wfname!=NULL; }


	QUEUE* getp_jump(){ return &_jump;} //再考？
	QUEUE* getp_OQ(int i){ return  &_OQ[i];} 

	KGLCMSEQ_QUE* getp_OQELE(int i){ return  &_OQELE[i];} 


	// LCM
	BaseStatusWithWeight getBaseStsW(void){
		BaseStatusWithWeight rtn;
		rtn._bnum   = _buf.get_num();
		rtn._bblock = _buf.get_block_num();
		rtn._wnum   = _wbuf.get_num();
		rtn._wblock = _wbuf.get_block_num();
		rtn._new_t  = _new_t;
		return rtn;
	}  

	void setBaseStsW(BaseStatusWithWeight &bsts){
		_buf.set_num(bsts._bnum);
		_buf.set_block_num(bsts._bblock);
		_wbuf.set_num(bsts._wnum);
		_wbuf.set_block_num(bsts._wblock);
		_new_t=bsts._new_t;
	}  

	// LCMSEQ
	BaseStatus getBaseSts(void){
		BaseStatus rtn;
		rtn._bnum   = _buf.get_num();
		rtn._bblock = _buf.get_block_num();
		rtn._new_t  = _new_t;
		return rtn;
	}  

	void setBaseSts(BaseStatus &bsts){
		_buf.set_num(bsts._bnum);
		_buf.set_block_num(bsts._bblock);
		_new_t=bsts._new_t;
	}  

	QUEUE dup_OQ(int i){
		return _OQ[i].dup();
	}

	void sizSort(int i,QUEUE_INT **o){
    _OQ[i].add_t ( _OQ[i].get_v() - o[i]);
    _OQ[i].set_v ( o[i]);
  }
	void reallocW(void){
		VEC_ID size =  MAX(_T.get_t(),_T.get_clms())+1;
		_w.realloc2(size);
		for(size_t i=0; i<size;i++){ _w[i] = 1;}
	}

	// normalize the vectors for inner product
	//sspc
	void normalize(WEIGHT *w){
		// org ARY_FILL
	  for(QUEUE_INT i=0; i<_T.get_clms() ; i++){ w[i] = 0; } 
		
		for (VEC_ID i=0 ; i < _T.get_t();i++){
			WEIGHT *y =_T.get_w(i);
			for(QUEUE_INT *x = _T.get_vv(i); x < _T.get_vv(i)+_T.get_vt(i);x++){
				w[*x] += (*y)*(*y); y++;
			}
		}
		for(QUEUE_INT i=0;i<_T.get_clms();i++){ w[i] = sqrt(w[i]); }

		for(VEC_ID i=0 ; i<_T.get_t();i++){
  	  WEIGHT *y = _T.get_w(i);
			for(QUEUE_INT *x=_T.get_vv(i);x<_T.get_vv(i)+_T.get_vt(i); x++){
				 *y /= w[*x]; y++;
			}
		}
	}

	void calloc_sc(VEC_ID sise){
		_sc = new char[sise]();	//calloc2
	}

	void delivery( WEIGHT *w, WEIGHT *pw, QUEUE *occ, QUEUE_INT m);
	void deliv( QUEUE *occ, QUEUE_INT m);


	void print ( QUEUE *occ, PERM *p);

	/**************************************************************/
	/* Find identical transactions in a subset of transactions, by radix-sort like method */
	/* infrequent items (refer LCM_occ) and items larger than item_max are ignored */
	/* INPUT: T:transactions, occ:subset of T represented by indices, result:array for output, item_max:largest item not to be ignored */
	/* OUTPUT: if transactions i1, i2,..., ik are the same, they have same value in T->mark[i]
	 (not all) isolated transaction may have mark 1 */
	/* use 0 to end-1 of QQ temporary, and QQ[i].t and QQ[i].s have to be 0. */
	/*************************************************************************/
	void find_same ( KGLCMSEQ_QUE *occ, QUEUE_INT end);

	/*****/
	/* remove duplicated transactions from occ, and add the weight of the removed trsacts to the representative one */
	/* duplicated trsacts are in occ[item_max]. Clear the queue when return */
	/* T->flag&TRSACT_MAKE_NEW: make new trsact for representative
   T->flag&TRSACT_INTSEC: take suffix intersection of the same trsacts
   T->flag&TRSACT_UNION: take union of the same trsacts */
	//void merge_trsact (QUEUE *o, QUEUE_INT end);
	void merge_trsact(KGLCMSEQ_QUE *o, QUEUE_INT end);


	void dbReduction(QUEUE *occ, QUEUE_INT item){

   	find_same(occ, item);
   	merge_trsact(item);
    reduce_occ(occ);
	}


	void printMes(const char *frm ,...){

			if( _params._flag&1 ){
				va_list ap;
				va_start(ap,frm);
				vfprintf(stderr,frm,ap);
				va_end(ap);
			}
		}
		
} ;

//#define TRSACT_NEGATIVE 33554432  // flag for whether some transaction weights are negative or not 
//, _clms_end;_clms_end(0),_clms_org,_clms_org(0),
//  size_t _eles_org;  // #elements in the original file
//	QUEUE* get_OQ(){ return  _OQ;} 
//	QUEUE_INT* get_OQ_v(int i){ return _OQ[i].get_v(); }
//	bool exist_trperm(void) { return _trperm!=NULL; }


//  void inc_OQt(int e){	_OQ[e].inc_t(); }
//	int get_OQ_end(int i){ return _OQ[i].get_end(); }
//  void inc_OQend(int e){	_OQ[e].inc_end(); }

// 	PERM get_trperm(int i) { return _trperm[i]; }
//	PERM   get_perm(int i){ return _perm[i];}
//	WEIGHT * endTw(int i)  { return _T.get_w(i)  + _T.get_vt(i);}
