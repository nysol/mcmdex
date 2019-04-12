/* library for standard macros and functions 
 by Takeaki Uno 2/22/2002   e-mail: uno@nii.jp
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please 
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */
#pragma once

#define WEIGHT_DOUBLE

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include<stdarg.h>

#ifdef USE_SIMD
 #include<emmintrin.h>   // use MMX-SSE2;
#endif


#if defined(__cplusplus) && defined(__GNUC__)
 #define _cplusplus_
#endif

// for visual C++  (use typeid insead of typeof)
#ifdef _MSC_VER
 #define typeof(a)  typeid(a).name()
#endif

#ifdef MULTI_CORE
#include <sys/types.h>
#include <pthread.h>
#endif

#define CORE_MAX 16


/* comment out the following line if no error check is needed */
//#define ERROR_CHECK
/* comment out the following if exit is not needed after each error routine */
//#define ERROR_RET

#ifdef ERROR_RET   // definition of the process for errors
  #define EXIT return
  #define EXIT0 return(0)
#else 
  #define EXIT exit(1)
  #define EXIT0 exit(1)
#endif

// for dealing with files more than 2GB
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#ifndef NULL
  #define NULL 0
#endif

#ifdef MTWISTER
 #define RANDOM ((long)(dsfmt_gv_genrand_close_open()*2147483648LL))
 #define RAND1 dsfmt_gv_genrand_close_open()
 #define RAND_INIT dsfmt_gv_init_gen_rand(514346237)
#elif defined(__GNUC__)
 #define RANDOM xor128()
 #define RAND1 ((double)xor128())/4294967296.0
 #define RAND_INIT xor128()
#else
 #define RANDOM rand()
 #define RAND1 ((double)rand())/2147483648.0
 #define RAND_INIT srand(0)
#endif


// 64bit integer
#ifdef LONG_32
 #define LONG int
 #define LONGHUGE INTHUGE
 #define LONGF "%d"
#elif !defined(LONG)
 #define LONG long long
 #define LONGHUGE 9000000000000000000LL
 #define LONGF "%lld"
#endif

// actual int (most proper sized integer, for the processor)
#ifdef INT_64
 #define INT LONG
 #define INTF LONGF
#else
 #define INT int
 #define INTF "%d"
#endif

#ifndef FILE_LONG 
 #define FILE_LONG LONG
 #define FILE_LONGHUGE LONGHUGE
 #define FILE_LONGF LONGF
#endif

#define UINTHUGE 4000000000U
#define INTHUGE 2000000000
#define USHORTHUGE 32767
#define SHORTHUGE 65535
#define DOUBLEHUGE 999999999999999999999999999999.9
#define ISEQUAL_VALUE   0.0000001
#define ISEQUAL_VALUE2  0.00001
#define PI 3.1415926535897932384647950288
#define PI_INT 31416
#define NPE 2.718281828459045235360287471352
#define NPE_INT 27183

#ifndef WEIGHT
 #ifdef WEIGHT_DOUBLE
  #define WEIGHT double
  #define WEIGHTHUGE DOUBLEHUGE
  #define WEIGHTF "%f"
 #else   //  define WEIGHT by int if it's undefined
  #define WEIGHT int
  #define WEIGHTHUGE INTHUGE
  #define WEIGHTF "%d"
 #endif
#endif

#ifndef PERM
 #ifdef PERM_LONG
  #define PERM LONG
  #define PERMHUGE LONGHUGE
  #define PERMF LONGF
 #else
  #define PERM int
  #define PERMHUGE INTHUGE
  #define PERMF "%d"
 #endif
#endif



extern INT common_INT;
extern char *common_pnt;
extern char *ERROR_MES;
extern int FILE_err;

extern char common_comm[], common_comm2[], *common_argv[];


/* lock&unlock for multi-core mode */
#ifdef MULTI_CORE
  extern int SPIN_LOCK_dummy;
  #define SPIN_LOCK(b,a)    (SPIN_LOCK_dummy=(((b)>1)&&pthread_spin_lock(&(a))))
  #define SPIN_UNLOCK(b,a)    (SPIN_LOCK_dummy=(((b)>1)&&pthread_spin_unlock(&(a))))
#else
  #define SPIN_LOCK(b,a)
  #define SPIN_UNLOCK(b,a)
#endif

/*  equal/inequal with allowing numerical error for double  */
#define ISEQUAL(a,b)	((a)-(b)<ISEQUAL_VALUE&&(b)-(a)<ISEQUAL_VALUE)
#define RANGE(a,b,c)  (((a)<=(b))&&((b)<=(c)))
#define BITRM(a,b)    ((a)-=((a)&(b)));

/*  macro for getting maximum/minimum of two values  */
#define MAX(a,b)      (((a)>(b))?(a):(b))
#define ENMAX(a,b)      ((a)=(((a)>(b))?(a):(b)))
#define MIN(a,b)      (((a)<(b))?(a):(b))
#define ENMIN(a,b)      ((a)=(((a)<(b))?(a):(b)))

/*  error routine  */
#define print_err(...)      fprintf(stderr,__VA_ARGS__)
#define print_mesf(S,...)  do{if(((S)->get_flag())&1)fprintf(stderr,__VA_ARGS__);}while(0)
#define error(mes,x)        do{ERROR_MES=mes;fprintf(stderr,"%s\n",mes);x;}while(0)
#define error_num(mes,n,x)  do{ERROR_MES=mes;fprintf(stderr,"%s: %g\n",mes,(double)(n));x;}while(0)
#define print_mes(S,...)  do{if(((S)->_flag)&1)fprintf(stderr,__VA_ARGS__);}while(0)

#define print_fname(s,fname,...)  do{if(fname)fprintf(stderr,s,fname);}while(0)
#define mfree(...)          mfree_(NULL, __VA_ARGS__, (void *)1)
#define mfree2(...)          mfree2_(NULL, __VA_ARGS__, (void *)1)

  
/* basic array operations and loops */
// #define   ARY_FILL(f,start,end,c) do{for(common_size_t=(size_t)(start);common_size_t<(size_t)(end);common_size_t++)(f)[common_size_t]=(c);}while(0)
#define   FLOOP(i,x,y)  for ((i)=(x) ; (i)<(y) ; (i)++)
#define   BLOOP(i,x,y)  for ((i)=(x) ; ((i)--)>(y) ; )
#define   MLOOP(z,x,M)  for ((z)=(x) ; *(z)<(M) ; (z)++)


/* allocate memory, and exit with error message if fault */
#ifdef _cplusplus_
#define   malloc2(f,b,x)     do{if(!((f)=(typeof(f))malloc(((size_t)sizeof((f)[0]))*(b)))){fprintf(stderr,"memory allocation error: line %d" #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof((f)[0])*(b)));ERROR_MES="out of memory";x;}}while(0)
#define   calloc2(f,b,x)     do{if(!((f)=(typeof(f))calloc(sizeof((f)[0]),b))){fprintf(stderr,"memory allocation error: line %d" #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof((f)[0])*(b)));ERROR_MES="out of memory";x;}}while(0)
#define   realloc2(f,b,x)    do{if(!(f=(typeof(f))realloc(f,((size_t)sizeof((f)[0]))*(b)))){fprintf(stderr,"memory allocation error: line %d" #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof((f)[0])*(b)));ERROR_MES="out of memory";x;}}while(0)
#else
#define   malloc2(f,b,x)     do{if(!((f)=malloc(((size_t)sizeof((f)[0]))*(b)))){fprintf(stderr,"memory allocation error: line %d" #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof((f)[0])*(b)));ERROR_MES="out of memory";x;}}while(0)
#define   calloc2(f,b,x)     do{if(!((f)=calloc(sizeof((f)[0]),b))){fprintf(stderr,"memory allocation error: line %d: " #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof((f)[0])*(b)));ERROR_MES="out of memory";x;}}while(0)
#define   realloc2(f,b,x)    do{if(!(f=realloc(f,((size_t)sizeof((f)[0]))*(b)))){fprintf(stderr,"memory allocation error: line %d: " #f " (" LONGF " byte)\n",__LINE__, (LONG)((LONG)sizeof((f)[0])*(b)));ERROR_MES="out of memory";x;}}while(0)
#endif

/* reallocate memory and expand the memory size */
#define   realloci(f,i,x)  do{if(!((i)&((i)-1)))realloc2(f,(i)*2+1,x);}while(0)

//#define   reallocx_(f,end,end2,e,x)  do{realloc2(f,end2,x);FLOOP(common_size_t,(size_t)end,(size_t)end2)(f)[common_size_t]=(e);}while(0)
//#define   reallocx(f,end,i,e,x)  do{if((size_t)(i)>=(size_t)(end)){reallocx_(f,end,MAX((end)*2+16,(i)+1),e,x);end=MAX((end)*2,(i)+1);}}while(0)


/* basic array operations */
#define   ARY_MAX(m,i,f,x,y)   do{(m)=(f)[x];(i)=(x);FLOOP(common_INT,(x)+1,(y))if((m)<(f)[common_INT]){(i)=common_INT;(m)=(f)[i];}}while(0)
#define   ARY_MIN(m,i,f,x,y)   do{(m)=(f)[x];(i)=(x);FLOOP(common_INT,(x)+1,y)if((m)>(f)[common_INT]){(i)=common_INT;(m)=(f)[i];}}while(0)
#define   ARY_SUM(f,v,x,y)       do{(f)=0;FLOOP(common_INT,x,y)(f)+=(v)[common_INT];}while(0)

/* macros for allocating memory with exiting if an error occurs */
#define free2(a)   do{if(a){free(a);(a)=NULL;}}while(0)
#define free2d(a)  do{if(a){free2((a)[0]);free(a);(a)=NULL;}}while(0)

/* a macro for open files with exiting if an error occurs */
#ifdef _MSC_
 #define   fopen2(f,a,b,x)     do{fopen_s(&f,a,b);if(!f){ERROR_MES="file open error";fprintf(stderr,"file open error: file name %s, open mode %s\n",a,b);x;}}while(0)
#else
 #define   fopen2(f,a,b,x)     do{if(!((f)=fopen(a,b))){ERROR_MES="file open error";fprintf(stderr,"file open error: file name %s, open mode %s\n",a,b);x;}}while(0)
#endif

#define fclose2(a) do{if(a){fclose(a);(a)=NULL;}}while(0)




#ifndef VEC_ID
 #ifdef VEC_ID_LONG
  #define VEC_ID LONG
  #define VEC_ID_END LONGHUGE
  #define VEC_IDF LONGF
 #else
  #define VEC_ID int
  #define VEC_ID_END INTHUGE
  #define VEC_IDF "%d"
 #endif
#endif




/*********************************************************/


#define SHOW_MESSAGE 1   // not print messages
#define SHOW_PROGRESS 2  // show progress of the computation
//#define APPEND_OUTPUT 4  // append the output to existing file
//#define NO_REMOVE 8      // do not remove the temporal files
#define LOAD_PERM 64      // permute the nodes/items by something
#define LOAD_RM_DUP 128   // duplicate items in each row, for loading data
#define LOAD_INCSORT 256  // sort rows in increasing order, for loading data
#define LOAD_DECSORT 512  // sort rows in decreasing order, for loading data
#define LOAD_ELE 1024     // load tuple-list file 
#define LOAD_TPOSE 2048     // transpose the file when load 
#define LOAD_DBLBUF 4096     // allocate double-size memory for element buffer
#define LOAD_WSORT 8192     // sort rows by their weights 
#define LOAD_SIZSORT 16384     // sort rows by their sizes 
#define LOAD_DECROWSORT 32768   // sort rows in decreasing order
#define LOAD_NUM 65536   // read #columns, #rows and #elements from the 1st line of the file

#define LOAD_EDGEW 131072     // read edge weight
//#define LOAD_ARCW 262144     // read arc weight
//#define LOAD_NODEW 524288     // read node weight
#define LOAD_BIPARTITE 1048576     // read bipartite graph
#define LOAD_EDGE 2097152     // read edge 
#define LOAD_ARC 4194304     // read arc 
#define LOAD_GRAPHNUM 8388608   // read #vertices and #edges from the 1st line of the file
#define LOAD_ID1 16777216       // node ID begins from 1
#define LOAD_COMP 33554432   // read as a complement  
#define LOAD_RC_SAME 67108864     // set #rows and #columns to the maximum of them


/* free many pointers*/
void mfree_(void *x, ...);
void mfree2_(void *x, ...);

/* remove many files */
void mremove_ (char *x, ...);

/* print a real number in a good style */
void fprint_real (FILE *fp, double f);
void print_real (double f);
void fprint_WEIGHT (FILE *fp, WEIGHT f);
void print_WEIGHT (WEIGHT f);


#define FILE_COUNT_ROWT 32   // count size of each row
#define FILE_COUNT_CLMT 64   // count size of each column
#define FILE_COUNT_NUM LOAD_NUM   // read #columns, #rows and #elements
#define FILE_COUNT_GRAPHNUM LOAD_GRAPHNUM   // read #vertices and #edges
#define FILE2_BUFSIZ 16384

#define FILE_COUNT_INT VEC_ID
#define FILE_COUNT_INTF VEC_IDF


/* quick sort macros // templateにする?*/ //common_INT common_pntどうにかする
#define QQSORT_ELE(a,x)  ((a *)(&(common_pnt[(*((PERM *)(x)))*common_INT])))


template<typename T>
int qsort_cmp_(const void *x,const void *y){
 if ( *(T *)x < *(T *)y ) return (-1); else return ( *(T *)x > *(T *)y );
}

template<typename T>
int qsort_cmp__(const void *x,const void *y){
 if ( *(T*)x > *(T*)y ) return (-1); else return ( *(T*)x < *(T*)y );
}

template<typename T>
int qqsort_cmp_(const void *x,const void *y){

 T *xx=QQSORT_ELE(T,x), *yy=QQSORT_ELE(T,y);
 if ( *xx < *yy ) return (-1); 
  else return ( *xx > *yy ); 
}

template<typename T>
int qqsort_cmp__(const void *x, const void *y){ 

 T *xx=QQSORT_ELE(T,x), *yy=QQSORT_ELE(T,y);
 if ( *xx > *yy ) return (-1); 
 else return ( *xx < *yy ); 
}


template<typename T>
void qsort_ (T *v, size_t siz, int unit){
 if ( unit == 1 || unit==-1 ) unit *= sizeof(T);
 if (unit<0) qsort (v, siz, -unit, qsort_cmp__<T>); else qsort (v, siz, unit, qsort_cmp_<T>); 
} 


template<typename T>
void qsort_perm__ (T *v, size_t siz, PERM *perm, int unit){

 if ( unit == 1 || unit==-1 ) unit *= sizeof(*v);  
 if ( unit == 1 || unit==-1 ) unit *= sizeof(PERM);  
 common_INT = MAX(unit,-unit); common_pnt = (char *)v;
 if (unit<0) qsort(perm, siz, sizeof(PERM), qqsort_cmp__<T>);
 else        qsort(perm, siz, sizeof(PERM), qqsort_cmp_<T>);

} 

template<typename T>
PERM *qsort_perm_ (T *v, size_t siz, int unit){

	PERM *perm; 
	malloc2(perm, siz, EXIT0);
	for(size_t i=0 ; i<siz; i++){ perm[i]=i; }
	qsort_perm__<T>(v, siz, perm, unit); 
	return (perm);
}

template<typename T>
size_t bin_search_ (T *v, T u, size_t siz, int unit){ 
 size_t s=0, t;
 T n;
 if ( unit == 1 ) unit *= sizeof (T); 
 while (1){ 
   if ( siz-s <= 2 ){ 
     if ( u <= *((T *)(((char *)v)+unit*s)) ) return (s);
     if ( siz == s+1 || u <= *((T *)(((char *)v)+unit*(s+1))) ) return (s+1);
     return (s+2);
   }
   t = (s+siz) /2;
   n = *((T *)(((char *)v)+unit*t));
   if ( u < n ) siz = t; else s = t;
 }
}

/* swap macro */
template<typename T>
void SWAP_(T *a,T *b){ T stmp = *a; *a=*b; *b=stmp; }


template<typename T ,typename TI>
T* reallocx(T *f, TI *end ,size_t i,T e){

	if( i >= *end ){
		size_t end2 = MAX((*end)*2+16,i+1);

		if(!( f = (T *) realloc( f , sizeof(T) * end2 ) ) ){

			//fprintf(stderr,"memory allocation error: line %d" #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof(T)*(b)));
		}
		for(TI j= *end ; j< end2  ; j++ ){
			f[j]=e;
		}
		*end=MAX((*end)*2,(i)+1);
	}
	return f;

}

template<typename T,typename TI>
T* reallocx(T* f, TI *end ,size_t i){

	if( i >= *end ){

		size_t end2 = MAX((*end)*2+16,i+1);

		if(!( f= (T *)realloc( f ,sizeof(T)*end2 ) ) ){
			//printf("x2 %d\n",end2);
			//fprintf(stderr,"memory allocation error: line %d" #f " (" LONGF " byte)\n",__LINE__,(LONG)((LONG)sizeof(T)*(b)));
		}
		for(TI j= *end ; j<end2 ; j++ ){
			f[j]=j;
		}
		*end=MAX((*end)*2,(i)+1);
	}
	return f;

}



// vectorでいいようなきもするが一応つくる？
template<class T>
class VECARY{
	size_t _end;
	T * _v;
	
	public:
	
	VECARY():_end(0),_v(NULL){}

	~VECARY(){ free2(_v); }

};




/* permute f so that f[i]=f[p[i]] (inverse perm). p will be destroyed (filled by end). s is temporary variable of type same as f[] */
//#define   ARY_INVPERMUTE_(f,p,s,end)  do{ FLOOP(common_INT,0,end){ if ( (p)[common_INT]<(end) ){ (s)=(f)[common_INT]; do { common_INT2=common_INT; common_INT=(p)[common_INT]; (f)[common_INT2]=(f)[common_INT]; (p)[common_INT2]=end; }while ( (p)[common_INT]<(end) ); (f)[common_INT2] = (s);}}}while(0)

/* permute f so that f[i]=f[p[i]] (inverse perm). not destroy p by allocating tmp memory,  s is temporary variable of type same as f[] */
//#define   ARY_INVPERMUTE(f,p,s,end,x) do{ calloc2(common_pnt,end,x);FLOOP(common_INT,0,end){ if ( common_pnt[common_INT]==0 ){ (s)=(f)[common_INT]; do{ common_INT2=common_INT; common_INT=(p)[common_INT]; (f)[common_INT2]=(f)[common_INT]; common_pnt[common_INT2]=1; }while( common_pnt[common_INT]==0 ); (f)[common_INT2] = (s); }} free(common_pnt); }while(0)


