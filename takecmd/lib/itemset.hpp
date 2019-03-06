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

//#define ITEMSET_ITERS2 4  // output #iters2
#define ITEMSET_PRE_FREQ 8  // output frequency preceding to each itemset
#define ITEMSET_FREQ 16  // output frequency following to each itemset
#define ITEMSET_ALL 32 // concat all combinations of "add" to each itemset

#define ITEMSET_TRSACT_ID 64  // output transaction ID's in occurrences
// #define ITEMSET_OUTPUT_EDGE 128  // output itemset as edge set (refer AGRAPH)
#define ITEMSET_IGNORE_BOUND 256 // ignore constraint for frequency
#define ITEMSET_RM_DUP_TRSACT 512 // remove duplicated transaction ID's
#define ITEMSET_MULTI_OCC_PRINT 1024 //print each component of occ
   // TRSACT_ID+MULTI_OCC_PRINT means print first two components of occ
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

//#define ITEMSET_RULE (ITEMSET_RULE_FRQ + ITEMSET_RULE_INFRQ + ITEMSET_RULE_RFRQ + ITEMSET_RULE_RINFRQ + ITEMSET_RFRQ + ITEMSET_RINFRQ + ITEMSET_SET_RULE)  // for check any rule is true
#define ITEMSET_RULE (ITEMSET_RULE_FRQ + ITEMSET_RULE_INFRQ + ITEMSET_RULE_RFRQ + ITEMSET_RULE_RINFRQ + ITEMSET_SET_RULE)  // for check any rule is true

#ifndef ITEMSET_INTERVAL
#define ITEMSET_INTERVAL 500000
#endif



#include"stdlib2.hpp"
#include"file2.hpp"
#include"queue.hpp"
#define AHEAP_KEY_WEIGHT
#include"aheap.hpp"


class ITEMSET{

	//  I->topk = INIT_AHEAP;
	//	_itemset = I->add = INIT_QUEUE;
  //I->minh = I->maxh = INIT_IHEAP;
	
	int _flag, _flag2; // <=_flag2 (LAMPの時)使われてないようなきが 
	int _progress;
  LONG _iters; //_iters2, _iters3;  //iterations
  LONG _solutions, _solutions2;  // number of solutions output
  LONG _max_solutions; // maximum solutions to be output
	LONG _outputs, _outputs2;
  QUEUE_INT _item_max, _item_max_org;  // (original) maximum item
  int _ub, _lb;   // upper/lower bounds for the itemset size
  int _len_ub, _len_lb;   // upper/lower bounds for the length of the pattern
  int _gap_ub, _gap_lb;   // upper/lower bounds for the gaps in the pattern

  LONG  _itemtopk_item, _itemtopk_item2,_itemtopk_end; // the size of itemtopk heaps; specify topkheap to be used/the size of each in the initiaization
  LONG _topk_k, _topk_frq;   // counter for topk bucket and #remainings

  WEIGHT _frq, _pfrq, _frq_ub, _frq_lb;  // upper/lower bounds for the frequency
  WEIGHT _total_weight;  // total weight of the input database
  double _ratio, _prob, _th;   // confidence and independent probability of the current pattern
  WEIGHT _rposi_lb, _rposi_ub, _posi_lb, _posi_ub, _nega_lb, _nega_ub;  // upper/lower bounds for the sum of positive/negative weights
  WEIGHT _setrule_lb;  // frequency lower bound for set rule
  int _dir;  // direction flag for AGRAPH & SGRAPH
  QUEUE_INT _target;  // target item for rule mining
  double _ratio_ub, _ratio_lb, _prob_ub, _prob_lb;   // upper/lower bounds for confidence and independent probability
  char _separator; // separator of items output
  int _digits;  // #decimals to be output, for non-natural numbers

  char _topk_sign, _itemtopk_sign;  // determine min/max (1, -1) for topk/itemtopk heaps 
  LONG *_patn;  // cardinarity of each patten group


  char *_itemflag;       // 1 if it is include in the pattern (and 2 if included in add)
  PERM *_perm;   // permutation array for output itemset: item => original item
  WEIGHT *_item_frq;    // frequency of each item
  LONG *_sc, *_sc2;    // #itemsets classified by the sizes / frequencies
  void *_X;  // pointer to the original data
  FILE *_fp;    // file pointer to the output file
  QUEUE_INT **_itemtopk_ary;  // topk solutions for each item
  WEIGHT *_set_weight;  // the frequency of each prefix of current itemset
  QUEUE **_set_occ;    // the occurrence of each prefix of current itemset


  LONG *_multi_outputs, *_multi_outputs2;    // #calls of ITEMSET_output_itemset or ITEMSET_solusion
  LONG *_multi_iters, *_multi_iters2, *_multi_iters3;  //iterations
  LONG *_multi_solutions, *_multi_solutions2;  // number of solutions output
  FILE2 *_multi_fp;  // output file2 pointer for multi-core mode
  int _multi_core;  // number of processors

  AHEAP *_itemtopk;  // heap for topk mining. valid if topk->h is not NULL
  //AHEAP **_itemtopk;  // heap for topk mining. valid if topk->h is not NULL


  AHEAP _topk; 

  IHEAP _minh, _maxh;  // for 2D LAMP mode

	//これはあとで
	QUEUE _itemset;   // current operating itemset
  QUEUE _add;       // for equisupport (hypercube decomposition)

#ifdef MULTI_CORE
  pthread_spinlock_t _lock_counter;   // couneter locker for jump counter
  pthread_spinlock_t _lock_sc;   // couneter locker for score counter
  pthread_spinlock_t _lock_output;   // couneter locker for #output 
#endif


	
	/* Output information about ITEMSET structure. flag&1: print frequency constraint */
	void print (int flag);

	/* topk.end>0 => initialize heap for topk mining */
	/* all pointers will be set to 0, but not for */
	/* if topK mining, set topk.end to "K" */
	//void init ();
	void end ();



	/* output frequency, coverage */
	void output_frequency (WEIGHT frq, WEIGHT pfrq, int core_id);


	/* output itemsets with adding all combination of "add"
	   at the first call, i has to be "add->t" */
	void solution_iter (QUEUE *occ, int core_id);
	void solution ( QUEUE *occ, int core_id);

	/*************************************************************************/
	/* ourput a rule */
	/*************************************************************************/
	void output_rule ( QUEUE *occ, double p1, double p2, size_t item, int core_id);

	/*************************************************************************/
	/* check all rules for a pair of itemset and item */
	/*************************************************************************/
	void check_rule ( WEIGHT *w, QUEUE *occ, size_t item, int core_id);


	void lamp (LONG s);
	void lamp2 (LONG s);
	void output_occ( QUEUE *occ, int core_id);



	public:
	// ,_iters2(0),_iters3(0)
	ITEMSET(void):	  
		_flag(0),_flag2(0),_progress(0),_iters(0),_solutions(0),_solutions2(0),
  	_max_solutions(0),_outputs(0),_outputs2(0),_item_max(0),_item_max_org(0),
		_ub(INTHUGE),_lb(0),_len_ub(INTHUGE),_len_lb(0),_gap_ub(INTHUGE),_gap_lb(0),
		_itemtopk_item(0),_itemtopk_item2(0),_itemtopk_end(0),
		_topk_k(0),_topk_frq(0),
		_frq(0),_pfrq(0),_frq_ub(WEIGHTHUGE),_frq_lb(-WEIGHTHUGE),_total_weight(0),
		_ratio(0.0),_prob(0.0),_th(0.0),
		_rposi_lb(0),_rposi_ub(1),_posi_lb(-WEIGHTHUGE),_posi_ub(WEIGHTHUGE),
		_nega_lb(-WEIGHTHUGE),_nega_ub(WEIGHTHUGE),_setrule_lb(-WEIGHTHUGE),
		_dir(0),_target(INTHUGE),
		_ratio_ub(1),_ratio_lb(0),_prob_ub(1),_prob_lb(0),
		_separator(' '),_digits(4),
		_topk_sign(1) ,_itemtopk_sign(1),  // initialization ; max topk
		_itemflag(NULL),_perm(NULL),_item_frq(NULL),
		_sc(NULL),_sc2(NULL),_X(NULL),_fp(NULL),
		_itemtopk_ary(NULL),
  	_set_weight(NULL),_set_occ(NULL),_patn(NULL),
		_multi_outputs(NULL),_multi_outputs2(NULL),
		_multi_iters(NULL),_multi_iters2(NULL),_multi_iters3(NULL),
		_multi_solutions(NULL),_multi_solutions2(NULL),_multi_fp(NULL),
		_multi_core(0),_itemtopk(NULL){}
	
		// SSPC
		void setParams(
			int iFlag, WEIGHT frq_lb,int len_ub,int len_lb,
			LONG topk_k,LONG itemtopk_item,LONG itemtopk_item2,
			LONG itemtopk_end , int multi_core, LONG max_solutions,char separator
		){
			_flag   = iFlag;
			_frq_lb = frq_lb;
			_len_ub = len_ub;
			_len_lb = len_lb;
			_topk_k = topk_k;
			_itemtopk_item = itemtopk_item;
			_itemtopk_item2 = itemtopk_item2;
			_itemtopk_end = itemtopk_end;
			_multi_core = multi_core;
			_max_solutions = max_solutions;
			_separator = separator;

		}
		// LCM
		void setParams(
			int flag , WEIGHT frq_lb , WEIGHT frq_ub , int lb , int  ub , QUEUE_INT target ,
			double ratio_lb , double ratio_ub , double prob_lb , double prob_ub,
			WEIGHT rposi_lb , WEIGHT rposi_ub , WEIGHT nega_lb , WEIGHT nega_ub,
			WEIGHT posi_lb , WEIGHT posi_ub, LONG topk_k , LONG max_solutions ,
			char separator , LONG digits
		){
			_flag = flag;
			_frq_lb = frq_lb;
			_frq_ub = frq_ub;
			_lb = lb;
			_ub = ub;
			_target = target;
			_ratio_lb = ratio_lb;
			_ratio_ub = ratio_ub;
			_prob_lb = prob_lb;
			_prob_ub = prob_ub;
			_rposi_lb = rposi_lb;
			_rposi_ub = rposi_ub;
			_nega_lb = nega_lb;
			_nega_ub = nega_ub;
			_posi_lb = posi_lb;
			_posi_ub = posi_ub;
			_topk_k = topk_k;
			_max_solutions = max_solutions;
			_separator = separator;
			_digits = digits;	
	}

		// LCMSEQ
		void setParams(
			int flag , WEIGHT frq_lb , WEIGHT frq_ub , int lb , int  ub , QUEUE_INT target ,
			double ratio_lb , double ratio_ub , double prob_lb , double prob_ub,
			WEIGHT rposi_lb , WEIGHT rposi_ub , WEIGHT nega_lb , WEIGHT nega_ub,
			WEIGHT posi_lb , WEIGHT posi_ub, LONG topk_k , LONG max_solutions ,
			char separator ,  WEIGHT frq,  WEIGHT pfrq, int len_ub
		){
			_flag = flag;
			_frq_lb = frq_lb;
			_frq_ub = frq_ub;
			_lb = lb;
			_ub = ub;
			_target = target;
			_ratio_lb = ratio_lb;
			_ratio_ub = ratio_ub;
			_prob_lb = prob_lb;
			_prob_ub = prob_ub;
			_rposi_lb = rposi_lb;
			_rposi_ub = rposi_ub;
			_nega_lb = nega_lb;
			_nega_ub = nega_ub;
			_posi_lb = posi_lb;
			_posi_ub = posi_ub;
			_topk_k = topk_k;
			_max_solutions = max_solutions;
			_separator = separator;
			_pfrq = pfrq;
			_frq = frq;
			_len_ub = len_ub;
		
		}





#ifdef MULTI_CORE
	pthread_spinlock_t get_lock_counter(){return _lock_counter;}
#else
	void get_lock_counter(){return; }
#endif

	void itemINS(QUEUE_INT v){ _itemset.push_back(v);}
	void iaddINS(QUEUE_INT v){ _add.push_back(v);}

	void item_dec_t(){ _itemset.dec_t();}

	QUEUE_INT * item_get_v(){ return _itemset.get_v();}
	QUEUE_INT   iadd_get_v(int i){ return _add.get_v(i);}
	QUEUE_INT * iadd_get_v(){ return _add.get_v();}

	void item_set_t(QUEUE_ID t){ _itemset.set_t(t);}
	void iadd_set_t(QUEUE_ID t){ _add.set_t(t);}

	void item_cal_prob(){ 
		QUEUE_INT *x;
		//MQUE_FLOOP_CLS (_itemset, x) _prob *= _item_frq[*x];
		// MQUE_FLOOP_CLS(V,z)    
		// for((z)=(V).get_v();(z)<(V).get_v()+(V).get_t() ; (z)++)
		for(x=_itemset.get_v();(x)<_itemset.get_v()+_itemset.get_t(); x++){
			_prob *= _item_frq[*x];
		}
	}

	void iadd_cal_prob(){ 
		QUEUE_INT *x;
		//MQUE_FLOOP_CLS (_add, x) _prob *= _item_frq[*x];
		for(x=_add.get_v();(x)<_add.get_v()+_add.get_t(); x++){
			_prob *= _item_frq[*x];
		}

	}


	QUEUE_ID iadd_get_t(){ return _add.get_t();}
	QUEUE_ID item_get_t(){ return _itemset.get_t();}


	QUEUE_INT item_del_item(){ 
  	QUEUE_INT item = _itemset.pop();
	  _itemflag[item]=0;
	  return item;
	}

	QUEUE_INT iadd_del_item(){ 
  	QUEUE_INT item = _add.pop();
	  _itemflag[item]=0;
	  return item;
	}


	QUEUE_INT get_target(void){ return _target;}
	QUEUE_INT get_item_max(void){ return _item_max;}
	LONG get_itemtopk_end(void){ return _itemtopk_end;}
	LONG get_itemtopk_item(void){ return _itemtopk_item;}

	AHEAP_ID get_topk_end(){ return _topk.end();}

	int get_flag(){ return _flag;} 
	int get_flag2(){ return _flag2;} 
	LONG get_iters(){ return _iters;} 

	int get_multi_core(){ return _multi_core;} 
	WEIGHT get_frq_lb(){ return _frq_lb;} 
	WEIGHT get_frq_ub(){ return _frq_ub;} 
	WEIGHT get_rposi_lb(){ return _rposi_lb;} 
	WEIGHT get_rposi_ub(){ return _rposi_ub;} 
	WEIGHT get_posi_lb(){ return _posi_lb;} 
	WEIGHT get_posi_ub(){ return _posi_ub;} 

	int get_len_lb(){ return _len_lb;} 
	int get_len_ub(){ return _len_ub;} 
	QUEUE* getp_itemset() { return &_itemset;}
	char get_separator(){ return _separator;} 
	LONG get_solutions(){ return _solutions;} 
	LONG get_sc(int i){ return _sc[i];}
	int get_ub(){return _ub;}

	double get_prob(){return _prob;}
	double get_prob_lb(){ return _prob_lb;} 
	double get_prob_ub(){ return _prob_ub;} 
	double get_ratio_lb(){ return _ratio_lb;}
	double get_ratio_ub(){ return _ratio_ub;}
	WEIGHT get_frq(){return _frq;}
	WEIGHT get_pfrq(){return _pfrq;}

	WEIGHT get_item_frq(int i){return _item_frq[i];}
	LONG get_topk_k(){ return _topk_k ;}

	int get_topk_base(){ return _topk.base();}

	int get_gap_ub(){return _gap_ub;}

	void set_itemset_t(QUEUE_ID v){ _itemset.set_t(v) ;}
	void set_itemset_v(int i,QUEUE_INT v){ _itemset.set_v(i,v);}
	QUEUE_ID get_itemset_t(){ return _itemset.get_t();}


	PERM get_perm(int i){ return _perm[i];}
	PERM* get_perm(){ return _perm;}
	
	FILE2* getp_multi_fp(int i){ return &_multi_fp[i]; }
 
	void set_target(QUEUE_INT v){ _target=v;}
	void set_multi_core( int v) { _multi_core=v;}
	void set_topk_k( LONG v){ _topk_k = v;}
	void set_itemtopk_item( LONG v){_itemtopk_item=v;}
	void set_itemtopk_item2( LONG v){_itemtopk_item2=v;}
	void set_itemtopk_end( LONG v){_itemtopk_end=v;}
	void set_len_lb(int v){ _len_lb =v; }
	void set_len_ub(int v){ _len_ub =v; }
	void set_max_solutions( LONG v){ _max_solutions=v;}
	void set_separator(char v){ _separator = v;}
	void set_frq_lb(WEIGHT v){ _frq_lb= v;}
	void set_frq_ub(WEIGHT v){ _frq_ub= v;}
	void set_prob_lb(double v){ _prob_lb= v;}
	void set_prob_ub(double v){ _prob_ub= v;}
	void set_ratio_lb(double v){ _prob_lb= v;}
	void set_ratio_ub(double v){ _prob_ub= v;}

	void set_rposi_lb(WEIGHT v){ _rposi_lb= v;}
	void set_rposi_ub(WEIGHT v){ _rposi_ub= v;}

	void set_gap_ub(int v){_gap_ub=v;}
	void set_setrule_lb(WEIGHT v){_setrule_lb=v;}

	void set_posi_lb(WEIGHT v){ _posi_lb= v;}
	void set_posi_ub(WEIGHT v){ _posi_ub= v;}
	void set_nega_lb(WEIGHT v){ _nega_lb= v;}
	void set_nega_ub(WEIGHT v){ _nega_ub= v;}

	void set_digits(int v){ _digits= v;}

	void set_perm(PERM* v){_perm =v;}

	void set_ub(int v){_ub=v;}
	void set_lb(int v){_lb=v;}
	void set_th(double v){_th=v;}
	void set_prob(double v){_prob=v;}

	void set_frq(WEIGHT v){_frq=v;}
	void set_pfrq(WEIGHT v){_pfrq=v;}

	void set_topk_base(int v){ _topk.base(v);}
	void set_topk_end(int v){ _topk.end(v);}

	void set_item_frq(int i,WEIGHT v){_item_frq[i]= v;}

	void set_maximum_frequency( WEIGHT v){
    ENMIN (_frq_ub, v); // set  maximum frequency
	}

	bool exist_perm(void){ return _perm!=NULL;}

  void set_set_weight(QUEUE_ID i, WEIGHT v){ _set_weight[i]=v;}
  void set_set_occ(QUEUE_ID i,QUEUE * v){ _set_occ[i]=v;}


	void union_flag(int flag){ _flag|=flag;}
	void union_flag2(int flag){ _flag2|=flag;}
	void sub_flag(int flag){ _flag-=flag;}

	void add_sc(int i,LONG v){ _sc[i]+=v;}
	void inc_iters(){ _iters++;}

	// void bitrm(int){ BITRM (_flag, ITEMSET_PRE_FREQ);}


	void set_total_weight(WEIGHT v) { _total_weight=v; }  

	void set_itemflag(char * v){ free2 (_itemflag); _itemflag = v;}

	void set_itemflag(int i,char v){_itemflag[i] = v;}
	char get_itemflag(int i){return _itemflag[i];}

	void alloc(char *fname, PERM *perm, QUEUE_INT item_max, size_t item_max_org);

	/* output an itemset to the output file */
	void output_itemset  ( QUEUE *occ, int core_id);
	void output_itemset_ ( QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, QUEUE *occ, QUEUE_INT itemtopk_item, QUEUE_INT itemtopk_item2, int core_id);

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

	void close(){
		int i;
	  if ( _multi_fp )
      FLOOP (i, 0, MAX(_multi_core,1)) _multi_fp[i].closew();
	
	}

};





