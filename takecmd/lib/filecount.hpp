/* library for standard macros and functions 
 by Takeaki Uno 2/22/2002   e-mail: uno@nii.jp
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */
#pragma once

#include "stdlib2.hpp"
#include "file2.hpp"
#include "queue.hpp"

#define FILE_COUNT_INT VEC_ID
#define FILE_COUNT_INTF VEC_IDF

//どっかで定義せんとだめ
#ifndef TRSACT_DEFAULT_WEIGHT
 #define TRSACT_DEFAULT_WEIGHT 0  // default weight of the transaction, for missing weights in weight file
#endif

#ifndef TRSACT_MAXNUM 
 #define TRSACT_MAXNUM 20000000LL
#endif

class FILE_COUNT{

  //int _flag;

   // #rows, #column, #elements, minimum elements
  QUEUE_INT _clms_org;
  VEC_ID    _rows_org;
  size_t    _eles_org;

  WEIGHT _total_w_org,_total_pw_org;
  FILE_COUNT_INT _clms , _rows , _eles;
  FILE_COUNT_INT _row_btm , _clm_btm;
  FILE_COUNT_INT _row_min , _row_max;
  FILE_COUNT_INT _clm_min , _clm_max;  // maximum/minimum size of column
  FILE_COUNT_INT _clm_end , _row_end;
  VEC_ID _end1; //2nd-trsact position

  // Limit 
  // lower/upper bound of #elements in a column/row. 
  // colunmn or row of out of range will be ignored
  WEIGHT   _w_lb , _w_ub ;
  VEC_ID   _clm_lb , _clm_ub; 
	QUEUE_ID _row_lb , _row_ub;
	double   _row_lb_, _row_ub_;
	double   _clm_lb_, _clm_ub_;
  
  

  WEIGHT _total_rw, _total_cw;  // WEIGHTs for rows/columns ... reserved.

  // size of each row/clmn
	VECARY <FILE_COUNT_INT> _rowt;
	VECARY <FILE_COUNT_INT> _clmt;

  //WEIGHT *_rw; 
  //WEIGHT *_cw;
	VECARY <WEIGHT> _rw;
	VECARY <WEIGHT> _cw;


  FILE_COUNT_INT _rw_end, _cw_end;

  PERM *_rperm, *_cperm;   // permutation (original->internal) of rows and columns
	bool _negaFLG;

	// 仮
	size_t _c_eles;
	size_t _c_clms;
	size_t _r_eles;
	size_t _r_clms;


	QUEUE_INT _weight_Scan(char *wf);
	int _file_count_T (FILE2 *fp,char *wf);
	int _file_count   (FILE2 *fp, char *wf);  // NOT LOAD_TPOSEの時




	public :
	
		FILE_COUNT(void):
			_clms_org(0) ,_rows_org(0) , _total_w_org(0) , _total_pw_org(0) ,_eles_org(0) ,
			_clms(0) , _rows(0) , _eles(0), _clm_end(0) , _row_end(0),
			_row_btm(0) , _clm_btm(0) , _row_min(0) , _row_max(0),
			_clm_min(0) , _clm_max(0) , _total_rw(0), _total_cw(0),
			_rw_end(0) , _cw_end(0) , _rperm(NULL) , _cperm(NULL),_negaFLG(false),
			_w_lb(-WEIGHTHUGE) , _w_ub(WEIGHTHUGE) ,
			_clm_lb(0) , _clm_ub(VEC_ID_END), 
			_row_lb(0) , _row_ub(QUEUE_IDHUGE),
		  _clm_lb_(0.0),_clm_ub_(0.0),_row_lb_(0.0),_row_ub_(0.0),
			_end1(0),_c_eles(0),_c_clms(0),_r_eles(0),_r_clms(0)
			{}
	
		int file_count (int flg, FILE2 *fp, FILE2 *fp2, char *wf);

		void count( 
			FILE2 *rfp, int flag, int skip_rows,
			int int_rows, int skip_clms, int int_clms, 
	 		FILE_COUNT_INT row_limit);

	
		 // 仮
		size_t c_clms(){ return _c_clms; }
		size_t c_eles(){ return _c_eles; }
		size_t r_clms(){ return _r_clms; }
		size_t r_eles(){ return _r_eles; }

		int  sumRow(size_t s,size_t e){ return _rowt.sum(s,e); }

		bool rowEmpty(){ return _rowt.empty(); }

		bool existNegative(){ return _negaFLG;}
		
		WEIGHT get_total_w_org(){ return _total_w_org;}
		WEIGHT get_total_pw_org(){ return _total_pw_org;}

		QUEUE_INT clms(void){return _clms_org; }
		VEC_ID    rows(void){return _rows_org; }
		size_t    eles(void){return _eles_org; }

		VEC_ID    end1(void){return _end1; }

		FILE_COUNT_INT get_clms(void){return _clms; }
		FILE_COUNT_INT get_rows(void){return _rows; }
		FILE_COUNT_INT get_eles(void){return _eles; }

		size_t get_clmt(VEC_ID tt){ return _clmt[tt]; }
    size_t get_rowt(VEC_ID tt){ return _rowt[tt]; }

		
		
		~FILE_COUNT(void){
			//mfree (_rw, _cw, _clmt, _rowt);
			//rowtは他でセットされる
		}

  	void setLimit(
  		WEIGHT w_lb , WEIGHT w_ub ,
  		double clm_lb_  = 0.0 , double clm_ub_  = 0.0, 
  		QUEUE_ID row_lb = 0   , QUEUE_ID row_ub = QUEUE_IDHUGE,
  		double row_lb_  = 0.0 , double row_ub_  = 0.0
  	){
 		 	_w_lb    = w_lb;
 		 	_w_ub    = w_ub;
	  	_clm_lb_ = clm_lb_; 
	  	_clm_ub_ = clm_ub_; 
			_row_lb  = row_lb;
			_row_ub  = row_ub;
			_row_lb_ = row_lb_;
			_row_ub_ = row_ub_;

  	};

		void setBoundsbyRate(){

		  // set lower/upper bounds if it is given by the ratio
	  	if ( _row_lb_ ) _row_lb = _rows_org * _row_lb_;
  		if ( _row_ub_ ) _row_ub = _rows_org * _row_ub_;
  		if ( _clm_lb_ ) _clm_lb = _clms_org * _clm_lb_;
  		if ( _clm_ub_ ) _clm_ub = _clms_org * _clm_ub_;
		
		}

		bool rPermGErows(VEC_ID t){
			return _rperm[t] <= _rows_org;
 		}
 
 
    size_t get_rw(VEC_ID tt){ 
    	return _rw.empty() ? 1 : _rw[tt] ;
    }

    void set_cperm(VEC_ID tt,PERM v){ _cperm[tt] = v; }
    void set_rperm(VEC_ID tt,PERM v){ _rperm[tt] = v; }

    PERM  get_rperm(VEC_ID t){ return _rperm[t]; }
    PERM  get_cperm(VEC_ID t){ return _cperm[t]; }
    PERM* get_rperm(void){ return _rperm; }
    PERM* get_cperm(void){ return _cperm; }


		PERM *clmw_perm_sort(int flg){
    	return _cw.qsort_perm(_clms_org, flg);
    }
    PERM *clmt_perm_sort(int flg){
      return _clmt.qsort_perm(_clms_org, flg);
    }

    PERM *roww_perm_sort(int flg){
    	if(_rw.empty()){
	      return _rowt.qsort_perm(_rows_org, flg);
	    }
	    else{
	      return _rw.qsort_perm( _rows_org, flg);
	    }
    }

    PERM *rowt_perm_sort(int flg){
      return _rowt.qsort_perm( _rows_org, flg);
    }

		// _headとstrIDを使うなら再考
    VEC_ID adjust_sep(VEC_ID sep,VEC_ID end,int flag){ 
    	size_t tt=0;
    	if(flag){
    		for(size_t t=0;t<_clms_org;t++){
			    if ( _cperm[t] <= _clms_org ){
			      if ( t == end && sep==0 ) sep = tt;
      			if ( t == sep && sep>0 )  sep = tt;
			    	tt++;
			    }
    		}
    	}
    	else{
    		for(size_t t=0;t<_rows_org;t++){
			    if ( _rperm[t] <= _rows_org ){
			      if ( t == end && sep==0 ) sep = tt;
      			if ( t == sep && sep>0 )  sep = tt;
			    	tt++;
			    }
    		}
    	}
    	return sep;
    }

		void initCperm(VEC_ID ttt , PERM *p ,QUEUE_INT c_end , bool flag);
		void initRperm(PERM *p , size_t base_clm, size_t base_ele);


		bool CheckRperm(VEC_ID t){
		
			if( ( _rperm[t] < _rows_org ) && (  _rperm[t] > 0 ) ){
				return true;
			} 
			return false;
		}

		
		bool RangeChecnkC(VEC_ID tt,WEIGHT w_lb ,WEIGHT w_ub , VEC_ID clm_lb ,VEC_ID clm_ub){
			return  ( RANGE(w_lb, _cw[tt], w_ub) && RANGE (clm_lb, _clmt[tt], clm_ub) );
		}
		bool RangeChecnkC(VEC_ID tt){
			return  ( RANGE(_w_lb, _cw[tt], _w_ub) && RANGE (_clm_lb, _clmt[tt], _clm_ub) );
		}

		bool RangeChecnkR(VEC_ID tt,QUEUE_ID row_lb ,QUEUE_ID row_ub ){
			return  RANGE(row_lb, _rowt[tt], row_ub);
		}
		bool RangeChecnkR(VEC_ID tt){
			return  RANGE(_row_lb, _rowt[tt], _row_ub);
		}
	

	void tpose(void){
	  QUEUE_INT swap_tmp = _clms_org;
		_clms_org = (QUEUE_INT)_rows_org;
		_rows_org = (VEC_ID)swap_tmp;
		
		VECARY<FILE_COUNT_INT>::swap(_clmt,_rowt);

	}

};








