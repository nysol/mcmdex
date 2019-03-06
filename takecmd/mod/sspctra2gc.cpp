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

#define WEIGHT_DOUBLE

#include "sspctra2gc.hpp"

void SSPC_tra2gc::_error(void){

  _ERROR_MES = "command explanation";

  print_err ("SSPC: [ISCfQq] [options] input-filename ratio/threshold [output-filename]\n\
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
void SSPC_tra2gc::read_param(int argc, char *argv[]){

  int c=1;
  
  _th = 0; 
  _th2 = 0;
  _II.set_multi_core(1);

  if ( argc < c+3 ){ _error(); return; }

  if ( !strchr (argv[c], '_') ){ _II.union_flag(SHOW_MESSAGE); _TT.union_flag(SHOW_MESSAGE); }

  if ( strchr (argv[c], 'f') ) _II.union_flag(ITEMSET_FREQ);

  if ( strchr (argv[c], 'i') ) _problem = SSPC_INCLUSION;
  else if ( strchr (argv[c], 'R') ) _problem = SSPC_RESEMBLANCE;
  else if ( strchr (argv[c], 'P') ) _problem = SSPC_PMI;
  else error ("i, I, s, S, R, T or C command has to be specified", EXIT);

  if ( !strchr (argv[c], 't') ) _TT.union_flag(LOAD_TPOSE);
  c++;
  
  while ( argv[c][0] == '-' ){
  	if (argv[c][1]=='T'&&argv[c][1]=='T'){
      _th = atoi(argv[c+1]);
      _problem |= SSPC_OUTPUT_INTERSECT;
  	}
  	else{
  		goto NEXT;
  	}
    c += 2;
    if ( argc<c+2 ){ _error(); return; }
  }

  NEXT:;
	_TT.set_fname(argv[c]);
  _II.set_frq_lb( atof(argv[c+1]));
  if ( argc>c+2 ) _output_fname = argv[c+2];

}



typedef struct {

  QUEUE_INT **_o;
  WEIGHT *_w;
  FILE *_fp;
  int _core_id;
  int *_lock_i;
} SSPC_MULTI_CORE;


WEIGHT SSPC_tra2gc::comp ( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq)
{
  WEIGHT cc, ccc;
  if ( (_problem & SSPC_RESEMBLANCE) && (cc= wi +wx -c) != 0 ){
  	return (c/cc);
  }
  else if ( (_problem & SSPC_PMI) && (cc=wi*wx) != 0 ) {
  	return ( log( ( c * _TT.get_t()) / cc ) /  -log ( c / _TT.get_t()));
  }
  else if ( (_problem & SSPC_FVALUE) && MAX(wi,wx) != 0 ){ 
  	cc = c/wi; ccc = c/wx; 
  	return ( 2*cc*ccc / (cc+ccc)); 
	}
  return (-WEIGHTHUGE);
}

void SSPC_tra2gc::output ( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id){

  size_t b;
  PERM *p = (PERM *)_position_fname;

    if ( _problem & SSPC_OUTPUT_INTERSECT ){
      _II.getp_multi_fp(core_id)->print_int ( _siz, 0);
      _II.getp_multi_fp(core_id)->putc( ' ');
    }
    itemset->set_v(0, p[i]); itemset->set_v(1,p[ii]);

    if ( _II.get_itemtopk_end() > 0 ){
      _II.output_itemset_ ( itemset, frq, frq, NULL, i, ii, core_id);
    }
      _II.output_itemset_ ( itemset, frq, frq, NULL, ii, i, core_id);
}

/* output a pair if it is in the similarity */

void SSPC_tra2gc::comp2 (QUEUE_ID i, QUEUE_ID x, WEIGHT c, WEIGHT wi, WEIGHT wx, double sq, QUEUE_INT *cnt, FILE *fp, QUEUE *itemset, int core_id){

  WEIGHT f1, f2, f1_=0, f2_=0, frq;
  // ITEMSET *II = &PP->_II;
  int f=0, f_=0;
  QUEUE_ID i_ = i, x_ = x;
  PERM *p = (PERM *)_position_fname;

	x_ = p[x_]; i_ = p[i_];

  if ( c < _th ) return;  // threshold for the intersection size 
  _siz = c; // outputting intersection size

  if ( _problem & (SSPC_INTERSECTION +SSPC_RESEMBLANCE +SSPC_INNERPRODUCT +SSPC_MAXINT +SSPC_MININT + SSPC_PMI + SSPC_FVALUE) ){
    frq = comp (c, wi, wx, sq);

    if ( frq == -WEIGHTHUGE ) return;
    if ( frq >= _II.get_frq_lb() ) output ( cnt, x, i, itemset, frq, core_id);

  }
  else {

    f1 = wi*_II.get_frq_lb(); f2 = wx*_II.get_frq_lb();  // size of i and x

		if ( _problem & SSPC_INCLUSION ){
      if ( c >= f2 ){
        frq = c / wx;
        output ( cnt, x, i, itemset, frq, core_id);
      }
      f = ( c >= f1 );
      frq = c / wi;
    } else return;
    if ( f ) output ( cnt, i, x, itemset, frq, core_id);
  }
}

/* initialization, for delivery */
WEIGHT *SSPC_tra2gc::init1 ( FILE **fp){
  WEIGHT *w, *y;
  QUEUE_ID i;
  QUEUE_INT *x;

  // initialization
  calloc2 (w, _TT.get_clms()*2, EXIT);


  _TT.delivery( w, w+_TT.get_clms(), NULL, _TT.get_clms());

  FLOOP (i, 0, _TT.get_clms()) _TT.set_OQ_end(i,0);
  return (w);
}

/* initialization */
QUEUE_INT **SSPC_tra2gc::init2 (){

  QUEUE_ID i, j;
  QUEUE_INT **o=NULL;

  // skipping items of large frequencies
  if ( _TT.get_flag() & LOAD_SIZSORT ){
    malloc2 (o, _TT.get_clms(), EXIT);
    FLOOP (i, 0, _TT.get_clms()){
      o[i] = _TT.get_OQ_v(i);
      
      _TT.set_OQ_v(i,_TT.get_OQ_t(i), INTHUGE);  // put end-mark at the last; also used in main loop
      for (j=0 ; _TT.get_OQ_v(i,j) < _II.get_len_lb() ; j++); //skip?
      _TT.move_OQ(i,j);
    }
  }
  return (o);
}

/* iteration for muticore mode */
void *SSPC_tra2gc::iter (void *p){

  SSPC_MULTI_CORE *SM = (SSPC_MULTI_CORE *)p;
  int core_id = SM->_core_id;
  QUEUE_INT **o = SM->_o;
  WEIGHT *w = SM->_w;
  FILE *fp = SM->_fp;
  QUEUE jump, itemset;
  char *mark = NULL;

  QUEUE_ID ii=0, t, m;
  QUEUE_INT i=0, i_=0, ff;
  QUEUE_INT *OQend, cnt=(_dir == 0)?1:0, *x, *oi, *oj;
  WEIGHT *occ_w, *occ_pw, c, *y, yy=0, wi=0, wx=0;
  double sq = 0;

  size_t b, bb;
  int f, pf = _TT.get_flag2()&TRSACT_NEGATIVE;  // dealing with only positive weitht, not yet implemented

  calloc2 (occ_w, _TT.get_clms(), EXIT);
  calloc2 (occ_pw, _TT.get_clms(), EXIT);
  calloc2 (OQend, _TT.get_clms(), EXIT);
  jump.alloc(_TT.get_clms());
  itemset.alloc(2);
  itemset.set_t(2);
  
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
      if ( _II.get_flag() & SHOW_PROGRESS ){
        if ( (int)((i-1)*100/_TT.get_clms()) < (int)(i*100/_TT.get_clms()) )
            fprintf (stderr, "%d%%\n", (int)(i*100/_TT.get_clms()));
      }
      SPIN_UNLOCK (_II.get_multi_core(), _II.get_lock_counter());  // unlock!!
    }
		m = (_dir>0)?_TT.get_sep():i;

    jump.cp_s2t();

    FLOOP (ii, _TT.get_OQ_s(i), _TT.get_OQ_t(i)){
      t = _TT.get_OQ_v(i,ii);

      if ( _TT.exist_Tw() ) y = _TT.get_Tw(t); else y = 0;

      MQUE_MLOOP_CLS (_TT.get_Tv(t), x, m){
        if ( OQend[*x] == 0 ){
          jump.INS(*x);
          occ_w[*x] = 0;
          if ( pf ) occ_pw[*x] = 0;
        }
        OQend[*x]++;
        if ( _TT.exist_Tw() ){
          occ_w[*x] += *y; 
          if ( *y>0 && pf) occ_pw[*x] += *y;
          y++;
        } else {
          occ_w[*x] += _TT.get_w(t); if ( pf ) occ_pw[*x] += _TT.get_pw(t);
        }
      }
    }

    MQUE_FLOOP_CLS (jump, x){
      if ( *x == i ) goto SKIP;
      c = occ_w[*x];

      if ( _TT.get_flag() & LOAD_SIZSORT ){
        for (oi=o[i],oj=o[*x] ; *oi<_II.get_len_lb() ; oi++ ){
          while ( *oj < *oi ) oj++;
          if ( *oi == *oj ) c += _TT.get_w(*oi);
        }
      }
      wi = w[i];
      wx = w[*x];

      comp2 ( i, *x, c, wi, wx, sq, &cnt, fp, &itemset, core_id);
      SKIP:;
      OQend[*x] = 0;
    }


    _TT.set_OQ_end(i, 0);
    i++;
  }
  mfree (mark, occ_w, occ_pw, OQend);
	//jump.end();
	//itemset.end();
  return (NULL);
}
/*************************************************************************/
/* SSPC main routine */
/*************************************************************************/
void SSPC_tra2gc::SSPCCORE(){
  SSPC_MULTI_CORE *SM = NULL;
  //ITEMSET *II = &_PP._II;
  //TRSACT *TT = &_PP._TT;
  QUEUE_ID i, begin = _dir>0?_TT.get_sep():0;
  QUEUE_INT **o;
  WEIGHT *w;
  FILE *fp = NULL;  // file pointer for the second output file

  int cnt;
  QUEUE itemset;

    // initialization
  w = init1 (&fp);
  o = init2 ();

  // for multi-core
  malloc2 (SM, _II.get_multi_core(), EXIT);
  BLOOP (i,  _II.get_multi_core(), 0){
   // SM[i]._PP = &_PP;
    SM[i]._o = o;
    SM[i]._w = w;
    SM[i]._fp = fp;
    SM[i]._core_id = i;
    SM[i]._lock_i = &begin;
    iter ((void*)(&SM[i]));
  }
  // termination
  if ( _TT.get_flag() & LOAD_SIZSORT ){
    FLOOP (i, 0, _TT.get_clms()){
	    _TT.sizSort(i,o);
    }
  }
  mfree (w, o, SM);
  fclose2 (fp);
}


/*************************************************************************/
/* main function of SSPC */
/*************************************************************************/
//int SSPC_main (int argc, char *argv[]){
int SSPC_tra2gc::run (int argc ,char* argv[]){

	read_param (argc, argv);

	if ( _ERROR_MES ) return 1;

	_TT.union_flag  (LOAD_INCSORT);
	_TT.union_flag2 (TRSACT_ALLOC_OCC);

  preLOAD();
	preALLOC();

  _TT.set_perm(NULL);

	_TT.realloc_w(MAX(_TT.get_t(), _TT.get_clms())+1);
  ARY_FILL (_TT.get_w(), 0, MAX(_TT.get_t(), _TT.get_clms())+1, 1);

  print_mesf (&_TT, "separated at %d\n", _TT.get_sep());

  _buf_end = 2;
  _position_fname = (char *)_II.get_perm(); _II.set_perm(NULL);

  if ( !ERROR_MES && _TT.get_clms()>1 ){
			VEC_ID i, e;
 			QUEUE_INT *x;

			QUEUE *occ = _TT.getp_OQ(_TT.get_clms());
			QUEUE *OQ = _TT.get_OQ();
			QUEUE *Q  = _TT.get_Tv();
			VEC_ID t =  _TT.get_t();
			QUEUE_INT M = _TT.get_clms() ;
			FLOOP(i, 0, occ? occ->get_t(): t){
   			e = occ? occ->get_v(i): i;
		    MLOOP (x, Q[e].get_v(), M){ OQ[*x].INS(e); }
		  }
      SSPCCORE();
  }
  _II.set_perm ( (PERM *)_position_fname);
  _II.merge_counters ();


  if ( _II.get_topk_end() > 0 || _II.get_itemtopk_end() > 0 ) _II.last_output ();
  else print_mesf (&_TT, LONGF " pairs are found\n", _II.get_sc(2));


  return (_ERROR_MES?1:0);

}
