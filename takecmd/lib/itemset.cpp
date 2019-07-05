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
void ITEMSET::alloc (char *fname, PERM *perm, QUEUE_INT item_max,size_t item_max_org){

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
      _params._frq_lb = -WEIGHTHUGE * _topk_sign;
    }
  }

	// allocate topkheap for each element
  if ( _params._itemtopk_end > 0 ){ 

    _itemtopk = new AHEAP[_params._itemtopk_end];
    if ( _params._itemtopk_item2 > 0 ){
			_itemtopk_ary = new QUEUE_INT*[_params._itemtopk_end]; //calloc2
    }

    for(LONG i = 0 ; i<_params._itemtopk_end ; i++){

      if ( _params._itemtopk_item2 > 0 ){
        _itemtopk_ary[i] = new QUEUE_INT [_params._itemtopk_item]; //calloc2
			}
    	_itemtopk[i].alloc(_params._itemtopk_item,-WEIGHTHUGE);

    }
  }
  
  if ( _params._flag&ITEMSET_SET_RULE ){

		_set_weight = new WEIGHT[siz](); //calloc2

    if ( _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ){

        _set_occ    = new QUEUE*[siz](); //calloc2
        _set_occELE = new KGLCMSEQ_QUE*[siz]();//calloc2
    }
  }

  _iters = _solutions = 0; //_iters2 =
  _item_max = item_max;

  if ( fname ){
  	// バッファ確保しないほうがいい？
    if ( strcmp (fname, "-") == 0 ) _fp.open(stdout);
    else{
    	if(_params._flag&ITEMSET_APPEND){ _fp.open(fname,"a");}
    	else                    { _fp.open(fname,"w");}
    }
  } 

  if ( _params._flag&ITEMSET_ITEMFRQ ){
	  _item_frq =  new WEIGHT[item_max+2];
	}
  if ( _params._flag&ITEMSET_RULE ){
    _itemflag = new char[item_max+2](); //calloc2
  }

  _total_weight = 1;
  j = MAX(_params._multi_core,1);

  _multi_iters = new LONG[j*3](); // calloc2
  _multi_outputs = _multi_iters + j;
  _multi_solutions = _multi_outputs + j;
  
  _multi_fp = FILE2::makeMultiFp(j,_fp);
  

#ifdef MULTI_CORE
  if ( _params._multi_core > 0 ){
    pthread_spin_init (_lock_counter, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init (_lock_sc, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init (_lock_output, PTHREAD_PROCESS_PRIVATE);
  }
#endif

  return;
  ERR:;

  exit(1);
}



/* sum the counters computed by each thread */
void ITEMSET::merge_counters (){

	for(size_t i=0 ; i<MAX(_params._multi_core,1) ; i++){ // FLOOP 

    _iters += _multi_iters[i];
    _solutions += _multi_solutions[i];
    if ( _multi_fp[i].exist_buf() ) _multi_fp[i].flush_last();
  }
  
	for(size_t i =0 ;i<MAX(_params._multi_core,1)*3 ;i++){ 
		_multi_iters[i] = 0; 
	}
}

/*******************************************************************/
/* output at the termination of the algorithm */
/* print #of itemsets of size k, for each k */
/*******************************************************************/
void ITEMSET::last_output (){

  QUEUE_ID i;
  LONG n=0, nn=0;
  WEIGHT w;
  unsigned char c;

  FILE2 *fp = &_multi_fp[0];

  merge_counters();

  if ( !(_params._flag&SHOW_MESSAGE) ) return;  // "no message" is specified


  if ( _params._itemtopk_end > 0 ){  // output values of the kth-best solution for each item

		for(n=0;n<_params._itemtopk_end;n++){

      c = 0;
			for(nn=0;nn<_itemtopk[n].end();nn++){ //FLOOP

        i = _itemtopk[n].findmin_head();
        w = _itemtopk[n].H(i);

        if ( w == -WEIGHTHUGE*_itemtopk_sign ) break;

        if ( _params._flag & ITEMSET_PRE_FREQ ){ fp->print_real ( w, 8, c); c = _params._separator; }
        fp->print_int (  _perm? _perm[_itemtopk_ary[n][i]]: _itemtopk_ary[n][i], c);
        c = _params._separator;
        if ( _params._flag & ITEMSET_FREQ ){ 
        	fp->print_real ( w, 8, c); 
        	c = _params._separator; 
        }
				_itemtopk[n].chg(i, WEIGHTHUGE);

      }
      fp->putch('\n');
      fp->flush();
    }
    fp->flush_last ();
    goto END;
  }

  if ( _params._topk_k > 0 ){  // output value of the kth-best solution

		OFILE2 ofp(stdout);
    if ( _topk.end() ){
      i = _topk.findmin_head();
      ofp.print(_topk.H(i)*_topk_sign); //fprint_WEIGHT
      ofp.print("\n");
    }
    else{
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
void ITEMSET::output_frequency ( WEIGHT frq, WEIGHT pfrq, int core_id){

  FILE2 *fp = &_multi_fp[core_id];

  if ( _params._flag&(ITEMSET_FREQ+ITEMSET_PRE_FREQ) ){
    if ( _params._flag&ITEMSET_FREQ ) fp->putch(' ');
    fp->print_WEIGHT (frq, _params._digits, '(');
    fp->putch( ')');
    if ( _params._flag&ITEMSET_PRE_FREQ ) fp->putch(' ');
  }

  if ( _params._flag&ITEMSET_OUTPUT_POSINEGA ){ // output positive sum, negative sum in the occurrence
    fp->putch(' ');
    fp->print_WEIGHT( pfrq, _params._digits, '(');
    fp->print_WEIGHT( pfrq-frq, _params._digits, ',');
    fp->print_WEIGHT( pfrq/(2*pfrq-frq), _params._digits, ',');
    fp->putch( ')');
  }
}

//for _trsact_h_
// QUEUE 用
void ITEMSET::output_occ ( QUEUE *occ, int core_id)
{
  QUEUE_INT *x;
  FILE2 *fp = &_multi_fp[core_id];

  VEC_ID j, ee = _rows_org;

  int flag = _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT), flush_flag=0;


	for(x=occ->begin() ; x < occ->end() ; x++ ){


    if ( (_params._flag&ITEMSET_RM_DUP_TRSACT)==0 || *x != ee ){
      fp->print_int ( _trperm ? _trperm[*x]: *x , _params._separator);
    }
    ee = *x;
    if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
      SPIN_LOCK(_multi_core, _lock_output);
      flush_flag = 1;
      fp->flush_();
    }
  }
  fp->putch('\n');
  if ( flush_flag ){
    fp->flush_ ();
    SPIN_UNLOCK(_multi_core, _lock_output);
  }
}

/* output an itemset to the output file */
void ITEMSET::output_itemset_(
	QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, 
	QUEUE *occ, 
	QUEUE_INT itemtopk_item, QUEUE_INT itemtopk_item2, 
	int core_id
){

  QUEUE_ID i;
  QUEUE_INT e;
  int flush_flag = 0;
  FILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;
  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) )
      fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions[core_id], _multi_outputs[core_id]);


	if( _checkNotBound( itemset->get_t() , frq , pfrq ) ) return;

  _multi_solutions[core_id]++;
  if ( _params._max_solutions>0 && _multi_solutions[core_id] > _params._max_solutions ){
    last_output ();
    // raiseする？
    fprintf(stderr, "reached to maximum number of solutions\n");
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);
  _sc[itemset->get_t()]++;
  if (_params._flag & ITEMSET_SC2) _sc2[(QUEUE_INT)frq]++;  // histogram for LAMP
  SPIN_UNLOCK(_multi_core, _lock_sc);


  if ( _params._itemtopk_end > 0 ){

    e = _itemtopk[itemtopk_item].findmin_head();

    if ( frq*_itemtopk_sign > _itemtopk[itemtopk_item].H(e) ){
      SPIN_LOCK(_multi_core, _lock_sc);
      _itemtopk[itemtopk_item].chg(e, frq * _itemtopk_sign);
      if ( _itemtopk_ary ) _itemtopk_ary[itemtopk_item][e] = itemtopk_item2;
      SPIN_UNLOCK(_multi_core, _lock_sc);
    }
    return;
  }

  if ( _params._topk_k > 0 ){
    if ( _topk.end() ){

      e = _topk.findmin_head();
      if ( frq * _topk_sign > _topk.H(e) ){

        SPIN_LOCK(_multi_core, _lock_sc);
        _topk.chg( e, frq * _topk_sign);
        e = _topk.findmin_head ();

        _params._frq_lb =  _topk_sign* _topk.H(e);

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    } else {  // histogram version
      if ( frq > _topk_frq ){

        SPIN_LOCK(_multi_core, _lock_sc);

        _sc2[_topk_frq]--;
        while (_sc2[_topk_frq]==0) _topk_frq++;
        _params._frq_lb = _topk_frq+1;

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    }
    return;
  }
  
  if ( fp ){
    if ( _params._flag&ITEMSET_PRE_FREQ ) output_frequency ( frq, pfrq, core_id);
    if ( (_params._flag & ITEMSET_NOT_ITEMSET) == 0 ){

			for(i=0;i<itemset->get_t();i++){
        e = itemset->get_v(i);
        fp->print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);
        if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
          SPIN_LOCK(_multi_core, _lock_output);
          flush_flag = 1;
          fp->flush_ ();
        }
      }
    }
    if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( frq, pfrq, core_id);
    if ( ((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || (_params._flag&ITEMSET_FREQ) || (_params._flag&ITEMSET_PRE_FREQ) ){
      fp->putch('\n');
    }
		// for _trsact_h_
    if (_params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT)) output_occ ( occ, core_id);

    if ( flush_flag ){
      fp->flush_ ();
      SPIN_UNLOCK(_multi_core, _lock_output);
    }
  }
}

void ITEMSET::output_itemset_(
	QUEUE *itemset, 
	WEIGHT frq, WEIGHT pfrq, 
	QUEUE *occ, 
	int core_id
){
  QUEUE_ID i;
  QUEUE_INT e;
  int flush_flag = 0;

  FILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;


  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) )
      fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions[core_id], _multi_outputs[core_id]);

	if( _checkNotBound( itemset->get_t() , frq , pfrq ) ) return;


  _multi_solutions[core_id]++;
  if ( _params._max_solutions>0 && _multi_solutions[core_id] > _params._max_solutions ){
    last_output ();
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);

  _sc[itemset->get_t()]++;
  if (_params._flag & ITEMSET_SC2) _sc2[(QUEUE_INT)frq]++;  // histogram for LAMP

  SPIN_UNLOCK(_multi_core, _lock_sc);

  if ( _params._topk_k > 0 ){

    if ( _topk.end() ){

      e = _topk.findmin_head();
      if ( frq * _topk_sign > _topk.H(e) ){

        SPIN_LOCK(_multi_core, _lock_sc);
        _topk.chg( e, frq * _topk_sign);
        e = _topk.findmin_head ();

        _params._frq_lb =  _topk_sign* _topk.H(e);

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    }
    else {  // histogram version
      if ( frq > _topk_frq ){

        SPIN_LOCK(_multi_core, _lock_sc);

        _sc2[_topk_frq]--;
        while (_sc2[_topk_frq]==0) _topk_frq++;
        _params._frq_lb = _topk_frq+1;

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    }
    return;
  }
  
  if ( fp ){
    if ( _params._flag&ITEMSET_PRE_FREQ ) output_frequency ( frq, pfrq, core_id);
    if ( (_params._flag & ITEMSET_NOT_ITEMSET) == 0 ){

			for(i=0;i<itemset->get_t();i++){
        e = itemset->get_v(i);
        fp->print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);
        if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
          SPIN_LOCK(_multi_core, _lock_output);
          flush_flag = 1;
          fp->flush_ ();
        }
      }
    }
    if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( frq, pfrq, core_id);
    if ( ((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || (_params._flag&ITEMSET_FREQ) || (_params._flag&ITEMSET_PRE_FREQ) ){
      fp->putch('\n');
    }
		// for _trsact_h_
    if (_params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT)) output_occ ( occ, core_id);

    if ( flush_flag ){
      fp->flush_ ();
      SPIN_UNLOCK(_multi_core, _lock_output);
    }
  }


}

/* output an itemset to the output file for sspc */
void ITEMSET::output_itemset_ (
	QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, 
	QUEUE_INT itemtopk_item, QUEUE_INT itemtopk_item2, 
	int core_id)
{

  QUEUE_ID i;
  QUEUE_INT e;

  int flush_flag = 0;
  FILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;

  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) ){
      fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions[core_id], _multi_outputs[core_id]);
  }

	if( _checkNotBound( itemset->get_t() , frq , pfrq ) ) return;

  _multi_solutions[core_id]++;

  if ( _params._max_solutions > 0 && _multi_solutions[core_id] > _params._max_solutions ){
    last_output ();
    // raiseする？
    fprintf(stderr,"reached to maximum number of solutions\n");
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);

  _sc[itemset->get_t()]++;

  SPIN_UNLOCK(_multi_core, _lock_sc);


  if ( _params._itemtopk_end > 0 ){

    e = _itemtopk[itemtopk_item].findmin_head();

    if ( frq*_itemtopk_sign > _itemtopk[itemtopk_item].H(e) ){

      SPIN_LOCK(_multi_core, _lock_sc);
      _itemtopk[itemtopk_item].chg(e, frq * _itemtopk_sign);
      if ( _itemtopk_ary ) _itemtopk_ary[itemtopk_item][e] = itemtopk_item2;
      SPIN_UNLOCK(_multi_core, _lock_sc);

    }
    return;
  }

  if ( _params._topk_k > 0 ){

    if ( _topk.end() ){

      e = _topk.findmin_head();
      if ( frq * _topk_sign > _topk.H(e) ){

        SPIN_LOCK(_multi_core, _lock_sc);
        _topk.chg( e, frq * _topk_sign);
        e = _topk.findmin_head ();
        _params._frq_lb = _topk.H(e) * _topk_sign;

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }

    } else {  
    	// histogram version
      if ( frq > _topk_frq ){

        SPIN_LOCK(_multi_core, _lock_sc);

        _sc2[_topk_frq]--;
        while (_sc2[_topk_frq]==0) _topk_frq++;
        _params._frq_lb = _topk_frq+1;

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    }
    return;
  }
  
  if ( fp ){

    if ( _params._flag&ITEMSET_PRE_FREQ ) output_frequency ( frq, pfrq, core_id);

    if ( (_params._flag & ITEMSET_NOT_ITEMSET) == 0 ){

			for(i=0;i<itemset->get_t();i++){ //FLOOP

        e = itemset->get_v(i);
        fp->print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);

        if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
          SPIN_LOCK(_multi_core, _lock_output);
          flush_flag = 1;
          fp->flush_ ();
        }

      }
    }

    if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( frq, pfrq, core_id);

    if ( ((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || 
    			(_params._flag & ITEMSET_FREQ) || 
    			(_params._flag & ITEMSET_PRE_FREQ) )
    {
      fp->putch('\n');
    }

    if ( flush_flag ){
      fp->flush_ ();
      SPIN_UNLOCK(_multi_core, _lock_output);
    }
  }
}

// for Mace 
void ITEMSET::output_itemset(int core_id){

  QUEUE_ID i;
  QUEUE_INT e;

  int flush_flag = 0;
  FILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;

  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) ){
      fprintf(stderr,
      			"---- " LONGF " solutions in " LONGF " candidates\n",
            _multi_solutions[core_id], _multi_outputs[core_id]);
  }

  if ( _itemset.get_t() < _params._lb || _itemset.get_t() > _params._ub ) return;

  _multi_solutions[core_id]++;

  if ( _params._max_solutions > 0 && _multi_solutions[core_id] > _params._max_solutions ){
    last_output ();
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);

  _sc[_itemset.get_t()]++;

  SPIN_UNLOCK(_multi_core, _lock_sc);

  if ( fp ){

		for(i=0;i<_itemset.get_t();i++){//FLOOP
      e = _itemset.get_v(i);
      fp->print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);
      if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
        SPIN_LOCK(_multi_core, _lock_output);
        flush_flag = 1;
      	fp->flush_ ();
      }
    }
    fp->putch('\n');

    if ( flush_flag ){
      fp->flush_ ();
      SPIN_UNLOCK(_multi_core, _lock_output);
    }
  }
}

void ITEMSET::output_itemset(QUEUE *occ, int core_id){
  output_itemset_ ( &_itemset, _params._frq, _params._pfrq, occ, _params._itemtopk_item, _params._itemtopk_item2, core_id);
}

/* output itemsets with adding all combination of "add"
   at the first call, i has to be "add->t" */
void ITEMSET::solution_iter (QUEUE *occ, int core_id){

  QUEUE_ID t = _add.get_t();
 	
  if ( _itemset.get_t() > _params._ub ) return;
  output_itemset( occ, core_id);

	// if ( _ERROR_MES ) return; エラー処理考える
	
  for(;_add.get_t()>0;){

    _itemset.push_back(_add.tail());
    _add.dec_t();

    solution_iter ( occ, core_id);

		// if ( _ERROR_MES ) return; エラー処理考える
    _itemset.dec_t();
  }
  _add.set_t(t);
}

void ITEMSET::solution (QUEUE *occ, int core_id){
  QUEUE_ID i;
  LONG s;

  if ( _itemset.get_t() > _params._ub ) return;
  if ( _params._flag & ITEMSET_ALL ){
    if ( _fp.exist() || _topk.end() ){
    	solution_iter ( occ, core_id);
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
    output_itemset( occ, core_id);
    _itemset.minus_t(_add.get_t());
  }
}

/*************************************************************************/
/* ourput a rule */
/*************************************************************************/
void ITEMSET::output_rule ( QUEUE *occ, double p1, double p2, size_t item, int core_id){

  FILE2 *fp = &_multi_fp[core_id];
  if ( fp->exist() && !(_topk.end()) ){
    fp->print_real ( p1, _params._digits, '(');
    fp->print_real ( p2, _params._digits, ',');
    fp->putch( ')');
    fp->print_int ( _perm[item], _params._separator);
    fp->puts ( " <= ");
  }

  if ( _params._flag & ITEMSET_RULE ){
    if( _params._flag & ITEMSET_RULE_ADD ){ solution (occ, core_id); }
    else { output_itemset ( occ, core_id); }
  } 
  else {
  	solution ( occ, core_id);
  }
}
/*************************************************************************/
/* check all rules for a pair of itemset and item */
/*************************************************************************/
void ITEMSET::check_rule (WEIGHT *w, QUEUE *occ, size_t item, int core_id){
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
    output_rule ( occ, p, ff, item, core_id);
  } else if ( _params._flag & (ITEMSET_RULE_RFRQ+ITEMSET_RULE_RINFRQ) ){
    if ( (_params._flag & ITEMSET_RULE_RFRQ) && (1-p) > _params._ratio_lb * (1-_item_frq[item]) ) return;
    if ( (_params._flag & ITEMSET_RULE_RINFRQ) && p > _params._ratio_ub * _item_frq[item] ) return;
    output_rule( occ, pp, ff, item, core_id);
  }
}

/*************************************************************************/
/* check all rules for an itemset and all items */
/*************************************************************************/
void ITEMSET::check_all_rule ( WEIGHT *w, QUEUE *occ, QUEUE *jump, WEIGHT total, int core_id){

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

        if ( _params._flag  & ITEMSET_PRE_FREQ ) output_frequency ( _params._frq, _params._pfrq, core_id);

				for(t=0;t<_itemset.get_t();t++){

          _multi_fp[core_id].print_int ( _itemset.get_v(t), t?_params._separator:0);
          if ( t == i ){
            _multi_fp[core_id].putch( ' ');
            _multi_fp[core_id].putch( '=');
            _multi_fp[core_id].putch( '>');
          }
          if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _multi_fp[core_id].needFlush() ){
            SPIN_LOCK(_multi_core, _lock_output);
            flush_flag = 1;
            _multi_fp[core_id].flush_ ();
          }
        }
        if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( _params._frq, _params._pfrq, core_id);
        _multi_fp[core_id].putch( ' ');
        _multi_fp[core_id].print_real ( _params._frq/_set_weight[i], _params._digits, '(');
        _multi_fp[core_id].putch( ')');
        _multi_fp[core_id].putch( '\n');

				//for _trsact_h_
        if ( _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ){
            output_occ ( _set_occ[i], core_id);
        }

        if ( flush_flag ){
          _multi_fp[core_id].flush_ ();
          SPIN_UNLOCK(_multi_core, _lock_output);
        }
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
             check_rule ( w, occ, *x, core_id);   
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
              check_rule ( w, occ, e, core_id);    
              // if (_ERROR_MES) return; //エラー処理かんがえる
              _add.set_v(i,e);
            }
            _add.inc_t();
          }

					for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
            check_rule ( w, occ, *x, core_id);   
  	        // if (_ERROR_MES) return;			//エラー処理かんがえる
					}

        }
        else {
          if ( _params._flag & (ITEMSET_RULE_INFRQ + ITEMSET_RULE_RINFRQ) ){

					  for(i=0;i<_item_max;i++){ //FLOOP
              if ( _itemflag[i] != 1 ){
                check_rule (w, occ, i, core_id);
                //if (_ERROR_MES) return;//エラー処理かんがえる
              }
            }
          }
        }
      }
    } 
    else {  // usual mining (not rule mining)
      if ( _fp.exist() && (_params._flag&(ITEMSET_RFRQ+ITEMSET_RINFRQ))){
        _multi_fp[core_id].print_real ( d, _params._digits, '[');
        _multi_fp[core_id].print_real ( _prob, _params._digits, ',');
        _multi_fp[core_id].putch( ']');
      }
      solution (occ, core_id);
    }
  }
}



// 以下　KGLCMSEQ_QUE

//for _trsact_h_
void ITEMSET::output_occ ( KGLCMSEQ_QUE *occ, int core_id){
  KGLCMSEQ_ELM *x;
  FILE2 *fp = &_multi_fp[core_id];

  VEC_ID j, ee = _rows_org;

  int flag = _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT), flush_flag=0;

  //MQUE_FLOOP__CLS (*occ, x, TT->get_occ_unit()){
	for(x=occ->begin() ; x< occ->end() ;  x++){

    if ( (_params._flag&ITEMSET_RM_DUP_TRSACT)==0 || x->_t != ee ){
      fp->print_int ( _trperm? _trperm[x->_t]: x->_t, _params._separator);
      if (flag == ITEMSET_MULTI_OCC_PRINT ){
        //FLOOP (j, 1, (VEC_ID)(TT->get_occ_unit()/sizeof(QUEUE_INT)))
        //    fp->print_int ( *(x+j), _separator);

            fp->print_int ( x->_s, _params._separator);
            fp->print_int ( x->_org, _params._separator);


      } else if ( flag == (ITEMSET_MULTI_OCC_PRINT+ITEMSET_TRSACT_ID) ){
         //fp->print_int ( *(x+1), _separator);
         fp->print_int ( x->_s, _params._separator);
      }
    }
    ee = x->_t;
    if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
      SPIN_LOCK(_params._multi_core, _lock_output);
      flush_flag = 1;
      fp->flush_();
    }
  }

  fp->putch('\n');
  if ( flush_flag ){
    fp->flush_ ();
    SPIN_UNLOCK(_multi_core, _lock_output);
  }

}

/* output an itemset to the output file */
void ITEMSET::output_itemset_ (QUEUE *itemset, WEIGHT frq, WEIGHT pfrq, KGLCMSEQ_QUE *occ, QUEUE_INT itemtopk_item, QUEUE_INT itemtopk_item2, int core_id){
  QUEUE_ID i;
  QUEUE_INT e;

  int flush_flag = 0;
  FILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;
  if ( (_params._flag&SHOW_PROGRESS ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) )
      fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions[core_id], _multi_outputs[core_id]);
  if ( itemset->get_t() < _params._lb || itemset->get_t() > _params._ub ) return;
  if ( (_params._flag&ITEMSET_IGNORE_BOUND)==0 && (frq < _params._frq_lb || frq > _params._frq_ub) ) return;
  if ( (_params._flag&ITEMSET_IGNORE_BOUND)==0 && (pfrq < _params._posi_lb || pfrq >_params. _posi_ub || (frq - _params._pfrq) > _params._nega_ub || (frq - _params._pfrq) < _params._nega_lb) ) return;

  _multi_solutions[core_id]++;
  if ( _params._max_solutions>0 && _multi_solutions[core_id] > _params._max_solutions ){
    last_output ();
    fprintf(stderr, "reached to maximum number of solutions\n");
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);
  _sc[itemset->get_t()]++;
  if (_params._flag & ITEMSET_SC2) _sc2[(QUEUE_INT)frq]++;  // histogram for LAMP
  SPIN_UNLOCK(_multi_core, _lock_sc);

  if ( _params._itemtopk_end > 0 ){

    e = _itemtopk[itemtopk_item].findmin_head();

    if ( frq*_itemtopk_sign > _itemtopk[itemtopk_item].H(e) ){
      SPIN_LOCK(_multi_core, _lock_sc);
      _itemtopk[itemtopk_item].chg(e, frq * _itemtopk_sign);
      if ( _itemtopk_ary ) _itemtopk_ary[itemtopk_item][e] = itemtopk_item2;
      SPIN_UNLOCK(_multi_core, _lock_sc);
    }
    return;
  }

  if ( _params._topk_k > 0 ){
    if ( _topk.end() ){

      e = _topk.findmin_head();
      if ( frq * _topk_sign > _topk.H(e) ){

        SPIN_LOCK(_multi_core, _lock_sc);
        _topk.chg( e, frq * _topk_sign);

        e = _topk.findmin_head ();
        _params._frq_lb = _topk.H(e) * _topk_sign;

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    } else {  // histogram version
      if ( frq > _topk_frq ){

        SPIN_LOCK(_multi_core, _lock_sc);

        _sc2[_topk_frq]--;
        while (_sc2[_topk_frq]==0) _topk_frq++;
        _params._frq_lb = _topk_frq+1;

        SPIN_UNLOCK(_multi_core, _lock_sc);

      }
    }
    return;
  }
  
  if ( fp ){

    if ( _params._flag&ITEMSET_PRE_FREQ ) output_frequency ( frq, pfrq, core_id);

    if ( (_params._flag & ITEMSET_NOT_ITEMSET) == 0 ){

      for(i=0;i<itemset->get_t();i++){
        e = itemset->get_v(i);
        fp->print_int ( _perm? _perm[e]: e, i==0? 0: _params._separator);
        if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || fp->needFlush() ){
          SPIN_LOCK(_multi_core, _lock_output);
          flush_flag = 1;
          fp->flush_();
        }
      }
    }
    if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( frq, pfrq, core_id);
    if ( ((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || (_params._flag&ITEMSET_FREQ) || (_params._flag&ITEMSET_PRE_FREQ) ){
      fp->putch('\n');
    }
		// for _trsact_h_
    if (_params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT)) output_occ ( occ, core_id);

    if ( flush_flag ){
      fp->flush_ ();
      SPIN_UNLOCK(_multi_core, _lock_output);
    }
  }
}


void ITEMSET::output_itemset ( KGLCMSEQ_QUE *occ, int core_id){
  output_itemset_ ( &_itemset, _params._frq,_params._pfrq, occ, _params._itemtopk_item, _params._itemtopk_item2, core_id);
}


/* output itemsets with adding all combination of "add"
   at the first call, i has to be "add->t" */
void ITEMSET::solution_iter (KGLCMSEQ_QUE *occ, int core_id){
  QUEUE_ID t=_add.get_t();
  if ( _itemset.get_t() > _params._ub ) return;
  output_itemset ( occ, core_id);

	// if ( _ERROR_MES ) return;//エラー処理考える
	
  //BLOOP (_add._t, _add._t, 0){
  for(;_add.get_dec_t()>0;){

    _itemset.push_back(_add.pop_back());

    solution_iter ( occ, core_id);

		//if ( _ERROR_MES ) return;//エラー処理考える

    _itemset.dec_t();

  }

  _add.set_t(t);

}

void ITEMSET::solution (KGLCMSEQ_QUE *occ, int core_id){

  QUEUE_ID i;
  LONG s;

  if ( _itemset.get_t() > _params._ub ) return;

  if ( _params._flag & ITEMSET_ALL ){
    if ( _fp.exist() || _topk.end() ){
    	solution_iter ( occ, core_id);
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
    output_itemset ( occ, core_id);
    _itemset.minus_t(_add.get_t());
  }
}

/*************************************************************************/
/* ourput a rule */
/*************************************************************************/
void ITEMSET::output_rule ( KGLCMSEQ_QUE *occ, double p1, double p2, size_t item, int core_id){

  FILE2 *fp = &_multi_fp[core_id];
  if ( fp->exist() && !(_topk.end()) ){
    fp->print_real ( p1, _params._digits, '(');
    fp->print_real ( p2, _params._digits, ',');
    fp->putch( ')');
    fp->print_int ( _perm[item], _params._separator);
    fp->puts ( " <= ");
  }
  if ( _params._flag & ITEMSET_RULE ){
    if ( _params._flag & ITEMSET_RULE_ADD ) solution (occ, core_id);
    else output_itemset ( occ, core_id);
  } else solution ( occ, core_id);

}

/*************************************************************************/
/* check all rules for a pair of itemset and item */
/*************************************************************************/
// lcm
void ITEMSET::check_rule (WEIGHT *w, KGLCMSEQ_QUE *occ, size_t item, int core_id){

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
    output_rule ( occ, p, ff, item, core_id);
  } else if ( _params._flag & (ITEMSET_RULE_RFRQ+ITEMSET_RULE_RINFRQ) ){
    if ( (_params._flag & ITEMSET_RULE_RFRQ) && (1-p) > _params._ratio_lb * (1-_item_frq[item]) ) return;
    if ( (_params._flag & ITEMSET_RULE_RINFRQ) && p > _params._ratio_ub * _item_frq[item] ) return;
    output_rule ( occ, pp, ff, item, core_id);
  }
}

/*************************************************************************/
/* check all rules for an itemset and all items */
/*************************************************************************/
// lcm_seq
void ITEMSET::check_all_rule ( WEIGHT *w, KGLCMSEQ_QUE *occ, QUEUE *jump, WEIGHT total, int core_id){

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
        if ( _params._flag  & ITEMSET_PRE_FREQ ) output_frequency ( _params._frq, _params._pfrq, core_id);

		    for(t=0;t<_itemset.get_t();t++) {

          _multi_fp[core_id].print_int ( _itemset.get_v(t), t?_params._separator:0);
          if ( t == i ){
            _multi_fp[core_id].putch(' ');
            _multi_fp[core_id].putch('=');
            _multi_fp[core_id].putch('>');
          }
          if ( !(_params._flag&ITEMSET_MULTI_OUTPUT) || _multi_fp[core_id].needFlush() ){
            SPIN_LOCK(_multi_core, _lock_output);
            flush_flag = 1;
            _multi_fp[core_id].flush_ ();
          }
        }
        if ( !(_params._flag&ITEMSET_PRE_FREQ) ) output_frequency ( _params._frq, _params._pfrq, core_id);
        _multi_fp[core_id].putch( ' ');
        _multi_fp[core_id].print_real ( _params._frq/_set_weight[i], _params._digits, '(');
        _multi_fp[core_id].putch( ')');
        _multi_fp[core_id].putch( '\n');

			//for _trsact_h_
        if ( _params._flag&(ITEMSET_TRSACT_ID+ITEMSET_MULTI_OCC_PRINT) ){
            output_occ ( _set_occELE[i], core_id);
        }

        if ( flush_flag ){
          _multi_fp[core_id].flush_ ();
          SPIN_UNLOCK(_multi_core, _lock_output);
        }
      }
    }
  }
    // constraint of relational frequency
  if ( ((_params._flag&ITEMSET_RFRQ)==0 || d >= _params._prob_lb * _prob ) 
      && ((_params._flag&ITEMSET_RINFRQ)==0 || d <= _prob * _params._prob_ub) ){
    if ( _params._flag&ITEMSET_RULE ){  //  rule mining routines
      if ( _itemset.get_t() == 0 ) return;
      if ( _params._target < _item_max ){

				for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
        // MQUE_FLOOP_CLS (*jump, x){
          if ( *x == _params._target ){ 
             check_rule ( w, occ, *x, core_id);  
             // if (_ERROR_MES) return; ////エラー処理考える
          }
        }
      } else {
        if ( _params._flag & (ITEMSET_RULE_FRQ + ITEMSET_RULE_RFRQ) ){
          if ( _add.get_t()>0 ){
            f = _add.get_v(_add.get_t()-1); t = _add.get_t(); _add.dec_t();
            //FLOOP (i, 0, t){
						for(i=0;i<t;i++){
              e = _add.get_v(i);
              _add.set_v(i,f);
              check_rule ( w, occ, e, core_id);    
              //if (_ERROR_MES) return; //エラー処理考える
              _add.set_v(i,e);
            }
            _add.inc_t();
          }
          // MQUE_FLOOP_CLS (*jump, x)
					for(x=jump->get_v();x<jump->get_v()+jump->get_t() ; x++){
            check_rule ( w, occ, *x, core_id);   
  	        //if (_ERROR_MES) return;	 //エラー処理考える
					}

        } else {
          if ( _params._flag & (ITEMSET_RULE_INFRQ + ITEMSET_RULE_RINFRQ) ){
            //FLOOP (i, 0, _item_max){
						for(i=0;i<_item_max;i++){
              if ( _itemflag[i] != 1 ){
                check_rule (w, occ, i, core_id);    
                // if (_ERROR_MES) return; //エラー処理考える
              }
            }
          }
        }
      }
    } else {  // usual mining (not rule mining)
      if ( _fp.exist() && (_params._flag&(ITEMSET_RFRQ+ITEMSET_RINFRQ))){
        _multi_fp[core_id].print_real ( d, _params._digits, '[');
        _multi_fp[core_id].print_real ( _prob, _params._digits, ',');
        _multi_fp[core_id].putch( ']');
      }
      solution (occ, core_id);
    }
  }
}


// つかってない？ 
//  _item_max_org = (QUEUE_INT)item_max_org;つかってない？ 
//    _solutions2 += _multi_solutions2[i];
//  _multi_solutions2 = _multi_solutions + j;
//  _multi_iters2 = _multi_iters + j;
//  _multi_iters3 = _multi_iters2 + j;

// _outputs += _multi_outputs[i];
// _outputs2 += _multi_outputs2[i];
// _iters2 += _multi_iters2[i];
// _iters3 += _multi_iters3[i];
//  _multi_outputs2 = _multi_outputs + j;
