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
// void SETFAMILY::alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles){
void SETFAMILY::alloc (VEC_ID rows, FILE_COUNT &fc, VEC_ID clms, size_t eles){

  VEC_ID i;

  if ( eles == 0 ) { _ele_end =  fc.sumRow(0, rows);  }
  else { _ele_end = eles; }

  //calloc2 (buf, (_ele_end*((_flag&LOAD_DBLBUF)?2:1) +(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)*_unit, EXIT);
  _buf = new QUEUE_INT[
  	(_ele_end*((_flag&LOAD_DBLBUF)?2:1) +
  	(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(rows,clms):rows)+2)
  ]();

  try {
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

  if ( !fc.rowEmpty() ){
  	for(i=0;i<rows;i++){
      _v[i].set_v(pos);
      _v[i].set_end( fc.get_rowt(i)+1 );
      pos += (fc.get_rowt(i) + 1);
    }
  }


}

/* allocate memory according to rows and rowt */
/* if eles == 0, compute eles from rowt and rows */
//void SETFAMILY::alloc_weight ( QUEUE_ID *t){
void SETFAMILY::alloc_weight (FILE_COUNT &fc){

  VEC_ID i;
  
	_w = new WEIGHT*[_end +1](); //calloc2 (_w, _end +1, EXIT);

	try{
	  _wbuf = new WEIGHT[_ele_end*((_flag&LOAD_DBLBUF)?2:1)+1](); //calloc2
	}catch(...){
		delete [] _w;
		throw;
	}
  _w[0] = _wbuf; 
  for(i=1;i<_t;i++){
  	_w[i] = _w[i-1] + ( fc.rowEmpty() ?  _v[i-1].get_t() : fc.get_rowt(i-1));
  }
}


/* sort and duplication check */
void SETFAMILY::sort(){
  VEC_ID i;
  PERM *p;
  WEIGHT *ww, w;
  QUEUE Q;
	
  int flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);
  // sort items in each row

  if ( flag ){   
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

    if ( _rw ){ any_INVPERMUTE_rw(p); }
    if ( _w ) { any_INVPERMUTE_w(p);  }
    ary_INVPERMUTE_(p,Q);

    delete [] p;
  }


  if (_flag&LOAD_RM_DUP){  // unify the duplicated edges
    for(i=0;i<_t;i++){
			_v[i].rm_dup_WEIGHT( _w?_w[i]:NULL);
		}
  }
}

void SETFAMILY::sort(int sflag){
	
  VEC_ID i;
  PERM *p = NULL;
  WEIGHT *ww, w;
  QUEUE Q;

	_flag |= sflag;
	
  int flag = (_flag&LOAD_INCSORT)? 1: ((_flag&LOAD_DECSORT)? -1: 0);

  // sort items in each row
  if ( flag ){   
    
    p = new PERM[_clms]; // p = malloc2 (p, _clms);

    for(i=0;i<_t;i++){
      _v[i].perm_WEIGHT( _w?_w[i]:NULL, p, flag);
    }
    delete [] p;
    p = NULL;
  }

  flag = ((_flag&(LOAD_SIZSORT+LOAD_WSORT))? ((_flag&LOAD_DECROWSORT)? -1: 1): 0);

  if ( flag ){  
		// sort the rows
		if ( _flag & LOAD_SIZSORT ) { p = qsort_perm_(_v, _t, flag); }
    else{ p = qsort_perm_(_rw, _t, flag); }

		_rperm = new PERM[_t]; // malloc2

		for(size_t st=0; st<_t ;st++){ _rperm[st]=-1; }

		for(int i=0;i<_t;i++){
			if(p[i]>=0 && p[i]<_t){ _rperm[p[i]]=i; }
		}

    if ( _rw ){ any_INVPERMUTE_rw(p); }
    if ( _w ) { any_INVPERMUTE_w(p) ; }
    
    ary_INVPERMUTE_(p,Q);

    delete [] p;
  }


  if (_flag&LOAD_RM_DUP){  // unify the duplicated edges
    for(i=0;i<_t;i++){
			_v[i].rm_dup_WEIGHT( _w?_w[i]:NULL);
		}
  }

}

// sgraphに移行?
//void SETFAMILY::_flie_load(FILE2 &fp, FILE_COUNT &C){
void SETFAMILY::_flie_load(IFILE2 &fp){
  WEIGHT z=0;
  VEC_ID t;
  LONG x, y;
  int fc=0, FILE_err_=0, flag2=_flag;
  int flag=0;

  int wflag = _flag&LOAD_EDGEW;
  //IFILE2 wfp;

  if ( flag ) flag2 = _flag | LOAD_EDGEW;

  fp.reset ();
  if ( _flag&(LOAD_GRAPHNUM) ) fp.read_until_newline ();
  t=0;
  do {

    if ( _flag&LOAD_ELE ){

      if ( fp.read_pair ( &x, &y, &z, flag2) ) continue;

    } else {
      x = t;
      FILE_err_ = fp.read_item ( NULL, &x, &y, &z, fc, flag2);
      if ( fp.Null() ) goto LOOP_END; //( FILE_err&4 )
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
      if ( fp.EolOrEof() ){ //( FILE_err&3 )
        LOOP_END:;
        t++; if ( t >= _t ) break;
        fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
      }
    }

  } while ( fp.NotEof()); //(FILE_err&2)==0 

}

/* scan file and load the data from file to SMAT structure */
// call from sgraph.ccp loadEDGE// sgraphに移行
void SETFAMILY::load (IFILE2 &fp,  int flag)
{

  //FILE2 fp; //_fname = fname;

  _flag = flag;

	_flie_load(fp);

  sort();

	// end mark これ意味ある？　どちらか一方でOK?
	//	for(VEC_ID i=0;i<_t;i++){  _v[i].set_v(_v[i].get_t(),_clms); }

  _eles = _ele_end;

	// これ意味ある？ 　どちらか一方でOK?
	for(int i=0 ; i< _t;i++){ 
		_v[i].set_v( _v[i].get_t() , _t);
	} 	// allvvInitByT();

  sort(); //なくせる？

}

//void SETFAMILY::fileRead(IFILE2 &fp,VEC_ID *pos)


//from Trsact (Not use weight)
void SETFAMILY::file_read(
	IFILE2 &fp, FILE_COUNT &C,
	VEC_ID *pos ,
	int flag, int tflag
){ 

  LONG x, y;

	do{
		if ( flag ){
    	if ( C.CheckRperm(*pos) ){ 
				_v[ C.rperm(*pos) ].set_v( _v[C.rperm(*pos)-1].end()+1 );
    	}
    }
    x = *pos;

	  y = fp.read_int(); // fp.read_item( &x, &y, tflag);

    if ( fp.Null() ) goto LOOP_END; //FILE_err&4

	  if ( (tflag & LOAD_TPOSE) || ((tflag&LOAD_EDGE) && x > y) ){
  		SWAP_<LONG>(&x, &y);
	  }

    if ( C.rperm(x)<=C.rows() && C.cperm(y) <= C.c_end() ){
      _v[ C.rperm(x) ].push_back( C.cperm(y) );
    }

    if ( fp.EolOrEof() ){ //( FILE_err&3 )
    	LOOP_END:;
      (*pos)++;
    }
	} while ( fp.NotEof() ); // (FILE_err&2)==0

	return;

}

//from Trsact (use weight)
void SETFAMILY::file_read(
	IFILE2 &fp, IFILE2 &wfp,FILE_COUNT &C, 
	VEC_ID *pos ,
	int flag, int tflag){ 


  LONG x, y;
  WEIGHT w=0;
  int fc=0, FILE_err=0;

	do{

		if ( flag ){
    	if ( C.CheckRperm(*pos) ){ 
				_v[ C.rperm(*pos) ].set_v( _v[C.rperm(*pos)-1].end()+1 );
    	}
    }
    x = *pos;
    FILE_err = fp.read_item(&wfp, &x, &y, &w, fc, tflag);

    if ( fp.Null() ) goto LOOP_END; //FILE_err&4

    if ( C.rperm(x)<=C.rows() && C.cperm(y) <= C.c_end() ){
  		_w[ C.rperm(x) ][ _v[C.rperm(x)].get_t() ] = w;
      _v[ C.rperm(x) ].push_back( C.cperm(y) );

    }

    if ( fp.EolOrEof() ){ //( FILE_err&3 )
    	LOOP_END:;
      (*pos)++;
      // even if next weight is not written, 
      // it is the rest of the previous line
      fc = FILE_err ? 0: 1; 
      FILE_err = 0; 
    }
	} while ( fp.NotEof() ); // (FILE_err&2)==0

	return;

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

	cmm_p = new char[_t]();

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
		char  *cmm_p = new char[_t]();

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

