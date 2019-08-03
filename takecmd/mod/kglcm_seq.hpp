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

	ItemSetParams _ipara;
	TrsactParams _tpara;
	// filecountで必要
	LimitVal _limVal;


	bool _pRatioFlg;

	int _root,_dir;
  double _th;
	WEIGHT *_occ_w,*_occ_pw;

	QUEUE _itemjump,_itemcand;

	int _problem;
	char *_output_fname;
	char *_outperm_fname;
	
	int _gap_ub;
	int _len_ub;
	
	//機能してない？
	double _rposi_ub,_rposi_lb;

	void help ();
	
	void occ_delivery (KGLCMSEQ_QUE *occ, int flag);
	void rm_infreq();
	void reduce_occ ( KGLCMSEQ_QUE *occ, QUEUE_INT item);
	void _init (KGLCMSEQ_QUE *occ);

	void LCMseq (QUEUE_INT item, KGLCMSEQ_QUE *occ);

	/* allocate arrays and structures */
	void preALLOC(){

		PERM *p;
		int j;
  	
		QUEUE_ID siz = _TT.get_clms();
		QUEUE_ID siz2 = _TT.get_t();
		PERM *perm = _TT.get_perm();

		_occ_w = new WEIGHT[siz+2]();
	  
		if(_TT.incNega()){
			_occ_pw = new WEIGHT[siz+2](); //calloc2
	  }else{
			_occ_pw = _occ_w;
	  }

		_itemjump.alloc(siz+2);
		_itemcand.alloc(siz+2);
	
    // set outperm
	  if ( _outperm_fname ){

  	  j = IFILE2::ARY_Load(p, _outperm_fname);
  
			if ( perm ){
				for(j=0;j<siz;j++){ perm[j] = p[perm[j]]; }
				delete [] p;
			}
    	else{
				perm = p;
			}
		}

		_II.alloc(_output_fname, perm, siz2, _TT.get_row_max());

		// _IIでできるはず
		_II.adjustTarget(siz);

  	return;
  	ERR:;

	  EXIT;
	}

	int setArgs(int argc, char *argv[]);

	public:

	KGLCMSEQ():
		_problem(0),_root(0),_dir(0),_th(0),_occ_w(NULL),_occ_pw(NULL),
		_outperm_fname(NULL),_output_fname(NULL),_len_ub(INTHUGE),
		_rposi_lb(0),_rposi_ub(1),_gap_ub(INTHUGE),_pRatioFlg(false)
		{};

	int run(int argc, char *argv[]);

};


