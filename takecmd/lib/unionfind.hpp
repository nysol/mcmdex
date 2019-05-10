#pragma once

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


class UNIONFIND{

	UNIONFIND_ID *_ID;
	UNIONFIND_ID *_set;
	UNIONFIND_ID _end;
	
	
	/* get the ID of belonging group, and flatten the ID tree */
	UNIONFIND_ID _getID (UNIONFIND_ID v){
  	UNIONFIND_ID vv = v, vvv;
  	while (v != _ID[v]) v = _ID[v];  // trace parents until the root (ID[v] = v, if v is a root)
  	while (vv != _ID[vv]){ vvv = vv; vv = _ID[vv]; _ID[vvv] = v; }
  	return v;
	}
	
	public:
		UNIONFIND():_ID(NULL),_set(NULL),_end(0){}

		~UNIONFIND(){
			delete [] _ID;
			delete [] _set;
		}

		bool empty(){ return _end==0 ;} 

		UNIONFIND_ID mark(size_t i){ return _ID[i] ;} 
		UNIONFIND_ID set(size_t i){ return _set[i] ;} 


		void alloc(UNIONFIND_ID siz){

			_end = siz;
			_ID = new UNIONFIND_ID[_end];//malloc2 
			for(int i=0; i<_end ;i++){ _ID[i] = i; }

			_set = new UNIONFIND_ID[_end];//malloc2 
			for(int i=0; i<_end ;i++){ _set[i] = i; }
		}

		void unify_set(UNIONFIND_ID u,UNIONFIND_ID v){
		  UNIONFIND_ID z;
  		v = _getID (v); // compute ID of v 
  		u = _getID (u); // compute ID of u 
	  	if ( u != v ){
  			if ( _set[u] == u ){ _set[u] = v; _ID[v] = u; } // attach u as the head of the list of v
    		else if ( _set[v] == v ){ _set[v] = u; _ID[u] = v; }  // attach v as the head of the list of u
    		else {
      		for (z=v; _set[z]!=z ; z=_set[z]);  // find the last in the list of v
      		_set[z] = _set[u]; _set[u] = v; // insert the list of v to list of u
      		_ID[v] = u; // set ID of (ID of v) to (ID of u)
    		}
	  	}
		}
	
};