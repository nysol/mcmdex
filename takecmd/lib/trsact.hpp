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
#define TRSACT_ALLOC_OCC 8388608  // make new transaction for each 
#define TRSACT_DELIV_SC 16777216  // look T->sc when delivery
#define TRSACT_NEGATIVE 33554432  // flag for whether some transaction weights are negative or not 
//#define TRSACT_INIT_SHRINK 65536  // allocate memory for database reduction
#define TRSACT_WRITE_PERM 67108864  // write item-order to file
#define TRSACT_1ST_SHRINK 134217728  // write item-order to file

#ifndef TRSACT_DEFAULT_WEIGHT
 #define TRSACT_DEFAULT_WEIGHT 0  // default weight of the transaction, for missing weights in weight file
#endif

#ifndef TRSACT_MAXNUM 
 #define TRSACT_MAXNUM 20000000LL
#endif


// LCM LCMseqで分ける
class TRSACT {

  int _flag;      
  int _flag2;  // <= いる？

  char *_fname,*_fname2;      // input file name
	char *_wfname;      //,*_wfname2 <-必要なら復活させる //weight file name 
	char *_item_wfname; //, *_item_wfname2 <-必要なら復活させる // item-weight file name
  char *_pfname; // item-permutation file name

  // lower/upper bound of #elements in a column/row. 
  // colunmn or row of out of range will be ignored
  VEC_ID _clm_lb, _clm_ub; 
  WEIGHT _w_lb, _w_ub;
  double _clm_lb_, _clm_ub_;
  QUEUE_ID _row_lb, _row_ub;
  double _row_lb_, _row_ub_;  

  FILE_COUNT _C;


  SETFAMILY _T;   // transaction

  WEIGHT *_w, *_pw;  // weight/positive-weight of transactions

  QUEUE_INT _clms_org, _clm_max, _clms_end;  // #items in original file, max size of clms, and max of (original item, internal item)

  // VEC_ID _rows_org, _row_max; // #transactions in the original file
  VEC_ID  _row_max; // #transactions in the original file

  VEC_ID _end1, _sep; // #trsact in 1st file, the ID of the last permed trsact of 1st file
  size_t _eles_org;  // #elements in the original file
  WEIGHT _total_w, _total_pw; 
  WEIGHT _th;  // threshold for frequency of items
  PERM *_perm, *_trperm; // original item permutation loaded from permutation file (and inverse)



  VEC_ID _str_num;  // number of database (itemset stream/string datasets) in T
  
  // for finding same transactions
  // queue of non-empty buckets, used in find_same_transactions  
  // _OQ,_OQELE別クラスにする？
  QUEUE _jump, *_OQ;   
  KGLCMSEQ_QUE _jumpELE, *_OQELE;   // queue of non-empty buckets, used in find_same_transactions  


  VEC_ID *_mark;  // marks for transactions
  QUEUE_INT **_shift;  // memory for shift positions of each transaction
  char *_sc;   // flag for non-active (in-frequent) items 
  
    // for extra transactions
  VEC_ID _new_t;     // the start ID of un-used transactions
  BASE _buf;   // buffer for transaction
  BASE _wbuf;   // buffer for itemweights

	/*****************************************/
	/* scan file "fp" with weight file wfp and count #items, #transactions in the file. */
	/*   count weight only if wfp!=NULL                                      */
	/* T->rows_org, clms_org, eles_org := #items, #transactions, #all items  */
	/*   ignore the transactions of size not in range T->clm_lb - clm_ub     */ 
	/* T->total_w, total_pw := sum of (positive) weights of transactions     */
	/* C.clmt[i],C.cw[i] := the number/(sum of weights) of transactions including i  */
	/****************************************/
	//int file_count (FILE_COUNT *C, FILE2 *fp, char *wf);

	/*   print transactions */
	void prop_print (void);

	// デストラクタへ
	void end (void){}

	/* allocate memory, set permutation, and free C.clmt,rowt,rw,cw */
	int alloc(void);

	/* load the file to allocated memory according to permutation, and free C.rw, C.cw */
	void file_read ( FILE2 *fp, FILE_COUNT *C, VEC_ID *t, int flag, char *iwfname);



	/*  copy transaction t to tt (only items i s.t. pw[i]>=th)                 **/
	void copy ( VEC_ID tt, VEC_ID t, QUEUE_INT end);


	void sort( FILE_COUNT *C);
	void sortELE( FILE_COUNT *C);

	/* occurrence deliver (only counting) */
	/* WARNING: next cell of the last item of each transaction must be INTHUGE */
	/* compute occurrence for items less than max item, in the database induced
	 by occ */

	/* if jump!=0, all i with non-zero occ[i].t will be inserted to jump */
	/* be careful for overflow of jump */
	/* if occ==NULL, scan all transactions */
	/* flag&1: count only positive weights */
	//void delivery_iter ( WEIGHT *w, WEIGHT *pw, VEC_ID t, QUEUE_INT m);
	//void delivery (QUEUE *jump, WEIGHT *w, WEIGHT *pw, QUEUE *occ, QUEUE_INT m);


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


	public:
	TRSACT(void):
  	_fname(NULL),_wfname(NULL),_item_wfname(NULL),_fname2(NULL),_pfname(NULL),
  	_flag(0),_flag2(0),_clms_org(0),_clm_max(0),_clms_end(0),
  	/*_rows_org(0),*/_row_max(0),_end1(0),_sep(0),
  	_perm(NULL), _trperm(NULL),_w(NULL),_pw(NULL),
  	_clm_lb(0),_clm_ub(VEC_ID_END),_row_lb(0),_row_ub(QUEUE_IDHUGE),
  	_clm_lb_(0.0),_clm_ub_(0.0),_row_lb_(0.0),_row_ub_(0.0),
  	_w_lb(-WEIGHTHUGE), _w_ub(WEIGHTHUGE),
  	_eles_org(0),_total_w(0),_total_pw(0),_str_num(0),
		_th(1),_mark(NULL),_shift(NULL),_OQ(NULL),_sc(NULL),_new_t(0)
		{}


		// _wfname2(NULL),_item_wfname2(NULL)
		
		
	~TRSACT(){
		//if ( _OQ ){ free2 (_OQ->_v ); free2 (_OQ[_T.get_clms()]._v); }
		/*
	  if ( _w != _pw ) free2 (_pw);
		mfree (_w, _perm, _trperm);
 		mfree (_mark, _shift, _sc, _OQ, _head, _strID);
 		*/
	}

	int loadMain(bool elef=false);

	// load transaction file to TRSACT 
	int load(
		int flag ,int flag2 ,
		char *fname,char *wfname,char *iwfname,char *fname2,
		WEIGHT w_lb,WEIGHT w_ub,double clm_lb_ ,double clm_ub_ ,
		QUEUE_ID row_lb,QUEUE_ID row_ub ,double row_lb_,double row_ub_,VEC_ID sep
	){
	  // パラメータセット
		_flag = flag;
		_flag2 = flag2;
		_fname = fname;
		_wfname = wfname;
		_item_wfname = iwfname;
		_fname2 = fname2; 
		_w_lb = w_lb;
		_w_ub = w_ub; 
		_clm_lb_ = clm_ub_; 
		_row_lb = row_lb;
		_row_ub = row_ub ;
		_row_lb_ = row_lb_;
		_row_ub_ = row_ub_;
		_sep = sep;
		return loadMain();

	}

	int load(
		int flag ,int flag2 ,
		char *fname,char *wfname,char *iwfname,char *fname2,
		WEIGHT w_lb,WEIGHT w_ub,bool eleflg=false
	){
	
		_flag = flag;
		_flag2 = flag2;
		_fname = fname;
		_wfname = wfname;
		_item_wfname = iwfname;
		_fname2 = fname2; 
		_w_lb = w_lb;
		_w_ub = w_ub; 

		return loadMain(eleflg);
		
	}


	// addjust _lenlb
	int addjust_lenlb(int ub,int lb){
		VEC_ID i;	 
	  for (i=0 ; i<_T.get_t() ; i++){
    	if ( _T.get_vt(i) <= ub ){ lb = i; break; }
    }
    return lb;
	}

	// using only sspc 
	void QUEINS(){
		VEC_ID i, e;
 		QUEUE_INT *x;

		QUEUE *occ = &_OQ[_T.get_clms()];
		VEC_ID t   = _T.get_t();
		QUEUE_INT M= _T.get_clms();
		
		for (i=0 ; i< (occ? occ->get_t(): t) ; (i)++){
   		e = occ? occ->get_v(i): i;
			for ( x=_T.get_vv(e) ; *x < M ; x++){ 
				_OQ[*x].push_back(e);
			}
		}
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

	QUEUE_INT **skipLaegeFreqItems(int len_lb)
  {
	  QUEUE_INT **o=NULL;

		// skipping items of large frequencies
		if ( _flag & LOAD_SIZSORT ){
			malloc2 (o, _T.get_clms(), exit(1));
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
	
	void clrMark_T(int i,char *mark){
		_T.clrMark(i,mark);
	}
	
	void clrMark_Q(int i,char *mark){
		_OQ[i].clrMark(mark);
	}

	void clrOQend(int i){ _OQ[i].endClr(); }
	void clrOQt(int i){ _OQ[i].tClr(); }

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



	//アクセッサ
	void set_perm(PERM * perm){ _perm = perm;}

	PERM * get_perm(void){ return _perm;}
	PERM   get_perm(int i){ return _perm[i];}
	VEC_ID get_clms(void){ return _T.get_clms();}
	size_t get_eles(void){ return _T.get_eles();}
	QUEUE_INT get_clms_org(void){ return _C.clms();}// _clms_org;}
	VEC_ID get_rows_org(void){ return _C.rows();}//_rows_org;}

	PERM get_trperm(int i) { return _trperm[i]; }
	PERM* get_trperm(void) { return _trperm; } //かり

	char * get_sc(){return _sc; }
	WEIGHT get_total_w(void){ return _total_w;}
	WEIGHT get_total_w_org(void){ return _C.get_total_w_org();}
	WEIGHT get_total_pw_org(void){ return _C.get_total_pw_org();}

	int get_bnum(void){return _buf.get_num();}  
	int get_bblock(void){return _buf.get_block_num();} 
	int get_wnum(void){return _wbuf.get_num();}  
	int get_wblock(void){return _wbuf.get_block_num();} 
	VEC_ID get_new_t(){ return _new_t;}

	int get_flag(void){ return _flag;}
	int get_flag2(void){ return _flag2;}

	VEC_ID get_t(void){ return _T.get_t();}
	VEC_ID get_sep(){ return _sep;}
	WEIGHT get_w(int i){ return _w[i];}
	
	

	WEIGHT * beginTw(int i){ return _T.get_w(i);}
	WEIGHT * endTw(int i)  { return _T.get_w(i)  + _T.get_vt(i);}


	QUEUE_INT * beginTv(int i){ return _T.get_vv(i);}
	QUEUE_INT * endTv(int i)  { return _T.get_vv(i) + _T.get_vt(i);}


	QUEUE_INT * beginOQv(int i){ return _OQ[i].get_v();}
	QUEUE_INT * endOQv(int i){ return _OQ[i].get_v()+ _OQ[i].get_t();}
	QUEUE_INT * startOQv(int i){ return _OQ[i].get_v()+_OQ[i].get_s();}

	KGLCMSEQ_ELM * beginOQvELE(int i){return _OQELE[i].get_v();}
	KGLCMSEQ_ELM * endOQvELE(int i){  return _OQELE[i].get_v()+ _OQELE[i].get_t();}


	void resizeOQ(int i,int v){ _OQ[i].resize(v); }

	QUEUE_ID get_OQ_t(int i){ return _OQ[i].get_t(); }
	QUEUE_ID get_OQ_s(int i){ return _OQ[i].get_s(); }
	QUEUE_INT get_OQ_v(int i,int j){ return _OQ[i].get_v(j); }

	QUEUE_ID get_OQ_tELE(int i){ return _OQELE[i].get_t(); }

	//	==== use in seq	====
	QUEUE_INT get_Tvv(int i,int j) { return _T.get_vv(i,j);}
	void set_Tvv(int i,int j,QUEUE_INT v) { _T.set_vv(i,j,v);}
	char  get_sc(int i){return _sc[i]; }
	void set_sc(int i , char v){ _sc[i]=v; }
  void inc_OQt(int e){	_OQ[e].inc_t(); }
	int get_OQ_end(int i){ return _OQ[i].get_end(); }
  void inc_OQend(int e){	_OQ[e].inc_end(); }

  void inc_OQtELE(int e){	_OQELE[e].inc_t(); }
  void inc_OQendELE(int e){	_OQELE[e].inc_end(); }
	int get_OQ_endELE(int i){ return _OQELE[i].get_end(); }

	VEC_ID get_mark(int i){ return _mark[i]; }

	QUEUE_ID get_Tvt(int i) { return _T.get_vt(i);}
	QUEUE* get_OQ(){ return  _OQ;} 
	QUEUE_INT* get_OQ_v(int i){ return _OQ[i].get_v(); }

	KGLCMSEQ_ELM * get_OQ_vELE(int i){ return _OQELE[i].get_v(); }
	KGLCMSEQ_QUE *  get_OQELE(){ return  _OQELE;} 

	VEC_ID get_row_max(void){ return _row_max;}
	QUEUE_INT get_clm_max(void){ return _clm_max;}

	void swap_Tvv(int i,int j){ _T.swap_vv(i,j); }


	//	==== use in seq	====

	//再考
	bool exist_Tw(void)	{ return _T.exist_w();} 
	bool exist_trperm(void) { return _trperm!=NULL; }
	bool exist_perm(void) { return _perm!=NULL; }
	bool exist_sc(void) { return _sc!=NULL; }
	QUEUE* getp_jump(){ return &_jump;} //再考？
	QUEUE* getp_OQ(int i){ return  &_OQ[i];} 

	 KGLCMSEQ_QUE* getp_OQELE(int i){ return  &_OQELE[i];} 


	void set_bnum(int v){return _buf.set_num(v);}  
	void set_bblock(int v){return _buf.set_block_num(v);} 
	void set_wnum(int v){return _wbuf.set_num(v);}  
	void set_wblock(int v){return _wbuf.set_block_num(v);} 
	void set_new_t(VEC_ID v){ _new_t=v;}

	void set_sc(char* v){_sc=v;}

	QUEUE dup_OQ(int i){
		return _OQ[i].dup();
	}

	void sizSort(int i,QUEUE_INT **o){
    _OQ[i].add_t ( _OQ[i].get_v() - o[i]);
    _OQ[i].set_v ( o[i]);
  }
	
	


	void reallocW(void){
		VEC_ID size =  MAX(_T.get_t(),_T.get_clms())+1;
	  realloc2 (_w , size ,EXIT);
		for(size_t i=0; i<size;i++){ _w[i] = 1;}
	}



	// normalize the vectors for inner product
	void normalize(WEIGHT *w){
	//  org ARY_FILL
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
		calloc2 (_sc, sise, return);
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

	void removeDupTrsacts(KGLCMSEQ_QUE *o);

	void dbReduction(QUEUE *occ, QUEUE_INT item){

   	find_same(occ, item);
   	merge_trsact(item);
    reduce_occ(occ);
	}

	// _occ_unit使わない
	/*
	void Mque_alloc(VEC_ID *p){
		size_t cmn_size_t = 0;

		for(VEC_ID cmm_vecid=0; cmm_vecid < _T.get_clms() ; cmm_vecid++){
			cmn_size_t += p[cmm_vecid];
		}
		_OQ = new QUEUE[_T.get_clms()+1];

		char *cmn_pnt;
		malloc2 (cmn_pnt,
		(cmn_size_t+(_T.get_clms())+2)*(sizeof(QUEUE_INT)), 
			{delete(_OQ); EXIT;}
		); 

		for(VEC_ID cmm_vecid=0; cmm_vecid < _T.get_clms() ; cmm_vecid++){
			_OQ[cmm_vecid].set_end(p[cmm_vecid]);
			_OQ[cmm_vecid].set_v((QUEUE_ID *)cmn_pnt);
			cmn_pnt +=  sizeof(QUEUE_INT) * (p[cmm_vecid]+(1));
		}
	}
	*/




	// _OQ クラス化した方がいい?
	void OccAlloc(){

    VEC_ID *p;
    p = _T.counting();

    _clm_max = p[0];
		size_t cmmsize = p[0];
		for(int cmm = 1 ; cmm < _T.get_clms() ;cmm++ ){
			cmmsize += p[cmm];
			if( _clm_max < p[cmm]){
				_clm_max=p[cmm];
			}
		}
			
		_OQ = new QUEUE[_T.get_clms()+1];

		QUEUE_INT *cmn_pnt;
		size_t pos=0;
		malloc2 (cmn_pnt, 
			(cmmsize+_T.get_clms()+2), 
			{delete(_OQ); EXIT;}
		); 
		for(VEC_ID cmmv =0; cmmv < _T.get_clms() ; cmmv++){

			_OQ[cmmv].alloc( p[cmmv],cmn_pnt+pos );

			pos += p[cmmv]+1;

		}
		_OQ[_T.get_clms()].alloc( MAX(_T.get_t(), _clm_max));


		// end is illegally set to 0, for the use in "TRSACT_find_same" 
		for(size_t i=0 ; i < _T.get_clms()+1 ; i++ ){
			_OQ[i].endClr();
		}

    // initial occurrence := all transactions
    // ARY_INIT_PERM (_OQ[_T.get_clms()].get_v(), _T.get_t());   
    _OQ[_T.get_clms()].initVprem(_T.get_t());

    free (p);
    
  }




	void Mque_allocELE(VEC_ID *p){
		size_t cmn_size_t = 0;

		for(VEC_ID cmm_vecid=0; cmm_vecid < _T.get_clms() ; cmm_vecid++){
			cmn_size_t += p[cmm_vecid];
		}

		_OQELE = new KGLCMSEQ_QUE[_T.get_clms()+1];

		char *cmn_pnt;
		malloc2 (cmn_pnt,
		(cmn_size_t+(_T.get_clms())+2)*(sizeof(KGLCMSEQ_ELM)), 
			{delete(_OQELE); EXIT;}
		); 

		for(VEC_ID cmm_vecid=0; cmm_vecid < _T.get_clms() ; cmm_vecid++){
			_OQELE[cmm_vecid].set_end(p[cmm_vecid]);
			_OQELE[cmm_vecid].set_v((KGLCMSEQ_ELM *)cmn_pnt);
			cmn_pnt += sizeof(KGLCMSEQ_ELM) * (p[cmm_vecid]+(1));
		}
	}

	
	
} ;





