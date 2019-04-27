/* frequent appearing item sequence enumeration algorithm based on LCM */
/* 2004/4/10 Takeaki Uno   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users.
   For the commercial use, please make a contact to Takeaki Uno. */


#define WEIGHT_DOUBLE

#include"trsact.hpp"
#include"problem.hpp"
#include "itemset.hpp" 
#include "queue.hpp" 


#define LCMSEQ_LEFTMOST 134217728
#define LCMSEQ_SET_RULE 268435456



class KGLCMSEQ{
	
	ITEMSET _II;
	TRSACT _TT;


	int _root,_dir;
  double _th;
	WEIGHT *_occ_w,*_occ_pw;

	char *_ERROR_MES;
	QUEUE _itemjump,_itemcand;



	int _problem;
	char *_output_fname;
	char *_outperm_fname;

	//_II
	int _iFlag;
	LONG _topk_k;
	int _lb,_ub;
	WEIGHT _frq_lb,_frq_ub;
	double _prob_lb,_prob_ub;
	double _ratio_lb,_ratio_ub;
	double _rposi_ub,_rposi_lb;
	double _nega_lb,_nega_ub;
	double _posi_lb,_posi_ub;
	int _gap_ub;
	int _target;
	int _max_solutions;
	char _separator;
	int _len_ub;
	WEIGHT _setrule_lb;
	

	//_TT
	int _tFlag , _tFlag2;
	char *_fname;
	char *_wfname;
	double _w_lb;

	
	
	void help ();
	
	void occ_delivery (KGLCMSEQ_QUE *occ, int flag);
	void rm_infreq ();
	void reduce_occ ( KGLCMSEQ_QUE *occ, QUEUE_INT item);
	void _init (KGLCMSEQ_QUE *occ);

	void LCMseq (QUEUE_INT item, KGLCMSEQ_QUE *occ);

	/* allocate arrays and structures */
	void preALLOC (QUEUE_ID siz, QUEUE_ID siz2, size_t siz3, PERM *perm ){

	  PERM *p;
  	int j;
		//int f	= PROBLEM_ITEMCAND +((_tFlag2&TRSACT_NEGATIVE)?PROBLEM_OCC_PW: PROBLEM_OCC_W) +PROBLEM_ITEMJUMP

  	//if ( f&(PROBLEM_OCC_W+PROBLEM_OCC_PW) ) 
  	// calloc2 (_occ_w, siz+2, goto ERR);
  	_occ_w = calloc2 (_occ_w, siz+2);


	  //if ( f&PROBLEM_OCC_PW ) calloc2 (_occ_pw, siz+2, goto ERR);
	  //else _occ_pw = _occ_w;
	  
	  if(_tFlag2&TRSACT_NEGATIVE){
		  //calloc2 (_occ_pw, siz+2, goto ERR);
		  _occ_pw = calloc2 (_occ_pw, siz+2);
	  }else{
		  _occ_pw = _occ_w;
	  }

		//if ( f&PROBLEM_ITEMJUMP ) _itemjump.alloc (siz+2);
	  //if ( f&PROBLEM_ITEMCAND ) _itemcand.alloc ( siz+2);
		_itemjump.alloc(siz+2);
		_itemcand.alloc(siz+2);
	
    // set outperm
	  if ( _outperm_fname ){

  	  j = FILE2::ARY_Load (p, _outperm_fname, 1);
    	if ( perm ){

     	 for(j=0;j<siz;j++){
     	 	 perm[j] = p[perm[j]];
     	 }
     	 delete [] p;
    	}
    	else{
				perm = p;
			}
  	}
		_II.alloc(_output_fname, perm, siz, siz3);

	  if ( _target < siz && _II.get_perm() ){
      //FLOOP (j, 0, _II.get_item_max()){ 
     	for(j=0;j<_II.get_item_max();j++){
      	if ( _target == _II.get_perm(j) ){ 
      		_II.set_target(j); 
      		break; 
      	} 
      }
    }
  	return;
  	ERR:;

	  EXIT;
	}

	int setArgs(int argc, char *argv[]);
  

	public:

	KGLCMSEQ():
		_problem(0),_root(0),_dir(0),_th(0),_occ_w(NULL),_occ_pw(NULL),
		_outperm_fname(NULL),_output_fname(NULL),_ERROR_MES(NULL),

		_ub(INTHUGE),_lb(0),
		_prob_ub(1),_prob_lb(0),
		_target(INTHUGE),
		_ratio_lb(0),_ratio_ub(1),
		_nega_lb(-WEIGHTHUGE),_nega_ub(WEIGHTHUGE),
		_posi_lb(-WEIGHTHUGE),_posi_ub(WEIGHTHUGE),
		_rposi_lb(0),_rposi_ub(1),_max_solutions(0),
		_separator(' '),_setrule_lb(-WEIGHTHUGE),
		_frq_lb(-WEIGHTHUGE),_frq_ub(WEIGHTHUGE),_topk_k(0),_gap_ub(INTHUGE),_len_ub(INTHUGE)
		{};

	int run(int argc, char *argv[]);

};



