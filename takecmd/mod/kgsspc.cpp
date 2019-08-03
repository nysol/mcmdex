/* SSPC: Similar Set Pair Comparison */
/* 2007/11/30 Takeaki Uno,   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about this code for the users.
   For the commercial use, please make a contact to Takeaki Uno. */

/*   internal_params.l1 = #solutions
  internal_params.l2 = #rows
  internal_params.l3 = #columns
*/

#include "kgsspc.hpp"

// comment out if single-core mode
//#define MULTI_CORE

#define WEIGHT_DOUBLE
typedef struct {

#ifdef MULTI_CORE
  pthread_t _thr; // thread identifier
#endif
  QUEUE_INT **_o;
  WEIGHT *_w;
  OFILE2 *_fp;
  int _core_id;
  int *_lock_i;

} SSPC_MULTI_CORE;

using namespace std;

void KGSSPC::help(void){

  fprintf(stderr,"SSPC: [ISCfQq] [options] input-filename ratio/threshold [output-filename]\n\
%%:show progress, _:no message, +:write solutions in append mode\n\
#:count the number of similar records for each record\n\
i(inclusion): find pairs [ratio] of items (weighted sum) of one is included in the other (1st is included in 2nd)\n\
I(both-inclusion): find pairs s.t. the size (weight sum) of intersection is [ratio] of both\n\
S:set similarity measure to |A\\cap B| / max{|A|,|B|}\n\
s:set similarity measure to |A\\cap B| / min{|A|,|B|}\n\
T(intersection): find pairs having common [threshld] items\n\
R(resemblance): find pairs s.t. |A\\capB|/|A\\cupB| >= [threshld]\n\
P(PMI): set similarity measure to log (|A\\capB|*|all| / (|A|*|B|)) where |all| is the number of all items\n\
F:set similarity measure to F-value (2*precision*recall)/(precision+recall)\n\
C(cosign distance): find pairs s.t. inner product of their normalized vectors >= [threshld]\n\
f,Q:output ratio/size of pairs following/preceding to the pairs\n\
N:normalize the ID of latter sets, in -c mode\n\
n:do not consider a and b in the set when comparing a and b\n\
Y(y):output elements of each set that contribute to no similarity (y:fast with much memory use)\n\
1:remove duplicated items in each transaction\n\
0:compare xth row with xth row\n\
t:transpose the database so that i-th transaction will be item i\n\
E:input column-row representation\n\
w:load weight of each item in each row (with E command)\n\
[options]\n\
-2 [num]:2nd input file name\n\
-9 [th] [filename]:write pairs satisfies 2nd threshold [th] to file [filename]\n\
-K [num]:output [num] pairs of most large similarities\n\
-k [num]:output [num] elements of most large similarities, for each element\n\
-w [filename]:read item weights from [filename]\n\
-W [filename]:read item weights in each row from [filename]\n\
-l,-u [num]:ignore transactions with size (weight sum) less/more than [num]\n\
  (-ll,-uu [ratio]:give the threshold by ratio of all items/transactions)\n\
-L,-U [num]: ignore items appearing less/more than [num]\n\
  (-LL,-UU [ratio]:give the threshold by ratio of all items/transactions)\n\
-c [num]:compare transactions of IDs less than num and the others (if 0 is given, automatically set to the boundary of the 1st and 2nd file)\n\
-b [num]:ignore pairs whose maximum common item is at most [num]\n\
-B [num]:ignore pairs whose minimum common item is at lease [num]\n\
-T [num]:ignore pairs whose intersection size is less than [num]\n\
    (-TT [num]: -T with outputting intersection size to the 1st column of each line\n\
-P [filename]:compare the pairs written in the file of [filename]\n\
-M [num]: for multi-core processors, use [num] processors (threads: need to be compiled with multi_core option\n\
-# [num]:stop after outputting [num] solutions\n\
-, [char]:give the separator of the numbers in the output\n\
-Q [filename]:replace the output numbers according to the permutation table given by [filename]\n\
# the 1st letter of input-filename cannot be '-'.\n\
# if the output file name is -, the solutions will be output to standard output.\n");
  EXIT;
}

/***********************************************************************/
/*  read parameters given by command line  */
/***********************************************************************/
int KGSSPC::setArgs(int argc, char *argv[]){

	int iflag = 0;

	int c=1;

	if ( argc < c+3 ){  help(); return 1; }

	if ( !strchr (argv[c], '_') ){ iflag |= SHOW_MESSAGE;  _showFlag= true; }
	if ( strchr (argv[c], '%') ) { iflag |= SHOW_PROGRESS; _progressFlag= true;}
	if ( strchr (argv[c], '+') ) iflag |= ITEMSET_APPEND;
	if ( strchr (argv[c], 'f') ) iflag |= ITEMSET_FREQ;
	if ( strchr (argv[c], 'Q') ) iflag |= ITEMSET_PRE_FREQ;

	if ( strchr (argv[c], 'i') )      _problem = SSPC_INCLUSION;
	else if ( strchr (argv[c], 'I') ) _problem = SSPC_SIMILARITY;
	else if ( strchr (argv[c], 'T') ) _problem = SSPC_INTERSECTION;
	else if ( strchr (argv[c], 's') ) _problem = SSPC_MININT;
	else if ( strchr (argv[c], 'S') ) _problem = SSPC_MAXINT;
	else if ( strchr (argv[c], 'R') ) _problem = SSPC_RESEMBLANCE;
	else if ( strchr (argv[c], 'P') ) _problem = SSPC_PMI;
	else if ( strchr (argv[c], 'F') ) _problem = SSPC_FVALUE;
	else if ( strchr (argv[c], 'C') ) _problem = SSPC_INNERPRODUCT;
	else {
		fprintf(stderr,"i, I, s, S, R, T or C command has to be specified\n");
		return 1; 
	}

	if ( strchr (argv[c], '#') ) _problem |= SSPC_COUNT;
	if ( strchr (argv[c], 'N') ) _problem |= PROBLEM_NORMALIZE;
	if ( strchr (argv[c], 'n') ) _problem |= SSPC_NO_NEIB;
	if ( strchr (argv[c], 'Y') ) _problem |= SSPC_POLISH;
	if ( strchr (argv[c], 'y') ) _problem |= SSPC_POLISH2;

	if ( !strchr (argv[c], 't') ) _tflag |= LOAD_TPOSE;
	if ( strchr (argv[c], 'E') )  _tflag |= LOAD_ELE;
	if ( strchr (argv[c], 'w') )  _tflag |= LOAD_EDGEW;
	if ( strchr (argv[c], '1') ){ _rmDupFlag = true ; }// _tflag |= LOAD_RM_DUP;
	if ( strchr (argv[c], '0') ) _problem |= SSPC_COMP_ITSELF;
	
	c++;
  
	while ( argv[c][0] == '-' ){

		switch (argv[c][1]){

			case 'K': 
				_ipara._topk_k = atoi(argv[c+1]);
			break; case 'k': 
				_itemtopk_item  = atoi(argv[c+1]); 
				_itemtopk_item2 = 1;

			break; case 'L': 
				if ( argv[c][2] == 'L' ) _limVal._row_lb_ = atof(argv[c+1]);
				else 										 _limVal._row_lb  = atoi(argv[c+1]); 

			break; case 'U':  
				if ( argv[c][2] == 'U' ) _limVal._row_ub_ = atof(argv[c+1]);
				else									   _limVal._row_ub  = atoi(argv[c+1]);

			break; case 'l':  
				if ( argv[c][2] == 'l' ) _limVal._clm_lb_ = atof(argv[c+1]);
				else                     _limVal._w_lb    = atof(argv[c+1]);

			break; case 'u': 
				if ( argv[c][2] == 'u' ) _limVal._clm_ub_ = atof(argv[c+1]);
				else                     _limVal._w_ub    = atof(argv[c+1]);

			break; case 'w': 
				_wfname = argv[c+1];

			break; case 'W':
				_iwfname = argv[c+1];
 
      //break; case 'c':  //別途考える
			//	_dir = 1; 
			//	_sep = _root = atoi(argv[c+1]) ;

			break; case '2': 
				_fname2 = argv[c+1];

			break; case '9': 
				_th2 = atof(argv[c+1]);
				c++; 
				_output_fname2 = argv[c+1];

			break; case 'b': 
				_len_lb = atoi(argv[c+1]);

			break; case 'B': 
				_len_ub = atoi(argv[c+1]);

			break; case 'T': 
				_th = atoi(argv[c+1]);
				if ( argv[c][2] == 'T' ){ _problem |= SSPC_OUTPUT_INTERSECT; }

			break; case 'M': 
				if ( atoi(argv[c+1]) <= 0 || atoi(argv[c+1])>CORE_MAX){
					fprintf(stderr,"number of cores has to be in 1 to: %d\n",atoi(argv[c+1]));
					return 1;
				}
				_ipara._multi_core = atoi(argv[c+1]);
          	
			break; case '#': 
				_ipara._max_solutions = atoi(argv[c+1]);

			break; case ',': 
				_ipara._separator = argv[c+1][0];

			break; case 'Q': 
				_outperm_fname = argv[c+1];

			break; default: 
				goto NEXT;
		}
    c += 2;
		if ( argc<c+2 ){ 
			help(); 
			return 1; 
		}
	}

	NEXT:;
	
	_fname = argv[c];
	_ipara._frq_lb = atof(argv[c+1]);
	
	if ( argc>c+2 ) _output_fname = argv[c+2];

	_ipara._flag = iflag;

	// using -b -B  :ignore pairs whose maximum/minimum common item
	if ( _len_ub < INTHUGE || _len_lb > 0 ){ 
		_rowSortDecFlag = true;
	}
	// boundary set
	_C.setLimit(_limVal);
	
	return 0;

}

WEIGHT KGSSPC::comp ( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq)
{

  WEIGHT cc, ccc;
  if ( _problem & SSPC_INTERSECTION ) return (c);
  else if ( _problem & SSPC_INNERPRODUCT ){
    if ( _T.exist_w() ) return (c);
    else return (c / sq / sqrt(wx));
  } 
  else if ( (_problem & SSPC_RESEMBLANCE) && (cc= wi +wx -c) != 0 ) return (c/cc);
  else if ( (_problem & SSPC_MAXINT) && (cc=MAX(wi,wx)) != 0 ) return (c/cc);
  else if ( (_problem & SSPC_MININT) && (cc=MIN(wi,wx)) != 0 ) return (c/cc);
  else if ( (_problem & SSPC_PMI) && (cc=wi*wx) != 0 ) return ( log( ( c * _T.get_t()) / cc ) /  -log ( c / _T.get_t()));
  else if ( (_problem & SSPC_FVALUE) && MAX(wi,wx) != 0 ){ cc = c/wi; ccc = c/wx; return ( 2*cc*ccc / (cc+ccc)); }

  return (-WEIGHTHUGE);
}

void KGSSPC::output(
	QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, 
	QUEUE *itemset, WEIGHT frq, int core_id
){

	size_t b;

	if ( _problem & (SSPC_POLISH+SSPC_POLISH2) ){

    _vecchr[i] = 1;

		// store the solution
		if ( _problem & SSPC_POLISH2 ){  

			SPIN_LOCK (_II.get_multi_core(), _II.get_lock_counter());

			// use deleted cell
			if ( (b = _itemary[_T.get_clms()]) ){
				_itemary[_T.get_clms()] = _buf[b]; 
      }
      else { // allocate new cell  
				b = _buf_end;
				_buf.realloci(b+30);
				_buf_end += 2;
			}
			_buf[b] = _itemary[i];  // insert the cell to list i
			_buf[b+1] = ii;
			_itemary[i] = b;

			SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());
    }
	} 
	else if ( _problem & SSPC_COUNT ){
		(*cnt)++;
	}
	else {

		if ( _problem & SSPC_OUTPUT_INTERSECT ){
			_II.print_int(core_id, _siz, 0);
			_II.putc(core_id,' ');
		}

		if ( _table_fname ){ 
			itemset->set_v(0,i);
			itemset->set_v(1,ii); 
		}
		else { 
			itemset->set_v(0, _positPERM[i]); 
			itemset->set_v(1,_positPERM[ii]); 
		}
		/*
		if ( (_problem&PROBLEM_NORMALIZE)&& _dir>0 ){
			if ( i  >= _sep ) itemset->minus_v(0,_root);
			if ( ii >= _sep ) itemset->minus_v(1,_root);
		}
    */
		if ( _itemtopk_end > 0 ){
			_II.output_itemset_k ( itemset, frq, frq,  i, ii, core_id);
			_II.output_itemset_k ( itemset, frq, frq,  ii, i, core_id);
		}
		else{
			_II.output_itemset_ ( itemset, frq, frq,  core_id);
		}
	}
}

/* output a pair if it is in the similarity */
void KGSSPC::comp2 (
	QUEUE_ID i, QUEUE_ID x, WEIGHT c, WEIGHT wi, WEIGHT wx, 
	double sq, QUEUE_INT *cnt, OFILE2 *fp, QUEUE *itemset, 
	int core_id)
{

  WEIGHT f1, f2, f1_=0, f2_=0, frq;

  int f=0, f_=0;
  QUEUE_ID i_ = i, x_ = x;

  if ( !_table_fname ){ x_ = _positPERM[x_]; i_ = _positPERM[i_]; }

	/*
  if ( (_problem&PROBLEM_NORMALIZE)&& _dir>0 ){
    if ( i_ >= _sep ) i_ -= _root;
    if ( x_ >= _sep ) x_ -= _root;
  }
	*/
	// threshold for the intersection size 
  if ( c < _th ) return;  

	// outputting intersection size
  _siz = c; 

	// S R C T s P F  (i I 以外) 
  if ( _problem & (SSPC_INTERSECTION +SSPC_RESEMBLANCE +SSPC_INNERPRODUCT +SSPC_MAXINT +SSPC_MININT + SSPC_PMI + SSPC_FVALUE) ){

    frq = comp (c, wi, wx, sq);

    if ( frq == -WEIGHTHUGE ) return;

    if ( frq >= _II.get_frq_lb() ) output( cnt, x, i, itemset, frq, core_id);

    if ( _output_fname2 && frq >= _th2 ){

      SPIN_LOCK(_II.get_multi_core(), _II._lock_output);
      fp->print("%d %d\n", x_, i_);
      SPIN_UNLOCK(_II.get_multi_core(), _II._lock_output);

    }
  } 
  else {

		// size of i and x
    f1 = wi * _II.get_frq_lb(); 
    f2 = wx * _II.get_frq_lb();  

    if ( _output_fname2 ){ f1_ = wi*_th2; f2_ = wx*_th2; }

    if ( _problem & SSPC_SIMILARITY ){

      if ( _output_fname2 ) f_ = ( (c >= f1_) && (c >= f2_) );
      else f = ( (c >= f1) && (c >= f2) );

      frq = MIN(c / wi, c / wx);

    }
  	else if( _problem & SSPC_INCLUSION ){

      if ( c >= f2 ){
        frq = c / wx;
        output ( cnt, x, i, itemset, frq, core_id);
      }

      f = ( c >= f1 );
      frq = c / wi;

      if ( _output_fname2 ){
        if ( c >= f2_ ){
          SPIN_LOCK(_II.get_multi_core(), _II.get_lock_output());
          fp->print("%d %d\n", x_, i_);
          SPIN_UNLOCK(_II.get_multi_core(), _II.get_lock_output());
        }
        f_ = (c >= f1_);
      }
    }
    else{
    	throw("non similrarity");
    }

    if ( f ) {
    	output ( cnt, i, x, itemset, frq, core_id);
    }

    if ( _output_fname2 && f_ ){
      SPIN_LOCK(_II.get_multi_core(), _II.get_lock_output());
      fp->print("%d %d\n", i_, x_);
      SPIN_UNLOCK(_II.get_multi_core(), _II.get_lock_output());
    }

  }
}

/* iteration for muticore mode */
void *KGSSPC::iter (void *p){

  SSPC_MULTI_CORE *SM = (SSPC_MULTI_CORE *)p;
  int core_id   = SM->_core_id;
  QUEUE_INT **o = SM->_o;
  WEIGHT *w     = SM->_w;
  OFILE2 *fp      = SM->_fp;

  char *mark = NULL;

  QUEUE jump(_T.get_clms()), itemset(2,2);

  QUEUE_ID ii=0, t, m;
  QUEUE_INT i=0, i_=0, ff;
  QUEUE_INT *OQend,  *x, *oi, *oj;
  //QUEUE_INT cnt = ((_problem&SSPC_COMP_ITSELF) && _dir == 0)?1:0;
	QUEUE_INT cnt = ( _problem&SSPC_COMP_ITSELF )?1:0;


  WEIGHT *occ_w,  c, *y;
  WEIGHT yy=0, wi=0, wx=0;


  double sq = 0;

  size_t b, bb;
  int f;

  if ( _problem & SSPC_NO_NEIB ){
  	mark = new char[_T.get_clms()]();
  }
  occ_w = new WEIGHT[_T.get_clms()]();
  OQend = new QUEUE_INT[_T.get_clms()]();

  while (1){


    if ( i == i_ ){

      i_ = 100000000 / (_T.get_eles() / _T.get_clms());
			
      SPIN_LOCK (_II.get_multi_core(), _II.get_lock_counter());  // lock!!

      if ( (i = *(SM->_lock_i)) >= _T.get_clms() ){
        SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());  // unlock!!
        break;
      }

      i_ = MIN(_T.get_clms(), i + 100);
      (*(SM->_lock_i)) = i_;

      if ( _progressFlag ){
        if ( (int)((i-1)*100/_T.get_clms()) < (int)(i*100/_T.get_clms()) )
            fprintf (stderr, "%d%%\n", (int)(i*100/_T.get_clms()));
      }
      SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());  // unlock!!

    }

    if ( (_problem & SSPC_INNERPRODUCT) && !_T.exist_w() ) sq = sqrt (w[i]);

    if ( _problem & (SSPC_POLISH+SSPC_POLISH2) ) m = _T.get_clms();
  	else m = i;

  	//else m = (_dir>0)?_sep:i;

    jump.setEndByStart(); //jump.set_t(jump.get_s());


    if (_problem & SSPC_NO_NEIB){ // for no_neib
    
			for( x=_T.begin(i) ; x < _T.end(i) ; x++){
				mark[*x] |= 1;
			}

			// for( x= _OQ[i].begin() ; x<_OQ[i].end() ; x++){
			for( x= _OQ.begin(i) ; x<_OQ.end(i) ; x++){
				mark[*x] |= 2;
			}

    }

		for(QUEUE_INT * iix = _OQ.start(i); iix<_OQ.end(i); iix++ ){

      t = *iix;

			// get item weight of current vector
      if ( _T.exist_w() && (_problem & SSPC_INNERPRODUCT)){ 

        y = _T.get_w(t);
	      for( x=_T.begin(t); *x < i  ; x++ ){ y++; }
        yy = *y;

      }

      if ( (_problem & SSPC_NO_NEIB) && t == i ) continue;

      if ( _T.exist_w() ){ y = _T.get_w(t); }
      else                 { y = 0;}


      for( x =_T.begin(t); *x < m ; x++){

        if ( (_problem & SSPC_POLISH2) && *x < i) continue;

        if ( (_problem & SSPC_NO_NEIB) && *x == t ) continue;

        if ( OQend[*x] == 0 ){
          jump.push_back(*x);
          occ_w[*x] = 0;
        }
        OQend[*x]++;

        if (_T.exist_w() ){
          if (_problem & SSPC_INNERPRODUCT){
            occ_w[*x] += (*y) * yy; 
          }
          else { 
          	occ_w[*x] += *y;
          }
          y++;
        }
        else {
          occ_w[*x] += _w[t]; 
        }
      }
    }

		for(x=jump.begin(); x < jump.end(); (x)++){

      if ( *x == i ) goto SKIP;

      c = occ_w[*x];

      if ( _rowSortDecFlag ){
        for (oi=o[i],oj=o[*x] ; *oi< _len_lb ; oi++ ){
          while ( *oj < *oi ) oj++;
          if ( *oi == *oj ) c += _w[*oi];
        }
      }

      wi = w[i];
      wx = w[*x];

      if (_problem & SSPC_NO_NEIB){  // for no_neib
        if ( mark[*x]&1 ) wi -= _w[*x];
        if ( mark[i] &2 ) wx -= _w[i];
      }

      comp2 ( i, *x, c, wi, wx, sq, &cnt, fp, &itemset, core_id);

      SKIP:;
      OQend[*x] = 0;

    }

		// selfcomparison
    //if ((_problem & SSPC_COMP_ITSELF) && _dir == 0) {
    //    output (
    //    	&cnt, i, i, &itemset, 
    //    	((_problem&PROBLEM_NORMALIZE)&& _dir>0)? i- _sep: i, //_dir>0ありえない
    //    	core_id
    //    );
		//	}
		
    
		// selfcomparison
    if ((_problem & SSPC_COMP_ITSELF) ) {
        output ( &cnt, i, i, &itemset, i, core_id );
    }

		// data polish;  clear OQ, and marks
    if ( _problem & (SSPC_POLISH+SSPC_POLISH2) ){  

      if ( _problem & SSPC_POLISH2 ){
          for (b=_itemary[i] ; b ; b=_buf[b]) _vecchr[_buf[b+1]] = 1;
      }
      f = 0;

      for(QUEUE_INT *iq = _OQ.start(i) ; iq < _OQ.end(i) ; iq++ ){

        ff = 0;

				for( x=_T.get_vv(*iq); *x < _T.get_clms() ; x++){
					if ( _vecchr[*x] ){ 
						ff = 1; 
						break; 
					}
        }
        if ( ff ){
          _II.print_int(core_id, *iq , f);
          f = _II.get_separator();
        }
      }

     	_II.putc(core_id,'\n');
			_II.flush(core_id);

      jump.clrMark( _vecchr );

			// data polish;  clear OQ, and marks
      if ( _problem & SSPC_POLISH2 ){  

        for (b=_itemary[i] ; b ; b=bb){ // insert cells to deleted cell queue
          bb = _buf[b];
          _vecchr[_buf[b+1]] = 0;
          _buf[b] = _itemary[_T.get_clms()];
          _itemary[_T.get_clms()] = b;
        }

      }
    }

    if (_problem & SSPC_NO_NEIB){ // for no_neib
    	_T.clrMark(i,mark); // これなに？
			_OQ.clrMark(i,mark);
    }
		_OQ.endClr(i);

    if ( _problem & SSPC_COUNT ){
      //while ( ii<_II.get_perm(i) ){
      while ( ii < _positPERM[i] ){
				_II.putc(core_id,'\n');
				_II.flush(core_id);
        ii++;
      }
      _II.print_int(core_id,cnt, 0);
      _II.putc(core_id,'\n');
      _II.flush(core_id);
      _II.add_sc(2,cnt);
      ii++;
    }
    i++;
  }
  delete [] mark;
  delete [] occ_w;
  delete [] OQend;

  //mfree (mark, occ_w,OQend);

  return (NULL);

}
/*************************************************************************/
/* SSPC main routine */
/*************************************************************************/
void KGSSPC::_SspcCore(){

	#ifdef MULTI_CORE
  	void *tr;
	#endif

  SSPC_MULTI_CORE *SM = NULL;
  OFILE2 fp;  // file pointer for the second output file


  // QUEUE_ID i; 
  //QUEUE_ID begin = (_problem&(SSPC_POLISH+SSPC_POLISH2))?0:(_dir>0?_sep:0);
  QUEUE_ID begin =0;
  QUEUE_INT **o=NULL;
  WEIGHT *w;
  int cnt;

	_OQ.setfromT(_T);

      
  if ( _output_fname2 ) fp.open(_output_fname2);

  // initialization
  w = new WEIGHT[_T.get_clms()*2]();
  
  if(_problem&SSPC_INNERPRODUCT){

  	if(!_T.exist_w()){ 
			for(QUEUE_INT i =0 ; i < _T.get_clms(); i++){
				_w[i] *= _w[i];
			}
  	}

	  //_TT.delivery( w, w+_T.get_clms(), NULL, _T.get_clms());
	  _jump.setEndByStart();
  	for(VEC_ID t=0 ; t<_T.get_t(); t++){
    	_T.delivery_iter( 
    		w, w+_T.get_clms(), t, _T.get_clms() ,
    		&_jump,_OQ.getOQ(), _w,_pw,
    		_C.existNegative()
    	);
    }

	  if(_T.exist_w() ){ 

		  for(QUEUE_INT i=0; i<_T.get_clms() ; i++){ w[i] = 0; } 
		
			for (VEC_ID i=0 ; i < _T.get_t();i++){
				WEIGHT *y =_T.get_w(i);
				for(QUEUE_INT *x = _T.get_vv(i); x < _T.get_vv(i)+_T.get_vt(i);x++){
					w[*x] += (*y)*(*y); y++;
				}
			}

			for(QUEUE_INT i=0;i<_T.get_clms();i++){ w[i] = sqrt(w[i]); }

			for(VEC_ID i=0 ; i<_T.get_t();i++){
  		  WEIGHT *y = _T.get_w(i);
				for(QUEUE_INT *x=_T.get_vv(i);x<_T.get_vv(i)+_T.get_vt(i); x++){
					 *y /= w[*x]; y++;
				}
			}
	  }//ここなに // normalize the vectors for inner product (c + -W の指定時
  }
  else{
	  //_TT.delivery( w, w+_T.get_clms(), NULL, _T.get_clms());
	  _jump.setEndByStart();
  	for(VEC_ID t=0 ; t<_T.get_t(); t++){
    	_T.delivery_iter( 
    		w, w+_T.get_clms(), t, _T.get_clms() ,
    		&_jump,_OQ.getOQ(), _w,_pw,
    		_C.existNegative()
    	);
    }
  }
    
	for(QUEUE_INT i =0 ;i < _T.get_clms();i++){
		_OQ.set_end(i,0);
	}


	// skipping items of large frequencies
	if ( _rowSortDecFlag ){

		o = new QUEUE_INT *[_T.get_clms()];

		for ( QUEUE_ID i =0 ; i < _T.get_clms() ; i++){
			o[i] = _OQ.get_v(i);
			_OQ.set_sentinel(i);
			QUEUE_ID j;
			for (j=0 ; _OQ.get_v(i,j) < _len_lb ; j++); //skip
			_OQ.move(i,j);
		}
	}


	// selfcomparison
  //if ((_problem & SSPC_COMP_ITSELF) && _dir == 0){ 
  if ((_problem & SSPC_COMP_ITSELF)){ 
 		QUEUE itemset(2,2);
		output( &cnt, 0, 0, &itemset, 0, 0);
    itemset.clear();
  }

  // for multi-core
	SM = new SSPC_MULTI_CORE[_II.get_multi_core()]; //malloc2


	for (int i=_II.get_multi_core(); (i--) > 0 ; ){
    SM[i]._o = o;
    SM[i]._w = w;
    SM[i]._fp = &fp;
    SM[i]._core_id = i;
    SM[i]._lock_i = &begin;
    
#ifdef MULTI_CORE
    if ( i > 0 ) pthread_create(&(SM[i]._thr), NULL, iter, (void*)(&SM[i]));
    else 
#endif
      iter((void*)(&SM[i]));
  }

  // wait until all-created-threads terminate
#ifdef MULTI_CORE

	for(int i=1 ; i < _II.get_multi_core(); i++){
		pthread_join(SM[i]._thr, &tr);
	}

#endif

  // termination これいる？
	if ( _rowSortDecFlag ){
		for(int i=1 ; i < _T.get_clms(); i++){
  	  _OQ.prefin(i,o[i]);

    }
  }

  delete [] w;
  delete [] o;
	delete [] SM;

}


/*************************************************************************/
/* main function of SSPC */
/*************************************************************************/
int KGSSPC::run (int argc ,char* argv[]){

	if( setArgs(argc, argv) ) return 1;

  IFILE2 fp(_fname) , fp2(_fname2) ;
  if( _tflag&LOAD_TPOSE ){
		_C.fileCountT(fp ,fp2 ,_wfname);
  }
  else{
		_C.fileCount(fp ,fp2 ,_wfname);
  }

	PERM *cperm = _C.makeCperm();

	if (cperm==NULL) { throw ("there is no frequent item"); }

	PERM *rperm = _C.makeRperm(_rowSortDecFlag);

	// f は 
  // (_T.get_eles() > _C.c_eles() && !(_flag & LOAD_TPOSE) );
	// (row eles ＞clm eles かつ TOPOSEでない (row eles == Tra No))
  // の内容　
  // fがセットさていない場合は　  _T.setVBufferが動くのでバッファが_Tにセットされている？

	int f = ( _C.r_eles() > _C.c_eles() && !(_tflag&LOAD_TPOSE));


	// _Tのバッファ _v _bufもセットされる
	_T.setSize4sspc(_C,_tflag&LOAD_TPOSE); 

  _w.malloc2( _T.get_end());

  if ( _C.existNegative() ){
  	_pw = new WEIGHT[_T.get_end()];
  }

  _perm = new PERM[_T.get_clms()+1](); //calloc2 

  _jump.alloc(_T.get_clms()+1);

  if ( (!_T.exist_w()) && _iwfname ) {
		 _T.alloc_weight(_C); 
	}


	// ============================この辺から _T .loadでまとめる===================================

  _trperm = new PERM[_T.get_t()];   //malloc2 

	VEC_ID tt=0 ;

  size_t pos = 0; 
  // set variables w.r.t rows
	for( VEC_ID t =0 ; t < _C.rows(); t++ ){

		if(	rperm[t] <= _C.rows() ) {

      _T.init_v(tt);
			_trperm[tt] = t;
			rperm[t] = tt;
      _w[tt]   = _C.get_rw(t);
      if ( _pw ) {  _pw[tt] = MAX (_w[tt], 0); }

      if ( !f ){ // elementのチェック 多分
        _T.setVBuffer(tt,pos); // _T の_v ,_buf連動させる)
        pos += _C.get_rowt(t)+1;
      }
			tt++;
		}
	}

  // make the inverse perm of items
  for(VEC_ID t =0 ; t < _C.clms() ; t++ ){
    if ( cperm[t] <= _C.c_end() ){
      _perm[cperm[t]] = t;
    }
  }

	// _file_read
  VEC_ID t=0;

	// _v _bufの連動
  if ( f ) {  _T.setVBuffer(0, 0); }

  fp.reset();	  
  if( _iwfname ){ // sspcでitem weightを指定した時のみ
	  IFILE2 wfp(_iwfname);
		_T.file_read( fp , wfp, _C , &t , f , _tflag);
  	
  }
  else{
		_T.file_read( fp , _C , &t ,f , _tflag);
	}

	// fp2にアイテムファイルは指定できないようになってたので
	if( fp2.exist() ){
		fp2.reset();
		_T.file_read( fp2 , _C , &t , f , _tflag);
	}

	_T.initQUEUEs();

	// sort rows for the case 
	// that some columns are not read
  if ( _rowSortDecFlag ){  _T.setInvPermute( rperm, _trperm , -1); }

	 _T.queueSortALL(1); 

  if ( _rmDupFlag ){	_T.rmDup(); }

	// ============================この辺まで _T .loadでまとめる===================================

	_OQ.alloc(_T);

	//_sep -cの指定がある時に利用される ここでOK？もとはTT.alloc
	//_sep = _TT.adjust_sep(_sep); // 

  if ( _len_ub < INTHUGE ){ 
 	  for (VEC_ID i=0 ; i<_T.get_t() ; i++){
    	if ( _T.get_vt(i) <= _len_ub ){ _len_lb = i; break; }
    }
  }	

  if ( _itemtopk_item > 0 ){  _itemtopk_end= _T.get_clms(); }

	_II.setParams( _ipara );

	QUEUE_ID siz  = _T.get_clms();
	QUEUE_ID siz2 = _T.get_t();

	_occ_w = new WEIGHT[siz+2]();
	_vecchr = new char[siz2+2]();

	if(_problem&SSPC_POLISH2) {
		_itemary = new QUEUE_INT[siz+2]();
	}

	if ( _outperm_fname ){

		PERM *p = NULL;

		IFILE2::ARY_Load( p ,_outperm_fname);
		if ( _perm ){
			for(int j =0 ;j < _T.get_clms() ; j++){
				_perm[j] = p[_perm[j]];
     	}
	    delete [] p;
	  }
		else {
    	_perm = p;
    }
	} 

	_II.alloc(_output_fname, _perm, siz, 0, _itemtopk_end,_itemtopk_item,_itemtopk_item2);

	VEC_ID size =  MAX(_T.get_t(),_T.get_clms())+1;
	_w.realloc2(size);

	for(size_t i=0; i<size;i++){ _w[i] = 1;}

  _buf_end = 2;
  _positPERM = _II.get_perm();

  _II.set_perm(NULL);
  
  if ( _T.get_clms()>1 ){  
  	_SspcCore(); 
  }

  _II.set_perm(_positPERM);

  _II.merge_counters();

  if ( _II.get_topk_end() > 0 || _itemtopk_end > 0 ){
  	 _II.last_output();
  }
  else{
  	fprintf(stderr , LONGF " pairs are found\n" , _II.get_sc(2));
  }
	_II.close();

  return 0;

}

std::vector<LONG> KGSSPC::mrun(int argc ,char* argv[]){
	KGSSPC mod;
	mod.run(argc,argv);
	return mod.iparam();
}


//	setfromT()
//	VEC_ID e;
// 	QUEUE_INT *x;
//	QUEUE *occ = &_OQ[_T.get_clms()];
//	QUEUE_INT M= _T.get_clms();
//	for (VEC_ID i=0 ; i< occ->get_t() ; (i)++){ // occ? occ->get_t(): t
//  	e = occ->get_v(i);
//		for ( x=_T.get_vv(e) ; *x < M ; x++){ 
//			_OQ[*x].push_back(e);
//		}
//	}
	//_OQ.alloc
//	VEC_ID *p = _T.counting();
//	QUEUE_INT clm_max = p[0];
//	size_t cmmsize = p[0];
//	for(int cmm = 1 ; cmm < _T.get_clms() ;cmm++ ){
//		cmmsize += p[cmm];
//		if( clm_max < p[cmm]){ clm_max = p[cmm]; }
//	}
//	_OQ = new QUEUE[_T.get_clms()+1]; 
//	QUEUE_INT *cmn_pnt;
//	try{
//		cmn_pnt = new QUEUE_INT[cmmsize+_T.get_clms()+2]; //malloc2
//	}catch(...){
//		delete[] _OQ;
//		throw;
//	}
//	size_t posx=0;
//	for(VEC_ID cmmv =0; cmmv < _T.get_clms() ; cmmv++){
//		_OQ[cmmv].alloc( p[cmmv],cmn_pnt+posx );
//		posx += p[cmmv]+1;
//	}
//	_OQ[_T.get_clms()].alloc( MAX(_T.get_t(), clm_max));
//	// end is illegally set to 0, for the use in "TRSACT_find_same" 
//	for(size_t i=0 ; i < _T.get_clms()+1 ; i++ ){
//		_OQ[i].endClr();
//	}
//	// initial occurrence := all transactions
//	_OQ[_T.get_clms()].initVprem(_T.get_t());  //ARY_INIT_PERM
//	delete [] p;

//	_row_max = _T.RowMax();つかってない

	// _TT.setParams(_tpara);

  // if( _TT.load()){ return 1; }
  
  // TT load

  //if( _C.file_count( _tpara._flag&LOAD_TPOSE , fp ,fp2 ,_tpara._wfname )) { 

	//int tflag = LOAD_INCSORT;
	//_tpara._flag  |= LOAD_INCSORT;

	// _preALLOC(); start

	//_TT.initialQueSet();
  	// _TT.printMes( LONGF " pairs are found\n", _II.get_sc(2));

//  _new_t = _T.get_t();

//void KGSSPC::_preALLOC(){
//	QUEUE_ID siz  = _T.get_clms();
//	QUEUE_ID siz2 = _T.get_t();
//	_occ_w = new WEIGHT[siz+2](); // calloc2
//	_vecchr = new char[siz2+2](); // calloc2
//	if(_problem&SSPC_POLISH2) {
//		_itemary = new QUEUE_INT[siz+2](); //calloc2
//	}
//	///PERM *perm = _TT.convPerm(_outperm_fname);
//	_II.alloc(_output_fname, perm, siz, 0, _itemtopk_end,_itemtopk_item,_itemtopk_item2);
//	//_TT.set_perm(NULL); // free対策？
//	//_TT.reallocW();
//	return;
//}

	//_TT.set_perm(NULL); // free対策？
		
	//_TT.reallocW();
	// _preALLOC(); end

	//o = _TT.skipLaegeFreqItems(_ipara._len_lb);
	//QUEUE_INT **o=NULL;


//prefin
//_OQ[i].add_t ( _OQ[i].get_v() - o[i]);
//_OQ[i].set_v ( o[i]);

