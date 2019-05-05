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

  char *_fname, *_wfname;      // input/weight file name
  int _flag;         // flag
  VEC_ID _end;
  VEC_ID _t;
  QUEUE_INT *_buf, *_buf2;
  VEC_ID _clms;
  size_t _eles, _ele_end;
  WEIGHT *_cw, *_rw, **_w, *_wbuf;
  int _unit;
  char *_cwfname, *_rwfname;     // weight file name
  PERM *_rperm, *_cperm;  // row permutation

  QUEUE *_v;
  
	char *_ERROR_MES;

	void flie_load(FILE2 *fp);

	void SMAT_alloc (VEC_ID rows, FILE_COUNT &fc, VEC_ID clms, size_t eles);

	//void end ();

	void print (FILE *fp);
	void print_weight (FILE *fp);

	void *getvec ( int i){ return &_v[i]; }

	public:

		SETFAMILY():
			_fname(NULL),_wfname(NULL), _flag(0),_v(NULL),
			_end(0),_t(0),_buf(NULL),_buf2(NULL),_clms(0),_eles(0),_ele_end(0),
 		 	_cw(NULL),_rw(NULL),_w(NULL),_wbuf(NULL),_unit(sizeof(QUEUE_INT)),
  		_cwfname(NULL),_rwfname(NULL),_rperm(NULL),_cperm(NULL),_ERROR_MES(NULL){}

		~SETFAMILY(){
			mfree (_buf2, _rw, _cw, _wbuf, _w, _cperm);
			delete []  _rperm;
			delete []  _buf;
			delete []  _v;
		}


		void alloc_weight (FILE_COUNT &fc);
		void alloc_w(void);
		void alloc(VEC_ID rows, FILE_COUNT &fc, VEC_ID clms, size_t eles);
		void sort(void);

		void load(int flag , char *fname);

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

			VEC_ID * p ;
  	  // calloc2 (p, _clms, {exit(1);});
  	  p = new VEC_ID[_clms]();

			for (VEC_ID iv=0 ; iv< _t ; iv++){

				for (QUEUE_INT *x=_v[iv].begin() ; *x < _clms ; x++){
					p[*x]++;
				}

			}
			return p;
		}

		QUEUE_INT * get_vv(int i)      { return _v[i].get_v(); }
		QUEUE_INT   get_vv(int i,int j){ return _v[i].get_v(j); }

		QUEUE_INT * end(int i) { return _v[i].end(); }

		QUEUE_ID get_vt(int i){ return _v[i].get_t(); }


		QUEUE get_v(int i){ return _v[i]; }

		QUEUE* getp_v(int i){ return &_v[i]; }
				

		void init_v(int i){ _v[i].init(); }

		void set_vv(int i,int j,QUEUE_INT v){ _v[i].set_v(j,v); }
		void set_vv(int i,QUEUE_INT* v){ _v[i].set_v(v); }

		void set_vt(int i,QUEUE_ID v){ _v[i].set_t(v); }

		void allvvInit(void){
			for(int i=0 ; i< _t;i++){
				_v[i].set_v( _v[i].get_t() , _clms);
			}
		}

		void allvvInitByT(void){
			for(int i=0 ; i< _t;i++){
				_v[i].set_v( _v[i].get_t() , _t);
			}
		}

		void setvvByPos(PERM pos){
			_v[pos].set_v( _v[pos-1].end() +1);
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
		// 要確認
		void alloc_v(){
		  _v = new QUEUE[_end];  //malloc2(_v, _end);
		}
		void alloc_buf(){
		  _buf = new QUEUE_INT[_eles+_end+1]; //malloc2 (buf,(_eles+_end+1)*_unit);

		}
		
		void allocBuffer(){			
		  _v   = new QUEUE[_end];  // _v = malloc2 (_v, _end );
		  _buf = new QUEUE_INT[_eles+_end+1]; //malloc2( buf,(_eles+_end+1) * _unit);
		  return ;
		}



		void queueSortALL(int flag){
			for(size_t t=0 ; t < _t ; t++){
				_v[t].queSort(flag);
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

		void any_INVPERMUTE_rw(PERM * rperm){

		  WEIGHT w;

			char  * cmm_p;
			int i1,i2;

			//cmm_p = calloc2(cmm_p,_t);
			cmm_p = new char[_t]();

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
  	VEC_ID get_end(void){ return _end; }
  	VEC_ID get_clms(void){ return _clms; }
  	int get_unit(void){ return _unit; }
  	size_t get_eles(void){ return _eles; }

  	VEC_ID postinc_clms(){ 
  		VEC_ID rtn = _clms++;
  		return rtn; 
  	}
  	
  	VEC_ID postinc_t(){
	  	VEC_ID rtn = _t++;
  		return rtn; 
  	}

  	void set_fname(char *fname){  _fname=fname; }
  	void set_wfname(char *wfname){  _wfname=wfname; }

  	void set_clms(VEC_ID clms){  _clms=clms; }
  	void set_t(VEC_ID t){  _t=t; }

  	void add_eles(size_t add ){  _eles+=add; }
  	void set_ele_end(size_t eles){ _ele_end=eles; }
  	void set_eles(size_t eles){ _eles=eles; }
  	void set_end(VEC_ID end){ _end=end; }
  	void set_buf(QUEUE_INT *buf){ _buf = buf; }

  	void dec_eles(){  _eles--; }

  	void adjustEnd(int dblFlg){ 
  		_ele_end = _eles;
  		_end = _t * ( dblFlg ? 2 : 1 ) + 1  ;
  	}

  	void set_w(int i,int j,WEIGHT w){ _w[i][j]=w;}
  	void set_w(int i,WEIGHT *w){ _w[i]=w;}
  	void set_rw(int i,WEIGHT w){ _rw[i]=w;}
  	void set_rw(WEIGHT *w){ _rw=w;}
  	void set_rperm(PERM *rperm){ _rperm=rperm;}

  	void setwByIW(PERM i,WEIGHT w){ 
  		_w[i][_v[i].get_t()]=w;
  	}



  	void union_flag(int flag){ _flag |= flag;}

		void setInvPermute(PERM *rperm,PERM *trperm,int flag);
		void replace_index(PERM *perm, PERM *invperm);

		void printMes(char *frm ,...){

			if( _flag&1 ){
				va_list ap;
				va_start(ap,frm);
				fprintf(stderr,frm,ap);
				va_end(ap);
			}
		}

};



