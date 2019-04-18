/*
    array-based simple heap (fixex size)
            12/Apr/2001   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */


#include"aheap.hpp"
#include"stdlib2.hpp"

/******************************************************************************/
/* IHEAP: variable size, index heap  */
/******************************************************************************/

/* print heap keys according to the structure of the heap */
void IHEAP::print (FILE *fp){

  IHEAP_ID i=0, j=1, ii;
  fprintf (fp, "siz:" IHEAP_IDF ", end: " IHEAP_IDF "\n", _siz, _end);
  while (i < _siz){
    ii = MIN (i+j, _siz);
    while (i < ii){
      fprintf (fp, IHEAP_KEYF "(" IHEAP_IDF "),", _x[i], _v[i]);
      i++;
    }
    fprintf (fp, "\n");
    j = j*2;
  }

}

/* allocate memory */
void IHEAP::alloc (IHEAP_ID num, int mode, IHEAP_KEY *x){

  if (num == 0) num = 16;
  //malloc2 (_v, num, EXIT);
  _v = malloc2 (_v, num);

  _end = num;
  _mode = mode;
  _x = x;
}

/* heap key comparison */
int IHEAP::compare (IHEAP_ID a, IHEAP_ID b){
 	if ( _mode == 1 ) return (_x[a] <= _x[b]);
  if ( _mode == 2 ) return (_x[a] >= _x[b]);
  return 0;
}


/* update IHEAP for decrease/increase of the value of i-th node to j
   return the index to that j is written */
IHEAP_ID IHEAP::dec ( IHEAP_ID i, IHEAP_ID j){

  IHEAP_ID ii;
  while (i > 0){
    ii = (i-1)/2;
    if ( compare ( _v[i], j) ) break;
    _v[i] = _v[ii];
    i = ii;
  }
  _v[i] = j;
  return (i);
}

IHEAP_ID IHEAP::inc ( IHEAP_ID i, IHEAP_ID j){

  IHEAP_ID jj, j1, j2, end = (_siz-1)/2;

  while (i < end){
    j1 = i*2+1; j2 = j1+1;
    jj = compare (_v[j1], _v[j2])? j1: j2;
    if ( compare ( j, _v[jj])) goto END;
    _v[i] = _v[jj];
    i = jj;
  }
  if (i == end && (_siz&1)==0){
    jj = i*2+1;
    if ( !compare (_v[jj], j)){ _v[i] = _v[jj]; i = jj; }
  }
  END:;
  _v[i] = j;
  return (i);
}



/* change the value of i-th node to j */
IHEAP_ID IHEAP::chg ( IHEAP_ID i, IHEAP_ID j){

  if ( _end <= 0 ) return (IHEAP_ID_HUGE);

  if ( compare ( j, _v[i]) ) return (dec ( i, j));
  else return (inc (i, j));

}

/* insert/delete an element */
IHEAP_ID IHEAP::ins ( IHEAP_ID j){
  _siz++;
  if (_siz >= _end){ 
  	_end = MAX(_end*2, _siz+1); 
  	
  	//realloc2 (_v, _end, exit(1));
  	_v = realloc2(_v, _end);
  }
  return dec(_siz-1, j);
}

