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

	VEC_ID _rows_org; // _Xのかわり
	PERM *_trperm;    // _Xのかわり 仮

  char *_itemflag;       // 1 if it is include in the pattern (and 2 if included in add)
  PERM *_perm;   // permutation array for output itemset: item => original item
  WEIGHT *_item_frq;    // frequency of each item

  LONG *_sc, *_sc2;    // #itemsets classified by the sizes / frequencies

  OFILE2 _fp;    // file pointer to the output file


  WEIGHT *_set_weight;  // the frequency of each prefix of current itemset
  QUEUE **_set_occ;    // the occurrence of each prefix of current itemset
  KGLCMSEQ_QUE **_set_occELE;    // the occurrence of each prefix of current itemset

  LONG _multi_outputs0;    // #calls of ITEMSET_output_itemset or ITEMSET_solusion
  LONG _multi_iters0;   //iterations
  LONG _multi_solutions0;  // number of solutions output


	// heap for topk mining. valid if topk->h is not NULL
  AHEAP _topk; 

	QUEUE _itemset;   // current operating itemset
  QUEUE _add;       // for equisupport (hypercube decomposition)

	/* Output information about ITEMSET structure. flag&1: print frequency constraint */
	void print(int flag);

	// ================================================
	//  OUTPUT
	// ================================================
	/* output frequency, coverage */
	void output_frequency (WEIGHT frq, WEIGHT pfrq);

	/*************************************************************************/
	//  OUTPUT QUEUE
	/*************************************************************************/
	/* output itemsets with adding all combination of "add"
	   at the first call, i has to be "add->t" */
	void solution_iter (QUEUE *occ);
	void solution ( QUEUE *occ);

	/* ourput a rule */
	void output_rule( QUEUE *occ, double p1, double p2, size_t item);

	/* check all rules for a pair of itemset and item */
	void check_rule( WEIGHT *w, QUEUE *occ, size_t item);

	void output_occ( QUEUE *occ );

	// t : itemset->get_t()でOK
	bool _checkNotBound( int t , WEIGHT frq, WEIGHT pfrq){
		return _params.checkNotBound( t , frq, pfrq);
	}


	/*************************************************************************/
	//  OUTPUT KGLCMSEQ_QUE
	/*************************************************************************/
	void output_occ( KGLCMSEQ_QUE *occ );
	void check_rule ( WEIGHT *w, KGLCMSEQ_QUE *occ, size_t item);
	void solution ( KGLCMSEQ_QUE *occ );
	void solution_iter (KGLCMSEQ_QUE *occ );
	void output_rule ( KGLCMSEQ_QUE *occ, double p1, double p2, size_t item);

	public:
					
	ITEMSET(void):	  
		_progress(0),_iters(0),_solutions(0),
  	_item_max(0),_topk_frq(0),_total_weight(0),
		_ratio(0.0),_prob(0.0),_th(0.0),
		_itemflag(NULL),_perm(NULL),_item_frq(NULL),
		_sc(NULL),_sc2(NULL),
  	_set_weight(NULL),_set_occ(NULL),
		_multi_outputs0(0),_multi_iters0(0),
		_multi_solutions0(0),
		_rows_org(0),_trperm(NULL)
		{}
	
	
	~ITEMSET(void){
	 	delete [] _sc;
  	delete [] _sc2;
  	delete [] _item_frq;
  	delete [] _itemflag;
  	delete [] _perm;
  	delete [] _set_weight;
  	delete [] _set_occ;

	}

	void setParams(ItemSetParams ipara ){
		_params = ipara;
	}

	// lcmseq
	void itemINS(QUEUE_INT v){ _itemset.push_back(v);}
	void item_dec_t(){ _itemset.dec_t();}



	//lcm
	QUEUE_INT * iadd_get_v(){ return _add.get_v();}
	QUEUE_ID iadd_get_t(){ return _add.get_t();}


	// lcmseq
	void item_cal_prob(){ 
		for(QUEUE_INT *x=_itemset.begin(); x<_itemset.end(); x++){
			_prob *= _item_frq[*x];
		}
	}


	// lcm  lcmseq
	QUEUE_INT get_target(void){ return _params._target;}
	QUEUE_INT get_item_max(void){ return _item_max;}

	QUEUE_ID item_get_t(){ return _itemset.get_t();}
	QUEUE_ID get_itemset_t(){ return _itemset.get_t();}


	//lcm
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

	
	// lcm  lcmseq
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


	// lcm  lcmseq
	int get_flag(){ return _params._flag;} 
	WEIGHT get_frq_lb(){ return _params._frq_lb;} 
	int get_ub(){return _params._ub;}
	WEIGHT get_frq(){return _params._frq;}
	WEIGHT get_pfrq(){return _params._pfrq;}
	void set_frq(WEIGHT v){_params._frq=v;}
	void set_pfrq(WEIGHT v){_params._pfrq=v;}
	void set_item_frq(int i,WEIGHT v){_item_frq[i]= v;}
	void inc_iters(){ _iters++;}
	void set_total_weight(WEIGHT v) { _total_weight=v; }  
	void set_rows_org(VEC_ID v){ _rows_org = v; }
	void set_trperm(PERM *v){ _trperm = v; }
	void set_itemflag(char * v){ 
		delete [] _itemflag; 
		_itemflag = v;
	}


	// lcm
	LONG get_iters(){ return _iters;} 
	WEIGHT get_posi_lb(){ return _params._posi_lb;} 
	double get_ratio_lb(){ return _params._ratio_lb;}
	void set_itemflag(int i,char v){ _itemflag[i] = v; }
	char get_itemflag(int i){ return _itemflag[i]; }


	// lcmseq
	double get_prob(){return _prob;}
	PERM get_perm(int i){ return _perm[i];}
	PERM* get_perm(){ return _perm;}
	void set_target(QUEUE_INT v){ _params._target=v;}
	void set_prob(double v){_prob=v;}
  void set_set_weight(QUEUE_ID i, WEIGHT v){ _set_weight[i]=v;}
  void set_set_occELE(QUEUE_ID i,KGLCMSEQ_QUE * v){ _set_occELE[i]=v;}


	// lcm
	void output_itemset_( QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, QUEUE *occ);
	void output_itemset(QUEUE *occ);
	void check_all_rule (WEIGHT *w, QUEUE *occ, QUEUE *jump, WEIGHT total);

	// lcmseq
	void output_itemset_( QUEUE *itemset, WEIGHT frq, WEIGHT pfrq ,KGLCMSEQ_QUE *occ);
	void output_itemset(KGLCMSEQ_QUE *occ); 	/* output an itemset to the output file */
	void check_all_rule (WEIGHT *w, KGLCMSEQ_QUE *occ, QUEUE *jump, WEIGHT total);


	// lcm lcmseq
	void alloc(char *fname, PERM *perm, QUEUE_INT item_max, size_t item_max_org);
	void last_output ();

};

