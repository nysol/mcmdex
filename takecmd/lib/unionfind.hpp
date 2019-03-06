
class UNIONFIND{
	UNIONFIND_ID *_ID
	UNIONFIND_ID *_set
	UNIONFIND_ID _end;
	
	
	/* get the ID of belonging group, and flatten the ID tree */
	UNIONFIND_ID UNIONFIND_getID (UNIONFIND_ID v){
  	UNIONFIND_ID vv = v, vvv;
  	while (v != _ID[v]) v = _ID[v];  // trace parents until the root (ID[v] = v, if v is a root)
  	while (vv != _ID[vv]){ vvv = vv; vv = _ID[vv]; _ID[vvv] = v; }
  	return v;
	}
	
	public:
	void alloc(UNIONFIND_ID siz){
		int i;
		_end = siz
		malloc2 (_ID, _end, EXIT);
		FLOOP (i, 0, end) _ID[i] = i;

		malloc2 (_set, _end, EXIT);
		FLOOP (i, 0, end) _set[i] = i;
	}

	void unify_set(UNIONFIND_ID u,UNIONFIND_ID v){
	  UNIONFIND_ID z;
  	v = getID (v); // compute ID of v 
  	u = getID (u); // compute ID of u 
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