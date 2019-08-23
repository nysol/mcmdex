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
#define WEIGHT_DOUBLE

#include"itemset.hpp"
#include"queue.hpp"
#include"aheap.hpp"
#include"stdlib2.hpp"
#include"trsact.hpp"

/*
	Output information about ITEMSET structure.
 flag&1: print frequency constraint 
*/
void ITEMSET::print (int flag){

  if ( _params._lb>0 || _params._ub<INTHUGE ){
    if ( _params._lb > 0 ) fprintf(stderr,"%d <= ", _params._lb);
    fprintf(stderr,"itemsets ");
    if ( _params._ub < INTHUGE ) fprintf(stderr," <= %d\n", _params._ub);
    fprintf(stderr,"\n");
  }

  if ( flag&1 ){
    if ( _params._frq_lb > -WEIGHTHUGE ) fprintf(stderr,WEIGHTF" <=", _params._frq_lb);
    fprintf(stderr," frequency ");
    if ( _params._frq_ub < WEIGHTHUGE ) fprintf(stderr," <="WEIGHTF, _params._frq_ub);
    fprintf(stderr,"\n");
  }
}

/* second initialization
   topk.end>0 => initialize heap for topk mining */
/* all pointers will be set to 0, but not for */
/* if topK mining, set topk.end to "K" */
// _output_fname, perm, siz, 0
void ITEMSET::alloc(
	char *fname, PERM *perm, QUEUE_INT item_max,size_t item_max_org
){

  LONG i, ii;
  size_t siz = (_params._flag&ITEMSET_USE_ORG)?item_max_org+2: item_max+2;
  int j;

  _prob = _ratio = 1.0;
  _params._frq = 0; // <<これOK?
  _perm = perm;

  _itemset.alloc((QUEUE_ID)siz,(QUEUE_ID)siz);

  if ( _params._flag&ITEMSET_ADD ) _add.alloc((QUEUE_ID)siz);

	_sc = new LONG[siz+2](); // calloc2

	// upper bound of frequency
  if ( _params._flag  & ITEMSET_SC2 ) {
		_sc2 = new LONG[int(_params._frq_ub+2)]();  // calloc2 
  }

	// allocate topk heap
  if ( _params._topk_k > 0 ){  
    if (_params._flag & ITEMSET_SC2){
      _params._frq_lb = 1; 
      _topk_frq = 0;
      _sc2[_topk_frq] = _params._topk_k;
    } else {
    	_topk.alloc(_params._topk_k,-WEIGHTHUGE);
      _params._frq_lb = -WEIGHTHUGE ;
    }
  }

  if ( _params._flag&ITEMSET_SET_RULE ){

		_set_weight = new WEIGHT[siz]();

    if ( _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ){
			_set_occ    = new QUEUE*[siz]();
			_set_occELE = new KGLCMSEQ_QUE*[siz]();
    }
  }

  _iters = _solutions = 0; //_iters2 =
  _item_max = item_max;

  if ( fname ){
  	// バッファ確保しないほうがいい？
    if ( strcmp (fname, "-") == 0 ) _fp.open(stdout);
    else{
    	if(_params._flag&ITEMSET_APPEND){ _fp.openA(fname);}
    	else                    { _fp.open(fname);}
    }
  } 

  if ( _params._flag&ITEMSET_ITEMFRQ ){
	  _item_frq =  new WEIGHT[item_max+2];
	}
  if ( _params._flag&ITEMSET_RULE ){
    _itemflag = new char[item_max+2](); //calloc2
  }

  _total_weight = 1;

  return;
}

/*******************************************************************/
/* output at the termination of the algorithm */
/* print #of itemsets of size k, for each k */
/*******************************************************************/
void ITEMSET::last_output(){

  QUEUE_ID i;
  LONG n=0, nn=0;
  WEIGHT w;
  unsigned char c;

  _fp.flush_last();

  if ( !(_params._flag&SHOW_MESSAGE) ) return;  // "no message" is specified

  if ( _params._topk_k > 0 ){  // output value of the kth-best solution
		OFILE2 ofp(stdout);
    if ( _topk.end() ){
      i = _topk.findmin_head();
      ofp.print(_topk.H(i)); 
      ofp.print("\n");
    }
    else{ //_sc2を使ってる
    	ofp.print(LONGF, _topk_frq);
    	ofp.print("\n");
    }
    goto END;
  }

	for(i=0;i<_itemset.get_end()+1;i++){ //FLOOP
    n += _sc[i];
    if ( _sc[i] != 0 ) nn = i;
  }

  if ( n!=0 ){ //OK?
    printf (LONGF "\n", n);
		for(i=0;i<nn+1;i++){ //FLOOP
			printf (LONGF "\n", _sc[i]);
		}

  }
  
  END:;
  fprintf(stderr,"iters=" LONGF, _iters);
  fprintf(stderr,"\n");
  
  if (_params._flag & ITEMSET_SC2){ 
		// count by frequency
		for(i=0;i<_params._frq_ub+1;i++){
      if ( _sc2[i] != 0 ) printf(QUEUE_INTF "," LONGF "\n", i, _sc2[i]);
    }
  }
}

/* output frequency, coverage */
//void ITEMSET::output_frequency ( WEIGHT frq, WEIGHT pfrq, int core_id){
void ITEMSET::output_frequency ( WEIGHT frq, WEIGHT pfrq){

  OFILE2 *fp = &_fp;

  if ( _params._flag&(ITEMSET_FREQ+ITEMSET_PRE_FREQ) ){
    if ( _params._flag&ITEMSET_FREQ ) _fp.putch(' ');
    _fp.print_WEIGHT (frq, _params._digits, '(');
    _fp.putch( ')');
    if ( _params._flag&ITEMSET_PRE_FREQ ) _fp.putch(' ');
  }
  if ( _params._flag&ITEMSET_OUTPUT_POSINEGA ){ // output positive sum, negative sum in the occurrence
    _fp.putch(' ');
    _fp.print_WEIGHT( pfrq, _params._digits, '(');
    _fp.print_WEIGHT( pfrq-frq, _params._digits, ',');
    _fp.print_WEIGHT( pfrq/(2*pfrq-frq), _params._digits, ',');
    _fp.putch( ')');
  }
}

//for _trsact_h_
// QUEUE 用
void ITEMSET::output_occ(QUEUE *occ)
{
  QUEUE_INT *x;

  VEC_ID j, ee = _rows_org;

  int flag = _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT), flush_flag=0;

	for(x=occ->begin() ; x < occ->end() ; x++ ){

    if ( (_params._flag&ITEMSET_RM_DUP_TRSACT)==0 || *x != ee ){
      _fp.print_int ( _trperm ? _trperm[*x]: *x , _params._separator);
    }
    ee = *x;
    if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _fp.needFlush() ){
      flush_flag = 1;
      _fp.flush_();
    }
  }
  _fp.putch('\n');
  if ( flush_flag ){
    _fp.flush_ ();
  }
}

/* output an itemset to the output file */
// lcm
void ITEMSET::output_itemset_(
	QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, 
	QUEUE *occ
){

  QUEUE_ID i;
  QUEUE_INT e;
  int flush_flag = 0;

  _multi_outputs0++;
  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs0%(ITEMSET_INTERVAL) == 0) )
      fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions0, _multi_outputs0);


	if( _checkNotBound( itemset->get_t() , frq , pfrq ) ) return;

  _multi_solutions0++;

  if ( _params._max_solutions>0 && _multi_solutions0 > _params._max_solutions ){
    last_output ();
    // raiseする？
    fprintf(stderr, "reached to maximum number of solutions\n");
    EXIT;
  }

  _sc[itemset->get_t()]++;
  if (_params._flag & ITEMSET_SC2) _sc2[(QUEUE_INT)frq]++;  // histogram for LAMP

  if ( _params._topk_k > 0 ){
    if ( _topk.end() ){

      e = _topk.findmin_head();
      
      if ( frq > _topk.H(e) ){

        _topk.chg( e, frq );
        e = _topk.findmin_head ();
        _params._frq_lb =  _topk.H(e);

      }
    } else {  // histogram version
      if ( frq > _topk_frq ){

        _sc2[_topk_frq]--;
        while (_sc2[_topk_frq]==0) _topk_frq++;
        _params._frq_lb = _topk_frq+1;

      }
    }
    return;
  }
  
  if ( _params._flag&ITEMSET_PRE_FREQ ) output_frequency ( frq, pfrq );

  if ( (_params._flag & ITEMSET_NOT_ITEMSET) == 0 ){

		for(i=0;i<itemset->get_t();i++){
			e = itemset->get_v(i);
			_fp.print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);
			if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _fp.needFlush() ){
				flush_flag = 1;
        _fp.flush_ ();
      }
    }
  }
	
	if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( frq, pfrq);
	if ( ((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || (_params._flag&ITEMSET_FREQ) || (_params._flag&ITEMSET_PRE_FREQ) ){
		_fp.putch('\n');
	}

	// for _trsact_h_
	if (_params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT)) output_occ ( occ );

	if ( flush_flag ){ _fp.flush_ (); }
}




// for lcm
void ITEMSET::output_itemset(QUEUE *occ ){
  output_itemset_ ( &_itemset, _params._frq, _params._pfrq, occ );
}

/* output itemsets with adding all combination of "add"
   at the first call, i has to be "add->t" */
void ITEMSET::solution_iter (QUEUE *occ ){

  QUEUE_ID t = _add.get_t();
 	
  if ( _itemset.get_t() > _params._ub ) return;
  output_itemset( occ );

  for(;_add.get_t()>0;){

    _itemset.push_back(_add.tail());
    _add.dec_t();

    solution_iter ( occ );

    _itemset.dec_t();
  }
  _add.set_t(t);
}

void ITEMSET::solution (QUEUE *occ ){

  QUEUE_ID i;
  LONG s;

  if ( _itemset.get_t() > _params._ub ) return;
  if ( _params._flag & ITEMSET_ALL ){
    if ( _fp.exist() || _topk.end() ){
    	solution_iter ( occ );
    }
    else {

      s=1; 

			for(i=0;i<_add.get_t()+1;i++){ //FLOOP

        _sc[_itemset.get_t()+i] += s;
        s = s*(_add.get_t()-i)/(i+1);
      }
      if (_params._flag & ITEMSET_SC2){
        s = 1<< _add.get_t();
        _sc2[(QUEUE_INT)_params._frq] += s;  // histogram for LAMP
        if ( _params._topk_k > 0 && _params._frq > _topk_frq ){ // top-k histogram version
          while (1){
            if ( _sc2[_topk_frq] > s ){ _sc2[_topk_frq] -= s; break; }
            s -= _sc2[_topk_frq];
            _sc2[_topk_frq++] = 0; 
          }
          _params._frq_lb = _topk_frq+1;
        }
      }
    }
  } else {
	  for(i=0;i<_add.get_t();i++){
	    _itemset.push_back(_add.get_v(i));
    }
    output_itemset( occ );
    _itemset.minus_t(_add.get_t());
  }
}

/*************************************************************************/
/* ourput a rule */
/*************************************************************************/
void ITEMSET::output_rule ( QUEUE *occ, double p1, double p2, size_t item ){

  OFILE2 *fp = &_fp;
  if ( fp->exist() && !(_topk.end()) ){
    fp->print_real ( p1, _params._digits, '(');
    fp->print_real ( p2, _params._digits, ',');
    fp->putch( ')');
    fp->print_int ( _perm[item], _params._separator);
    fp->puts ( " <= ");
  }

  if ( _params._flag & ITEMSET_RULE ){
    if( _params._flag & ITEMSET_RULE_ADD ){ solution (occ ); }
    else { output_itemset ( occ ); }
  } 
  else {
  	solution ( occ );
  }
}
/*************************************************************************/
/* check all rules for a pair of itemset and item */
/*************************************************************************/
void ITEMSET::check_rule (WEIGHT *w, QUEUE *occ, size_t item ){
  double p = w[item]/_params._frq, pp, ff;

  if ( _itemflag[item]==1 ) return;
  if ( w[item] <= -WEIGHTHUGE ) p = 0;
  pp = p; 
  ff = _item_frq[item];
  if ( _params._flag & ITEMSET_RULE_SUPP ){ 
  	pp = w[item]; 
  	ff *= _total_weight; 
  }

  if ( _params._flag & (ITEMSET_RULE_FRQ+ITEMSET_RULE_INFRQ)){
    if ( (_params._flag & ITEMSET_RULE_FRQ) && p < _params._ratio_lb ) return;
    if ( (_params._flag & ITEMSET_RULE_INFRQ) && p > _params._ratio_ub ) return;
    output_rule ( occ, p, ff, item );
  } else if ( _params._flag & (ITEMSET_RULE_RFRQ+ITEMSET_RULE_RINFRQ) ){
    if ( (_params._flag & ITEMSET_RULE_RFRQ) && (1-p) > _params._ratio_lb * (1-_item_frq[item]) ) return;
    if ( (_params._flag & ITEMSET_RULE_RINFRQ) && p > _params._ratio_ub * _item_frq[item] ) return;
    output_rule( occ, pp, ff, item );
  }
}

/*************************************************************************/
/* check all rules for an itemset and all items */
/*************************************************************************/
void ITEMSET::check_all_rule ( WEIGHT *w, QUEUE *occ, QUEUE *jump, WEIGHT total ){

  QUEUE_ID i, t;
  QUEUE_INT e, f=0, *x;
  WEIGHT d = _params._frq/total;
  int flush_flag = 0;

    // checking out of range for itemset size and (posi/nega) frequency
  if ( _itemset.get_t()+_add.get_t() < _params._lb || _itemset.get_t()>_params._ub || (!(_params._flag&ITEMSET_ALL) && _itemset.get_t()+_add.get_t()>_params._ub)) return;
  if ( !(_params._flag&ITEMSET_IGNORE_BOUND) && (_params._frq < _params._frq_lb || _params._frq > _params._frq_ub) ) return;
  if ( !(_params._flag&ITEMSET_IGNORE_BOUND) && (_params._pfrq < _params._posi_lb || _params._pfrq > _params._posi_ub || (_params._frq - _params._pfrq) > _params._nega_ub || (_params._frq - _params._pfrq) < _params._nega_lb) ) return;

  if ( _params._flag&ITEMSET_SET_RULE ){  // itemset->itemset rule for sequence mining

	  for(i=0;i< _itemset.get_t()-1;i++){

      if ( _params._frq/_set_weight[i] >= _params._setrule_lb && _fp.exist() ){
        _sc[i]++;
        if ( _params._flag  & ITEMSET_SC2)     _sc2[(QUEUE_INT)_params._frq]++;  // histogram for LAMP

        if ( _params._flag  & ITEMSET_PRE_FREQ ) output_frequency ( _params._frq, _params._pfrq );

				for(t=0;t<_itemset.get_t();t++){

          _fp.print_int ( _itemset.get_v(t), t?_params._separator:0);
          if ( t == i ){
            _fp.putch( ' ');
            _fp.putch( '=');
            _fp.putch( '>');
          }
          if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _fp.needFlush() ){
            //SPIN_LOCK(_multi_core, _lock_output);
            flush_flag = 1;
            _fp.flush_ ();
          }
        }
        if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( _params._frq, _params._pfrq );
        _fp.putch( ' ');
        _fp.print_real ( _params._frq/_set_weight[i], _params._digits, '(');
        _fp.putch( ')');
        _fp.putch( '\n');

				//for _trsact_h_
        if ( _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ){
            output_occ ( _set_occ[i] );
        }

        if ( flush_flag ){ _fp.flush_ (); }
      }
    }
  }
  
  // constraint of relational frequency
  if ( ((_params._flag&ITEMSET_RFRQ)==0 || d >= _params._prob_lb * _prob ) 
      && ((_params._flag&ITEMSET_RINFRQ)==0 || d <= _prob * _params._prob_ub) )
  {
    if ( _params._flag&ITEMSET_RULE ){  //  rule mining routines

      if ( _itemset.get_t() == 0 ) return;

      if ( _params._target < _item_max ){

				for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
          if ( *x == _params._target ){ 
             check_rule ( w, occ, *x );   
             // if (_ERROR_MES) return;エラー処理考える
          }
        }
      } 
      else {
        if ( _params._flag & (ITEMSET_RULE_FRQ + ITEMSET_RULE_RFRQ) ){

          if ( _add.get_t()>0 ){
            f = _add.get_v(_add.get_t()-1); t = _add.get_t(); _add.dec_t();
					  for(i=0;i<t;i++){
              e = _add.get_v(i);
              _add.set_v(i,f);
              check_rule ( w, occ, e );    
              // if (_ERROR_MES) return; //エラー処理かんがえる
              _add.set_v(i,e);
            }
            _add.inc_t();
          }

					for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
            check_rule ( w, occ, *x );   
  	        // if (_ERROR_MES) return;			//エラー処理かんがえる
					}

        }
        else {
          if ( _params._flag & (ITEMSET_RULE_INFRQ + ITEMSET_RULE_RINFRQ) ){

					  for(i=0;i<_item_max;i++){ //FLOOP
              if ( _itemflag[i] != 1 ){
                check_rule (w, occ, i );
                //if (_ERROR_MES) return;//エラー処理かんがえる
              }
            }
          }
        }
      }
    } 
    else {  // usual mining (not rule mining)
      if ( _fp.exist() && (_params._flag&(ITEMSET_RFRQ+ITEMSET_RINFRQ))){
        _fp.print_real ( d, _params._digits, '[');
        _fp.print_real ( _prob, _params._digits, ',');
        _fp.putch( ']');
      }
      solution (occ );
    }
  }
}

// 以下　KGLCMSEQ_QUE
void ITEMSET::output_occ ( KGLCMSEQ_QUE *occ ){
  KGLCMSEQ_ELM *x;

  VEC_ID j, ee = _rows_org;

  int flag = _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT), flush_flag=0;

	for(x=occ->begin() ; x< occ->end() ;  x++){

    if ( (_params._flag&ITEMSET_RM_DUP_TRSACT)==0 || x->_t != ee ){
      _fp.print_int ( _trperm? _trperm[x->_t]: x->_t, _params._separator);
      if (flag == ITEMSET_MULTI_OCC_PRINT ){

        _fp.print_int ( x->_s, _params._separator);
        _fp.print_int ( x->_org, _params._separator);


      } else if ( flag == (ITEMSET_MULTI_OCC_PRINT+ITEMSET_TRSACT_ID) ){
         _fp.print_int ( x->_s, _params._separator);
      }
    }
    ee = x->_t;
    if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _fp.needFlush() ){
      flush_flag = 1;
      _fp.flush_();
    }
  }

  _fp.putch('\n');
  if ( flush_flag ){ _fp.flush_ ();}

}

/* output an itemset to the output file */
void ITEMSET::output_itemset_ (
	QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, KGLCMSEQ_QUE *occ
)
{
  QUEUE_ID i;
  QUEUE_INT e;

  int flush_flag = 0;
  
  _multi_outputs0++;
  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs0%(ITEMSET_INTERVAL) == 0) )
      fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions0, _multi_outputs0);
  if ( itemset->get_t() < _params._lb || itemset->get_t() > _params._ub ) return;
  if ( (_params._flag&ITEMSET_IGNORE_BOUND)==0 && (frq < _params._frq_lb || frq > _params._frq_ub) ) return;
  if ( (_params._flag&ITEMSET_IGNORE_BOUND)==0 && (pfrq < _params._posi_lb || pfrq >_params. _posi_ub || (frq - _params._pfrq) > _params._nega_ub || (frq - _params._pfrq) < _params._nega_lb) ) return;

  _multi_solutions0++;
  if ( _params._max_solutions>0 && _multi_solutions0 > _params._max_solutions ){
    last_output ();
    fprintf(stderr, "reached to maximum number of solutions\n");
    EXIT;
  }

  _sc[itemset->get_t()]++;

  if (_params._flag & ITEMSET_SC2) _sc2[(QUEUE_INT)frq]++;  // histogram for LAMP


  if ( _params._topk_k > 0 ){

    if ( _topk.end() ){

      e = _topk.findmin_head();

      if ( frq  > _topk.H(e) ){

        _topk.chg( e, frq );
        e = _topk.findmin_head ();
        _params._frq_lb = _topk.H(e);

      }
    } else {  // histogram version
      if ( frq > _topk_frq ){
        _sc2[_topk_frq]--;
        while (_sc2[_topk_frq]==0) _topk_frq++;
        _params._frq_lb = _topk_frq+1;
      }
    }
    return;
  }

	if ( _params._flag&ITEMSET_PRE_FREQ ) output_frequency ( frq, pfrq);

	if ( (_params._flag & ITEMSET_NOT_ITEMSET) == 0 ){

		for(i=0;i<itemset->get_t();i++){
			e = itemset->get_v(i);
			_fp.print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);
			if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _fp.needFlush() ){
          flush_flag = 1;
          _fp.flush_();
      }
    }
  }

	if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( frq, pfrq);
	
	if ( ((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || (_params._flag&ITEMSET_FREQ) || (_params._flag&ITEMSET_PRE_FREQ) ){
      _fp.putch('\n');
  }
	
	if (_params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT)) output_occ ( occ );
	
	if ( flush_flag ){ _fp.flush_ ();}
}


void ITEMSET::output_itemset ( KGLCMSEQ_QUE *occ){
  output_itemset_ ( &_itemset, _params._frq,_params._pfrq, occ);
}


/* output itemsets with adding all combination of "add"
   at the first call, i has to be "add->t" */
void ITEMSET::solution_iter (KGLCMSEQ_QUE *occ)
{

  QUEUE_ID t=_add.get_t();

  if ( _itemset.get_t() > _params._ub ) return;

  output_itemset ( occ );

  for(;_add.get_dec_t()>0;){

    _itemset.push_back(_add.pop_back());

    solution_iter ( occ );//エラー処理考える

    _itemset.dec_t();

  }

  _add.set_t(t);

}

void ITEMSET::solution (KGLCMSEQ_QUE *occ){

  QUEUE_ID i;
  LONG s;

  if ( _itemset.get_t() > _params._ub ) return;

  if ( _params._flag & ITEMSET_ALL ){
    if ( _fp.exist() || _topk.end() ){
    	solution_iter ( occ );
    }
    else {
      s=1; 

			for(i=0;i<_add.get_t()+1;i++){
        _sc[_itemset.get_t()+i] += s;
        s = s*(_add.get_t()-i)/(i+1);
      }

      if (_params._flag & ITEMSET_SC2){
        s = 1<< _add.get_t();
        _sc2[(QUEUE_INT)_params._frq] += s;  // histogram for LAMP
        if ( _params._topk_k > 0 && _params._frq > _topk_frq ){ // top-k histogram version
          while (1){
            if ( _sc2[_topk_frq] > s ){ _sc2[_topk_frq] -= s; break; }
            s -= _sc2[_topk_frq];
            _sc2[_topk_frq++] = 0; 
          }
          _params._frq_lb = _topk_frq+1;
        }
      }
    }
  } else {
    for(i=0;i<_add.get_t();i++) {
    	_itemset.push_back(_add.get_v(i));
    }
    output_itemset ( occ );
    _itemset.minus_t(_add.get_t());
  }
}

/*************************************************************************/
/* ourput a rule */
/*************************************************************************/
void ITEMSET::output_rule ( KGLCMSEQ_QUE *occ, double p1, double p2, size_t item){

  if ( _fp.exist() && !(_topk.end()) ){
    _fp.print_real ( p1, _params._digits, '(');
    _fp.print_real ( p2, _params._digits, ',');
    _fp.putch( ')');
    _fp.print_int ( _perm[item], _params._separator);
    _fp.puts ( " <= ");
  }
  if ( _params._flag & ITEMSET_RULE ){
    if ( _params._flag & ITEMSET_RULE_ADD ) solution (occ);
    else output_itemset ( occ );
  } else solution ( occ );

}

/*************************************************************************/
/* check all rules for a pair of itemset and item */
/*************************************************************************/
// lcmseq
void ITEMSET::check_rule (WEIGHT *w, KGLCMSEQ_QUE *occ, size_t item){

  double p = w[item]/_params._frq, pp, ff;
 	_prob = 1.0;
	for(QUEUE_INT *x=_itemset.begin(); x<_itemset.end(); x++){
		_prob *= _item_frq[*x];
	}
	for(QUEUE_INT *x=_add.begin();x<_add.end(); x++){
		_prob *= _item_frq[*x];
	}


  if ( _itemflag[item]==1 ) return;
  if ( w[item] <= -WEIGHTHUGE ) p = 0;
  pp = p; ff = _item_frq[item];
  if ( _params._flag & ITEMSET_RULE_SUPP ){ pp = w[item]; ff *= _total_weight; }

  if ( _params._flag & (ITEMSET_RULE_FRQ+ITEMSET_RULE_INFRQ)){
    if ( (_params._flag & ITEMSET_RULE_FRQ) && p < _params._ratio_lb ) return;
    if ( (_params._flag & ITEMSET_RULE_INFRQ) && p > _params._ratio_ub ) return;
    output_rule ( occ, p, ff, item);
  } else if ( _params._flag & (ITEMSET_RULE_RFRQ+ITEMSET_RULE_RINFRQ) ){
    if ( (_params._flag & ITEMSET_RULE_RFRQ) && (1-p) > _params._ratio_lb * (1-_item_frq[item]) ) return;
    if ( (_params._flag & ITEMSET_RULE_RINFRQ) && p > _params._ratio_ub * _item_frq[item] ) return;
    output_rule ( occ, pp, ff, item);
  }
}

/*************************************************************************/
/* check all rules for an itemset and all items */
/*************************************************************************/
// lcm_seq
void ITEMSET::check_all_rule ( WEIGHT *w, KGLCMSEQ_QUE *occ, QUEUE *jump, WEIGHT total){

  QUEUE_ID i, t;
  QUEUE_INT e, f=0, *x;
  WEIGHT d = _params._frq/total;
  int flush_flag = 0;

    // checking out of range for itemset size and (posi/nega) frequency
  if ( _itemset.get_t()+_add.get_t() < _params._lb || _itemset.get_t()>_params._ub || (!(_params._flag&ITEMSET_ALL) && _itemset.get_t()+_add.get_t()>_params._ub)) return;
  if ( !(_params._flag&ITEMSET_IGNORE_BOUND) && (_params._frq < _params._frq_lb || _params._frq > _params._frq_ub) ) return;
  if ( !(_params._flag&ITEMSET_IGNORE_BOUND) && (_params._pfrq < _params._posi_lb || _params._pfrq > _params._posi_ub || (_params._frq - _params._pfrq) > _params._nega_ub || (_params._frq - _params._pfrq) < _params._nega_lb) ) return;

  if ( _params._flag&ITEMSET_SET_RULE ){  // itemset->itemset rule for sequence mining

    for(i=0;i<_itemset.get_t()-1;i++) {

      if ( _params._frq/_set_weight[i] >= _params._setrule_lb && _fp.exist() ){
        _sc[i]++;
        if ( _params._flag  & ITEMSET_SC2)     _sc2[(QUEUE_INT)_params._frq]++;  // histogram for LAMP
        if ( _params._flag  & ITEMSET_PRE_FREQ ) output_frequency ( _params._frq, _params._pfrq);

		    for(t=0;t<_itemset.get_t();t++) {

          _fp.print_int ( _itemset.get_v(t), t?_params._separator:0);
          if ( t == i ){
            _fp.putch(' ');
            _fp.putch('=');
            _fp.putch('>');
          }
          if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _fp.needFlush() ){
            //SPIN_LOCK(_multi_core, _lock_output);
            flush_flag = 1;
            _fp.flush_ ();
          }
        }
        if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( _params._frq, _params._pfrq);
        _fp.putch( ' ');
        _fp.print_real ( _params._frq/_set_weight[i], _params._digits, '(');
        _fp.putch( ')');
        _fp.putch( '\n');

			//for _trsact_h_
        if ( _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ){
            output_occ ( _set_occELE[i]);
        }

        if ( flush_flag ){
          _fp.flush_ ();
        }
      }
    }
  }
    // constraint of relational frequency
  if ( ((_params._flag&ITEMSET_RFRQ)==0 || d >= _params._prob_lb * _prob ) 
      && ((_params._flag&ITEMSET_RINFRQ)==0 || d <= _prob * _params._prob_ub) ){

		//  rule mining routines
    if ( _params._flag&ITEMSET_RULE ){  

      if ( _itemset.get_t() == 0 ) return;

      if ( _params._target < _item_max ){

				for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
          if ( *x == _params._target ){ 
             check_rule ( w, occ, *x);  
          }
        }
      }
      else {

        if ( _params._flag & (ITEMSET_RULE_FRQ + ITEMSET_RULE_RFRQ) ){

          if ( _add.get_t()>0 ){

            f = _add.get_v(_add.get_t()-1); 
            t = _add.get_t(); 
            _add.dec_t();

						for(i=0;i<t;i++){

              e = _add.get_v(i);
              _add.set_v(i,f);
              check_rule ( w, occ, e);    //エラー処理考える
              _add.set_v(i,e);

            }
            _add.inc_t();
          }

					for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
            check_rule ( w, occ, *x); //エラー処理考える   
					}

        } else {
          if ( _params._flag & (ITEMSET_RULE_INFRQ + ITEMSET_RULE_RINFRQ) ){

						for(i=0;i<_item_max;i++){
              if ( _itemflag[i] != 1 ){
                check_rule (w, occ, i); //エラー処理考える    
              }
            }
          }
        }
      }
    }
    else {  // usual mining (not rule mining)

      if ( _fp.exist() && (_params._flag&(ITEMSET_RFRQ+ITEMSET_RINFRQ))){
        _fp.print_real ( d, _params._digits, '[');
        _fp.print_real ( _prob, _params._digits, ',');
        _fp.putch( ']');
      }
      solution (occ);

    }
  }

}

      //_params._frq_lb = -WEIGHTHUGE * _topk_sign;
      //ofp.print(_topk.H(i)*_topk_sign); 
//if ( frq * _topk_sign > _topk.H(e) ){

        //_topk.chg( e, frq * _topk_sign);
        //_params._frq_lb =  _topk_sign* _topk.H(e);
              //if ( frq * _topk_sign > _topk.H(e) ){
        //_topk.chg( e, frq * _topk_sign);
        //_params._frq_lb = _topk.H(e) * _topk_sign;

