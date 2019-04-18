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
#pragma once

#include "stdlib2.hpp"
#include "vec.hpp"
#include "queue.hpp"



/*  structure for graph  */
class SGRAPH {

  char *_fname;      // input file name

  int _flag;         // flag for load routine

  PERM   *_perm;    // node permutation (nodes->original)

  SETFAMILY _edge;      // setfamily for edge,

  QUEUE_INT *_itemary; // item Count ARRAY


	public:
	SGRAPH():
		_fname(NULL),_flag(0),
		_itemary(NULL),_perm(NULL) {}

	~SGRAPH(){
	  mfree (_perm,_itemary);
	}


	int itemAlloc(size_t siz){
		//calloc2(_itemary, siz+2, return 1); 
		_itemary = calloc2(_itemary, siz+2); 

		return 0;
	}

	void itemCntUp(QUEUE_INT item){

		QUEUE_INT *x;

		for( x=_edge.get_vv(item); *x < item ; x++){
			_itemary[*x]++;
		}

	}

	void itemCntDown(QUEUE_INT item){

		QUEUE_INT *x;

		for(QUEUE_INT *x=_edge.get_vv(item); *x < item ; x++){
			_itemary[*x]--;
		}

	}

	QUEUE_INT itemCnt(QUEUE_INT item){ return _itemary[item]; }
	

	void adaptPerm(VEC_ID t,PERM * perm){

		PERM *sperm = NULL;
		PERM *tmp   = NULL;

    // malloc2 (sperm, _edge.get_t(), EXIT);
    sperm = malloc2(sperm, _edge.get_t());

		for(size_t i=0 ; i< _edge.get_t(); i++){  sperm[i]=i; }
		
		for(size_t i=0 ; i <  MIN(t, _edge.get_t()) ;i++ ){
			sperm[i] = perm[i];
		}
		
		//malloc2(tmp , _edge.get_t() , {free(sperm);EXIT;});
		try{
			tmp = malloc2(tmp , _edge.get_t() );
		}catch(...){
			free(sperm);
			throw;
		}

		for(size_t st=0; st < _edge.get_t() ;st++){ tmp[st] = -1; }
		
		for(int i=0;i<_edge.get_t();i++){
			if(sperm[i]>=0 && sperm[i] < _edge.get_t() ){ tmp[sperm[i]]=i; }
		}

		replace_index(sperm, tmp);

    mfree (tmp, sperm);

		_perm = NULL;
		

	}


	void edgeSetEnd(){

		for(size_t i=0 ; i< _edge.get_t() ; i++){
			_edge.set_vv(i,_edge.get_vt(i),_edge.get_t());
		}
	}

	QUEUE_INT *	skipedge(QUEUE_INT v,QUEUE_INT w){
		QUEUE_INT * x;
		for(x = _edge.get_vv(v); *x < w ;x++){;}
		return x;
	}

	QUEUE_INT * edge_vv(QUEUE_INT i) { return _edge.get_vv(i); }
	QUEUE_ID edge_vt(int i){ return _edge.get_vt(i); }

	QUEUE_INT * edgeEnd(int i){ return _edge.end(i); }

	QUEUE_INT edge_Lastvv(QUEUE_INT i) { 
		return _edge.get_vv(i,_edge.get_vt(i)-1); 
	}


	VEC_ID edge_t(){ return _edge.get_t(); }
	void   edge_sort(){  _edge.sort(); }
	VEC_ID edge_eles(){ return _edge.get_eles(); }


	QUEUE* getp_v(int i){ return _edge.getp_v(i); }

	// これは再考
	void edge_union_flag(int flag){ _edge.union_flag(flag);} 

	int loadEDGE(int flag ,char* fname);

	/* remove all selfloops */
	void rm_selfloop (){ _edge.rmSelfLoop(); }

	// replace node i by perm and invperm 
	void replace_index (PERM *perm, PERM *invperm){
		_edge.replace_index(perm,invperm);
	  _perm = perm;

	}
	char * initOQ(QUEUE *);
	

} ;





