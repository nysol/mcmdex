/* take the intersection of each cluster */
/* 8/Nov/2008   Takeaki Uno  */
#pragma once

#include "stdlib2.hpp"
#include "fstar.hpp"
#include "problem.hpp"
#include "unionfind.hpp"

#define MEDSET_CC 4
#define MEDSET_IND 8
#define MEDSET_NO_HIST 16
#define MEDSET_RATIO 32
#define MEDSET_ALLNUM 64
#define MEDSET_BELOW 128


class kgClusterForCC{

	// cnt: cluster siz, if v is representative, 
	// and #vertices covering v, if v isn't representative 

	FSTAR_INT _xmax;

	VECARY<UNIONFIND_ID> _mark,_set,_cnt;
	// _cnt以外は　class UNIONFINDと近い_mark => ID

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

	kgClusterForCC():_xmax(0){}

	~kgClusterForCC(){}

	void read(IFILE2 *fp){ 

		LONG x, y;

		// merge the connponents to be connected by using spray tree
		do {

	    if ( fp->read_pair( &x, &y ) ) continue;

			ENMAX(_xmax, MAX(x, y)+1);

			_mark.reallocSeq(_xmax);
			_set .reallocSeq(_xmax);

			_unify_set(x, y);

		} while ( fp->NotEof() ); //(FILE_err&2)==0 
		
	}

	void readWithCnt(IFILE2 *fp){

		LONG x, y, yy;
		FSTAR_INT flag = 0;

    // merge the connponents to be connected by using spray tree
	  do {

	    flag = 0;

  	  do {

	      if ( fp->read_pair( &x, &y ) ) continue;

  	    ENMAX (_xmax, MAX(x, y)+1);

				_mark.reallocSeq(_xmax);
				_set .reallocSeq(_xmax);
		    _cnt .reallocx(_xmax, (UNIONFIND_ID)0);

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

			} while ( fp->NotEof()); //(FILE_err&2)==0 

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
			ofp.putch('\n');
		}
		return;
	}

};


class KGMEDSET{

  IFILE2 _fp;
	OFILE2 _ofp; // org II._fp

	int _problem;

	char *	_input_fname;
	char *  _output_fname;

	int _num;
  double _th ;
	FSTAR _FS;

	// _FS 
	int _fsFlag;
	char *_set_fname;


	/* read commands and options from command line */
	int setArgs(int argc, char *argv[]);

	void help();

	/* read file , and output the histogram of each line */
	void _MedsetCore();

	void _printMes(char *frm ,...){

		if( _fsFlag&1 ){
			va_list ap;
			va_start(ap,frm);
			vfprintf(stderr,frm,ap);
			va_end(ap);
		}
	}


	public:

	KGMEDSET():
		_problem(0),_fsFlag(0),_input_fname(NULL),_num(0),_th(0),
		_output_fname(NULL),_set_fname(NULL){}
		
	int run (int argc, char *argv[]);
	static int mrun(int argc, char *argv[]);


};






