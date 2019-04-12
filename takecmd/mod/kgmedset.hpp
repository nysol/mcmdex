/* take the intersection of each cluster */
/* 8/Nov/2008   Takeaki Uno  */
#pragma once

#include "fstar.hpp"
#include "problem.hpp"

#define MEDSET_CC 4
#define MEDSET_IND 8
#define MEDSET_NO_HIST 16
#define MEDSET_RATIO 32
#define MEDSET_ALLNUM 64
#define MEDSET_BELOW 128

#ifndef UNIONFIND_ID
 #ifdef UNIONFIND_ID_LONG
  #define UNIONFIND_ID LONG
  #define UNIONFIND_ID_END LONGHUGE
  #define UNIONFIND_IDF LONGF
 #elif defined(UNIONFIND_ID_QUEUE)
  #define UNIONFIND_ID QUEUE_ID
  #define UNIONFIND_ID_END QUEUE_ID_END
  #define UNIONFIND_IDF QUEUE_IDF
 #elif defined(UNIONFIND_ID_ALIST)
  #define UNIONFIND_ID ALIST_ID
  #define UNIONFIND_ID_END ALIST_ID_END
  #define UNIONFIND_IDF ALIST_IDF
 #else
  #define UNIONFIND_ID int
  #define UNIONFIND_ID_END INTHUGE
  #define UNIONFIND_IDF "%d"
 #endif
#endif

class KGMEDSET{

  FILE2 _fp;
	FILE *_ofp; // org II._fp

	int _problem;

	char *	_input_fname;
	char *  _output_fname;

	int _dir;
	int _num;
  double _th ;
	char *_ERROR_MES;
	FSTAR _FS;


	// _FS 
	int _fsFlag;
	char *_set_fname;


	void help();


	/* read commands and options from command line */
	int setArgs(int argc, char *argv[]);


	/* output clusters to the output file */
	void print_clusters (FSTAR_INT *mark, FSTAR_INT *set, FSTAR_INT xmax);

	/* read file, output the histogram of each line */
	void cc_clustering();
	void ind_clustering();
	void read_file();



	public:

	KGMEDSET():
		_problem(0),_fsFlag(0),_input_fname(NULL),_dir(0),_num(0),_th(0),
		_output_fname(NULL),_ofp(NULL),_set_fname(NULL),_ERROR_MES(NULL){}
		
	int run (int argc, char *argv[]);
	static int mrun(int argc, char *argv[]);


};






