/* SSPC: Similar Set Pair Comparison */
/* 2007/11/30 Takeaki Uno,   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about this code for the users.
   For the commercial use, please make a contact to Takeaki Uno. */

/*   internal_params.l1 = #solutions
  internal_params.l2 = #rows
  internal_params.l3 = #columns
*/

#include "kgsspc.hpp"

// comment out if single-core mode
//#define MULTI_CORE

#define WEIGHT_DOUBLE


typedef struct {

#ifdef MULTI_CORE
  pthread_t _thr; // thread identifier
#endif
  QUEUE_INT **_o;
  WEIGHT *_w;
  OFILE2 *_fp;
  int _core_id;
  int *_lock_i;

} SSPC_MULTI_CORE;

using namespace std;


WEIGHT KGSSPC::_comp ( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq)
{

  WEIGHT cc, ccc;
  if ( _P._problem & SSPC_INTERSECTION ) return (c);
  else if ( _P._problem & SSPC_INNERPRODUCT ){
    if ( _T.exist_w() ) return (c);
    else return (c / sq / sqrt(wx));
  } 
  else if ( (_P._problem & SSPC_RESEMBLANCE) && (cc= wi +wx -c) != 0 ) return (c/cc);
  else if ( (_P._problem & SSPC_MAXINT) && (cc=MAX(wi,wx)) != 0 ) return (c/cc);
  else if ( (_P._problem & SSPC_MININT) && (cc=MIN(wi,wx)) != 0 ) return (c/cc);
  else if ( (_P._problem & SSPC_PMI) && (cc=wi*wx) != 0 ) return ( log( ( c * _T.get_t()) / cc ) /  -log ( c / _T.get_t()));
  else if ( (_P._problem & SSPC_FVALUE) && MAX(wi,wx) != 0 ){ cc = c/wi; ccc = c/wx; return ( 2*cc*ccc / (cc+ccc)); }

  return (-WEIGHTHUGE);
}


/*******************************************************************/
/* output at the termination of the algorithm */
/* print #of itemsets of size k, for each k */
/*******************************************************************/
void KGSSPC::_last_output (){

  QUEUE_ID i;
  LONG n=0, nn=0;
  WEIGHT w;
  unsigned char c;

  OFILE2 *fp = &_multi_fp[0];

  _merge_counters();

  if ( !(_P._showFlag) ) return;  // "no message" is specified

  if ( _itemtopk.size() > 0 ){  // output values of the kth-best solution for each item

		for(n=0; n < _itemtopk.size() ;n++){

      c = 0;
			for(nn=0;nn<_itemtopk.end(n);nn++){ //FLOOP

        i = _itemtopk.findmin_head(n);
        w = _itemtopk.H(n,i);

        if ( w == -WEIGHTHUGE) break;

        if ( _P._iflag & ITEMSET_PRE_FREQ ){ fp->print_real ( w, 8, c); c = _P._separator; }
        fp->print_int (  _perm? _perm[_itemtopk_ary[n][i]]: _itemtopk_ary[n][i], c);
        c = _P._separator;
        if ( _P._iflag & ITEMSET_FREQ ){ 
        	fp->print_real ( w, 8, c); 
        	c = _P._separator; 
        }
				_itemtopk.chg(n,i, WEIGHTHUGE);

      }
      fp->putch('\n');
      fp->flush();
    }
    fp->flush_last ();
    goto END;
  }

  if ( _P._topk_k > 0 ){  // output value of the kth-best solution
		OFILE2 ofp(stdout);
		i = _topk.findmin_head();
		ofp.print(_topk.H(i)); 
		ofp.print("\n");
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
  
}

/* output frequency, coverage */
void KGSSPC::_output_frequency ( WEIGHT frq, int core_id){

  OFILE2 *fp = &_multi_fp[core_id];
  if ( _P._iflag&(ITEMSET_FREQ+ITEMSET_PRE_FREQ) ){
	  if ( _P._iflag&ITEMSET_FREQ ) fp->putch(' ');
  	fp->print_WEIGHT (frq, _P._digits, '(');
  	fp->putch( ')');
  	if ( _P._iflag&ITEMSET_PRE_FREQ ) fp->putch(' ');
  }

}

/* output an itemset to the output file for sspc -k */
void KGSSPC::_output_itemset_k (
	QUEUE *itemset, WEIGHT frq,
	QUEUE_INT itemtopk_item, QUEUE_INT itemtopk_item2, 
	int core_id)
{

  QUEUE_ID i;
  QUEUE_INT e;

  int flush_flag = 0;
  OFILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;

	if ( (_P._progressFlag ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) ){
		fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions[core_id], _multi_outputs[core_id]);
	}


	if ( frq < _frq_lb ) return;

  _multi_solutions[core_id]++;

  if ( _P._max_solutions > 0 && _multi_solutions[core_id] > _P._max_solutions ){
    _last_output();
    fprintf(stderr,"reached to maximum number of solutions\n");
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);

  _sc[itemset->get_t()]++;

  SPIN_UNLOCK(_multi_core, _lock_sc);

 	// _params._itemtopk_end > 0
	e = _itemtopk.findmin_head(itemtopk_item);


  if ( frq > _itemtopk.H(itemtopk_item,e) ){
  	SPIN_LOCK(_multi_core, _lock_sc);
		_itemtopk.chg(itemtopk_item,e, frq );
		if ( _itemtopk_ary ) _itemtopk_ary[itemtopk_item][e] = itemtopk_item2;
		SPIN_UNLOCK(_multi_core, _lock_sc);
	}
	
	return;

}

void KGSSPC::_output_itemset_ (
	QUEUE *itemset, WEIGHT frq, int core_id)
{

  QUEUE_ID i;
  QUEUE_INT e;

  int flush_flag = 0;
  OFILE2 *fp = &_multi_fp[core_id];
  
  _multi_outputs[core_id]++;

	if ( (_P._progressFlag ) && (_multi_outputs[core_id]%(ITEMSET_INTERVAL) == 0) ){
		fprintf(stderr,"---- " LONGF " solutions in " LONGF " candidates\n",
                  _multi_solutions[core_id], _multi_outputs[core_id]);
	}

	if ( frq < _frq_lb ) return;

  _multi_solutions[core_id]++;

  if ( _P._max_solutions > 0 && _multi_solutions[core_id] >  _P._max_solutions ){
    _last_output();
    fprintf(stderr,"reached to maximum number of solutions\n");
    EXIT;
  }

  SPIN_LOCK(_multi_core, _lock_sc);

  _sc[itemset->get_t()]++;

  SPIN_UNLOCK(_multi_core, _lock_sc);

  if ( _P._topk_k > 0 ){
  
		e = _topk.findmin_head();
		if ( frq  > _topk.H(e) ){

			SPIN_LOCK(_multi_core, _lock_sc);
			_topk.chg( e, frq );
			e = _topk.findmin_head ();
			_frq_lb = _topk.H(e) ;


			SPIN_UNLOCK(_multi_core, _lock_sc);

		}
    return;
  }

  if ( fp ){

    if ( _P._iflag&ITEMSET_PRE_FREQ ) _output_frequency ( frq, core_id);


		for(i=0;i<itemset->get_t();i++){ //FLOOP

			e = itemset->get_v(i);
			fp->print_int ( _perm? _perm[e]: e, i==0? 0:_P._separator);

			if ( fp->needFlush() ){
				SPIN_LOCK(_multi_core, _lock_output);
				flush_flag = 1;
				fp->flush_ ();
			}
		}

    if ( !(_P._iflag&ITEMSET_PRE_FREQ) ) _output_frequency ( frq, core_id);
		//((_params._flag & ITEMSET_NOT_ITEMSET) == 0) || 
		//(_params._flag & ITEMSET_FREQ) || (_params._flag & ITEMSET_PRE_FREQ) )

    fp->putch('\n');

    if ( flush_flag ){
      fp->flush_ ();
      SPIN_UNLOCK(_multi_core, _lock_output);
    }
  }
}

void KGSSPC::_output(
	QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, 
	QUEUE *itemset, WEIGHT frq, int core_id
){

	size_t b;

	if ( _P._problem & (SSPC_POLISH+SSPC_POLISH2) ){

    _vecchr[i] = 1;

		// store the solution
		if ( _P._problem & SSPC_POLISH2 ){  

			SPIN_LOCK (_P._multi_core, _lock_counter);

			// use deleted cell
			if ( (b = _itemary[_T.get_clms()]) ){
				_itemary[_T.get_clms()] = _buf[b]; 
      }
      else { // allocate new cell  
				b = _buf_end;
				_buf.realloci(b+30);
				_buf_end += 2;
			}
			_buf[b] = _itemary[i];  // insert the cell to list i
			_buf[b+1] = ii;
			_itemary[i] = b;

			SPIN_UNLOCK (_P._multi_core, _lock_counter);
    }
	} 
	else if ( _P._problem & SSPC_COUNT ){
		(*cnt)++;
	}
	else {

		if ( _P._problem & SSPC_OUTPUT_INTERSECT ){
			_multi_fp[core_id].print_int(_siz, 0);
			_multi_fp[core_id].putch(' ');
		}

		if ( _P._table_fname ){ 
			itemset->set_v(0,i);
			itemset->set_v(1,ii); 
		}
		else { 
			itemset->set_v(0, _positPERM[i]); 
			itemset->set_v(1,_positPERM[ii]); 
		}
		/*
		if ( (_P._problem&PROBLEM_NORMALIZE)&& _dir>0 ){
			if ( i  >= _sep ) itemset->minus_v(0,_root);
			if ( ii >= _sep ) itemset->minus_v(1,_root);
		}
    */

		if ( _P._itemtopk_end > 0 ){
			_output_itemset_k ( itemset, frq, i, ii, core_id);
			_output_itemset_k ( itemset, frq, ii, i, core_id);
		}
		else{
			_output_itemset_ ( itemset, frq,  core_id); //_II.output_itemset_
		}
	}
}

/* output a pair if it is in the similarity */
void KGSSPC::_comp2 (
	QUEUE_ID i, QUEUE_ID x, WEIGHT c, WEIGHT wi, WEIGHT wx, 
	double sq, QUEUE_INT *cnt, OFILE2 *fp, QUEUE *itemset, 
	int core_id)
{

  WEIGHT f1, f2, f1_=0, f2_=0, frq;

  int f=0, f_=0;
  QUEUE_ID i_ = i, x_ = x;

  if ( !_P._table_fname ){ x_ = _positPERM[x_]; i_ = _positPERM[i_]; }

	/*
  if ( (_problem&PROBLEM_NORMALIZE)&& _dir>0 ){
    if ( i_ >= _sep ) i_ -= _root;
    if ( x_ >= _sep ) x_ -= _root;
  }
	*/
	// threshold for the intersection size 
  if ( c < _P._th ) return;  

	// outputting intersection size
  _siz = c; 

	// S R C T s P F  (i I 以外) 
  if ( _P._problem & (SSPC_INTERSECTION +SSPC_RESEMBLANCE +SSPC_INNERPRODUCT +SSPC_MAXINT +SSPC_MININT + SSPC_PMI + SSPC_FVALUE) ){

    frq = _comp (c, wi, wx, sq);

    if ( frq == -WEIGHTHUGE ) return;

    if ( frq >= _P._frq_lb ) _output( cnt, x, i, itemset, frq, core_id);

    if ( _P._output_fname2 && frq >= _P._th2 ){

      SPIN_LOCK(_P._multi_core, _lock_output);
      fp->print("%d %d\n", x_, i_);
      SPIN_UNLOCK(_P._multi_core, lock_output);

    }
  } 
  else {

		// size of i and x
    f1 = wi * _P._frq_lb; 
    f2 = wx * _P._frq_lb;  

    if ( _P._output_fname2 ){ 
    	f1_ = wi*_P._th2; 
    	f2_ = wx*_P._th2; 
    }

    if ( _P._problem & SSPC_SIMILARITY ){

      if ( _P._output_fname2 ) f_ = ( (c >= f1_) && (c >= f2_) );
      else f = ( (c >= f1) && (c >= f2) );

      frq = MIN(c / wi, c / wx);

    }
  	else if( _P._problem & SSPC_INCLUSION ){

      if ( c >= f2 ){
        frq = c / wx;
        _output ( cnt, x, i, itemset, frq, core_id);
      }

      f = ( c >= f1 );
      frq = c / wi;

      if ( _P._output_fname2 ){
        if ( c >= f2_ ){
          SPIN_LOCK(_P._multi_core, _lock_output);
          fp->print("%d %d\n", x_, i_);
          SPIN_UNLOCK(_P._multi_core, _lock_output);
        }
        f_ = (c >= f1_);
      }
    }
    else{
    	throw("non similrarity");
    }

    if ( f ) {
    	_output ( cnt, i, x, itemset, frq, core_id);
    }

    if ( _P._output_fname2 && f_ ){
      SPIN_LOCK(_P._multi_core, _lock_output);
      fp->print("%d %d\n", i_, x_);
      SPIN_UNLOCK(_P._multi_core, _lock_output);
    }

  }
}

/* iteration for muticore mode */
void *KGSSPC::_iter (void *p){

  SSPC_MULTI_CORE *SM = (SSPC_MULTI_CORE *)p;
  int core_id   = SM->_core_id;
  QUEUE_INT **o = SM->_o;
  WEIGHT *w     = SM->_w;
  OFILE2 *fp      = SM->_fp;

  char *mark = NULL;

  QUEUE jump(_T.get_clms()), itemset(2,2);

  QUEUE_ID ii=0, t, m;
  QUEUE_INT i=0, i_=0, ff;
  QUEUE_INT *OQend,  *x, *oi, *oj;
  //QUEUE_INT cnt = ((_problem&SSPC_COMP_ITSELF) && _dir == 0)?1:0;
	QUEUE_INT cnt = ( _P._problem&SSPC_COMP_ITSELF )?1:0;

  WEIGHT *occ_w,  c, *y;
  WEIGHT yy=0, wi=0, wx=0;


  double sq = 0;

  size_t b, bb;
  int f;

  if ( _P._problem & SSPC_NO_NEIB ){
  	mark = new char[_T.get_clms()]();
  }
  occ_w = new WEIGHT[_T.get_clms()]();
  OQend = new QUEUE_INT[_T.get_clms()]();

  while (1){


    if ( i == i_ ){

      i_ = 100000000 / (_T.get_eles() / _T.get_clms());
			
      SPIN_LOCK (_P._multi_core, _lock_counter);  // lock!!

      if ( (i = *(SM->_lock_i)) >= _T.get_clms() ){
        SPIN_UNLOCK (_P._multi_core, _lock_counter);  // unlock!!
        break;
      }

      i_ = MIN(_T.get_clms(), i + 100);
      (*(SM->_lock_i)) = i_;

      if ( _P._progressFlag ){
        if ( (int)((i-1)*100/_T.get_clms()) < (int)(i*100/_T.get_clms()) )
            fprintf (stderr, "%d%%\n", (int)(i*100/_T.get_clms()));
      }
      SPIN_UNLOCK (_P._multi_core, _lock_counter);  // unlock!!

    }

    if ( (_P._problem & SSPC_INNERPRODUCT) && !_T.exist_w() ) sq = sqrt (w[i]);

    if ( _P._problem & (SSPC_POLISH+SSPC_POLISH2) ) m = _T.get_clms();
  	else m = i;
  	//else m = (_dir>0)?_sep:i;

    jump.setEndByStart(); 

    if (_P._problem & SSPC_NO_NEIB){ // for no_neib
    
			for( x=_T.begin(i) ; x < _T.end(i) ; x++){
				mark[*x] |= 1;
			}
			for( x= _OQ.begin(i) ; x<_OQ.end(i) ; x++){
				mark[*x] |= 2;
			}

    }

		for(QUEUE_INT * iix = _OQ.start(i); iix<_OQ.end(i); iix++ ){

      t = *iix;

			// get item weight of current vector
      if ( _T.exist_w() && (_P._problem & SSPC_INNERPRODUCT)){ 

        y = _T.get_w(t);
	      for( x=_T.begin(t); *x < i  ; x++ ){ y++; }
        yy = *y;

      }

      if ( (_P._problem & SSPC_NO_NEIB) && t == i ) continue;

      if ( _T.exist_w() ){ y = _T.get_w(t); }
      else                 { y = 0;}


      for( x =_T.begin(t); *x < m ; x++){

        if ( (_P._problem & SSPC_POLISH2) && *x < i) continue;
        if ( (_P._problem & SSPC_NO_NEIB) && *x == t ) continue;

        if ( OQend[*x] == 0 ){
          jump.push_back(*x);
          occ_w[*x] = 0;
        }
        OQend[*x]++;

        if (_T.exist_w() ){
          if (_P._problem & SSPC_INNERPRODUCT){
            occ_w[*x] += (*y) * yy; 
          }
          else { 
          	occ_w[*x] += *y;
          }
          y++;
        }
        else {
          occ_w[*x] += _w[t]; 
        }
      }
    }

		for(x=jump.begin(); x < jump.end(); (x)++){

      if ( *x == i ) goto SKIP;

      c = occ_w[*x];

      if ( _P._rowSortDecFlag ){
        for (oi=o[i],oj=o[*x] ; *oi< _P._len_lb ; oi++ ){
          while ( *oj < *oi ) oj++;
          if ( *oi == *oj ) c += _w[*oi];
        }
      }

      wi = w[i];
      wx = w[*x];

      if (_P._problem & SSPC_NO_NEIB){  // for no_neib
        if ( mark[*x]&1 ) wi -= _w[*x];
        if ( mark[i] &2 ) wx -= _w[i];
      }

      _comp2 ( i, *x, c, wi, wx, sq, &cnt, fp, &itemset, core_id);

      SKIP:;
      OQend[*x] = 0;

    }

		// selfcomparison
    //if ((_problem & SSPC_COMP_ITSELF) && _dir == 0) {
    //    output (
    //    	&cnt, i, i, &itemset, 
    //    	((_problem&PROBLEM_NORMALIZE)&& _dir>0)? i- _sep: i, //_dir>0ありえない
    //    	core_id
    //    );
		//	}
		// selfcomparison
    if ((_P._problem & SSPC_COMP_ITSELF) ) {
        _output ( &cnt, i, i, &itemset, i, core_id );
    }

		// data polish;  clear OQ, and marks
    if ( _P._problem & (SSPC_POLISH+SSPC_POLISH2) ){  

      if ( _P._problem & SSPC_POLISH2 ){
          for (b=_itemary[i] ; b ; b=_buf[b]) _vecchr[_buf[b+1]] = 1;
      }
      f = 0;

      for(QUEUE_INT *iq = _OQ.start(i) ; iq < _OQ.end(i) ; iq++ ){

        ff = 0;

				for( x=_T.get_vv(*iq); *x < _T.get_clms() ; x++){
					if ( _vecchr[*x] ){ 
						ff = 1; 
						break; 
					}
        }
        if ( ff ){
					_multi_fp[core_id].print_int(*iq, f);
          _multi_fp[core_id].putch(' ');
          f = _P._separator;
        }
      }
      
     	_multi_fp[core_id].putch('\n');
     	_multi_fp[core_id].flush();

      jump.clrMark( _vecchr );

      if ( _P._problem & SSPC_POLISH2 ){  
				// data polish;  clear OQ, and marks
        for (b=_itemary[i] ; b ; b=bb){ // insert cells to deleted cell queue
          bb = _buf[b];
          _vecchr[_buf[b+1]] = 0;
          _buf[b] = _itemary[_T.get_clms()];
          _itemary[_T.get_clms()] = b;
        }

      }
    }

    if (_P._problem & SSPC_NO_NEIB){ // for no_neib
    	_T.clrMark(i,mark); // これなに？
			_OQ.clrMark(i,mark);
    }
		_OQ.endClr(i);

    if ( _P._problem & SSPC_COUNT ){
      while ( ii < _positPERM[i] ){
				_multi_fp[core_id].putch('\n');
				_multi_fp[core_id].flush();
        ii++;
      }
			_multi_fp[core_id].print_int(cnt, 0);
			_multi_fp[core_id].putch('\n');
			_multi_fp[core_id].flush();
      _sc[2]+=cnt;
      ii++;
    }
    i++;
  }
  delete [] mark;
  delete [] occ_w;
  delete [] OQend;

  //mfree (mark, occ_w,OQend);

  return (NULL);

}
/*************************************************************************/
/* SSPC main routine */
/*************************************************************************/
void KGSSPC::_SspcCore(){

	#ifdef MULTI_CORE
  	void *tr;
	#endif

	SSPC_MULTI_CORE *SM = NULL;
  OFILE2 fp;  // file pointer for the second output file

  // QUEUE_ID i; 
  //QUEUE_ID begin = (_problem&(SSPC_POLISH+SSPC_POLISH2))?0:(_dir>0?_sep:0);
  QUEUE_ID begin =0;
  QUEUE_INT **o=NULL;
  WEIGHT *w;
  int cnt;

	_OQ.setfromT(_T);

      
  if ( _P._output_fname2 ) fp.open(_P._output_fname2);

  // initialization
  w = new WEIGHT[_T.get_clms()*2]();
  
  if(_P._problem&SSPC_INNERPRODUCT){

  	if(!_T.exist_w()){ 
			for(QUEUE_INT i =0 ; i < _T.get_clms(); i++){
				_w[i] *= _w[i];
			}
  	}

	  _jump.setEndByStart();
  	for(VEC_ID t=0 ; t<_T.get_t(); t++){
    	_T.delivery_iter( 
    		w, w+_T.get_clms(), t, _T.get_clms() ,
    		&_jump,_OQ.getOQ(), _w,_pw,
    		_C.existNegative()
    	);
    }

	  if(_T.exist_w() ){ 

		  for(QUEUE_INT i=0; i<_T.get_clms() ; i++){ w[i] = 0; } 
		
			for (VEC_ID i=0 ; i < _T.get_t();i++){
				WEIGHT *y =_T.get_w(i);
				for(QUEUE_INT *x = _T.get_vv(i); x < _T.get_vv(i)+_T.get_vt(i);x++){
					w[*x] += (*y)*(*y); y++;
				}
			}

			for(QUEUE_INT i=0;i<_T.get_clms();i++){ w[i] = sqrt(w[i]); }

			for(VEC_ID i=0 ; i<_T.get_t();i++){
  		  WEIGHT *y = _T.get_w(i);
				for(QUEUE_INT *x=_T.get_vv(i);x<_T.get_vv(i)+_T.get_vt(i); x++){
					 *y /= w[*x]; y++;
				}
			}
	  }//ここなに // normalize the vectors for inner product (c + -W の指定時
  }
  else{
	  //_TT.delivery( w, w+_T.get_clms(), NULL, _T.get_clms());
	  _jump.setEndByStart();
  	for(VEC_ID t=0 ; t<_T.get_t(); t++){
    	_T.delivery_iter( 
    		w, w+_T.get_clms(), t, _T.get_clms() ,
    		&_jump,_OQ.getOQ(), _w,_pw,
    		_C.existNegative()
    	);
    }
  }
    
	for(QUEUE_INT i =0 ;i < _T.get_clms();i++){
		_OQ.set_end(i,0);
	}


	// skipping items of large frequencies
	if ( _P._rowSortDecFlag ){

		o = new QUEUE_INT *[_T.get_clms()];

		for ( QUEUE_ID i =0 ; i < _T.get_clms() ; i++){
			o[i] = _OQ.get_v(i);
			_OQ.set_sentinel(i);
			QUEUE_ID j;
			for (j=0 ; _OQ.get_v(i,j) < _P._len_lb ; j++); //skip
			_OQ.move(i,j);
		}
	}


	// selfcomparison
  //if ((_problem & SSPC_COMP_ITSELF) && _dir == 0){ 
  if ((_P._problem & SSPC_COMP_ITSELF)){ 
 		QUEUE itemset(2,2);
		_output( &cnt, 0, 0, &itemset, 0, 0);
    itemset.clear();
  }

  // for multi-core
	SM = new SSPC_MULTI_CORE[_P._multi_core]; //malloc2


	for (int i=_P._multi_core; (i--) > 0 ; ){
    SM[i]._o = o;
    SM[i]._w = w;
    SM[i]._fp = &fp;
    SM[i]._core_id = i;
    SM[i]._lock_i = &begin;
    
#ifdef MULTI_CORE
    if ( i > 0 ) pthread_create(&(SM[i]._thr), NULL, iter, (void*)(&SM[i]));
    else 
#endif
      _iter((void*)(&SM[i]));
  }

  // wait until all-created-threads terminate
#ifdef MULTI_CORE

	for(int i=1 ; i < _P._multi_core; i++){
		pthread_join(SM[i]._thr, &tr);
	}

#endif

  // termination これいる？
	if ( _P._rowSortDecFlag ){
		for(int i=1 ; i < _T.get_clms(); i++){
  	  _OQ.prefin(i,o[i]);
    }
  }

  delete [] w;
  delete [] o;
	delete [] SM;

}


/*************************************************************************/
/* main function of SSPC */
/*************************************************************************/
int KGSSPC::_runMain(){

	// boundary set
	_C.setLimit(_P._limVal);


  IFILE2 fp(_P._fname) , fp2(_P._fname2) ;
  
	// count file 	
	
	//_C.fileCountA(fp ,fp2 ,_P._wfname);

	if( _P._tposeF ){
		_C.fileCountT(fp ,fp2 ,_P._wfname);
	}
	else{
		_C.fileCount(fp ,fp2 ,_P._wfname);
	}

	// Make PERM &inverse perm 
	//PERM *cperm = 
	_C.makeCperm();
	if (_C.c_clms()==0) { throw ("there is no frequent item"); }
	//PERM *rperm = 
	_C.makeRperm(_P._rowSortDecFlag); 	// 多いものから順番並べるいみある？_trperm使い方に寄りそう


	// _C.show_rperm();

	// make SPace
  _jump.alloc( _C.c_clms()+1 );
	_w.malloc2 ( _C.r_clms()+1 );
   if ( _C.existNegative() ){
  	_pw = new WEIGHT[_C.r_clms()+1 ];
  }
 
 
	// _Tのバッファ _v _bufもセットされる
	_T.setSize4sspc(_C,_P._tposeF,_P._iwfname); 

	// ============================この辺から _T .loadでまとめる===================================


  // set variables w.r.t rows
  // (r_eles c_elesは基本同じ：制約にひっかかると数が変わってくる)
	int fflag = _C.rGTc(_P._tposeF);

	VEC_ID tt=0 ;
  size_t pos = 0; 
	for( VEC_ID t =0 ; t < _C.rows(); t++ ){

		if(	_C.rperm(t) <= _C.rows() ) {

      _T.init_v(tt);
      _C.replaceRperm(tt,t); //ここでrpemが変わる。。もどる？

      _w[tt]   = _C.get_rw(t);
      if ( _pw ) {  _pw[tt] = MAX (_w[tt], 0); }

      if ( !fflag ){ 
        _T.setVBuffer(tt,pos);
        pos += _C.get_rowt(t)+1;
      }
			tt++;
		}
	}
  if ( fflag ) {  _T.setVBuffer(0, 0); }


	// _file_read
	//_T.fileRead( fp ,fp2, _P._iwfname , _C , fflag , _P._tflag)
	VEC_ID t =0;
  fp.reset();	  
  if( _P._iwfname ){ // sspcでitem weightを指定した時のみ
	  IFILE2 wfp(_P._iwfname);
		_T.file_read( fp , wfp, _C , &t , fflag , _P._tflag);
  }
  else{
		_T.file_read( fp , _C , &t ,fflag, _P._tflag);
	}	

	// fp2にアイテムファイルは指定できないようになってたので
	if( fp2.exist() ){
		fp2.reset();
		_T.file_read( fp2 , _C , &t , fflag , _P._tflag);
	}

	_T.initQUEUEs();

	// sort rows for the case 
	// that some columns are not read
  if ( _P._rowSortDecFlag ){  _T.setInvPermute( _C.get_rperm(), _C.get_rInvperm() , -1); }

	 _T.queueSortALL(1); 

  if ( _P._rmDupFlag ){	_T.rmDup(); }

	// ============================この辺まで _T .loadでまとめる===================================

	_OQ.alloc(_T);

	//_sep -cの指定がある時に利用される ここでOK？もとはTT.alloc
	//_sep = _TT.adjust_sep(_sep); // 
	//if(_tflag&LOAD_TPOSE){ _sep = _C.adjust_ClmSep(_sep);}
	//else                 { _sep = _C.adjust_RowSep(_sep);}

  if ( _P._len_ub < INTHUGE ){ 
 	  for (VEC_ID i=0 ; i<_T.get_t() ; i++){
    	if ( _T.get_vt(i) <= _P._len_ub ){ _P._len_lb = i; break; }
    }
  }	

  if ( _P._itemtopk_item > 0 ){  _P._itemtopk_end= _C.c_clms(); }

	_occ_w  = new WEIGHT[_C.c_clms()+2]();
	_vecchr = new char  [_C.r_clms()+2]();

	if(_P._problem&SSPC_POLISH2) {
		_itemary = new QUEUE_INT[_C.c_clms()+2]();
	}

	if ( _P._outperm_fname ){
		PERM *p = NULL;
		IFILE2::ARY_Load( p ,_P._outperm_fname);
		_C.replacecInvperm(p);
	  delete [] p;
	} 

  _itemset.alloc(_C.c_clms()+2 , _C.c_clms()+2);

	_sc = new LONG[_C.c_clms()+2]();

	// allocate topk heap
  if ( _P._topk_k > 0 ){
		_topk.alloc( _P._topk_k ,-WEIGHTHUGE);
		_frq_lb = -WEIGHTHUGE ;
  }

	// allocate topkheap for each element
  if ( _P._itemtopk_end > 0 ){ 

    _itemtopk.setSize(_P._itemtopk_end,_P._itemtopk_item,-WEIGHTHUGE);

    if ( _P._itemtopk_item2 > 0 ){
			_itemtopk_ary = new QUEUE_INT*[_itemtopk.size()];
	    for(LONG i = 0 ; i<_P._itemtopk_end ; i++){
        _itemtopk_ary[i] = new QUEUE_INT[_P._itemtopk_item];   
      }
    }
  }

  _iters = _solutions = 0; //_iters2 =
  _item_max = _C.c_clms();

  if ( _P._output_fname ){
  	// 出力バッファ確保しないほうがいい？
    if ( strcmp (_P._output_fname, "-") == 0 ) _ofp.open(stdout);
    else{
    	if(_P._iflag&ITEMSET_APPEND){ _ofp.openA( _P._output_fname);}
    	else                     { _ofp.open( _P._output_fname);}
    }
  } 

  int j = MAX(_P._multi_core,1);
  _multi_iters = new LONG[j*3](); 
  _multi_outputs = _multi_iters + j;
  _multi_solutions = _multi_outputs + j;
  _multi_fp = OFILE2::makeMultiFp(j,_ofp);


#ifdef MULTI_CORE
  if ( _params._multi_core > 0 ){
    pthread_spin_init (_lock_counter, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init (_lock_sc, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init (_lock_output, PTHREAD_PROCESS_PRIVATE);
  }
#endif


  // II alloc
	VEC_ID maxsize =  MAX( _C.r_clms() , _C.c_clms() )+1;
	_w.realloc2(maxsize);

	for(size_t i=0; i<maxsize;i++){ _w[i] = 1;}

  _positPERM = _C.get_cInvperm();
  _perm=NULL;

  if ( _T.get_clms()>1 ){  
  	_SspcCore(); 
  }
	_perm = _C.get_cInvperm(); // -K _P._itemtopk_end時必要

  _merge_counters();

  if ( _topk.end() > 0 || _P._itemtopk_end > 0 ){
		_last_output();
  }
  else{
  	fprintf(stderr , LONGF " pairs are found\n" , _sc[2]);
  }
	_oclose();

  return 0;

}

int KGSSPC::run(kgSspcParams para){
	_P = para;

	if(_P._dir==1){
		//return _runMainC();
	}
  else if ( _P._table_fname ){ 
		//return SSPC_list_comp (&PP); 
  }
	else{
		return _runMain();
	}
	return 1;
}

int KGSSPC::run(int argc ,char* argv[]){

	if( _P.setArgs(argc, argv) )return 1;
	if(_P._dir==1){
		//return _runMainC();
	}
  else if ( _P._table_fname ){ 
		//return SSPC_list_comp (&PP); 
  }
	else{
		return _runMain();
	}
	return 1;
		
}


std::vector<LONG> KGSSPC::mrun(int argc ,char* argv[]){
	KGSSPC mod;
	mod.run(argc,argv);

	return mod.iparam();


}


/*

  //_w.malloc2( _T.get_end());

  //if ( _C.existNegative() ){
  //	_pw = new WEIGHT[_T.get_end()];
  //}

  //_perm = new PERM[_T.get_clms()+1]();

  //_jump.alloc(_T.get_clms()+1);

  //if ( (!_T.exist_w()) && _P._iwfname ) {
	//	 _T.alloc_weight(_C); 
	//}
*/