/* Linear time Closed itemset Miner for Frequent Itemset Mining problems */
/* 2004/4/10 Takeaki Uno,   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about LCM for the users.
   For the commercial use, please make a contact to Takeaki Uno. */

#pragma once


#define WEIGHT_DOUBLE
#define TRSACT_DEFAULT_WEIGHT 1

// use the complement graph of the constraint graph
#define LCM_UNCONST 16777216   

// an item will be dealt as "equisupp" when "positive"-frequency 
// is equal to the positive-frequency of the current itemset 
#define LCM_POSI_EQUISUPP 33554432   

#define ERROR_RET 

#include"trsact.hpp"
#include"sgraph.hpp"
#include"problem.hpp"
#include "itemset.hpp" 


class KGLCM{

	ITEMSET _II;
	TRSACT _TT;
	SGRAPH _SG;


	ItemSetParams _ipara;
	TrsactParams _tpara;

	bool _progressFlag;
	int _problem;

	double _th;
	char* _outperm_fname;
	char* _output_fname;
	char* _t_pfname;

	double _rposi_ub,_rposi_lb;

	bool _pRatioFlg;


	char *_pfname;


	// SG
	char *_sgfname;
	//int _sgFlag;

	WEIGHT * _occ_w,*_occ_w2,*_occ_pw,*_occ_pw2;

	QUEUE _itemcand;
	QUEUE _oo;


	bool _clmsFlag; //orignal _clms

	void help();


	/* allocate arrays and structures */
	void preALOCC(){

		QUEUE_ID siz = _TT.get_clms();
		PERM *perm   = _TT.get_perm();

	  PERM *p=NULL;
  	int j;

		_occ_w = new WEIGHT[siz+2]();//calloc2 

		if ( _TT.incNega()){
			_occ_pw = new WEIGHT[siz+2]();//calloc2
		}else{
			_occ_pw = _occ_w;
		}

		if( _problem&PROBLEM_FREQSET){
			_occ_w2 = _occ_w;
			_occ_pw2 = _occ_pw; 
		}
		else{
  	 	_occ_w2 = new WEIGHT[siz+2](); //calloc2 
  	 	
    	if ( _TT.incNega() ){
    		 _occ_pw2 =  new WEIGHT[siz+2](); //calloc2
    	}
			else{
				_occ_pw2 = _occ_w2;
			}
		}

	  if(_sgfname) { _SG.itemAlloc(siz+2); }

	  _itemcand.alloc(siz+2);

	   // set outperm
	  if ( _outperm_fname ){
  	  IFILE2::ARY_Load(p,_outperm_fname);
    	if ( perm ){
    		for(j=0;j<siz;j++){
					perm[j] = p[perm[j]];
				}
				delete [] p;
	    }
  	  else perm = p;
  	}

		_II.alloc(_output_fname, perm, siz, 0);

		// adjust target
		_II.adjustTarget(siz);

	  return;

	}

	void _init (){

  	QUEUE_INT i;

		preALOCC();

		// threshold for database reduction
		// ここは_IIで
		_th = (_II.get_flag()&ITEMSET_RULE)? 
  			((_II.get_flag() & ITEMSET_RULE_INFRQ)? -WEIGHTHUGE : _II.get_frq_lb() * _II.get_ratio_lb() ): _II.get_frq_lb();
  			 

		if ( _TT.isShrink()){
  		_oo = _TT.dup_OQ(_TT.get_clms()); // preserve occ
  	}
  	else {
  		_oo.allocBySequence(_TT.get_t());
  	}

		_TT.set_perm(NULL);

	  _TT.resizeOQ(_TT.get_clms(),0);

		if ( !(_TT.exist_sc()) ) _TT.calloc_sc( _TT.get_clms()+2);
		
		_II.set_itemflag(_TT.get_sc());

		_II.set_frq( _TT.get_total_w_org()); 
  	_II.set_pfrq( _TT.get_total_pw_org());


	  if ( _sgfname ){

		  if ( _SG.edge_t() < _TT.get_clms() ){
    		// dmyセットすべき？
      	_TT.printMes( "#nodes in constraint graph is smaller than #items\n");
	    }

  	  if ( _TT.exist_perm() ){
    		_SG.adaptPerm(_TT.get_t(), _TT.get_perm());
    	}
  		_SG.edge_sort(LOAD_INCSORT +LOAD_RM_DUP);
  	}



	  _II.set_total_weight(0); //_TT.get_total_w_org()?
		_II.set_rows_org(_TT.get_rows_org());
		_II.set_trperm(_TT.get_trperm());
 
	}	

	int setArgs(int argc, char *argv[]);

	void reduce_occ_by_posi_equisupp ( QUEUE *occ, QUEUE_INT item, QUEUE_INT full);

  QUEUE_INT maximality_check (QUEUE *occ, QUEUE_INT item, QUEUE_INT *fmax, QUEUE_INT *cnt);

	void LCMCORE (int item, QUEUE *occ, WEIGHT frq, WEIGHT pfrq);


	public:



	KGLCM():
		_problem(0),_th(0),_clmsFlag(false),
		_occ_w(NULL),_occ_w2(NULL),_occ_pw(NULL),_occ_pw2(NULL),
		_outperm_fname(NULL),_output_fname(NULL),
		_sgfname(NULL),
		_rposi_lb(0),_rposi_ub(1),_pRatioFlg(false),_progressFlag(false)
		{};

	int run (int argc, char *argv[]);
	

};

