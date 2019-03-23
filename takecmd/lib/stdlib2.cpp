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

size_t common_size_t;
INT common_INT, common_INT2;
char *common_charp, *common_pnt;
FILE *common_FILE;

char common_comm[1024], common_comm2[1024], *common_argv[100];  // max. command length = 2048, max. #params = 100

char *ERROR_MES = NULL;
int print_time_flag=0;
PARAMS internal_params;

#ifdef MULTI_CORE
int SPIN_LOCK_dummy;
#endif


FILE_COUNT_INT common_FILE_COUNT_INT;
LONG common_LONG;
int common_int;

  /* bitmasks, used for bit operations */
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

/* union finding algorithm, initially, any i has to have ID of i */
void UNIONFIND_init (UNIONFIND_ID **ID, UNIONFIND_ID **set, UNIONFIND_ID end){
  UNIONFIND_ID i;
  if ( ID ){ malloc2 (*ID, end, EXIT); FLOOP (i, 0, end) (*ID)[i] = i; }
  if ( set ){ malloc2 (*set, end, EXIT); FLOOP (i, 0, end) (*set)[i] = i; }
}

/* get the ID of belonging group, and flatten the ID tree */
UNIONFIND_ID UNIONFIND_getID (UNIONFIND_ID v, UNIONFIND_ID *ID){
  UNIONFIND_ID vv = v, vvv;
  while (v != ID[v]) v = ID[v];  // trace parents until the root (ID[v] = v, if v is a root)
  while (vv != ID[vv]){ vvv = vv; vv = ID[vv]; ID[vvv] = v; }
  return (v);
}
   // unify to sets to which u and v belong
void UNIONFIND_unify (UNIONFIND_ID u, UNIONFIND_ID v, UNIONFIND_ID *ID){
  v = UNIONFIND_getID (v, ID); // compute ID of v 
  u = UNIONFIND_getID (u, ID); // compute ID of u 
  ID[v] = u;  // set ID of (ID of v) to (ID of u)
}
   // convert ID array to list so that each list includes a group and its head is representative
void UNIONFIND_convert_to_set (UNIONFIND_ID *ID, UNIONFIND_ID end){
  UNIONFIND_ID i, r;
  FLOOP (i, 0, end) UNIONFIND_getID (i, ID);  // make all items linking to the root
  FLOOP (i, 0, end){
    r = ID[i]; if ( r == i ) continue;   // head of the list
    if ( ID[r] == r ){ ID[r] = i; ID[i] = i; }  // initialize the list
    else { ID[i] = ID[r]; ID[r] = i; }  // insert to the next ot the head of the list
  }
}
    // maintain ID and list to representing the sets simultaneously
void UNIONFIND_unify_set (UNIONFIND_ID u, UNIONFIND_ID v, UNIONFIND_ID *ID, UNIONFIND_ID *set){
  UNIONFIND_ID z;
  v = UNIONFIND_getID (v, ID); // compute ID of v 
  u = UNIONFIND_getID (u, ID); // compute ID of u 
  if ( u != v ){
    if ( set[u] == u ){ set[u] = v; ID[v] = u; } // attach u as the head of the list of v
    else if ( set[v] == v ){ set[v] = u; ID[u] = v; }  // attach v as the head of the list of u
    else {
      for (z=v; set[z]!=z ; z=set[z]);  // find the last in the list of v
      set[z] = set[u]; set[u] = v; // insert the list of v to list of u
      ID[v] = u; // set ID of (ID of v) to (ID of u)
    }
  }
}

/* allocate memory according to rows and rowt */

void VEC::alloc ( VEC_ID clms){

	_type = TYPE_VEC;  // mark to identify type of the structure
	_v = NULL;
	_end=0;
	_t=0;
  _end = clms;

  calloc2 (_v, clms+1, EXIT);
}
/* terminate routine for VEC */
void VEC::end (){
  free2 (_v);
	_type = TYPE_VEC;  // mark to identify type of the structure
	_v = NULL;
	_end=0;
	_t=0;
}


