/* library for sparse vector */
/* Takeaki Uno    27/Dec/2008 */
#pragma once


//#define USE_MATH

#include"math.h"
#include"stdlib2.hpp"
#include"file2.hpp"
#include"filecount.hpp"
#include"queue.hpp"

#ifndef SVEC_VAL
 #ifdef SVEC_VAL_INT
  #define SVEC_VAL int
  #define SVEC_VAL2 LONG
  #define SVEC_VAL_END INTHUGE
  #define SVEC_VAL2_END LONGHUGE
  #define SVEC_VALF "%d"
 #else
  #define SVEC_VAL double
  #define SVEC_VAL2 double
  #define SVEC_VAL_END DOUBLEHUGE
  #define SVEC_VAL2_END DOUBLEHUGE
  #define SVEC_VALF "%f"
 #endif
#endif


class SETFAMILY{

  int _flag;

  VEC_ID _end;
  VEC_ID _t;
  QUEUE_INT *_buf;
  VEC_ID _clms;
  size_t _eles;  //, _ele_end;
  WEIGHT *_rw, **_w, *_wbuf;

  PERM *_rperm, *_cperm;  // row permutation
  
  PERM *_sortPerm;

  QUEUE *_v;
  
	void _flie_load(IFILE2 &fp);

	//void alloc(VEC_ID rows, FILE_COUNT &fc, VEC_ID clms, size_t eles);


	void any_INVPERMUTE_rw(PERM * rperm){

		WEIGHT w;

		int i1,i2;
		char  *cmm_p =  new char[_t](); //calloc2

		for(i1=0; i1 < _t ; i1++){
			if ( cmm_p[i1]==0 ){ 
				w = _rw[i1]; 
				do{ 
					i2 = i1;
					i1 = rperm[i1];
					_rw[i2] = _rw[i1];
					cmm_p[i2] = 1 ;
				} while(cmm_p[i1]==0) ;
				_rw[i2] = w; 
			}
		}
		delete [] cmm_p; 
	}

	public:
		//,_ele_end(0),
		SETFAMILY():
			 _flag(0),_v(NULL),
			_end(0),_t(0),_buf(NULL),_clms(0),_eles(0),
			_rw(NULL),_w(NULL),_wbuf(NULL),
  		_rperm(NULL),_cperm(NULL){}

		~SETFAMILY(){
			mfree ( _rw, _wbuf, _w, _cperm);
			delete []  _rperm;
			delete []  _buf;
			delete []  _v;
		}

		// どっちか一方でいい？
		// sgraph _edge.sort(flag) とprivate  load
		void sort();
		void sort(int flag);

		void alloc_w (){  _w = new WEIGHT*[_end](); }

		void alloc_weight (FILE_COUNT &fc);


		void load (IFILE2 &fp, int flag);

		// call from trsact.cpp
		void file_read(IFILE2 &fp,             FILE_COUNT &C , VEC_ID *pos ,int flag,int tflag);
		void file_read(IFILE2 &fp,IFILE2 &wfp, FILE_COUNT &C , VEC_ID *pos ,int flag,int tflag);


		void fileRead (IFILE2 &fp,IFILE2 &fp2, char *iwfname ,FILE_COUNT &C ,int flag,int tflag);


		
		void clrMark(int i,char* mark){
			_v[i].clrMark(mark);
		}

		void delivery_iter( 
			WEIGHT *w, WEIGHT *pw, 
			VEC_ID t, QUEUE_INT m, 
			QUEUE *jmp,QUEUE *oq,
			VECARY<WEIGHT> &tw, WEIGHT *tpw,int f){
		
		  WEIGHT *y=0;
  		QUEUE_INT *x;

		  if ( _w!=NULL ){  y = _w[t]; }

		  _v[t].delivery_iter( w, pw, t, m ,jmp,oq,tw,tpw,y,f);

		  
		}

		VEC_ID * counting(){

			VEC_ID * p = new VEC_ID[_clms](); // calloc2

			for (VEC_ID iv=0 ; iv< _t ; iv++){

				for (QUEUE_INT *x=_v[iv].begin() ; *x < _clms ; x++){
					p[*x]++;
				}

			}
			return p;
		}

		PERM *__qsort_perm_v(int unit);


		QUEUE_INT * get_vv(int i)      { return _v[i].get_v(); }
		QUEUE_INT   get_vv(int i,int j){ return _v[i].get_v(j); }

		QUEUE_INT *begin(int i) { return _v[i].begin(); }
		QUEUE_INT * end(int i) { return _v[i].end(); }

		QUEUE_ID get_vt(int i){ return _v[i].get_t(); }


		QUEUE get_v(int i){ return _v[i]; }

		QUEUE* getp_v(int i){ return &_v[i]; }
				

		void init_v(int i){ _v[i].init(); }

		void set_vv(int i,int j,QUEUE_INT v){ _v[i].set_v(j,v); }
		void set_vv(int i,QUEUE_INT* v){ _v[i].set_v(v); }

		void set_vt(int i,QUEUE_ID v){ _v[i].set_t(v); }

		void initQUEUEs(void){
			for(int i=0 ; i< _t;i++){
				//std::cerr << "qinit " << i << " " << _v[i].get_t() << " " << _clms << std::endl;
				//_v[i].set_v( _v[i].get_t() , _clms);
				_v[i].setStopper(_clms);
				//_v[i].set_v( _v[i].get_t() , _clms);
			}
		}

		void setVBuffer(int i,size_t v){
			 _v[i].set_v( _buf+v ); 
		}

		void qsort_perm(PERM * rperm,int flag){
			qsort_perm__( _v, _t, rperm, flag); 
		}

		void push_back(int i,QUEUE_INT v){ _v[i].push_back(v); }

		bool exist_v(){ return _v!= NULL ;}

		LONG get_v_ele(int i,QUEUE_INT v){ return _v[i].ele(v); }

		void v_rm(int u,QUEUE_ID i){ _v[u].rm(i); }

		void vw_rm(QUEUE_INT u,QUEUE_ID v){ 

		  QUEUE_INT i;

			if ( (i=(QUEUE_INT)_v[u].ele(v) ) >= 0 ){
				_v[u].rm(i); 
				if(_w){ _w[u][i] = _w[u][_v[u].get_t()]; }
				_eles--;
			}

			if ( (i=(QUEUE_INT)_v[v].ele(u) ) >= 0 ){
				_v[v].rm(i); 
				if(_w){ _w[v][i] = _w[v][_v[v].get_t()]; }
				_eles--;
			}

		}
		
		void vw_mk(QUEUE_INT u, QUEUE_INT v, WEIGHT w){
			if(_w){
				_w[u][_v[u].get_t()] = w;
				_w[v][_v[v].get_t()] = w;
			}
			_v[u].push_back(v);
			_v[v].push_back(u);
			_eles += 2;
  	}

		void rmSelfLoop(){

			QUEUE_ID jj;
			QUEUE_INT x;

			if ( !_v ) { return ;}

			for(QUEUE_ID i=0 ; i<_t ; i++){

	      jj = 0;
	
				for(QUEUE_ID j=0 ; j < _v[i].get_t() ; j++){
		
					x = _v[i].get_v(j);		

      	  if ( x != i ){
        	  if ( j != jj ){
        	  	_v[i].cp_v(j,jj);
        	    if ( _w ){ _w[i][jj]=_w[i][j]; }
	          }
  	        jj++;
    	    }
      	}
      	_v[i].set_t(jj);
			}
		}

		void swap_vv(int i,int j){
			_v[i].swap_v(j);
		}


		void setSize(FILE_COUNT &_C,int flag){			

 			if ( _C.r_eles() > _C.c_eles() && !(flag & LOAD_TPOSE) ){
 				_eles = _C.c_eles();
 			}
 			else{
 				_eles = _C.r_eles();
 			}

			_clms = _C.c_clms(); 
			_t    = _C.r_clms();
			
  		//_ele_end = _eles;

  		_end = _t * ( (flag&LOAD_DBLBUF) ? 2 : 1 ) + 1  ;

		  _v   = new QUEUE[_end]; 
		  _buf = new QUEUE_INT[_eles+_end+1];

		  return ;

		}

		// _eles 要素数 TPOSEの場合はtraでの件数優先
  	//_ele_end = _eles;
		void setSize4sspc(FILE_COUNT &_C,bool tpose,char *iwfname){			

			_eles = _C.OptimalEleSize(tpose);
			_clms = _C.c_clms(); 
			_t    = _C.r_clms();
  		_end     = _t  + 1  ;

		  _v   = new QUEUE[_end];
		  _buf = new QUEUE_INT[_eles+_end+1];

			if ( _w==NULL && iwfname ) {

				_w    = new WEIGHT*[_end +1]();//();
			  _wbuf = new WEIGHT [_eles+1]();
			  _w[0] = _wbuf; 
			  for(size_t i=1; i<_t ; i++){
			  	_w[i] = _w[i-1] + _C.get_rowt(i-1);
  			}

		 	}

		  return ;

		}

		void setSize_sg(FILE_COUNT &C,int flag){			

		  int wflag = flag&LOAD_EDGEW;

			if ( _clms == 0 ) _clms = C.clms();
			if ( _t == 0 )    _t = C.rows();

			//alloc ( _t, C, _clms, 0);
			//_ele_end =  C.sumRow(0, _t);
			_eles = C.sumRow(0, _t);

		  //_buf = new QUEUE_INT[
  		//	(_ele_end*((_flag&LOAD_DBLBUF)?2:1) +
		  //	(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(_t,_clms):_t)+2)
  		//]();

		  _buf = new QUEUE_INT[
  			(_eles*((_flag&LOAD_DBLBUF)?2:1) +
		  	(((_flag&LOAD_DBLBUF)||(_flag&LOAD_ARC))?MAX(_t,_clms):_t)+2)
  		]();


			try {
	 			_v = new QUEUE[_t+1];
			} catch(...){
				delete [] _buf;
				throw;
			}
			for(size_t i =0 ;i<_t;i++){ 
				_v[i] = QUEUE(); 
			}
 
  		_end = _t;

			QUEUE_INT *pos = _buf;

		  if ( !C.rowEmpty() ){
  			for(VEC_ID i=0 ; i<_t ; i++){
      		_v[i].set_v(pos);
      		_v[i].set_end( C.get_rowt(i)+1 );
      		pos += (C.get_rowt(i) + 1);
    		}
		  }

			if ( wflag ) alloc_weight ( C );

		  return ;

		}

		void queueSortALL(int flag){

			for(size_t t=0 ; t < _t ; t++){
				//std::cerr << "st " << t << "/" << _t << std::endl;
				_v[t].queSort(flag);
				//std::cerr << "et " << t << "/" << _t << std::endl;
			}

		}

		void rmDup(){
			for(size_t t=0 ; t < _t ; t++){
				_v[t].rmDup(_clms);
			}
		}

		VEC_ID RowMax(){
			QUEUE_ID rmax = _v[0].get_t();
			VEC_ID rpos=0;
			for( size_t t=1 ; t < _t ; t++ ){
				if( rmax < _v[t].get_t() ){
					rpos=t;
					rmax =_v[t].get_t();
				}
			}
			return rmax;
		}
		
		void any_INVPERMUTE(PERM * rperm){

		  QUEUE Q;
			int i1,i2;
			char * cmm_p = new char[_t](); //calloc2( cmm_p ,_t);

			for(i1=0; i1 < _t ; i1++){
				if ( cmm_p[i1]==0 ){ 
					Q = _v[i1]; 
					do{ 
						i2 = i1;
						i1 = rperm[i1];
						_v[i2] = _v[i1];
						cmm_p[i2] = 1 ;
					} while(cmm_p[i1]==0) ;
					_v[i2] = Q; 
				}
			}

			delete [] cmm_p; 

		}

		void ary_INVPERMUTE( PERM *invperm ,QUEUE& Q,VEC_ID num){

			int i1,i2;
			char  * cmm_p = new char[num](); //calloc2(cmm_p,num);

			for(i1=0; i1 < num ; i1++){

				if ( cmm_p[i1]==0 ){ 
					Q = _v[i1]; 
					do{ 
						i2 = i1;
						i1 = invperm[i1];
						_v[i2] = _v[i1];
						cmm_p[i2] = 1 ;
					} while(cmm_p[i1]==0);
					_v[i2] = Q; 
				}

			}
			delete [] cmm_p; 
		}

		void any_INVPERMUTE_w(PERM * rperm){

		  WEIGHT * ww;
			int i1,i2;

			char  * cmm_p = new char[_t](); //calloc2(cmm_p,_t);

			for(i1=0; i1 < _t ; i1++){
				if ( cmm_p[i1]==0 ){ 
					ww = _w[i1]; 
					do{ 
						i2 = i1;
						i1 = rperm[i1];
						_w[i2] = _w[i1];
						cmm_p[i2] = 1 ;
					} while(cmm_p[i1]==0) ;
					_w[i2] = ww; 
				}
			}
			delete [] cmm_p; 
		}


		void ary_INVPERMUTE_(PERM *p,QUEUE& Q){

			int i1 ,i2;

			for( i1=0; i1 < _t ; i1++){

				if ( p[i1] < _t ){ 
					Q = _v[1];

					do{

						i2 = i1;
						i1 = p[i1];
						_v[i2] = _v[i1]; 
						p[i2] = _t; 

					} while( p[i1] < _t ); 

					_v[i2] = Q;
				}
			}
		}

		void any_INVPERMUTE_(PERM * trperm,PERM * rperm){
			PERM pp;
			int i1,i2;
			
			for( i1=0; i1 < _t ; i1++){
				if ( rperm[i1] < _t ){ 
					pp = trperm[i1]; 
					do { 
						i2 = i1; 
						i1 = rperm[i1]; 
						trperm[i2]=trperm[i1]; 
						rperm[i2]=_t; 
					}while ( rperm[i1] < _t ); 
					trperm[i2] = pp;
				}
			}
		}

  	bool exist_w(){ return _w!=NULL; }
  	bool exist_rw(){ return _rw!=NULL; }

  	WEIGHT **get_w(){ return _w; }
  	WEIGHT *get_w(int i){ return _w[i]; }
  	WEIGHT get_w(int i,int j){ return _w[i][j]; }

  	WEIGHT get_rw(int i){ return _rw[i]; }

  	PERM *get_rperm(){ return _rperm;}

  	QUEUE_INT *get_buf(){ return _buf; }

  	VEC_ID get_t(void){ return _t; }
  	VEC_ID get_clms(void){ return _clms; }
  	VEC_ID get_end(void){ return _end; }

  	size_t get_eles(void){ return _eles; }

  	void set_w(int i,int j,WEIGHT w){ _w[i][j]=w;}
  	void set_w(int i,WEIGHT *w){ _w[i]=w;}
  	void set_rw(int i,WEIGHT w){ _rw[i]=w;}
  	void set_rw(WEIGHT *w){ _rw=w;}
  	void set_rperm(PERM *rperm){ _rperm=rperm;}

		void setInvPermute(PERM *rperm,PERM *trperm,int flag);
		void replace_index(PERM *perm, PERM *invperm);

};

		//readFile(IFILE2 &fp, IFILE2 &fp2, int flag);
