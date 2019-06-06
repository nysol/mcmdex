/*  QUEUE based Transaction library, including database reduction.
            25/Nov/2007   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#include <iostream>
using namespace std;

#include"trsact.hpp"
#include"base.hpp"
#include"vec.hpp"
#include"queue.hpp"
#include"stdlib2.hpp"

void TRSACT::prop_print (){

  if ( !(_flag & SHOW_MESSAGE) ) return;

  fprintf(stderr,"trsact: %s", _fname);
  if(_fname2){ 
  	fprintf(stderr," ,2nd-trsact %s (from ID %d)", _fname2, _C.end1()); 
  }
  fprintf(stderr," ,#transactions %d ,#items %d ,size %zd", _C.rows(), _C.clms(),_C.eles());
  fprintf(stderr," extracted database: #transactions %d ,#items %d ,size %zd", _T.get_t(), _T.get_clms(), _T.get_eles());

	if(_wfname)      { fprintf(stderr," ,weightfile %s", _wfname); }
	if(_item_wfname) { fprintf(stderr," ,itemweightfile %s", _item_wfname); }
	if(_pfname)      { fprintf(stderr," ,item-order-file %s", _pfname); }
  fprintf(stderr,"\n");

}

// _OQ クラス化した方がいい?
void TRSACT::_OccAlloc(void){

    VEC_ID *p;
    p = _T.counting();

    _clm_max = p[0];
		size_t cmmsize = p[0];
		for(int cmm = 1 ; cmm < _T.get_clms() ;cmm++ ){
			cmmsize += p[cmm];
			if( _clm_max < p[cmm]){
				_clm_max=p[cmm];
			}
		}
			
		_OQ = new QUEUE[_T.get_clms()+1]; 

		QUEUE_INT *cmn_pnt;
		size_t pos=0;
		try{
			cmn_pnt = new QUEUE_INT[cmmsize+_T.get_clms()+2]; //malloc2
		}catch(...){
			delete[] _OQ;
			throw;
		}

		for(VEC_ID cmmv =0; cmmv < _T.get_clms() ; cmmv++){

			_OQ[cmmv].alloc( p[cmmv],cmn_pnt+pos );

			pos += p[cmmv]+1;

		}
		_OQ[_T.get_clms()].alloc( MAX(_T.get_t(), _clm_max));


		// end is illegally set to 0, for the use in "TRSACT_find_same" 
		for(size_t i=0 ; i < _T.get_clms()+1 ; i++ ){
			_OQ[i].endClr();
		}

    // initial occurrence := all transactions
    _OQ[_T.get_clms()].initVprem(_T.get_t());  //ARY_INIT_PERM

    delete [] p;
    
}


/* 
	allocate memory, set permutation, and free C.clmt,rowt,rw,cw 
*/
void TRSACT::_alloc(){

  VEC_ID t, tt=0, ttt, ttt_max, h, flag, org;
  FILE_COUNT_INT *ct;
  size_t s=0;
  PERM *q, *p=NULL;
  char *buf;

  if ( _flag2&TRSACT_SHRINK ) _flag |= LOAD_DBLBUF;

	_clms_end = _C.c_end();

	_T.setSize(_C,_flag); // _Tのバッファ _v _bufもセットされる

  _w.malloc2( _T.get_end());

  if ( _flag2&TRSACT_NEGATIVE ) { 	// たぶんこう？
  	_pw = new WEIGHT[_T.get_end()]; //malloc2
  }
  else{
  	_pw = NULL;
  }


  _perm = new PERM[_T.get_clms()+1](); //calloc2 

  _jump.alloc(_T.get_clms()+1);

	int bufSize = MAX( (int)_row_max*4, (int)(_T.get_eles()+1)/10+_T.get_clms()+100 );

  _buf.alloc ( sizeof(QUEUE_INT), bufSize );
  _wbuf.alloc( sizeof(WEIGHT), bufSize);

  if ( _flag2&TRSACT_SHRINK ){
    
    _mark  = new VEC_ID[_T.get_end()];       //malloc2
    _shift = new QUEUE_INT*[_T.get_end()]; //malloc2
    _sc    = new char[_T.get_clms()]();        //calloc2

  }

  if ( !_T.exist_w() ) {
  	if(_flag2&TRSACT_UNION){ _T.alloc_w();}
		if( _item_wfname )     { _T.alloc_weight(_C); }
	}

  _trperm = new PERM[_T.get_t()];   //malloc2 

  // set variables w.r.t rows
  tt=0 ;
  size_t pos = 0; 

	for( VEC_ID t =0 ; t < _C.rows(); t++ ){

		if ( _C.rPermGErows(t) ) {

      _T.init_v(tt);
			_trperm[tt] = t;
			_C.set_rperm(t, tt);

      _w[tt] = _C.get_rw(t);

      if ( _pw ) {  _pw[tt] = MAX (_w[tt], 0); }

      if ( !flag ){
        _T.setVBuffer(tt,pos); // _T の_v ,_buf連動させる)
        pos += _C.get_rowt(t)+1;
      }
			tt++;
		}
	}

  // make the inverse perm of items
  for(VEC_ID t =0 ; t < _C.clms() ; t++ ){
    if ( _C.cperm(t) <= _clms_end ){
      _perm[_C.cperm(t)] = t;
    }
  }

	// _sep = _C.adjust_sep(_sep,_end1,_flag&LOAD_TPOSE);
  _new_t = _T.get_t();

  return ;

}


/* load the file to allocated memory according to permutation */
// flag ( _C.r_eles() > _C.c_eles() && !(_flag & LOAD_TPOSE) );
void TRSACT::_file_read (FILE2 &fp, FILE2 &fp2,  int flag)
{ 

  QUEUE_ID tt;
  VEC_ID t=0;

  if ( flag ) {  _T.setVBuffer(0, 0); }


  fp.reset();
  if(_flag&(LOAD_NUM+LOAD_GRAPHNUM)){
  	fp.read_until_newline(); 
  }
	  
  if( _item_wfname ){ // sspcでitem weightを指定した時のみ
	  FILE2 wfp(_item_wfname);
		_T.file_read( fp , wfp, _C , &t , flag , _flag);
  	
  }
  else{
		_T.file_read( fp , _C , &t ,flag , _flag);
	}

	// fp2にアイテムファイルは指定できないようになってたので
	if( fp2.exist() ){
		fp2.reset();
	  if(_flag&(LOAD_NUM+LOAD_GRAPHNUM)){
  		fp2.read_until_newline (); 
  	}
		_T.file_read( fp2 , _C , &t , flag , _flag);
	}

	_T.initQUEUEs();

}
/* sort the transactions and items according to the flag, allocate OQ, and database reduction */
/* causion! not adopt for itemweights!!!!! */
void TRSACT::_sort(){

  VEC_ID t, *p;
  int f;
	int flag;//<<=これもうちょっと考える
  PERM pp;
  QUEUE Q;
  QUEUE_ID i;
  WEIGHT *ww;

	// _T.allvvInit(); //これいる？

  flag = (_flag&(LOAD_SIZSORT+LOAD_WSORT)? ((_flag&LOAD_DECROWSORT)? -1:1):0) *sizeof(QUEUE);

	// sort rows for the case 
	// that some columns are not read
  if ( flag ){  _T.setInvPermute( _C.get_rperm(), _trperm , flag); }



  if ( _flag & LOAD_PERM ) {  flag = 1; }
  else{  flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0); }

  if ( flag ){ _T.queueSortALL(flag); }

  if ( _flag & LOAD_RM_DUP ){	_T.rmDup(); }

	_row_max = _T.RowMax();

	_OccAlloc();  // _flag2 & TRSACT_ALLOC_OCC+TRSACT_SHRINK

	// shrinking database
  if ( _flag2&TRSACT_1ST_SHRINK ){

    Q = _OQ[_T.get_clms()];

    _OQ[_T.get_clms()].tClr();

    find_same( &Q, _T.get_clms());

		removeDupTrsacts(); 

    _OQ[_T.get_clms()].tClr();
	  
	  // make resulted occ
    for(VEC_ID t = 0; t < _T.get_t() ; t++){
			if ( _mark[t]>0 ) _OQ[_T.get_clms()].push_back(t);  
    }

  }
  
}


/* sort the transactions and items according to the flag, allocate OQ, and database reduction */
/* causion! not adopt for itemweights!!!!! */
void TRSACT::_sortELE(){
  VEC_ID t, *p;
  int f;
  int flag; //<<=これもうちょっと考える

  KGLCMSEQ_QUE Q;
  QUEUE_ID i;

	//_T.allvvInit();

  flag = (_flag&(LOAD_SIZSORT+LOAD_WSORT)? ((_flag&LOAD_DECROWSORT)? -1:1):0) *sizeof(QUEUE); // QUEUEsでOK?

  if ( flag ){   
  	// sort rows for the case that some columns are not read
		//_T.setInvPermute( C->get_rperm(),_trperm,flag); << = これでOK?
		_T.qsort_perm(_C.get_rperm(), flag);
		_T.any_INVPERMUTE( _C.get_rperm());

		if ( _T.exist_w() ){ _T.any_INVPERMUTE_w(_C.get_rperm()); }

		_T.any_INVPERMUTE_(_trperm, _C.get_rperm());
		
  }

  //free2 (C->rperm); free2 (C->cperm);

  if ( _flag & LOAD_PERM ) flag = 1;
  else flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);

  if ( flag ){ _T.queueSortALL(flag); }
  
  if ( _flag & LOAD_RM_DUP ){ _T.rmDup(); }

	_row_max = _T.RowMax();

	//if ( _flag2 & TRSACT_ALLOC_OCC+TRSACT_SHRINK ){
	{
    
    VECARY<VEC_ID> p;
    p.calloc2(_T.get_clms()); 

		//===== QUEUE_delivery==============
		VEC_ID iv, ev; 
	  QUEUE_INT *x;
		for (iv=0 ; iv<_T.get_t() ; iv++){
    	ev =  iv;
    	for(x=_T.get_vv(ev); *x < _T.get_clms() ;x++){ p[*x]++; }
		}
		//===================

    _clm_max = p.max( 0 , _T.get_clms());

    Mque_allocELE(p);

		_OQELE[_T.get_clms()].alloc( MAX(_T.get_t(), _clm_max));

		// end is illegally set to 0, for the use in "TRSACT_find_same" 
    for(i=0;i<_T.get_clms()+1 ;i++){
	    _OQELE[i].set_end(0);   
  	}

    // initial occurrence := all transactions
		for(size_t i=0 ; i< _T.get_t(); i++){ 
			_OQELE[_T.get_clms()].set_v(i,i);
		}
    _OQELE[_T.get_clms()].set_t( _T.get_t());
  }

	// shrinking database
  if ( _flag2&TRSACT_1ST_SHRINK ){

    Q = _OQELE[_T.get_clms()];
    _OQELE[_T.get_clms()].set_t(0);

    find_same ( &Q, _T.get_clms());

		removeDupTrsacts(&_OQELE[_T.get_clms()]);

    _OQELE[_T.get_clms()].set_t(0);

    for(t=0;t<_T.get_t();t++){
    	if ( _mark[t]>0 ) _OQELE[_T.get_clms()].push_backt(t);  // make resulted occ
    }
  }
}

/*****************************************/
/* load transaction file and its weight  */
// _fname
// _wfname (-w:sspcの場合) 
// _item_wfname (-W:sspcの場合)
// _fname2  (-2:sspcの場合)
// 以下二つは指定される事がない
// 必要であれば追加する
// _wfname2  
// _item_wfname2
/*****************************************/
int TRSACT::loadMain(bool elef){

  FILE2 fp(_fname) , fp2(_fname2) ;

  int f;

	// PreRead for count  
	// swap variables in the case of transpose
	// set lower/upper bounds if it is given by the ratio
  if( _C.file_count( _flag&LOAD_TPOSE , fp ,fp2 ,_wfname )) { 
	  fprintf(stderr,"file_count ERROR");
  	return 1;
  }

	if( _C.existNegative()){ _flag2 |= TRSACT_NEGATIVE; }
	
	//_cPerm = _C.initCperm(_pfname ,_flag ,_flag2);
	//_rPerm = _C.initRperm(_flag);
	
 	_C.makePerm(_pfname ,_flag ,_flag2);

	
	// f は 
  // flag = (_T.get_eles() > _C.c_eles() && !(_flag & LOAD_TPOSE) );
  // の内容　
  // flagがセットさていない場合は
  // _T.setVBufferが動くのでバッファが_Tにセットされている？
  // 
	f = ( _C.r_eles() > _C.c_eles() && !(_flag & LOAD_TPOSE) );
  
  _alloc();

  VEC_ID t=0;

	// _T.file_read(fp , _C, &t, f, _flag&(LOAD_NUM+LOAD_GRAPHNUM), _item_wfname);
	//  _file_read( &fp, &_C, &t, f, _item_wfname);
  //	if ( _fname2 ){
	//	  _file_read( &fp2, &_C, &t, f, NULL); 
	//	}

  _file_read( fp , fp2 , f);

  if(elef){ _sortELE(); }
	else    { _sort   (); }


  if (_ERROR_MES){
	  fprintf(stderr,"TRSACT::loadMain");
		return 1;
  } 

  prop_print();

  return 0;

}

void TRSACT::delivery (WEIGHT *w, WEIGHT *pw, QUEUE *occ, QUEUE_INT m){

  _jump.setEndByStart();

  if (occ) {

  	for(QUEUE_INT *b = occ->start() ; b < occ->end() ; b++){
    	_T.delivery_iter(
    		 w, pw, *b, m ,
    		 &_jump,_OQ,
    		 _w,_pw,
    		 _flag2&TRSACT_NEGATIVE);
	  }

	}
	else{

  	for(VEC_ID t=0 ; t<_T.get_t(); t++){
    	_T.delivery_iter( 
    		w, pw, t, m ,
    		&_jump,_OQ, _w,_pw,
    		_flag2&TRSACT_NEGATIVE);
    }
	}
	

}

/* usual delivery (make transpose) with checking sc
   don't touch jump */
/* if (T->flag2&TRSACT_DELIV_SC), do not stack to items e with non-zero T->sc[e] */
// using only lcm 
void TRSACT::deliv ( QUEUE *occ, QUEUE_INT m){

  VEC_ID i, t;
  QUEUE_INT *x,*b;

  if (occ) {
  	for(b = occ->start() ; b < occ->end() ; b++){
			for(x=_T.get_vv(*b); *x < m ; x++){
       	if ( !_sc[*x] ) _OQ[*x].push_back(*b);
			}
  	}
  }
  else{
  	for(VEC_ID t=0;t<_T.get_t();t++){
			for(x=_T.get_vv(t); *x < m ; x++){
       	if ( !_sc[*x] ) _OQ[*x].push_back(t);
      }
  	}
  }

}

/**************************************************************/
/* Find identical transactions in a subset of transactions, by radix-sort like method */
/* infrequent items (refer LCM_occ) and items larger than item_max are ignored */
/* INPUT: T:transactions, occ:subset of T represented by indices, result:array for output, end:largest item not to be ignored */
/* OUTPUT: if transactions i1, i2,..., ik are the same, they have same value in T->mark[i]
 (not all) isolated transaction may have mark 1 */
/* use 0 to end-1 of T->mark, T->jump, T->shift and T->OQ temporary
   T->OQ[i].t and OQ[i].s have to be 0. */
/*************************************************************************/
void TRSACT::find_same (QUEUE *occ, QUEUE_INT end){

  VEC_ID mark=2, t_end;

  QUEUE *o=occ, *EQ, *QQ = _OQ;
  QUEUE_INT *x, *y, e;
  QUEUE_ID ot = occ->get_t();

  // initialization
	for(x=occ->begin(); x <occ->end() ; x++){
		_mark[*x] = mark; 
		_shift[*x] = _T.get_vv(*x); 
	}

  _jump.setEndByStart(); 
  QQ[_T.get_clms()].sClr();

  while (1){

    if ( o->size()  == 1 ){
    	 _mark[o->pop_back()] = 1;  // no same transactions; mark by 1
    }
    if ( o->get_t() == 0 ) goto END;

    // if previously inserted transactions are in different group, 
    // then change their marks with incrementing mark by one
    mark++; 
    for (x=o->start() ; x < o->end() ; x++){
    	_mark[*x] = mark;
    }

    t_end = o->get_t();
    o->posClr();
    
    // insert each t to buckets
    for (x=o->get_v() ; x<o->get_v()+t_end ; x++){
         // get next item in transaction t
      do{

        e = *(_shift[*x]);
        _shift[*x]++;

        if ( e >= end ){ e = _T.get_clms(); break; }

      }while ( _sc[e] );

      EQ = &QQ[e];
      // if previously inserted transactions are in different group, 
      // then change their mark to the transaction ID of top transacion.
      y = EQ->start();

      if ( EQ->posCheck() && _mark[*y] != _mark[*x] ){
        if ( EQ->size() == 1 ){
	        // the tail of the queue has no same transaction; mark the tail by 1
        	_mark[EQ->pop_back()] = 1; 
        }
        else {
          mark++; 
          for ( ; y< EQ->end(); y++){
          	_mark[*y] = mark;
          }
          EQ->setStartByEnd();
        }
      } 
      else if ( EQ->get_t() == 0 && e<_T.get_clms() ){
      	_jump.push_back(e);
      }
      EQ->push_back(*x);  // insert t to bucket of e
    }
    END:;
    if ( _jump.size() == 0 ) break;
    o = &QQ[_jump.pop_back()];
  }

  // same transactions are in queue of item_max
  if ( QQ[_T.get_clms()].size() == 1 ){
  	_mark[QQ[_T.get_clms()].pop_back()] = 1;
  }

  if ( occ != &QQ[_T.get_clms()] ) occ->set_t(ot);

}


void TRSACT::find_same (KGLCMSEQ_QUE *occ, QUEUE_INT end){
  VEC_ID mark=2, t_end;
  KGLCMSEQ_QUE *o=occ, *EQ, *QQ = _OQELE;
  QUEUE_INT *x, e;
  QUEUE_ID ot = occ->get_t();
  // initialization
	for(KGLCMSEQ_ELM *xx=occ->begin(); xx <occ->end() ; xx++){
		_mark[xx->_t] = mark; 
		_shift[xx->_t] = _T.get_vv(xx->_t); 
	}

  _jump.setEndByStart(); 
  QQ[_T.get_clms()].set_s(0);
  int vcnt=0;
	int iii=0;

  while (1){
    if ( o->size()  == 1 ){
    	 _mark[o->pop_back()] = 1;  // no same transactions; mark by 1
    }
    if ( o->get_t() == 0 ) goto END;
    // if previously inserted transactions are in different group, then change their marks with incrementing mark by one
    mark++; 
    for (KGLCMSEQ_ELM *xx=o->start() ; xx < o->end() ; xx++){
    	_mark[xx->_t] = mark;
    }

    t_end = o->get_t();
    o->posClr();
    
    // insert each t to buckets
    for (KGLCMSEQ_ELM *xx=o->begin() ; xx<o->begin()+t_end ; xx++){
			// get next item in transaction t
      do{
        e = *(_shift[xx->_t]);
        _shift[xx->_t]++;
        if ( e >= end ){ e = _T.get_clms(); break; }
      }while ( _sc[e] );

      EQ = &QQ[e];
      // if previously inserted transactions are in different group, 
      // then change their mark to the transaction ID of top transacion.
			KGLCMSEQ_ELM * y = EQ->start();

      if ( EQ->posCheck() && _mark[y->_t] != _mark[xx->_t] ){
        if ( EQ->size() == 1 ){
	        // the tail of the queue has no same transaction; mark the tail by 1
        	_mark[EQ->pop_back()] = 1; 
        }
        else {
          mark++; 
          for ( ; y< EQ->end(); y++){
          	_mark[y->_t] = mark;
          }
          EQ->setStartByEnd();
        }
      } 
      else if ( EQ->get_t() == 0 && e<_T.get_clms() ){
      	_jump.push_back(e);
      }
      EQ->push_back(*xx);  // insert t to bucket of e
    }
    END:;
    if ( _jump.size() == 0 ) break;
    o = &QQ[_jump.pop_back()];
  }

  // same transactions are in queue of item_max
  if ( QQ[_T.get_clms()].size() == 1 ){
		//QQ[_T.get_clms()].dec_t();
  	//_mark[QQ[_T.get_clms()].get_v(QQ[_T.get_clms()].get_t())] = 1;
  	_mark[QQ[_T.get_clms()].pop_back()] = 1;

  }

  if ( occ != &QQ[_T.get_clms()] ) occ->set_t(ot);

}

/****************************************************************************/
/*  copy transaction t to tt (only items i s.t. sc[i]==0)                 **/
/* T->w has to be allocated. itemweight will be alocated even if T->w[t] == NULL */
/****************************************************************************/
void TRSACT::copy ( VEC_ID tt, VEC_ID t, QUEUE_INT end){


  QUEUE_INT *x, *buf;
  WEIGHT *wbuf = NULL, tw = _w[t], *w = _T.exist_w()? _T.get_w(t): NULL;
  int bnum = _buf.get_num(), bblock = _buf.get_block_num(), wflag = (w || (_flag2&TRSACT_UNION));


  buf = (QUEUE_INT *)_buf.get_memory ( _T.get_vt(t)+1);

	if ( _ERROR_MES ) return;
  if ( wflag ){

  	wbuf = (WEIGHT *)_wbuf.get_memory ( _T.get_vt(t)+1);
  	_T.set_w(tt,wbuf);
  }

	if ( _ERROR_MES ){ _buf.set_num(bnum); _buf.set_block_num(bblock); return; }
  _T.set_vv(tt, buf);
  _w[tt] = _w[t];
  if ( _flag2&TRSACT_NEGATIVE ) _pw[tt] = _pw[t];

	for(x=_T.get_vv(t); *((QUEUE_INT *)x)<(end) ; x++){
  // MQUE_MLOOP_CLS (_T.get_v(t), x, end){
    if ( !_sc[*x] ){
      *buf = *x; buf++;
      if ( wflag ){ *wbuf = w? *w: tw; wbuf++; }
    }
    if ( w ) w++;
  }
  _T.set_vt(tt,(VEC_ID)(buf - _T.get_vv(tt)));
  *buf = _T.get_clms();
  _buf.set_num( (int)(buf - ((QUEUE_INT *)_buf.get_base(_buf.get_block_num())) + 1));
  if ( wflag ) _wbuf.set_num( (int)(wbuf - ((WEIGHT *)_wbuf.get_base(_wbuf.get_block_num())) + 1) );
}

/****************************************************************************/
/*  intersection of transaction t and tt (only items i s.t. sc[i]==0)     **/
/*  shift is the array of pointers indicates the start of each transaction **/
/****************************************************************************/
void TRSACT::suffix_and (VEC_ID tt, VEC_ID t){

  QUEUE_INT *x=_shift[tt], *y=_shift[t], *xx=_shift[tt];
  while ( *x < _T.get_clms() && *y < _T.get_clms() ){
    if ( *x > *y ) y++;
    else {
      if ( *x == *y ){
        if ( !_sc[*x] ){ *xx = *x; xx++; }
        y++;
      }
      x++;
    }
  }
  _T.set_vt (tt,(VEC_ID)(xx - _T.get_vv(tt)));
  *xx = _T.get_clms();
  _buf.set_num( (int)(xx - ((QUEUE_INT *)_buf.get_base(_buf.get_block_num())) + 1) );

}


/***************************************************************************/
/*  take union of transaction t to tt (only items i s.t. pw[i]>=th)        */
/* CAUSION: t has to be placed at the last of trsact_buf2.                 */
/*   if the size of t inclreases, the following memory will be overwrited  */
/* if memory (T->buf, T->wbuf) is short, do nothing and return 1           */
/* T->T.w[t] can be NULL, but T->T.w[x] can not                            */
/***************************************************************************/
void TRSACT::itemweight_union (VEC_ID tt, VEC_ID t){

  int bnum = _buf.get_num(), bblock = _buf.get_block_num();

  QUEUE_ID siz = _T.get_vt(tt) + _T.get_vt(t);
  QUEUE_INT *xx_end = _T.get_vv(tt) + siz, *xx = xx_end;
  QUEUE_INT *x = _T.get_vv(tt) + _T.get_vt(tt)-1, *y = _T.get_vv(t) + _T.get_vt(t)-1;

  WEIGHT *ww = _T.get_w(tt) +siz, *wx = _T.get_w(tt) +_T.get_vt(tt)-1, *wy = _T.get_w(t) +_T.get_vt(t)-1;
  WEIGHT tw = _w[t];

  int flag=0, wf = (_T.get_w(t)!=NULL);

    // if sufficiently large memory can not be taken from the current memory block, use the next block
  if ( xx_end >= (QUEUE_INT *)_buf.get_base(_buf.get_block_num()) + _buf.get_block_siz() ){
    xx_end = xx = ((QUEUE_INT*) _buf.get_memory ( _buf.get_block_siz())) +siz;
		if (_ERROR_MES) return;
    ww = ((WEIGHT *)_wbuf.get_memory (_wbuf.get_block_siz())) +siz;
		if ( _ERROR_MES ){ _buf.set_num(bnum); _buf.set_block_num(bblock); return; }
    flag =1;
  }

  if ( _ERROR_MES ) return;

    // take union and store it in the allocated memory
  while ( x >= _T.get_vv(tt) && y >= _T.get_vv(t) ){
    if ( *x > *y ){
      if ( !_sc[*x] ){ *xx = *x; *ww = *wx; xx--; ww--; }
      x--; wx--;
      if ( x < _T.get_vv(tt) ){ 
        while ( y >= _T.get_vv(t) ){
          if ( !_sc[*y] ){ *xx = *y; *ww = wf? *wy: tw;  xx--; ww--; }
          y--; wy--;
        }
      }
    } else {
      if ( !_sc[*y] ){
        *ww = wf? *wy: tw; *xx = *y;
        if ( *x == *y ){ *ww += *wx; x--; wx--; }
        xx--; ww--;
      }
      y--; wy--;
      if ( y < _T.get_vv(t) ){
        while ( x >= _T.get_vv(tt) ){
          if ( !_sc[*x] ){ *xx = *x; *ww = *wx; xx--; ww--; }
          x--; wx--;
        }
      }
    }
  }
  _T.set_vt(tt, (VEC_ID)(xx_end -xx));
  
    // if [tt].v will overflow, set [tt].v to the top of next memory block
  if ( flag ){
    if ( _T.get_vv(tt) + _T.get_vt(tt)+1 >= (QUEUE_INT *)_buf.get_base(_buf.get_block_num()-1) +_buf.get_block_siz() ){
      _T.set_vv(tt, (QUEUE_INT *)_buf.get_base(_buf.get_block_num()));
      _T.set_w(tt,(WEIGHT *)_wbuf.get_base(_wbuf.get_block_num()));
    } else {  // new memory block is allocated, but the transaction fits in the previous block
      _buf.dec_block_num();
      _wbuf.dec_block_num();
    }
  }
    
    // copy the union to the original position
  for ( x=_T.get_vv(tt),wx=_T.get_w(tt) ; xx<xx_end ; ){
    xx++; ww++;
    *x = *xx; *wx = *ww;
    x++; wx++;
  }
  *x = _T.get_clms();
  int numtmp = (int)(x - ((QUEUE_INT *)_buf.get_base(_buf.get_block_num())) +1);
  _wbuf.set_num(numtmp);
  _buf.set_num(numtmp);
  return;

}



void TRSACT::removeDupTrsacts(){

  VEC_ID mark = 0, tt=0;
  QUEUE_INT *x;

 	QUEUE *o = &_OQ[_T.get_clms()];

	for(QUEUE_INT * x = o->begin() ; x < o->end() ; x++){

    if ( mark == _mark[*x] ){
      _mark[*x] = 0;   // mark of unified (deleted) transaction
      _w[tt] += _w[*x]; if ( _pw ) _pw[tt] += _pw[*x];
    }

		// *x is not the same to the previous, or memory short 
    if ( mark != _mark[*x] && _mark[*x] > 1 ){
      mark = _mark[*x];
			tt = *x;
      _mark[*x] = tt+2;
    }

  }
  o->posClr();
}

/*****/
/* merge duplicated transactions in occ according to those having same value in T->mark
   the mark except for the representative will be zero, for each group of the same transactions
   the mark of the representative will be its (new) ID +2 (one is for identical transaction) */
/* T->flag2&TRSACT_MAKE_NEW: make new trsact for representative
   T->flag2&TRSACT_INTSEC: take suffix intersection of the same trsacts
   T->flag2&TRSACT_UNION: take union of the same trsacts */
/* o will be cleard after the execution */
void TRSACT::merge_trsact (QUEUE_INT end){

  VEC_ID mark = 0, tt=0;
  QUEUE_INT *x;
 	QUEUE *o = &_OQ[_T.get_clms()];

	for(QUEUE_INT * x = o->begin() ; x < o->end() ; x++){

    if ( mark == _mark[*x] ){
      _mark[*x] = 0;   // mark of unified (deleted) transaction
      _w[tt] += _w[*x]; if ( _pw ) _pw[tt] += _pw[*x];
      if ( _flag2 & TRSACT_INTSEC ){
        suffix_and(tt, *x);
        _buf.set_num( (int)(_T.get_vv(tt) - (QUEUE_INT *)_buf.get_base(_buf.get_block_num())  +_T.get_vt(tt) +1) );
      }
      if ( _flag2 & TRSACT_UNION ){
        itemweight_union (tt, *x);
        if ( _ERROR_MES ) _mark[*x] = *x+2; // do not merge if not enough memory
      }
    }

		// *x is not the same to the previous, or memory short 
    if ( mark != _mark[*x] && _mark[*x] > 1 ){
    
      mark = _mark[*x];

      if ( _flag2&TRSACT_MAKE_NEW ){

        tt = _new_t++;

        copy ( tt, *x, (_flag2&(TRSACT_INTSEC+TRSACT_UNION))? _T.get_clms(): end);

        if( _ERROR_MES ){ _new_t--; tt = *x; }
        else { for (_shift[tt]=_T.get_vv(tt) ; *(_shift[tt])<end ; _shift[tt]++);}

      } 
      else{
      	tt = *x;
      }
      _mark[*x] = tt+2;

    }

  }
  o->posClr();
}


void TRSACT::removeDupTrsacts(KGLCMSEQ_QUE *o){

  VEC_ID mark = 0, tt=0;
  QUEUE_INT x;

	for(KGLCMSEQ_ELM * xx = o->begin() ; xx < o->end() ; xx++){
		x =xx->_t;
    if ( mark == _mark[x] ){
      _mark[x] = 0;   // mark of unified (deleted) transaction
      _w[tt] += _w[x]; if ( _pw ) _pw[tt] += _pw[x];
    }

		// *x is not the same to the previous, or memory short 
    if ( mark != _mark[x] && _mark[x] > 1 ){
    
      mark = _mark[x];
    	tt = x;
      _mark[x] = tt+2;
    }
  }
  o->posClr();
}

void TRSACT::merge_trsact( KGLCMSEQ_QUE *o, QUEUE_INT end){

  VEC_ID mark = 0, tt=0;
  QUEUE_INT x;

	for(KGLCMSEQ_ELM * xx = o->begin() ; xx < o->end() ; xx++){
		x =xx->_t;
    if ( mark == _mark[x] ){
      _mark[x] = 0;   // mark of unified (deleted) transaction
      _w[tt] += _w[x]; if ( _pw ) _pw[tt] += _pw[x];
      if ( _flag2 & TRSACT_INTSEC ){
        suffix_and (tt, x);
        _buf.set_num( (int)(_T.get_vv(tt) - (QUEUE_INT *)_buf.get_base(_buf.get_block_num())  +_T.get_vt(tt) +1) );
      }
      if ( _flag2 & TRSACT_UNION ){
        itemweight_union (tt, x);
        if ( _ERROR_MES ) _mark[x] = x+2; // do not merge if not enough memory
      }
    }

		// *x is not the same to the previous, or memory short 
    if ( mark != _mark[x] && _mark[x] > 1 ){
    
      mark = _mark[x];

      if ( _flag2&TRSACT_MAKE_NEW ){

        tt = _new_t++;

        copy ( tt, x, (_flag2&(TRSACT_INTSEC+TRSACT_UNION))? _T.get_clms(): end);

        if( _ERROR_MES ){ _new_t--; tt = x; }
        else { for (_shift[tt]=_T.get_vv(tt) ; *(_shift[tt])<end ; _shift[tt]++);}

      } 
      else{
      	tt = x;
      }
      _mark[x] = tt+2;

    }

  }
  o->posClr();
}

/* remove the unified transactions from occ (consider T->occ_unit) */
void TRSACT::reduce_occ(QUEUE *occ){

  QUEUE_INT *x, *y=occ->begin();
  QUEUE_ID i=0;

	for( x = occ->begin(); x < occ->end() ; x++){

		if ( _mark[*x] == 0 ) continue;

		*y = _mark[*x] > 1 ? _mark[*x]-2 : *x;
		y++; 
		i++;
  } 
  occ->resize(i);
}




/***********************************/
/*  DEBUG print transactions            */
/***********************************/ 
/* print the profiles of the transaction database */
void TRSACT::print ( QUEUE *occ, PERM *p){
  VEC_ID i, t;
  QUEUE_ID j;
  QUEUE_INT e;

  for(i=0; i< ( occ? occ->get_t(): _T.get_t());i++){

    t = occ? *((QUEUE_INT *)occ->getp_v(i)): i;
    if ( occ ) printf (QUEUE_INTF "::: ", t);
    for (j=0; j<_T.get_vt(t) ; j++){
      e = _T.get_vv(t,j);
      printf (QUEUE_INTF, p? p[e]: e);
      //if ( T->T.w ) printf ("(" WEIGHTF ")", T->T.w[t][j]);
      printf (",");
    }
    //if ( T->w ) printf (" :" WEIGHTF " ", T->w[t]);
    //printf (" (" QUEUE_INTF ")\n", T->T.v[t].end);
  }
}




