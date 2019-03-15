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


#define WEIGHT_DOUBLE

#include "sgraph.hpp"
#include "itemset.hpp"
#include "problem.hpp"

#define VBMMARK_MAX 16  /* MAXsize of BITMAP */ 
#define VBMINT unsigned long     /* variable type for BITMAP */
#define VBMINT_MAX 30  /* MAXsize of BITMAP */

struct MACEVBM {
  VBMINT *_edge;  /* BITMAP representation w.r.t. columns of vertices in the current clique */
  VBMINT *_set, *_reset;  /* array for BITMASKs */
  int *_pos;   /* positions of vertices of the clique in the bitmap */
  QUEUE _dellist;
  char *_mark;
  int _mark_max;
  
	MACEVBM():
		_edge(NULL),_set(NULL),_reset(NULL),_pos(NULL)
	{
		_dellist.set_v(NULL);
	}


  void alloc(size_t size){

    malloc2 (_edge,  size, goto ERR);
    malloc2 (_pos,   size, goto ERR);
    malloc2 (_set,   VBMINT_MAX, goto ERR);
    malloc2 (_reset, VBMINT_MAX, goto ERR);
    _dellist.alloc(VBMINT_MAX+2);
    _dellist.set_t(VBMINT_MAX);


    //ARY_FILL (_VV._edge, 0, _SG.edge_t(), 0);
		for(size_t i =0; i<size ; i++){ _edge[i] = 0; }
		VBMINT p;
    for (size_t i=0,p=1 ; i<VBMINT_MAX ; i++,p*=2){
      _set[i] = p;
      _reset[i] = -1-p;
      _dellist.set_v(i, i);
    }
    return ;
	  ERR:;
		_dellist.clear();
		free2 (_edge);
		free2 (_pos);
		free2 (_set);
		free2 (_reset);
  }
} ;

//int **PP->shift;    /* pointers to the positions of the current processing items in each transaction */



class KGMACE{

	int _problem;

	char *_outperm_fname;
	char *_output_fname;

	SGRAPH _SG;
	ITEMSET _II;
	MACEVBM _VV;

	// _II
	int _iFlag;
	int _lb ,_ub;
	int _max_solutions;
	char _separator;

	// SG
	char *_sgfname;
	int _sgFlag;


	char* _ERROR_MES;


	QUEUE_ID **_shift;
	VEC_ID *_occ_t;


	QUEUE *_OQ;
	QUEUE _itemcand;


	void VBM_set_vertex (QUEUE_INT v);
	void VBM_reset_vertex (QUEUE_INT v);
	void VBM_set_diff_vertexes ( QUEUE *K1, QUEUE *K2);
	void VBM_reset_diff_vertexes (QUEUE *K1, QUEUE *K2);
	void add_vertex (QUEUE *K, QUEUE_INT v);
	void scan_vertex_list ( QUEUE_INT v, QUEUE_INT w);
	void extend (QUEUE *K, QUEUE_INT w);
	
	LONG parent_check ( QUEUE *K, QUEUE_INT w);
	LONG VBM_parent_check ( QUEUE *K, QUEUE *Q, QUEUE_INT w);
	LONG parent_check_max ( QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w);

	LONG parent_check_parent (QUEUE *K, QUEUE *ad, QUEUE *Q, QUEUE_INT w);


	void clq_iter ( QUEUE_INT v, QUEUE *occ);
	void iter (int v);
	void MACECORE ();
	void preRUN();

	void help(void);

	int setArgs (int argc, char *argv[]);

	//void read_param (int argc, char *argv[]);


	/* allocate arrays and structures */
	void preALLOC (){

	  PERM *p=NULL;

	  int j;

	 // PROBLEM_ITEMJUMP + PROBLEM_ITEMCAND + PROBLEM_SHIFT + PROBLEM_OCC_T

		calloc2 (_shift, _SG.edge_t()+2, goto ERR);
		calloc2 (_occ_t, _SG.edge_t()+2, goto ERR);

		_itemcand.alloc(_SG.edge_t()+2);

    // set outperm
		if ( _outperm_fname ){
			j = FILE2::ARY_Load (p,  _outperm_fname, 1);
	  }

  	_II.alloc(_output_fname, p, _SG.edge_t() , _SG.edge_eles());

 		
 		return;

	  ERR:;
 		EXIT;
	}


	public:
	KGMACE():
		_problem(0),_outperm_fname(NULL),_output_fname(NULL),
		_sgFlag(0),_sgfname(NULL),_ub(INTHUGE),_lb(0),_iFlag(0),
		_max_solutions(0),_separator(' '),_ERROR_MES(NULL){
		
	}

	int run(int argc ,char* argv[]);
	static int mrun(int argc ,char* argv[]);

};

