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
	VEC_ID i, e;
  QUEUE_INT *x;

  FLOOP(i, 0, occ? occ->_t: _t){

    e = occ? occ->_v[i]: i;

    if ( c ){
      if ( jump ){ 
      	MLOOP (x, Q[e]._v, M){ 
      		if ( c[*x]==0 ) jump->push_back(*x); 
      		c[*x]++; 
      	}
      } 
      else { 
      	MLOOP (x, Q[e]._v, M) c[*x]++; 
      }
    }
    else {
      if ( jump ){ 
      	MLOOP (x, Q[e]._v, M){ 
      		if ( OQ[*x]._t==0 ) jump->push_back(*x); 
      		OQ[*x].push_back(e); 
      	}
      }
      else{
      	 MLOOP (x, Q[e]._v, M){ OQ[*x].push_back(e); }
      }
    }
  }
}


/* sort a QUEUE with WEIGHT, with already allocated memory */
void QUEUE::perm_WEIGHT (WEIGHT *w, PERM *invperm, int flag){
  WEIGHT y;
  if ( w ){
    // ARY_INIT_PERM (invperm, _t);
		for(size_t i=0 ; i<_t; i++){ invperm[i]=i; }
    qsort_perm__<QUEUE_INT> (_v, _t, invperm, flag);
    ARY_INVPERMUTE_ (w, invperm, y, _t);


  }
  qsort_<INT> (_v, _t, flag);
}

/* 
	remove (or unify) the consecutive same ID's in a QUEUE (duplication delete, if sorted) 
*/
void QUEUE::rm_dup_WEIGHT (WEIGHT *w){
  VEC_ID j, jj=0;
  if ( w ){
    FLOOP (j, 1, _t){
      if ( _v[j-1] != _v[j] ){
        _v[++jj] = _v[j];
        w[jj] = w[j];
      } else w[jj] += w[j];
    }
  } else FLOOP (j, 1, _t){
    if ( _v[j-1] != _v[j] ) _v[++jj] = _v[j];
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

  //MQUE_FLOOP (*jump, x) cnt += Q[*x].t;

	for(x=_v; x < _v+_t ; x++) cnt += Q[*x]._t;
  if ( cnt == 0 ){ *QQ=NULL; return; }

  malloc2 (buf, l*unit + (cnt+l)*u, EXIT);

  *QQ = (QUEUE*)buf; buf += sizeof(*Q) *l;
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
void QUEUE::occ_dupELE ( KGLCMSEQ_QUE **QQ, KGLCMSEQ_QUE *Q, WEIGHT **ww, WEIGHT *w, WEIGHT **ppw, WEIGHT *pw, int u){
  QUEUE_ID i, l=_t-_s; //QUEUE_LENGTH_(*jump);
  size_t cnt=0;
  QUEUE_INT e, *x;
  char *buf;

  int unit = sizeof(*Q) + (w?sizeof(*w):0) + (pw?sizeof(*pw):0);
 
  ENMAX (u, sizeof(*x));

  //MQUE_FLOOP (*jump, x) cnt += Q[*x].t;

	for(x=_v; x < _v+_t ; x++) cnt += Q[*x]._t;
  if ( cnt == 0 ){ *QQ=NULL; return; }

  malloc2 (buf, l*unit + (cnt+l)*u, EXIT);

  *QQ = (KGLCMSEQ_QUE*)buf; buf += sizeof(*Q) *l;
  if ( w ){ *ww = (WEIGHT *)buf; buf += sizeof(*w)*l; }
  if ( pw ){ *ppw = (WEIGHT *)buf; buf += sizeof(*pw)*l; }
  for (i=0 ; i< _t ; i++){
    e = _v[i];
    (*QQ)[i]._end = e;
    (*QQ)[i]._v = (KGLCMSEQ_ELM *)buf;
    (*QQ)[i]._t = Q[e]._t;
    memcpy (buf, Q[e]._v, (Q[e]._t+1)*u);
    buf += (Q[e]._t+1) *u;
    if ( w ) (*ww)[i] = w[e];
    if ( pw ) (*ppw)[i] = pw[e];
  }

}
/// ======= 以下の必要なら復活させる =================
/* return the position of the first element having value e. return -1 if no such element exists */
/*
LONG QUEUE::ele (QUEUE_INT e){
  QUEUE_INT *x;
  //MQUE_FLOOP (*Q, x)
  for( x=_v; (x)<_v+_t ; x++){
    if ( *x == e ) return (x - _v);
  }
  return (-1);
}
*/
/* insert an element to the tail */
/*
void QUEUE::ins_ (QUEUE_INT e){
  _v[_t] = e;
  _t++;
}
void QUEUE::ins (QUEUE_INT e){
  _v[_t] = e;
  QUE_t_INC();
  if (_s == _t ) error_num ("QUEUE_ins: overflow", _s, EXIT);
}
*/
/* insert an element to the head */
/*
void QUEUE::ins_head_ (QUEUE_INT e){
  _s--;
  _v[_s] = e;
}
void QUEUE::ins_head (QUEUE_INT e){
	QUE_s_DEC();
  _v[_s] = e;
  if (_s == _t ) error_num ("QUEUE_ins_head: underflow", _s, EXIT);
}
*/
/* extract an element from the head, without checking underflow */
/*QUEUE_INT QUEUE::ext_(){
  _s++;
  return (_v[_s-1]);
}

QUEUE_INT QUEUE::ext (){
  QUEUE_INT e;
  if (_s == _t ) error_num ("QUEUE_ext: empty queue", _s, EXIT0);
  e = _v[_s];
 	QUE_s_INC();
  return ( e);
}
*/
/* extract an element from the tail, without checking underflow */
//QUEUE_INT QUEUE::ext_tail_ (){
//  (_t)--;
//  return (_v[_t]);
//}

//QUEUE_INT QUEUE::ext_tail (){
//  if ( _s == _t ) error_num ("QUEUE_ext_tail: empty queue", _s, EXIT0);
//  QUE_t_DEC();
//  return (_v[_t]);
//}

/* remove the j-th element and replace it by the tail */
/*
void QUEUE::rm_ (QUEUE_ID j){
  _t--;
  _v[j] = _v[_t];
}
void QUEUE::rm ( QUEUE_ID j){
  if ( _s <= _t ){
    if ( j < _s || j >= _t ) error ("QUEUE_rm: j is out of queue", EXIT);
  } else if ( j < _s && j >= _t ) error ("QUEUE_rm: j is out of queue", EXIT);
  QUE_t_DEC();
  _v[j] = _v[_t];
}
*/
/* remove the j-th element and replace it by the head */
/*
void QUEUE::rm_head_ ( QUEUE_ID j){
  _v[j] = _v[_s];
  _s++;
}
void QUEUE::rm_head ( QUEUE_ID j){
  if ( _s <= _t ){
    if ( j < _s || j >= _t ) error ("QUEUE_rm: j is out of queue", EXIT);
  } else if ( j < _s && j >= _t ) error ("QUEUE_rm: j is out of queue", EXIT);
  _v[j] = _v[_s];
  QUE_s_INC();
}
*/
/* remove the j-th element and shift the following elements to fill the gap */
/*
int QUEUE::rm_ele_ (QUEUE_INT e){
  QUEUE_ID i;
	//#define QUEUE_F_LOOP_(Q,i)  for((i)=(Q)._s;(i)<(Q)._t;(i)++)
	for(i=_s;i<_t;i++){
    if ( _v[i] == e ){
      memcpy ( &(_v[i]), &(_v[i+1]), (_t-i-1)*sizeof(QUEUE_INT));
      _t--;
      return (1);
    }
  }
  return (0);
}  
*/
/* insert e to the position determined by the increasing order of elements */
/*
void QUEUE::ins_ele_(QUEUE_INT e){
  QUEUE_ID i;
  QUEUE_INT ee;

	//#define QUEUE_BE_LOOP_(Q,i,x)  for((i)=(Q)._t-1;((i)>=(Q)._s)?((x=(Q)._v[i])||1):0;(i)--)
	for(i=_t-1; (i>=_s)?((ee=_v[i])||1):0; i--){ // ||でいい？

    if ( ee<e ) break;
    _v[i+1] = ee;
  }
  _v[i+1] = e;
  _t++;
}
*/

/* Append Q2 to the tail of Q1. Q2 will not be deleted */
//void QUEUE::concat_ (QUEUE *Q2){
//  memcpy ( &(_v[_t]), &(Q2->_v[Q2->_s]), (Q2->_t-Q2->_s)*sizeof(QUEUE_INT));
//  _t += Q2->_t-Q2->_s;
//}
//void QUEUE::concat ( QUEUE *Q2){
//  QUEUE_ID e = Q2->_s;
//  while ( e != Q2->_t){
//    ins ( Q2->_v[e]);
//    e = Q2->QUE_INC(e);
//    //QUEUE_INCREMENT(*Q2,e);
//  }
//}

/* Append Q2 to the tail of Q1. Q2 will be deleted */
//void QUEUE::append_ (QUEUE *Q2){
//  concat_(Q2);
//	//#define QUEUE_RMALL(Q) ((Q)._t=(Q)._s)
//	Q2->RMALL();
//  //QUEUE_RMALL (*Q2);
//}
//void QUEUE::append (QUEUE *Q2){ // more improvement can be
//  while ( Q2->_s != Q2->_t )
//      ins (Q2->ext());
//}

/* Append from j to jj th elements to the tail of Q1. Q2 will not be deleted */
//void QUEUE::subconcat_ ( QUEUE *Q2, QUEUE_ID j, QUEUE_ID jj){
//  for ( ; j<=jj ; j++){
//    _v[_t] = Q2->_v[j];
//    _t++;
//  }
//}
//void QUEUE::subconcat ( QUEUE *Q2, QUEUE_ID j, QUEUE_ID jj){
//  while (1){
//    ins (Q2->_v[j]);
//    if ( j == jj ) break;
//    j = Q2->QUE_INC(j);
// } 
//}

/* initialize Q1 by length of Q2, and copy Q2 to Q1 */
//QUEUE *Q1,
/*あとまわし
void QUEUE::store_ ( QUEUE *Q2){
  QUEUE_alloc (Q1, QUEUE_LENGTH(*Q2));
  QUEUE_concat_ (Q1, Q2);
}
*/
//QUEUE *Q1,
/*あとまわし
void QUEUE_store ( QUEUE *Q2){
  QUEUE_alloc (Q1, QUEUE_LENGTH(*Q2));
  QUEUE_concat (Q1, Q2);
}
*/
/* copy Q2 to Q1 and delete Q2 */
//QUEUE *Q1, 
/*あとまわし
void QUEUE_restore_ (QUEUE *Q2){
  QUEUE_RMALL (*Q1);
  QUEUE_concat_ (Q1, Q2);
  QUEUE_end (Q2);
}
*/
//QUEUE *Q1,
/*
void QUEUE_restore ( QUEUE *Q2){
  QUEUE_RMALL (*Q1);
  QUEUE_concat (Q1, Q2);
  QUEUE_end (Q2);
}
*/

/* copy Q2 to Q1 */
/*
void QUEUE::cpy_ ( QUEUE *Q2){
  _s = _t = 0;
  concat_ ( Q2);
}
void QUEUE::cpy (QUEUE *Q2){
  //QUEUE_RMALL (*this);
	RMALL();

  concat(Q2);
}
*/
/* compare two queues */
/*
int QUEUE::cmp_ (QUEUE *Q2){
  QUEUE_INT *x, *y=Q2->_v;
	
	//MQUE_FLOOP(V,z)    for((z)=(V).v;(z)<(V).v+(V).t ; (z)++)
  //MQUE_FLOOP (*this, x){
  for(x=_v;(x)<_v+_t;x++){
    if ( *x != *y ) return (0);
    y++;
  }
  return (1);
}
*/

/* copy l elements of Q2 starting from s2 to the s1th position of Q1.
   size of Q1 is not increasing */
/*
void QUEUE::subcpy_ (QUEUE_ID s1, QUEUE *Q2, QUEUE_ID s2, QUEUE_ID l){
  memcpy ( &(_v[s1]), &(Q2->_v[s2]), (l-s2)*sizeof(QUEUE_INT));
}
void QUEUE::subcpy ( QUEUE_ID s1, QUEUE *Q2, QUEUE_ID s2, QUEUE_ID l){
  for ( ; s2!=l ; s1 = QUE_INC(s1), s2 = Q2->QUE_INC(s2) )
      _v[s1] = Q2->_v[s2];
  _v[s1] = Q2->_v[s2];
}
*/
/* duplicate Q2 to Q1. The memory size will be the length of Q2 */
/*
QUEUE QUEUE::dup_ (){
  QUEUE QQ;
  QQ.alloc(MAX(_t+1, _end-1));
  QQ.cpy_(this);
  return (QQ);
}
*/
/* merge Q1 and Q2 by insert all elements of Q2 to Q1 with deleting duplications. Both Q1 and Q2 have to be sorted in increasing order */
/*
void QUEUE::merge_ ( QUEUE *Q2){

  QUEUE_ID i=_t-1, j=Q2->_t-1, t=i+j-Q2->_s+1;
  QUEUE_INT ei, ej;
  if ( i+1 == _s || j+1 == _s ){
    concat_ (Q2);
    return;
  }
  _t = t+1;
  ei = _v[i];
  ej = Q2->_v[j];
  while (1){
    if ( ei > ej ){
      _v[t] = ei;
      if ( i == _s ){
        subcpy_ ( _s, Q2, Q2->_s, j);
        return;
      }
      i--;
      ei = _v[i];
    } else {
      _v[t] = ej;
      if ( j == Q2->_s ) return;
      j--;
      ej = Q2->_v[j];
    }
    t--;
  }
}
void QUEUE::merge (QUEUE *Q2){
  QUEUE_ID i=_t, j=Q2->_t;
  QUEUE_INT ei, ej;
  QUEUE_ID t = (_t + Q2->LENGTH()-1) % _end;
  if ( LENGTH() + Q2->LENGTH() >= _end ){
    print_err ("QUEUE_merge: overflow Q1->end="QUEUE_INTF", Q1length="QUEUE_INTF", Q2length="QUEUE_INTF"\n", _end, LENGTH(), Q2->LENGTH());
    exit (1);
  }
  if ( i == _s || j == Q2->_s ){
    concat (Q2);
    return;
  }

  _t = t;
  i = QUE_DEC(i);
  j = Q2->QUE_DEC(j);

  ei = _v[i];
  ej = Q2->_v[j];
  while (1){
    if ( ei > ej ){
      _v[t] = ei;
      if ( i == _s ){
        subcpy ( _s, Q2, Q2->_s, (j+Q2->_end-Q2->_s)%Q2->_end);
        return;
      }
      i = QUE_DEC(i);
      ei = _v[i];
    } else {  
      _v[t] = ej;
      if ( j == Q2->_s ) return;
      j=Q2->QUE_DEC(j);
      ej = Q2->_v[j];
    }
    QUE_t_DEC();
  }
}
*/

/* delete all elements of Q1 included in Q2.
 both Q1 and Q2 have to be sorted in increasing order */
/*
void QUEUE::minus_ (QUEUE *Q2){
  QUEUE_ID i=_s, i2 = Q2->_s, ii=_s;
  while ( i != _t && i2 != Q2->_t){
    if (_v[i] > Q2->_v[i2] ) i2++;
    else {
      if (_v[i] < Q2->_v[i2] ){
        _v[ii] = _v[i];
        ii++;
      }
      i++;
    }
  }
  while ( i != _t ){
    _v[ii] = _v[i];
    i++;
    ii++;
  }
  _t = ii;
}
void QUEUE::minus (QUEUE *Q2){
  QUEUE_ID i=_s, i2 = Q2->_s, ii=_s;
  while ( i != _t && i2 != Q2->_t ){
    if ( _v[i] > Q2->_v[i2] ) i2 = Q2->QUE_INC(i2);
    else {
      if ( _v[i] < Q2->_v[i2] ){
        _v[ii] = _v[i];
        ii = QUE_INC(ii);
      }
      i = QUE_INC(i);
    }
  }
  while ( i != _t ){
    _v[ii] = _v[i];
    i = QUE_INC(i);
    ii = QUE_INC(ii);
  }
  _t = ii;
}
*/

/* Delete all elements of Q1 which are not included in Q2. 
 both Q1 and Q2 have to be sorted in increasing order */
/*
QUEUE_ID QUEUE::intsec_( QUEUE *Q2){
  QUEUE_ID i=_s, i2 = Q2->_s, c=0;
  while ( i != _t ){
    if ( _v[i] > Q2->_v[i2] ){
      if ( ++i2 == Q2->_t ) break;
    } else {
      if ( _v[i] == Q2->_v[i2] ) c++;
      i++;
    }
  }
  return (c);
}
void QUEUE::and_ (QUEUE *Q2){
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
void QUEUE::_and (QUEUE *Q2){
  QUEUE_ID i=_s, i2 = Q2->_s, ii=_s;
  while ( i != _t && i2 != Q2->_t){
    if ( _v[i] > Q2->_v[i2] ) i2 = Q2->QUE_INC(i2);
    else {
      if ( _v[i] == Q2->_v[i2] ){
        _v[ii] = _v[i];
        ii = QUE_INC(ii);
      }
      i = QUE_INC(i);
    }
  }
  _t = ii;
}

*/

/* insertion sort */
/*
void QUEUE::sort (){
  QUEUE_ID i = _s, j, jj;
  QUEUE_INT e;
  if ( i== _t ) return;
  i=QUE_INC(i);
  for ( ; i!=_t ; i=QUE_INC(i) ){
    e=_v[i]; 
    j=i; 
    while (1){
      jj = j;
      j=QUE_DEC(j);
      if ( _v[j] <= e ) { _v[jj] = e; break; }
      _v[jj] = _v[j];
      if ( j == _s) { _v[j] = e; break; }
    }
  }
}
*/

/* print a queue */
/*
void QUEUE::print (){
  QUEUE_ID i;
  for ( i=_s ; i!=_t ; ){
    printf (QUEUE_INTF" ", _v[i]);
    i++;
    //i=QUE_INC(i);
  }
  printf ("\n");
}
*/
/* permutation version */
/*
void QUEUE::perm_print (QUEUE_ID *q){
  QUEUE_ID i;
  for ( i=_s ; i!=_t ; ){
    printf (QUEUE_INTF" ", q[_v[i]]);
    i=QUE_INC(i);
  }
  printf ("\n");
}
void QUEUE::printn (){
  QUEUE_ID i;
  for ( i=_s ; i!=_t ; ){
    printf (QUEUE_INTF" ", _v[i]);
    i=QUE_INC(i);
  }
}
void QUEUE::perm_printn ( QUEUE_ID *q){
  QUEUE_ID i;
  for ( i=_s ; i!=_t ; ){
    printf (QUEUE_INTF" ",q[_v[i]]);
    i=QUE_INC(i);
  }
}
*/
/*

void QUEUE::print_(){
  QUEUE_ID i;
  printf("s="QUEUE_IDF",t="QUEUE_INTF": ", _s, _t);
  for ( i=_s ; i!=_t ; ){
    printf (QUEUE_INTF" ",_v[i]);
    //i=QUE_INC(i);
    i++;
  }
  printf ("\n");
}
void QUEUE::print__(){
  QUEUE_ID i;
  printf("s="QUEUE_IDF",t="QUEUE_IDF": ", _s, _t);
  for ( i=_s ; i!=_t ; i++ ) printf (QUEUE_INTF" ",_v[i]);
  printf ("\n");
}
int qqsort_cmp_VECt (const void *x, const void *y){
  if ( QQSORT_ELEt(QUEUE,x) < QQSORT_ELEt(QUEUE,y) ) return (-1);
  else return ( QQSORT_ELEt(QUEUE,x) > QQSORT_ELEt(QUEUE,y) );
}
int qqsort_cmp__VECt (const void *x, const void *y){
  if ( QQSORT_ELEt(QUEUE,x) > QQSORT_ELEt(QUEUE,y) ) return (-1);
  else return ( QQSORT_ELEt(QUEUE,x) < QQSORT_ELEt(QUEUE,y) );
}

void qsort_perm__VECt (QUEUE *v, size_t siz, PERM *perm, int unit){
  if ( unit == 1 || unit==-1 ) unit *= sizeof(PERM);
	printf("uniuni %d\n",unit);

  common_INT=MAX(unit,-unit); common_pnt=(char *)v;
  if (unit<0) qsort (perm, siz, sizeof(PERM), qqsort_cmp__VECt);
  else qsort (perm, siz, sizeof(PERM), qqsort_cmp_VECt);
}
*/