/*  itemset search input/output common routines
            25/Nov/2007   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

/* routines for itemset mining */

/* parameters for ITEMSET.flag */
#pragma once
#define WEIGHT_DOUBLE

#define ITEMSET_PRE_FREQ 8  // output frequency preceding to each itemset
#define ITEMSET_FREQ 16  // output frequency following to each itemset
#define ITEMSET_ALL 32 // concat all combinations of "add" to each itemset

#define ITEMSET_TRSACT_ID 64  // output transaction ID's in occurrences

#define ITEMSET_IGNORE_BOUND 256 // ignore constraint for frequency
#define ITEMSET_RM_DUP_TRSACT 512 // remove duplicated transaction ID's
#define ITEMSET_MULTI_OCC_PRINT 1024 //print each component of occ

#define ITEMSET_NOT_ITEMSET  2048 // do not print itemset to the output file
#define ITEMSET_RULE_SUPP  4096 // output confidence and item frquency by abusolute value
#define ITEMSET_OUTPUT_POSINEGA  8192 // output negative/positive frequencies
#define ITEMSET_MULTI_OUTPUT 16384 // for multi-core mode
#define ITEMSET_USE_ORG 32768 // use item_max_org to the size of use
#define ITEMSET_ITEMFRQ 65536 // allocate item_frq
#define ITEMSET_ADD 131072    // allocate add

#define ITEMSET_RULE_FRQ 262144
#define ITEMSET_RULE_INFRQ 524288
#define ITEMSET_RULE_RFRQ 1048576
#define ITEMSET_RULE_RINFRQ 2097152
#define ITEMSET_RFRQ 4194304
#define ITEMSET_RINFRQ 8388608
#define ITEMSET_POSI_RATIO 16777216
#define ITEMSET_SET_RULE 134217728

#define ITEMSET_APPEND 268435456   // append the output to the fiile
#define ITEMSET_RULE_ADD 536870912   // append items in add to the solution, for rule output
#define ITEMSET_SC2 1073741824   // count #items classified by frequencies


#define ITEMSET_LAMP 256   // LAMP mode
#define ITEMSET_LAMP2 512   // 2D LAMP mode

#define ITEMSET_RULE (ITEMSET_RULE_FRQ + ITEMSET_RULE_INFRQ + ITEMSET_RULE_RFRQ + ITEMSET_RULE_RINFRQ + ITEMSET_SET_RULE)  // for check any rule is true

#ifndef ITEMSET_INTERVAL
#define ITEMSET_INTERVAL 500000
#endif



#include"stdlib2.hpp"
#include"file2.hpp"
#include"queue.hpp"
#define AHEAP_KEY_WEIGHT
#include"aheap.hpp"



struct ItemSetParams {
	
	// ALL
	int _flag;
	LONG _max_solutions;
	char _separator;
	
	// LCM LCMSEQ MACE
	int _lb , _ub;
	
	// LCM LCMSEQ
	WEIGHT _nega_lb ,_nega_ub;
	WEIGHT _posi_lb ,_posi_ub;
	double _prob_lb ,_prob_ub;
	double _ratio_lb ,_ratio_ub;

	// LCM LCMSEQ SSPC
  WEIGHT _frq_lb;
	// LCM	LCMSEQ
  WEIGHT _frq_ub;

	// LCMSEQ
	WEIGHT _frq, _pfrq;
	WEIGHT _setrule_lb;


	// LCM LCMSEQ SSPC
	LONG _topk_k;


	int _multi_core;

	//LCM LCMSEQ
	QUEUE_INT _target;

	// LCM
	int _digits;

	ItemSetParams(void):
		_flag(0),_max_solutions(0),	
		_separator(' '),
		_ub(INTHUGE),_lb(0),
		_nega_lb(-WEIGHTHUGE),_nega_ub(WEIGHTHUGE),
		_posi_lb(-WEIGHTHUGE),_posi_ub(WEIGHTHUGE),
		_prob_ub(1),_prob_lb(0),
		_ratio_ub(1),_ratio_lb(0),
		_multi_core(1),		
		_topk_k(0),
		_frq_ub(WEIGHTHUGE),_frq_lb(-WEIGHTHUGE),	
		_target(INTHUGE),
		_digits(4),
		_frq(0),_pfrq(0),
		_setrule_lb(-WEIGHTHUGE){}


	bool checkNotBound( int t , WEIGHT frq, WEIGHT pfrq){
	
	  if ( t < _lb || t > _ub ) return true ;

  	if ( (_flag&ITEMSET_IGNORE_BOUND)==0 ) {
	  	if ( frq < _frq_lb || frq > _frq_ub ) return true;
	  	if ( pfrq < _posi_lb || pfrq > _posi_ub || 
	  			(frq - _pfrq) > _nega_ub || (frq - _pfrq) < _nega_lb ) return true;
	  			
	  }
		return false;
	}


};


class ITEMSET{

	ItemSetParams _params;

	int _progress;
	
  LONG _iters;         // iterations
  LONG _solutions;     // number of solutions output

  QUEUE_INT _item_max;  // (original) maximum item

	// counter for topk bucket and #remainings
  LONG _topk_frq;   

	// total weight of the input database
  WEIGHT _total_weight;  

	 // confidence and independent probability of the current pattern
  double _ratio, _prob, _th;  


	// determine min/max (1, -1) for topk/itemtopk heaps 
  char _topk_sign, _itemtopk_sign;  

	VEC_ID _rows_org; // _Xのかわり
	PERM *_trperm;    // _Xのかわり 仮

  char *_itemflag;       // 1 if it is include in the pattern (and 2 if included in add)
  PERM *_perm;   // permutation array for output itemset: item => original item
  WEIGHT *_item_frq;    // frequency of each item

  LONG *_sc, *_sc2;    // #itemsets classified by the sizes / frequencies

  OFILE2 _fp;    // file pointer to the output file

  QUEUE_INT **_itemtopk_ary;  // topk solutions for each item

  WEIGHT *_set_weight;  // the frequency of each prefix of current itemset
  QUEUE **_set_occ;    // the occurrence of each prefix of current itemset
  KGLCMSEQ_QUE **_set_occELE;    // the occurrence of each prefix of current itemset

  OFILE2 *_multi_fp;  // output file2 pointer for multi-core mode
	// counter
  LONG *_multi_outputs;    // #calls of ITEMSET_output_itemset or ITEMSET_solusion
  LONG *_multi_iters;   //iterations
  LONG *_multi_solutions;  // number of solutions output


	// heap for topk mining. valid if topk->h is not NULL
  AHEAPARY _itemtopk;   

  AHEAP _topk; 

	//これはあとで
	QUEUE _itemset;   // current operating itemset
  QUEUE _add;       // for equisupport (hypercube decomposition)

#ifdef MULTI_CORE
  pthread_spinlock_t _lock_counter;   // couneter locker for jump counter
  pthread_spinlock_t _lock_sc;   // couneter locker for score counter
  pthread_spinlock_t _lock_output;   // couneter locker for #output 
#endif

	/* Output information about ITEMSET structure. flag&1: print frequency constraint */
	void print(int flag);

	// ================================================
	//  OUTPUT
	// ================================================
	/* output frequency, coverage */
	void output_frequency (WEIGHT frq, WEIGHT pfrq, int core_id);

	/*************************************************************************/
	//  OUTPUT QUEUE
	/*************************************************************************/
	/* output itemsets with adding all combination of "add"
	   at the first call, i has to be "add->t" */
	void solution_iter (QUEUE *occ, int core_id);
	void solution ( QUEUE *occ, int core_id);

	/* ourput a rule */
	void output_rule( QUEUE *occ, double p1, double p2, size_t item, int core_id);

	/* check all rules for a pair of itemset and item */
	void check_rule( WEIGHT *w, QUEUE *occ, size_t item, int core_id);

	void output_occ( QUEUE *occ, int core_id);

	// t : itemset->get_t()でOK
	bool _checkNotBound( int t , WEIGHT frq, WEIGHT pfrq){
		return _params.checkNotBound( t , frq, pfrq);
	}


	/*************************************************************************/
	//  OUTPUT KGLCMSEQ_QUE
	/*************************************************************************/
	void output_occ( KGLCMSEQ_QUE *occ, int core_id);
	void check_rule ( WEIGHT *w, KGLCMSEQ_QUE *occ, size_t item, int core_id);
	void solution ( KGLCMSEQ_QUE *occ, int core_id);
	void solution_iter (KGLCMSEQ_QUE *occ, int core_id);
	void output_rule ( KGLCMSEQ_QUE *occ, double p1, double p2, size_t item, int core_id);

	public:
					
	ITEMSET(void):	  
		_progress(0),_iters(0),_solutions(0),
  	_item_max(0),_topk_frq(0),_total_weight(0),
		_ratio(0.0),_prob(0.0),_th(0.0),
		_topk_sign(1) ,_itemtopk_sign(1),  // initialization ; max topk << 常に1？
		_itemflag(NULL),_perm(NULL),_item_frq(NULL),
		_sc(NULL),_sc2(NULL),
  	_set_weight(NULL),_set_occ(NULL),
		_multi_outputs(NULL),_multi_iters(NULL),
		_multi_solutions(NULL),_multi_fp(NULL),
		_itemtopk_ary(NULL),
		_rows_org(0),_trperm(NULL)
		{}
	
	
	~ITEMSET(void){

		if ( _itemtopk_ary ){ // _itemtopk_aryを_itemtopk.に入れる?
			for(LONG i=0;i< _itemtopk.size() ; i++){
				delete [] _itemtopk_ary[i];
			}	  
	  	delete [] _itemtopk_ary; 
	 	}


	 	delete [] _sc;
  	delete [] _sc2;
  	delete [] _item_frq;
  	delete [] _itemflag;
  	delete [] _perm;
  	delete [] _set_weight;
  	delete [] _set_occ;
  
	  if ( _multi_fp ){
			for(size_t i=0;i<MAX(_params._multi_core,1);i++){
				_multi_fp[i].clear();
			}
		  delete[] _multi_fp;
	  	_multi_fp  = NULL;
	  }

  	delete [] _multi_iters ;

		#ifdef MULTI_CORE
  	if ( _params._multi_core>0 ){
    	pthread_spin_destroy(&_lock_counter);
    	pthread_spin_destroy(&_lock_sc);
    	pthread_spin_destroy(&_lock_output);
	  }
		#endif

	}

	void setParams(ItemSetParams ipara ){
		_params = ipara;
	}

		
#ifdef MULTI_CORE
	pthread_spinlock_t get_lock_counter(){return _lock_counter;}
#else
	void get_lock_counter(){return; }
#endif

	void itemINS(QUEUE_INT v){ _itemset.push_back(v);}
	void item_dec_t(){ _itemset.dec_t();}

	QUEUE_INT * item_get_v(){ return _itemset.get_v();}
	QUEUE_INT   iadd_get_v(int i){ return _add.get_v(i);}
	QUEUE_INT * iadd_get_v(){ return _add.get_v();}

	void item_set_t(QUEUE_ID t){ _itemset.set_t(t);}

	void item_cal_prob(){ 
		for(QUEUE_INT *x=_itemset.begin(); x<_itemset.end(); x++){
			_prob *= _item_frq[*x];
		}
	}

	void iadd_cal_prob(){ 
		for(QUEUE_INT *x=_add.begin();(x)<_add.end(); x++){
			_prob *= _item_frq[*x];
		}
	}

	QUEUE_ID iadd_get_t(){ return _add.get_t();}
	QUEUE_ID item_get_t(){ return _itemset.get_t();}

	void addCurrent(QUEUE_INT v){
		_itemset.push_back(v);
	  _itemflag[v]=1;	
	}
	void addEquiSupport(QUEUE_INT v){
		_add.push_back(v);
	  _itemflag[v]=1;	
	}

	QUEUE_INT delCurrent(void){ 
  	QUEUE_INT item = _itemset.pop_back();
	  _itemflag[item]=0;
	  return item;
	}

	QUEUE_INT delEquiSupport(void){ 
  	QUEUE_INT item = _add.pop_back();
	  _itemflag[item]=0;
	  return item;
	}

	QUEUE_INT get_target(void){ return _params._target;}
	
	void adjustTarget(QUEUE_ID siz){
		if( _params._target < siz && _perm ){
     	for(size_t j=0; j< _item_max ;j++){
      	if ( _params._target == _perm[j] ){ 
      		_params._target = j ; 
      		break; 
      	} 
      }
		}
		return;
	}
	
	
	QUEUE_INT get_item_max(void){ return _item_max;}

	AHEAP_ID get_topk_end(){ return _topk.end();}

	int get_flag(){ return _params._flag;} 
	LONG get_iters(){ return _iters;} 

	int get_multi_core(){ return _params._multi_core;} 
	WEIGHT get_frq_lb(){ return _params._frq_lb;} 
	WEIGHT get_frq_ub(){ return _params._frq_ub;} 
	WEIGHT get_posi_lb(){ return _params._posi_lb;} 
	WEIGHT get_posi_ub(){ return _params._posi_ub;} 

	QUEUE* getp_itemset() { return &_itemset;}
	char get_separator(){ return _params._separator;} 
	LONG get_solutions(){ return _solutions;} 
	LONG get_sc(int i){ return _sc[i];}
	int get_ub(){return _params._ub;}

	double get_prob(){return _prob;}
	double get_prob_lb(){ return _params._prob_lb;} 
	double get_prob_ub(){ return _params._prob_ub;} 
	double get_ratio_lb(){ return _params._ratio_lb;}
	double get_ratio_ub(){ return _params._ratio_ub;}
	WEIGHT get_frq(){return _params._frq;}
	WEIGHT get_pfrq(){return _params._pfrq;}

	WEIGHT get_item_frq(int i){return _item_frq[i];}
	LONG get_topk_k(){ return _params._topk_k ;}

	int get_topk_base(){ return _topk.base();}


	void set_itemset_t(QUEUE_ID v){ _itemset.set_t(v) ;}
	void set_itemset_v(int i,QUEUE_INT v){ _itemset.set_v(i,v);}

	QUEUE_ID get_itemset_t(){ return _itemset.get_t();}


	PERM get_perm(int i){ return _perm[i];}
	PERM* get_perm(){ return _perm;}

	void print_int(int coreID,LONG n, char c){
		_multi_fp[coreID].print_int( n, c);
	}
	void putc(int coreID, char c){
		_multi_fp[coreID].putch(c);
	}
	void flush(int coreID){
		_multi_fp[coreID].flush();
	}

	void set_target(QUEUE_INT v){ _params._target=v;}
	void set_perm(PERM* v){_perm =v;}

	void set_ub(int v){_params._ub=v;}
	void set_lb(int v){_params._lb=v;}
	void set_th(double v){_th=v;}
	void set_prob(double v){_prob=v;}

	void set_frq(WEIGHT v){_params._frq=v;}
	void set_pfrq(WEIGHT v){_params._pfrq=v;}

	void set_topk_base(int v){ _topk.base(v);}
	void set_topk_end(int v){ _topk.end(v);}

	void set_item_frq(int i,WEIGHT v){_item_frq[i]= v;}


	bool exist_perm(void){ return _perm!=NULL;}

  void set_set_weight(QUEUE_ID i, WEIGHT v){ _set_weight[i]=v;}
  void set_set_occ(QUEUE_ID i,QUEUE * v){ _set_occ[i]=v;}
  void set_set_occELE(QUEUE_ID i,KGLCMSEQ_QUE * v){ _set_occELE[i]=v;}

	void add_sc(int i,LONG v){ _sc[i]+=v;}
	void inc_iters(){ _iters++;}

	void set_total_weight(WEIGHT v) { _total_weight=v; }  
	void set_rows_org(VEC_ID v){ _rows_org = v; }
	void set_trperm(PERM *v){ _trperm = v; }

	void set_itemflag(char * v){ 
		delete [] _itemflag; //free
		_itemflag = v;
	}

	void set_itemflag(int i,char v){ _itemflag[i] = v; }
	char get_itemflag(int i){ return _itemflag[i]; }


	void alloc(char *fname, PERM *perm, QUEUE_INT item_max, size_t item_max_org);

	//sspc
	void alloc(char *fname, PERM *perm, QUEUE_INT item_max,size_t item_max_org,
						LONG _itk_end ,LONG _itk_item, LONG _itk_item2);


	void QueMemCopy(QUEUE &from){
		_itemset.MemCopy(from);
	}

	// for lcm
	void output_itemset_( 
		QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, QUEUE *occ,  
		int core_id);

	// for lcm seq
	void output_itemset_(
		QUEUE *itemset, WEIGHT frq, WEIGHT pfrq ,KGLCMSEQ_QUE *occ, 
		int core_id);

	/* output an itemset to the output file for sspc */
	void output_itemset_(
		QUEUE *itemset, WEIGHT frq, WEIGHT pfrq,
		int core_id);

	/* output an itemset to the output file for sspc -k */
	void output_itemset_k(
		QUEUE *itemset, WEIGHT frq, WEIGHT pfrq,
		QUEUE_INT itemtopk_item, QUEUE_INT itemtopk_item2, 
		int core_id);


	// for mace
	void output_itemset(int core_id);

	/* output an itemset to the output file */
	void output_itemset(QUEUE *occ, int core_id);
	void output_itemset(KGLCMSEQ_QUE *occ, int core_id);


	/*******************************************************************/
	/* output at the termination of the algorithm */
	/* print #of itemsets of size k, for each k */
	/*******************************************************************/
	void last_output ();

	/* sum the counters computed by each thread */
	void merge_counters ();

	/*************************************************************************/
	/* check all rules for an itemset and all items */
	/*************************************************************************/
	void check_all_rule (WEIGHT *w, QUEUE *occ, QUEUE *jump, WEIGHT total, int core_id);

	void check_all_rule (WEIGHT *w, KGLCMSEQ_QUE *occ, QUEUE *jump, WEIGHT total, int core_id);

	void close(){
	
	  // fpは共有してるのでcloseはしない
	  if( _multi_fp ){
	  	for(int i = 0 ; i < MAX(_params._multi_core,1) ; i++){
	  		_multi_fp[i].clearbuf();
	  	}
		}
	}

};




