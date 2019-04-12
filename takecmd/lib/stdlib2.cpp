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
#ifdef MTWISTER
#include"dSFMT.c"
#endif

char *common_pnt;
INT common_INT;
char common_comm[1024], common_comm2[1024], *common_argv[100];  // max. command length = 2048, max. #params = 100
char *ERROR_MES = NULL;

#ifdef MULTI_CORE
int SPIN_LOCK_dummy;
#endif


FILE_COUNT_INT common_FILE_COUNT_INT;
LONG common_LONG;
int common_int;


/* free many pointers */
void mfree_(void *x, ...){
  va_list argp;
  void *a;
  va_start (argp, x);
  while((a = va_arg(argp, void *)) != (void*)1){ free2 (a); }
  va_end (argp);
}
/* free many pointers */
void mfree2_(void *x, ...){
  va_list argp;
  void *a;
  va_start (argp, x);
  while((a = va_arg(argp, void *)) != (void*)1){ free2 (*((char **)a)); }
  va_end (argp);
}
/* remove many files */
void mremove_ (char *x, ...){
  va_list argp;
  char *a;
  va_start (argp, x);
  while((a = va_arg(argp, char *))){
    sprintf (common_comm, "%s%s", x, a);
    remove (common_comm);
  }
  va_end (argp);
}

unsigned long xor128(){ 
    static unsigned long x=123456789,y=362436069,z=521288629,w=88675123; 
    unsigned long t; 
    t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) ); 
}

/* print a real number in a good style */
void fprint_real (FILE *fp, double f){
  char s[200];
  size_t i;
  i = sprintf (s, "%f", f);
  while ( s[i-1] == '0' ) i--;
  if ( s[i-1] == '.' ) i--;
  s[i] = 0;
  fprintf (fp, s);
}
void print_real (double f){
  fprint_real (stdout, f);
}

void fprint_WEIGHT (FILE *fp, WEIGHT f){
#ifdef WEIGHT_DOUBLE
  fprint_real (fp, f);
#else
  fprintf (fp, "%d", f);
#endif
}
void print_WEIGHT (WEIGHT f){
  fprint_WEIGHT (stdout, f);
}

