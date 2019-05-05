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
  //char *buf;
  if ( eles == 0 ) { _ele_end = ARY_SUM( rowt, 0, rows); }
  else { _ele_end = eles; }

  //calloc2 (buf, (_ele_end*((_flag&LOAD_DBLBUF)?2:1) +(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)*_unit, EXIT);

  //buf = calloc2 (
  //	buf, 
  //	(_ele_end*((_flag&LOAD_DBLBUF)?2:1) +(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)*_unit
  //);
  //_buf = (QUEUE_INT *)buf;

  _buf = new QUEUE_INT[(_ele_end*((_flag&LOAD_DBLBUF)?2:1) +(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)]();
  //malloc2 (_v, rows+1, {free(_buf);EXIT;});
  try {
	 //_v = malloc2 (_v, rows+1 );
	 _v = new QUEUE[rows+1];
	} catch(...){
		free(_buf);
		delete [] _buf;
		throw;
	}
	for(size_t i =0 ;i<rows;i++){ 
		_v[i] = QUEUE(); 
	}

  _end = rows;
  _clms = clms;

	QUEUE_INT *pos = _buf;
  if ( rowt ){
  	for(i=0;i<rows;i++){
      _v[i].set_v(pos);
      _v[i].set_end(rowt[i]+1);
      pos += (rowt[i] +1);
    }
  }
}

/* allocate memory according to rows and rowt */
/* if eles == 0, compute eles from rowt and rows */
void SETFAMILY::alloc_weight ( QUEUE_ID *t){
  VEC_ID i;
  //calloc2 (_w, _end +1, EXIT);
	_w = new WEIGHT*[_end +1]();

  //calloc2 (_wbuf, _ele_end*((_flag&LOAD_DBLBUF)?2:1)+1, {free(_w);EXIT;});
	try{
	  _wbuf = new WEIGHT[_ele_end*((_flag&LOAD_DBLBUF)?2:1)+1]();

	}catch(...){
		delete [] _w;
		throw;
	}
  _w[0] = _wbuf; 
  //FLOOP (i, 1, _t){
  for(i=1;i<_t;i++){
  	_w[i] = _w[i-1] + (t?t[i-1]:_v[i-1].get_t());
  }
}

void SETFAMILY::alloc_w (){
  //calloc2 (_w, _end, EXIT);
  _w = new WEIGHT*[_end]();
}


/* sort and duplication check */
void SETFAMILY::sort (){
  VEC_ID i;
  PERM *p;
  WEIGHT *ww, w;
  QUEUE Q;

  int flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);
  // sort items in each row
  if ( flag ){   
    //malloc2 (p, _clms, EXIT);
    // p = malloc2 (p, _clms);
    p = new PERM[_clms];
    for(i=0;i<_t;i++){
      _v[i].perm_WEIGHT( _w?_w[i]:NULL, p, flag);
    }
    delete [] p;
    p=NULL;
  }

  flag = ((_flag&(LOAD_SIZSORT+LOAD_WSORT))? ((_flag&LOAD_DECROWSORT)? -1: 1): 0);

  if ( flag ){  
		// sort the rows
		if ( _flag & LOAD_SIZSORT ) {
			p = qsort_perm_(_v, _t, flag);
		}
    else{
    	p = qsort_perm_(_rw, _t, flag);
    }

		//_rperm = malloc2(_rperm,_t);
		_rperm = new PERM[_t];

		for(size_t st=0; st<_t ;st++){ _rperm[st]=-1; }

		for(int i=0;i<_t;i++){
			if(p[i]>=0 && p[i]<_t){ _rperm[p[i]]=i; }
		}

    if ( _rw ){
			any_INVPERMUTE_rw(p);
    }
    if ( _w ){
	    any_INVPERMUTE_w(p);
    }
    
    ary_INVPERMUTE_(p,Q);

    delete [] p;
  }

  if (_flag&LOAD_RM_DUP){  // unify the duplicated edges
    //FLOOP (i, 0, _t){
    for(i=0;i<_t;i++){
			_v[i].rm_dup_WEIGHT( _w?_w[i]:NULL);
		}
  }
}

void SETFAMILY::SMAT_alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){
  VEC_ID i;
  if ( eles == 0 ) { _ele_end = ARY_SUM( rowt, 0, rows); }
  else {  _ele_end = eles; } 

  //calloc2 (_buf, _ele_end*((_flag&LOAD_DBLBUF)?2:1) +rows +2, EXIT);
  _buf = new QUEUE_INT[_ele_end*((_flag&LOAD_DBLBUF)?2:1) +rows +2]();

	try{
	  //_v = malloc2 (_v, rows+1);
	  _v = new QUEUE[rows+1];
	}catch(...){
		delete [] _buf;
		throw;
	}
  //ARY_FILL (_v, 0, rows, SVEC() );

	for(size_t _common_size_t =0;_common_size_t<rows;_common_size_t++){
		_v[_common_size_t].set_end(0);
	  _v[_common_size_t].set_t(0);
	}

  _end = rows;
  _clms = clms;
  if ( rowt ){
    //FLOOP (i, 0, rows){
    for(i=0;i<rows;i++){
      _v[i].set_v( i? _v[i-1].get_v() + rowt[i-1] +1: _buf);
      _v[i].set_end( rowt[i]);
    }
  }
}


void SETFAMILY::flie_load(FILE2 *fp){

  WEIGHT z=0;
  VEC_ID t;
  LONG x, y;
  int fc=0, FILE_err_=0, flag2=_flag;
  int flag=0;

  int wflag =  ( _wfname || (_flag&LOAD_EDGEW));
  FILE_COUNT C;
  FILE2 wfp;

  if ( flag && !_wfname ) flag2 = _flag | LOAD_EDGEW;

  C.count(fp,(_flag&(LOAD_ELE+LOAD_TPOSE+LOAD_RC_SAME+LOAD_EDGE)) | FILE_COUNT_ROWT, 0, 0, 0, (flag2&LOAD_EDGEW)?1:0, 0);

  if ( _clms == 0 ) _clms = C.get_clms();
  if ( _t == 0 ) _t = C.get_rows();
  if ( flag ) SMAT_alloc ( _t, C.getp_rowt(), _clms, 0);
  else {
    alloc ( _t, C.getp_rowt(), _clms, 0);
    if ( wflag ) alloc_weight ( C.getp_rowt());
  }

  if ( _wfname ) wfp.open ( _wfname, "r");
  if ( _ERROR_MES ) return;

  fp->reset ();
  if ( _flag&(LOAD_NUM+LOAD_GRAPHNUM) ) fp->read_until_newline ();
  t=0;
  do {
    if ( _flag&LOAD_ELE ){
      if ( fp->read_pair ( &x, &y, &z, flag2) ) continue;
    } else {
      x = t;
      FILE_err_ = fp->read_item ( _wfname?&wfp:NULL, &x, &y, &z, fc, flag2);
      //if ( FILE_err&4 ) goto LOOP_END;
      if ( fp->readNG() ) goto LOOP_END;

    }

    if ( y >= _clms || x >= _t ) continue;

    if ( flag ){
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
      //if ( FILE_err&3 ){
      if ( fp->getOK() ){
        LOOP_END:;
        t++; if ( t >= _t ) break;
        fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
      }
    }
//  } while ( (FILE_err&2)==0 );
  } while ( fp->eof());
  if ( _wfname ) wfp.close ();

}


/* scan file and load the data from file to SMAT structure */
void SETFAMILY::load (int flag , char *fname)
{

  FILE2 fp;
  VEC_ID i;
  _fname = fname;
  _flag = flag;

  fp.open ( _fname, "r");
	flie_load(&fp);
  fp.close ();     

  if(_ERROR_MES) EXIT;

  printMes("setfamily: %s ,#rows %d ,#clms %d ,#eles %zd", _fname, _t, _clms, _eles);

  if (_wfname ) printMes(" ,weightfile %s", _wfname);
  printMes("\n");
 
  sort ();

	// end mark これいる？
	for(i=0;i<_t;i++){
	  _v[i].set_v(_v[i].get_t(),_clms); 
	}

  _eles = _ele_end;

	for(int i=0 ; i< _t;i++){ // allvvInitByT();
		_v[i].set_v( _v[i].get_t() , _t);
	}

  sort(); //これいる？

}

void SETFAMILY::replace_index(PERM *perm, PERM *invperm)
{
	if ( _v ){
		for(size_t i=0; i<_t ; i++){
			for( QUEUE_INT *x = _v[i].begin() ; x < _v[i].end() ; x++ ){
				*x = perm[*x];
			}
		}
		// INVPERMUTE
		char * cmmp=NULL; 
		QUEUE Q;
		int i1,i2;
		cmmp = new char[_t](); // calloc2(cmmp,_t,EXIT);
		for( i1 = 0; i1 < _t ; i1++ ){
			if ( cmmp[i1]==0 ){ 
				Q = _v[i1]; 
				do{ 
					i2 = i1; 
					i1 = invperm[i1]; 
					_v[i2]=_v[i1]; 
					cmmp[i2] = 1;
				} while( cmmp[i1]==0 );
				_v[i2] = Q; 
			}
		}
		delete [] cmmp;
	}

	if ( _w ){
		// INVPERMUTE
		char * cmmp=NULL; 
		WEIGHT *w;
		int i1,i2;
		
		cmmp = new char[_t](); //calloc2(cmmp,_t,EXIT);
		for( i1 = 0; i1 < _t ; i1++ ){
			if ( cmmp[i1]==0 ){ 
				w = _w[i1]; 
					do{ 
						i2 = i1; 
						i1 = invperm[i1]; 
						_w[i2]=_w[i1]; 
						cmmp[i2] = 1;
					} while( cmmp[i1]==0 );
					_w[i2] = w; 
			}
		}
		delete [] cmmp;
	}

}


void SETFAMILY::setInvPermute(PERM *rperm,PERM *trperm,int flag)
{
	QUEUE Q;	
	char  *cmm_p=NULL;
	int cmm_i,cmm_i2;

	qsort_perm__( _v, _t, rperm, flag); 

	
	cmm_p = new char[_t](); //calloc2(cmm_p,_t,EXIT);

	for(cmm_i=0;cmm_i<_t;cmm_i++){
		if ( cmm_p[cmm_i]==0 ){ 
			Q = _v[cmm_i]; 
			do{ 
				cmm_i2=cmm_i; 
				cmm_i=rperm[cmm_i]; 
				_v[cmm_i2]=_v[cmm_i]; 
				cmm_p[cmm_i2]=1; 
			}while( cmm_p[cmm_i]==0 ); 
			_v[cmm_i2] = Q; 
		}
	}
	delete [] cmm_p; 
			
	if(_w){

		WEIGHT *ww;
		char  *cmm_p = new char[_t](); //calloc2(cmm_p,_t,EXIT);

		for(cmm_i=0;cmm_i<_t;cmm_i++){
			if ( cmm_p[cmm_i]==0 ){ 
				ww = _w[cmm_i]; 
				do{ 
					cmm_i2=cmm_i; 
					cmm_i=rperm[cmm_i]; 
					_w[cmm_i2]=_w[cmm_i]; 
					cmm_p[cmm_i2]=1; 
				}while( cmm_p[cmm_i]==0 ); 
				_w[cmm_i2] = (ww); 
			}
		}
		delete [] cmm_p; 

	} 
			
	PERM pp;

	for(cmm_i=0;cmm_i<_t;cmm_i++){

		if ( rperm[cmm_i]< _t ){ 
			pp = trperm[cmm_i]; 
			do { 
				cmm_i2=cmm_i; 
				cmm_i=rperm[cmm_i]; 
				trperm[cmm_i2]=trperm[cmm_i]; 
				rperm[cmm_i2]=_t; //?
			}while ( rperm[cmm_i]< _t ); 
			trperm[cmm_i2] = pp;
		}
	}
}
