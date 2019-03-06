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

#include"sgraph.hpp"
#include"problem.hpp"

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
} ;

//int **PP->shift;    /* pointers to the positions of the current processing items in each transaction */



class MACE{
	int _problem;
	char *_outperm_fname;
	char *_output_fname;
  char *_ERROR_MES;
	VEC_ID *_occ_t;
	QUEUE *_OQ;
	QUEUE_ID **_shift;
	QUEUE _itemcand;

	SGRAPH _SG;
	ITEMSET _II;
	MACEVBM _VV;


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
	void preALLOC(QUEUE_ID siz, QUEUE_ID siz2, size_t siz3, PERM *perm, int f);

	void _error(void);

	void preLOAD(){
  	if ( _SG._fname ){ 

  		_SG.load();
  		if (ERROR_MES) EXIT; 
  	}

	}
	void read_param (int argc, char *argv[]);

	public:
	MACE():_problem(0),_outperm_fname(NULL),_output_fname(NULL){
		_ERROR_MES=NULL;
	}

	int run(int argc ,char* argv[]);
	static int mrun(int argc ,char* argv[]);

};

