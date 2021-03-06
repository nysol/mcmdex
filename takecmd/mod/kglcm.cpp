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

#include"kglcm.hpp"

void KGLCM::help(){

  fprintf(stderr,"LCM: [FCMfQIq] [options] input-filename support [output-filename]\n\
%%:show progress, _:no message, +:write solutions in append mode\n\
F:frequent itemset mining, C:closed frequent itemset mining\n\
M:maximal frequent itemset mining, P:positive-closed itemset mining\n\
f,Q:output frequency following/preceding to each output itemset\n\
A:output positive/negative frequency, and their ratio\n\
R:output redundant items for rule mining (usually, redundant items are removed, to be minimal, in the case of rule mining)\n\
I:output ID's of transactions including each pattern\n\
i:do not output itemset to the output file (only rules)\n\
s:output confidence and item frequency by absolute values\n\
t:transpose the input database (item i will be i-th transaction, and i-th transaction will be item i)\n\
[options]\n\
-K [num]:output [num] most frequent itemsets\n\
-l,-u [num]:output itemsets with size at least/most [num]\n\
-U [num]:upper bound for support(maximum support)\n\
-w [filename]:read weights of transactions from the file\n\
-c,-C [filename]:read item constraint/un-constraint file\n\
-i [num]: find association rule for item [num]\n\
-a,-A [ratio]: find association rules of confidence at least/most [ratio]\n\
-r,-R [ratio]: find association rules of relational confidence at least/most [ratio]\n\
-f,F [ratio]: output itemsets with frequency no less/greater than [ratio] times the frequency given by product of the probability of each item appearance\n\
-p,-P [num]: output itemset only if (frequency)/(abusolute frequency) is no less/no greater than [num]\n\
-n,-N [num]: output itemset only if its negative frequency is no less/no greater than [num] (negative frequency is the sum of weights of transactions having negative weights)\n\
-o,-O [num]: output itemset only if its positive frequency is no less/no greater than [num] (positive frequency is the sum of weights of transactions having positive weights)\n\
-m,-M [filename]:read/write item permutation from/to file [filename]\n\
-# [num]:stop after outputting [num] solutions\n\
-, [char]:give the separator of the numbers in the output\n\
-. [num]:give the number of decimal digits int the output numbers\n\
-Q [filename]:replace the output numbers according to the permutation table given by [filename]\n\
# the 1st letter of input-filename cannot be '-'.\n\
# if the output file name is -, the solutions will be output to standard output.\n");

	// LAMPはべつにしよう
  fprintf(stderr,"LCM_LAMP: [FCMfQIq] -LAMP #positives [-LAMP #transctions] [options] input-filename [alpha]\n");
  

  EXIT;
}

/***********************************************************************/
/*  read parameters given by command line  */
/***********************************************************************/
int KGLCM::setArgs(int argc, char *argv[]){

  int c=1, f=0;
  int iflag =0;
  int tflag =0;
  int tflag2 =0;

  if ( argc < c+3 ){ help(); return 1; }
  
  if ( !strchr (argv[c], '_') ){ iflag |= SHOW_MESSAGE; tflag |= SHOW_MESSAGE; }
  if ( strchr (argv[c], '%') ) { iflag |= SHOW_PROGRESS;  _progressFlag= true; }
  if ( strchr (argv[c], '+') ) iflag |= ITEMSET_APPEND;
  if ( strchr (argv[c], 'f') ) iflag |= ITEMSET_FREQ;
  if ( strchr (argv[c], 'Q') ) iflag |= ITEMSET_PRE_FREQ;
  if ( strchr (argv[c], 'R') ) iflag |= ITEMSET_RULE_ADD;
  if ( strchr (argv[c], 'A') ) iflag |= ITEMSET_OUTPUT_POSINEGA;

  if ( strchr (argv[c], 'C') )     { _problem |= PROBLEM_CLOSED;   tflag2 |= TRSACT_INTSEC;}
  else if ( strchr (argv[c], 'F') ){ _problem |= PROBLEM_FREQSET;  iflag  |= ITEMSET_ALL;  }
  else if ( strchr (argv[c], 'M') ){ _problem |= PROBLEM_MAXIMAL;  tflag2 |= TRSACT_UNION; }
  else {
  	fprintf(stderr,"one of F, C, M has to be given");
  	return 1;
  }

  if ( strchr (argv[c], 'P') ) _problem |= LCM_POSI_EQUISUPP;
  if ( strchr (argv[c], 'I') ) iflag |= ITEMSET_TRSACT_ID;
  if ( strchr (argv[c], 'i') ) iflag |= ITEMSET_NOT_ITEMSET;
  if ( strchr (argv[c], 's') ) iflag |= ITEMSET_RULE_SUPP;
  if ( strchr (argv[c], 't') ) tflag |= LOAD_TPOSE;

  c++;
  
  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){
      case 'K': 
      	if ( _problem & PROBLEM_MAXIMAL ){
          fprintf(stderr,"M command and -K option can not be given simltaneously");
          return 1;
        }
        _ipara._topk_k = (LONG)atof(argv[c+1]);

      break; case 'm': 
      	_pfname = argv[c+1] ;

      break; case 'M': 
      	_pfname = argv[c+1]; 
      	tflag2 |= TRSACT_WRITE_PERM;

      break; case 'l': 
				_ipara._lb = atoi(argv[c+1]);

      break; case 'u':
				_ipara._ub = atoi(argv[c+1]);
       
      break; case 'U': 
      	_ipara._frq_ub = (WEIGHT)atof(argv[c+1]);

      break; case 'w': 
      	_tpara._wfname = argv[c+1];

      break; case 'c': 
      	_sgfname = argv[c+1];
      	
      break; case 'C': 
      	_sgfname = argv[c+1]; 
      	_problem |= LCM_UNCONST;

      break; case 'f': 
      	_ipara._prob_lb = atof(argv[c+1]);
      	if ( !RANGE (0, _ipara._prob_lb, 1) ){
          fprintf(stderr,"ratio has to be in [0,1] %f",atof(argv[c+1]));
          return 1;
        }
        iflag |= ITEMSET_RFRQ; 
        f++;

      break; case 'F': 
      	_ipara._prob_ub = atof(argv[c+1]);
      	if ( !RANGE (0, _ipara._prob_ub, 1) ){
          fprintf(stderr,"ratio has to be in [0,1] %f",atof(argv[c+1]));
          return 1;
        }
        iflag |= ITEMSET_RINFRQ; 
        f++;

      break; case 'i': 
      	_ipara._target = atoi(argv[c+1]);

      break; case 'a': 
	      _ipara._ratio_lb = atof(argv[c+1]);
      	if ( !RANGE (0, _ipara._ratio_lb , 1) ){
          fprintf(stderr,"ratio has to be in [0,1] %f",atof(argv[c+1]));
          return 1;
				}
        iflag |= ITEMSET_RULE_FRQ; 
        f|=1;

      break; case 'A': 
      	_ipara._ratio_ub = atof(argv[c+1]);
      	if ( !RANGE (0, _ipara._ratio_ub, 1) ){
          fprintf(stderr,"ratio has to be in [0,1] %f",atof(argv[c+1]));
          return 1;
        }
        iflag |= ITEMSET_RULE_INFRQ;
        f|=1;

      break; case 'r': // a,rは同時に指定できない？
      	_ipara._ratio_lb = atof(argv[c+1]);
      	if ( !RANGE (0, _ipara._ratio_lb, 1) ){
          fprintf(stderr,"ratio has to be in [0,1] %f",atof(argv[c+1]));
          return 1;
        }
        iflag |= ITEMSET_RULE_RFRQ; 
        f|=2;

      break; case 'R': // A,Rは同時に指定できない
      	_ipara._ratio_ub = atof(argv[c+1]);
      	if ( !RANGE (0, _ipara._ratio_ub, 1) ){
          fprintf(stderr,"ratio has to be in [0,1] %f",atof(argv[c+1]));
          return 1;
        }
        iflag |= ITEMSET_RULE_RINFRQ; 
        f|=2;
      
      break; case 'P': 
	      _pRatioFlg = true;
      	iflag |= ITEMSET_IGNORE_BOUND; 
      	_rposi_ub = atof(argv[c+1]); 
      	f|=4;

      break; case 'p': 
	      _pRatioFlg = true;
      	iflag |= ITEMSET_IGNORE_BOUND; 
      	_rposi_lb = atof(argv[c+1]); 
      	f|=4;

      break; case 'n': 
      	_ipara._nega_lb = atof(argv[c+1]);
      
      break; case 'N': 
      	_ipara._nega_ub = atof(argv[c+1]);
      
      break; case 'o': 
      	_ipara._posi_lb = atof(argv[c+1]);
      
      break; case 'O': 
      	_ipara._posi_ub = atof(argv[c+1]);
      
      break; case '#': 
      	_ipara._max_solutions = atoi(argv[c+1]);
      
      break; case ',': 
      	_ipara._separator = argv[c+1][0] ;
      
      break; case '.': 
      	_ipara._digits = atoi(argv[c+1]);
      
      break; case 'Q': 
      	_outperm_fname = argv[c+1];
      
      break; default: 
      	goto NEXT;
    }
    c += 2;
    if ( argc < c+2 ){ help(); return 1; }
  }

  NEXT:;

  if ( (f&3)==3 || (f&5)==5 || (f&6)==6 ){
  	fprintf(stderr,"-f, -F, -a, -A, -p, -P, -r and -R can not specified simultaneously");
  	return 1; 
  }

  if ( f && (iflag & ITEMSET_PRE_FREQ) ){
  	iflag -= ( iflag & ITEMSET_PRE_FREQ );
  }

  if ( ( _problem & PROBLEM_CLOSED ) && _sgfname ){
  	fprintf(stderr,"closed itemset mining does not work with item constraints");
   	return 1; 
 }

  if ( (_problem & PROBLEM_FREQSET) && (iflag & (ITEMSET_RULE + ITEMSET_RFRQ + ITEMSET_RINFRQ)) ){
    _problem |= PROBLEM_CLOSED; 
    _problem -= (_problem & PROBLEM_FREQSET );
  	iflag -= ( iflag & ITEMSET_ALL );

  }

  _tpara._fname = argv[c];
  _ipara._frq_lb = (WEIGHT)atof(argv[c+1]);

  if ( argc>c+2 ) _output_fname = argv[c+2];

  tflag  |= (LOAD_PERM +LOAD_DECSORT +LOAD_RM_DUP);


  tflag2 |= TRSACT_FRQSORT +TRSACT_MAKE_NEW +TRSACT_DELIV_SC + 
  							((iflag & ITEMSET_TRSACT_ID) ? 0: (TRSACT_SHRINK+TRSACT_1ST_SHRINK));

  if ( iflag & ITEMSET_RULE ){ _limVal._w_lb = -WEIGHTHUGE; } 
  else											 { _limVal._w_lb = _ipara._frq_lb;}

	iflag |= (ITEMSET_ITEMFRQ + ITEMSET_ADD); 

  if (!_tpara._wfname){// なぜwegihtがない時だけ
    if( _ipara._topk_k > 0 ) iflag |= ITEMSET_SC2; 
  }

  // Fが指定されていて かつ -A ,-a -r -R が指定されていない時
	_clmsFlag = ((_problem&PROBLEM_FREQSET)&&(iflag&ITEMSET_RULE)==0);

	_ipara._flag  = iflag;
	_tpara._flag  = tflag;
	_tpara._flag2 = tflag2;


  return 0;
}

/* remove unnecessary transactions which do not include all posi_closed items */
/* scan of each transaction is up to item */
void KGLCM::reduce_occ_by_posi_equisupp ( 
	QUEUE *occ, QUEUE_INT item, QUEUE_INT full
){

  QUEUE_ID ii=0;

  QUEUE_INT *x, *y, *z, cnt;

	for(x=occ->begin();x<occ->end();x++ ){

    if ( _TT.get_w(*x)>= 0 ) continue;
    cnt = 0;
    
		for(y=_TT.beginTv(*x); *y < item ; y++){  
    	if ( _II.get_itemflag(*y) == 2 ) cnt++;
    }

    if ( cnt==full ) occ->set_v(ii++,*x);
    else {

      _II.set_frq( _II.get_frq() - _TT.get_w(*x));

			for(z=_TT.beginTv(*x) ; *z<item ; z++){
	      _occ_w[*z] -= _TT.get_w(*x);
	    }
    }
  }

  occ->set_t (ii);

	for(x=_itemcand.begin() ; x<_itemcand.end() ; x++){
	    if ( _II.get_itemflag(*x) == 2 ) _II.set_itemflag(*x, 1);
  }
}

/*************************************************************************/
/* ppc check and maximality check */
/* INPUT: O:occurrence, jump:items, th:support, frq:frequency, add:itemset
   OUTPUT: maximum item i s.t. frq(i)=frq
   OPERATION: remove infrequent items from jump, and 
    insert items i to "add" s.t. frq(i)=frq                              */
/*************************************************************************/
/* functions
  1. when closed itemset mining or maximal frequent itemset mining, find all items
   included in all transactions in occ (checked by pfrq, occ_w
   if there is such an item with index>item, ppc condition is violated, and return non-negative value
  2. when constraint graph is given, set the frequency (occ_w) of the items which can
   not be added to itemset to infrequent number.
  3. count the size of reduced database
  4. call LCM_reduce_occ_posi
 */
QUEUE_INT KGLCM::maximality_check (
	QUEUE *occ, QUEUE_INT item, 
	QUEUE_INT *fmax, QUEUE_INT *cnt)
{

  QUEUE_INT m = _TT.get_clms(), full=0, *x;
  WEIGHT w=-WEIGHTHUGE;
  *fmax = _TT.get_clms(); *cnt=0;


  for(x=_TT.beginJump();x<_TT.endJump() ; x++){

    if ( _II.get_itemflag(*x) == 1) continue;

    if ( _sgfname && ( (((_problem & LCM_UNCONST)==0) && (_SG.itemCnt(*x)>0) ) || 
        ((_problem & LCM_UNCONST) && (_SG.itemCnt(*x)<_II.item_get_t() ))) ){
      // e can not be added by item constraint
      _II.set_itemflag(*x,3);
    } 
    else if ( ISEQUAL(_occ_pw[*x],_II.get_pfrq()) && ( ISEQUAL(_occ_w[*x],_II.get_frq()) || (_problem & LCM_POSI_EQUISUPP) ) ){ // check e is included in all transactions in occ

      if ( *x<item ){
        if ( !_sgfname ){ // add item as "equisupport"

					_II.addEquiSupport(*x);
          if ( (_problem&LCM_POSI_EQUISUPP) && (_II.get_flag()&ITEMSET_RULE) ){				
						// in POSI_EQUISUPP, occ_w[*x] is not equal to II->frq, 
						// thus we have to deal it in the rule mining
          	 _II.set_itemflag(*x,0); 
          }
        }
        if ( !ISEQUAL(_occ_w[*x],_II.get_frq()) ){ full++; _II.set_itemflag(*x, 2); }

      }
      else {
      	 // an item in prefix can be added without going to another closed itemset
      	 m = *x;
      }

    }
    else {
      if ( *x<item ) (*cnt)++;
      _II.set_itemflag( *x , _occ_pw[*x] < _th? 3: 0); // mark item by freq/infreq

      if ( _occ_w[*x] > w ){
        *fmax = *x;
        w = _occ_w[*x];
      }
    }
  }
  if ( full && (_problem & LCM_POSI_EQUISUPP) && m<item ) // m<item always holds in frequent itemset mining
       reduce_occ_by_posi_equisupp (occ, item, full);
  return (m);
}

/***************************************************************/
/* iteration of LCM ver. 5 */
/* INPUT: item:tail of the current solution, t_new,buf:head of the list of 
 ID and buffer memory of new transactions */
/*************************************************************************/
void KGLCM::LCMCORE (int item, QUEUE *occ, WEIGHT frq, WEIGHT pfrq){

	BaseStatusWithWeight bStsSave  = _TT.getBaseStsW();


  QUEUE_INT cnt, f, *x, m, e;


	// _clmsFlag = ((_problem&PROBLEM_FREQSET)&&(_iFlag&ITEMSET_RULE)==0);
  QUEUE_INT imax = _clmsFlag ? item: _TT.get_clms();


  QUEUE_ID js = _itemcand.get_s(); 
  QUEUE_ID qt = _II.iadd_get_t();


  _itemcand.setStartByEnd();

  _II.inc_iters();

	// -P , -p
  WEIGHT rposi=0.0;
  if ( _pRatioFlg && pfrq!=0 ) {
  	rposi = pfrq / (pfrq + pfrq - _II.get_frq());
  }

  _TT.delivery( _occ_w, _occ_pw, occ, imax);

	 // for ratio pruning when item target is set.
  if ( 
  	_II.get_target() < _II.get_item_max() && 
  	_occ_w[_II.get_target()] < _II.get_frq_lb()*_II.get_ratio_lb() )
  {
		_TT.clrOQendByJump();
  	goto END; 
  }

  // if the itemset is empty, set frq to the original #trsactions, and compute item_frq's
  if ( _II.get_itemset_t() == 0 ){
    if ( _TT.get_total_w_org() != 0.0 ){
    	for(size_t i=0; i < _TT.get_clms();i++){
        _II.set_item_frq( i , _occ_w[i]/_TT.get_total_w_org() );
      }
    }
  }

	// セットし直し？
  _II.set_frq(frq); 
  _II.set_pfrq(pfrq);

  m = maximality_check ( occ, item, &f, &cnt);


  if ( !(_problem & PROBLEM_FREQSET) && m<_TT.get_clms() ){  // ppc check

		_TT.clrOQendByJump();
    goto END;

  }

  if ( !(_problem&PROBLEM_MAXIMAL) || f >= _TT.get_clms() || _occ_w[f] < _II.get_frq_lb() ){

    if ( !_pRatioFlg || (rposi<=_rposi_ub && rposi>=_rposi_lb) ){
    			
      _II.check_all_rule ( _occ_w, occ, _TT.getp_jump(), _TT.get_total_pw_org() );     

    }
  } 

  // select freqeut (and addible) items with smaller indices
	for(x=_TT.beginJump();x<_TT.endJump() ; x++){
		

		// in the case of freqset mining, automatically done by rightmost sweep;
    _TT.clrOQend(*x);  

    if ( _occ_w[*x] < _II.get_frq_lb() ) _occ_w[*x] = 0;  // modified 21/May/2018

    if ( *x<item && _II.get_itemflag(*x) == 0 ){
      _itemcand.push_back(*x);
      _occ_w2[*x] = _occ_w[*x];
      if ( _TT.incNega() ) _occ_pw2[*x] = _occ_pw[*x];

    }
  }

  if ( _itemcand.size()==0 || _II.get_itemset_t() >= _II.get_ub() ) goto END;

  qsort_<QUEUE_INT> (_itemcand.get_v()+_itemcand.get_s(), _itemcand.get_t()- _itemcand.get_s(), -1);
  qsort_<QUEUE_INT> (_II.iadd_get_v()+qt, _II.iadd_get_t()-qt, -1);

	// database reduction
  if ( cnt>2 && (_II.get_flag() & ITEMSET_TRSACT_ID)==0 && _II.get_itemset_t() >0){
  	_TT.dbReduction(occ, item);
  }
  
	// occurrence deliver
	_TT.deliv(occ, item);

	// loop for recursive calls
  cnt = _itemcand.size(); f=0;   // for showing progress
  

  while (_itemcand.size() > 0 ){

    e = _itemcand.pop_back();

    if ( _occ_pw2[e] >= MAX(_II.get_frq_lb(), _II.get_posi_lb()) ){  // if the item is frequent

			_II.addCurrent(e);
		  if ( _sgfname  ){ _SG.itemCntUp(e); }
      LCMCORE( e, _TT.getp_OQ(e), _occ_w2[e], _occ_pw2[e]); // recursive call
			_II.delCurrent();
		  if ( _sgfname  ){ _SG.itemCntDown(e); }
			
    }
    // clear the occurrences, for the further delivery
    _TT.clrOQend(e);
    _TT.clrOQt(e);

    _occ_w[e] = _occ_pw[e] = -WEIGHTHUGE;  // unnecessary?
    
    if ( _progressFlag && (_II.item_get_t() == 0 ) ){
      f++; 
      fprintf(stderr,"%d/%d (" LONGF " iterations)\n", f, cnt, _II.get_iters());
    }
  }
	
	_TT.setBaseStsW(bStsSave);


 END:;

  while ( _II.iadd_get_t() > qt ) {

		QUEUE_INT item = _II.delEquiSupport();
		if ( _sgfname ){ _SG.itemCntDown(item); }

  }

	_itemcand.setEndByStart();  //_itemcand.set_t(_itemcand.get_s());
  _itemcand.set_s(js);

}

/*************************************************************************/
/* main of LCM ver. 5 */
/*************************************************************************/
int KGLCM::run (int argc, char *argv[]){


	if( setArgs(argc, argv) ) return 1;

	_TT.setParams(_tpara,_limVal);


  if( _TT.load() ){ return 1; }

	if ( _sgfname ){ 
		if (_SG.loadEDGE(LOAD_EDGE + LOAD_RC_SAME , _sgfname)) return 1;
  }


  if (!_TT.exist_wfname()){// adjust なぜwegihtがない時だけ
		ENMIN(_ipara._frq_ub, (WEIGHT)_TT.get_t());
  }

	_II.setParams(_ipara );
 
  if ( _TT.get_clms()>0 ){
    _init();
		LCMCORE(_TT.get_clms(), &_oo, _TT.get_total_w_org(), _TT.get_total_pw_org());
    _II.last_output();
  }

  return 0;
}


