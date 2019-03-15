/*  Common problem input/output routines /structure
            25/Nov/2007   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

/***************************************************/
#pragma once
//#include "fstar.hpp"
//#include "itemset.hpp" 
//#include "trsact.hpp" 
//#include "vec.hpp" 
//#include "base.hpp" 





//#define PROBLEM_DOC 64

/*****  parameters for PROBLEM initialization, given to flag  *****/

#define PROBLEM_FREQSET 1
#define PROBLEM_MAXIMAL 2
#define PROBLEM_CLOSED 4

#define PROBLEM_NORMALIZE   32  // print density threshold

#define PROBLEM_ITEMARY 128 // alloc itemary
#define PROBLEM_ITEMJUMP 256 // alloc itemjump
#define PROBLEM_ITEMCAND 2048 // alloc itemcand
#define PROBLEM_VECCHR 262144  //alloc vecchr
#define PROBLEM_OCC_T 524288 // alloc occ_t
#define PROBLEM_SHIFT 1048576  // allocate shift
#define PROBLEM_OCC_W 2097152  // weight/positive-weight sum for items


#define PROBLEM_EX_MAXIMAL 8
#define PROBLEM_EX_CLOSED 16
// #define PROBLEM_MINIMAL 32

//#define PROBLEM_ITEMFLAG 512  // alloc itemflag
//#define PROBLEM_ITEMMARK 1024  // alloc itemmark

// #define PROBLEM_VECARY 4096 // alloc itemary
//#define PROBLEM_VECJUMP 8192 // alloc vecjump
//#define PROBLEM_VECFLAG 16384  // alloc vecflag
//#define PROBLEM_VECMARK 32768  // alloc vecmark
//#define PROBLEM_VECCAND 65536 // alloc veccand
//#define PROBLEM_ITEMCHR 131072  //alloc itemchr

#define PROBLEM_OCC_PW 4194304  // weight/positive-weight sum for items
#define PROBLEM_OCC_W2 8388608  // weight/positive-weight sum for items
//#define PROBLEM_ITEMLIST 16777216  // alist for items
//#define PROBLEM_VECLIST 33554432  // alist for vecs
//#define PROBLEM_VECW 67108864  // weight array for vecs

//#define PROBLEM_OCC1 16 // alloc occ
//#define PROBLEM_OCC2 32 // alloc occ and ins all to list 0
//#define PROBLEM_OCC3 48 // alloc occ and ins all to list "siz"

#if 0
struct PROBLEM {
	char *_ERROR_MES;
  clock_t _start_time, _end_time;
  int _problem, _problem2;
  LONG _prog;
  int _prog2;
  double _dense;
  char *_input_fname, *_input_fname2;
  char *_output_fname, *_output_fname2;
  char *_workdir, *_workdir2;
  
  char *_weight_fname;
  char *_table_fname, *_table2_fname;
  char *_outperm_fname, *_outperm_fname2;
  char *_header_fname, *_position_fname, *_position2_fname, *_sc_fname;
  
  ITEMSET _II, _II2;
  QUEUE _ff;      // for agraph search
  int *_vf, *_dep; // for agraph search

  int _root, _dir, _edge_dir;
  double _th, _th2, _th3;   // thresholds
  double _ratio, _ratio2;  // ratio
  int _num, _siz, _dim, _len, _width, _height, _gap_ub, _gap_lb;
  int _xmax, _ymax, _pxmax, _pymax;
  int _tmax, _tmin;
  QUEUE_INT _clms;
  VEC_ID _rows;
  WEIGHT _cost, _cost2;

  QUEUE_ID **_shift;
  QUEUE _itemjump, _itemcand, _vecjump, _veccand, *_OQ, *_OQ2, *_VQ, *_VQ2;   // for delivery
  QUEUE_INT *_itemary;
  char *_itemchr, *_vecchr;
  int *_itemmark, *_itemflag, *_vecmark, *_vecflag;  // mark for vector
  VEC_ID *_vecary, *_occ_t;
  WEIGHT *_occ_w, *_occ_pw, *_occ_w2, *_occ_pw2, *_vecw;
  QUEUE _oo;
  QUEUE_INT *_buf, *_buf_org;
  size_t _buf_end;
  
  char *_pat;   // pattern string
  int _plen, _perr;  // pattern length and #error allowed

  BASE _B; // from base.h
  TRSACT _TT,_TT2; //  from trsact.h
  FSTAR _FS, _FS2; //from  fstar_h

  MAT _MM, _MM2; // from _vec_h_
  SMAT _SM, _SM2;  // from _vec_h_
  SETFAMILY _FF, _FF2; // from _vec_h_


  //_itemjump = _itemcand = _vecjump = _veccand = INIT_QUEUE;  // for delivery
	//_ff (INIT_QUEUE),
  //_oo = INIT_QUEUE;
	//#ifdef _base_h_
 // _B = INIT_BASE;
	//#endif
	//#ifdef _trsact_h_
  //TRSACT_init (&P->TT);
  //TRSACT_init (&P->TT2);

  //_MM = INIT_MAT;
  //_MM2 = INIT_MAT;
  //_SM = INIT_SMAT;
  //_SM2 = INIT_SMAT;
  //_FF = INIT_SETFAMILY;
  //_FF2 = INIT_SETFAMILY;

	PROBLEM(void)
		:_ERROR_MES(NULL),_problem(0),_problem2(0),_prog(0),_prog2(0),
		_input_fname(NULL),_input_fname2(NULL),_output_fname(NULL),_output_fname2(NULL),
	  _workdir(NULL),_workdir2(NULL),
	  _weight_fname(NULL),_header_fname(NULL),_table_fname(NULL),_sc_fname(NULL),
	  _position_fname(NULL),_position2_fname(NULL),_outperm_fname(NULL),
	  _root(0),_dir(0),_edge_dir(0),
	  _th(0),_th2(0), _th3(0),_ratio(0),_ratio2(0),
	  _num(0),_siz(0),_dim(0),_len(0),_width(0),_height(0),
	  _rows(0), _clms(0),_gap_ub(INTHUGE),_gap_lb(0),
		_xmax(0),_ymax(0),_pxmax(0),_pymax(0),
		_tmax(0),_tmin(0),_cost(0),_cost2(0),
  	_vf(NULL),_dep(NULL),_shift(NULL),
  	_occ_w(NULL),_occ_pw(NULL),_occ_w2(NULL),_occ_pw2(NULL),
	  _buf(NULL),_buf_org(NULL),
	  _buf_end(0),
	  _itemchr(NULL),_vecchr(NULL),
		_OQ(NULL),_OQ2(NULL),_VQ(NULL),_VQ2(NULL),
	  _itemary(NULL),_itemmark(NULL),_itemflag(NULL),_vecmark(NULL),_vecflag(NULL),
		_occ_t(NULL),_vecary(NULL),_vecw(NULL),_pat(NULL),
	  _plen(0),_perr(0)
	{
		
		printf("%p\n",&_TT._fname);		
		printf("%p\n",&_TT2._fname);		
			if(_TT2._fname){cerr << "t notNULL" << endl;} 
			else { cerr << "t NULL" << endl; } 
			if(_TT2._fname){cerr << "t2 notNULL" << endl;} 
			else { cerr << "t2 NULL" << endl; } 
	}


	//void PROBLEM_print (PROBLEM *P);
	void print(void);

	/*****  print usage of the program *****/
	//void PROBLEM_error ();
	void _error(void);

	/*****  read parameters given by command line  *****/
	//void PROBLEM_read_param (int argc, char *argv[], PROBLEM *P);
	//void read_param (int argc, char *argv[]);



	/*****  PROBLEM and ITEMSET initialization *****/
	/* all pointers are set to NULL, but don't touch filenames */
	//void PROBLEM_init (PROBLEM *P);
	//void init(void);

	/*****  PROBLEM initialization: load the files given by filenames   ******/
	//void PROBLEM_load (PROBLEM *P);
	void load(void);

	/*****  allocate memory according to flag  *****/
	//void PROBLEM_alloc (PROBLEM *PP, QUEUE_ID siz, QUEUE_ID siz2, size_t siz3, PERM *p, int f);

	//あと
	void alloc (QUEUE_ID siz, QUEUE_ID siz2, size_t siz3, PERM *p, int f);

	/* termination of problem */
	void end(void);

};
#endif
// not USE in sspc
/*
#ifdef _alist_h_
  ALIST itemlist, veclist;
#endif

#ifdef _agraph_h_
  AGRAPH AG, AG2;
#endif

#ifdef _alist_h_
  MALIST occ;
#endif

#ifdef _sgraph_h_
  SGRAPH SG, SG2;
#endif
#ifdef _seq_h_
  SEQ SS, SS2;
#endif
#ifdef _pos_h_
  POS PS, PS2;
#endif
#ifdef _barray_h_
  BARRAY BA;
  BARRAY BA2;
#endif
#ifdef _fstar_h_
  FSTAR FS, FS2;
#endif


*/
