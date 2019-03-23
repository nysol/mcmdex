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

/****************************************************************************/
/* d = degree of node i := G->edge/in/out.v[i].t
   d = max/min (in/out) degree := VEC_MAXT(d,G->edge.v/in.v/out.v,0,...->t)  (VEC_MINT, resp.)
   #nodes :=  SGRAPH_NODE_NUM(G)
   #edges :=  G->edge.eles/2
   #arcs :=   G->in.eles or G->out.eles
   load_node_weight := ARY_LOAD_WEIGHT(G->node_w,WEIGHT,filename,counter,"errormes", EXIT) 
   load_node_weight := ARY_LOAD_WEIGHT(G->node_w,WEIGHT,filename,counter,"errormes", EXIT) 

   sort_node by size := SGRAPH_sort_node_iter (G, qsort_perm_VECt ((VEC *)Q, G->node_end, flag)
   sort_node by weight := SGRAPH_sort_node_iter (G, qsort_perm_WEIGHT (w, G->node_end, flag)
*/
/****************************************************************************/
#pragma once

//#ifndef _sgraph_h_
//#define _sgraph_h_

#include"stdlib2.hpp"
#include"vec.hpp"

#define SGRAPH_NODE_NUM  MAX(_edge.get_t(),_in.get_t())

/*  structure for graph  */
class SGRAPH {
  unsigned char _type;   // structure type flag
  char *_fname;      // input file name
  int _flag;         // flag for load routine

  QUEUE_INT _node1_num;   // the size of vertex set 1, bipartite case. otherwise 0
  WEIGHT *_node_w, *_wbuf;    // pointer to the node weight array(int)
  PERM *_perm;       // node permutation (nodes->original)

	//この辺つかわれてない？
  char *_wfname, *_nwfname;     // edge/node weight file name

  SETFAMILY _edge, _in, _out;  // setfamily for edge, in-arc, out-arc

  QUEUE_INT *_itemary;


	public:
	SGRAPH():
		_type(TYPE_SGRAPH),_fname(NULL),_flag(0),_node1_num(0),_itemary(NULL),
		_node_w(NULL),_wbuf(NULL),_perm(NULL),_wfname(NULL),_nwfname(NULL)
	{}


	int itemAlloc(size_t siz){

		calloc2(_itemary, siz+2, return 1); 
		
		return 0;

	}

	void itemCntUp(QUEUE_INT item){

		for(QUEUE_INT *x=_edge.get_vv(item); *x < item ; x++){
			_itemary[*x]++;
		}
	}
	void itemCntDown(QUEUE_INT item){

		for(QUEUE_INT *x=_edge.get_vv(item); *x < item ; x++){
			_itemary[*x]--;
		}
	}
	QUEUE_INT itemCnt(QUEUE_INT item){ return _itemary[item]; }
	
	void adaptPerm(VEC_ID t,PERM * perm){

		PERM *sperm = NULL;
		PERM *tmp=NULL;

    malloc2 (sperm, _edge.get_t(), EXIT);
    
		for(size_t i=0 ; i< _edge.get_t(); i++){  sperm[i]=i; }
		
		for(size_t i=0 ; i <  MIN(t, _edge.get_t()) ;i++ ){
			sperm[i] = perm[i];
		}
		
		malloc2(tmp , _edge.get_t() , {free(sperm);EXIT;});

		for(size_t st=0; st < _edge.get_t() ;st++){ tmp[st] = -1; }
		
		for(int i=0;i<_edge.get_t();i++){
			if(sperm[i]>=0 && sperm[i] < _edge.get_t() ){ tmp[sperm[i]]=i; }
		}

		replace_index(sperm, tmp);
    mfree (tmp, sperm);
		_perm =NULL;
		
		/*
      malloc2 (sperm, _SG.edge_t(), EXIT);
      //ARY_INIT_PERM (sperm, _SG._edge.get_t());
			for(size_t i=0 ; i< _SG.edge_t(); i++){ sperm[i]=i; }

      FLOOP (i, 0, MIN(_TT.get_t(), _SG.edge_t())) sperm[i] = _TT.get_perm(i);

      //ARY_INV_PERM (tmp, sperm, _SG._edge.get_t(), {free(sperm);EXIT;});
			malloc2(tmp,_SG.edge_t(),{free(sperm);EXIT;});
			for(size_t st=0; st<_SG.edge_t() ;st++){ tmp[st]=-1; }
			for(int i=0;i<_SG.edge_t();i++){
				if(sperm[i]>=0 && sperm[i]<_SG.edge_t()){ tmp[sperm[i]]=i; }
			}
      _SG.replace_index (sperm, tmp);
      mfree (tmp, sperm);
      _SG.set_perm( NULL);
		*/
	}
	
	



	void edgeSetEnd(){
		//FLOOP (i, 0, _SG.edge_t()){
		for(size_t i=0 ; i< _edge.get_t() ; i++){
			_edge.set_vv(i,_edge.get_vt(i),_edge.get_t());
		}
	}

	QUEUE_INT * edge_vv(QUEUE_INT item) { return _edge.get_vv(item); }


	VEC_ID edge_t(){ return _edge.get_t(); }
	void edge_sort(){  _edge.sort(); }
	VEC_ID edge_eles(){ return _edge.get_eles(); }

	QUEUE_ID edge_vt(int i){ 
		return _edge.get_vt(i);
	}

	void edge_setvv(int i,int j, QUEUE_ID v ) { 
		_edge.set_vv(i,j,v);
	}


	QUEUE* getp_v(int i){
		return _edge.getp_v(i);
	}

	QUEUE* getp_v(){
		return _edge.getp_v();
	}

	// これは再考
	void edge_union_flag(int flag){ _edge.union_flag(flag);} 

	void set_perm( PERM *perm){ _perm=perm; }


	/*  initialization, termination, allocate arrays for weights, copy and duplication */
	void alloc (int node_num, size_t edge_num, size_t arc_num);
	// void SGRAPH_cpy (SGRAPH *G2, SGRAPH *G); 未使用
	void end ();

	/*  make/take/remove edge e as connecting vertices u and v,
	 and  edge (u,v). 
 	 do nothing if when make already existing edge, or delete non-existing edge.
 	 with range check of parameters */
	void edge_mk ( QUEUE_INT u, QUEUE_INT v, WEIGHT w);
	void edge_rm ( QUEUE_INT u, QUEUE_INT v);
	void arc_mk ( QUEUE_INT u, QUEUE_INT v, WEIGHT w);
	void arc_rm ( QUEUE_INT u, QUEUE_INT v);

	/* subroutine of sort_edge_list */
	//void SGRAPH_sort_edge_list_iter (QUEUE *Q, WEIGHT **w, PERM *invperm, VEC_ID i, int flag);

	/* sort each array list, increasing if flag=1, and decreasing if flag=-1 */
	//void SGRAPH_sort_edge_list (SGRAPH *G, int flag);

	/* replace node i by perm and invperm */
	void replace_index ( PERM *perm, PERM *invperm);

	/* sort the nodes by permutation given by tmp */
	//PERM *SGRAPH_sort_node_iter (SGRAPH *G, PERM *tmp);

	/* sort the nodes by degrees, increasing if flag=1, decreasing if flag=-1 */
	//PERM *SGRAPH_sort_node_t (SGRAPH *G, QUEUE *Q, int flag);

	/* sort the nodes by node_weight, increasing if flag=1, decreasing if flag=-1 */
	//PERM *SGRAPH_sort_node_w (SGRAPH *G, WEIGHT *w, int flag);

	/* remove multiple edges/arcs and self loops 
   it works only when after applying sort_incident_edges */
	//void SGRAPH_simple (SGRAPH *G, int flag);


	//void sort_node (int flag); // 未使用？
	void perm_node ( PERM *tmp); // private?




/******************* print routines *************************************/


	/*  print graph by numbers  */
	//void print (FILE *fp);

	/* Write the graph to file. Edges, arcs, and nodes from 0 to node_num/edge_num/arc_num are written to file. Parameters are
	  (graph) (file name) (not write edge weight => 0) (not write node weight => 0) */
	//void SGRAPH_save (SGRAPH *G, char *fname);

	/* graph load routine. Allocate memory as much as the size of input file.
	   parameters are, 
	   (graph) (file name) (read edges?) (read arcs?) (read node weights?) (read edge weight?) (bipartite?) */
	/* In the row of each vertex, write only vertices larger than it connected by an edge */
	int load (int flag ,char* fname);
	///void SGRAPH_load_node_weight (SGRAPH *G, char *filename);未使用？

	void rm_selfloop ();


} ;





