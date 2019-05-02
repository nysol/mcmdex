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

#include "kglcm_seq.hpp"

void KGLCMSEQ::help(){
  _ERROR_MES = "command explanation";
  fprintf(stderr,"LCMseq: [FCfQIq] [options] input-filename support [output-filename]\n\
%%:show progress, _:no message, +:write solutions in append mode\n\
F:position occurrence, C:document occurrence\n\
m:output extension maximal patterns only, c:output extension closed patterns only\n \
f,Q:output frequency following/preceding to each output sequence\n\
A:output coverages for positive/negative transactions\n\
I(J):output ID's of transactions including each pattern, if J is given, an occurrence is written in a complete stype; transaction ID, starting position and ending position\n\
i:do not output itemset to the output file (only rules)\n\
s:output confidence and item frequency by absolute values\n\
t:transpose the input database (item i will be i-th transaction, and i-th transaction will be item i)\n\
[options]\n\
-K [num]: output [num] most frequent sequences\n\
-l,-u [num]: output sequences with size at least/most [num]\n\
-U [num]: upper bound for support(maximum support)\n\
-g [num]: restrict gap length of each consequtive items by [num]\n\
-G [num]: restrict window size of the occurrence by [num]\n\
-w [filename]:read weights of transactions from the file\n\
-i [num]: find association rule for item [num]\n\
-a,-A [ratio]: find association rules of confidence at least/most [ratio]\n\
-r,-R [ratio]: find association rules of relational confidence at least/most [ratio]\n\
-f,-F [ratio]: output sequences with frequency no less/greater than [ratio] times the frequency given by the product of appearance probability of each item\n\
-p,-P [num]: output sequence only if (frequency)/(abusolute frequency) is no less/no greater than [num]\n\
-n,-N [num]: output sequence only if its negative frequency is no less/no greater than [num] (negative frequency is the sum of weights of transactions having negative weights)\n\
-o,-O [num]: output sequence only if its positive frequency is no less/no greater than [num] (positive frequency is the sum of weights of transactions having positive weights)\n\
-s [num]: output itemset rule (of the form (a,b,c) => (d,e)) with confidence at least [num] (only those whose frequency of the result is no less than the support)\n\
-# [num]: stop after outputting [num] solutions\n\
-, [char]:give the separator of the numbers in the output\n\
-Q [filename]:replace the output numbers according to the permutation table given by [filename]\n\
# the 1st letter of input-filename cannot be '-'.\n\
# if the output file name is -, the solutions will be output to standard output.\n");
  EXIT;
}

/***********************************************************************/
/*  read parameters given by command line  */
/***********************************************************************/
int KGLCMSEQ::setArgs(int argc, char *argv[]){

  //ITEMSET *II = &PP->II;
  int c=1, f=0;
  if( argc < c+3 ){ help(); return 1; }
  
  if(strchr(argv[c],'C')){ 
  	_problem |= PROBLEM_CLOSED+LCMSEQ_LEFTMOST;  
  	_iFlag |= ITEMSET_RM_DUP_TRSACT; 
  }
  else if (strchr( argv[c], 'F') ){
  	 _problem |= PROBLEM_FREQSET; 
  }
  else { 
  	fprintf(stderr,"F or C command has to be specified");
  	return 1;
  }

  if ( !strchr (argv[c], '_') ) { _iFlag |= SHOW_MESSAGE; _tFlag |= SHOW_MESSAGE; }
  if ( strchr (argv[c], '%') )  { _iFlag |= SHOW_PROGRESS; }
  if ( strchr (argv[c], '+') )  { _iFlag |= ITEMSET_APPEND;}
  if ( strchr (argv[c], 'f') )  { _iFlag |= ITEMSET_FREQ;  }
  if ( strchr (argv[c], 'A') )  { _iFlag |= ITEMSET_OUTPUT_POSINEGA; }
  if ( strchr (argv[c], 'R') )  { _problem |= ITEMSET_POSI_RATIO; _iFlag |= ITEMSET_IGNORE_BOUND; }
  if ( strchr (argv[c], 'Q') )  { _iFlag |= ITEMSET_PRE_FREQ; }
  if ( strchr (argv[c], 'I') || strchr (argv[c], 'J') ){
    _iFlag |= ITEMSET_TRSACT_ID ;  // single occurrence

    if ( _problem & PROBLEM_FREQSET ) { _iFlag |= ITEMSET_MULTI_OCC_PRINT; } // output pair

    if ( strchr (argv[c], 'J') ){
	    _iFlag -= ITEMSET_TRSACT_ID; // for outputting tuple いみわからん
      _iFlag |= ITEMSET_MULTI_OCC_PRINT;
    }
  }
  if ( strchr (argv[c], 'i') ) { _iFlag |= ITEMSET_NOT_ITEMSET; }
  if ( strchr (argv[c], 's') ) { _iFlag |= ITEMSET_RULE_SUPP; }
  if ( strchr (argv[c], 't') ) { _tFlag |= LOAD_TPOSE; }
  if ( strchr (argv[c], 'm') ) { _problem |= PROBLEM_EX_MAXIMAL;}
  if ( strchr (argv[c], 'c') ) { _problem |= PROBLEM_EX_CLOSED; }
  c++;
  
  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){
      case 'K': 
      	_topk_k = (LONG)atof(argv[c+1]);

      break; case 'l': 
      	_lb = atoi(argv[c+1]);

      break; case 'u': 
      	_ub = atoi(argv[c+1]);
      	
      break; case 'U': 
      	_frq_ub = (WEIGHT)atof(argv[c+1]);

      break; case 'g':
      	_gap_ub = atoi(argv[c+1]);

      break; case 'G': 
      	_len_ub = atoi(argv[c+1]);
      	
      break; case 'w': 
      	_wfname = argv[c+1];

      break; case 'f': 
      	_prob_lb = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_RFRQ; 
      	f++;

      break; case 'F': 
      	_prob_ub = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_RINFRQ; 
      	f++;

      break; case 'i': 
      	_target = atoi(argv[c+1]);

      break; case 'a': 
      	_ratio_lb = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_RULE_FRQ; 
      	f|=1;

      break; case 'A': 
      	_ratio_ub = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_RULE_INFRQ; 
      	f|=1;

      break; case 'r': 
      	_ratio_lb = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_RULE_RFRQ; 
      	f|=2;
      	
      break; case 'R': 
      	_ratio_ub = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_RULE_RINFRQ; 
      	f|=2;

      break; case 'P': 
      	_iFlag |= ITEMSET_POSI_RATIO; 
      	_iFlag |= ITEMSET_IGNORE_BOUND; 
      	_rposi_ub = atof(argv[c+1]); 
      	f|=4;

      break; case 'p': 
      	_iFlag |= ITEMSET_POSI_RATIO; 
      	_iFlag |= ITEMSET_IGNORE_BOUND; 
      	_rposi_lb = atof(argv[c+1]); 
      	f|=4;

      break; case 'n': 
      	_nega_lb = atof(argv[c+1]);

      break; case 'N': 
      	_nega_ub = atof(argv[c+1]);

      break; case 'o': 
      	_posi_lb = atof(argv[c+1]);

      break; case 'O': 
      	_posi_ub = atof(argv[c+1]);

      break; case 's': 
      	_setrule_lb = atof(argv[c+1]); 
      	_iFlag |= ITEMSET_SET_RULE;

      break; case '#': 
      	_max_solutions = atoi(argv[c+1]);

      break; case ',': 
      	_separator = argv[c+1][0];
      	
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
      fprintf(stderr , "-f, -F, -a, -A, -p, -P, -r and -R can not specified simultaneously\n");
      return 1;
  }

  if ( f ) {
		_iFlag -= ( _iFlag & ITEMSET_PRE_FREQ );  
  }

  if ( _len_ub < INTHUGE || _gap_ub < INTHUGE ) {
		_problem -= ( _problem & LCMSEQ_LEFTMOST );
  }

  _fname = argv[c];
  _frq_lb = (WEIGHT)atof(argv[c+1]);

  if ( argc>c+2 ){
  	_output_fname = argv[c+2];
  }
  return 0;

}



/*******************************************************/
/* compute occurrences of all frequent items */
/* if flag!=NULL, construct each OQ[i] (derivery), o.w., compute frequency of i */
/*******************************************************/
void KGLCMSEQ::occ_delivery (KGLCMSEQ_QUE *occ, int flag){

  QUEUE_ID j;
  QUEUE_INT e, m, *x;
  WEIGHT w;
  KGLCMSEQ_ELM *u, *uu, *u_end = occ->_v + (occ->_t-1);
  int f = _TT.get_flag2()&TRSACT_NEGATIVE;
  int fl = (!(_problem&PROBLEM_CLOSED)&&!flag) || (!(_problem&LCMSEQ_LEFTMOST)&&flag);

	for( u =  occ->begin() ; u < occ->end() ; u++){

    m = MAX (MAX(0, u->_s -_II.get_gap_ub()), u->_org -(_II.get_len_ub()-1));

    w = _TT.get_w(u->_t);

    if ( _II.get_itemset_t() == 0 ) m = 0;

    _itemjump.set_t(0);

   	//BLOOP (j, u->_s, m){
    for(j=u->_s;(j--)>m;){

      e = _TT.get_Tvv(u->_t,j); // e:= letter
			
			// not leftmost, or the first appearance of the letter
      if ( fl || _TT.get_sc(e) == 0 ){   
      
				// mark the letter
        if ( !fl ){ _TT.set_sc(e,1); _itemjump.push_back(e); } 
        
        // if occurrence computing
        if ( flag ){  
          uu = & ( (KGLCMSEQ_ELM *) (_TT.beginOQvELE(e)) ) [_TT.get_OQ_tELE(e)];
          uu->_t = u->_t;
          uu->_s = j;
          uu->_org = _II.item_get_t()? u->_org: j;
          _TT.inc_OQtELE(e);

        } 
        // for just frequency counting
        else {  
	        // initialize weights if this is the first insertion
          if ( _TT.get_OQ_endELE(e) == 0 ){ 
            _itemcand.push_back(e);
            _occ_w[e] = _occ_pw[e] = 0;
          }
          _TT.inc_OQendELE(e);
          _occ_w[e] += w;
          if ( f && w>0 ) _occ_pw[e] += w;
        }
      }
    }

    if ( !fl ){
    	for ( x = _itemjump.begin() ; x < _itemjump.end() ; x++ ) {
	    	// delete first item mark
	    	 _TT.set_sc(*x, 0);  
  	  }
  	}
  }
}


/* remove infrequent items from jump, and set active/in-active marks */
void KGLCMSEQ::rm_infreq (){

  QUEUE_ID i=_itemcand.get_s(), j=_itemcand.get_t();
  QUEUE_INT x;

  
  while (i < j){
    x = _itemcand.get_v(i);

    if ( _occ_pw[x] >= _II.get_frq_lb() ){ 

    	_TT.set_sc(x, 0); 
    	i++; 
    }
    else {
      _TT.clrOQtELE(x);
      _TT.clrOQendELE(x);
      _itemcand.set_v(i, _itemcand.get_v(--j));
      _itemcand.set_v(j, x);
			// remove infrequent items only when gap constraint not given
      if ( _root ) _TT.set_sc(x,3);  

    }
  }
  _itemcand.set_t(i);

}

/* remove merged occurrences from occ, and re-set temporary end-marks marked in each occurrence */
void KGLCMSEQ::reduce_occ ( KGLCMSEQ_QUE *occ, QUEUE_INT item){
	// _TT へ移動?
  KGLCMSEQ_ELM *u, *uu=occ->_v;

	for( u =  occ->begin() ; u < occ->end() ; u++){

		_TT.set_Tvv(u->_t,u->_s,item);

    if ( _TT.get_mark(u->_t) == 0 ) continue;
    *uu = *u;
    // update positions in occ, for shrinked transactions
    if ( _TT.get_mark(u->_t) > 1 ){
      uu->_t = _TT.get_mark(u->_t) -2;
			// actually, org is not used when database is shrinked
      uu->_org = _TT.get_Tvt(uu->_t) + u->_org - u->_s;  
      uu->_s = _TT.get_Tvt(uu->_t);
    }
    uu++;
  }
  occ->_t = (VEC_ID)(uu - occ->_v);
}

/***************************************************************/
/* iteration of LCMseq */
/* INPUT: occurrences of current sequence */
/*************************************************************************/
void KGLCMSEQ::LCMseq (QUEUE_INT item, KGLCMSEQ_QUE *occ){


  QUEUE_ID js= _itemcand.get_s(), jt=0, i, j;
  VEC_ID new_t = _TT.get_new_t();
  int bnum = _TT.get_bnum();
  int bblock = _TT.get_bblock();
  int output_flag = 1;
  QUEUE_INT *x, cnt=0, tt=_TT.get_rows_org();
  WEIGHT *w=NULL, *pw=NULL;
  double prob = _II.get_prob();
  KGLCMSEQ_ELM *u, L;

  //QUEUE *Q = NULL;  
  KGLCMSEQ_QUE *Q= NULL;

	// re-computing frequency, for (document occurrence & non-leftmost)
  if ( (_problem & PROBLEM_CLOSED) && !(_problem & LCMSEQ_LEFTMOST)){
    _II.set_frq (0);
    _II.set_pfrq(0);

		for( u =  occ->begin() ; u < occ->end() ; u++){
      if ( u->_t != tt ){
        _II.set_frq ( _II.get_frq() +_TT.get_w(u->_t) );
        if ( _TT.get_w(u->_t) > 0 ) _II.set_pfrq ( _II.get_pfrq() + _TT.get_w(u->_t));
      }
      tt = u->_t;
    }
  }

  _itemcand.set_s(_itemcand.get_t()); // initilization for the re-use of queue
  _II.inc_iters();
  if ( _problem & PROBLEM_EX_CLOSED ) _th = _II.get_frq();  // threshold value for for ex_maximal/ex_closed check; in the case of maximal, it is always II->frq_lb
  if ( _II.get_flag()&ITEMSET_POSI_RATIO && _II.get_pfrq()!=0 ) _II.set_frq ( _II.get_frq() / (_II.get_pfrq()+_II.get_pfrq()-_II.get_frq()));

    // if the itemset is empty, set frq to the original #trsactions, and compute item_frq's

  occ_delivery( occ, 0);


  if ( _II.get_itemset_t() == 0 ){
		_II.set_frq ( _TT.get_total_w_org());
    if ( _II.get_frq() != 0 )
        //FLOOP (i, 0, _TT.get_clms()){
        for(i=0;i<_TT.get_clms();i++){
         _II.set_item_frq(i,_occ_w[i]/_TT.get_total_w_org());
        }
  }

  _II.set_prob(1.0);
  _II.item_cal_prob();

    // extending maximality/closedness check
  if ( _problem & (PROBLEM_EX_MAXIMAL+PROBLEM_EX_CLOSED) ){
  
		for( x =  _itemcand.begin() ; x < _itemcand.end() ;x++){
			if ( _occ_w[*x] >= _th ) { output_flag = 0; }
		}
  }

  if ( output_flag ){
      _II.check_all_rule (_occ_w, occ, &_itemcand, _TT.get_total_pw_org(), 0);
  }

	for( x = _itemcand.begin();x < _itemcand.end() ;x++){
		if ( _occ_pw[*x] >= _II.get_frq_lb() ){ cnt++; }
	}
  

  if ( cnt == 0 || _II.get_itemset_t() >= _II.get_ub() ) goto END;

	///////     database reduction     ///////////
  if ( cnt > 5 && _dir && occ->_t>2 && _II.get_itemset_t()>0 ){ //なんで５？

		Q = _TT.getp_OQELE(_TT.get_clms());
    Q->set_s(0);
    Q->set_t(0);

		for( u = occ->begin(); u < occ->end() ; u++){
			_TT.set_Tvv(u->_t,u->_s,_TT.get_clms());
      Q->push_back(*u);
    }

    _TT.find_same ( Q, _TT.get_clms());
    _TT.merge_trsact ( Q, _TT.get_clms());

    // erase end-mark of each occurrence transaction, and remove unified occurrences
    reduce_occ (occ, item);

		//これOK?
		for( KGLCMSEQ_ELM * xxx = Q->begin(); xxx < Q->end() ; xxx++){ 
			_TT.set_Tvv(u->_t,u->_s,item);
		}

    i=0;

		for( u = occ->begin(); u < occ->end() ; u++){ //これOK?

      if ( _TT.get_Tvv(u->_t,u->_s) == item ) occ->_v[i++] = *u;
      else _TT.set_Tvv ( u->_t,u->_s,item);

    }
    occ->_t = i;
  }

	/////////////     deliverly    /////////////
  jt = _itemcand.get_t();
  occ_delivery (occ, 1);
  rm_infreq();

  cnt = _itemcand.size();
	_itemcand.occ_dupELE( 
		&Q, _TT.get_OQELE(), &w, _occ_w, &pw, _occ_pw, sizeof(KGLCMSEQ_ELM)
	);

  if ( Q == NULL ) goto END;

	for(x=_itemcand.begin(); x < _itemcand.end() ;x++ ){
  	_TT.clrOQendELE(*x);
  	_TT.clrOQtELE(*x);
  }

  _itemcand.set_t(_itemcand.get_s());

	/************ recursive calls ***************/
  //FLOOP (i, 0, cnt){
	for(i=0;i<cnt;i++){

    _II.set_frq(w[i]);
    _II.set_pfrq(pw[i]);
    if ( _II.get_flag() & ITEMSET_SET_RULE ){
      _II.set_set_weight(_II.get_itemset_t(), _II.get_frq());
			_II.set_set_occELE(_II.get_itemset_t(), &Q[i]);

    }
    _II.itemINS(Q[i].get_end());
    // reverse occurrence order for the first iteration 
    // (for the process of overlapping occurrences)
    if ( _II.get_itemset_t() == 0 ){    
      u = _TT.get_OQ_vELE(i);

      //FLOOP (j, 0, _TT.get_OQ_tELE(i)/2){
      for(i=0;j<_TT.get_OQ_tELE(i)/2;j++){
        L = u[j]; u[j] = u[_TT.get_OQ_tELE(i)-j-1]; u[_TT.get_OQ_tELE(i)-j-1] = L;
      }
    }
    LCMseq ( Q[i].get_end(),&Q[i]);

    _II.item_dec_t();

  }
  free2 (Q);

  _TT.set_new_t(new_t);
  _TT.set_bnum(bnum);
  _TT.set_bblock(bblock);

  //FLOOP (x, _itemcand.get_v()+cnt, _itemcand.get_v() + jt) {
 	for(x= _itemcand.get_v()+cnt ; x <  _itemcand.get_v() + jt ;x++){

	  _TT.set_sc(*x,0); // clear infrequent markF
	}

  END:;
  
  for(x=_itemcand.begin() ; x<_itemcand.end();x++){
  	_TT.clrOQendELE(*x);
  	_TT.clrOQtELE(*x);
  }
  _itemcand.set_t(_itemcand.get_s());
  _itemcand.set_s(js);
  _II.set_prob( prob);
}

/*************************************************************************/
/* initialization of LCMseq ver. 2 */
/*************************************************************************/
void KGLCMSEQ::_init (KGLCMSEQ_QUE *occ){

  VEC_ID i;
  QUEUE_ID j;
  QUEUE_INT *x;
  KGLCMSEQ_ELM L;


	preALLOC(
		_TT.get_clms(), _TT.get_t(), _TT.get_row_max(), _TT.get_perm()
	);
	

  //malloc2 (occ->_v, _TT.get_t(), EXIT);
	occ->_v = new KGLCMSEQ_ELM[_TT.get_t()];
	 

  occ->_end = _TT.get_clm_max(); 
  occ->_s = occ->_t = 0;

  _TT.set_perm(NULL);
  if ( _II.get_perm() && RANGE(0, _II.get_target(), _II.get_item_max()) ) _II.set_target ( _II.get_perm(_II.get_target()));
  
  if ( !(_TT.exist_sc()) ) {  	
  	_TT.calloc_sc(_TT.get_clms()+2);
  }
  
  _II.set_itemflag(_TT.get_sc());


  // make occurrence & reverse each transaction
  for(size_t ii=0 ; ii< _TT.get_t() ; ii++){ // _TTへ移動
  	for(size_t jj=0; jj < _TT.get_Tvt(ii)/2 ; jj++){
	  	_TT.swap_Tvv(ii,jj);
	  }
	}

  if ( _II.get_len_ub() >= INTHUGE ) _II.set_len_ub(_TT.get_row_max());
  if ( _II.get_gap_ub() >= INTHUGE ) _II.set_gap_ub( _TT.get_row_max());
  _II.set_total_weight( _TT.get_total_w_org());
	_II.set_rows_org(_TT.get_rows_org());
	_II.set_trperm(_TT.get_trperm());

  i=0;

	for( KGLCMSEQ_ELM * xx=_TT.beginOQvELE(_TT.get_clms()) ; xx<_TT.endOQvELE(_TT.get_clms()) ; xx++)
	{ 
    L._t = xx->_t;
    L._s = L._org = _TT.get_Tvt(xx->_t);   // !! org is originally -1
    occ->push_back(L);
  }

	// flag for shrink or not
  _dir = (_problem&LCMSEQ_LEFTMOST) && _II.get_len_ub()>=_TT.get_row_max() && !(_II.get_flag()&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT));   

	 // flag for removing infrequent item or not
  _root = _II.get_gap_ub()>=_TT.get_row_max() && _II.get_len_ub()>=_TT.get_row_max();  

  _th = _II.get_frq_lb();
}

/*************************************************************************/
/* main of LCMseq ver. 2 */
/*************************************************************************/
int KGLCMSEQ::run(int argc, char *argv[]){

  KGLCMSEQ_QUE occ;
  occ._v = NULL;
  
  setArgs (argc, argv);
	
	if ( _ERROR_MES ) return (1);

	
	_tFlag2 |= 
		TRSACT_MAKE_NEW + 
		TRSACT_ALLOC_OCC + 
		( (_iFlag & (ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ) ? 0 : (TRSACT_SHRINK+TRSACT_1ST_SHRINK));

  _w_lb =  
  	(((_iFlag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT)) && (_problem & PROBLEM_FREQSET)) || (_iFlag&ITEMSET_RULE) || _gap_ub<INTHUGE || _len_ub<INTHUGE )? -WEIGHTHUGE: _frq_lb;

	if(
		_TT.load(
			_tFlag,_tFlag2,
			_fname,_wfname,NULL,NULL,
			_w_lb,WEIGHTHUGE,true)
	){
		return 1;
	}
	
	_iFlag |= (ITEMSET_USE_ORG +ITEMSET_ITEMFRQ);

	_II.setParams(
		_iFlag,_frq_lb,_frq_ub,_lb,_ub,_target,
		_ratio_lb,_ratio_ub,_prob_lb,_prob_ub,
		_rposi_lb,_rposi_ub,_nega_lb,_nega_ub,
		_posi_lb,_posi_ub,
		_topk_k,_max_solutions,_separator,
		_TT.get_total_w_org(),_TT.get_total_pw_org(), // frq, pfrq
		_len_ub,_setrule_lb
	);

  if ( _ERROR_MES ){ return 1; }
  
  _init( &occ); 

  if( _ERROR_MES ){ return 1; }
  
  LCMseq(_TT.get_clms(), &occ);

  _II.last_output();

  
  free2 (occ._v);
  _TT.set_sc(NULL);

  return (_ERROR_MES?1:0);

}



