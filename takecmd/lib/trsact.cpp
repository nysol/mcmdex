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

/***********************************/
/*   print transactions            */
/***********************************/
void TRSACT::print ( QUEUE *occ, PERM *p){
  VEC_ID i, t;
  QUEUE_ID j;
  QUEUE_INT e;
  FLOOP (i, 0, occ? occ->get_t(): _T.get_t()){
    t = occ? *((QUEUE_INT *)occ->getp_v(i*_occ_unit)): i;
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

/* print the profiles of the transaction database */
void TRSACT::prop_print (){
  if ( !(_flag & SHOW_MESSAGE) ) return;
  print_err ("trsact: %s", _fname);
  if(_fname2){  print_err(" ,2nd-trsact %s (from ID %d)", _fname2, _end1); }

///* あとで戻す
  print_err (" ,#transactions %d ,#items %d ,size %zd", _C.rows(), _C.clms(),_C.eles());
//*/

  print_err (" extracted database: #transactions %d ,#items %d ,size %zd", _T.get_t(), _T.get_clms(), _T.get_eles());
/*
  print_fname (" ,weightfile %s", _wfname);

  print_fname (" ,itemweightfile %s", _item_wfname);

  print_fname (" ,item-order-file %s", _pfname);
*/
  // print_fname (" ,2nd-itemweightfile %s", _item_wfname2);
  // print_fname (" ,2nd-weightfile %s", _wfname2);

  print_err ("\n");
}


/**************************************************************/
void TRSACT::end (){
/*
  if ( _OQ ){ free2 (_OQ->_v ); free2 (_OQ[_T.get_clms()]._v); }
  //free2 (_T._w);
  //_T.end();
  if ( _w != _pw ) free2 (_pw);
  mfree (_w, _perm, _trperm);
  mfree (_mark, _shift, _sc, _OQ, _head, _strID);
  _jump.end ();
  // _buf.end ();
  //_wbuf.end();
  //TRSACT_init (T);
*/
}

#ifndef TRSACT_MAXNUM 
 #define TRSACT_MAXNUM 20000000LL
#endif

/*****************************************/
/* scan file "fp" with weight file wfp and count #items, #transactions in the file. */
/*   count weight only if wfp!=NULL                                      */
/* T->rows_org, clms_org, eles_org := #items, #transactions, #all items  */
/*   ignore the transactions of size not in range T->clm_lb - clm_ub     */ 
/* T->total_w, total_pw := sum of (positive) weights of transactions     */
/* C->clmt[i],C->cw[i] := the number/(sum of weights) of transactions including i  */
/****************************************/
int TRSACT::file_count ( FILE_COUNT *C, FILE2 *fp, char *wf){
/*  QUEUE_INT i, item, kk=0, k, jump_end=0;
  WEIGHT w, s;
  VEC_ID *jump=NULL;
  //FILE2 wfp = INIT_FILE2;
  FILE2 wfp;

  LONG jj;

  if ( wf ){
	  wfp.open(wf, "r");

		#ifdef WEIGHT_DOUBLE
		  kk = wfp.ARY_Scan_DBL(1);
		#else
		  kk = wfp.ARY_Scan_INT(1);
		#endif

    kk += _rows_org;
    realloc2 (C->rw, kk+1, goto ERR);
    wfp.reset();
    wfp.ARY_Read(C->rw, kk);
    ARY_MIN (w, i, C->rw, 0, kk);
    if ( w<0 ) _flag2 |= TRSACT_NEGATIVE;
    wfp.close();
  }
  do {
    s=0;
    k=0;
    w = wf? (_rows_org<kk? C->rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;
    do {
      jj = fp->read_int();
      item = (QUEUE_INT)jj;
      if ( (FILE_err&4)==0 && jj<TRSACT_MAXNUM && jj>=0 ){
        ENMAX (_clms_org, item+1);  // update #items
        reallocx (jump, jump_end, k, 0, goto ERR);
        jump[k] = item;
        k++;
        s += wf? (item<kk? MAX(C->rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

           // count/weight-sum for the transpose mode
        reallocx (C->clmt, C->clm_end, item, 0, goto ERR);
        C->clmt[item]++;
        if ( !(_flag&LOAD_TPOSE) ){
          reallocx (C->cw, C->cw_end, item, 0, goto ERR);
          C->cw[item] += MAX(w,0);    // sum up positive weights
        }
      }
    } while ( (FILE_err&3)==0);

       // count/weight-sum for the transpose mode
    reallocx (C->rowt, C->row_end, _rows_org, 0, goto ERR);
    C->rowt[_rows_org] = k;
    if ( _flag&LOAD_TPOSE ){
      reallocx (C->cw, C->cw_end, _rows_org, 0, goto ERR);
      C->cw[_rows_org] = s;    // sum up positive weights
    }
    if ( k==0 && FILE_err&2 ) break;
    _rows_org++;  // increase #transaction
    
    if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items
    if ( k==0 ){
       _str_num++;  // increase #streams if empty transaction is read
    } else {
      _eles_org += k;
      if ( (!(_flag&LOAD_TPOSE) && !RANGE (_row_lb, k, _row_ub))
          || ((_flag&LOAD_TPOSE) && (!RANGE(_w_lb, s, _w_ub) || !RANGE (_clm_lb, k, _clm_ub)) ) ) FLOOP (i, 0, k) C->clmt[jump[i]]--; 
    }
  } while ( (FILE_err&2)==0);

  free2 (jump);
    // swap the variables in transpose mode
  if ( C->rw == NULL ){ _total_w_org = _total_pw_org = _rows_org; return 0; } 
  C->clm_btm = MIN(kk, _rows_org);
  reallocx (C->rw, kk, _rows_org, TRSACT_DEFAULT_WEIGHT, goto ERR);
  FLOOP (k, 0, _rows_org){
    _total_w_org += C->rw[k];
    _total_pw_org += MAX(C->rw[k],0);
  }
  return 0;
  ERR:;
  wfp.close();
  mfree (C->rw, C->cw, C->clmt, C->rowt, jump);
*/
  return 1;

}

/* allocate memory, set permutation, and free C.clmt,rowt,rw,cw */
int TRSACT::alloc (FILE_COUNT *C){
/*
  VEC_ID t, tt=0, ttt, ttt_max, h, flag, org;
  FILE_COUNT_INT *ct;
  size_t s=0;
  PERM *q, *p=NULL;
  char *buf;

    // swap variables in the case of transpose
  if ( _flag & LOAD_TPOSE ){
	  QUEUE_INT swap_tmp = _clms_org;
		_clms_org = (QUEUE_INT)_rows_org;
		_rows_org = (VEC_ID)swap_tmp;
		
    //common_QUEUE_INT = _clms_org; _clms_org = (QUEUE_INT)_rows_org; _rows_org = (VEC_ID)common_QUEUE_INT;
    SWAP_PNT (C->clmt, C->rowt);
    //SWAP_<FILE_COUNT_INT*> (C->clmt, C->rowt);
  }
  ttt_max = ttt = _clms_org;

  // set lower/upper bounds if it is given by the ratio
  if ( _row_lb_ ) _row_lb = _rows_org * _row_lb_;
  if ( _row_ub_ ) _row_ub = _rows_org * _row_ub_;
  if ( _clm_lb_ ) _clm_lb = _clms_org * _clm_lb_;
  if ( _clm_ub_ ) _clm_ub = _clms_org * _clm_ub_;

  if ( _flag2&TRSACT_SHRINK ) _flag |= LOAD_DBLBUF;

  // count valid columns/elements
  if ( _pfname && !(_flag2&TRSACT_WRITE_PERM) ){
    //ARY_LOAD (p, QUEUE_INT, ttt, _pfname, 1, EXIT0);
		ttt = FILE2::ARY_Load (p, _pfname, 1);
    ARY_MAX (ttt_max, tt, p, 0, ttt);
    _T.set_clms(ttt_max+1);
  }
  else {
    if ( _flag&LOAD_PERM ){
      if ( _flag2&TRSACT_FRQSORT )
        p = qsort_perm_<WEIGHT> (C->cw, _clms_org, (_flag&LOAD_INCSORT)?1:-1);
      else 
      	p = qsort_perm_<FILE_COUNT_INT> (C->clmt, _clms_org, (_flag&LOAD_INCSORT)?1:-1);
    }
    if ( _pfname ) {
    	// ARY_WRITE (_pfname, p, _clms_org, PERMF " ", EXIT0);
    	FILE *fp;
			fopen2(fp,_pfname,"w",EXIT0);
			for( size_t i=0 ; i< _clms_org ;i++){
				fprintf(fp, PERMF " " ,p[common_size_t]);
			}
			fputc('\n',fp);
			fclose(fp);

    }

  }

  _clms_end = MAX (_clms_org, _T.get_clms());

  //ARY_FILL (C->cperm, 0, _clms_org, _clms_org+1);
  malloc2 (C->cperm, _clms_org+1, EXIT0);
	for(size_t i =0 ;i<_clms_org;i++){ 
		C->cperm[i] = _clms_org+1; 
	}



  FLOOP (t, 0, ttt){
    tt = p? p[t]: t;
    if ( tt >= _clms_org ) continue;
    if ( RANGE(_w_lb, C->cw[tt], _w_ub) && RANGE (_clm_lb, C->clmt[tt], _clm_ub)){
      s += C->clmt[tt];
      C->cperm[tt] = (_pfname && !(_flag2&TRSACT_WRITE_PERM))? t: _T.postinc_clms();
    } else C->cperm[tt] = _clms_end+1;
  }
  free2 (p);

  if ( _T.get_clms() == 0 ) error ("there is no frequent item", return 0);

    // count valid rows/elements
  if ( _flag&(LOAD_SIZSORT+LOAD_WSORT) ){
    if ( _flag&LOAD_WSORT && C->rw )
      p = qsort_perm_<WEIGHT> (C->rw, _rows_org, (_flag&LOAD_DECROWSORT)?-1:1);
    else 
    	p = qsort_perm_<FILE_COUNT_INT> (C->rowt, _rows_org, (_flag&LOAD_DECROWSORT)?-1:1);
  }

  malloc2 (C->rperm, _rows_org, EXIT0);
  FLOOP (t, 0, _rows_org){  // compute #elements according to rowt, and set rperm
    tt = p? p[t]: t;
    if ( RANGE (_row_lb, C->rowt[tt], _row_ub) ){
      C->rperm[tt] = _T.postinc_t();
      _T.add_eles(C->rowt[t]);
    } else C->rperm[tt] = _rows_org+1;
  }

  free2 (p); 
  free2 (C->cw);

  flag = (_T.get_eles() > s && !(_flag & LOAD_TPOSE) );
  if ( flag ) _T.set_eles(s);
  _T.set_ele_end(_T.get_eles());

  _T.set_end(_T.get_t() * ((_flag&LOAD_DBLBUF)? 2: 1)+1);
  malloc2 (_w, _T.get_end(), EXIT0);

  if ( TRSACT_NEGATIVE ) malloc2 (_pw, _T.get_end(), EXIT0);
  else _pw = NULL;

  malloc2 (_trperm, _T.get_t(), EXIT0);


//  malloc2 (_T._v, _T.get_end(), EXIT0);
//  malloc2 (buf, (_T.get_eles()+_T.get_end()+1)*_T.get_unit(), EXIT0);
//  _T.set_buf((QUEUE_INT *)buf);

	_T.alloc_v();
  _T.alloc_buf();

	//要確認
  buf = (char *)_T.get_buf();



  calloc2 (_perm, _T.get_clms()+1, EXIT0);
  _jump.alloc(_T.get_clms()+1);
  _buf.alloc (sizeof(QUEUE_INT), MAX((int)_row_max*4,(int)(_T.get_eles()+1)/10+_T.get_clms()+100));
  _wbuf.alloc(sizeof(WEIGHT), MAX((int)_row_max*4,(int)(_T.get_eles()+1)/10+_T.get_clms()+100));

  if ( _flag2&TRSACT_SHRINK ){
    malloc2 (_mark, _T.get_end(), EXIT0);
    malloc2 (_shift, _T.get_end(), EXIT0);
    calloc2 (_sc, _T.get_clms(), EXIT0);
  }
  
  //if ( _flag2&TRSACT_MULTI_STREAM ){
  //  malloc2 (_head, _str_num+2, EXIT0);
  //  malloc2 (_strID, (_flag&LOAD_TPOSE)?_T.get_clms():_T.get_end(), EXIT0);
  //}
  if ( !_T.exist_w() && (_flag2&TRSACT_UNION)) {
  	WEIGHT **wtmp = _T.get_w();
  	calloc2 (wtmp, _T.get_end(), EXIT0);
  }
	if ( !_T.exist_w() && _item_wfname ) _T.alloc_weight (C->rowt);


	if ( ERROR_MES ) return(0);

  // set variables w.r.t rows
  tt=0; FLOOP (t, 0, _rows_org){
    if ( C->rperm[t] <= _rows_org ){
      _T.init_v(tt);// = INIT_QUEUE;
      _trperm[tt] = t;
      C->rperm[t] = tt;
      _w[tt] = C->rw? C->rw[t]: 1;
      if ( _pw ) _pw[tt] = MAX (_w[tt], 0);
      if ( !flag ){
        _T.set_vv(tt,(QUEUE_INT *)buf);
        buf += (C->rowt[t]+1)*_T.get_unit();
      }
      tt++;
    }
  }
  free2 (C->rw);

  // make the inverse perm of items
  FLOOP (t, 0, _clms_org)
      if ( C->cperm[t] <= _clms_end ) _perm[C->cperm[t]] = t;

  // set head of each stream, and stream ID of each transaction

  //if ( _flag2&TRSACT_MULTI_STREAM ){
  //  malloc2 (_head, _str_num+2, EXIT0);
  //  malloc2 (_strID, (_flag&LOAD_TPOSE)?_T.get_clms():_T.get_end(), EXIT0);
  //}

  org = (_flag&LOAD_TPOSE)? _clms_org: _rows_org;
  q = (_flag&LOAD_TPOSE)? C->cperm: C->rperm;
  ct = (_flag&LOAD_TPOSE)? C->clmt: C->rowt;

  h=1; tt=0; FLOOP (t, 0, org){
    if ( q[t] <= org ){
      if ( t == _end1 && _sep==0 ) _sep = tt;
      if ( t == _sep && _sep>0 ) _sep = tt;
      //if ( _strID ) _strID[tt] = h;
      tt++;
    }
    if ( _head && ct[t]==0 ) _head[h++] = tt+1;
  }

  _new_t = _T.get_t();

  return ( flag );
  */
 	return 1; 
}



/* allocate memory, set permutation, and free C.clmt,rowt,rw,cw */
int TRSACT::talloc (){

  VEC_ID t, tt=0, ttt, ttt_max, h, flag, org;
  FILE_COUNT_INT *ct;
  size_t s=0;
  PERM *q, *p=NULL;
  char *buf;

    // swap variables in the case of transpose
  if ( _flag & LOAD_TPOSE ){
  	_C.tpose();
  }
  ttt_max = ttt = _C.clms();

  // set lower/upper bounds if it is given by the ratio
  if ( _row_lb_ ) _row_lb = _C.rows() * _row_lb_;
  if ( _row_ub_ ) _row_ub = _C.rows() * _row_ub_;
  if ( _clm_lb_ ) _clm_lb = _C.clms() * _clm_lb_;
  if ( _clm_ub_ ) _clm_ub = _C.clms() * _clm_ub_;

  if ( _flag2&TRSACT_SHRINK ) _flag |= LOAD_DBLBUF;

  // count valid columns/elements
  if ( _pfname && !(_flag2&TRSACT_WRITE_PERM) ){

    //ARY_LOAD (p, QUEUE_INT, ttt, _pfname, 1, EXIT0);
		ttt = FILE2::ARY_Load (p, _pfname, 1);
    ARY_MAX (ttt_max, tt, p, 0, ttt);
    _T.set_clms(ttt_max+1);


  }
  else {

    if ( _flag&LOAD_PERM ){

      if ( _flag2&TRSACT_FRQSORT ){
      	p = _C.clmw_perm_sort((_flag&LOAD_INCSORT)?1:-1);
        //p = qsort_perm_<WEIGHT> (C->cw, _clms_org, (_flag&LOAD_INCSORT)?1:-1);
      }
      else {
      	p = _C.clmt_perm_sort((_flag&LOAD_INCSORT)?1:-1);
      	//p = qsort_perm_<FILE_COUNT_INT> (C->clmt, _clms_org, (_flag&LOAD_INCSORT)?1:-1);
      }
    }
    if ( _pfname ) {
    	// ARY_WRITE (_pfname, p, _clms_org, PERMF " ", EXIT0);
    	FILE *fp;
			fopen2(fp,_pfname,"w",EXIT0);
			for( size_t i=0 ; i< _C.clms() ;i++){
				fprintf(fp, PERMF " " ,p[i]);
			}
			fputc('\n',fp);
			fclose(fp);
    }

  }

  _clms_end = MAX (_C.clms(), _T.get_clms());


	_C.cpermFILL();
	// この部分をまとめる
  FLOOP (t, 0, ttt){
    tt = p? p[t]: t;
    if ( tt >= _C.clms() ) continue;
    // if ( RANGE(_w_lb, C->cw[tt], _w_ub) && RANGE (_clm_lb, C->clmt[tt], _clm_ub)){
    //printf("xx %f %f %d %d\n",_w_lb,_w_ub,_clm_lb,_clm_ub);

    if( _C.RangeChecnkC(tt,_w_lb,_w_ub,_clm_lb,_clm_ub) ){
      s += _C.get_clmt(tt);
       // <<== _T.postinc_clmsどうにかして
      _C.set_cperm( tt, (_pfname && !(_flag2&TRSACT_WRITE_PERM))? t: _T.postinc_clms() );
    } else {
    	 _C.set_cperm( tt , _clms_end+1);
    }
  }
  free2 (p);
  // ここまでのまとめたほうがいい cpemへのセット

  if ( _T.get_clms() == 0 ) error ("there is no frequent item", exit(0));

  // ここから ================================
	// count valid rows/elements
  if ( _flag&(LOAD_SIZSORT+LOAD_WSORT) ){
    if ( _flag&LOAD_WSORT ){
    	p =_C.roww_perm_sort((_flag&LOAD_DECROWSORT)?-1:1);
      //p = qsort_perm_<WEIGHT> (C->rw, _rows_org, (_flag&LOAD_DECROWSORT)?-1:1);
    }	else {
      p =_C.rowt_perm_sort((_flag&LOAD_DECROWSORT)?-1:1);
    	//p = qsort_perm_<FILE_COUNT_INT> (C->rowt, _rows_org, (_flag&LOAD_DECROWSORT)?-1:1);
    }
  }
	_C.rpermFILL();
	// 下もまとめる  
  FLOOP (t, 0, _C.rows()){  // compute #elements according to rowt, and set rperm
    tt = p? p[t]: t;
    if ( _C.RangeChecnkR (tt, _row_lb, _row_ub) ){
      _C.set_rperm(tt, _T.postinc_t());
      _T.add_eles(_C.get_rowt(t));
    } else{
    	_C.set_rperm( tt,_C.rows()+1);
    }
  }
  free2 (p); 
  // ここまでのまとめたほうがいい rpemへのセット

  flag = (_T.get_eles() > s && !(_flag & LOAD_TPOSE) );
  if ( flag ){ _T.set_eles(s); }

  _T.set_ele_end(_T.get_eles());

  _T.set_end(_T.get_t() * ((_flag&LOAD_DBLBUF)? 2: 1)+1);
  malloc2 (_w, _T.get_end(), EXIT0);

  if ( TRSACT_NEGATIVE ) malloc2 (_pw, _T.get_end(), EXIT0);
  else _pw = NULL;

  malloc2 (_trperm, _T.get_t(), EXIT0);

/*
  malloc2 (_T._v, _T.get_end(), EXIT0);
  malloc2 (buf, (_T.get_eles()+_T.get_end()+1)*_T.get_unit(), EXIT0);
  _T.set_buf((QUEUE_INT *)buf);
*/
	_T.alloc_v();
  _T.alloc_buf();

	//要確認
  buf = (char *)_T.get_buf();

  calloc2 (_perm, _T.get_clms()+1, EXIT0);

  _jump.alloc(_T.get_clms()+1);
  _buf.alloc (sizeof(QUEUE_INT), MAX((int)_row_max*4,(int)(_T.get_eles()+1)/10+_T.get_clms()+100));
  _wbuf.alloc(sizeof(WEIGHT), MAX((int)_row_max*4,(int)(_T.get_eles()+1)/10+_T.get_clms()+100));

  if ( _flag2&TRSACT_SHRINK ){
    malloc2 (_mark, _T.get_end(), EXIT0);
    malloc2 (_shift, _T.get_end(), EXIT0);
    calloc2 (_sc, _T.get_clms(), EXIT0);
  }
  /*
  if ( _flag2&TRSACT_MULTI_STREAM ){
    malloc2 (_head, _str_num+2, EXIT0);
    malloc2 (_strID, (_flag&LOAD_TPOSE)?_T.get_clms():_T.get_end(), EXIT0);
  }*/
  if ( !_T.exist_w() && (_flag2&TRSACT_UNION)) {
  	//WEIGHT **wtmp = _T.get_w();
  	//calloc2 (wtmp, _T.get_end(), EXIT0);
		_T.alloc_w();

  }
	if ( !_T.exist_w() && _item_wfname ) _T.alloc_weight ( _C.getp_rowt());


	if ( ERROR_MES ) return(0);

  // set variables w.r.t rows
  tt=0; 
	FLOOP (t, 0, _C.rows()){
    if ( _C.get_rperm(t) <= _C.rows() ){
      _T.init_v(tt);// = INIT_QUEUE;
      _trperm[tt] = t;
      _C.set_rperm(t, tt);
      _w[tt] = _C.get_rw(t);
      if ( _pw ) _pw[tt] = MAX (_w[tt], 0);
      if ( !flag ){
        _T.set_vv(tt,(QUEUE_INT *)buf);
        buf += (_C.get_rowt(t)+1)*_T.get_unit();
      }
      tt++;
    }
  }
  // free2 (C->rw);

  // make the inverse perm of items
  FLOOP (t, 0, _C.clms())
      if ( _C.get_cperm(t) <= _clms_end ) _perm[_C.get_cperm(t)] = t;

  // set head of each stream, and stream ID of each transaction
  /*
  if ( _flag2&TRSACT_MULTI_STREAM ){
    malloc2 (_head, _str_num+2, EXIT0);
    malloc2 (_strID, (_flag&LOAD_TPOSE)?_T.get_clms():_T.get_end(), EXIT0);
  }*/

  //org = (_flag&LOAD_TPOSE)? _clms_org: _rows_org;
  //q = (_flag&LOAD_TPOSE)? C->cperm: C->rperm;
  //ct = (_flag&LOAD_TPOSE)? C->clmt: C->rowt;

	
  //h=1; tt=0; 
  //FLOOP (t, 0, org){
  //  if ( q[t] <= org ){
  //    if ( t == _end1 && _sep==0 ) _sep = tt;
  //    if ( t == _sep && _sep>0 ) _sep = tt;
  //    //if ( _strID ) _strID[tt] = h;
  //    tt++;
  // }
  //  //if ( _head && ct[t]==0 ) _head[h++] = tt+1;
  //}
	_sep = _C.adjust_sep(_sep,_end1,_flag&LOAD_TPOSE);

  _new_t = _T.get_t();

  return ( flag );
}


/* load the file to allocated memory according to permutation */
void TRSACT::file_read (FILE2 *fp, FILE_COUNT *C, VEC_ID *t, int flag, char *iwfname){

  QUEUE_ID tt;
  LONG x, y;
  FILE2 wfp;
  WEIGHT w=0;
  int fc=0, FILE_err_=0;

  fp->reset ();
  if ( _flag&(LOAD_NUM+LOAD_GRAPHNUM) ) fp->read_until_newline ();
  if ( iwfname ) wfp.open ( iwfname, "r");
  if ( flag ) { _T.set_vv(0, _T.get_buf()); }

  do {

    if ( flag ){
    	// ここどうにかしたい

      if ( C->get_rperm(*t) < C->rows() ){
        if ( C->get_rperm(*t) > 0 ) { 
	        //_T._v[C->rperm[*t]].set_v ( _T._v[C->rperm[*t]-1].get_v() + _T._v[C->rperm[*t]-1].get_t() +1);
  	      _T.set_vv ( C->get_rperm(*t) , _T.get_vv(C->get_rperm(*t)-1) + _T.get_vt(C->get_rperm(*t)-1)+1);
          
        }
      }
    }

    x = *t;
    FILE_err_ = fp->read_item (iwfname?&wfp:NULL, &x, &y, &w, fc, _flag);


    if ( FILE_err&4 ) goto LOOP_END;


    if ( C->get_rperm(x)<=C->rows() && C->get_cperm(y)<=_clms_end ){

      if ( iwfname ) _T.set_w( C->get_rperm(x),_T.get_vt(C->get_rperm(x)), w);

      _T.INS_v( C->get_rperm(x) ,C->get_cperm(y));

    }

    if ( FILE_err&3 ){
      LOOP_END:;
      (*t)++;
      fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
    }
  } while ( (FILE_err&2)==0 );

	_T.set_vv_all();

  if ( iwfname ) wfp.close ();

}

/* sort the transactions and items according to the flag, allocate OQ, and database reduction */
/* causion! not adopt for itemweights!!!!! */
void TRSACT::sort ( FILE_COUNT *C, int flag){
  VEC_ID t, *p;
  int f;
  PERM pp;
  QUEUE Q;
  QUEUE_ID i;
  WEIGHT *ww;

	_T.set_vv_all();
  //FLOOP (t, 0, _T.get_t()) _T._v[t].set_v( _T._v[t].get_t() , _T.get_clms());

  flag = (_flag&(LOAD_SIZSORT+LOAD_WSORT)? ((_flag&LOAD_DECROWSORT)? -1:1):0) *sizeof(QUEUE);

  if ( flag ){   // sort rows for the case that some columns are not read
	  //FLOOP (t, 0, _rows_org){  // compute #elements according to rowt, and set rperm
	  //	printf("%d\n",C->rperm[t]);
	  //}

   // qsort_perm__VECt ((VEC *)_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions
		//qsort_perm__<VEC> ((VEC *)_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions
		//qsort_perm__VECt(_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions
		//qsort_perm__VECt((VEC *)_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions

		//qsort_perm__(_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactionsx

		// friendにする？
		_T.qsort_perm(C->get_rperm(), flag);


		_T.any_INVPERMUTE( C->get_rperm());
    //ARY_INVPERMUTE (_T._v, C->rperm, Q, _T.get_t(), EXIT);  // sort transactions
    if ( _T.exist_w() ) {
    	ARY_INVPERMUTE (_T.get_w(), C->get_rperm(), ww, _T.get_t(), EXIT); // sort rows of itemweighs 
    }
    ARY_INVPERMUTE_ (_trperm, C->get_rperm(), pp, _T.get_t()); 
  }

  //free2 (C->rperm); free2 (C->cperm);


  if ( _flag & LOAD_PERM ) flag = 1;
  else flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);

  if ( flag ){
    FLOOP (t, 0, _T.get_t()) qsort_<QUEUE_INT> (_T.get_vv(t), _T.get_vt(t), flag);
  }
  
  if ( _flag & LOAD_RM_DUP ){
    FLOOP (t, 0, _T.get_t()){
    	if(_T.get_vt(t)>1){
				INT cmmn_INT=1;
				for (INT cmmn_INT2=1 ; cmmn_INT2<_T.get_vt(t) ; cmmn_INT2++){

					if ( _T.get_vv(t,cmmn_INT2-1) != _T.get_vv(t,cmmn_INT2) )
  					_T.set_vv( t, cmmn_INT++ , _T.get_vv(t,cmmn_INT2));
				}
				_T.set_vt(t,cmmn_INT);
			}
			_T.set_vv(t, _T.get_vt(t), _T.get_clms()); 
		}
  }

	//ST_MAX(m,i,S,a,x,y)   
  //ST_MAX (_row_max, i, _T._v, _t, 0, _T.get_t());
	// QUEUEのメソッド化
	_row_max=_T.get_vt(0);
	i=0;
	INT cmmn_INT;
	FLOOP(cmmn_INT,1,_T.get_t()){
		if(_row_max <_T.get_vt(cmmn_INT)){
			i=cmmn_INT;
			_row_max=_T.get_vt(i);
		}
	}

  if ( _flag2&(TRSACT_ALLOC_OCC+TRSACT_SHRINK) ){


    calloc2 (p, _T.get_clms(), EXIT);
    
    // QUEUE_delivery (NULL, p, NULL, _T._v, NULL, _T._t, _T._clms);
		//===================
		VEC_ID iv, ev;
	  QUEUE_INT *x;
		for (iv=0 ; iv<_T.get_t() ; iv++){
    	ev =  iv;
    	MLOOP (x, _T.get_vv(ev), _T.get_clms()) p[*x]++;
		}
		//===================
    ARY_MAX (_clm_max, i, p, 0, _T.get_clms());

		
    Mque_alloc(p);

   	//QUEUE_alloc (&_OQ[_T._clms], MAX(_T._t, _clm_max));
		_OQ[_T.get_clms()].alloc( MAX(_T.get_t(), _clm_max));

    FLOOP (i, 0, _T.get_clms()+1) _OQ[i].set_end(0);   // end is illegally set to 0, for the use in "TRSACT_find_same" 

    // initial occurrence := all transactions
    // ARY_INIT_PERM (_OQ[_T.get_clms()].get_v(), _T.get_t());   
		for(size_t i=0 ; i< _T.get_t(); i++){ 
			_OQ[_T.get_clms()].set_v(i,i);
		}
    _OQ[_T.get_clms()].set_t( _T.get_t());

    free (p);

  }

    // shrinking database

  if ( _flag2&TRSACT_1ST_SHRINK ){

    Q = _OQ[_T.get_clms()];
    _OQ[_T.get_clms()].set_t(0);

    find_same ( &Q, _T.get_clms());

    f = _flag2;  // preserve the flag
    BITRM (_flag2 ,TRSACT_MAKE_NEW +TRSACT_UNION +TRSACT_INTSEC);
    merge_trsact ( &_OQ[_T.get_clms()], _T.get_clms()); // just remove duplicated trsacts
    _flag2 = f;  // recover flag
    _OQ[_T.get_clms()].set_t(0);
    FLOOP (t, 0, _T.get_t()) if ( _mark[t]>0 ) _OQ[_T.get_clms()].push_back(t);  // make resulted occ
  }
  
}


/* sort the transactions and items according to the flag, allocate OQ, and database reduction */
/* causion! not adopt for itemweights!!!!! */
void TRSACT::sortELE ( FILE_COUNT *C, int flag){
  VEC_ID t, *p;
  int f;
  PERM pp;
  KGLCMSEQ_QUE Q;
  QUEUE_ID i;
  WEIGHT *ww;

	_T.set_vv_all();
  //FLOOP (t, 0, _T.get_t()) _T._v[t].set_v( _T._v[t].get_t() , _T.get_clms());

  flag = (_flag&(LOAD_SIZSORT+LOAD_WSORT)? ((_flag&LOAD_DECROWSORT)? -1:1):0) *sizeof(QUEUE);

  if ( flag ){   // sort rows for the case that some columns are not read
	  //FLOOP (t, 0, _rows_org){  // compute #elements according to rowt, and set rperm
	  //	printf("%d\n",C->rperm[t]);
	  //}

   // qsort_perm__VECt ((VEC *)_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions
		//qsort_perm__<VEC> ((VEC *)_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions
		//qsort_perm__VECt(_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions
		//qsort_perm__VECt((VEC *)_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactions

		//qsort_perm__(_T._v, _T.get_t(), C->rperm, flag); // determine the order of transactionsx

		// friendにする？
		_T.qsort_perm(C->get_rperm(), flag);


		_T.any_INVPERMUTE( C->get_rperm());
    //ARY_INVPERMUTE (_T._v, C->rperm, Q, _T.get_t(), EXIT);  // sort transactions
    if ( _T.exist_w() ) {
    	ARY_INVPERMUTE (_T.get_w(), C->get_rperm(), ww, _T.get_t(), EXIT); // sort rows of itemweighs 
    }
    ARY_INVPERMUTE_ (_trperm, C->get_rperm(), pp, _T.get_t()); 
  }

  //free2 (C->rperm); free2 (C->cperm);


  if ( _flag & LOAD_PERM ) flag = 1;
  else flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);

  if ( flag ){
    FLOOP (t, 0, _T.get_t()) qsort_<QUEUE_INT> (_T.get_vv(t), _T.get_vt(t), flag);
  }
  
  if ( _flag & LOAD_RM_DUP ){
    FLOOP (t, 0, _T.get_t()){
    	if(_T.get_vt(t)>1){
				INT cmmn_INT=1;
				for (INT cmmn_INT2=1 ; cmmn_INT2<_T.get_vt(t) ; cmmn_INT2++){

					if ( _T.get_vv(t,cmmn_INT2-1) != _T.get_vv(t,cmmn_INT2) )
  					_T.set_vv( t, cmmn_INT++ , _T.get_vv(t,cmmn_INT2));
				}
				_T.set_vt(t,cmmn_INT);
			}
			_T.set_vv(t, _T.get_vt(t), _T.get_clms()); 
		}
  }

	//ST_MAX(m,i,S,a,x,y)   
  //ST_MAX (_row_max, i, _T._v, _t, 0, _T.get_t());
	// QUEUEのメソッド化
	_row_max=_T.get_vt(0);
	i=0;
	INT cmmn_INT;
	FLOOP(cmmn_INT,1,_T.get_t()){
		if(_row_max <_T.get_vt(cmmn_INT)){
			i=cmmn_INT;
			_row_max=_T.get_vt(i);
		}
	}

  if ( _flag2&(TRSACT_ALLOC_OCC+TRSACT_SHRINK) ){


    calloc2 (p, _T.get_clms(), EXIT);
    
    // QUEUE_delivery (NULL, p, NULL, _T._v, NULL, _T._t, _T._clms);
		//===================
		VEC_ID iv, ev;
	  QUEUE_INT *x;
		for (iv=0 ; iv<_T.get_t() ; iv++){
    	ev =  iv;
    	MLOOP (x, _T.get_vv(ev), _T.get_clms()) p[*x]++;
		}
		//===================
    ARY_MAX (_clm_max, i, p, 0, _T.get_clms());

    Mque_allocELE(p);

   	//QUEUE_alloc (&_OQ[_T._clms], MAX(_T._t, _clm_max));
		_OQELE[_T.get_clms()].alloc( MAX(_T.get_t(), _clm_max));

    FLOOP (i, 0, _T.get_clms()+1) _OQELE[i].set_end(0);   // end is illegally set to 0, for the use in "TRSACT_find_same" 

    // initial occurrence := all transactions
    // ARY_INIT_PERM (_OQ[_T.get_clms()].get_v(), _T.get_t());   
		for(size_t i=0 ; i< _T.get_t(); i++){ 
			_OQELE[_T.get_clms()].set_v(i,i);
		}
    _OQELE[_T.get_clms()].set_t( _T.get_t());

    free (p);

  }

    // shrinking database

  if ( _flag2&TRSACT_1ST_SHRINK ){

    Q = _OQELE[_T.get_clms()];
    _OQELE[_T.get_clms()].set_t(0);

    find_same ( &Q, _T.get_clms());

    f = _flag2;  // preserve the flag
    BITRM (_flag2 ,TRSACT_MAKE_NEW +TRSACT_UNION +TRSACT_INTSEC);
    merge_trsact ( &_OQELE[_T.get_clms()], _T.get_clms()); // just remove duplicated trsacts
    _flag2 = f;  // recover flag
    _OQELE[_T.get_clms()].set_t(0);
    FLOOP (t, 0, _T.get_t()) {
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
/*int TRSACT::load (
	int flag ,int flag2 ,
	char *fname,char *wfname,char *iwfname,char *fname2,// ファイル 
	// サイズパラメータ 
	WEIGHT w_lb,WEIGHT w_ub,double clm_lb_,double clm_ub_ ,
	QUEUE_ID row_lb,QUEUE_ID row_ub ,double row_lb_,double row_ub_
){*/
int TRSACT::loadMain(bool elef){

  FILE2 fp , fp2 ;
//  FILE_COUNT C = INIT_FILE_COUNT;
  VEC_ID t=0;
  int f;
  // パラメータセット
  /*
	_flag = flag;
	_flag2 = flag2;
	_fname = fname;
	_wfname = wfname;
	_item_wfname = iwfname;
	_fname2 = fname2; 
	_w_lb = w_lb;
	_w_ub = w_ub; 
	_clm_lb_ = clm_ub_; 
	_row_lb = row_lb;
	_row_ub = row_ub ;
	_row_lb_ = row_lb_;
	_row_ub_ = row_ub_;
	*/


  fp.open( _fname, "r");
  if(_flag&LOAD_TPOSE){
  	if ( _C.file_count ( &fp, _w_lb , _w_ub ,_clm_lb , _clm_ub ,_wfname) ){ return 1; } 
  }
  else{
  	if ( _C.file_count ( &fp, _row_lb, _row_ub , _wfname) ){ return 1; } 

  }

  _end1 = _C.rows();

  if ( _fname2 ){
  	fp2.open ( _fname2, "r");
  	if(_flag&LOAD_TPOSE){
  		
  		if ( _C.file_count ( &fp2, _w_lb , _w_ub ,_clm_lb , _clm_ub ,NULL) ){ return 1; } // ※
 	 	}
  	else{
  		if ( _C.file_count ( &fp2, _row_lb, _row_ub , NULL) ){ return 1; } // ※
  	}
	}
  f = talloc();
  file_read ( &fp, &_C, &t, f, _item_wfname);
  if (ERROR_MES) goto END;
  
  if ( _fname2 ) file_read ( &fp2, &_C, &t, f, NULL); // ※
  
  //if (ERROR_MES){ mfree (C.rowt, C.clmt); goto END; }
  int i;
  if(elef){
	  sortELE(&_C, f);
	}
	else{
	  sort (&_C, f);
	}

  END:;
  fp.close ();
  fp2.close ();
  if (ERROR_MES) end(); 
  else prop_print();

  return 0;
}


/* iteration of delivery; operate one transaction */
/* use OQ.end to count the number of items */
/* jump will be cleared (t := s) at the beginning */
void TRSACT::delivery_iter ( WEIGHT *w, WEIGHT *pw, VEC_ID t, QUEUE_INT m){

  WEIGHT *y=0;
  QUEUE_INT *x;
  int f = _flag2&TRSACT_NEGATIVE;

  if ( _T.exist_w() ){
  	 y = _T.get_w(t);
  }

	for(x=_T.get_vv(t); *((QUEUE_INT *)x)<(m) ; x++){


    if ( _OQ[*x].get_end() == 0 ){ 
    	_jump.push_back(*x); 
    	w[*x] = 0; 
    	if ( f ) pw[*x] = 0; 
    }
    _OQ[*x].inc_end();

    if ( y ){
      w[*x] += *y; if ( *y>0 && f) pw[*x] += *y;
      y++;
    } else {
      w[*x] += _w[t]; 
      if ( f ) pw[*x] += _pw[t];
    }
  }
}
void TRSACT::delivery (WEIGHT *w, WEIGHT *pw, QUEUE *occ, QUEUE_INT m){

  VEC_ID i, t;
  //int cnt = 0;
  char *b = (char *)(occ?occ->get_v(): NULL);
  _jump.set_t( _jump.get_s());

	
	FLOOP (i, occ?occ->get_s():0, occ?occ->get_t():_T.get_t()){

    t = occ? *((QUEUE_INT *)b): i;
    // delivery_iter ( w, pw, t, m);
    
		_T.delivery_iter( w, pw, t, m ,&_jump,_OQ,_w,_pw,_flag2&TRSACT_NEGATIVE);

    b += _occ_unit;
  }

}

/* usual delivery (make transpose) with checking sc
   don't touch jump */
/* if (T->flag2&TRSACT_DELIV_SC), do not stack to items e with non-zero T->sc[e] */
void TRSACT::deliv ( QUEUE *occ, QUEUE_INT m){

  VEC_ID i, t;
  QUEUE_INT *x;
  char *b = (char *)(occ?occ->get_v(): NULL);

  if ( _flag2&TRSACT_DELIV_SC ){
  	
    FLOOP (i, occ?occ->get_s():0, occ?occ->get_t():_T.get_t()){
      t = occ? *((QUEUE_INT *)b): i;

			for(x=_T.get_vv(t); *((QUEUE_INT *)x)<(m) ; x++){

       	if ( !_sc[*x] ) _OQ[*x].push_back(t);
      }
      b += _occ_unit;

    }
  } else {

    FLOOP (i, occ?occ->get_s():0, occ?occ->get_t():_T.get_t()){
      t = occ? *((QUEUE_INT *)b): i;
			for(x=_T.get_vv(t); *((QUEUE_INT *)x)<(m) ; x++){
				_OQ[*x].push_back(t);
			}
      b += _occ_unit;
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
  QQ[_T.get_clms()].set_s(0);

  while (1){
    if ( o->size()  == 1 ){
    	 _mark[o->pop_back()] = 1;  // no same transactions; mark by 1
    }
    if ( o->get_t() == 0 ) goto END;
    // if previously inserted transactions are in different group, then change their marks with incrementing mark by one
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
		//QQ[_T.get_clms()].dec_t();
  	//_mark[QQ[_T.get_clms()].get_v(QQ[_T.get_clms()].get_t())] = 1;
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

	if ( ERROR_MES ) return;
  if ( wflag ){

  	wbuf = (WEIGHT *)_wbuf.get_memory ( _T.get_vt(t)+1);
  	_T.set_w(tt,wbuf);
  }

	if ( ERROR_MES ){ _buf.set_num(bnum); _buf.set_block_num(bblock); return; }
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
		if (ERROR_MES) return;
    ww = ((WEIGHT *)_wbuf.get_memory (_wbuf.get_block_siz())) +siz;
		if ( ERROR_MES ){ _buf.set_num(bnum); _buf.set_block_num(bblock); return; }
    flag =1;
  }

  if ( ERROR_MES ) return;

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



/*****/
/* merge duplicated transactions in occ according to those having same value in T->mark
   the mark except for the representative will be zero, for each group of the same transactions
   the mark of the representative will be its (new) ID +2 (one is for identical transaction) */
/* T->flag2&TRSACT_MAKE_NEW: make new trsact for representative
   T->flag2&TRSACT_INTSEC: take suffix intersection of the same trsacts
   T->flag2&TRSACT_UNION: take union of the same trsacts */
/* o will be cleard after the execution */
void TRSACT::merge_trsact ( QUEUE *o, QUEUE_INT end){

  VEC_ID mark = 0, tt=0;
  QUEUE_INT *x;

	for(QUEUE_INT * x = o->begin() ; x < o->end() ; x++){

    if ( mark == _mark[*x] ){
      _mark[*x] = 0;   // mark of unified (deleted) transaction
      _w[tt] += _w[*x]; if ( _pw ) _pw[tt] += _pw[*x];
      if ( _flag2 & TRSACT_INTSEC ){
        suffix_and (tt, *x);
        _buf.set_num( (int)(_T.get_vv(tt) - (QUEUE_INT *)_buf.get_base(_buf.get_block_num())  +_T.get_vt(tt) +1) );
      }
      if ( _flag2 & TRSACT_UNION ){
        itemweight_union (tt, *x);
        if ( ERROR_MES ) _mark[*x] = *x+2; // do not merge if not enough memory
      }
    }

		// *x is not the same to the previous, or memory short 
    if ( mark != _mark[*x] && _mark[*x] > 1 ){
    
      mark = _mark[*x];

      if ( _flag2&TRSACT_MAKE_NEW ){

        tt = _new_t++;

        copy ( tt, *x, (_flag2&(TRSACT_INTSEC+TRSACT_UNION))? _T.get_clms(): end);

        if( ERROR_MES ){ _new_t--; tt = *x; }
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

void TRSACT::merge_trsact ( KGLCMSEQ_QUE *o, QUEUE_INT end){

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
        if ( ERROR_MES ) _mark[x] = x+2; // do not merge if not enough memory
      }
    }

		// *x is not the same to the previous, or memory short 
    if ( mark != _mark[x] && _mark[x] > 1 ){
    
      mark = _mark[x];

      if ( _flag2&TRSACT_MAKE_NEW ){

        tt = _new_t++;

        copy ( tt, x, (_flag2&(TRSACT_INTSEC+TRSACT_UNION))? _T.get_clms(): end);

        if( ERROR_MES ){ _new_t--; tt = x; }
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

///* extension
/*
void TRSACT::reduce_occ_NotQUEUE_INT(QUEUE *occ){

  QUEUE_INT *x, *y=occ->get_v();

	for( 
		x=occ->begin() ;
		(char *)(x) < ((char *)occ->get_v()) + ((occ)->get_t()*_occ_unit);
	  x= (QUEUE_INT *)( (char *)x + _occ_unit )
	){
     if ( _mark[*x] == 0 ) continue;
     memcpy (y, x, _occ_unit);
     *y = _mark[*x]>1? _mark[*x]-2: *x;
     y = (QUEUE_INT *)(((char *)y)+_occ_unit);
     i++;
	}

}
//*/
