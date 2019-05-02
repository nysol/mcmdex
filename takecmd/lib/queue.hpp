/* Library of queue: spped priority implementation 
            12/Apr/2001   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */
#pragma once

#include <iostream> // debug
#include "stdlib2.hpp"

#ifdef QUEUE_INT_LONG
	#define QUEUE_INT LONG    // define the type before if change is needed 
	#define QUEUE_INTHUGE LONGHUGE    // comment out if QUEUE_INT is "short"
	#define QUEUE_INTF LONGF
	#ifndef QUEUE_INT_END
		#define QUEUE_INT_END LONGHUGE
	#endif
#else
	#define QUEUE_INT int    // define the type before if change is needed 
	#define QUEUE_INTHUGE INTHUGE    // comment out if QUEUE_INT is "short"
	#define QUEUE_INTF "%d"
	#ifndef QUEUE_INT_END
		#define QUEUE_INT_END INTHUGE
	#endif
#endif

#ifdef QUEUE_ID_LONG
	#define QUEUE_ID LONG    // define the type before if change is needed 
	#define QUEUE_IDHUGE LONGHUGE    // comment out if QUEUE_INT is "short"
	#define QUEUE_IDF LONGF
#else
	#define QUEUE_ID int    // define the type before if change is needed 
	#define QUEUE_IDHUGE INTHUGE    // comment out if QUEUE_INT is "short"
	#define QUEUE_IDF "%d"
#endif

struct KGLCMSEQ_ELM{
  QUEUE_INT _t;   // transaction ID
  QUEUE_INT _s;   // previous position
  QUEUE_INT _org;  // original position
} ;

struct KGLCMSEQ_QUE{
  unsigned char _type;  // type of the structure
  KGLCMSEQ_ELM *_v;  // pointer to the array
  QUEUE_ID _end;  // the length of the array
  QUEUE_ID _t;  // end position+1
  QUEUE_ID _s;  // start position

	KGLCMSEQ_ELM * get_v(){ return _v;}
	QUEUE_ID get_t(){ return _t;}
	QUEUE_ID get_end(){ return _end;}
	int size()   { return  _t - _s ;}

	void set_end(QUEUE_ID val){ _end = val;}

	void set_s(QUEUE_ID val){ _s = val;}
	void set_t(QUEUE_ID val){ _t = val;}
	void set_v(KGLCMSEQ_ELM *val){ _v = val;}
	void set_v(int i,QUEUE_ID v){ _v[i]._t=v;}

	void endClr(){ _end = 0;  }
	void tClr()  { _t = 0;  }
	bool posCheck(){ return  _s < _t ;}

	//void INS(KGLCMSEQ_ELM v) { _v[_t++]=v; }
	void push_back(KGLCMSEQ_ELM v) { _v[_t++]=v; }

	void push_backt(QUEUE_ID val) {
		KGLCMSEQ_ELM vval;
		vval._t=val;
		 _v[_t++]=(vval); 
	}

	QUEUE_INT pop_back(){return _v[--_t]._t;}

	KGLCMSEQ_ELM * begin(){ return _v; }
	KGLCMSEQ_ELM * start(){ return _v + _s; }

	KGLCMSEQ_ELM * end()  { return _v + _t; }
	void posClr(){ _t = 0; _s = 0; }

	/* initialization, not fill the memory by 0 */
	void alloc (QUEUE_ID siz){
		_end =  siz+1;
		//malloc2(_v, siz+1, EXIT);
		_v =new KGLCMSEQ_ELM[siz+1];

	}


	void inc_t(){ _t++;}
	void inc_end(){ _end++;}

	void show(){
		KGLCMSEQ_ELM * xx =_v;
		xx++;		
		std::cerr << "sque show " <<  _v << " - " << (_v + _t) << " (" << xx << ") " << _t << std::endl;
			for(KGLCMSEQ_ELM * x = _v ;x < _v + _t; x++){
				std::cerr << "sque " << x->_t << " " << x->_s  << " " << x->_org << std::endl; 
			}
	}
	void setStartByEnd(){ _s = _t; }

} ;


class QUEUE {

  QUEUE_INT *_v;  // pointer to the array
  QUEUE_ID _end;  // the length of the array
  QUEUE_ID _t;  // end position+1
  QUEUE_ID _s;  // start position

	/* 
		delivery: transpose that matrinx (transaction database) Q. Each row of the 
	 transposed matrix is called occurrence.

		variables to be set.
		OQ:array for occurrences, c: for counting frequency, jump: list of items with non-empty OQ
		if c!=NULL, count the frequency and set to c, and set occurrences to OQ, otherwise.
		if jump==NULL, then the list of non-empty item will not be generated
		Q:matrix, of an array of QUEUE, occ: list of rows of Q to be scaned, t; maximum ID of the
		 row to be scaned; if occ==NULL, occ will be ignored, otherwise t will be ignored.
		 M: end mark of each QUEUE. 
	*/
	void delivery(
		QUEUE *OQ, 
		VEC_ID *c, QUEUE *jump, 
		QUEUE *Q, QUEUE *occ, 
		VEC_ID t, QUEUE_INT M);

  /* print */
	void print ();
		

	public:

	QUEUE(void):_v(NULL),_end(0),_t(0),_s(0){}

	QUEUE(QUEUE_ID siz ,QUEUE_ID t):_end(siz+1),_t(t),_s(0){
		//malloc2(_v, siz+1, EXIT);
		_v = new QUEUE_INT[siz+1];
	}
	QUEUE(QUEUE_ID siz):_end(siz+1),_t(0),_s(0){
		//malloc2(_v, siz+1, EXIT);
		_v = new QUEUE_INT[siz+1];
	}

	~QUEUE(void){
	  // free2 (_v); setvが有る限りむり
	}

	bool operator>(const QUEUE& rhs) const{
	  if ( _t < rhs._t ) return (false);
  	else return ( _t >  rhs._t);
	}
	bool operator<(const QUEUE& rhs) const{
	  if ( _t > rhs._t ) return (false);
  	else return ( _t < rhs._t);
	}
	void clrMark( char* mark ){
		for(QUEUE_INT * x=_v; x<_v+_t ; x++){
			mark[*x] = 0;
		}
	}

	void clear(){
		printf("clear\n");
	  //free2 (_v);
		delete [] _v;
		_v = NULL;
		_end=0;
		_t=0;
		_s=0;
	}



	QUEUE_INT * begin(){ return _v; }
	QUEUE_INT * start(){ return _v + _s; }
	QUEUE_INT * end()  { return _v + _t; }
	void resize(QUEUE_ID v){ _t=v; }
	bool posCheck(){ return  _s < _t ;}

	QUEUE_INT tail(){ return _v[_t-1];}

	// この２つ一緒 size() , LENGTH_()
	int size()   { return  _t - _s ;}

	// この３つ一緒 pop_back ,pop ,ext_tail_
	QUEUE_INT pop_back(){ return _v[--_t]; }

	// insと同じ？ org QUE_INS INs
	void push_back(QUEUE_INT v){
		 _v[_t++]=v; 
	}



	//void INS(QUEUE_INT v){ _v[_t++]=v; }

	void show(){
		std::cerr << "QUE ";
		for(QUEUE_INT * x= begin() ; x < end(); x++){
			std::cerr << " " << *x  ;
 		}
 		std::cerr << std::endl;
	
	}

	void posClr(){ _t = 0; _s = 0; }
	void endClr(){ _end = 0;  }
	void tClr()  { _t = 0;  }
	void sClr()  { _s = 0;  }

	void setEndByStart(){ _t = _s; }
	void setStartByEnd(){ _s = _t; }

	void setStopper(){ _v[_t] = -1; }


	bool exist(){ return ( _t > _s ); }
	
	void queSort(int flag){ qsort_<QUEUE_INT>( _v, _t , flag); }

	void queSortfromS(int flag){ qsort_<QUEUE_INT>( _v + _s, _t - _s , flag); }

	void rmDup(QUEUE_INT dmy){ 
		if( _t > 1 ){
			int cmm1 = 1;
			for( int cmm2 = 1 ; cmm2 < _t ; cmm2++){

				if( _v[cmm2-1] != _v[cmm2] ){
					_v[cmm1++] = _v[cmm2]; 
				}
			}
			_t = cmm1;		
		}
		_v[_t] = dmy;
	}

   void initVprem(size_t vt){

		for(size_t i=0 ; i< vt; i++){ 
			_v[i]=i; 
		}
		_t = vt;

	}
	QUEUE_ID    get_t(){return _t;}
	QUEUE_ID    get_s(){return _s;}
	QUEUE_ID    get_end(){return _end;}
	QUEUE_INT * get_v(){return _v;}
	QUEUE_INT   get_v(int i){ return _v[i];}

	QUEUE_INT * getp_v(int i){ return &_v[i];}

	QUEUE_INT    get_v_dec_t(){return _v[--_t];}


	void set_t(QUEUE_ID v){ _t=v;}
	void set_s(QUEUE_ID v){ _s=v;}

	//void cp_s2t(){ _t=_s; }

	void set_v(QUEUE_ID *v){
		//std::cerr << "sv" << std::endl;
	
		//std::cerr << "setVb " << v << std::endl;
		_v=v;
		//std::cerr << "setVa " << _v << std::endl;
	}
	void set_v(int i,QUEUE_ID v){ _v[i]=v;}

	void set_end(QUEUE_ID v){ _end=v;}
	void set_endv(QUEUE_ID end,QUEUE_ID *v){ _end=end; _v=v;  }

	void inc_t(){ _t++;}
	QUEUE_ID get_dec_t(){ return _t--;}

	void dec_t(){ _t--; }

	void inc_end(){ _end++;}

	void add_t(QUEUE_ID v){ _t+=v;}

	void minus_v(int i,int v){ _v[i] -= v;}
	void minus_t(int v){ _t -= v;}

	void cp_v(int fr,int to){ _v[to] = _v[fr]; }


	void swap_v(int i){
		QUEUE_INT swap_tmp = _v[i];
		_v[i] = _v[_t-1-i];
		_v[_t-1-i] = swap_tmp;
	}

	void move(int j){
		_v = &_v[j];
		_t -= j;
	}

	/* initialization, not fill the memory by 0 */
	void alloc (QUEUE_ID siz){
		_end =  siz+1;
		//malloc2(_v, siz+1, EXIT);
		_v = new QUEUE_ID[siz+1];

	}
	void alloc (QUEUE_ID siz , QUEUE_ID end){
		//malloc2(_v, siz+1, EXIT);
		_v = new QUEUE_ID[siz+1];
		_end =  end;
	}

	void alloc(QUEUE_ID siz , QUEUE_INT *v){
		_v = v;
		_end =  siz;
	}

	void allocBySequence(QUEUE_ID siz){

		_end =  siz+1;
		//malloc2(_v, siz+1, EXIT);
		_v = new QUEUE_ID[siz+1];
		for(size_t i=0 ; i< siz; i++){ _v[i] = i; }
		_t = siz;

	}


	void init(){
		_v = NULL;
		_end=0;
		_t=0;
		_s=0;
	}

	QUEUE dup(){
	  QUEUE QQ;
  	QQ.alloc(MAX(_t+1, _end-1));
		//  	concat_ ( Q2);
		//  	QQ.cpy_(this);
	  memcpy ( &(QQ._v[0]), &(_v[_s]), (_t-_s)*sizeof(QUEUE_INT));
	  QQ._s = 0;
	  QQ._t = _t - _s;
  	return QQ;
	}

	void MemCopy(QUEUE &Q2){
		_t=Q2.get_t();
		memcpy(_v, Q2.get_v(), sizeof(QUEUE_INT) * _t);	
	}


	void cpy(QUEUE *Q2){
  	
		_t=_s;  //QUEUE_RMALL (*this);

	  //concat(Q2);
		QUEUE_ID e = Q2->_s;
		while ( e != Q2->_t){
			_v[_t] = e;		
			_t = ( (_t>=_end-1) ? 0 : _t+1);
			if (_s == _t ) {
				fprintf(stderr,"QUEUE_ins: overflow: "QUEUE_IDF"\n",_s);
				exit(1);
			}
			e =  ( e >= Q2->_end-1) ? 0 : e+1 ;
		}

	}

	void and_(QUEUE *Q2){ //void and_  ( QUEUE *Q2);
	
	  QUEUE_ID i=_s, i2 = Q2->_s, ii=_s;
	  while ( i != _t ){
  	  if ( _v[i] > Q2->_v[i2] ){
    	  if ( ++i2 == Q2->_t ) break;
    	} else {
      	if ( _v[i] == Q2->_v[i2] ) _v[ii++] = _v[i];
     		i++;
	    }
  	}
  	_t = ii;
	}

	/***********************************************************************/
	/* duplicate occ's in jump, ( copy occ's to allocated QUEUE array) */
	/* Q[i].end := original item, clear each original occ */
	/* buffer size is multiplied by u */
	/*******************************************************/
	void occ_dup (
		QUEUE **QQ, QUEUE *Q,
		 WEIGHT **ww, WEIGHT *w, WEIGHT **ppw, WEIGHT *pw, int u);

	void occ_dupELE(
		KGLCMSEQ_QUE **QQ, KGLCMSEQ_QUE *Q,
		 WEIGHT **ww, WEIGHT *w, WEIGHT **ppw, WEIGHT *pw, int u);


	/* sort a QUEUE with WEIGHT, with already allocated memory (size have to no less than the size of QUEUE) */
	void perm_WEIGHT (WEIGHT *w, PERM *invperm, int flag);

	/* remove (or unify) the consecutive same ID's in a QUEUE (duplication delete, if sorted) */
	void rm_dup_WEIGHT (WEIGHT *w);

	/* 
		return the position of the first element having value e. 
		return -1 if no such element exists 
	*/
	LONG ele (QUEUE_INT e){
	  QUEUE_INT *x;
  	for( x=_v ; x < _v + _t ; x++){
    	if ( *x == e ) return (x - _v);
	  }
  	return (-1);
	}

	// error checkいる？
	void rm( QUEUE_ID j){
	  if ( _s <= _t ){
  	  if ( j < _s || j >= _t ) {
    		error ("QUEUE_rm: j is out of queue", EXIT);
    	}
  	} 
  	else if ( j < _s && j >= _t ) {
  		error ("QUEUE_rm: j is out of queue", EXIT);
	  }
		//void tLoopDec(){ _t = ( (_t==0) ? _end-1 : _t-1); }
  	_t = ( (_t==0) ? _end-1 : _t-1 ); 
  	_v[j] = _v[_t];
	}


	void delivery_iter( 
			WEIGHT *w, WEIGHT *pw, 
			VEC_ID t, QUEUE_INT m, 
			QUEUE *jmp,QUEUE *oq,
			WEIGHT *tw, WEIGHT *tpw,WEIGHT *y,int f)
	{

			QUEUE_INT *x;

			for( x=_v; *x <m ; x++){
				if ( oq[*x]._end == 0 ){ 
					//jmp->INS(*x); 
					jmp->_v[jmp->_t++] = *x; 

					w[*x] = 0; 
					if ( f ) pw[*x] = 0; 
   			}
    		oq[*x]._end++;

		    if ( y ){
    		  w[*x] += *y; if ( *y>0 && f) pw[*x] += *y;
      		y++;
    		}
    		else {
					w[*x] += tw[t]; 
					if ( f ) pw[*x] += tpw[t];
				}
			}			
	}

	
};

