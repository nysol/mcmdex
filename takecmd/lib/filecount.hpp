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

/*
typedef struct {
  int flag;
  FILE_COUNT_INT clms, rows, eles, clm_end, row_end, row_btm, clm_btm; // #rows, #column, #elements, minimum elements
  FILE_COUNT_INT row_min, row_max, clm_min, clm_max;  // maximum/minimum size of column
  FILE_COUNT_INT *rowt, *clmt;   // size of each row/clmn
  WEIGHT total_rw, total_cw, *rw, *cw;  // WEIGHTs for rows/columns ... reserved.
  FILE_COUNT_INT rw_end, cw_end;
  PERM *rperm, *cperm;   // permutation (original->internal) of rows and columns
} FILE_COUNT;

extern FILE_COUNT INIT_FILE_COUNT;

INIT_FILE_COUNT = {0,0,0,0,0,0,0,0,0,0,0,0,NULL,NULL,0,0,NULL,NULL,0,0,NULL,NULL};
*/

class FILE_COUNT{

  int flag;

   // #rows, #column, #elements, minimum elements
  QUEUE_INT _clms_org;
  VEC_ID    _rows_org;
  size_t    _eles_org;
  WEIGHT _total_w_org,_total_pw_org;
  FILE_COUNT_INT _clms , _rows , _eles;
  FILE_COUNT_INT _clm_end , _row_end;
  FILE_COUNT_INT _row_btm , _clm_btm;
  FILE_COUNT_INT _row_min , _row_max;
  FILE_COUNT_INT _clm_min , _clm_max;  // maximum/minimum size of column
  
  
  FILE_COUNT_INT *_rowt, *_clmt;   // size of each row/clmn

  WEIGHT _total_rw, _total_cw;  // WEIGHTs for rows/columns ... reserved.
  WEIGHT *_rw, *_cw;

  FILE_COUNT_INT _rw_end, _cw_end;

  PERM *_rperm, *_cperm;   // permutation (original->internal) of rows and columns

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
		
		wfp.close();
		return kk;
	
	}

	public :
	
		FILE_COUNT(void):
			_clms_org(0) ,_rows_org(0) , _total_w_org(0) , _total_pw_org(0) ,_eles_org(0) ,
			_clms(0) , _rows(0) , _eles(0), _clm_end(0) , _row_end(0),
			_row_btm(0) , _clm_btm(0) , _row_min(0) , _row_max(0),
			_clm_min(0) , _clm_max(0) , _rowt(NULL) , _clmt(NULL),
			_total_rw(0), _total_cw(0), _rw(NULL) , _cw(NULL),
			_rw_end(0) , _cw_end(0) , _rperm(NULL) , _cperm(NULL){}
	

		~FILE_COUNT(void){
			mfree (_rw, _cw, _clmt, _rowt);
			//rowtは他でセットされる
		}
		
		
		
		WEIGHT get_total_w_org(){ return _total_w_org;}
		WEIGHT get_total_pw_org(){ return _total_pw_org;}

		QUEUE_INT clms(void){return _clms_org; }
		VEC_ID    rows(void){return _rows_org; }
		size_t    eles(void){return _eles_org; }

		FILE_COUNT_INT get_clms(void){return _clms; }
		FILE_COUNT_INT get_rows(void){return _rows; }
		FILE_COUNT_INT get_eles(void){return _eles; }

		size_t get_clmt(VEC_ID tt){ return _clmt[tt]; }
    size_t get_rowt(VEC_ID tt){ return _rowt[tt]; }

		FILE_COUNT_INT * getp_rowt(){ return _rowt; }// たぶん NULLにした方がいい


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
		void permPrn(){
				std::cerr << "cperm" << std::endl;
			for(size_t i =0 ;i<_clms_org;i++){ 
				std::cerr << i << ":"<< _cperm[i] << std::endl;
			}

				std::cerr << "rperm" << std::endl;
			for(size_t i =0 ;i<_rows_org;i++){ 
				std::cerr << i << ":"<< _rperm[i] << std::endl;
			}
		}
	*/	
		
		


		void cpermFILL(){

			malloc2 (_cperm, _clms_org+1, exit(1));
			for(size_t i =0 ;i<_clms_org;i++){ 
				_cperm[i] = _clms_org+1; 
			}
		}

		void rpermFILL(){

			malloc2 (_rperm, _rows_org, exit(1));
			//for(size_t i =0 ;i<_clms_org;i++){ 
			//	_rperm[i] = _clms_org+1; 
			//}
		}
		
		
		bool RangeChecnkC(VEC_ID tt,WEIGHT w_lb ,WEIGHT w_ub , VEC_ID clm_lb ,VEC_ID clm_ub){

			return  ( RANGE(w_lb, _cw[tt], w_ub) && RANGE (clm_lb, _clmt[tt], clm_ub) );
		}
		bool RangeChecnkR(VEC_ID tt,QUEUE_ID row_lb ,QUEUE_ID row_ub ){
			return  RANGE(row_lb, _rowt[tt], row_ub);
		}
	

	void tpose(void){
	  QUEUE_INT swap_tmp = _clms_org;
		_clms_org = (QUEUE_INT)_rows_org;
		_rows_org = (VEC_ID)swap_tmp;
		
    SWAP_PNT (_clmt, _rowt);
    //SWAP_<FILE_COUNT_INT*> (C->clmt, C->rowt);
	
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
	int file_count (FILE2 *fp, WEIGHT w_lb ,WEIGHT w_ub , VEC_ID clm_lb ,VEC_ID clm_ub ,char *wf){

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
        	reallocx (jump, jump_end, k, 0, goto ERR);
      		jump[k] = item;
        	k++;
        	s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

          // count/weight-sum for the transpose mode
        	reallocx (_clmt, _clm_end, item, 0, goto ERR);
        	_clmt[item]++;
	      }

  		} while ( (FILE_err&3)==0);

       // count/weight-sum for the transpose mode
			reallocx (_rowt, _row_end, _rows_org, 0, goto ERR);
    	_rowt[_rows_org] = k;

			// LOAD_TPOSEの時
			reallocx (_cw, _cw_end, _rows_org, 0, goto ERR);
      _cw[_rows_org] = s;    // sum up positive weights

			if ( k==0 && FILE_err&2 ) break;
			_rows_org++;  // increase #transaction

    	
			if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

			//if ( k==0 ){
			//	_str_num++;  // increase #streams if empty transaction is read
			//}
			//if ( k!=0 ){
			//	_eles_org += k;
			//	// LOAD_TPOSEの時はこの条件
	    //  if ( !RANGE(w_lb, s, w_ub) || !RANGE (clm_lb, k, clm_ub)  ){
      //		FLOOP (i, 0, k) _clmt[jump[i]]--; 
      //	}
			//}

			_eles_org += k;

			// LOAD_TPOSEの時はこの条件
	    if ( !RANGE(w_lb, s, w_ub) || !RANGE (clm_lb, k, clm_ub)  ){
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
		reallocx (_rw, kk, _rows_org, TRSACT_DEFAULT_WEIGHT, goto ERR);
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

	int file_count (FILE2 *fp, QUEUE_ID row_lb, QUEUE_ID row_ub ,char *wf){

	
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
        	reallocx (jump, jump_end, k, 0, goto ERR);
      		jump[k] = item;
        	k++;
        	s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

          // count/weight-sum for the transpose mode
        	reallocx (_clmt, _clm_end, item, 0, goto ERR);
        	_clmt[item]++;

					// NOT TPOSE
          reallocx (_cw, _cw_end, item, 0, goto ERR);
          _cw[item] += MAX(w,0);    // sum up positive weights
	      }

  		} while ( (FILE_err&3)==0);

       // count/weight-sum for the transpose mode
			reallocx (_rowt, _row_end, _rows_org, 0, goto ERR);
    	_rowt[_rows_org] = k;

			if ( k==0 && FILE_err&2 ) break;

			_rows_org++;  // increase #transaction
    
			if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

			//if ( k==0 ){
			//	_str_num++;  // increase #streams if empty transaction is read
			//}
			//if ( k!=0 ){
			//	_eles_org += k;
			//	// LOAD_TPOSEの時はこの条件
	    //  if ( !RANGE(w_lb, s, w_ub) || !RANGE (clm_lb, k, clm_ub)  ){
      //		FLOOP (i, 0, k) _clmt[jump[i]]--; 
      //	}
			//}

			_eles_org += k;

			// NOT LOAD_TPOSEの時はこの条件
	    if( !RANGE (row_lb, k, row_ub) ){
      	FLOOP (i, 0, k) _clmt[jump[i]]--; 
      }


		} while ( (FILE_err&2)==0);

		free2 (jump);
		
		/*
		std::cerr << "_clm" << std::endl;
		for(int i=0;i<_clm_end;i++){
			std::cerr << "clm:" << i << " " << _clmt[i]<< std::endl;
		}
		
		std::cerr << "_row" << std::endl;
		for(int i=0;i<_row_end;i++){
			std::cerr << "rowt:" << i << " " << _rowt[i]<< std::endl;
		}
		//exit(1);
		*/
	



    // swap the variables in transpose mode
  	if ( _rw == NULL ){
  		_total_w_org = _total_pw_org = _rows_org; 
  		return 0; 
  	} 
		_clm_btm = MIN(kk, _rows_org);
		reallocx (_rw, kk, _rows_org, TRSACT_DEFAULT_WEIGHT, goto ERR);
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

	  FILE_COUNT C ;
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
    	  	reallocx (_clmt, _clm_end, _clms, 0, goto END);
    	  }
			}
			
			if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && x >= _clms ){
      	_clms = x+1;
				if ( fc ) { 
					reallocx (_clmt, _clm_end, _clms, 0, goto END);
				}
	    }
 
 			if ( x >= _rows ){
      	_rows = x+1;
				if ( fr ) { 
					reallocx (_rowt, _row_end, _rows, 0, goto END);
				}
			}
		
			if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && y >= _rows ){ // for undirected edge version
      	_rows = y+1;
	      if ( fr ) { 
	      	reallocx (_rowt, _row_end, _rows, 0, goto END);
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
  	reallocx (_clmt, _clm_end, _clms, 0, goto END);
  }
  if ( fr ){
    reallocx (_rowt, _row_end, _rows, 0, goto END);
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








