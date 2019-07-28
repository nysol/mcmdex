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


#pragma once

#define SSPC_INCLUSION 1
#define SSPC_SIMILARITY 2
#define SSPC_INTERSECTION 4
#define SSPC_RESEMBLANCE 8
#define SSPC_INNERPRODUCT 16
#define SSPC_MININT 32
#define SSPC_MAXINT 64
#define SSPC_PMI 128
#define SSPC_FVALUE 256
#define SSPC_COUNT 2048
#define SSPC_MATRIX 4096
#define SSPC_NO_NEIB 16384
#define SSPC_POLISH 32768
#define SSPC_POLISH2 65536
#define SSPC_OUTPUT_INTERSECT 131072
#define SSPC_COMP_ITSELF 262144

#include <vector>
#include "trsact.hpp"
#include "problem.hpp"
#include "itemset.hpp"

class KGSSPC{

	// receive parameter
	// SSPC
	int _problem;
  double _th2;
	double _th;

	char *_output_fname2;
	char *_table_fname;
	char *_outperm_fname;
	char *_output_fname;

	bool _progressFlag;

	LONG _itemtopk_item;
	LONG _itemtopk_item2;
	LONG _itemtopk_end;

	ItemSetParams _ipara;
	TrsactParams _tpara;

	// Trsact
	/*
  FILE_COUNT _C;
  SETFAMILY _T;   // transaction
  VECARY<WEIGHT> _w;
  WEIGHT *_pw;  // weight/positive-weight of transactions
	PERM *_perm;
  QUEUE _jump, *_OQ;   
*/

	int _siz;

	PERM *_positPERM; // ( org _position_fname)
	WEIGHT *_occ_w;

	// POLISH2のときのみ利用
	//（y:output elements of each set that 
	// contribute to no similarity (fast with much memory use)）
  QUEUE_INT *_itemary;
	VECARY <QUEUE_INT> _buf;
  size_t _buf_end;

	// POLISH or POLISH2の時のみ
	char  *_vecchr;  

	ITEMSET _II;
	TRSACT _TT;

	void output ( QUEUE_INT *cnt, QUEUE_INT i, QUEUE_INT ii, QUEUE *itemset, WEIGHT frq, int core_id);

	void *iter(void *p);

	WEIGHT comp( WEIGHT c, WEIGHT wi, WEIGHT wx, WEIGHT sq);
	void   comp2( QUEUE_ID i, QUEUE_ID x, 
								WEIGHT c, WEIGHT wi, WEIGHT wx, 
								double sq, QUEUE_INT *cnt, OFILE2 *fp, 
								QUEUE *itemset, int core_id
							);

	void _SspcCore();

	/* allocate arrays and structures */
	void _preALLOC();
	
	int setArgs(int argc, char *argv[]);

 	void help();

	public :
	KGSSPC():
		_problem(0),_siz(0),
		_th(0),_th2(0),_progressFlag(false),
		_output_fname(NULL),_output_fname2(NULL),
		_outperm_fname(NULL),_table_fname(NULL),
		_positPERM(NULL),_vecchr(NULL),
		_occ_w(NULL),_itemary(NULL),
		_buf_end(0),
		_itemtopk_item(0),_itemtopk_item2(0),_itemtopk_end(0)
		{}

	int run(int argc ,char* argv[]);
	
	vector<LONG> iparam(){ 
		vector<LONG> rtn(2);
		rtn[0] = _II.get_solutions(); 
		rtn[1] = _TT.get_clms_org();
		return rtn;
	}
	
	static vector<LONG> mrun(int argc ,char* argv[]);

};

//		_root(0),_dir(0),	_sep(0),
//  int _dir; 
//  int _root; 
//  int _sep;



