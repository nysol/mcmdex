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
  WEIGHT _w_lb , _w_ub ;
  VEC_ID _clm_lb , _clm_ub; 
	QUEUE_ID _row_lb, _row_ub;
  
  
  FILE_COUNT_INT *_rowt, *_clmt;   // size of each row/clmn

  WEIGHT _total_rw, _total_cw;  // WEIGHTs for rows/columns ... reserved.
  WEIGHT *_rw, *_cw;

  FILE_COUNT_INT _rw_end, _cw_end;

  PERM *_rperm, *_cperm;   // permutation (original->internal) of rows and columns
	bool _negaFLG;

	// 仮
	size_t _c_eles;
	size_t _c_clms;
	size_t _r_eles;
	size_t _r_clms;


	QUEUE_INT weight_Scan(char *wf){

	  FILE2 wfp;
		WEIGHT w;
		QUEUE_INT kk=0;

		wfp.open(wf, "r");
		#ifdef WEIGHT_DOUBLE
			kk = wfp.ARY_Scan_DBL(1);
		#else
			kk = wfp.ARY_Scan_INT(1);
		#endif

	  kk += _rows_org;
	  realloc2 (_rw, kk+1, exit(1));
		wfp.reset();
	  wfp.ARY_Read(_rw, kk);
	  size_t i;

		ARY_MIN (w, i, _rw, 0, kk);
		
//		if ( w<0 ) _flag2 |= TRSACT_NEGATIVE;どこかでセットする
		if ( w<0 ) { _negaFLG = true;}
		wfp.close();
		return kk;
	
	}


	/*****************************************/
	/* scan file "fp" with weight file wfp and count #items, #transactions in the file. */
	/*   count weight only if wfp!=NULL                                      */
	/* T->rows_org, clms_org, eles_org := #items, #transactions, #all items  */
	/*   ignore the transactions of size not in range T->clm_lb - clm_ub     */ 
	/* T->total_w, total_pw := sum of (positive) weights of transactions     */
	/* C->clmt[i],C->cw[i] := the number/(sum of weights) of transactions including i  */
	/****************************************/
	// LOAD_TPOSEの時
	int _file_count_T (FILE2 *fp,char *wf){

	  QUEUE_INT i, item, kk=0, k, jump_end=0;
	  WEIGHT w, s;
	  VEC_ID *jump=NULL;

  	LONG jj;

	  if ( wf ){ kk = weight_Scan(wf); }

	  do {

  	  s=0; k=0;

    	w = wf? (_rows_org<kk? _rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;

    	do {

      	jj = fp->read_int();
      	item = (QUEUE_INT)jj;

      	if ( (FILE_err&4)==0 && jj<TRSACT_MAXNUM && jj>=0 ){

       		ENMAX (_clms_org, item+1);  // update #items
        	//reallocx (jump, jump_end, k, 0, goto ERR);
        	jump = reallocx(jump, &jump_end, k, 0);


      		jump[k] = item;
        	k++;
        	s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

          // count/weight-sum for the transpose mode
        	//reallocx (_clmt, _clm_end, item, 0, goto ERR);
        	_clmt = reallocx(_clmt, &_clm_end, item, 0);

        	_clmt[item]++;
	      }

  		} while ( (FILE_err&3)==0);

       // count/weight-sum for the transpose mode
			//reallocx (_rowt, _row_end, _rows_org, 0, goto ERR);
			_rowt = reallocx (_rowt, &_row_end, _rows_org, 0);

    	_rowt[_rows_org] = k;

			// LOAD_TPOSEの時
			//reallocx (_cw, _cw_end, _rows_org, 0, goto ERR);
			_cw = reallocx<WEIGHT>(_cw, &_cw_end, _rows_org, 0);

      _cw[_rows_org] = s;    // sum up positive weights

			if ( k==0 && FILE_err&2 ) break;
			_rows_org++;  // increase #transaction

    	
			if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

			_eles_org += k;

			// LOAD_TPOSEの時はこの条件
	    if ( !RANGE( _w_lb, s, _w_ub) || !RANGE (_clm_lb, k, _clm_ub)  ){
      	FLOOP (i, 0, k) _clmt[jump[i]]--; 
      }


		} while ( (FILE_err&2)==0);

		free2 (jump);
    // swap the variables in transpose mode
  	if ( _rw == NULL ){
  		_total_w_org = _total_pw_org = _rows_org; 
  		return 0; 
  	} 
		_clm_btm = MIN(kk, _rows_org);
		//reallocx (_rw, kk, _rows_org, TRSACT_DEFAULT_WEIGHT, goto ERR);
		_rw = reallocx<WEIGHT>(_rw, &kk, _rows_org, TRSACT_DEFAULT_WEIGHT);

		FLOOP (k, 0, _rows_org){
  	  _total_w_org += _rw[k];
    	_total_pw_org += MAX(_rw[k],0);
	  }
	  return 0;
  
	  ERR:;
		//wfp.close();
		mfree (jump);
  	return 1;
	}

	// NOT LOAD_TPOSEの時
	//int file_count (FILE2 *fp, QUEUE_ID row_lb, QUEUE_ID row_ub ,char *wf){

	int _file_count (FILE2 *fp, char *wf){
	
	  QUEUE_INT i, item, kk=0, k, jump_end=0;
	  WEIGHT w, s;
	  VEC_ID *jump=NULL;

  	LONG jj;
	  if ( wf ){ kk = weight_Scan(wf); }

	  do {

  	  s=0; k=0;

    	w = wf? (_rows_org<kk? _rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;

    	do {

      	jj = fp->read_int();
      	item = (QUEUE_INT)jj;

      	if ( (FILE_err&4)==0 && jj<TRSACT_MAXNUM && jj>=0 ){

       		ENMAX (_clms_org, item+1);  // update #items
        	// reallocx (jump, jump_end, k, 0, goto ERR);
        	jump = reallocx(jump, &jump_end, k, 0);

      		jump[k] = item;
        	k++;
        	s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

          // count/weight-sum for the transpose mode
        	//reallocx (_clmt, _clm_end, item, 0, goto ERR);
        	_clmt = reallocx (_clmt, &_clm_end, item, 0);
        	_clmt[item]++;

					// NOT TPOSE
          //reallocx (_cw, _cw_end, item, 0, goto ERR);
          _cw = reallocx<WEIGHT>(_cw, &_cw_end, item, 0);

          _cw[item] += MAX(w,0);    // sum up positive weights
	      }

  		} while ( (FILE_err&3)==0);

       // count/weight-sum for the transpose mode
			//reallocx (_rowt, _row_end, _rows_org, 0, goto ERR);
			_rowt = reallocx (_rowt, &_row_end, _rows_org, 0);
    	_rowt[_rows_org] = k;

			if ( k==0 && FILE_err&2 ) break;

			_rows_org++;  // increase #transaction
    
			if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

			_eles_org += k;

			// NOT LOAD_TPOSEの時はこの条件
	    if( !RANGE (_row_lb, k, _row_ub) ){
      	FLOOP (i, 0, k) _clmt[jump[i]]--; 
      }


		} while ( (FILE_err&2)==0);

		free2 (jump);
		
    // swap the variables in transpose mode
  	if ( _rw == NULL ){
  		_total_w_org = _total_pw_org = _rows_org; 
  		return 0; 
  	} 
		_clm_btm = MIN(kk, _rows_org);
		//reallocx (_rw, kk, _rows_org, TRSACT_DEFAULT_WEIGHT, goto ERR);
		_rw = reallocx<WEIGHT>(_rw, &kk, _rows_org, TRSACT_DEFAULT_WEIGHT);

		FLOOP (k, 0, _rows_org){
  	  _total_w_org += _rw[k];
    	_total_pw_org += MAX(_rw[k],0);
	  }


	  return 0;
  
	  ERR:;
		//wfp.close();
		mfree (jump);
  	return 1;
	}

	public :
	
		FILE_COUNT(void):
			_clms_org(0) ,_rows_org(0) , _total_w_org(0) , _total_pw_org(0) ,_eles_org(0) ,
			_clms(0) , _rows(0) , _eles(0), _clm_end(0) , _row_end(0),
			_row_btm(0) , _clm_btm(0) , _row_min(0) , _row_max(0),
			_clm_min(0) , _clm_max(0) , _rowt(NULL) , _clmt(NULL),
			_total_rw(0), _total_cw(0), _rw(NULL) , _cw(NULL),
			_rw_end(0) , _cw_end(0) , _rperm(NULL) , _cperm(NULL),_negaFLG(false),
			_w_lb(0) , _w_ub(0) ,_clm_lb(0) , _clm_ub(0), _row_lb(0), _row_ub(0),_end1(0),
			_c_eles(0),_c_clms(0),_r_eles(0),_r_clms(0)
			{}
	
		 // 仮
		size_t c_clms(){ return _c_clms; }
		size_t c_eles(){ return _c_eles; }
		size_t r_clms(){ return _r_clms; }
		size_t r_eles(){ return _r_eles; }
		
		
		~FILE_COUNT(void){
			mfree (_rw, _cw, _clmt, _rowt);
			//rowtは他でセットされる
		}

  	void setLimit(
  		WEIGHT w_lb , WEIGHT w_ub ,
  		VEC_ID clm_lb , VEC_ID clm_ub, 
  		QUEUE_ID row_lb, QUEUE_ID row_ub )
  	{
 		 	_w_lb = w_lb;
 		 	_w_ub = w_ub;
	  	_clm_lb = clm_lb; 
	  	_clm_ub = clm_ub; 
			_row_lb = row_lb;
			_row_ub = row_ub;
  	};
		
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

		FILE_COUNT_INT * getp_rowt(){ return _rowt; }// たぶん NULLにした方がいい


		bool rPermGErows(VEC_ID t){
			return _rperm[t] <= _rows_org;
 		}
 
 
    size_t get_rw(VEC_ID tt){ 
    	return _rw ? _rw[tt] :1 ;
    }

    void set_cperm(VEC_ID tt,PERM v){ _cperm[tt] = v; }
    void set_rperm(VEC_ID tt,PERM v){ _rperm[tt] = v; }

    PERM  get_rperm(VEC_ID t){ return _rperm[t]; }
    PERM  get_cperm(VEC_ID t){ return _cperm[t]; }
    PERM* get_rperm(void){ return _rperm; }
    PERM* get_cperm(void){ return _cperm; }


		PERM *clmw_perm_sort(int flg){
		
    	return qsort_perm_<WEIGHT> (_cw, _clms_org, flg);
    }
    PERM *clmt_perm_sort(int flg){
      return qsort_perm_<FILE_COUNT_INT> (_clmt, _clms_org, flg);
    }
    PERM *roww_perm_sort(int flg){
    	if(_rw){
	      return qsort_perm_<WEIGHT> (_rw, _rows_org, flg);
	    }
	    else{
	      return qsort_perm_<FILE_COUNT_INT> (_rowt, _rows_org, flg);
	    }
	    
    }
    PERM *rowt_perm_sort(int flg){
      return qsort_perm_<FILE_COUNT_INT> (_rowt, _rows_org, flg);
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

/*
		void cpermFILL(){

			malloc2 (_cperm, _clms_org+1, exit(1));
			for(size_t i =0 ;i<_clms_org;i++){ 
				_cperm[i] = _clms_org+1; 
			}
		}
*/
		void initCperm(VEC_ID ttt , PERM *p ,QUEUE_INT c_end , bool flag){

			_c_eles=0;
			_c_clms=0;

			malloc2 (_cperm, _clms_org+1, exit(1));
			for(size_t i =0 ;i<_clms_org;i++){ 
				_cperm[i] = _clms_org+1; 
			}

			VEC_ID tt =0 ;
		  for(size_t t=0; t < ttt; t++){
		    tt = p? p[t]: t;
		    if ( tt >= _clms_org ) continue;
		    
		    if ( RangeChecnkC(tt) ){
  		    _c_eles += _clmt[tt];
		    	_cperm[tt] = flag ? t : _c_clms++ ;
		    }else {
    	 		_cperm[tt] = c_end ;
    		}
		  }
		  return ;
		}

		/*
		void rpermFILL(){

			malloc2 (_rperm, _rows_org, exit(1));
			//for(size_t i =0 ;i<_clms_org;i++){ 
			//	_rperm[i] = _clms_org+1; 
			//}
		}
		*/
		
		void initRperm(PERM *p , size_t base_clm, size_t base_ele){

			_r_eles = base_ele;
			_r_clms = base_clm;

			malloc2 (_rperm, _rows_org, exit(1));
		  // compute #elements according to rowt, and set rperm
			VEC_ID tt=0;
			for( VEC_ID t=0 ; t<_rows_org ; t++){
				tt = p? p[t]: t;
		    if ( RangeChecnkR(tt)){
			    _rperm[tt] = _r_clms++;
    		  _r_eles += _rowt[tt];
    		}
    		else{
					_rperm[tt] = _rows_org+1;
    		}
			}
		}

		bool CheckRperm(VEC_ID t){
			if( _rperm[t] < _rows_org ){
				if( _rperm[t] > 0 ){
					return true;
				}
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
		
		FILE_COUNT_INT * swap_pnt = _clmt;
		_clmt = _rowt;
		_rowt = swap_pnt;

	}
	
	int file_count (int flg, FILE2 *fp, FILE2 *fp2, char *wf){

		if(flg){ // TPOSE
			if( _file_count_T ( fp, wf) ) { return 1; }	
		  _end1 = _rows_org;
		  if(fp2){
				if( _file_count_T ( fp2, NULL) ) { return 1; }
			}

		} 
		else{
			if( _file_count( fp, wf) ){ return 1;	}
		  _end1 = _rows_org;
		  if(fp2){
				if( _file_count(fp2, NULL) ) { return 1;	}
			}
		}
		return 0;
	}

	void count ( 
		FILE2 *rfp,
		int flag, int skip_rows,
		int int_rows, int skip_clms, int int_clms, 
	 	FILE_COUNT_INT row_limit)
	{
		
	  FILE_COUNT_INT k=0, j, x, y, t=0;

  	// flags for using rowt, and clmt, that counts elements in each row/column
  	int fr = flag&FILE_COUNT_ROWT, fc = flag&FILE_COUNT_CLMT; 

		// fe,ft: flag for ele mode, and transposition
	  int fe = flag&LOAD_ELE, ft = flag&LOAD_TPOSE;  

  	//_flag = flag;

	  FLOOP (j, 0, skip_rows) rfp->read_until_newline ();

	  if ( flag & (FILE_COUNT_NUM+FILE_COUNT_GRAPHNUM) ){

  	  _clms = (FILE_COUNT_INT) rfp->read_int ();
    	_rows = (flag & FILE_COUNT_NUM)? (FILE_COUNT_INT) rfp->read_int(): _clms;
    	_eles = (FILE_COUNT_INT) rfp->read_int();

	    if ( !(flag & (FILE_COUNT_ROWT + FILE_COUNT_CLMT)) ) return ;
  	  rfp->read_until_newline ();
		}

	  do {
	    if ( fe ){
			
				FLOOP (j, 0, skip_clms){ 
					rfp->read_double (); 
					if ( FILE_err&3 ) goto ROW_END; 
				}
			
				x = (FILE_COUNT_INT) rfp->read_int (); //printf ("%d\n", FILE_err);
				if ( FILE_err&3 ) goto ROW_END;
			
      	y = (FILE_COUNT_INT) rfp->read_int (); 
      	if ( FILE_err&4 ) goto ROW_END;

      	rfp->read_until_newline ();
    	}
    	else 
    	{
      	if ( k==0 ) {
      		FLOOP (j, 0, skip_clms){ 
      			rfp->read_double (); 
      			if (FILE_err&3) goto ROW_END; 
      		}
      	}
				x = t;
      	y = (FILE_COUNT_INT)rfp->read_int (); 
      	if (FILE_err&4 ) goto ROW_END;
      	FLOOP (j, 0, int_clms){ 
      		rfp->read_double (); 
      		if (FILE_err&3 ) break; 
      	}
				k++;
    	}
    	
	    if ( ft ){
	    	SWAP_<FILE_COUNT_INT>(&x, &y);
	    }

	    if ( y >= _clms ){
  	    _clms = y+1;
    	  if ( fc ) {
    	  	//reallocx (_clmt, _clm_end, _clms, 0, goto END);
    	  	_clmt = reallocx(_clmt, &_clm_end, _clms, 0);
    	  }
			}
			
			if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && x >= _clms ){
      	_clms = x+1;
				if ( fc ) { 
					//reallocx (_clmt, _clm_end, _clms, 0, goto END);
					_clmt = reallocx (_clmt, &_clm_end, _clms, 0);
				}
	    }
 
 			if ( x >= _rows ){
      	_rows = x+1;
				if ( fr ) { 
					//reallocx (_rowt, _row_end, _rows, 0, goto END);
					_rowt = reallocx (_rowt, &_row_end, _rows, 0);
				}
			}
		
			if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && y >= _rows ){ // for undirected edge version
      	_rows = y+1;
	      if ( fr ) { 
	      	//reallocx (_rowt, _row_end, _rows, 0, goto END);
	      	_rowt = reallocx (_rowt, &_row_end, _rows, 0);
	      }
    }
    
    if ( x < _clm_btm || _eles == 0 ) {  _clm_btm = x; }
    if ( y < _row_btm || _eles == 0 ) {  _row_btm = y; }
    if ( fc ) { _clmt[y]++; }
    if ( fr ){ 
    	_rowt[x]++; 
    	if ( flag&LOAD_EDGE && x != y ){ _rowt[y]++; }
    }	
    
    _eles++;

    ROW_END:;

    if ( !fe && (FILE_err&1) ){

      t++;

      if ( flag&(LOAD_RC_SAME+LOAD_EDGE) ){
        ENMAX (_clms, t); ENMAX (_rows, t);
      } 
      else if ( ft ) {	
      	_clms = t;
      } 
      else { 
      	_rows = t;
      }

      ENMAX (_clm_max, k);
      ENMIN (_clm_min, k);

      FLOOP (j, 0, int_rows){
      	rfp->read_until_newline ();
      }
      if ( row_limit>0 && t>=row_limit ) { break; }
    } 
    else if ( row_limit > 0 && _eles>=row_limit ) {
    	break;
    }

  } while ( (FILE_err&2)==0 );

  if ( fc ){ 
  	//reallocx (_clmt, _clm_end, _clms, 0, goto END);
  	_clmt = reallocx (_clmt, &_clm_end, _clms, 0);
  }
  if ( fr ){
    //reallocx (_rowt, _row_end, _rows, 0, goto END);
    _rowt = reallocx (_rowt, &_row_end, _rows, 0);
    ARY_MAX (_row_max, k, _rowt, 0, _rows);
    ARY_MIN (_row_min, k, _rowt, 0, _rows);
  }
  if ( fe && _clmt ){
    ARY_MAX (_clm_max, k, _clmt, 0, _clms);
    ARY_MIN (_clm_min, k, _clmt, 0, _clms);
  }
  END:;
  // if ( ERROR_MES ) mfree (C.rowt, C.clmt);
  return ;
}

};








