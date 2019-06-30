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

  _th = 0; 
  _th2 = 0;
  _multi_core =1;

  int c=1;

  if ( argc < c+3 ){  help(); return 1; }

  if ( !strchr (argv[c], '_') ){ _iFlag |= SHOW_MESSAGE; _tFlag|= SHOW_MESSAGE; }
  if ( strchr (argv[c], '%') ) _iFlag |= SHOW_PROGRESS;
  if ( strchr (argv[c], '+') ) _iFlag |= ITEMSET_APPEND;
  if ( strchr (argv[c], 'f') ) _iFlag |= ITEMSET_FREQ;
  if ( strchr (argv[c], 'Q') ) _iFlag |= ITEMSET_PRE_FREQ;

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

  if ( !strchr (argv[c], 't') ) _tFlag |= LOAD_TPOSE;
  if ( strchr (argv[c], 'E') )  _tFlag |= LOAD_ELE;
  if ( strchr (argv[c], 'w') )  _tFlag |= LOAD_EDGEW;
  if ( strchr (argv[c], '1') )  _tFlag |= LOAD_RM_DUP;
  if ( strchr (argv[c], '0') )_problem |= SSPC_COMP_ITSELF;
  c++;
  
  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){

			case 'K': 
				_topk_k = atoi(argv[c+1]);

    	break; case 'k': 
    		_itemtopk_item  = atoi(argv[c+1]); 
    		_itemtopk_item2 = 1;

			break; case 'L': 
      	if ( argv[c][2] == 'L' ) _row_lb_ = atof(argv[c+1]);
        else 										 _row_lb  = atoi(argv[c+1]); 

			break; case 'U':  
      	if ( argv[c][2] == 'U' ) _row_ub_ = atof(argv[c+1]);
        else									   _row_ub  = atoi(argv[c+1]);

			break; case 'l':  
      	if ( argv[c][2] == 'l' ) _clm_lb_ = atof(argv[c+1]);
        else                     _w_lb    = atof(argv[c+1]);

			break; case 'u': 
      	if ( argv[c][2] == 'u' ) _clm_ub_ = atof(argv[c+1]);
        else                     _w_ub    = atof(argv[c+1]);

      break; case 'w': 
      	_wfname = argv[c+1];

      break; case 'W':
      	_item_wfname = argv[c+1];
 
      break; case 'c': 
				_dir = 1; 
				_sep = _root = atoi(argv[c+1]) ;

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
				_multi_core = atoi(argv[c+1]);
          	
      break; case '#': 
      	_max_solutions = atoi(argv[c+1]);

      break; case ',': 
      	_separator = argv[c+1][0];

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
  _frq_lb = atof(argv[c+1]);
  if ( argc>c+2 ) _output_fname = argv[c+2];

	return 0;

}

WEIGHT KGSSPC::comp ( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq)
{
  WEIGHT cc, ccc;
  if ( _problem & SSPC_INTERSECTION ) return (c);
  else if ( _problem & SSPC_INNERPRODUCT ){
    if ( _TT.exist_Tw() ) return (c);
    else return (c / sq / sqrt(wx));
  } 
  else if ( (_problem & SSPC_RESEMBLANCE) && (cc= wi +wx -c) != 0 ) return (c/cc);
  else if ( (_problem & SSPC_MAXINT) && (cc=MAX(wi,wx)) != 0 ) return (c/cc);
  else if ( (_problem & SSPC_MININT) && (cc=MIN(wi,wx)) != 0 ) return (c/cc);
  else if ( (_problem & SSPC_PMI) && (cc=wi*wx) != 0 ) return ( log( ( c * _TT.get_t()) / cc ) /  -log ( c / _TT.get_t()));
  else if ( (_problem & SSPC_FVALUE) && MAX(wi,wx) != 0 ){ cc = c/wi; ccc = c/wx; return ( 2*cc*ccc / (cc+ccc)); }

  return (-WEIGHTHUGE);
}

void KGSSPC::output ( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id){

  size_t b;
	PERM *p = _positPERM;

  if ( _problem & (SSPC_POLISH+SSPC_POLISH2) ){

    _vecchr[i] = 1;

    if ( _problem & SSPC_POLISH2 ){  // store the solution

      SPIN_LOCK (_II.get_multi_core(), _II.get_lock_counter());  // lock!!

      if ( (b = _itemary[_TT.get_clms()]) ){
      	// use deleted cell
        _itemary[_TT.get_clms()] = _buf[b]; 
      }
      else {  
      	// allocate new cell
        b = _buf_end;
        _buf.realloci(b+30);
        _buf_end += 2;
      }

      _buf[b] = _itemary[i];  // insert the cell to list i
      _buf[b+1] = ii;
      _itemary[i] = b;

      SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());  // unlock!!

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
  
    if ( (_problem&PROBLEM_NORMALIZE)&& _dir>0 ){
      if ( i  >= _sep ) itemset->minus_v(0,_root);
      if ( ii >= _sep ) itemset->minus_v(1,_root);
    }

    if ( _II.get_itemtopk_end() > 0 ){
      _II.output_itemset_ ( itemset, frq, frq,  i, ii, core_id);
    }
      _II.output_itemset_ ( itemset, frq, frq,  ii, i, core_id);
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
  PERM *p = _positPERM;

  if ( !_table_fname ){ x_ = _positPERM[x_]; i_ = _positPERM[i_]; }

  if ( (_problem&PROBLEM_NORMALIZE)&& _dir>0 ){
    if ( i_ >= _sep ) i_ -= _root;
    if ( x_ >= _sep ) x_ -= _root;
  }

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

      SPIN_LOCK(_II._multi_core, _II._lock_output);
      fp->print("%d %d\n", x_, i_);
      SPIN_UNLOCK(_II._multi_core, _II._lock_output);

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

  QUEUE jump(_TT.get_clms()), itemset(2,2);

  QUEUE_ID ii=0, t, m;
  QUEUE_INT i=0, i_=0, ff;
  QUEUE_INT *OQend,  *x, *oi, *oj;
  QUEUE_INT cnt = ((_problem&SSPC_COMP_ITSELF) && _dir == 0)?1:0;

  WEIGHT *occ_w,  c, *y;
  WEIGHT yy=0, wi=0, wx=0;


  double sq = 0;

  size_t b, bb;
  int f;

  if ( _problem & SSPC_NO_NEIB ){
  	mark = new char[_TT.get_clms()](); //calloc2
  }
  occ_w = new WEIGHT[_TT.get_clms()](); //calloc2 
  OQend = new QUEUE_INT[_TT.get_clms()](); //calloc2

  while (1){

    if ( i == i_ ){

      i_ = 100000000 / (_TT.get_eles() / _TT.get_clms());
			
      SPIN_LOCK (_II.get_multi_core(), _II.get_lock_counter());  // lock!!

      if ( (i = *(SM->_lock_i)) >= _TT.get_clms() ){
        SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());  // unlock!!
        break;
      }

      i_ = MIN(_TT.get_clms(), i + 100);
      (*(SM->_lock_i)) = i_;

      if ( _iFlag & SHOW_PROGRESS ){
        if ( (int)((i-1)*100/_TT.get_clms()) < (int)(i*100/_TT.get_clms()) )
            fprintf (stderr, "%d%%\n", (int)(i*100/_TT.get_clms()));
      }
      SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());  // unlock!!

    }

    if ( (_problem & SSPC_INNERPRODUCT) && !_TT.exist_Tw() ) sq = sqrt (w[i]);

    if ( _problem & (SSPC_POLISH+SSPC_POLISH2) ) m = _TT.get_clms();
    else m = (_dir>0)?_sep:i;

    jump.setEndByStart(); //jump.set_t(jump.get_s());


    if (_problem & SSPC_NO_NEIB){ // for no_neib
    
			for( x=_TT.beginTv(i) ; x < _TT.endTv(i) ; x++){
				mark[*x] |= 1;
			}

			for( x=_TT.beginOQv(i) ; x<_TT.endOQv(i) ; x++){
				mark[*x] |= 2;
			}

    }

		for(QUEUE_INT * iix = _TT.startOQv(i); iix< _TT.endOQv(i); iix++ ){

      t = *iix;

			// get item weight of current vector
      if ( _TT.exist_Tw() && (_problem & SSPC_INNERPRODUCT)){ 

        y = _TT.beginTw(t);
	      for( x=_TT.beginTv(t); *x < i  ; x++ ){ y++; }
        yy = *y;

      }

      if ( (_problem & SSPC_NO_NEIB) && t == i ) continue;

      if ( _TT.exist_Tw() ){ y = _TT.beginTw(t); }
      else                 { y = 0;}


      for( x =_TT.beginTv(t); *x < m ; x++){

        if ( (_problem & SSPC_POLISH2) && *x < i) continue;

        if ( (_problem & SSPC_NO_NEIB) && *x == t ) continue;

        if ( OQend[*x] == 0 ){
          jump.push_back(*x);
          occ_w[*x] = 0;
        }
        OQend[*x]++;

        if ( _TT.exist_Tw() ){
          if (_problem & SSPC_INNERPRODUCT){
            occ_w[*x] += (*y) * yy; 
          }
          else { 
          	occ_w[*x] += *y;
          }
          y++;
        }
        else {
          occ_w[*x] += _TT.get_w(t); 
        }
      }
    }

		for(x=jump.begin(); x < jump.end(); (x)++){

      if ( *x == i ) goto SKIP;

      c = occ_w[*x];

      if ( _tFlag & LOAD_SIZSORT ){
        for (oi=o[i],oj=o[*x] ; *oi<_II.get_len_lb() ; oi++ ){
          while ( *oj < *oi ) oj++;
          if ( *oi == *oj ) c += _TT.get_w(*oi);
        }
      }

      wi = w[i];
      wx = w[*x];

      if (_problem & SSPC_NO_NEIB){  // for no_neib
        if ( mark[*x]&1 ) wi -= _TT.get_w(*x);
        if ( mark[i] &2 ) wx -= _TT.get_w(i);
      }

      comp2 ( i, *x, c, wi, wx, sq, &cnt, fp, &itemset, core_id);

      SKIP:;
      OQend[*x] = 0;

    }

		// selfcomparison
    if ((_problem & SSPC_COMP_ITSELF) && _dir == 0) {
        output (
        	&cnt, i, i, &itemset, 
        	((_problem&PROBLEM_NORMALIZE)&& _dir>0)? i- _sep: i, //_dir>0ありえない
        	core_id
        );
        
    }

		// data polish;  clear OQ, and marks
    if ( _problem & (SSPC_POLISH+SSPC_POLISH2) ){  
      if ( _problem & SSPC_POLISH2 )
          for (b=_itemary[i] ; b ; b=_buf[b]) _vecchr[_buf[b+1]] = 1;

      f = 0;

      for(QUEUE_INT *iq = _TT.startOQv(i) ; iq < _TT.endOQv(i) ; iq++ ){

        ff = 0;

				for( x=_TT.beginTv(*iq); *x < _TT.get_clms() ; x++){
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
          _buf[b] = _itemary[_TT.get_clms()];
          _itemary[_TT.get_clms()] = b;
        }

      }
    }

    if (_problem & SSPC_NO_NEIB){ // for no_neib
    	_TT.clrMark_T(i,mark); // これなに？
	   	_TT.clrMark_Q(i,mark);
    }

		_TT.clrOQend(i);

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

  mfree (mark, occ_w,OQend);

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


  QUEUE_ID i; 
  QUEUE_ID begin = (_problem&(SSPC_POLISH+SSPC_POLISH2))?0:(_dir>0?_sep:0);
  QUEUE_INT **o=NULL;
  WEIGHT *w;
  int cnt;

	_TT.initialQueSet();
      
  if ( _output_fname2 ) fp.open(_output_fname2);

  // initialization
  w = new WEIGHT[_TT.get_clms()*2](); //calloc2
  
  if(_problem&SSPC_INNERPRODUCT){

  	if(!_TT.exist_Tw()){ _TT.multipule_w(); }//ここなに
	  _TT.delivery( w, w+_TT.get_clms(), NULL, _TT.get_clms());
	  if(_TT.exist_Tw() ){ _TT.normalize(w); }//ここなに // normalize the vectors for inner product (c + -W の指定時
  }
  else{
	  _TT.delivery( w, w+_TT.get_clms(), NULL, _TT.get_clms());
  }
    
	_TT.setOQend();

	o = _TT.skipLaegeFreqItems(_len_lb);

	// selfcomparison
  if ((_problem & SSPC_COMP_ITSELF) && _dir == 0){ 
 		QUEUE itemset(2,2);
		output( &cnt, 0, 0, &itemset, 0, 0);
    itemset.clear();
  }

  // for multi-core
	SM = new SSPC_MULTI_CORE[_II.get_multi_core()]; //malloc2

	for (i=_II.get_multi_core(); (i--) > 0 ; ){
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
	if ( _tFlag & LOAD_SIZSORT ){
		for(int i=1 ; i < _TT.get_clms(); i++){
    	_TT.sizSort(i,o);
    }
  }

  delete [] w;
  delete [] o;
	delete SM;

}

void KGSSPC::_preALLOC(){

	QUEUE_ID siz  = _TT.get_clms();
	QUEUE_ID siz2 = _TT.get_t();

	_occ_w = new WEIGHT[siz+2](); // calloc2
	_vecchr = new char[siz2+2](); // calloc2

	if(_problem&SSPC_POLISH2) {
		_itemary = new QUEUE_INT[siz+2](); //calloc2
	}

	PERM *perm = _TT.convPerm(_outperm_fname);

	_II.alloc(_output_fname, perm, siz, 0);

	_TT.set_perm(NULL); // free対策？
		
	_TT.reallocW();

	return;

}

/*************************************************************************/
/* main function of SSPC */
/*************************************************************************/
int KGSSPC::run (int argc ,char* argv[]){

	if( setArgs(argc, argv) ) return 1;

	_tFlag  |= LOAD_INCSORT;

 	if ( _len_ub < INTHUGE || _len_lb > 0 ){	
 		_tFlag |= (LOAD_SIZSORT+LOAD_DECROWSORT);
 	}

  if( 
  	_TT.load(
			_tFlag, _fname,_wfname,_item_wfname,_fname2,
			_w_lb,_w_ub,_clm_lb_,_clm_ub_,
			_row_lb,_row_ub,_row_lb_,_row_ub_)
	){ 
		return 1;
	}

	// _sep -cの指定がある時に利用される ここでOK？もとはTT.alloc
	_sep = _TT.adjust_sep(_sep); // 

  if ( _len_ub < INTHUGE ){ _len_lb = _TT.addjust_lenlb(_len_ub,_len_lb); }

  if ( _itemtopk_item > 0 ){  _itemtopk_end= _TT.get_clms(); }

	_II.setParams(
		_iFlag,_frq_lb,_len_ub,_len_lb,
		_topk_k,
		_itemtopk_item,_itemtopk_item2,_itemtopk_end,
		_multi_core,_max_solutions,_separator
	);

	_preALLOC();
	
	_TT.printMes("separated at "VEC_IDF"\n", _sep);

  _buf_end = 2;
  _positPERM = _II.get_perm();
  _II.set_perm(NULL);

  if ( _TT.get_clms()>1 ){  _SspcCore(); }

  _II.set_perm(_positPERM);

  _II.merge_counters();

  if ( _II.get_topk_end() > 0 || _II.get_itemtopk_end ()> 0 ){
  	
  	 _II.last_output();
  }
  else{
  	 _TT.printMes( LONGF " pairs are found\n", _II.get_sc(2));
  }
	_II.close();

  return 0;

}

vector<LONG> KGSSPC::mrun(int argc ,char* argv[]){
	KGSSPC mod;
	mod.run(argc,argv);
	return mod.iparam();
}


/*
//   if ( strchr (argv[c], 'M') ) _problem |= SSPC_MATRIX; (SSPCmat)
//   あとで実装
      break; case 'P': 
      	_table_fname = argv[c+1];
        if ( _tFlag & LOAD_TPOSE ) _tFlag -= LOAD_TPOSE; 
        else                       _tFlag |= LOAD_TPOSE;

    // HELPにもなにので後回し
		if ( _table_fname ){ list_comp(); }
    else if(_problem&SSPC_MATRIX){ SSPCmat();  }// HELPにもなにので後回し
    else {
    }
    //

// unify & SIZSORT & itemweigts are not yer implemented  
// not yet checked -L,-U option, -l.-u, -i,-I options 
void KGSSPC::list_comp(){

  LONG i, j;
  QUEUE_INT *x, *y, *yy;
  WEIGHT *xw=0, *yw=0, c, *w, wx, wi;
  FILE2 fp;
  OFILE2 fp2;
  
  int cnt=0;

	// for outputting usual numbers in ITEMSET_output_itemset, 
	// that are not items (o.w., will be permuted by II->perm)
  PERM *p = _II.get_perm();
  _II.set_perm(NULL); 


  if ( _output_fname2 ) fp2.open(_output_fname2);

  _II.set_itemset_t(2);

  w = new WEIGHT[_TT.get_t()]();  //calloc2

  if ( (_problem & SSPC_INNERPRODUCT) && !_TT.exist_Tw() ){
		_TT.multipule_w();
  }

  for(i=0; i < _TT.get_t() ;i++ ){
    w[i] = 0;
    if ( (_problem & SSPC_INNERPRODUCT) && _TT.exist_Tw()){

			for ( WEIGHT * yw =_TT.beginTw(i) ; yw < _TT.endTw(i) ; yw++){
				w[i] += (*yw)*(*yw);
			}
      w[i] = sqrt (w[i]);
			for ( WEIGHT * yw =_TT.beginTw(i) ; yw < _TT.endTw(i) ; yw++){
				(*yw) /= w[i];
			}

    } else {

			for(x=_TT.beginTv(i) ; x < _TT.endTv(i) ; x++){
				w[i] += _TT.get_w(*x);		
			}

    }
  }

	fp.open( _table_fname, "r");
  do {

    if ( fp.read_pair ( &i, &j, &c, 0) ) continue;

    y  = _TT.beginTv(j);
    yy = _TT.endTv(j); 

    c = 0; wi = w[i], wx = w[j];

    if ( _TT.exist_Tw() ){ 
    	xw = _TT.beginTw(i); 
    	yw = _TT.beginTw(j); 
    }

    if (_problem & SSPC_NO_NEIB){

      //MQUE_FLOOP_CLS (_TT.get_Tv(i), x){
			for(x=_TT.beginTv(i);x<_TT.endTv(i) ; x++){

        if ( *x == j ) {
        	wi -= (_problem & SSPC_INNERPRODUCT)?  (*xw) * (*xw): _TT.get_w(*x);
        }
        while (*y < *x){
          if ( *y == i ) {
	          wx -= (_problem & SSPC_INNERPRODUCT)?  (*yw) * (*yw): _TT.get_w(*y);
	        }
          y++; yw++;
          if ( y == yy ) goto END2;
        }
        if ( *x == *y && *x != j && *y != i ){  // for usual
          if ( _TT.exist_Tw() ){
            if (_problem & SSPC_INNERPRODUCT){
              c += (*xw) * (*yw);
            } else c += *yw;
          } else c += _TT.get_w(*x);
        }
        xw++;
        END2:;
      }
      while (y != yy){
        if ( *y == i ) wx -= (_problem & SSPC_INNERPRODUCT)?  (*yw) * (*yw): _TT.get_w(*y);
        y++; yw++;
      }
    } else {

			for(x=_TT.beginTv(i);x<_TT.endTv(i) ; x++){

        while (*y < *x){
          y++; yw++;
          if ( y == yy ) goto END;
        }

        if ( *x == *y ){  
          if ( _TT.exist_Tw() ){
            if (_problem & SSPC_INNERPRODUCT){
              c += (*xw) * (*yw);
            } else c += *yw;
          } else c += _TT.get_w(*x);
        }
      }
      xw++;
      END:;
    }
    comp2( i, j, c, wi, wx, sqrt(w[i]), &cnt, &fp2, _II.getp_itemset(), 0);

  } while ( fp.NotEof() );
  
  _II.set_perm(p);

	delete [] w;

}

*/