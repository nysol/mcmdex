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

#include "trsact.hpp"
#include "filecount.hpp"


VEC_ID FILE_COUNT::adjust_ClmSep(VEC_ID sep){

	size_t tt=0;
  for(size_t t=0;t<_clms_org;t++){
		if( _cperm[t] <= _clms_org ){
			if( t == _end1 && sep==0 ) sep = tt;
			if( t == sep && sep>0 )  sep = tt;
			tt++;
		}
	}
  return sep;
} 

VEC_ID FILE_COUNT::adjust_RowSep(VEC_ID sep){
	size_t tt=0;
	for(size_t t=0;t<_rows_org;t++){
		if ( _rperm[t] <= _rows_org ){
			if ( t == _end1 && sep==0 ) sep = tt;
			if ( t == sep && sep>0 )  sep = tt;
			tt++;
		}
	}
  return sep;
}
    
    
QUEUE_INT FILE_COUNT::_weight_Scan(char *wf){

	IFILE2 wfp;
	wfp.open(wf);

	#ifdef WEIGHT_DOUBLE
		QUEUE_INT kk = wfp.ARY_Scan_DBL();
	#else
		QUEUE_INT kk = wfp.ARY_Scan_INT();
	#endif

	kk += _rows_org;
	_rw.realloc2(kk+1);

	wfp.reset();
	wfp.VARY_Read(_rw, kk);

	if ( _rw.min(0, kk) < 0 ) {
		_negaFLG = true;
	}

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
int FILE_COUNT::_file_count_T(IFILE2 &fp,char *wf){

	QUEUE_INT item, kk=0, k;
	VECARY<VEC_ID> jump;
	WEIGHT w, s;

	if ( wf ){ kk = _weight_Scan(wf); }

	do {

		s=0; k=0;

		w = wf? (_rows_org<kk? _rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;

		do {
			
			item = (QUEUE_INT)fp.read_int();

			if ( fp.NotNull() && item < TRSACT_MAXNUM && item >= 0 ){ //(FILE_err&4)==0

				// update #items
				ENMAX (_clms_org, item+1);  
        	
				jump.reallocx( k, 0); //これ0でOK？ 
				jump[k] = item;
				k++;
				s += wf? (item<kk? MAX(_rw[item],0): TRSACT_DEFAULT_WEIGHT): 1;

	      // count/weight-sum for the transpose mode
    	  _clmt.reallocx(item, 0); 
      	_clmt[item]++;
	    }

  	} while ( fp.NotEol() );

		// count/weight-sum for the transpose mode
		_rowt.reallocx(_rows_org, 0); 
		_rowt[_rows_org] = k;

		// LOAD_TPOSEの時
		_cw.reallocx( _rows_org, 0); 
    _cw[_rows_org] = s;     // sum up positive weights

		if ( k==0 && fp.Eof() ) break;
		_rows_org++;  // increase #transaction

    // un-weighted case; weighted sum is #included-items
		if ( !wf ) s = k; 

		_eles_org += k;

		// LOAD_TPOSEの時はこの条件
		if( !_limVal.clmOK(s,k) ){
			for(int i0 = 0 ; i0 < k ; i0++){
				_clmt[jump[i0]]--; 
			}
		}

	} while ( fp.NotEof() );

	// swap the variables in transpose mode
	if ( _rw.empty() ){
		_total_w_org = _total_pw_org = _rows_org; 
		return 0; 
	} 

	_rw.reallocx(kk,_rows_org, TRSACT_DEFAULT_WEIGHT);

	for(int i0 = 0 ; i0 < _rows_org ; i0++){
		_total_w_org  += _rw[i0];
		_total_pw_org += MAX(_rw[i0],0);
	}

	return 0;
  
}



// NOT LOAD_TPOSEの時
int FILE_COUNT::_file_count(IFILE2 &fp, char *wf){
	
  // WEIGHT s;
  // QUEUE_INT i,
  
  QUEUE_INT item, kk=0, k;
  WEIGHT w;
  VECARY<VEC_ID> jump;
	LONG jj;

	if ( wf ){ kk = _weight_Scan(wf); }

	do {

		// s=0; 
		k=0;

		w = wf? (_rows_org<kk? _rw[_rows_org]: TRSACT_DEFAULT_WEIGHT): 1;

		do {
			
			item = fp.read_int();

			if ( fp.NotNull() && item < TRSACT_MAXNUM && item >= 0 ){

				ENMAX (_clms_org, item+1);  // update #items

				jump.reallocx(k, 0); 
				jump[k] = item;

				k++;

				// count/weight-sum for the transpose mode
				_clmt.reallocx(item, 0);
				_clmt[item]++;

				// NOT TPOSE
				_cw.reallocx(item, 0);
				_cw[item] += MAX(w,0);    // sum up positive weights

			}

		} while ( fp.NotEol());

		// count/weight-sum for the transpose mode
		_rowt.reallocx (_rows_org, 0);
		_rowt[_rows_org] = k;

		if ( k==0 && fp.Eof() ) break;

		_rows_org++;  // increase #transaction
    
		//if ( !wf ) s = k;   // un-weighted case; weighted sum is #included-items

		_eles_org += k;

		// NOT LOAD_TPOSEの時はこの条件
		if( !_limVal.rowOK(k) ){
			for(int i0=0 ; i0 < k ; i0++ ){
				_clmt[jump[i0]]--; 
			}
		}

	} while ( fp.NotEof() );


	// swap the variables in transpose mode
	if ( _rw.empty() ){
		_total_w_org = _total_pw_org = _rows_org; 
		return 0; 
	} 

	kk = _rw.reallocx( kk, _rows_org, TRSACT_DEFAULT_WEIGHT);

	for(int i0=0 ; i0 < _rows_org ; i0++ ){
		_total_w_org += _rw[i0];
		_total_pw_org += MAX(_rw[i0],0);
	}
	
	return 0;

}

// call from trsact 
// wf2は未実装
int FILE_COUNT::file_count(int flg, IFILE2 &fp, IFILE2 &fp2, char *wf, char *wf2){

	if(flg){ // TPOSE

		if( _file_count_T(fp, wf) ) { return 1; }	
		_end1 = _rows_org;

		if( fp2.exist() ){
			if( _file_count_T ( fp2, NULL) ) { return 1; }
		}
		// swap variables in the case of transpose
		_tpose();

	}
	else{
		if( _file_count( fp, wf) ){ return 1;	}
		_end1 = _rows_org;

		if( fp2.exist() ){
			if( _file_count(fp2, NULL) ) { return 1;	}
		}

	}

	_limVal.setBoundsbyRate(_rows_org,_clms_org);

	return 0;

}
// fstar.cpp <<=kggrphfil.cpp
void FILE_COUNT::countFS(IFILE2 *rfp,int flag, int int_clms)
{
	FILE_COUNT_INT k=0, j, x, y, t=0;

  // flags for using rowt, and clmt, that counts elements in each row/column
  int fr = flag&FILE_COUNT_ROWT; // <<ここは必ずtrue 
  int fc = flag&FILE_COUNT_CLMT; 

	// fe,ft: flag for ele mode, and transposition
	int fe = flag&LOAD_ELE; 
	int ft = flag&LOAD_TPOSE;  

	if ( flag & (FILE_COUNT_GRAPHNUM) ){

		_clms = (FILE_COUNT_INT) rfp->read_int ();
		_rows = _clms; //?
		_eles = (FILE_COUNT_INT) rfp->read_int();

  	rfp->read_until_newline ();
	}

	do {
		
		if ( fe ){
			
			x = (FILE_COUNT_INT)rfp->read_int ();
			if ( rfp->EolOrEof() ) goto ROW_END; //  FILE_err&3 
			
      y = (FILE_COUNT_INT)rfp->read_int (); 
			if ( rfp->Null() ) goto ROW_END; //  FILE_err&4 ここあってる？

      rfp->read_until_newline ();

    }
		else{
    	
			x = t;
			y = (FILE_COUNT_INT)rfp->read_int (); 

			if ( rfp->Null() ) goto ROW_END; // FILE_err&4  ここあってる？

			for(int i0=0 ; i0 <int_clms ; i0++){
				rfp->read_double (); 
				if ( rfp->EolOrEof() ){  break; }//FILE_err&3
      }
		}

		if ( ft ){ SWAP_<FILE_COUNT_INT>(&x, &y); }

		if ( y >= _clms ){
			_clms = y+1;
			if ( fc ) {_clmt.reallocx(_clms, 0);}
		}
			
		if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && x >= _clms ){
			_clms = x+1;
			if ( fc ) { _clmt.reallocx (_clms, 0); }
	  }

		if ( x >= _rows ){
			_rows = x+1;
			if ( fr ) { _rowt.reallocx(_rows, 0); }
		}

		// for undirected edge version	
		if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && y >= _rows ){ 
			_rows = y+1;
			if ( fr ) { _rowt.reallocx(_rows, 0); }
		}

    if ( fc ) { _clmt[y]++; }

    if ( fr ){ 
    	_rowt[x]++; 
    	if ( flag&LOAD_EDGE && x != y ){ _rowt[y]++; }
    }	

		_eles++;

		ROW_END:;

		if ( !fe && rfp->NL() ){ // !fe && (FILE_err&1)

			t++;

			if ( flag&(LOAD_RC_SAME+LOAD_EDGE) ){
				ENMAX (_clms, t); 
				ENMAX (_rows, t);
			}
			else if ( ft ) { _clms = t; } 
      else { _rows = t; }
      
    } 

  } while ( rfp->NotEof());

  if ( fc ){ _clmt.reallocx(_clms, 0); }
  if ( fr ){ _rowt.reallocx(_rows, 0); }

  END:;

  return ;
}

// kgmedset.cpp   FILE_COUNT_ROWT + LOAD_TPOSE( t パラメータ)
//ft : flag&LOAD_TPOSE; 
// FILE_COUNT::countの特殊
// on fr 
// on or off ft
// off fe fc
void FILE_COUNT::countFST( IFILE2 *rfp, int ft )
{

	FILE_COUNT_INT k=0, j, x, y, t=0;

  // flags for using rowt, and clmt, that counts elements in each row/column
	// fe,ft: flag for ele mode, and transposition
  //int fc = flag&FILE_COUNT_CLMT; 
	//int fe = flag&LOAD_ELE; 

	do {

		x = t;
		y = (FILE_COUNT_INT)rfp->read_int (); 

		if ( rfp->Null() ) goto ROW_END; // FILE_err&4  ここあってる？

    	
	  if ( ft ){ SWAP_<FILE_COUNT_INT>(&x, &y); }

	  if ( y >= _clms ){ _clms = y+1; }
		if ( x >= _rows ){
    	_rows = x+1;
			_rowt.reallocx(_rows, 0);
		}

		_rowt[x]++; 
    _eles++;

	  ROW_END:;

  	if ( rfp->NL() ){ // !fe && (FILE_err&1)

    	t++;
      if ( ft ) { _clms = t; } 
     	else { _rows = t; }
	  } 

  } while ( rfp->NotEof());

 	_rowt.reallocx( _rows, 0);

  END:;

  return ;
}

// kgmace.cpp FILE_COUNT_ROWT + LOAD_RC_SAME + LOAD_EDGE + LOAD_ELE( e パラメータ)
// kglcm.cpp  FILE_COUNT_ROWT + LOAD_RC_SAME + LOAD_EDGE 
// fe :flag&LOAD_ELE
// FILE_COUNT::countの特殊
// on fr
// on or off fe
// off  ft fc
void FILE_COUNT::countSG(IFILE2 *rfp,int fe)
{

	FILE_COUNT_INT x, y, t=0;

	do{

		if ( fe ){

			x = (FILE_COUNT_INT)rfp->read_int ();
			if ( rfp->EolOrEof() ) goto ROW_END; //  FILE_err&3 ここあってる？
			
			y = (FILE_COUNT_INT) rfp->read_int (); 
			if ( rfp->Null() ) goto ROW_END; //  FILE_err&4 ここあってる？

			rfp->read_until_newline ();

		}
		else{
			x = t;
			y = (FILE_COUNT_INT)rfp->read_int(); 
			if ( rfp->Null() ) goto ROW_END; // FILE_err&4  ここあってる？
		}

		if ( y >= _clms ){ _clms = y+1; }
		if ( x >= _clms ){ _clms = x+1; }
 
		if ( x >= _rows ){
			_rows = x+1;
			_rowt.reallocx(_rows, 0);
		}

		if ( y >= _rows ){  // for undirected edge version	
			_rows = y+1;
			_rowt.reallocx(_rows, 0);
		}
		
		_rowt[x]++; 
		if ( x != y ){ _rowt[y]++; }

		_eles++;

		ROW_END:;

		if ( !fe && rfp->NL() ){ // !fe && (FILE_err&1)
			t++;
			ENMAX (_clms, t); 
			ENMAX (_rows, t);
		} 

	} while ( rfp->NotEof());

	_rowt.reallocx( _rows, 0);

  return ;

}

//void FILE_COUNT::initCperm(VEC_ID ttt , PERM *p ,QUEUE_INT c_end , bool flag){
// pfname :filename
// wflg : true:write to pfname file   false: read from pfname file
// このクラス_crpermはいらない？
void FILE_COUNT::initCperm(int tflag,int tflag2){

	// ttt :perm Size 
	VEC_ID ttt_max = _clms_org;
	VEC_ID ttt = _clms_org;
  PERM *p=NULL;

  // LOAD_PERM TRSACT_FRQSORT  デフォルトでセットされる(LCM) //この辺りも分ける？
  // LOAD_INCSORTはセットされない(sgflagにセットされるかのうせいあり)
	if ( tflag&LOAD_PERM ){ 
		if ( tflag2&TRSACT_FRQSORT ){ 
			p = _clmw_perm_sort((tflag&LOAD_INCSORT)?1:-1);
		}
		else {
			p = _clmt_perm_sort((tflag&LOAD_INCSORT)?1:-1);
		}
	}
	
	_clms_end = MAX(_clms_org, ttt_max);
	_c_eles=0;
	_c_clms=0;

	_cperm = new PERM[_clms_org+1]; // malloc2
	for(size_t i =0 ;i<_clms_org;i++){ 
		_cperm[i] = _clms_org+1; 
	}
	VEC_ID tt =0 ;
	for(size_t t=0; t < ttt; t++){
		tt = p? p[t]: t;
		if ( tt >= _clms_org ) continue;
		    
		if( _limVal.clmOK(_cw[tt],_clmt[tt]) ){
			_c_eles += _clmt[tt];
			_cperm[tt] = _c_clms++ ;
		}
		else{
			_cperm[tt] = _clms_end+1 ;
		}
	}
	delete [] p;
	return ;
}


void FILE_COUNT::initRperm(int tflag){

  PERM *p=NULL;

  if( tflag& LOAD_SIZSORT){ //( tflag&(LOAD_SIZSORT+LOAD_WSORT) )
		p = _rowt_perm_sort((tflag&LOAD_DECROWSORT)?-1:1);
  }
 
	_rperm = new PERM[_rows_org];//malloc2
			
	// compute #elements according to rowt, and set rperm
	VEC_ID tt=0;
	for( VEC_ID t=0 ; t<_rows_org ; t++){
		tt = p? p[t]: t; 
		if(_limVal.rowOK(_rowt[tt])){
			_rperm[tt] = _r_clms++;
			_r_eles += _rowt[tt];
		}
		else{
			_rperm[tt] = _rows_org+1;
		}
	}
	return;
}
	

void FILE_COUNT::makePerm(int tflag,int tflag2){

	initCperm(tflag,tflag2);
	if ( _c_clms == 0 ) throw ("there is no frequent item");
	initRperm( tflag ); 

	return ;
}




