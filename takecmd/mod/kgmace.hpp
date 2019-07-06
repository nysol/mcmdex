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


#pragma once
#include "sgraph.hpp"
#include "problem.hpp"


#define WEIGHT_DOUBLE

#ifndef ITEMSET_INTERVAL
#define ITEMSET_INTERVAL 500000
#endif



#define VBMMARK_MAX 16  /* MAXsize of BITMAP */ 
#define VBMINT unsigned long     /* variable type for BITMAP */
#define VBMINT_MAX 30  /* MAXsize of BITMAP */

//char *_mark;
//int _mark_max;
class MACEVBM {

  VBMINT *_edge;  /* BITMAP representation w.r.t. columns of vertices in the current clique */
  VBMINT *_set, *_reset;  /* array for BITMASKs */
  int *_pos;   /* positions of vertices of the clique in the bitmap */
  QUEUE _dellist;
  
  public:
	MACEVBM():_edge(NULL),_set(NULL),_reset(NULL),_pos(NULL)
	{
		_dellist.set_v(NULL);
	}

	~MACEVBM(){
		_dellist.clear();
		delete [] _edge;
		delete [] _pos;
		delete [] _set;
		delete [] _reset;	
	}


  void alloc(size_t size){

    _edge  = new VBMINT[size];
    _pos   = new int[size];
    _set   = new VBMINT[VBMINT_MAX];
    _reset = new VBMINT[VBMINT_MAX];


    _dellist.alloc(VBMINT_MAX+2,VBMINT_MAX);

		for(size_t i =0; i<size ; i++){ _edge[i] = 0; }

		_dellist.set_t(VBMINT_MAX);

		VBMINT p;
    for (size_t i=0,p=1 ; i<VBMINT_MAX ; i++,p*=2){
      _set[i] = p;
      _reset[i] = -1-p;
      _dellist.set_v(i, i);
    }
	  ERR:;
    return ;
  }
  
  VBMINT reset_vertex(QUEUE_INT v){
		_dellist.push_back(_pos[v]);
  	return _reset[_pos[v]];
  }
  VBMINT set_vertex(QUEUE_INT v){
	  _pos[v] = _dellist.pop_back();
  	return _set[_pos[v]];
  }

  void andEdge(QUEUE_INT x,VBMINT p){
  	_edge[x] &= p;
  }
  
  void orEdge(QUEUE_INT x,VBMINT p){
	  _edge[x] |= p;
	}

  VBMINT orByQUE(QUEUE Q){
		VBMINT p=0;
		for(size_t i=0 ; i < Q.get_t() ; i++){
  		p |= _set[_pos[Q.get_v(i)]];
	  }
	  return p;
	}
	
  VBMINT getSetByPos(QUEUE_INT z){
		return _set[_pos[z]]; 
	}
	bool existEdge(QUEUE_INT x, VBMINT pp){
		return pp == (pp&_edge[x]);
	}
} ;


class KGMACE{

	int _problem;

	FILE2 _fp; 

	char *_outperm_fname;
	char *_output_fname;

	SGRAPH _SG;
	MACEVBM _VV;

	bool _outApend;
	bool _onMsg;
	bool _onProgress;

	// _II
	// int _iFlag;
	int _lb ,_ub;
	int _max_solutions;
	char _separator;

	// base _II
	QUEUE _itemset;   // current operating itemset
  QUEUE _add;       // for equisupport (hypercube decomposition)
  LONG _iters;      // iterations
  LONG _outputs;
  LONG _solutions;
	PERM *_perm;

	// SG
	char *_sgfname;
	int _sgFlag;


	QUEUE_ID **_shift;

	QUEUE *_OQ;
	char *_OQbuf;
	QUEUE _itemcand;


	void VBM_set_vertex (QUEUE_INT v);
	void VBM_reset_vertex (QUEUE_INT v);

	void VBM_set_diff_vertexes ( QUEUE_INT K1, QUEUE_INT K2);
	void VBM_reset_diff_vertexes (QUEUE_INT K1, QUEUE_INT K2);
	LONG VBM_parent_check ( QUEUE_INT K , QUEUE_INT w);

	void scan_vertex_list ( QUEUE_INT v, QUEUE_INT w);

	LONG parent_check ( QUEUE_INT K, QUEUE_INT w);

	void add_vertex (QUEUE_INT K, QUEUE_INT v);
	void extend (QUEUE_INT w);
	void clq_iter ( QUEUE_INT v, QUEUE *occ);
	void iter (int v);
	void MACECORE ();

	void help(void);

	int setArgs (int argc, char *argv[]);



	// for Mace 
	/*******************************************************************/
	/* output at the termination of the algorithm */
	/* print #of itemsets of size k, for each k */
	/*******************************************************************/
	void _last_output (){

  	QUEUE_ID i;
  	LONG n=0, nn=0;
  	WEIGHT w;
  	unsigned char c;

	  FILE2 *fp = &_fp;

  	if ( !_onMsg ) return;  // "no message" is specified
  
 		fprintf(stderr,"iters=" LONGF, _iters);
  	fprintf(stderr,"\n");  
	}

	void _output_itemset(){

	  QUEUE_ID i;
  	QUEUE_INT e;

	  int flush_flag = 0;

  	FILE2 *fp = &_fp;
  
	  _outputs++;

	  if ( _onProgress && (_outputs%(ITEMSET_INTERVAL) == 0) ){
  	    fprintf(stderr,
    	  			"---- " LONGF " solutions in " LONGF " candidates\n",
							_solutions, _outputs);
	  }

	  if ( _itemset.get_t() < _lb || _itemset.get_t() > _ub ) return;

  	_solutions++;

	  if ( _max_solutions > 0 && _solutions > _max_solutions ){
  	  _last_output ();
    	EXIT; //returnでする方法考える
	  }
	  if ( fp ){

			for(i=0;i<_itemset.get_t();i++){
    	  e = _itemset.get_v(i);
      	fp->print_int( _perm? _perm[e]: e, i==0? 0: _separator);
      	fp->flush_ ();
  	  }
    	fp->putch('\n');
      fp->flush_ ();
  	}
	}

	/* allocate arrays and structures */
	void preALLOC (){

	  size_t siz = _SG.edge_t()+2;

		_shift = new QUEUE_ID *[siz](); //calloc2

		_itemcand.alloc(siz);

    // set outperm
		if ( _outperm_fname ){
			FILE2::ARY_Load(_perm , _outperm_fname);
	  }

  	_itemset.alloc((QUEUE_ID)siz,(QUEUE_ID)siz);
		_add.alloc((QUEUE_ID)siz);

		_iters = 0;
		_outputs = 0;
		_solutions = 0;

		if(_output_fname){
			// バッファ確保しないほうがいい？
			if ( strcmp (_output_fname, "-") == 0 ) _fp.open(stdout);
			else{
				if(_outApend){ _fp.open(_output_fname,"a");}
				else         { _fp.open(_output_fname,"w");}
			}
		}
 		return;
	}



	void _queInit(){

		_OQ = new QUEUE[_SG.edge_t()+1];

		QUEUE_INT *x;
		VEC_ID eSize = _SG.edge_t();

		VEC_ID *occ_t =  new VEC_ID[eSize+2](); // calloc2

		for (VEC_ID iv=0 ; iv< eSize; iv++){
			for ( x= _SG.edge_vv(iv) ; *x < eSize ; x++){ occ_t[*x]++; }
		}
		size_t OQMemSize = 0;
		for(VEC_ID i=0; i < eSize ; i++){ OQMemSize += occ_t[i]; }

		if(!
			( _OQbuf = (char*)malloc(
					sizeof(char) * 
					( OQMemSize+(eSize*2)+2)*(sizeof(QUEUE_INT))  
			))
		){ 
			delete [] occ_t;
			throw("memory allocation error : SGRAPH::initOQ");
		}
		char *cmn_pnt = _OQbuf;
		for(VEC_ID i=0; i < eSize ;i++){
			_OQ[i].set_endv( occ_t[i],(QUEUE_ID *)cmn_pnt);
			cmn_pnt += (sizeof(QUEUE_INT)) * (occ_t[i]+(2));
		}
		delete [] occ_t;
	}


	public:

	KGMACE():
		_problem(0),_outperm_fname(NULL),_output_fname(NULL),_sgFlag(0),_sgfname(NULL),
		_ub(INTHUGE),_lb(0),_max_solutions(0),_separator(' '),_perm(NULL),_OQbuf(NULL),
		_outApend(false),_onMsg(true),_onProgress(false){}

	~KGMACE(){
		delete [] _shift;
  	delete [] _perm;
  	delete [] _OQ;
  	if(_OQbuf){ free(_OQbuf); } 
	}
	int run(int argc ,char* argv[]);
	static int mrun(int argc ,char* argv[]);

};

/*

	// PROBLEM_ITEMJUMP + PROBLEM_ITEMCAND + PROBLEM_SHIFT + PROBLEM_OCC_T
	//LONG parent_check_max ( QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w);
	//LONG parent_check_parent (QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w);
	//void add_vertex (QUEUE *K, QUEUE_INT v);
	//void extend (QUEUE *K, QUEUE_INT w);
	//void VBM_set_diff_vertexes ( QUEUE *K1, QUEUE *K2);
	//void VBM_reset_diff_vertexes (QUEUE *K1, QUEUE *K2);
	// LONG VBM_parent_check ( QUEUE *K, QUEUE *Q, QUEUE_INT w);
	//LONG parent_check ( QUEUE *K, QUEUE_INT w);
	// VEC_ID *_occ_t;

// 表示させたいなら復活させる
// 		_sc = new LONG[siz+2](); // calloc2
//	  _sc[_itemset.get_t()]++;
//	LONG *_sc;
//		for(i=0;i<_itemset.get_end()+1;i++){
//  	  n += _sc[i];
//    	if ( _sc[i] != 0 ) nn = i;
//	  }
// 
 //  	if ( n!=0 ){ //OK?
// 	   printf (LONGF "\n", n);
//			for(i=0;i<nn+1;i++){  printf (LONGF "\n", _sc[i]); }
//	  }
//		delete [] _sc;

*/
