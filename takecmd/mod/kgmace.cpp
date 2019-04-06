/* MACE: MAximal Clique Enumerater */
/* ver 1.0 1/Sep/2005 Takeaki Uno   e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, do not forget to 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about this code for the users.
   For the commercial use, please make a contact to Takeaki Uno. */


#define WEIGHT_DOUBLE

#include"kgmace.hpp"
#include"sgraph.hpp"
#include"problem.hpp"
#include"stdlib2.hpp"



void KGMACE::help(){

  print_err ("mace MCqVe [options] input-filename [output-filename]\n\
%%:show progress, _:no message, +:write solutions in append mode\n\
C:enumerate cliques, M:enumerate maximal cliques, e:edge_list format\n\
[options]\n\
-l [num]:output cliques with size at least [num]\n\
-u [num]:output cliques with size at most [num]\n\
-# [num]:stop after outputting [num] solutions\n\
-, [char]:give the separator of the numbers in the output\n\
-Q [filename]:replace the output numbers according to the permutation table given by [filename]\n\
if the 1st letter of input-filename is '-', be considered as 'parameter list'\n");
  EXIT;
}


/***************************************************/
/*  read parameters from command line              */
/***************************************************/
int KGMACE::setArgs (int argc, char *argv[]){

  int c=1;
  if ( argc < c+2 ){ help(); return 1; }

  if ( !strchr (argv[c], '_') ){ _iFlag |= SHOW_MESSAGE; _sgFlag |= SHOW_MESSAGE; }
  if ( strchr (argv[c], '%') ) _iFlag |= SHOW_PROGRESS;
  if ( strchr (argv[c], '+') ) _iFlag |= ITEMSET_APPEND;

  if ( strchr (argv[c], 'M') ) _problem = PROBLEM_MAXIMAL;
  else if ( strchr (argv[c], 'C') ) _problem = PROBLEM_FREQSET;
  else error ("M or C command has to be specified", EXIT);

  if ( strchr (argv[c], 'e') ) _sgFlag |= LOAD_ELE;

  c++;

  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){

      case 'l': 
      	_lb = atoi (argv[c+1]) ;

      break; case 'u': 
      	_ub = atoi(argv[c+1]);

      break; case '#': 
      	_max_solutions = atoi(argv[c+1]);

      break; case ',': 
      	_separator = argv[c+1][0] ;

      break; case 'Q': 
      
      	_outperm_fname = argv[c+1];

      break; default: goto NEXT;
    }
    c += 2;
    if ( argc < c+1 ){ help(); return 1; }
  }

  NEXT:;

  _sgfname = argv[c];
  if ( argc>c+1 ) _output_fname = argv[c+1];

  return 0;
}

/******************************************************************/
/* iteration of clique enumeration   */
/******************************************************************/
void KGMACE::clq_iter ( QUEUE_INT v, QUEUE *occ){

  QUEUE_INT *x, *xx, *y;

 	_II.itemINS(v);
  _II.output_itemset ((QUEUE*) NULL, 0);

  if ( _II.get_itemset_t() >= _II.get_ub() ) goto END;  // upper bound of clique


	for ( y = occ->get_v() ; *y < v ; y++){

    xx = _SG.edge_vv(*y);
    
    for ( x = occ->get_v(); *x < _SG.edge_t() ; x++ ){

      while ( *x > *xx ) xx++;
      if ( *x == *xx ) _OQ[*y].push_back(*x);
    }

    _OQ[*y].push_back( _SG.edge_t());

    clq_iter ( *y, &_OQ[*y]);

    _OQ[*y].set_t (0);

  }
  END:;

  _II.item_dec_t();

}

/******************************************************************/
/* bitmap routines   */
/******************************************************************/
void KGMACE::VBM_set_vertex (QUEUE_INT v){
  QUEUE_INT *x;
  VBMINT p;
  _VV._pos[v] = _VV._dellist.pop_back();
  p = _VV._set[_VV._pos[v]];
  MLOOP (x, _SG.edge_vv(v), _SG.edge_t()) _VV._edge[*x] |= p;
}

void KGMACE::VBM_reset_vertex (QUEUE_INT v){
  QUEUE_INT *x;
  VBMINT p;
  _VV._dellist.push_back( _VV._pos[v]);
  p = _VV._reset[_VV._pos[v]];
  MLOOP (x, _SG.edge_vv(v), _SG.edge_t()) _VV._edge[*x] &= p;
}
void KGMACE::VBM_set_diff_vertexes ( QUEUE *K1, QUEUE *K2){
  QUEUE_INT *x, *y = K2->get_v();

  //MQUE_FLOOP_CLS (*K1, x){
  for(x =K1->begin(); x < K1->end() ; x++  ){
    if ( *x == *y ) y++;
    else VBM_set_vertex (*x);
  }
}
void KGMACE::VBM_reset_diff_vertexes (QUEUE *K1, QUEUE *K2){
  QUEUE_INT *x, *y = K2->get_v();

  //MQUE_FLOOP_CLS (*K1, x){
  for(x =K1->begin(); x < K1->end() ; x++  ){
    if ( *x == *y ) y = y-K2->get_v()<K2->get_t()-1? y+1: y; 
    else VBM_reset_vertex (*x);
  }
}


/******************************************************************/
/* add a vertex v to clique K */
/******************************************************************/
void KGMACE::add_vertex (QUEUE *K, QUEUE_INT v){
  K->push_back(v);
  if ( _problem & PROBLEM_CLOSED ){
    if ( K->get_t() > VBMINT_MAX ) _problem -= PROBLEM_CLOSED;
    else VBM_set_vertex (v);
  }
}

/******************************************************************/
/* add a vertex v to clique K */
/******************************************************************/
void KGMACE::scan_vertex_list ( QUEUE_INT v, QUEUE_INT w){
  QUEUE_INT *xx;

  // MQUE_MLOOP_CLS (_SG.edge_vv(v), xx, w){
  for(xx=_SG.edge_vv(v) ; *xx < w ; xx++ ){
    if ( _OQ[*xx].get_t() == 0 ) _itemcand.push_back(*xx);
    _OQ[*xx].push_back(v); 
  }
}

/* K := lex. muximum maximal clique including K (w.r.t. vertices <w ) */
/* MACE_occ[v] := N(v) \cap K */
void KGMACE::extend (QUEUE *K, QUEUE_INT w){

  QUEUE_INT *x, v;

	for(x = K->begin() ; x < K->end() ;x++  ){ 
		scan_vertex_list (*x, w);
	}
  v = K->get_v(0);

  //MQUE_MLOOP_CLS(_SG._edge._v[v], x, w);
	for(x=_SG.edge_vv(v); *x < w ;x++){;}

         // x := position of vertex w in the list Q[v(= head of K)] 
  for (x-- ; x>=_SG.edge_vv(v) ; x--){
    if ( _OQ[*x].get_t() == K->get_t() ){
       scan_vertex_list ( *x, *x);
       add_vertex ( K, *x);
    }
  }
}

/****************************************************************/
/* check the maximality of K\cap N(w) (=MACE_occ[w]),
   and whether the parent of C(K\cap N(w)) = K or not. */
/****************************************************************/
//LONG MACE::parent_check ( QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w){
//ad => II._add Q =>_OQ
LONG KGMACE::parent_check ( QUEUE *K, QUEUE_INT w){


  QUEUE_INT j=0, e, i, flag =1;
//  QUEUE_INT v=Q[w].get_v(0), *y = _SG._edge._v[w].get_v() + _SG._edge._v[w].get_t()-1, *x, *zz=Q[w].get_v(), *Z;

  QUEUE_INT v=_OQ[w].get_v(0), *y = _SG.edge_vv(w) + _SG.edge_vt(w)-1, *x, *zz=_OQ[w].get_v(), *Z;

	 _II.iadd_set_t(0);
  //ad->set_t(0);
  K->set_v(K->get_t(), -1);  // loop stopper

  FLOOP (i, 0, _OQ[w].get_t()){
    e = _OQ[w].get_v(i);
     // pointers to the positions of the current processing items in each transaction
    _shift[i] = _SG.edge_vv(e) + _SG.edge_vt(e)-1;
  }

  for (x=_SG.edge_vv(v) + _SG.edge_vt(v)-1 ; *x>w ; x--){
    if ( *x <= (e=K->get_v(j)) ){ // skip if *x \in K (or w<*x)
      if ( zz-_OQ[w].get_v() < _OQ[w].get_t() && *zz == e ) zz++;   // "zz-Q[w].v < Q[w].t &&" is added, 2014/1/10 
      else {  // insert *x to Queue "ad" if *x is not in K\cap N(w)
        _shift[_OQ[w].get_t() + _II.iadd_get_t()] = _SG.edge_vv(e) + _SG.edge_vt(e)-1-j;
        //ad->INS(e);
        _II.iaddINS(e);
      }
      if ( *x < e ) x++;
      j++;
      continue;
    }
    i = 0;
    while (1){   // check *x is adjacent to all vertices in K\cap N(w) or not, one-by-one. if not, then break the loop
      while ( *_shift[i]>*x ) _shift[i]--;
      if ( *_shift[i] < *x ) goto LOOP_END;
      i++;
      if ( i== _OQ[w].get_t() ){ // if *x is adjacent to all 
        if ( y<_SG.edge_vv(w) ) goto NEXT;
        while ( *y>*x ){
          if ( --y < _SG.edge_vv(w) ) goto NEXT;
        }
        if ( *y==*x ) return (*x);  //if *x is adjacent to w, then not maximal
        break;
      }
    }
    NEXT:;
    while (flag){   // check *x is adjacent to all vertices in K_{\le w}. If not, then break the loop
      if ( i== _OQ[w].get_t() + _II.iadd_get_t() ) return (*x); // if *x is adjacent to all, MACE_occ[w] is not a child
      Z = _SG.edge_vv( _II.iadd_get_v(i-_OQ[w].get_t()) );
      while ( *_shift[i]>*x ){
        _shift[i]--;
        if ( _shift[i] < Z ){ flag = 0; goto LOOP_END; } // reached to the end of the adjacency list of the i-th added vertex, thus no further vertex can be pass this check, and set flag to 0 not to come here again.
      }
      if ( *_shift[i] < *x ) goto LOOP_END;
      i++;
    }
    LOOP_END:;
  }
  return (-1);
}

/****************************************************************/
/* check the maximality of K\cap N(w) (=MACE_occ[w]),
   ad whether the parent of C(K\cap N(w)) = K or not. */
/*  BITMAP version */
/****************************************************************/
LONG KGMACE::VBM_parent_check ( QUEUE *K, QUEUE *Q, QUEUE_INT w){

  QUEUE_INT v=Q[w].get_v(0);
  QUEUE_ID i;
  VBMINT p=0, pp;

  QUEUE_INT *y = _SG.edge_vv(w) + _SG.edge_vt(w)-1, *x, *z=K->get_v();

  K->set_v(K->get_t(), -1);  // loop stopper

  FLOOP (i, 0, Q[w].get_t()) {
  	p |= _VV._set[_VV._pos[Q[w].get_v(i)]];
  }

  pp = p;

  for (x=_SG.edge_vv(v) + _SG.edge_vt(v)-1 ; *x>w ; x--){
    while ( *x < *z ){ 
    	pp |= _VV._set[_VV._pos[*z]]; 
    	z++; 
    }

    if ( *x == *z ){ pp |= _VV._set[_VV._pos[*z]]; z++; continue; }
    if ( pp==(pp&_VV._edge[*x]) ) return (*x);  // parentness

		// maximality w.r.t P\cap N(w) (=occ[w]) 
    if ( p == (p & _VV._edge[*x]) ){ 

      if ( y<_SG.edge_vv(w) ) goto NEXT;

      // check *x is incident to w?  
      while ( *x < *y ){  
        y--;
        if ( y<_SG.edge_vv(w) ) goto NEXT;
      }
      // if *x is incident to w, parent is different.
      if ( *x==*y ) return (*x); 
    }
    NEXT:;
  }
  return (-1);
}

/*************************************************************************/
/*  simple routine for checking the maximality of a clique,
     and parent-child relation, for debugging */
/*************************************************************************/
LONG KGMACE::parent_check_max ( QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w){
  QUEUE_INT *x;
	
	ad->cpy( _SG.getp_v(w) );

	for( x = Q[w].begin() ; x < Q[w].end() ; x++){
  	 ad->and_(_SG.getp_v(w));
  }

  if ( ad->get_t()==0 ) return (-1);
  if ( ad->get_v(ad->get_t()-1) > w ) return (ad->get_v(ad->get_t()-1));

  return (-1);

}

LONG KGMACE::parent_check_parent (QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w){

  QUEUE_INT t=0, *x, i;

  K->set_v(K->get_t(),-1); // loop stopper;

  ad->cpy ( _SG.getp_v(Q[w].get_v(0)) );

	for(x = Q[w].begin(); x < Q[w].end() ; x++){
  	ad->and_(_SG.getp_v(*x));
  }

  while ( ad->get_t() > 0 ){

    i = ad->pop_back();
    if ( i<w ) return (-1);

    while ( i<K->get_v(t) ) t++;

    if ( i > K->get_v(t) ) return (i);

    ad->and_( _SG.getp_v(i) );
  }
  return (-1);
}


/*************************************************************************/
/* MACE main iteration */
/*************************************************************************/
void KGMACE::iter (int v){

  LONG ii;
  QUEUE_INT u;
  QUEUE_ID js = _itemcand.get_s();
  QUEUE *Q = _OQ;


  _II.inc_iters();
  _itemcand.set_s(_itemcand.get_t());

  add_vertex( &Q[v], v);

  extend ( &Q[v], v);

  _II.set_itemset_t(0);

  memcpy (_II.item_get_v(), Q[v].get_v(), sizeof(QUEUE_INT)*Q[v].get_t());

  _II.item_set_t(Q[v].get_t());
  _II.output_itemset ( (QUEUE*)NULL, 0);

  qsort_<QUEUE_INT>(_itemcand.getp_v(_itemcand.get_s()), _itemcand.get_t()-_itemcand.get_s(), -1);

  while ( _itemcand.get_t() > _itemcand.get_s() ){
    u = _itemcand.pop_back();
    if ( u == Q[v].tail() ){
      Q[v].dec_t();
      if ( _problem & PROBLEM_CLOSED ) VBM_reset_vertex (u);
    } 
    else {
      if ( _problem & PROBLEM_CLOSED ) ii = VBM_parent_check (&Q[v],_OQ, u);
      else ii = parent_check ( &Q[v], u);

      if ( ii==-1 ){
        if (_problem & PROBLEM_CLOSED) VBM_reset_diff_vertexes ( &Q[v], &Q[u]);
        iter(u); // recursive call for a child
        if (_problem & PROBLEM_CLOSED) VBM_set_diff_vertexes (&Q[v], &Q[u]);
      }
    }
    Q[u].set_t (0);
  }
  _itemcand.set_s(js);
  if ( _problem & PROBLEM_CLOSED ) VBM_reset_vertex (v);

}

/* MACE main */
void KGMACE::MACECORE (){

  QUEUE_INT v;

	for( QUEUE_INT v=0 ; v < _SG.edge_t() ;v++ ){

    if ( _SG.edge_vt(v)==0 ){
      _II.item_set_t(0);
      _II.itemINS(v);
      _II.output_itemset((QUEUE*) NULL, 0);
    } 
    //else if ( E[v].get_v(E[v].get_t()-1) <= v ){
		else if ( _SG.edge_Lastvv(v) <= v ){    
      iter (v);
    }
    _OQ[v].set_t(0);
  }
}




int KGMACE::run (int argc, char *argv[]){
  
  QUEUE_INT v;
  int flag = 0;
	

	if( setArgs (argc, argv) ) { return (1); }

	// load base prerun
  _sgFlag |= LOAD_INCSORT + LOAD_RM_DUP + LOAD_EDGE;
  
  
	// _sgFlag
	// =>  LOAD_INCSORT + LOAD_RM_DUP + LOAD_EDGE 
	//   or  LOAD_INCSORT + LOAD_RM_DUP + LOAD_EDGE + LOAD_ELE;
  if ( _SG.loadEDGE(_sgFlag,_sgfname) ) { return 1; }



	// _iFlag ＝＞ messege関係 + ITEMSET_ADD
	
  _iFlag  |= ITEMSET_ADD;

	_II.setParams( 
		_iFlag,_lb,_ub,_max_solutions ,_separator
	);

	preALLOC();

	// flag for the use of VBM
  if ( _problem & PROBLEM_MAXIMAL ){
    _problem |= PROBLEM_CLOSED;  
    flag = 1;
  }

  _SG.rm_selfloop();
	_SG.edgeSetEnd();

	int i;
  VBMINT p;
	
	
	_OQ = new QUEUE[_SG.edge_t()+1];
	char *cmn_pnt = _SG.initOQ( _OQ );
	
  if ( _problem & PROBLEM_CLOSED ){ // _VVをクラス化
  }

	// ここまで prerun
	if( _ERROR_MES || _SG.edge_eles() <= 0 ){
	  return (_ERROR_MES?1:0);
	}
	


  if ( _problem & PROBLEM_FREQSET ){
    FLOOP (v, 0, _SG.edge_t()){
			clq_iter ( v, _SG.getp_v(v) );
    }
  } 
  else{
  	_VV.alloc(_SG.edge_t());
    MACECORE();
  }
  _II.last_output();


  if ( flag ){
    free2 (_VV._edge);
    free2 (_VV._pos);
    free2 (_VV._set);
    free2 (_VV._reset);
    //_VV._dellist.end();
  }
	  return (_ERROR_MES?1:0);

}

int KGMACE::mrun(int argc ,char* argv[]){
	return KGMACE().run(argc,argv);
}
