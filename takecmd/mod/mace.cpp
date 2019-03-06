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

#include"mace.hpp"
#include"sgraph.hpp"
#include"problem.hpp"
#include"stdlib2.hpp"



void MACE::_error (){
  _ERROR_MES = "command explanation";
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
void MACE::read_param (int argc, char *argv[]){

  int c=1;
  if ( argc < c+2 ){ _error (); return; }

  if ( !strchr (argv[c], '_') ){ _II.union_flag( SHOW_MESSAGE); _SG._flag |= SHOW_MESSAGE; }
  if ( strchr (argv[c], '%') ) _II.union_flag(SHOW_PROGRESS);
  if ( strchr (argv[c], '+') ) _II.union_flag(ITEMSET_APPEND);
  if ( strchr (argv[c], 'M') ) _problem = PROBLEM_MAXIMAL;
  else if ( strchr (argv[c], 'C') ) _problem = PROBLEM_FREQSET;
  else error ("M or C command has to be specified", EXIT);
  if ( strchr (argv[c], 'e') ) _SG._flag |= LOAD_ELE;
  c++;

  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){
      case 'l': _II.set_lb(atoi (argv[c+1]));
      break; case 'u': _II.set_ub(atoi(argv[c+1]));
      break; case '#': _II.set_max_solutions(atoi(argv[c+1]));
      break; case ',': _II.set_separator(argv[c+1][0]);
      break; case 'Q': _outperm_fname = argv[c+1];
      break; default: goto NEXT;
    }
    c += 2;
    if ( argc < c+1 ){ _error (); return; }
  }

  NEXT:;
  _SG._fname = argv[c];
  if ( argc>c+1 ) _output_fname = argv[c+1];
}



/* allocate arrays and structures */
void MACE::preALLOC (QUEUE_ID siz, QUEUE_ID siz2, size_t siz3, PERM *perm, int f){


  PERM *p;

  int j;

  if ( f&PROBLEM_SHIFT ) calloc2 (_shift, siz+2, goto ERR);
  if ( f&PROBLEM_OCC_T ) calloc2 (_occ_t, siz+2, goto ERR);

 if ( f&PROBLEM_ITEMCAND ) _itemcand.alloc(siz+2);

    // set outperm
  if ( _outperm_fname ){
    FILE2::ARY_Load (p,  j, _outperm_fname, 1);
    if ( perm ){
      FLOOP (j, 0, siz) perm[j] = p[perm[j]];
      free2 (p);
    } else perm = p;
  }
  _II.alloc(_output_fname, perm, siz, siz3);


  if ( _II.get_target()<siz && _II.exist_perm() )
      FLOOP (j, 0, _II.get_item_max()){ if ( _II.get_target() == _II.get_perm(j) ){ _II.set_target(j); break; } }
  return;

  ERR:;
  //end();
  EXIT;

}

/***************************************************/
/*  initialization                                 */
/***************************************************/
void MACE::preRUN (){

  QUEUE_INT i;
  VBMINT p;

  _II.union_flag(ITEMSET_ADD);
  _SG._flag |= LOAD_INCSORT + LOAD_RM_DUP + LOAD_EDGE;

	preLOAD();
	if (_ERROR_MES) return;
	
  preALLOC (_SG._edge.get_t(), _SG._edge.get_t(), _SG._edge.get_eles(), NULL, PROBLEM_ITEMJUMP + PROBLEM_ITEMCAND + PROBLEM_SHIFT + PROBLEM_OCC_T);

  _SG.rm_selfloop();

  FLOOP (i, 0, _SG._edge.get_t()) _SG._edge._v[i].set_v( _SG._edge._v[i].get_t() , _SG._edge.get_t());

// delivery
  //QUEUE_delivery (NULL, _occ_t, NULL, _SG.edge.v, NULL, _SG._edge.get_t(), _SG._edge.get_t());

	//===================
	VEC_ID iv, ev;
	QUEUE_INT *x;
	for (iv=0 ; iv<_SG._edge.get_t(); iv++){
  	ev =  iv;
  	MLOOP (x, _SG._edge._v[ev].get_v(), _SG._edge.get_t()) _occ_t[*x]++;
	}
	// MQUE_ALLOC(Q,rows,rowt,unit,ext,x)   
  //MQUE_ALLOC (_OQ, _SG._edge.get_t(), _occ_t, 0, 2, EXIT);
  {
		size_t cmn_size_t = 0;
		for(VEC_ID cmm_vecid=0; cmm_vecid < _SG._edge.get_t() ; cmm_vecid++){
			cmn_size_t += _occ_t[cmm_vecid];
		}
		_OQ = new QUEUE[_SG._edge.get_t()+1];
		char *cmn_pnt;
		malloc2 (cmn_pnt,(cmn_size_t+(_SG._edge.get_t()*2)+2)*(sizeof(QUEUE_INT)), {delete(_OQ); EXIT;}); 

		for(VEC_ID cmm_vecid=0; cmm_vecid < _SG._edge.get_t() ; cmm_vecid++){
			_OQ[cmm_vecid].set_end(_occ_t[cmm_vecid]);
			_OQ[cmm_vecid].set_v((QUEUE_ID *)cmn_pnt);
			cmn_pnt += (sizeof(QUEUE_INT)) * (_occ_t[cmm_vecid]+(2));
		}
  }

  if ( _problem & PROBLEM_CLOSED ){
    _VV._edge = _VV._set = _VV._reset = NULL; _VV._pos = NULL; _VV._dellist.set_v(NULL);
    malloc2 (_VV._edge, _SG._edge.get_t(), goto ERR);
    malloc2 (_VV._pos, _SG._edge.get_t(), goto ERR);
    malloc2 (_VV._set, VBMINT_MAX, goto ERR);
    malloc2 (_VV._reset, VBMINT_MAX, goto ERR);
    //QUEUE_alloc (&VV->dellist, VBMINT_MAX+2);
    _VV._dellist.alloc(VBMINT_MAX+2);
		if ( _ERROR_MES ) goto ERR;
    _VV._dellist.set_t(VBMINT_MAX);

    ARY_FILL (_VV._edge, 0, _SG._edge.get_t(), 0);
    for (i=0,p=1 ; i<VBMINT_MAX ; i++,p*=2){
      _VV._set[i] = p;
//        VV->reset[i] = 0xffffffff-p;
      _VV._reset[i] = -1-p;
      _VV._dellist.set_v(i, i);
    }
//      for (i=1,MACEVBM_mark_max=1 ; i<VBMMARK_MAX ; i++ ) MACEVBM_mark_max*=2;
//      malloc2 (MACEVBM_mark, char, MACEVBM_mark_max, "MACE_init:MACEVBM_mark");
//      for (i=0 ; i<MACEVBM_mark_max ; i++) MACEVBM_mark[i] = 0;
  }
  return;
  ERR:;
	_VV._dellist.clear();
  free2 (_VV._edge);
  free2 (_VV._pos);
  free2 (_VV._set);
  free2 (_VV._reset);
}


/******************************************************************/
/* iteration of clique enumeration   */
/******************************************************************/
void MACE::clq_iter ( QUEUE_INT v, QUEUE *occ){
  //SGRAPH *G=&PP->SG;
  //ITEMSET *II=&PP->II;
  QUEUE_INT *x, *xx, *y;

 	_II.itemINS(v);
  _II.output_itemset ( NULL, 0);
  if ( _II.get_itemset_t() >= _II.get_ub() ) goto END;  // upper bound of clique
  
  MLOOP (y, occ->get_v(), v){
    xx = _SG._edge._v[*y].get_v();
    MLOOP (x, occ->get_v(), _SG._edge.get_t()){
      while ( *x > *xx ) xx++;
      if ( *x == *xx ) _OQ[*y].INS(*x);
    }
    _OQ[*y].INS( _SG._edge.get_t());
    clq_iter ( *y, &_OQ[*y]);
    _OQ[*y].set_t (0);
  }
  END:;
  _II.item_dec_t();
}


/******************************************************************/
/******************************************************************/
/******************************************************************/

/******************************************************************/
/* bitmap routines   */
/******************************************************************/
void MACE::VBM_set_vertex (QUEUE_INT v){
  QUEUE_INT *x;
  VBMINT p;
  //VV->pos[v] =QUEUE_ext_tail_ (&VV->dellist);
  _VV._pos[v] = _VV._dellist.ext_tail_();
  p = _VV._set[_VV._pos[v]];
  MLOOP (x, _SG._edge._v[v].get_v(), _SG._edge.get_t()) _VV._edge[*x] |= p;
}

void MACE::VBM_reset_vertex (QUEUE_INT v){
  QUEUE_INT *x;
  VBMINT p;
  _VV._dellist.INS( _VV._pos[v]);
  p = _VV._reset[_VV._pos[v]];
  MLOOP (x, _SG._edge._v[v].get_v(), _SG._edge.get_t()) _VV._edge[*x] &= p;
}
void MACE::VBM_set_diff_vertexes ( QUEUE *K1, QUEUE *K2){
  QUEUE_INT *x, *y = K2->get_v();
  MQUE_FLOOP_CLS (*K1, x){
    if ( *x == *y ) y++;
    else VBM_set_vertex (*x);
  }
}
void MACE::VBM_reset_diff_vertexes (QUEUE *K1, QUEUE *K2){
  QUEUE_INT *x, *y = K2->get_v();
  MQUE_FLOOP_CLS (*K1, x){
    if ( *x == *y ) y = y-K2->get_v()<K2->get_t()-1? y+1: y; 
    else VBM_reset_vertex (*x);
  }
}


/******************************************************************/
/* add a vertex v to clique K */
/******************************************************************/
void MACE::add_vertex (QUEUE *K, QUEUE_INT v){
  K->INS(v);
  if ( _problem & PROBLEM_CLOSED ){
    if ( K->get_t() > VBMINT_MAX ) _problem -= PROBLEM_CLOSED;
    else VBM_set_vertex (v);
  }
}

/******************************************************************/
/* add a vertex v to clique K */
/******************************************************************/
void MACE::scan_vertex_list ( QUEUE_INT v, QUEUE_INT w){
  QUEUE_INT *xx;
  MQUE_MLOOP_CLS (_SG._edge._v[v], xx, w){
    if ( _OQ[*xx].get_t() == 0 ) _itemcand.INS(*xx);
    _OQ[*xx].INS(v); 
  }
}

/* K := lex. muximum maximal clique including K (w.r.t. vertices <w ) */
/* MACE_occ[v] := N(v) \cap K */
void MACE::extend (QUEUE *K, QUEUE_INT w){
  QUEUE_INT *x, v;

  MQUE_FLOOP_CLS(*K, x) scan_vertex_list (*x, w);
  v = K->get_v(0);
  MQUE_MLOOP_CLS(_SG._edge._v[v], x, w);
         // x := position of vertex w in the list Q[v(= head of K)] 
  for (x-- ; x>=_SG._edge._v[v].get_v() ; x--){
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
LONG MACE::parent_check ( QUEUE *K, QUEUE_INT w){


  QUEUE_INT j=0, e, i, flag =1;
//  QUEUE_INT v=Q[w].get_v(0), *y = _SG._edge._v[w].get_v() + _SG._edge._v[w].get_t()-1, *x, *zz=Q[w].get_v(), *Z;

  QUEUE_INT v=_OQ[w].get_v(0), *y = _SG._edge._v[w].get_v() + _SG._edge._v[w].get_t()-1, *x, *zz=_OQ[w].get_v(), *Z;

	 _II.iadd_set_t(0);
  //ad->set_t(0);
  K->set_v(K->get_t(), -1);  // loop stopper

  FLOOP (i, 0, _OQ[w].get_t()){
    e = _OQ[w].get_v(i);
     // pointers to the positions of the current processing items in each transaction
    _shift[i] = _SG._edge._v[e].get_v() + _SG._edge._v[e].get_t()-1;
  }
  for (x=_SG._edge._v[v].get_v() + _SG._edge._v[v].get_t()-1 ; *x>w ; x--){
    if ( *x <= (e=K->get_v(j)) ){ // skip if *x \in K (or w<*x)
      if ( zz-_OQ[w].get_v() < _OQ[w].get_t() && *zz == e ) zz++;   // "zz-Q[w].v < Q[w].t &&" is added, 2014/1/10 
      else {  // insert *x to Queue "ad" if *x is not in K\cap N(w)
        _shift[_OQ[w].get_t() + _II.iadd_get_t()] = _SG._edge._v[e].get_v() + _SG._edge._v[e].get_t()-1-j;
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
        if ( y<_SG._edge._v[w].get_v() ) goto NEXT;
        while ( *y>*x ){
          if ( --y < _SG._edge._v[w].get_v() ) goto NEXT;
        }
        if ( *y==*x ) return (*x);  //if *x is adjacent to w, then not maximal
        break;
      }
    }
    NEXT:;
    while (flag){   // check *x is adjacent to all vertices in K_{\le w}. If not, then break the loop
      if ( i== _OQ[w].get_t() + _II.iadd_get_t() ) return (*x); // if *x is adjacent to all, MACE_occ[w] is not a child
      Z = _SG._edge._v[ _II.iadd_get_v(i-_OQ[w].get_t())].get_v();
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
LONG MACE::VBM_parent_check ( QUEUE *K, QUEUE *Q, QUEUE_INT w){
  QUEUE_INT v=Q[w].get_v(0);
  QUEUE_ID i;
  VBMINT p=0, pp;
  QUEUE_INT *y = _SG._edge._v[w].get_v() + _SG._edge._v[w].get_t()-1, *x, *z=K->get_v();
  K->set_v(K->get_t(), -1);  // loop stopper
  FLOOP (i, 0, Q[w].get_t()) p |= _VV._set[_VV._pos[Q[w].get_v(i)]];
  pp = p;
  for (x=_SG._edge._v[v].get_v() + _SG._edge._v[v].get_t()-1 ; *x>w ; x--){
    while ( *x < *z ){ pp |= _VV._set[_VV._pos[*z]]; z++; }
//    if ( *z>=0 ) pp |= VV->set[VV->pos[*z]];

    if ( *x == *z ){ pp |= _VV._set[_VV._pos[*z]]; z++; continue; }
    if ( pp==(pp&_VV._edge[*x]) ) return (*x);  // parentness

    if ( p == (p & _VV._edge[*x]) ){ // maximality w.r.t P\cap N(w) (=occ[w]) 
      if ( y<_SG._edge._v[w].get_v() ) goto NEXT;
      while ( *x < *y ){  // check *x is incident to w?  
        y--;
        if ( y<_SG._edge._v[w].get_v() ) goto NEXT;
      }
      if ( *x==*y ) return (*x); // if *x is incident to w, parent is different.
    }
    NEXT:;
  }
  return (-1);
}

/*************************************************************************/
/*  simple routine for checking the maximality of a clique,
     and parent-child relation, for debugging */
/*************************************************************************/
LONG MACE::parent_check_max ( QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w){
  QUEUE_INT *x;
  //QUEUE_cpy (ad, &G->edge.v[w]);
  ad->cpy(&_SG._edge._v[w]);

  MQUE_FLOOP_CLS (Q[w], x) ad->and_ (& _SG._edge._v[*x]);
  if ( ad->get_t()==0 ) return (-1);
  if ( ad->get_v(ad->get_t()-1) > w ) return (ad->get_v(ad->get_t()-1));
  return (-1);
}

LONG MACE::parent_check_parent (QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w){
  QUEUE_INT t=0, *x, i;
  K->set_v(K->get_t(),-1); // loop stopper;
  ad->cpy (&_SG._edge._v[Q[w].get_v(0)]);

  MQUE_FLOOP_CLS (Q[w], x) ad->and_(&_SG._edge._v[*x]);

  while ( ad->get_t() > 0 ){
    i = ad->ext_tail_();
    if ( i<w ) return (-1);
    while ( i<K->get_v(t) ) t++;
    if ( i > K->get_v(t) ) return (i);
    ad->and_(&_SG._edge._v[i]);
  }
  return (-1);
}


/*************************************************************************/
/* MACE main iteration */
/*************************************************************************/
void MACE::iter (int v){
  LONG ii;
  QUEUE_INT u;
  QUEUE_ID js = _itemcand.get_s();
  QUEUE *Q = _OQ;

//printf ("%d:   ", II->iters);
//printf ("%d:  ", v); QUEUE_print__ (&Q[v]);

  _II.inc_iters();
  _itemcand.set_s(_itemcand.get_t());

  add_vertex ( &Q[v], v);
  extend ( &Q[v], v);
//  MACE_make_same_list (v);

  _II.set_itemset_t(0);
  memcpy (_II.item_get_v(), Q[v].get_v(), sizeof(QUEUE_INT)*Q[v].get_t());
  _II.item_set_t(Q[v].get_t());
  _II.output_itemset ( NULL, 0);

  qsort_<QUEUE_INT>(_itemcand.getp_v(_itemcand.get_s()), _itemcand.get_t()-_itemcand.get_s(), -1);

  while ( _itemcand.get_t() > _itemcand.get_s() ){
    u = _itemcand.ext_tail_();
    if ( u == Q[v].TAIL_() ){
      Q[v].dec_t();
      if ( _problem & PROBLEM_CLOSED ) VBM_reset_vertex (u);
    } else {
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
void MACE::MACECORE (){
  QUEUE *E = _SG._edge._v;
  QUEUE_INT v;

  FLOOP (v, 0, _SG._edge.get_t()){
    if ( E[v].get_t()==0 ){
      _II.item_set_t(0);
      _II.itemINS(v);
      _II.output_itemset ( NULL, 0);
    } else if ( E[v].get_v(E[v].get_t()-1) <= v ){
      iter (v);
    }
    _OQ[v].set_t(0);
  }
}




int MACE::run (int argc, char *argv[]){
  QUEUE_INT v, flag = 0;
	

	read_param (argc, argv);

	if ( _ERROR_MES ) return (1);

  if ( _problem & PROBLEM_MAXIMAL ){
    _problem |= PROBLEM_CLOSED;  // flag for the use of VBM
    flag = 1;
  }

  preRUN();

  if ( !_ERROR_MES && _SG._edge.get_eles() > 0 ){
    if ( _problem & PROBLEM_FREQSET ){
      FLOOP (v, 0, _SG._edge.get_t()) clq_iter ( v, &_SG._edge._v[v]);
    } 
    else{
    	 MACECORE();
    }
    _II.last_output();
  }

  if ( flag ){
    free2 (_VV._edge);
    free2 (_VV._pos);
    free2 (_VV._set);
    free2 (_VV._reset);
    //_VV._dellist.end();
  }

  return (_ERROR_MES?1:0);
}

int MACE::mrun(int argc ,char* argv[]){
	return MACE().run(argc,argv);
}
