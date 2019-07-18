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

//どっかで定義せんとだめ( lcmの時1 それ以外の時は0になる)
#ifndef TRSACT_DEFAULT_WEIGHT
 #define TRSACT_DEFAULT_WEIGHT 0  // default weight of the transaction, for missing weights in weight file
#endif

#ifndef TRSACT_MAXNUM 
 #define TRSACT_MAXNUM 20000000LL
#endif

struct LimitVal{

	//ALL
	WEIGHT _w_lb;
	WEIGHT _w_ub;

	//sspc
	double _clm_lb_; 
	double _clm_ub_; 
	QUEUE_ID _row_lb;
	QUEUE_ID _row_ub;
	double _row_lb_;
	double _row_ub_;

	// calc only
  VEC_ID _clm_lb;
  VEC_ID _clm_ub; 


	LimitVal():
		_w_lb(-WEIGHTHUGE) , _w_ub(WEIGHTHUGE) ,
		_clm_lb_(0.0),_clm_ub_(0.0),
		_row_lb(0) , _row_ub(QUEUE_IDHUGE),
		_row_lb_(0.0),_row_ub_(0.0),
		_clm_lb(0) , _clm_ub(VEC_ID_END){}

	// set lower/upper bounds if it is given by the ratio
	void setBoundsbyRate(VEC_ID rows,QUEUE_INT clms){

	  if ( _row_lb_ ) _row_lb = rows * _row_lb_;
  	if ( _row_ub_ ) _row_ub = rows * _row_ub_;
  	if ( _clm_lb_ ) _clm_lb = clms * _clm_lb_;
  	if ( _clm_ub_ ) _clm_ub = clms * _clm_ub_;
	}


	bool clmOK(WEIGHT s,QUEUE_INT k){
		return  ( RANGE( _w_lb, s, _w_ub) && RANGE (_clm_lb, k, _clm_ub) );
	}


		//if(RANGE(_w_lb, _cw[tt], _w_ub) && RANGE (_clm_lb, _clmt[tt], _clm_ub) ){

	bool rowOK(QUEUE_INT k){
		return ( RANGE (_row_lb, k, _row_ub) );
	}



};


class FILE_COUNT{

	// #rows, #column, #elements, minimum elements
  VEC_ID    _rows_org;
  QUEUE_INT _clms_org;
  size_t    _eles_org;

  WEIGHT    _total_w_org,_total_pw_org;

  FILE_COUNT_INT _clms , _rows , _eles;
  WEIGHT _total_rw, _total_cw;         // WEIGHTs for rows/columns ... reserved.

  VEC_ID _end1; //2nd-trsact position

	QUEUE_INT _clms_end; // trsact org


  // Limit 
  // lower/upper bound of #elements in a column/row. 
  // colunmn or row of out of range will be ignored
	LimitVal _limVal;
  /*
  WEIGHT   _w_lb , _w_ub ;
  VEC_ID   _clm_lb , _clm_ub; 
	QUEUE_ID _row_lb , _row_ub;
	double   _row_lb_, _row_ub_;
	double   _clm_lb_, _clm_ub_;
  */
  
  // size of each row/clmn
	VECARY <FILE_COUNT_INT> _rowt;
	VECARY <FILE_COUNT_INT> _clmt;

	VECARY <WEIGHT> _rw;
	VECARY <WEIGHT> _cw;

  PERM *_rperm, *_cperm;   // permutation (original->internal) of rows and columns

	bool _negaFLG;

	// 仮
	size_t _c_eles;
	size_t _c_clms;
	size_t _r_eles;
	size_t _r_clms;


	QUEUE_INT _weight_Scan(char *wf);
	int _file_count_T (IFILE2 &fp, char *wf);
	int _file_count   (IFILE2 &fp, char *wf);  // NOT LOAD_TPOSEの時



	PERM *_clmw_perm_sort(int flg){
    return _cw.qsort_perm(_clms_org, flg);
  }
	PERM *_clmt_perm_sort(int flg){
		return _clmt.qsort_perm(_clms_org, flg);
	}

	PERM *_rowt_perm_sort(int flg){
		return _rowt.qsort_perm( _rows_org, flg);
	}


	void _tpose(void){

	  QUEUE_INT swap_tmp = _clms_org;
		_clms_org = (QUEUE_INT)_rows_org;
		_rows_org = (VEC_ID)swap_tmp;
		
		VECARY<FILE_COUNT_INT>::swap(_clmt,_rowt);

	}
	

	public :

		FILE_COUNT(void):
			_clms_org(0) ,_rows_org(0) , 
			_total_w_org(0) , _total_pw_org(0) ,
			_eles_org(0) ,_clms(0) , _rows(0) , _eles(0),
			 _total_rw(0), _total_cw(0),
			 _rperm(NULL) , _cperm(NULL),_negaFLG(false),
			_end1(0),_c_eles(0),_c_clms(0),_r_eles(0),_r_clms(0)
			{}
	
	
		// call from trsact.cpp
		int file_count(int flg, IFILE2 &fp, IFILE2 &fp2, char *wf,char *wf2=NULL);

		// call from sgraph.cpp 
		void countSG (IFILE2 *rfp, int flag);
		void countFST(IFILE2 *rfp, int flag);

		// call from fstar.cpp
		void countFS (IFILE2 *rfp, int flag, int int_clms);

		 // 仮
		size_t c_clms(){ return _c_clms; }
		size_t c_eles(){ return _c_eles; }
		size_t r_clms(){ return _r_clms; }
		size_t r_eles(){ return _r_eles; }
		size_t c_end() { return _clms_end; }


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

  	void setLimit(LimitVal val){
	  	_limVal = val;
  	}
/*
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
*/

		bool rPermGErows(VEC_ID t){
			return _rperm[t] <= _rows_org;
 		}
 
 
    WEIGHT get_rw(VEC_ID tt){ 
    	return _rw.empty() ? 1 : _rw[tt] ;
    }

    void set_rperm(VEC_ID tt,PERM v){ _rperm[tt] = v; }

    PERM  rperm(VEC_ID t){ return _rperm[t]; }
    PERM  cperm(VEC_ID t){ return _cperm[t]; }


    PERM* get_rperm(void){ return _rperm; }
    PERM* get_cperm(void){ return _cperm; }


		// _headとstrIDを使うなら再考
		// (イマイチなにしたいのかわからん)
    VEC_ID adjust_ClmSep(VEC_ID sep);
    VEC_ID adjust_RowSep(VEC_ID sep);

		//void makePerm(char *pfname,int tflag,int tflag2);

		void makePerm(int tflag,int tflag2);

		//void initCperm(char *pfname,int tflag,int tflag2);
		void initCperm(int tflag,int tflag2);
		void initRperm(int tflag);

		//以下 using vec.cpp
		int  sumRow(size_t s,size_t e){ return _rowt.sum(s,e); }

		bool CheckRperm(VEC_ID t){ 
			return ( ( _rperm[t] < _rows_org ) && (  _rperm[t] > 0 ) );
		}
		

};


//	_w_lb(-WEIGHTHUGE) , _w_ub(WEIGHTHUGE) ,
//	_clm_lb(0) , _clm_ub(VEC_ID_END), 
//	_row_lb(0) , _row_ub(QUEUE_IDHUGE),
//	_clm_lb_(0.0),_clm_ub_(0.0),
//	_row_lb_(0.0),_row_ub_(0.0),

// int _flag;
//  FILE_COUNT_INT _row_btm , _clm_btm;
// FILE_COUNT_INT _clm_end , _row_end; // <=たぶんこれいらん
// FILE_COUNT_INT _rw_end, _cw_end;
//  FILE_COUNT_INT _row_min , _row_max;
//  FILE_COUNT_INT _clm_min , _clm_max;  // maximum/minimum size of column
// 	QUEUE_INT _clms_max; // trsact org

//			 _row_min(0) , _row_max(0),
//			_clm_min(0) , _clm_max(0) ,
// _clm_end(0) , _row_end(0),_rw_end(0) , _cw_end(0) 
// _row_btm(0) , _clm_btm(0) ,
//size_t c_max(){ return _clms_max; }
//		bool RangeChecnkC(VEC_ID tt,WEIGHT w_lb ,WEIGHT w_ub , VEC_ID clm_lb ,VEC_ID clm_ub){
//			return  ( RANGE(w_lb, _cw[tt], w_ub) && RANGE (clm_lb, _clmt[tt], clm_ub) );
//		}
//		bool RangeChecnkR(VEC_ID tt,QUEUE_ID row_lb ,QUEUE_ID row_ub ){
//			return  RANGE(row_lb, _rowt[tt], row_ub);
//		}
//		bool RangeChecnkC(VEC_ID tt){
//			return  ( RANGE(_w_lb, _cw[tt], _w_ub) && RANGE (_clm_lb, _clmt[tt], _clm_ub) );
//		}
//		bool RangeChecnkR(VEC_ID tt){
//			return  RANGE(_row_lb, _rowt[tt], _row_ub);
//		}
//		void initCperm(VEC_ID ttt , PERM *p ,QUEUE_INT c_end , bool flag);
//		void initRperm(PERM *p , size_t base_clm, size_t base_ele);
//
/* CheckRperm(VEC_ID t){ 		org
			if( ( _rperm[t] < _rows_org ) && (  _rperm[t] > 0 ) ){
				return true;
			} 
			return false;
	}
	PERM *_roww_perm_sort(int flg){
		if(_rw.empty()){ return _rowt.qsort_perm(_rows_org, flg); }
		else           { return _rw.qsort_perm( _rows_org, flg);  }
	}
*/
		/*
		void countFS( 
			FILE2 *rfp, int flag, int skip_rows,
			int int_rows, int skip_clms, int int_clms, 
	 		FILE_COUNT_INT row_limit);
		*/
	




