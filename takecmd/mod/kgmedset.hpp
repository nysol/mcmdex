/* take the intersection of each cluster */
/* 8/Nov/2008   Takeaki Uno  */
#pragma once

#include "stdlib2.hpp"
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
 #else
  #define UNIONFIND_ID int
  #define UNIONFIND_ID_END INTHUGE
  #define UNIONFIND_IDF "%d"
 #endif
#endif

class kgClusterForCC{

	// cnt: cluster siz, if v is representative, and #vertices covering v, if v isn't representative 
	UNIONFIND_ID *_mark, *_set, *_cnt;
	UNIONFIND_ID _end1, _end2, _end3;
	FSTAR_INT _xmax;

	// get the ID of belonging group, and flatten the ID tree 
	UNIONFIND_ID _getID (UNIONFIND_ID v){
  	UNIONFIND_ID vv = v, vvv;
  	// trace parents until the root (ID[v] = v, if v is a root)
  	while (v  != _mark[v] ){ v = _mark[v]; }
  	while (vv != _mark[vv]){ vvv = vv; vv = _mark[vv]; _mark[vvv] = v; }
  	return v;
	}

  // maintain ID and list to representing the sets simultaneously
	void _unify_set(UNIONFIND_ID u, UNIONFIND_ID v){

		UNIONFIND_ID z;

		v = _getID(v); // compute ID of v 
		u = _getID(u); // compute ID of u 

		if ( u != v ){
			if ( _set[u] == u ){ _set[u] = v; _mark[v] = u; } // attach u as the head of the list of v
			else if ( _set[v] == v ){ _set[v] = u; _mark[u] = v; }  // attach v as the head of the list of u
			else {
				for (z=v; _set[z]!=z ; z=_set[z]);  // find the last in the list of v
				_set[z] = _set[u]; _set[u] = v; // insert the list of v to list of u
				_mark[v] = u; // set ID of (ID of v) to (ID of u)
			}
		}
	}

	public:

	kgClusterForCC():
		_mark(NULL),_set(NULL),_cnt(NULL),
		_end1(0), _end2(0), _end3(0),_xmax(0){}

	~kgClusterForCC(){
		mfree(_mark, _set,_cnt);
	}
		

	void read(FILE2 *fp){

		LONG x, y;

		// merge the connponents to be connected by using spray tree
		do {

			if ( fp->read_pair( &x, &y, NULL, 0) ) continue;

			ENMAX(_xmax, MAX(x, y)+1);

			_mark = reallocx (_mark, &_end1, _xmax);
			_set  = reallocx (_set , &_end2, _xmax);

			_unify_set(x, y);

		//} while ( (FILE_err&2)==0 );
		} while ( fp->eof());
		
	}

	void readWithCnt(FILE2 *fp){

		LONG x, y, yy;
		FSTAR_INT flag =0;

    // merge the connponents to be connected by using spray tree
	  do {
	    flag = 0;

  	  do {

	      if ( fp->read_pair ( &x, &y, NULL, 0) ) continue;

  	    ENMAX (_xmax, MAX(x, y)+1);

    	  _mark = reallocx(_mark, &_end1, _xmax);
				_set  = reallocx(_set,  &_end2, _xmax);
     	 _cnt  = reallocx(_cnt,  &_end3, _xmax, (UNIONFIND_ID)0);

	      if ( _cnt[x] < _cnt[y] ) SWAP_<LONG> (&x, &y);

  	    if ( _mark[x] == x && _mark[y] == y ){

    	    if ( _set[x] == x && !( _set[y]== y && _cnt[y]>0) ){ 
						_unify_set (y, x); 
        		_cnt[y]++; 
        		_cnt[x] = 1; 
						flag = 1; 
					}
					else {
						do {

	            yy = _set[y];
  	          _set[y] = y; 
    	        y = yy;

      	 	    _mark[y] = y;
          	  _cnt[y]--;

	          } while (y != _set[y]);
	        }
 
  	      if ( _set[y] == y ){ 
    	    	_unify_set (x, y); 
      	  	_cnt[x]++; 
       			_cnt[y] = 1; 
 						flag = 1;
					}
	      }

  	    if ( _mark[x] == x ){ _cnt[y]++; }
				else if ( _mark[y] == y ){ _cnt[x]++; }

	    //} while ( (FILE_err&2)==0 );
			} while ( fp->eof());

		} while (flag);	

		return;
	}

	/* output clusters to the output file */
	void print(OFILE2 &ofp, int num)
	{

  	FSTAR_INT x, c;
  
		for(FSTAR_INT i=0;i<_xmax;i++){

  	  if ( _mark[i] != i ) continue;

	    c = 0; 
  	  x = i;

	    while (1){

  	    c++;
    	  if ( _set[x] == x ) break;
      	x = _set[x];

	    }

   	 if ( c < num ) continue;

	    x = i;

  	  while (1){

    	  ofp.print(""FSTAR_INTF" ", x);
				if ( _set[x] == x ) break;
				x = _set[x];

			}
			ofp.putc('\n');
		}
		return;
	}

};


class KGMEDSET{

  FILE2 _fp;
	OFILE2 _ofp; // org II._fp

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
		_output_fname(NULL),_set_fname(NULL),_ERROR_MES(NULL){}
		
	int run (int argc, char *argv[]);
	static int mrun(int argc, char *argv[]);


};






