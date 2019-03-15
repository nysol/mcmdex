/* library for vector and sparse vector, and matrix */
/* Takeaki Uno    27/Dec/2008 */

//#ifndef _vec_c_
//#define _vec_c_

#include"vec.hpp"
#include"stdlib2.hpp"
#include"queue.hpp"
#include"filecount.hpp"

/* allocate memory according to rows and rowt */
/* if eles == 0, compute eles from rowt and rows */
void SETFAMILY::alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){

  VEC_ID i;
  char *buf;
  if ( eles == 0 ) ARY_SUM (_ele_end, rowt, 0, rows); else _ele_end = eles;
  calloc2 (buf, (_ele_end*((_flag&LOAD_DBLBUF)?2:1) +(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)*_unit, EXIT);
  _buf = (QUEUE_INT *)buf;
  
  //ARY_FILL (_v, 0, rows, QUEUE());  
  malloc2 (_v, rows+1, {free(_buf);EXIT;});
	for(size_t i =0 ;i<rows;i++){ _v[i] = QUEUE(); }

  _end = rows;
  _clms = clms;
  if ( rowt ){
    FLOOP (i, 0, rows){
      _v[i].set_v((QUEUE_INT *)buf);
      buf += (rowt[i] +1)*_unit;
      _v[i].set_end(rowt[i]+1);
    }
  }
}

/* allocate memory according to rows and rowt */
/* if eles == 0, compute eles from rowt and rows */
void SETFAMILY::alloc_weight ( QUEUE_ID *t){
  VEC_ID i;
  calloc2 (_w, _end +1, EXIT);
  calloc2 (_wbuf, _ele_end*((_flag&LOAD_DBLBUF)?2:1)+1, {free(_w);EXIT;});
  _w[0] = _wbuf; FLOOP (i, 1, _t) _w[i] = _w[i-1] + (t?t[i-1]:_v[i-1].get_t());
}

void SETFAMILY::alloc_w (){
  calloc2 (_w, _end, EXIT);
}

/* terminate routine for MAT */
void SETFAMILY::end (){
  mfree (_buf, _buf2, _v, _rw, _cw, _wbuf, _w, _rperm, _cperm);
  // *M = INIT_SETFAMILY;
	//_type=TYPE_SETFAMILY;
	_fname=NULL;
	_wfname=NULL;
	_flag=0;
	_v=NULL;
	_end=0;
	_t=0;
	_buf=NULL;
	_buf2=NULL;
	_clms=0;
	_eles=0;
	_ele_end=0;
	_cw=NULL;
	_rw=NULL;
	_w=NULL;
	_wbuf=NULL;
	_unit=sizeof(QUEUE_INT);
	_cwfname=NULL;
	_rwfname=NULL;
	_rperm=NULL;
	_cperm=NULL;

}


/* sort and duplication check */
void SETFAMILY::sort (){
  VEC_ID i;
  PERM *p;
  WEIGHT *ww, w;
  QUEUE Q;
  int flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);
  if ( flag ){   // sort items in each row
    malloc2 (p, _clms, EXIT);
    FLOOP (i, 0, _t)
      _v[i].perm_WEIGHT( _w?_w[i]:NULL, p, flag);
    free (p);
  }
  flag = ((_flag&(LOAD_SIZSORT+LOAD_WSORT))? ((_flag&LOAD_DECROWSORT)? -1: 1): 0);
  if ( flag ){   // sort the rows
    if ( _flag & LOAD_SIZSORT ) p = qsort_perm_<VEC> ((VEC *)_v, _t, flag*sizeof(QUEUE));
    else p = qsort_perm_<WEIGHT> (_rw, _t, flag*sizeof(WEIGHT));

		//ARY_INV_PERM(f,p,end,EXIT) 
    //ARY_INV_PERM (_rperm, p, _t, EXIT);
		malloc2(_rperm,_t,EXIT);
		for(size_t st=0; st<_t ;st++){ _rperm[st]=-1; }
		for(int i=0;i<_t;i++){
			if(p[i]>=0 && p[i]<_t){ _rperm[p[i]]=i; }
		}

    if ( _rw ) ARY_INVPERMUTE (_rw, p, w, _t, EXIT);
    if ( _w ) ARY_INVPERMUTE (_w, p, ww, _t, EXIT);
    ARY_INVPERMUTE_ (_v, p, Q, _t);
    free2 (p);
  }
  if (_flag&LOAD_RM_DUP){  // unify the duplicated edges
    FLOOP (i, 0, _t)
			_v[i].rm_dup_WEIGHT( _w?_w[i]:NULL);
  }
}
void SETFAMILY::SMAT_alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){
  VEC_ID i;
  if ( eles == 0 ) ARY_SUM (_ele_end, rowt, 0, rows); else _ele_end = eles;
  calloc2 (_buf, _ele_end*((_flag&LOAD_DBLBUF)?2:1) +rows +2, EXIT);
  malloc2 (_v, rows+1, {free(_buf);EXIT;});
  //ARY_FILL (_v, 0, rows, SVEC() );

	for(size_t _common_size_t =0;common_size_t<rows;common_size_t++){
		_v[common_size_t].set_end( 0);
	  _v[common_size_t].set_t(0);
	}


  _end = rows;
  _clms = clms;
  if ( rowt ){
    FLOOP (i, 0, rows){
      _v[i].set_v( i? _v[i-1].get_v() + rowt[i-1] +1: _buf);
      _v[i].set_end( rowt[i]);
    }
  }
}

void SETFAMILY::SMAT_flie_load(FILE2 *fp){

  WEIGHT z=0;
  VEC_ID t;
  LONG x, y;
  int fc=0, FILE_err_=0, flag2=_flag;
  int flag=0;
 // int wflag = (_type==TYPE_SETFAMILY && ( _wfname || (_flag&LOAD_EDGEW)));
  int wflag =  ( _wfname || (_flag&LOAD_EDGEW));
  FILE_COUNT C;
  FILE2 wfp;

  if ( flag && !_wfname ) flag2 = _flag | LOAD_EDGEW;
//  C = FILE2_count (fp, (M->flag&(LOAD_ELE+LOAD_TPOSE+LOAD_EDGE+LOAD_RC_SAME)) | FILE_COUNT_ROWT, 0, 0, 0, (flag||flag2)? 1: 0, 0);
  //C = fp->count((_flag&(LOAD_ELE+LOAD_TPOSE+LOAD_RC_SAME+LOAD_EDGE)) | FILE_COUNT_ROWT, 0, 0, 0, (flag2&LOAD_EDGEW)?1:0, 0);

  C.count(fp,(_flag&(LOAD_ELE+LOAD_TPOSE+LOAD_RC_SAME+LOAD_EDGE)) | FILE_COUNT_ROWT, 0, 0, 0, (flag2&LOAD_EDGEW)?1:0, 0);

  if ( _clms == 0 ) _clms = C.get_clms();
  if ( _t == 0 ) _t = C.get_rows();
  if ( flag ) SMAT_alloc ( _t, C.getp_rowt(), _clms, 0);
  else {
    alloc ( _t, C.getp_rowt(), _clms, 0);
    if ( wflag ) alloc_weight ( C.getp_rowt());
  }
  //free2 (C.rowt);

  if ( _wfname ) wfp.open ( _wfname, "r");
  if ( ERROR_MES ) return;

  fp->reset ();
  if ( _flag&(LOAD_NUM+LOAD_GRAPHNUM) ) fp->read_until_newline ();
  t=0;
  do {
    if ( _flag&LOAD_ELE ){
      if ( fp->read_pair ( &x, &y, &z, flag2) ) continue;
    } else {
      x = t;
      FILE_err_ = fp->read_item ( _wfname?&wfp:NULL, &x, &y, &z, fc, flag2);
      if ( FILE_err&4 ) goto LOOP_END;
    }

    if ( y >= _clms || x >= _t ) continue;

    if ( flag ){
      //_v[x]._v[_v[x]._t]._i = y;
      //_v[x]._v[_v[x]._t]._a = z;
      _v[x].inc_t();
    } else {
      if ( wflag ) _w[x][_v[x].get_t()] = z;
      _v[x].push_back(y);
      if ( (_flag&LOAD_EDGE) && x != y ){
        if ( wflag ) _w[y][_v[y].get_t()] = z;
        _v[y].push_back(x);
      }
    }
    if ( !(_flag&LOAD_ELE) ){
      fc = 0;
      if ( FILE_err&3 ){
        LOOP_END:;
        t++; if ( t >= _t ) break;
        fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
      }
    }
  } while ( (FILE_err&2)==0 );
  if ( _wfname ) wfp.close ();

}
/* scan file and load the data from file to SMAT structure */
void SETFAMILY::load (){
///* smat と連動
  FILE2 fp;
  VEC_ID i;

  //_type = TYPE_SETFAMILY;
  fp.open ( _fname, "r");

	SMAT_flie_load(&fp);


  fp.close ();     if(ERROR_MES) EXIT;
  print_mes (this, "setfamily: %s ,#rows %d ,#clms %d ,#eles %zd", _fname, _t, _clms, _eles);
  if (_wfname ) print_mes (this, " ,weightfile %s", _wfname);

  if ( _cwfname ){
    load_column_weight ();
    if ( ERROR_MES ){ end (); EXIT; }
    print_mes (this, " ,column-weightfile %s", _cwfname);
  }
  if ( _rwfname ){
    load_row_weight ();            if ( ERROR_MES ){ end (); EXIT; }
    print_mes (this, " ,row-weightfile %s", _rwfname);
  }
  print_mes (this, "\n");
 
  sort ();
  FLOOP (i, 0, _t) _v[i].set_v(_v[i].get_t(),_clms);  // end mark
  _eles = _ele_end;
}

/* load column/row weight from the file */
void SETFAMILY::load_column_weight (){

  int i = FILE2::ARY_Load(_cw, _cwfname, 1);
  //ARY_LOAD (_cw, WEIGHT, i, _cwfname, 1, EXIT);
  reallocx (_cw, i, _clms+1, 0, EXIT);
}

void SETFAMILY::load_row_weight (){
  //i = ARY_LOAD (_rw, WEIGHT,i, _rwfname, 1;
  int i = FILE2::ARY_Load(_rw,_rwfname, 1);
  reallocx (_rw, i, _t+1, 0, EXIT);
}




//SVEC INIT_SVEC_ELE = {0,0};
//SMAT INIT_SMAT = {TYPE_SMAT,NULL,NULL,0,NULL,0,0,NULL,NULL,0,0,0};
//SETFAMILY INIT_SETFAMILY = INIT_SETFAMILY_;
/*
QSORT_TYPE (SVEC_VAL, SVEC_VAL)
QSORT_TYPE (SVEC_VAL2, SVEC_VAL2)
*/

/* allocate memory according to rows and rowt */
/*
void VEC_alloc (VEC *V, VEC_ID clms){
  *V = INIT_VEC;
  V->end = clms;
  calloc2 (V->v, clms+1, EXIT);
}
*/
/* terminate routine for VEC */
/*void VEC_end (VEC *V){
  free2 (V->v);
  *V = INIT_VEC;
}*/

/* allocate memory according to rows and rowt */
/*
void MAT::alloc (VEC_ID rows, VEC_ID clms){
  VEC_ID i, clms2 = clms+(clms%2?1:2);
  calloc2 (_v, rows+1, EXIT);
  calloc2 (_buf_org, clms2 * (rows+1)+4, {free(_v);EXIT;});

  _buf = _buf_org; 
  // ADDR_FLOOR16(_buf);
	char * chtmp;
	chtmp =((char *)_buf)+15;
	_buf = (typeof(_buf))(chtmp-(((size_t)chtmp)&15));

  _end = rows;
  _clms = clms;
  FLOOP (i, 0, rows){
    _v[i]._end = _v[i]._t = clms;
    _v[i]._v   = _buf + i*(clms2);
//    printf ("%p %p\n", M->buf, M->v[i].v);
  }
}
*/
/* terminate routine for MAT */
/*
void MAT::end (){
  free2 (_buf_org);
  free2 (_buf2_org);// double free?
  free2 (_v);

	// MAT INIT_MAT = {TYPE_MAT,NULL,NULL,0,NULL,0,0,NULL,NULL,0,0,NULL,NULL};
  // *M = INIT_MAT;

  // mark to identify type of the structure
   //_type = TYPE_MAT;
	_fname = NULL;
	_wfname= NULL;      // input/weight file name
	_flag=0;         // flag

	_v=NULL;
	_end=0; 
	_t=0;
  _buf=NULL;
  _buf2=NULL;
	_clms=0;
  size_t _eles=0;
	_buf_org=NULL;
	_buf2_org=NULL;
	
}*/

/* allocate memory */
/*
void SVEC::alloc (VEC_ID end){

	// SVEC INIT_SVEC = {TYPE_SVEC,NULL,0,0};
  // *V = INIT_SVEC;
  _type = TYPE_SVEC;
  _end = 0;
  _t = 0;
  calloc2 (_v, end+1, EXIT);
  _end = end;
  _t = 0;
}
*/
/* terminate routine for SVEC */
/*
void SVEC::end (){
  free2 (_v);

  // *V = INIT_SVEC;
  _type = TYPE_SVEC;
  _end = 0;
  _t = 0;

}
*/
/* allocate memory according to rows and rowt */
/*
void SMAT::alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){
  VEC_ID i;
  if ( eles == 0 ) ARY_SUM (_ele_end, rowt, 0, rows); else _ele_end = eles;
  calloc2 (_buf, _ele_end*((_flag&LOAD_DBLBUF)?2:1) +rows +2, EXIT);
  malloc2 (_v, rows+1, {free(_buf);EXIT;});
  //ARY_FILL (_v, 0, rows, SVEC() );
	for(size_t _common_size_t =0;common_size_t<rows;common_size_t++){
		_v[common_size_t]._type=TYPE_SMAT;
		_v[common_size_t]._end = 0;
	  _v[common_size_t]._t = 0;
	}

  _end = rows;
  _clms = clms;
  if ( rowt ){
    FLOOP (i, 0, rows){
      _v[i]._v = i? _v[i-1]._v + rowt[i-1] +1: _buf;
      _v[i]._end = rowt[i];
    }
  }
}
*/
/* terminate routine for MAT */
/*
void SMAT::end (){
  free2 (_buf);
  free2 (_buf2);
  free2 (_v);
	//SMAT INIT_SMAT = {TYPE_SMAT,NULL,NULL,0,NULL,0,0,NULL,NULL,0,0,0};
  // *M = INIT_SMAT;

  _type = TYPE_SMAT;
  _fname = NULL;
  _wfname = NULL;
  _flag = 0;
  _v = NULL;
  _end = 0;
  _t = 0;
  _buf = NULL;
  _buf2 = NULL;
  _clms = 0;
  _eles = 0;
  _ele_end = 0;
  
}

*/


/****************************************************************/
/****************************************************************/
/****************************************************************/

/* read binary file for MAT */
/* each unit-byte will be one number. if unit<0, the sign of unit is flipped, and each value is minesed the half of the maximum */
/*
void MAT::load_bin (FILE2 *fp, int unit){
  VEC_ID flag=0, i, j, jj;
  size_t siz=0;
  VEC_VAL z, neg=0;

  if ( unit < 0 ){
    unit = -unit; flag = 1; neg=128;
    FLOOP (jj, 0, unit-1) neg *= 256;
  }
  if ( _t == 0 ){  // determine #rows if M->t is 0 (not specified)
		siz = fp->get_fsize();
    _t = (VEC_ID)(siz / unit / _clms);
    if ( _flag & LOAD_TPOSE ) SWAP_<VEC_ID>(&_t, &_clms);
  }
  alloc(_t, _clms);  if (ERROR_MES) return;
  _end = _t;
  FLOOP (i, 0, _t){
    FLOOP (j, 0, _clms){
      z=0; FLOOP (jj, 0, unit){ z *= 256; z += fp->getc(); }
      if ( flag ) z -= neg;
      if ( _flag & LOAD_TPOSE ) _v[j]._v[i] = z;
      else _v[i]._v[j] = z;
    }
  }
}
*/

/* segmentation fault for illegal files */
/* count/read the number in file for MAT */
/* if *rows>0, only read count the numbers in a row, for the first scan. */
/*
void MAT::file_load (FILE2 *fp){
  QUEUE_ID c;
  VEC_ID t=0;
  double p;

  for (t=0 ; (FILE_err&2)==0 ; t++){

    //ARY_SCAN (c, double, *fp, 0);
    c = fp->ARY_Scan_DBL(0);

    if ( _flag & LOAD_TPOSE ){
      if ( _t == 0 ){ _t = c; if (_clms>0 ) break; }
    } else if ( _clms == 0 ){ _clms = c; if ( _t>0 ) break; }
    if ( c == 0 ) t--;
  }
  if ( _flag & LOAD_TPOSE ){ if ( _clms==0 ) _clms = t;} else if ( _t==0 ) _t = t;
  fp->reset ();
  _end = _t;
  alloc (_t, _clms); if (ERROR_MES) return;
  FLOOP (t, 0, (_flag&LOAD_TPOSE)? _clms: _t){
    FLOOP (c, 0, (_flag&LOAD_TPOSE)? _t: _clms){
      p = fp->read_double();
      if ( FILE_err==1 || FILE_err==2 ) break;
      if ( _flag&LOAD_TPOSE ) _v[c]._v[t] = p;
      else _v[t]._v[c] = p;
      if ( FILE_err==5 || FILE_err==6 ) break;
    }
    FLOOP (c, c, (_flag&LOAD_TPOSE)? _t: _clms){
      if ( _flag&LOAD_TPOSE ) _v[c]._v[t] = 0;
      else _v[t]._v[c] = 0;
    }
    if ( !FILE_err ) fp->read_until_newline ();
  }
}
*/
/* load file with switching the format according to the flag */
/*
void MAT::load (){
  FILE2 fp ;
  int unit=0;
#ifdef USE_MATH
  VEC_ID i;
#endif
  if ( _flag & VEC_LOAD_BIN ) unit = 1;
  else if ( _flag & VEC_LOAD_BIN2 ) unit = 2;
  else if ( _flag & VEC_LOAD_BIN4 ) unit = 4;
  if ( _flag & VEC_LOAD_CENTERIZE ) unit = -unit;

  fp.open( _fname, "rb");
  if ( unit ) load_bin (&fp, unit);
  else file_load (&fp);

  fp.close (); if (ERROR_MES) EXIT;
#ifdef USE_MATH
  if ( _flag&VEC_NORMALIZE ) FLOOP (i, 0, _t) ARY_NORMALIZE (_v[i].v,_v[i].t);
#endif
  print_mes (this, "mat: %s ,#rows %d ,#clms %d\n", _fname, _t, _clms);
}
*/

/* scan file and read the numbers for SMAT */
/* flag&1? SMAT, SETFAMILY,  flag&2? tuple list format: array list :*/ 
/*

void SMAT::file_load (FILE2 *fp){
	printf("sload\n");
  double z=0;
  VEC_ID t;
  LONG x, y;
  int fc=0, FILE_err_=0, flag=(M->type==TYPE_SMAT), flag2=M->flag;
  int wflag = (M->type==TYPE_SETFAMILY && (((SETFAMILY *)M)->wfname || (M->flag&LOAD_EDGEW)));
  FILE_COUNT C;
  FILE2 wfp;

  if ( flag && !M->wfname ) flag2 = M->flag | LOAD_EDGEW;
//  C = FILE2_count (fp, (M->flag&(LOAD_ELE+LOAD_TPOSE+LOAD_EDGE+LOAD_RC_SAME)) | FILE_COUNT_ROWT, 0, 0, 0, (flag||flag2)? 1: 0, 0);
  C = FILE2_count (fp, (M->flag&(LOAD_ELE+LOAD_TPOSE+LOAD_RC_SAME+LOAD_EDGE)) | FILE_COUNT_ROWT, 0, 0, 0, (flag2&LOAD_EDGEW)?1:0, 0);
  if ( M->clms == 0 ) M->clms = C.clms;
  if ( M->t == 0 ) M->t = C.rows;
  if ( flag ) SMAT_alloc (M, M->t, C.rowt, M->clms, 0);
  else {
    SETFAMILY_alloc ((SETFAMILY *)M, M->t, C.rowt, M->clms, 0);
    if ( wflag ) SETFAMILY_alloc_weight ((SETFAMILY *)M, C.rowt);
  }
  free2 (C.rowt);
  if ( ((SETFAMILY *)M)->wfname ) FILE2_open (wfp, ((SETFAMILY *)M)->wfname, "r", EXIT);
  if ( ERROR_MES ) return;

  FILE2_reset (fp);
  if ( M->flag&(LOAD_NUM+LOAD_GRAPHNUM) ) FILE2_read_until_newline (fp);
  t=0;
  do {
    if ( M->flag&LOAD_ELE ){
      if ( FILE2_read_pair (fp, &x, &y, &z, flag2) ) continue;
    } else {
      x = t;
      FILE_err_ = FILE2_read_item (fp, ((SETFAMILY *)M)->wfname?&wfp:NULL, &x, &y, &z, fc, flag2);
      if ( FILE_err&4 ) goto LOOP_END;
    }
//  printf ("%d %d       %d %d\n", x, M->t, y, M->clms);
    if ( y >= M->clms || x >= M->t ) continue;
//  printf ("## %d %d\n", x, y);
    if ( flag ){
      M->v[x].v[M->v[x].t].i = y;
      M->v[x].v[M->v[x].t].a = z;
      M->v[x].t++;
    } else {
      if ( wflag ) ((SETFAMILY *)M)->w[x][((SETFAMILY *)M)->v[x].t] = z;
      QUE_INS (((SETFAMILY *)M)->v[x], y);
      if ( (M->flag&LOAD_EDGE) && x != y ){
        if ( wflag ) ((SETFAMILY *)M)->w[y][((SETFAMILY *)M)->v[y].t] = z;
        QUE_INS (((SETFAMILY *)M)->v[y], x);
      }
    }
    if ( !(M->flag&LOAD_ELE) ){
      fc = 0;
      if ( FILE_err&3 ){
        LOOP_END:;
        t++; if ( t >= M->t ) break;
        fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
      }
    }
  } while ( (FILE_err&2)==0 );
  if ( ((SETFAMILY *)M)->wfname ) FILE2_close (&wfp);
}
*/


/* load file with switching the format according to the flag */
/*
void SMAT::load (){
  FILE2 fp ;
  VEC_ID i;
  _type = TYPE_SMAT;
  fp.open (_fname, "r");
  file_load ( &fp);
  fp.close();    if (ERROR_MES) EXIT;
  FLOOP (i, 0, _t) _v[i]._v[_v[i]._t].i = _clms;  // end mark

#ifdef USE_MATH
	//あと
  //if ( _flag&VEC_NORMALIZE ) FLOOP (i, 0, _t) SVEC::normalize (&M->v[i]); // normalize
  //
#endif
  if (_flag&LOAD_INCSORT)
      FLOOP (i, 0, _t) qsort_<VEC_ID> ((VEC_ID *)(_v[i]._v), _v[i]._t, sizeof(SVEC_ELE));
  if (_flag&LOAD_DECSORT)
      FLOOP (i, 0, _t) qsort_<VEC_ID> ((VEC_ID *)(_v[i]._v), _v[i]._t, -(int)sizeof(SVEC_ELE));
  if (_flag&LOAD_RM_DUP)
      FLOOP (i, 0, _t) MQUE_UNIFY (_v[i], SVEC_VAL);
  _eles = _ele_end;
  print_mes (this, "smat: %s ,#rows %d ,#clms %d ,#eles %zd\n", _fname, _t, _clms, _eles);

}
*/


/* print routines */
/*
void MAT::print (FILE *fp){
  VEC *V;
  MQUE_FLOOP (*this, V) ARY_FPRINT (fp, V->_v, 0, V->_t, VEC_VALF" ");
}
void SVEC::print (FILE *fp){
  SVEC_ELE *x;
  MQUE_FLOOP (*this, x) fprintf (fp, "("QUEUE_IDF","SVEC_VALF") ", (*x).i, (*x).a);
  fputc ('\n', fp);
}
void SMAT_print (FILE *fp){
  SVEC *V;
  // MQUE_FLOOP (*this, V) SVEC_print (fp, V);
}
void SETFAMILY::print (FILE *fp){
  QUEUE_ID i, j;
  FLOOP (j, 0, _t){
    if ( _rw ) fprintf (fp, "[" WEIGHTF "] ", _rw[j]);
    FLOOP (i, 0, _v[j].get_t()){
      if ( i>0 ) fprintf (fp, " ");
      fprintf (fp, QUEUE_INTF, _v[j].get_v(i));
      if ( _w ) fprintf (fp, "(" WEIGHTF ")", _w[j][i]);
    }
    fprintf (fp, "\n");
  }
}
*/

/*
void SETFAMILY_print_WEIGHT (FILE *fp, SETFAMILY *M){
  if ( M->w ){
     printf (","); fprint_WEIGHT (stdout, M->w[i][j]); }
  printf ("\n");
}
*/

/****************************************************************/
/** Inner product routines **************************************/
/****************************************************************/
/*
SVEC_VAL2 SVEC_inpro (SVEC *V1, SVEC *V2){
  VEC_ID i1, i2=0;
  SVEC_VAL2 sum=0;
  FLOOP (i1, 0, V1->_t){
    while (V2->_v[i2].i < V1->_v[i1].i) i2++;
    if (V2->_v[i2].i == V1->_v[i1].i) sum += ((SVEC_VAL2)V2->_v[i2].a)*V1->_v[i1].a;
  }
  return (sum);
}
*/

/* get ith vector */ //class継承させる
//TYPE_MAT) return (&MM->v[i]);
//TYPE_SMAT) return (&((SVEC *)M)->v[i]);
//TYPE_SETFAMILY) return (&((QUEUE *)M)->v[i]);

/*
void *MVEC::getvec ( int i, int flag){
  MAT *MM = (MAT *)M;
  if (MM->type==TYPE_MAT) return (&MM->v[i]);
  if (MM->type==TYPE_SMAT) return (&((SVEC *)M)->v[i]);
  if (MM->type==TYPE_SETFAMILY) return (&((QUEUE *)M)->v[i]);
  return (NULL);
}
*/

/* compute the inner product of two vectors */
/*
double VEC_inpro (VEC *V1, VEC *V2){
  VEC_VAL sum=0;
  VEC_VAL *v1 = V1->v, *v2 = V2->v, *v_end = v1 + MIN (V1->end, V2->end), *vv=v_end-1;
#ifdef USE_SIMD
  __m128d u1, u2, u3;
  double r[2];
  if ( v1 < vv ){
    u3 = _mm_load_pd (v1); v1 += 2;
    u2 = _mm_load_pd (v2); v2 += 2;
    u3 = _mm_mul_pd (u3, u2);
    while ( v1 < vv ){
      u1 = _mm_load_pd (v1); v1 += 2;
      u2 = _mm_load_pd (v2); v2 += 2;
      u1 = _mm_mul_pd (u1, u2);
      u3 = _mm_add_pd (u3, u1);
    }
    _mm_storeu_pd (r, u3);
    sum = r[0]+r[1];
    _mm_empty();
  }
#else
  VEC_VAL a0, a1;
  while ( v1 < vv ){
    a0 = *v1 * *v2; v1++; v2++;
    a1 = *v1 * *v2; v1++; v2++;
    sum += a0 + a1;
  }
#endif
  if ( v1 < v_end ){ sum += *v1 * *v2; }
  return (sum);
}
*/
/* compute the l1-norm of two vectors */
/*
double VEC_l1dist (VEC *V1, VEC *V2){
  VEC_ID i, end=MIN(V1->end,V2->end);
  double sum=0;
  FLOOP (i, 0, end) sum += abs (((double)V1->v[i])- ((double)V2->v[i]));
  return (sum);
}
*/
/* compute the l-infinity-norm of two vectors */
/*
double VEC_linfdist (VEC *V1, VEC *V2){
  VEC_ID i, end=MIN(V1->end,V2->end);
  double m=0;
  FLOOP (i, 0, end) ENMAX (m, abs (((double)V1->v[i])- ((double)V2->v[i])));
  return (m);
}
*/

/*
double SETFAMILY_resemblance (QUEUE *Q1, QUEUE *Q2){
  int *x, *y=Q2->v, *yy = y+Q2->t, s=0;
  MQUE_FLOOP (*Q1, x){
    while ( *y < *x ){ if ( ++y == yy ) goto END; }
    if ( *y == *x ){ s++; if ( ++y == yy ) goto END; }
  }
  END:;
  return ( ((double)s) / ((double)(Q1->t + Q2->t)));
}

*/
//#ifdef USE_MATH

/****************************************************************/
/** Norm computation and normalization   ************************/
/****************************************************************/
/*
double SVEC_norm (SVEC *V){
  SVEC_ELE *v;
  double sum=0;
  MQUE_FLOOP (*V, v) sum += ((double)(v->a)) * (v->a);
  return (sqrt(sum));
}
void SVEC_normalize (SVEC *V){
  SVEC_ELE *v;
  double norm = SVEC_norm (V);
  MQUE_FLOOP (*V, v) v->a /= norm;
}

double VEC_norm (VEC *V){
  return (sqrt (VEC_inpro (V, V)));
}

void VEC_normalize (VEC *V){
  double norm = VEC_norm (V);
  VEC_VAL *v = V->v, *v_end = v + V->end;
#ifdef USE_SIMD
  __m128d u1, u2;
  while ( v < v_end ){
    u1 = _mm_load_pd (v);
    u2 = _mm_load1_pd (&norm);
    u1 = _mm_div_pd (u1, u2);
    _mm_storeu_pd (v, u1);
  }
  _mm_empty();
  if ( v < v_end ) *v /= norm;
#else
  while ( v < v_end ) *v /= norm;
#endif
}
*/
/****************************************************************/
/** Euclidean distance routines *********************************/
/****************************************************************/

/* compute the Euclidean distance of two vectors (VEC) */
/*
double VEC_eucdist_ (VEC *V1, VEC *V2){
  double sum=0, a0;
  VEC_VAL *v1 = V1->v, *v2 = V2->v, *v_end = v1 + MIN (V1->end, V2->end), *vv=v_end-1;
#ifdef USE_SIMD
  __m128d u1, u2, u3;
  double r[2];
  if ( v1 < vv ){
    u3 = _mm_load_pd (v1); v1 += 2;
    u2 = _mm_load_pd (v2); v2 += 2;
    u3 = _mm_sub_pd (u3, u2);
    u3 = _mm_mul_pd (u3, u3);
    while ( v1 < vv ){
      u1 = _mm_load_pd (v1); v1 += 2;
      u2 = _mm_load_pd (v2); v2 += 2;
      u1 = _mm_sub_pd (u1, u2);
      u1 = _mm_mul_pd (u1, u1);
      u3 = _mm_add_pd (u3, u1);
    }
    _mm_storeu_pd (r, u3);
    sum = r[0]+r[1];
    _mm_empty();
  }
#else
  double a1;
  while ( v1 < vv ){
    a0 = *v1 - *v2; v1++; v2++;
    a1 = *v1 - *v2; v1++; v2++;
    sum += a0*a0 + a1*a1;
  }
#endif
  if ( v1 < v_end ){ a0 = *v1 - *v2; sum += a0*a0; }
  return (sum);
}

double VEC_eucdist (VEC *V1, VEC *V2){
  double p = SQRT (VEC_eucdist_ (V1, V2));
#ifdef USE_SIMD
  _mm_empty ();
#endif
  return (p);
}
*/
/* compute the Euclidean distance of two vectors (SVEC)*/
/*
double SVEC_eucdist_ (SVEC *V1, SVEC *V2){
  VEC_ID i1, i2;
  double sum=0, a;
  for ( i1=i2=0 ; i1<V1->t && i2<V2->t ; ){
    if (V2->v[i2].i > V1->v[i1].i) a = V1->v[i1].a;
    else if (V2->v[i2].i < V1->v[i1].i) a = V2->v[i2].a;
    else a = ((double)V2->v[i2].a) - ((double)V1->v[i1].a);
    sum += a*a;
  }
  return (sum);
}

double SVEC_eucdist (SVEC *V1, SVEC *V2){
  return (sqrt (SVEC_eucdist (V1, V2)));
}
*/
/* compute the Euclidean distance of two vectors (VEC * SVEC)*/
/*
double VEC_SVEC_eucdist (VEC *V1, SVEC *V2){
  VEC_ID i, i2=0;
  double sum=0, a;
  FLOOP (i, 0, V1->end){
    if ( i < V2->v[i2].i ) a = V1->v[i];
    else { a = ((double)V1->v[i]) - ((double)V2->v[i2].a); i2++; }
    sum += a*a;
  }
  return (sqrt(sum));
}
*/
/**********************************************************/
/* Euclidean distance of vector and set */
/*
double VEC_QUEUE_eucdist (VEC *V, QUEUE *Q){
  VEC_ID i;
  QUEUE_ID i2=0;
  double sum=0, a;
  FLOOP (i, 0, V->end){
    if ( i < Q->v[i2] ) a = V->v[i];
    else { a = ((double)V->v[i]) - 1.0; i2++; }
    sum += a*a;
  }
  return (sqrt(sum));
}
*/
/* compute Euclidean distance of two sets */
/*
double QUEUE_eucdist (QUEUE *Q1, QUEUE *Q2){
  double f;
  MQUE_UNION(f, *Q1, *Q2);
  return (sqrt(f));
}

double MVEC_norm (void *V){
  VEC *VV = (VEC *)V;
  double p;
  if (VV->type==TYPE_VEC){ ARY_NORM (p, VV->v, VV->t); return (p); }
  if (VV->type==TYPE_SVEC) return (SVEC_norm ((SVEC *)V));
  if (VV->type==TYPE_QUEUE) return (sqrt(((QUEUE*)V)->t));
  return (0.0);
}

double MMAT_norm_i (void *M, int i){
  MAT *MM = (MAT *)M;
  double p;
  if (MM->type==TYPE_MAT){ ARY_NORM (p, MM->v[i].v, MM->v[i].t); return (p); }
  if (MM->type==TYPE_SMAT) return (SVEC_norm (&((SMAT *)M)->v[i]));
  if (MM->type==TYPE_SETFAMILY) return (sqrt (((SETFAMILY *)M)->v[i].t));
  return (0.0);
}

double MVEC_eucdist (void *V, void *U){
  VEC *VV = (VEC *)V;
  double p;
  if (VV->type==TYPE_VEC) return (VEC_eucdist ((VEC *)V, (VEC *)U));
  if (VV->type==TYPE_SVEC) return (SVEC_eucdist ((SVEC *)V, (SVEC *)U));
  if (VV->type==TYPE_QUEUE){ MQUE_DIF (p, *((QUEUE *)V), *((QUEUE *)U)); return (sqrt(p));}
  return (0.0);
}

double MMAT_eucdist_ij (void *M, int i, int j){
  MAT *MM=(MAT *)M;
  double p;
  if (MM->type==TYPE_MAT) return (VEC_eucdist ( &MM->v[i], &MM->v[j] ));
  if (MM->type==TYPE_SMAT) return (SVEC_eucdist ( &((SMAT *)M)->v[i], &((SMAT *)M)->v[j]));
  if (MM->type==TYPE_SETFAMILY){ MQUE_DIF (p, ((SETFAMILY *)M)->v[i], ((SETFAMILY *)M)->v[j]); return (sqrt(p)); }
  return (0.0);
}
*/

//#endif

/**********************************************************/
/**   multi-vector routines  ******************************/
/**********************************************************/

/* compute the inner product, Euclidean distance for multi vector */
/*
double MVEC_inpro (void *V, void *U){
  VEC *VV = (VEC *)V, *UU = (VEC *)U;
  double p;
  if (VV->type==TYPE_VEC){
    if (UU->type==TYPE_VEC){ ARY_INPRO (p, VV->v, UU->v, VV->t); return (p); }
    if (UU->type==TYPE_SVEC){ ARY_SVEC_INPRO (p, *((SVEC *)U), VV->v); return (p); }
    if (UU->type==TYPE_QUEUE){ ARY_QUEUE_INPRO (p, *((QUEUE *)U), VV->v); return (p); }
  }
  if (VV->type==TYPE_SVEC){
    if (UU->type==TYPE_VEC){ ARY_SVEC_INPRO (p, *((SVEC *)V), UU->v); return (p);}
    if (UU->type==TYPE_SVEC) return (SVEC_inpro ((SVEC *)V, (SVEC *)U));
//  if (UU->type==TYPE_QUEUE) return (VEC_QUEUE_inpro (V, U));
  }
  if (VV->type==TYPE_QUEUE){
    if (UU->type==TYPE_VEC){ ARY_QUEUE_INPRO (p, *((QUEUE *)V), UU->v); return (p); }
//    else if (UU->type==TYPE_SVEC) return (SVEC_inpro (V, U));
    if (UU->type==TYPE_QUEUE){ MQUE_INTSEC (p, *((QUEUE *)V), *((QUEUE *)U)); return (p);}
  }
  return (0.0);
}

double MVEC_double_inpro (void *V, double *w){
  VEC *VV = (VEC *)V;
  double p;
  if (VV->type==TYPE_VEC){ ARY_INPRO (p, VV->v, w, VV->t); return (p); }
  if (VV->type==TYPE_SVEC){ ARY_SVEC_INPRO (p, *((SVEC *)V), w); return (p); }
  if (VV->type==TYPE_QUEUE){ ARY_QUEUE_INPRO (p, *((QUEUE *)V), w); return (p); }
  return (0.0);
}
*/
/* compute the inner product, euclidean distance for i,jth vector */
/*
double MMAT_inpro_ij (void *M, int i, int j){
  MAT *MM = (MAT *)M;
  double p;
  if (MM->type==TYPE_MAT){ ARY_INPRO (p, MM->v[i].v, MM->v[j].v, MM->v[j].t); return (p); }
  if (MM->type==TYPE_SMAT) return (SVEC_inpro (&((SMAT *)M)->v[i], &((SMAT *)M)->v[j]));
  if (MM->type==TYPE_SETFAMILY){
     p = QUEUE_intsec_ (&((SETFAMILY *)M)->v[i], &((SETFAMILY *)M)->v[j]); return (p); }
  return (0.0);
}

double MMAT_double_inpro_i (void *M, int i, double *w){
  MAT *MM = (MAT *)M;
  double p;
  if (MM->type==TYPE_MAT){ ARY_INPRO (p, MM->v[i].v, w, MM->v[i].t); return (p); }
  if (MM->type==TYPE_SMAT){ ARY_SVEC_INPRO (p, ((SMAT *)M)->v[i], w); return (p); }
  if (MM->type==TYPE_SETFAMILY){ ARY_QUEUE_INPRO (p, ((SETFAMILY *)M)->v[i], w); return (p); }
  return (0.0);
}

#ifdef _barray_h_
void SETFAMILY_to_BARRAY (BARRAY *A, SETFAMILY *F){
  VEC_ID t;
  size_t i=0;
  BARRAY_init (A, F->clms, F->t);
  FLOOP (t, 0, F->t){
    BARRAY_set_subset (&A->v[i], &F->v[t]);
    i += A->xend;
  }
}
#endif
*/
//#endif


