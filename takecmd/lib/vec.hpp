/* library for sparse vector */
/* Takeaki Uno    27/Dec/2008 */
#pragma once


//#define USE_MATH

#include"math.h"
#include"stdlib2.hpp"
#include"file2.hpp"
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
  //unsigned char _type;  // mark to identify type of the structure
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

	void SMAT_flie_load(FILE2 *fp);

	void SMAT_alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles);

	void end ();

	void load_weight ();
	void load_row_weight ();
	void load_column_weight ();
	void print (FILE *fp);
	void print_weight (FILE *fp);

	void *getvec ( int i){
		return &_v[i];
	}
  QUEUE *_v;

	public:

		SETFAMILY():
			_fname(NULL),_wfname(NULL), _flag(0),_v(NULL),
			_end(0),_t(0),_buf(NULL),_buf2(NULL),_clms(0),_eles(0),_ele_end(0),
 		 	_cw(NULL),_rw(NULL),_w(NULL),_wbuf(NULL),_unit(sizeof(QUEUE_INT)),
  		_cwfname(NULL),_rwfname(NULL),_rperm(NULL),_cperm(NULL){}

		~SETFAMILY(){
			mfree (_buf, _buf2, _v, _rw, _cw, _wbuf, _w, _rperm, _cperm);
		}
		void alloc_weight (QUEUE_ID *t);
		void alloc_w();
		void alloc (VEC_ID rows, VEC_ID *rowt, VEC_ID clms, size_t eles);
		void load();
		void sort();

		void clrMark(int i,char* mark){
			_v[i].clrMark(mark);
		}

		void delivery_iter( 
			WEIGHT *w, WEIGHT *pw, 
			VEC_ID t, QUEUE_INT m, 
			QUEUE *jmp,QUEUE *oq,
			WEIGHT *tw, WEIGHT *tpw,int f){
		
		  WEIGHT *y=0;
  		QUEUE_INT *x;

		  if ( _w!=NULL ){  y = _w[t]; }
		  _v[t].delivery_iter( w, pw, t, m ,jmp,oq,tw,tpw,y,f);
		  
		}



		QUEUE_ID get_vt(int i){ return _v[i].get_t(); }
		QUEUE_INT * get_vv(int i){ return _v[i].get_v(); }
		QUEUE_INT get_vv(int i,int j){ return _v[i].get_v(j); }
		QUEUE get_v(int i){ return _v[i]; }

		QUEUE* getp_v(int i){ return &_v[i]; }
		
		QUEUE* getp_v(){ return _v; }
		

		void init_v(int i){ _v[i].init(); }

		void set_vv(int i,int j,QUEUE_INT v){ _v[i].set_v(j,v); }
		void set_vv(int i,QUEUE_INT* v){ _v[i].set_v(v); }

		void set_vt(int i,QUEUE_ID v){ _v[i].set_t(v); }

		void set_vv_all(void){
			for(int i=0 ; i< _t;i++){
				_v[i].set_v( _v[i].get_t() , _clms);
			}
		}
		  
		
		void qsort_perm(PERM * rperm,int flag){
			qsort_perm__( _v, _t, rperm, flag); 
		}

		void any_INVPERMUTE(PERM * rperm){
		  QUEUE Q;	
			ARY_INVPERMUTE (_v, rperm, Q, _t, EXIT);  // sort transactions
		}


		void INS_v(int i,QUEUE_INT v){ _v[i].push_back(v); }

		bool exist_v(){ return _v!= NULL ;}

		LONG get_v_ele(int i,QUEUE_INT v){ return _v[i].ele(v); }

		void v_rm(int u,QUEUE_ID i){ _v[u].rm(i); }

		void swap_vv(int i,int j){
			_v[i].swap_v(j);
		}
		// 要確認
		void alloc_v(){
		  //malloc2 (_v, _end, EXIT0);		
		  malloc2 (_v, _end,  exit(1) );		
		}
		void alloc_buf(){
			char *buf;
		  //malloc2 (buf,(_eles+_end+1)*_unit, EXIT0);
		  malloc2 (buf,(_eles+_end+1)*_unit, exit(1));
		  _buf = (QUEUE_INT *)buf;

		}
		void ary_INVPERMUTE_(PERM *p,QUEUE& Q){
			ARY_INVPERMUTE_(_v, p, Q, _t);
		}
			
		void ary_INVPERMUTE( PERM *invperm ,QUEUE& Q,VEC_ID num){
	    ARY_INVPERMUTE (_v, invperm, Q, num, EXIT);
		}

		// アクセッサ  		
  	//QUEUE *get_v(int i){ return _w[i]; }

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

  	void set_w(int i,int j,WEIGHT w){ _w[i][j]=w;}
  	void set_w(int i,WEIGHT *w){ _w[i]=w;}
  	void set_rw(int i,WEIGHT w){ _rw[i]=w;}
  	void set_rw(WEIGHT *w){ _rw=w;}
  	void set_rperm(PERM *rperm){ _rperm=rperm;}

  	void union_flag(int flag){ _flag |= flag;}
  	void trim_flag(int flag){ BITRM(_flag, flag);}



};



