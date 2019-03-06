/* library for standard macros and functions */
/* by Takeaki Uno 2/22/2002, e-mail: uno@nii.jp
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#include"stdlib2.hpp"
#include"file2.hpp"
#ifdef MTWISTER
#include"dSFMT.c"
#endif



short FILE2_POW[5] = {1, 10, 100, 1000, 10000};

/* fast file routine, getc, putc, puts, */
int FILE2::getc(){
  int c;

  if ( _buf >= _buf_end ){
    if ( (_buf_end < _buf_org+FILE2_BUFSIZ) && (_buf_end>=_buf_org) ){ FILE_err=2; return (-1); }
    _buf = _buf_org;
    _buf_end = _buf_org + fread (_buf, 1, FILE2_BUFSIZ, _fp);
    return getc ();
  }
  c = (unsigned char)(*(_buf));
  _buf++;
  return (c);
}
/* Read an integer/a double from the file and return it,
    with read through the non-numeric letters.
   If it reaches to the end-of-file just after reading a number, then set FILE_err=2,
   if it reads a newline just after reading a number, then set FILE_err=1.
   If read either the end-of-file or newline before reading an integer,
   return 5, and 6 */
FILE_LONG FILE2::read_int(){
  FILE_LONG item;
  int sign=1, ch;
  FILE_err = 0;
  while (1){
    ch = getc();
    if ( ch>='0' && ch<='9' ) break;
    if ( ch == '\n' ){ FILE_err = 5; return (-INTHUGE); }
    if ( ch < 0 ){ FILE_err = 6; return (-INTHUGE); }
    if ( ch=='-' ) sign = -1;
  }
  for (item=ch-'0' ; 1 ; item=item*10 +ch-'0'){
    ch = getc();
    if ( ch<'0' || ch>'9' ){
      if ( ch == '\n' ) FILE_err = 1;
      if ( ch < 0 ) FILE_err = 2;
      return (item*sign);
    }
  }
}


double FILE2::read_double (){
  double item, geta=1;
  int sign=1, ch, n=0, d, flag=0;
  FILE_err = 0;
  while (1){
    ch = getc();
    if ( ch<'0' || ch>'9' ){
      if ( ch == '\n' ){ FILE_err = 5; return (-DOUBLEHUGE); }
      if ( ch < 0 ){ FILE_err = 6; return (-DOUBLEHUGE); }
      if ( ch=='-' ) sign = -1;
      else if ( ch=='.' ){ flag = 1; geta = 0.1; }
      else { sign = 1; geta = 1; flag = 0; }
    } else break;
  }
  item = 0; d = 0; item = ch-'0'; if ( flag ) item /= 10;

  while (1){
    ch = getc();
    if ( ch < '0' || ch > '9' ){
      if ( ch == '\n' ){ FILE_err = 1; break; }
      else if ( ch < 0 ){ FILE_err = 2; break; }
      else if ( ch == '.' ){  // decimal
        if ( flag ) break;   // second decimal; illigal syntax
        if ( d ) item = item * FILE2_POW[d] + n;
        flag = 1; n = 0; d = 0;
      } else if ( (ch & 0xdf) == 'E' ){  // power
        if ( d ){
          if ( flag ) item += geta * n / FILE2_POW[d];
          else item = item * FILE2_POW[d] + n;
        }
        flag = n = 0;
        ch = getc();
        if ( ch == '-' ){ flag = 1; ch = getc(); } // power is minus
        else if ( ch == '+' ) ch = getc(); // power is plus
        else if ( ch < '0' || ch > '9') break;
        for ( n=ch-'0' ; 1 ; n=n*10 + ch-'0' ){
          ch = getc();
          if ( ch<'0' || ch>'9' ){
            if ( ch == '\n' ) FILE_err = 1;
            if ( ch < 0 ) FILE_err = 2;
            if ( flag ){
              while ( n>=9 ){ item /= 1000000000.0; n -= 9; }
              while ( n>=3 ){ item /= 1000.0; n -= 3; }
              while ( n>=1 ){ item /= 10.0; n --; }
            } else {
              while ( n>=9 ){ item *= 1000000000.0; n -= 9; }
              while ( n>=3 ){ item *= 1000.0; n -= 3; }
              while ( n>=1 ){ item *= 10.0; n --; }
            }
            return (item*sign);
          }
        }
      } else break;
    } else {
      if ( n>0 ) n *= 10;
      n += ch - '0';
      if ( ++d == 5 ){
        if ( flag ){
          geta /= 100000.0;
          item += geta * n;
        } else item = item * 100000.0 + n;
        n = d = 0;
      }
    }
  }
  if ( d ){
    if ( flag ) item += geta * n / FILE2_POW[d];
    else item = item * FILE2_POW[d] + n;
  }
  return (item*sign);

}

/* read a WEIGHT from file */
WEIGHT FILE2::read_WEIGHT (){
	#ifdef WEIGHT_DOUBLE
  	return (read_double());
	#else
  	return ((WEIGHT)read_int());
	#endif
}

void FILE2::flush_last (){
  if ( _bit > 0 ) _buf++;
  if ( _buf > _buf_org ){
    fwrite (_buf_org, _buf-_buf_org, 1, _fp);
    _buf = _buf_org;
  }
}

void FILE2::flush_ (){
  fwrite (_buf_org, _buf-_buf_org, 1, _fp);
  *_buf_org = *_buf;
  _buf = _buf_org;
}


static void STR_print_int_ (char **s, LONG n, int len, char c){
  char *ss;
  if ( c ){ **s = c; (*s)++; }
  if ( n == 0 ){ **s = '0'; (*s)++; return; }
  if ( n < 0 ){ **s = '-'; (*s)++; n = -n; }
  (*s)+=len;
  ss = (*s)-1;
  **s = 0;
  while ( len>0 ){ *ss = '0'+(char)(n%10); ss--; n/=10; len--; }
}

/* fast file routine, print number, c is the char to be printed preceding to the number
   if c==0, nothing will be printed preceding the number
   if len<0 then the #digits following '.' does not change (filed by '0')  */
static void STR_print_int (char **s, LONG n, char c){
  LONG nn = n;
  char *ss;
  if ( c ){ **s = c; (*s)++; }
  if ( n == 0 ){ **s = '0'; (*s)++; return; }
  if ( n < 0 ){ **s = '-'; (*s)++; n = -n; }
  while ( nn>99999 ){ nn /= 1000000; (*s)+=6; }
  while ( nn>99 ){ nn /= 1000; (*s)+=3; }
  while ( nn>0 ){ nn /= 10; (*s)++; }
  ss = (*s)-1;
  **s = 0;
  while ( n>0 ){ *ss = '0'+(char)(n%10); ss--; n/=10; }
}

/* print a real number to string pointed by *s */
static void STR_print_real (char **s, double n, int len, char c){
  int flag=1, d=0, dd, ll = len;
  LONG nn;

  if ( c ){ **s = c; (*s)++; }
  if ( n<0 ){ **s = '-'; (*s)++; n = -n; }
  if ( n<1 ){ **s = '0'; (*s)++; } 
  else {
    while ( n > (1LL<<60) ){ d++; n /= (1000000000000000000LL);}
    nn = n; STR_print_int (s, nn, 0);
    BLOOP (d, d, 0) FLOOP (dd, 0, 18){ **s = '0'; (*s)++; }
    n -= nn;
  }
  if ( len == 0 ) return;  // no decimal digits
  **s = '.'; (*s)++;
  if ( len<0 ){ len = -len; flag = 0; }
  nn = 1;
  if ( len >= 100 ) nn = len;
  else {
    while ( len >= 6 ){ nn *= 1000000; len -= 6; }
    while ( len >= 3 ){ nn *= 1000; len -= 3; }
    while ( len >= 1 ){ nn *= 10; len --; }
  }
  nn = n*nn;
  STR_print_int_ (s, nn, ll, 0);
  if ( flag ){
    while ( *(*s -1) == '0' ) (*s)--;
    if ( *(*s -1) == '.' ) (*s)--;
  }
  **s = 0;
}

/* fast file routine, print number, c is the char to be printed preceding to the number
   if c==0, nothing will be printed preceding the number
   if len<0 then the #digits following '.' does not change (filed by '0')  */
void FILE2::print_int ( LONG n, char c){
  STR_print_int (&(_buf), n, c);
}
void FILE2::print_real (double n, int len, char c){
  STR_print_real (&(_buf), n, len, c);
}

void FILE2::print_WEIGHT ( WEIGHT w, int len, char c){
#ifdef WEIGHT_DOUBLE
  print_real( w, len, c);
#else
  print_int( w, c);
#endif
}



void FILE2::flush (){
  if ( _buf > _buf_org+FILE2_BUFSIZ/2 ) flush_ ();
}
/* read through the file until newline or EOF */
void FILE2::read_until_newline (){
  int ch;
  if (FILE_err & 3) return;
  while (1){
    ch = getc();
    if ( ch == '\n' ){ FILE_err = 5; return; }
    if ( ch < 0 ){ FILE_err = 6; return; }
  }
}

/* read an edge := a pair of numbers /(and its weight) */
int FILE2::read_pair ( LONG *x, LONG *y, WEIGHT *w, int flag){
  *x = read_int();
  if (FILE_err&4) return (1);
  *y = read_int ();
  if (FILE_err&4) return (1);
  if ( flag & LOAD_ID1 ){ (*x)--; (*y)--; }
  if ( flag & LOAD_EDGEW ) *w = read_double ();
  read_until_newline ();
  if ( (flag & LOAD_TPOSE) || ((flag&LOAD_EDGE) && *x > *y) ) SWAP_<LONG>(x, y);
  return (0);
}
/* read an item and its weight (auto-adjast when */
int FILE2::read_item (FILE2 *wfp, LONG *x, LONG *y, WEIGHT *w, int fc, int flag){
  int f, ff=0;
  *y = read_int ();
  if ( flag & LOAD_ID1 ){ (*y)--; (*x)--; }
  if ( FILE_err&4 ) return (0);
  if ( flag & LOAD_EDGEW ) *w = read_double ();
  else if ( wfp ){
    f = FILE_err; FILE_err = 0;
    *w = wfp->read_WEIGHT ();
    if ( (FILE_err&4) && fc ) *w = wfp->read_double ();
    ff = FILE_err;
    FILE_err = f;
  }
  if ( (flag & LOAD_TPOSE) || ((flag&LOAD_EDGE) && *x > *y) ) SWAP_<LONG>(x, y);
  return (ff);
}

/* count the clms, rows, items, each row size, each column size  */
/* file types can be, array list and element list*/
/* support transpose */
// filecount.hppへ
/*
FILE_COUNT FILE2::count ( 
	int flag, int skip_rows,
	 int int_rows, int skip_clms, int int_clms, 
	 FILE_COUNT_INT row_limit)
	 {

  FILE_COUNT_INT k=0, j, x, y, t=0;

  // flags for using rowt, and clmt, that counts elements in each row/column
  int fr = flag&FILE_COUNT_ROWT, fc = flag&FILE_COUNT_CLMT; 

	// fe,ft: flag for ele mode, and transposition
  int fe = flag&LOAD_ELE, ft = flag&LOAD_TPOSE;  

  FILE_COUNT C = INIT_FILE_COUNT;
  C.flag = flag;

  FLOOP (j, 0, skip_rows) read_until_newline ();

  if ( flag & (FILE_COUNT_NUM+FILE_COUNT_GRAPHNUM) ){

    C.clms = (FILE_COUNT_INT) read_int ();
    C.rows = (flag & FILE_COUNT_NUM)? (FILE_COUNT_INT) read_int(): C.clms;
    C.eles = (FILE_COUNT_INT) read_int ();

    if ( !(flag & (FILE_COUNT_ROWT + FILE_COUNT_CLMT)) ) return (C);
    read_until_newline ();
  }

  do {

    if ( fe ){
      FLOOP (j, 0, skip_clms){ read_double (); if ( FILE_err&3 ) goto ROW_END; }
      x = (FILE_COUNT_INT) read_int (); //printf ("%d\n", FILE_err);
			if ( FILE_err&3 ) goto ROW_END;
      y = (FILE_COUNT_INT) read_int (); if ( FILE_err&4 ) goto ROW_END;
      read_until_newline ();
    } else {
      if ( k==0 ) FLOOP (j, 0, skip_clms){ read_double (); if (FILE_err&3) goto ROW_END; }
      x = t;
      y = (FILE_COUNT_INT)read_int (); if (FILE_err&4 ) goto ROW_END;
      FLOOP (j, 0, int_clms){ read_double (); if (FILE_err&3 ) break; }
      k++;
    }
    if ( ft ) SWAP_<FILE_COUNT_INT>(&x, &y);
    if ( y >= C.clms ){
      C.clms = y+1;
      if ( fc ) reallocx (C.clmt, C.clm_end, C.clms, 0, goto END);
    }
    if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && x >= C.clms ){
      C.clms = x+1;
      if ( fc ) reallocx (C.clmt, C.clm_end, C.clms, 0, goto END);
    }
    if ( x >= C.rows ){
      C.rows = x+1;
      if ( fr ) reallocx (C.rowt, C.row_end, C.rows, 0, goto END);
    }
    if ( (flag&(LOAD_RC_SAME+LOAD_EDGE)) && y >= C.rows ){ // for undirected edge version
      C.rows = y+1;
      if ( fr ) reallocx (C.rowt, C.row_end, C.rows, 0, goto END);
    }
    
    if ( x < C.clm_btm || C.eles == 0 ) C.clm_btm = x;
    if ( y < C.row_btm || C.eles == 0 ) C.row_btm = y;
    if ( fc ) C.clmt[y]++;
    if ( fr ){ C.rowt[x]++; if ( flag&LOAD_EDGE && x != y ) C.rowt[y]++; }
    C.eles++;

    ROW_END:;
    if ( !fe && (FILE_err&1) ){
      t++;
      if ( flag&(LOAD_RC_SAME+LOAD_EDGE) ){
        ENMAX (C.clms, t); ENMAX (C.rows, t);
      } else if ( ft ) C.clms = t; else C.rows = t;

      ENMAX (C.clm_max, k);
      ENMIN (C.clm_min, k);
      FLOOP (j, 0, int_rows) read_until_newline ();
      if ( row_limit>0 && t>=row_limit ) break;
    } else if ( row_limit>0 && C.eles>=row_limit ) break;

  } while ( (FILE_err&2)==0 );
  if ( fc ) reallocx (C.clmt, C.clm_end, C.clms, 0, goto END);
  if ( fr ){
    reallocx (C.rowt, C.row_end, C.rows, 0, goto END);
    ARY_MAX (C.row_max, k, C.rowt, 0, C.rows);
    ARY_MIN (C.row_min, k, C.rowt, 0, C.rows);
  }
  if ( fe && C.clmt ){
    ARY_MAX (C.clm_max, k, C.clmt, 0, C.clms);
    ARY_MIN (C.clm_min, k, C.clmt, 0, C.clms);
  }
  END:;
  if ( ERROR_MES ) mfree (C.rowt, C.clmt);
  return (C);
}*/

void FILE2::closew (){
  flush_last ();
  fclose2 (_fp);
  free2 (_buf_org);
  _buf = _buf_end = 0;
}

/********************  file I/O routines  ********************************/

int FILE_err;  /*  signals  0: for normal termination
                   1: read a number, then encountered a newline,
                   2: read a number, then encountered the end-of-file
                   5: read no number, and encountered a newline
                   6: read no number, and encountered the end-of-file */
/*
void FILE2_flush_ (FILE2 *fp){
#ifndef _FILE2_LOAD_FROM_MEMORY_
  fwrite (fp->buf_org, fp->buf-fp->buf_org, 1, fp->fp);
  *fp->buf_org = *fp->buf;
  fp->buf = fp->buf_org;
#endif
}

void FILE2_copy (char *f1, char *f2){   // copy file f1 to f2, f2 will be cleared or created
  FILE *fp, *fp2;
  char buf[16384];
  int s;

  fopen2 (fp, f1, "r", EXIT);
  fopen2 (fp2, f2, "w", EXIT);
  do {
    s = fread (buf, 1, 16384, fp);
    fwrite (buf, 1, s, fp2);
  } while (s);
  fclose (fp);
  fclose (fp2);
}

void FILE2_puts (FILE2 *fp, char *s){
#ifndef _FILE2_LOAD_FROM_MEMORY_
  while ( *s != 0 ){
    *(fp->buf) = *s;
    s++;
    fp->buf++;
  }
#endif
}
void FILE2_putc (FILE2 *fp, char c){
#ifndef _FILE2_LOAD_FROM_MEMORY_
  *(fp->buf) = c;
  fp->buf++;
#endif
}
int FILE2_getbit (FILE2 *fp){
  int a;
  unsigned char *p;
  if ( fp->buf >= fp->buf_end ){
    if ( (fp->buf_end < fp->buf_org+FILE2_BUFSIZ) && (fp->buf_end>=fp->buf_org) ){ FILE_err=2; return (-1); }
    fp->buf = fp->buf_org;
    fp->buf_end = fp->buf_org + fread (fp->buf, 1, FILE2_BUFSIZ, fp->fp);
  }
  p = (unsigned char *)fp->buf; a = (*p) %2; (*p) /=2;
  if ( (++fp->bit) == 8 ){ fp->bit = 0; fp->buf++; }
  return (a);
}
void FILE2_putbit (FILE2 *fp, int a){   // a is non-zero => 1
  if ( a ){
    a = 1 << fp->bit;
    *fp->buf |= a;
  }
  if ( (++fp->bit) == 8 ){
    fp->bit = 0;
    fp->buf++;
    FILE2_flush (fp);
    *fp->buf = 0;
  }
}
*/
/* fast file routine, print number, c is the char to be printed preceding to the number
   if c==0, nothing will be printed preceding the number
   if len<0 then the #digits following '.' does not change (filed by '0')  */
/*
void FILE2_print_int (FILE2 *fp, LONG n, char c){
#ifdef _FILE2_LOAD_FROM_MEMORY_
  FILE2_WRITE_MEMORY (int, n);
#else
  STR_print_int (&(fp->buf), n, c);
#endif
}
*/
/* print a real number to string pointed by *s,
    (print_reale prints in format "x.xxxxxe+10e"
   [len] is #decimal digits to be printed
   for speeding up, give 10^{len} as [len], when len >1 */
/*
void FILE2_print_reale (FILE2 *fp, double n, int len, char c){
#ifdef _FILE2_LOAD_FROM_MEMORY_
  FILE2_WRITE_MEMORY (double, n);
#else
  STR_print_reale (&(fp->buf), n, len, c);
#endif
}*/

/* print a real number to string pointed by *s */
/*
void FILE2_print_real (FILE2 *fp, double n, int len, char c){
#ifdef _FILE2_LOAD_FROM_MEMORY_
  FILE2_WRITE_MEMORY (double, n);
#else
  STR_print_real (&(fp->buf), n, len, c);
#endif
}
void FILE2_print_real__ (FILE2 *fp, double n, int len, char c){
  int i=0, flag=1, d=0, dd=0;
  char *back;
  LONG nn;

  if ( c ) FILE2_putc (fp, c);
  if ( n<0 ){ FILE2_putc (fp, '-'); n *= -1; }
  if ( n<1 ){ *(fp->buf) = '0'; fp->buf++; } 
  else {
    while ( n > (1LL<<60) ){ d++; n /= (1000000000000000000LL);}
    nn = n; FILE2_print_int (fp, nn, 0);
    BLOOP (d, d, 0) FLOOP (dd, 0, 18) FILE2_putc (fp, '0');
    n -= nn;
  }
  if ( len == 0 ) return;  // no digits smaller than 1
  *(fp->buf) = '.'; back = fp->buf;
  fp->buf++;
  if ( len<0 ){ len = -len; flag = 0; }
  for (d=0 ; len>0 ; len--){
    if ( d==0 ){
      d = 4;
      n *= 10000.0;
      dd = n;
      n -= dd;
    }
    if ( --d > 0 ){
      i = dd/FILE2_POW[d];
      dd -= i*FILE2_POW[d];
    }
    *(fp->buf) = '0'+i;
    fp->buf++;
    if ( i>0 ) back = fp->buf;
  }
  if ( flag ) fp->buf = back;
}
void FILE2_print_real_ (FILE2 *fp, double n, int len, char c){
  int i=0, flag=1;
  double j=1;
  char *back;

  if ( c ) FILE2_putc (fp, c);
  if ( n<0 ){ FILE2_putc (fp, '-'); n *= -1; }
  while ( n >= j ) j*=10;
  if ( j==1 ){ *(fp->buf) = '0'; fp->buf++; }
  else while ( j>1 ){
    j /= 10;
    i = (int)(n/j);
    *(fp->buf) = '0'+i;
    n -= j*i;
    fp->buf++;
  }
  *(fp->buf) = '.'; back = fp->buf;
  fp->buf++;
  if ( len<0 ){ len = -len; flag = 0; }
  for ( ; len>0 ; len--){
    n *= 10.0;
    i = (int)n;
    *(fp->buf) = '0'+i;
    n -= i;
    fp->buf++;
    if ( i>0 ) back = fp->buf;
  }
  if ( flag ) fp->buf = back;
}
*/
/******/
/*
void FILE2_print_WEIGHT (FILE2 *fp, WEIGHT w, int len, char c){
#ifdef WEIGHT_DOUBLE
  FILE2_print_real(fp, w, len, c);
#else
  FILE2_print_int(fp, w, c);
#endif
}*/

// print integer to file in a bit string manner
// first bit 0 -> 0to127
// 10 -> 128 to 16384+127
// 110 -> 16384+128 to 32*65536+16384+127
// 1110 -> ... to 16*
// max: limit of the bit length 
/*
void FILE2_print_mpint (FILE2 *fp, LONG a, int max, int skip){
  int i=0, bb;
  LONG b, x=0;
  bb = 1<<skip; b = bb;
  while ( a-x >= b ){ x += b; b *= bb; i++; if ( (i+1)*skip >= max ) break; }
  a -= x;
  FLOOP (x, 0, i) FILE2_putbit (fp, 1); // length of the integer
  if ( (i+1)*skip < max ) FILE2_putbit (fp, 0);  // end mark
  else for ( ; max < (i+1)*skip ; max++, b/=2);
  for (b/=2 ; b>0 ; b/=2) FILE2_putbit (fp, (a&b)>0);
}

LONG FILE2_read_mpint (FILE2 *fp, int max, int skip){
  LONG a, b, x=0;
  int i=0, j, bb;
  for (b=bb=1<<skip; (j=FILE2_getbit (fp)) ; i++){
    if ( j<0 ){ FILE_err = 2; return (0); }
    x += b; b *= bb;
    if ( (i+2)*skip >= max ){ 
      for ( ; max < (i+2)*skip ; max++, b/=2);
      break;
    }
  }
  for (a=0 ; b>1 ; b/=2) a = a*2 + FILE2_getbit(fp);
  return (a+x);
}
*/
/* print lowest k bits to file */
/*
void FILE2_print_intbit (FILE2 *fp, LONG a, int k){
  LONG b= 1LL << (k-1);
  while (b>0){ FILE2_putbit (fp, (a&b)>0); b /= 2; }
}*/
/* read k bits from file as integer */
/*
LONG FILE2_read_intbit (FILE2 *fp, int k){
  LONG a=0;
  while (k-->0){ a = a*2 + FILE2_getbit (fp); }
  return (a);
}
*/
//#define FILE2_READ_CH(v) \

//#define FILE2_READ_(v)
/*
double FILE2_read_double_ (FILE2 *fp){
  double item, geta=1;
  int sign=1, ch;
  FILE_err = 0;
  while (1){
    ch = FILE2_getc(fp);
    if ( ch < 0 ){ FILE_err = 6; return (-DOUBLEHUGE); }
    if ( ch == '\n' ){ FILE_err = 5; return (-DOUBLEHUGE); }
    if ( ch=='-' ) sign *= -1;
    else if ( ch=='.' ) geta = 0.1;
    else if ( ch>='0' && ch<='9' ) break;
    else { sign = 1; geta = 1; }
  }

  item = geta * (ch-'0');
  if ( geta < 1.0 ) geta *= .1;
  while (1){
    ch = FILE2_getc(fp);
    if ( ch == '\n' ){ FILE_err = 1; return (item*sign); }
    if ( ch<0 ){ FILE_err = 2; return (item*sign); }
    if ( ch == '.' ) geta = .1;
    else if ( (ch < '0') || (ch > '9')) return (item*sign);
    else if ( geta < 1.0 ){
      item += geta*(ch-'0');
      geta *= 0.1;
    } else item = item*10 + (ch-'0');
  }
}

*/
/*


void FILE2_printf (FILE2 *fp, char *mes, ...){
  va_list argp;
  va_start (argp, mes);
  sprintf (fp->buf, mes, argp);
  va_end (argp);
}
*/



/* load edge list type file */
// S:*struct, fp,wfp:*FILE2, input-file and weight-file, ary:*
/*
#define FILE2_READ_FILE_EDGE(S, fp, wfp, ary, wei) {\
 size_t i, x, y; \
 WEIGHT w; \
 FILE2_reset (fp);\
 if ( (S)->flag&(LOAD_NUM+LOAD_GRAPHNUM) ) FILE2_read_until_newline (fp); \
 if ( (S)->flag & (FSTAR_DEG_CHK+FSTAR_IN_CHK+FSTAR_OUT_CHK) ){
    FLOOP (i, 0, F->edge_num_org){
      while ( FSTAR_read_pair (F, fp, &x, &y, &w) );
      if ( FSTAR_eval_edge (F, x, y, w) == 0 ){
        F->fstar[x]--;
        if ( F->flag & LOAD_EDGE ) F->fstar[y]--;
      }
    }
  }

  FSTAR_calc_fstar (F);
  FILE2_reset (fp);
  if ( F->flag&(LOAD_NUM+LOAD_GRAPHNUM) ) FILE2_read_until_newline (fp);

  FLOOP (i, 0, F->edge_num_org){
    while ( FSTAR_read_pair (F, fp, &x, &y, &w) );
    if ( F->fstar[x+1] > F->fstar[x] && ( !(F->flag & LOAD_EDGE) || F->fstar[y+1]>F->fstar[y]) ){
      if ( FSTAR_eval_edge (F, x, y, w) == 0 ||
           !(F->flag & (FSTAR_DEG_CHK+FSTAR_IN_CHK+FSTAR_OUT_CHK)) ){  // 14/02 added, not confirmed!!!!
        if ( F->flag & LOAD_EDGEW ){ F->edge_w[F->fstar[x]] = w; }
        F->edge[F->fstar[x]++] = y;
        if ( F->flag & LOAD_EDGE ){
          if ( F->flag & LOAD_EDGEW ){ F->edge_w[F->fstar[y]] = w; }
          F->edge[F->fstar[y]++] = x;
        }
      }
    }
  }
  BLOOP (i, F->out_node_num, 0) F->fstar[i+1] = F->fstar[i];
  F->fstar[0] = 0;
}
*/


/*
size_t common_size_t;
INT common_INT, common_INT2;
char *common_charp, *common_pnt;
FILE *common_FILE;
FILE2 common_FILE2;
PERM common_PERM;
char common_comm[1024], common_comm2[1024], *common_argv[100];  // max. command length = 2048, max. #params = 100

char *ERROR_MES = NULL;
int print_time_flag=0;
PARAMS internal_params;
#ifdef MULTI_CORE
int SPIN_LOCK_dummy;
#endif
FILE2 INIT_FILE2 = {TYPE_FILE2,NULL,NULL,NULL,NULL,0};
VEC INIT_VEC = {TYPE_VEC,NULL,0,0};
FILE_COUNT INIT_FILE_COUNT = {0,0,0,0,0,0,0,0,0,0,0,0,NULL,NULL,0,0,NULL,NULL,0,0,NULL,NULL};
short FILE2_POW[5] = {1, 10, 100, 1000, 10000};
#ifdef _FILE2_LOAD_FROM_MEMORY_
void *__load_from_memory__=NULL, *__load_from_memory_org__=NULL, *__load_from_memory_org2__=NULL;
char *__write_to_memory__=NULL, *__write_to_memory_org__=NULL, *__write_to_memory_next__=NULL;
#endif

FILE_COUNT_INT common_FILE_COUNT_INT;
LONG common_LONG;
int common_int;
*/
  /* bitmasks, used for bit operations */
/*
int BITMASK_UPPER1[32] = { 0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
                           0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
                           0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
                           0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
                           0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
                           0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
                           0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
                           0xf0000000, 0xe0000000, 0xc0000000, 0x80000000 };
int BITMASK_UPPER1_[32] = { 0xfffffffe, 0xfffffffc, 0xfffffff8, 0xfffffff0,
                            0xffffffe0, 0xffffffc0, 0xffffff80, 0xffffff00,
                            0xfffffe00, 0xfffffc00, 0xfffff800, 0xfffff000,
                            0xffffe000, 0xffffc000, 0xffff8000, 0xffff0000,
                            0xfffe0000, 0xfffc0000, 0xfff80000, 0xfff00000,
                            0xffe00000, 0xffc00000, 0xff800000, 0xff000000,
                            0xfe000000, 0xfc000000, 0xf8000000, 0xf0000000,
                            0xe0000000, 0xc0000000, 0x80000000, 0x00000000 };

int BITMASK_LOWER1[32] = { 0x00000000, 0x00000001, 0x00000003, 0x00000007,
                           0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
                           0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
                           0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
                           0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
                           0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
                           0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
                           0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff };
int BITMASK_LOWER1_[32] = { 0x00000001, 0x00000003, 0x00000007, 0x0000000f,
                            0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
                            0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
                            0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
                            0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
                            0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
                            0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
                            0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff };

int BITMASK_1[32] = { 0x00000001, 0x00000002, 0x00000004, 0x00000008,
                      0x00000010, 0x00000020, 0x00000040, 0x00000080,
                      0x00000100, 0x00000200, 0x00000400, 0x00000800,
                      0x00001000, 0x00002000, 0x00004000, 0x00008000,
                      0x00010000, 0x00020000, 0x00040000, 0x00080000,
                      0x00100000, 0x00200000, 0x00400000, 0x00800000,
                      0x01000000, 0x02000000, 0x04000000, 0x08000000,
                      0x10000000, 0x20000000, 0x40000000, 0x80000000 };
int BITMASK_31[32] = { 0xfffffffe, 0xfffffffd, 0xfffffffb, 0xfffffff7,
                       0xffffffef, 0xffffffdf, 0xffffffbf, 0xffffff7f,
                       0xfffffeff, 0xfffffdff, 0xfffffbff, 0xfffff7ff,
                       0xffffefff, 0xffffdfff, 0xffffbfff, 0xffff7fff,
                       0xfffeffff, 0xfffdffff, 0xfffbffff, 0xfff7ffff,
                       0xffefffff, 0xffdfffff, 0xffbfffff, 0xff7fffff,
                       0xfeffffff, 0xfdffffff, 0xfbffffff, 0xf7ffffff,
                       0xefffffff, 0xdfffffff, 0xbfffffff, 0x7fffffff };

int BITMASK_16[8] = { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000,
                      0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000 };
int BITMASK_UPPER16[8] = { 0xffffffff, 0xfffffff0, 0xffffff00, 0xfffff000,
                           0xffff0000, 0xfff00000, 0xff000000, 0xf0000000 };
int BITMASK_LOWER16[8] = { 0x0000000f, 0x000000ff, 0x00000fff, 0x0000ffff,
                           0x000fffff, 0x00ffffff, 0x0fffffff, 0xffffffff };
int BITMASK_FACT16[8] = { 0x1, 0x10, 0x100, 0x1000,
                          0x10000, 0x100000, 0x1000000,0x10000000 };

*/


/******************************************/
/*  ==== terminology for comments  ====
  range check: to check the input parameter is valid, or in the valid range.
  If a function does not have this, its comment has "no range check"
  */
   
/*  ====  rules for the name of functions and routines  ====
  init: initialization for an object, structure, etc. memory is allocated 
   if needed.
  end: compared to init, termination of structures, etc.
   free allocated memory if it exists, but not free itself.
   different from ordinary new, create, del, free.

  cpy: copy an object without allocating memory
  dup: make a duplication of an object with allocating new memory

  new: new. allocate memory for new object. also used for re-allocation from 
   the list of deleted objects
  del: delete. free memory, or insert it to the list of deleted objects

  ins : insert. insert an element (active, not deleted) to an object, possible
     at the given position.
  out : extract. extract an element, possibly specified, from an object.
        it will be not deleted.
  rm  : extract, and delete
  rmall: delete all (specified) elements of an object
  mk  : make. new+insÅB
  mv  : move. move the elements from an object to another,
         or change the position.

  update : update an object, possibly of specified position, to the exact,
           or to the current one.
  chg    : change the status of an object to the specified one.
  
  prv: point the previous element
  nxt: point the next element
  end: the maximum size (allocated size) of an array, etc.
  num: the (current) number of elements in an array, etc.
  kth: for the use of "first k object"
  tkth: for the use of "first k object from the end". to kth.
  rnd: random
  print: print structure and etc.
  find: search or find an specified element from the set of structures, etc.
*/

/*  ==== rules for the name of variables ====
  - use i or j for the counter in loops
  - use e for elements
  - use k for the specified rank
  - use f or flag for flagment
  - use m for maximum value or minimum value
  - use c for counters
*/


