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
    if ( (_buf_end < _buf_org+FILE2_BUFSIZ) && (_buf_end>=_buf_org) ){ _FILE_err=2; return (-1); }
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
  _FILE_err = 0;
  while (1){
    ch = getc();
    if ( ch>='0' && ch<='9' ) break;
    if ( ch == '\n' ){ _FILE_err = 5; return (-INTHUGE); }
    if ( ch < 0 ){ _FILE_err = 6; return (-INTHUGE); }
    if ( ch=='-' ) sign = -1;
  }
  for (item=ch-'0' ; 1 ; item=item*10 +ch-'0'){
    ch = getc();
    if ( ch<'0' || ch>'9' ){
      if ( ch == '\n' ) _FILE_err = 1;
      if ( ch < 0 ) _FILE_err = 2;
      return (item*sign);
    }
  }
}


double FILE2::read_double (){
  double item, geta=1;
  int sign=1, ch, n=0, d, flag=0;
  _FILE_err = 0;
  while (1){
    ch = getc();
    if ( ch<'0' || ch>'9' ){
      if ( ch == '\n' ){ _FILE_err = 5; return (-DOUBLEHUGE); }
      if ( ch < 0 ){ _FILE_err = 6; return (-DOUBLEHUGE); }
      if ( ch=='-' ) sign = -1;
      else if ( ch=='.' ){ flag = 1; geta = 0.1; }
      else { sign = 1; geta = 1; flag = 0; }
    } else break;
  }
  item = 0; d = 0; item = ch-'0'; if ( flag ) item /= 10;

  while (1){
    ch = getc();
    if ( ch < '0' || ch > '9' ){
      if ( ch == '\n' ){ _FILE_err = 1; break; }
      else if ( ch < 0 ){ _FILE_err = 2; break; }
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
            if ( ch == '\n' ) _FILE_err = 1;
            if ( ch < 0 ) _FILE_err = 2;
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
    //BLOOP (d, d, 0){
    	//FLOOP (dd, 0, 18){ 
		for(d=d;(d--)>0;){
			for(dd=0 ;dd < 18 ;dd++){
    		**s = '0'; (*s)++; 
    	}
    }
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
  if (_FILE_err & 3) return;
  while (1){
    ch = getc();
    if ( ch == '\n' ){ _FILE_err = 5; return; }
    if ( ch < 0 ){ _FILE_err = 6; return; }
  }
}

/* read an edge := a pair of numbers /(and its weight) */
int FILE2::read_pair ( LONG *x, LONG *y, WEIGHT *w, int flag){
  *x = read_int();
  if (_FILE_err&4) return (1);
  *y = read_int ();
  if (_FILE_err&4) return (1);
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
  if ( _FILE_err&4 ) return (0);
  if ( flag & LOAD_EDGEW ) *w = read_double ();
  else if ( wfp ){
    f = _FILE_err; _FILE_err = 0;
    *w = wfp->read_WEIGHT ();
    if ( (_FILE_err&4) && fc ) *w = wfp->read_double ();
    ff = _FILE_err;
    _FILE_err = f;
  }
  if ( (flag & LOAD_TPOSE) || ((flag&LOAD_EDGE) && *x > *y) ) SWAP_<LONG>(x, y);
  return (ff);
}


void FILE2::closew (){
  flush_last ();
  _fclose2();
  free2 (_buf_org);
  _buf = _buf_end = 0;
}

template <>
void FILE2::ARY_Read<double>(double *f,size_t num) {

	for (size_t i=0 ; i < num  ; i++){
		do{
			f[i]=read_double();
		}while((_FILE_err&6)==4);

	 	if(_FILE_err&2)break;
 	}
}


int FILE2::ARY_Load(double *f,char* fname,int d){

		FILE2 cmn;
		int num;

		cmn.open(fname,"r");
		num = cmn.ARY_Scan_DBL(d);
		f = malloc2(f,(num)+1);
		cmn.reset();
		cmn.ARY_Read(f,num);
		cmn.close();
		return num;
}



/********************  file I/O routines  ********************************/

//int FILE_err;  
/*  signals  0: for normal termination
                   1: read a number, then encountered a newline,
                   2: read a number, then encountered the end-of-file
                   5: read no number, and encountered a newline
                   6: read no number, and encountered the end-of-file */


