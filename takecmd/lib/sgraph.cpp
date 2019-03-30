/*  graph library by array list
            12/Feb/2002    by Takeaki Uno
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#include"sgraph.hpp"
#include"vec.hpp"


/*  initialization  */
void SGRAPH::alloc ( QUEUE_ID nodes, size_t edge_num, size_t arc_num){
  if ( edge_num > 0 ){
    _edge.alloc(nodes, NULL, nodes, edge_num);
    if ( _flag&LOAD_EDGEW && (!ERROR_MES) ) _edge.alloc_weight ( NULL);
  }
  if ( arc_num > 0 ){
    _in.alloc ( nodes, NULL, nodes, arc_num);
    _out.alloc( nodes, NULL, nodes, arc_num);
    if ( _flag&LOAD_EDGEW && (!ERROR_MES) ){
      _in.alloc_weight ( NULL);
      _out.alloc_weight ( NULL);
    }
  }
  //if (_flag&LOAD_NODEW) calloc2 (_node_w, nodes, _node_w=0);
  if ( ERROR_MES ){ end (); EXIT; }
}

/* copy graph G to graph G2. Underconstruction */
//void SGRAPH_cpy (SGRAPH *G2, SGRAPH *G){}

/* free graph object  */
void SGRAPH::end (){
  //_edge.end();
  //_in.end();
  //_out.end();
  mfree (_wbuf, _perm);
  //*G = INIT_SGRAPH;
}


/*  make an edge between u and v.
   If they are already connected, it will be a multiple edge */
void SGRAPH::edge_mk (QUEUE_INT u, QUEUE_INT v, WEIGHT w){
  if ( _edge.exist_w() ){
    _edge.set_w(u,_edge.get_vt(u),w);
    _edge.set_w(v,_edge.get_vt(v),w);
  }
  _edge.push_back(u,v);
  _edge.push_back(v,u);
  _edge.add_eles(2);
}

/*  make an arc between u and v.
   If they are already connected, it will be a multiple arc */
void SGRAPH::arc_mk (QUEUE_INT u, QUEUE_INT v, WEIGHT w){
  if ( _out.exist_w() ){
  	_out.set_w(u,_out.get_vt(u),w);
  }
  if ( _in.exist_w() ){
  	_in.set_w(v,_in.get_vt(v), w);
  }
  _out.push_back(u,v);
  _in.push_back(v,u);
  _in.add_eles(1);
  _out.add_eles(1);
}

/* Delete the edge connecting u and v. If edge (u,v) does not exist, nothing will occur. */
static void SGRAPH_edge_rm_iter (SETFAMILY *M, QUEUE_INT u, QUEUE_INT v){
  QUEUE_INT i;
  //if ( (i = (QUEUE_INT)QUEUE_ele (&M->v[u], v)) >= 0 ){
  // QUEUE_rm (&M->v[u], i);
	if ( (i = (QUEUE_INT) M->get_v_ele(u,v) ) >= 0 ){  
    M->v_rm(u,i);
    if ( M->exist_w() ) M->set_w(u,i, M->get_w(u,M->get_vt(u)));
    M->add_eles(-1);
  }
}

/* Delete the edge connecting u and v. If edge (u,v) does not exist, nothing will occur. */
void SGRAPH::edge_rm (QUEUE_INT u, QUEUE_INT v){
  SGRAPH_edge_rm_iter (&_edge, u, v);
  SGRAPH_edge_rm_iter (&_edge, v, u);
}

/* Delete the arc connecting u and v. If arc (u,v) does not exist, nothing will occur. */
void SGRAPH::arc_rm ( QUEUE_INT u, QUEUE_INT v){
  SGRAPH_edge_rm_iter (&_out, u, v);
  SGRAPH_edge_rm_iter (&_in, v, u);
}

/*  print graph by numbers  */
/*
void SGRAPH::print (FILE *fp){
  VEC_ID i, j;
  QUEUE_INT e;
  
  fprintf (fp, "#node "VEC_IDF" ,#edge %zd ,#arc %zd\n", SGRAPH_NODE_NUM, _edge.get_eles(), _in.get_eles());
  FLOOP (i, 0, SGRAPH_NODE_NUM ){
    fprintf (fp, "NODE "VEC_IDF" ", i);
    if ( _node_w ){ fputc ('(', fp); print_WEIGHT (_node_w[i]); fputc (')', fp); }
    fprintf (fp, " >>\n");
    if ( _edge.exist_v() && _edge.get_vt(i) ){
      fprintf (fp, "    edge      : ");
      for (j=0; j<_edge.get_vt(i) ; j++){
        e = _edge.get_vv(i,j);
        fprintf (fp, VEC_IDF, e);
        if ( _edge.exist_w() ){ fputc ('(', fp); print_WEIGHT (_edge.get_w(i,j)); fputc (')', fp); }
        fputc (',', fp);
      }
      fputc ('\n', fp);
    }
    if ( _in.exist_v() ){
      if ( _in.get_vt(i) ){
        fprintf (fp, "    in-arc      : ");
        for (j=0; j<_in.get_vt(i) ; j++){
          e = _in.get_vv(i,j);
          fprintf (fp, VEC_IDF, e);
          if ( _in.exist_w() ){ fputc ('(', fp); print_WEIGHT (_in.get_w(i,j)); fputc (')', fp); }
          fputc (',', fp);
        }
        fputc ('\n', fp);
      }
    }
    if ( _out.exist_v() ){
      if ( _out.get_vt(i) ){
        fprintf (fp, "    out-arc      : ");
        for (j=0; j<_out.get_vt(i) ; j++){
          e = _out.get_vv(i,j);
          fprintf (fp, VEC_IDF, e);
          if ( _out.exist_w() ){ fputc ('(', fp); print_WEIGHT (_out.get_w(i,j)); fputc (')', fp);}
          fputc (',', fp);
        }
        fputc ('\n', fp);
      }
    }
  }
}
*/
/* Output a graph to file
  Vertices, edges, arcs less than node_num, edge_num, arc_num are written to the file. Input parameters are
  (graph) (file name) (flag)
  SGRAPH_READ_NODEW 512 // read node weight
  SGRAPH_READ_EDGEW 1024 // read edge weight
*/
/*
  format of file:(including notifications to make input file)
   
  the ith row corresponds to node i-1, and
    ID list of nodes adjacent to i, and having ID > i, for undirected graph
    ID list of nodes adjacent to i by out-going arc of i, for directed graph
   Separator is ",", but graph load routine accepts any letter for 
    separator but not a number.
   If the graph has both edges and arcs, write them in two lines separately,
    so a node then uses two lines, and #nodes = #lines/2.
  
    ==  Notifications to make input file ==
   Notice that if 0th line has node 2, and the 2nd line has 0, then there
    will be multiple edge (0,2) and (2,0).
   The read routine does not make error with multiple edges, it is allowed.

   The ID of nodes begin from 0. After reading graph, node_num is set to
    node_end.

   Input file example, without weights, E={(0,1),(0,2),(1,1),(1,3),(2,3)}
===========
   1,2
   1 3
   3
   
   [EOF]
=========
   Nodes are 0,1, and 2, both edges and arcs exist, with node/edge/arc weights)
   5000,1,30
   0,50,1,20,
   100,1,3
   2,20
   200
   
   [EOF]
=======
   where node weights are 5000, 100, 200, and edges and their weights are
    (0,1),30,   (1,1),3
    arcs and their weights are (0,0),50,   (0,1), 20,   (1,2), 20

    In the case of bipartite graph, write the adjacent-node lists only for 
     the node in node set one.
     
    
*/

/* graph load routine. Allocate memory as much as the size of input file.
   parameters are, 
   (graph) (file name) 
 LOAD_EDGE // read undirected edge from file
 LOAD_ARC // read directed arc from file
 LOAD_BIPARTITE // load bipartite graph
 LOAD_NODEW // read node weight
 LOAD_EDGEW // read edge weight
*/
/* In the bipartite case, even if the IDs of node set 2 begin from 0, i.e.,
   overlaps with node 1, the routine automatically correct them. */
/* Directed bipartite graph, all arcs are considered to be from node set 1
 to node set 2. If both directions exist, read as a general graph, and set
  node1_num later in some way. */
/* The routine compares the maximum node index and #lines, and set #node
  to the larger one. However, if node weight exists, weights will be included 
  in the candidates of maximum index, thus in this case we fix #node := #lines.
  In the case of bipartite graph, the routine compares, but the weights of 
   non-existing lines will be -1. */


/* load edges/arcs (determined by G->flag) from file */
// LCM用 undirect
int SGRAPH::loadEDGE (char* fname){

  VEC_ID i;
  _fname = fname;

	_edge.load( LOAD_EDGE + LOAD_RC_SAME , _fname);

	/*
	_edge.allvvInitByT();

  _edge.set_rw(_node_w);
  _edge.sort();
  _edge.set_rw(NULL); 
	*/
  _perm = _edge.get_rperm(); 
  _edge.set_rperm(NULL);

  print_mes (this, "sgraph: %s ,#nodes %d ,#edges %zd ,#arcs %zd", _fname, SGRAPH_NODE_NUM, _edge.get_eles()/2,  _edge.get_eles());
  if ( _wfname ) print_mes (this, " ,weight file: %s", _wfname);
  if (_nwfname ) print_mes (this, " ,node weight file: %s", _nwfname);
  print_mes (this, "\n");
	return 0;
}



int SGRAPH::load (int flag ,char* fname){

  VEC_ID i;
  QUEUE_ID *c, j;
  QUEUE_INT e;
  SETFAMILY *F1, *F2;
  WEIGHT *ww;
  PERM *p;
  QUEUE Q;
  _flag = flag;
  _fname = fname;
  
//  if ( G->flag&LOAD_EDGE ){ F1 = F2 = &G->edge; F1->flag |= LOAD_DBLBUF; }
  if ( _flag&LOAD_EDGE ) F1 = F2 = &_edge;
  else {
    F1 = &_in; F2 = &_out;
    F1->union_flag(LOAD_ARC);
    if ( _flag & LOAD_TPOSE ) F1->union_flag(LOAD_TPOSE);
  }
  F1->union_flag( _flag & (LOAD_ELE + LOAD_EDGEW + LOAD_EDGE + LOAD_RC_SAME + LOAD_ID1 + LOAD_NUM + LOAD_GRAPHNUM));
  if ( !(_flag&LOAD_BIPARTITE) ) F1->union_flag(LOAD_RC_SAME);

  F1->set_fname(_fname); 
  F1->set_wfname(_wfname);

  F1->load();

  if ( _nwfname ){
  
	  i = FILE2::ARY_Load(_node_w, _nwfname, 1);
    reallocx_ (_node_w, i, SGRAPH_NODE_NUM + 1, 0, EXIT);
  }

  FLOOP (i, 0, F1->get_t()) F1->set_vv(i,F1->get_vt(i),F1->get_t()); // set endmark

    // adjast so that #rows and #colums are the same

  if ( !(_flag&LOAD_EDGE) ){  // make opposite-direction arc
    calloc2 (c, F1->get_t(), EXIT);
    //QUEUE_delivery (NULL, c, NULL, F1->v, NULL, F1->t, F1->t);  // comp. size of each adjacent list
		VEC_ID iv, ev;
	  QUEUE_INT *x;
		for (iv=0 ; iv<F1->get_t() ; iv++){
    	ev =  iv;
    	MLOOP(x, F1->get_vv(ev), F1->get_t()) c[*x]++;
		}
    F2->set_t(F1->get_clms()); 
    F2->set_clms(F1->get_t());
    F2->alloc( F1->get_t(), c, F1->get_t(), 0);
    if ( F1->exist_w() ) F2->alloc_weight (c);

    FLOOP (i, 0, F1->get_t()) c[i] = F1->get_vt(i);
    FLOOP (i, 0, F1->get_t()){
      if ( F2->exist_rw() ) F2->set_rw(i,F1->get_rw(i));
      FLOOP (j, 0, c[i]){
        e = F1->get_vv(i,j);
        if ( F2->exist_w() ) F2->set_w(e,F2->get_vt(e),F1->get_w(i,j));
        F2->push_back(e,i);
      }
    }
    free (c);
    F2->set_clms(F2->get_t()); 
    FLOOP (i, 0, F2->get_t()) F2->set_vv(i, F2->get_vt(i),F2->get_t()); // set endmark
  }
  
    // sort the nodes
  F1->union_flag(_flag); F1->set_rw(_node_w);
  F1->sort();
  F1->set_rw(NULL); _perm = F1->get_rperm(); F1->set_rperm(NULL);
  if ( F1 != F2 ){
    F2->union_flag( _flag); 
    F2->trim_flag(LOAD_SIZSORT+LOAD_WSORT);
    //BITRM(F2->get_flag(), LOAD_SIZSORT+LOAD_WSORT);
    F2->sort ();
    if ( _flag & (LOAD_SIZSORT+LOAD_WSORT) ){

      //ARY_INV_PERM(p, _perm, F2->get_t(), EXIT);
			malloc2(p,F2->get_t(),EXIT);
			for(size_t st=0; st<F2->get_t() ;st++){ p[st]=-1; }
			for(int i=0;i<F2->get_t();i++){
				if(_perm[i]>=0 && _perm[i]<F2->get_t()){ p[_perm[i]]=i; }
			}


      if ( F2->exist_w() ){
      	ARY_INVPERMUTE (F2->get_w(), p, ww, F2->get_t(), EXIT);
      }
      F2->ary_INVPERMUTE_(p,Q);
      //ARY_INVPERMUTE_ (F2->_v, p, Q, F2->get_t());
      free2 (p);
    }
  }
  print_mes (this, "sgraph: %s ,#nodes %d ,#edges %zd ,#arcs %zd", _fname, SGRAPH_NODE_NUM, _edge.get_eles()/2,  _in.get_eles());
  if ( _wfname ) print_mes (this, " ,weight file: %s", _wfname);
  if (_nwfname ) print_mes (this, " ,node weight file: %s", _nwfname);
  print_mes (this, "\n");
	return 0;
}

/* replace node i by perm[i] */
void SGRAPH::replace_index (PERM *perm, PERM *invperm){
  QUEUE_INT *x;
  VEC_ID i;
  QUEUE Q;
  WEIGHT *w, ww;

  VEC_ID node_num = SGRAPH_NODE_NUM;

  if ( _edge.exist_v() ){
    FLOOP (i, 0, SGRAPH_NODE_NUM ){
			for(x=_edge.get_vv(i); x<_edge.get_vv(i) + _edge.get_vt(i) ; x++){
				*x = perm[*x];
			}
    }
    _edge.ary_INVPERMUTE( invperm, Q, node_num);
  }

  if ( _in.exist_v() ){
    FLOOP (i, 0, SGRAPH_NODE_NUM ) {
    	for(x=_in.get_vv(i); x<_in.get_vv(i) + _in.get_vt(i) ; x++){
    		*x = perm[*x];
    	}
    }
		//ここ正しい？
    _in.ary_INVPERMUTE(invperm, Q, node_num); 
  }

  if ( _out.exist_v() ){
    FLOOP (i, 0, SGRAPH_NODE_NUM ){
      for(x=_out.get_vv(i); x<_out.get_vv(i) + _out.get_vt(i) ; x++){
    		*x = perm[*x];
    	}
    }
	  //ここ正しい？
    _out.ary_INVPERMUTE ( invperm, Q, node_num);
  }

  if ( _edge.exist_w() ) ARY_INVPERMUTE (_edge.get_w(), invperm, w, SGRAPH_NODE_NUM , EXIT);
  if ( _in.exist_w() ) ARY_INVPERMUTE (_in.get_w(), invperm, w, SGRAPH_NODE_NUM , EXIT);
  if ( _out.exist_w() ) ARY_INVPERMUTE (_out.get_w(), invperm, w, SGRAPH_NODE_NUM , EXIT);
  if ( _node_w ) ARY_INVPERMUTE (_node_w, invperm, ww, SGRAPH_NODE_NUM , EXIT);

  _perm = perm;

}


/* remove all selfloops */
void SGRAPH::rm_selfloop (){
  QUEUE_ID i, j, jj;
  QUEUE_INT x;
  FLOOP (i, 0, SGRAPH_NODE_NUM ){
    if ( _edge.exist_v() ){
      jj = 0;
      FLOOP (j, 0, _edge.get_vt(i)){
        x = _edge.get_vv(i,j);
        if ( x != i ){
          if ( j != jj ){
            _edge.set_vv(i,jj,_edge.get_vv(i,j));
            if ( _edge.exist_w() ) _edge.set_w(i,jj, _edge.get_w(i,j));
          }
          jj++;
        }
      }
      _edge.set_vt(i,jj);
    }
    if ( _in.exist_v() ){
      jj = 0;
      FLOOP (j, 0, _in.get_vt(i)){
        x = _in.get_vv(i,j);
        if ( x != i ){
          if ( j != jj ){
            _in.set_vv(i,jj,_in.get_vv(i,j));
            if ( _in.exist_w() ) _in.set_w(i,jj,_in.get_w(i,j));
          }
          jj++;
        }
      }
      _in.set_vt(i,jj);
    }
    if ( _out.exist_v() ){
      jj = 0;
      FLOOP (j, 0, _out.get_vt(i)){
        x = _out.get_vv(i,j);
        if ( x != i ){
          if ( j != jj ){
            _out.set_vv(i,jj,_out.get_vv(i,j));
            if ( _out.exist_w() ) _out.set_w(i,jj,_out.get_w(i,j));
          }
          jj++;
        }
      }
      _out.set_vt(i,jj);
    }
  }
}
