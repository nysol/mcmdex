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


#include"queue.hpp"
#include"stdlib2.hpp"


/* tranpose the matrix ; counting/transpose/memory_allocate */
/* OQ: QUEUE to transposed matrix, c: count the column sizes if its given
   jump: QUEUE to store ID's of non-empty columns, Q: the matrix, occ: the rows to be transposed
   t: if occ=NULL, Q[0] to Q[t-1] will be transposed, M: maximum column ID to be transposed */
//
void QUEUE::delivery( QUEUE *OQ, VEC_ID *c, QUEUE *jump, QUEUE *Q, QUEUE *occ, VEC_ID t, QUEUE_INT M){  
	VEC_ID  e;
  QUEUE_INT *x;

  //FLOOP(i, 0, occ? occ->_t: _t){
	for(VEC_ID i =0 ; i < (occ? occ->_t: _t) ;i++){
    e = occ? occ->_v[i]: i;

    if ( c ){
      if ( jump ){ 
				for (x = Q[e]._v ; *x < M ; x++){
      		if ( c[*x]==0 ) jump->push_back(*x); 
      		c[*x]++; 
      	}
      } 
      else { 
				for (x = Q[e]._v ; *x < M ; x++){  c[*x]++; }
      }
    }
    else {
      if ( jump ){ 
				for (x = Q[e]._v ; *x < M ; x++){
      		if ( OQ[*x]._t==0 ) jump->push_back(*x); 
      		OQ[*x].push_back(e); 
      	}
      }
      else{
      		for (x = Q[e]._v ; *x < M ; x++){ OQ[*x].push_back(e); }
      }
    }
  }
}


/* sort a QUEUE with WEIGHT, with already allocated memory */
void QUEUE::perm_WEIGHT (WEIGHT *w, PERM *invperm, int flag){
  WEIGHT y;
  int cmn_i2;
  if ( w ){
    // ARY_INIT_PERM (invperm, _t);
		for(size_t i=0 ; i<_t; i++){ invperm[i]=i; }

    qsort_perm__(_v, _t, invperm, flag);

		//FLOOP(cmn_i,0,_t){ 
		for(int cmn_i=0; cmn_i< _t; cmn_i++ ){
			if ( invperm[cmn_i]<_t ){ 
				y=w[cmn_i]; 
				do { 
					cmn_i2=cmn_i; 
					cmn_i=invperm[cmn_i]; 
					w[cmn_i2]=w[cmn_i]; 
					invperm[cmn_i2]=_t; 
				}while ( invperm[cmn_i]<_t ); 
				w[cmn_i2] = y;
			}
		}
  }
  qsort_<INT> (_v, _t, flag);

}

/* 
	remove (or unify) the consecutive same ID's in a QUEUE (duplication delete, if sorted) 
*/
void QUEUE::rm_dup_WEIGHT (WEIGHT *w){
  VEC_ID j, jj=0;
  if ( w ){
    //FLOOP (j, 1, _t){
		for(j=1;j<_t;j++){
      if ( _v[j-1] != _v[j] ){
        _v[++jj] = _v[j];
        w[jj] = w[j];
      } else w[jj] += w[j];
    }
  }
  else{
  	//FLOOP (j, 1, _t){
		for(j=1;j<_t;j++){
    	if ( _v[j-1] != _v[j] ) _v[++jj] = _v[j];
	  }
	}
  if ( _t>0 ) _t = jj+1;
}

/***********************************************************************/
/* duplicate occ's in jump, ( copy occ's to allocated QUEUE array) */
/* Q[i].end := original item, clear each original occ */
/* buffer size is multiplied by u */
/*******************************************************/
// あとまわし
//QUEUE *jump,
void QUEUE::occ_dup ( QUEUE **QQ, QUEUE *Q, WEIGHT **ww, WEIGHT *w, WEIGHT **ppw, WEIGHT *pw, int u){
  QUEUE_ID i, l=_t-_s; //QUEUE_LENGTH_(*jump);
  size_t cnt=0;
  QUEUE_INT e, *x;
  char *buf;

  int unit = sizeof(*Q) + (w?sizeof(*w):0) + (pw?sizeof(*pw):0);
 
  ENMAX (u, sizeof(*x));

	for(x=_v; x < _v+_t ; x++) cnt += Q[*x]._t;

  if ( cnt == 0 ){ *QQ=NULL; return; }

  //buf = malloc2 (buf, l*unit + (cnt+l)*u);
	if(!( buf = (char*)malloc(sizeof(char)* ( l*unit + (cnt+l)*u ) ))){
		throw("memory allocation error : malloc2");
	}


  *QQ = (QUEUE*)buf; 
  buf += sizeof(*Q) * l;


  if ( w ){ *ww = (WEIGHT *)buf; buf += sizeof(*w)*l; }
  if ( pw ){ *ppw = (WEIGHT *)buf; buf += sizeof(*pw)*l; }
  for (i=0 ; i< _t ; i++){
    e = _v[i];
    (*QQ)[i]._end = e;
    (*QQ)[i]._v = (QUEUE_INT *)buf;
    (*QQ)[i]._t = Q[e]._t;
    memcpy (buf, Q[e]._v, (Q[e]._t+1)*u);
    buf += (Q[e]._t+1) *u;
    if ( w ) (*ww)[i] = w[e];
    if ( pw ) (*ppw)[i] = pw[e];
  }

}


//QUEUE *jump,
void QUEUE::occ_dupELE ( 
	KGLCMSEQ_QUE **QQ, KGLCMSEQ_QUE *Q, 
	WEIGHT **ww, WEIGHT *w, WEIGHT **ppw, WEIGHT *pw, int u){
  QUEUE_ID i, l=_t-_s; //QUEUE_LENGTH_(*jump);
  size_t cnt=0;
  QUEUE_INT e, *x;
  char *buf;

  int unit = sizeof(*Q) + (w?sizeof(*w):0) + (pw?sizeof(*pw):0);
 
  ENMAX (u, sizeof(*x));

  //MQUE_FLOOP (*jump, x) cnt += Q[*x].t;

	for(x=_v; x < _v+_t ; x++) cnt += Q[*x].get_t();
  if ( cnt == 0 ){ *QQ=NULL; return; }

  //buf = malloc2 (buf, l*unit + (cnt+l)*u);

	if(!( buf = (char*)malloc(sizeof(char)*( l*unit + (cnt+l)*u) ))){
		throw("memory allocation error : malloc2");
	}

  *QQ = (KGLCMSEQ_QUE*)buf; 
  buf += sizeof(*Q) *l;

  if ( w ){ 
  	*ww = (WEIGHT *)buf; 
  	buf += sizeof(*w)*l; 
  }
  if ( pw ){ 
  	*ppw = (WEIGHT *)buf; 
  	buf += sizeof(*pw)*l; 
  }

  for (i=0 ; i< _t ; i++){
    e = _v[i];    
    (*QQ)[i].set_end(e);
    (*QQ)[i].set_v((KGLCMSEQ_ELM *)buf);
    (*QQ)[i].set_t(Q[e].get_t());
    memcpy(buf, Q[e].get_v(), (Q[e].get_t()+1)*u);
    buf += (Q[e].get_t()+1) *u;
    if ( w ) (*ww)[i] = w[e];
    if ( pw ) (*ppw)[i] = pw[e];
  }

}
