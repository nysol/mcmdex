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

#if defined(__cplusplus) && defined(__GNUC__)
 #define _cplusplus_
#endif

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

#ifdef MULTI_CORE
#include <sys/types.h>
#include <pthread.h>
int SPIN_LOCK_dummy;
#endif

#define CORE_MAX 16

#ifdef MTWISTER
#include"dSFMT.c"
#endif

// definition of the process for errors
#ifdef ERROR_RET  
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

extern char *common_pnt;
extern INT common_INT;

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

/*  macro for getting maximum/minimum of two values  */
#define MAX(a,b)      (((a)>(b))?(a):(b))
#define ENMAX(a,b)      ((a)=(((a)>(b))?(a):(b)))
#define MIN(a,b)      (((a)<(b))?(a):(b))
#define ENMIN(a,b)      ((a)=(((a)<(b))?(a):(b)))

/*  error routine  */
#define error(mes,x)        do{fprintf(stderr,"%s\n",mes);x;}while(0)


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

#define FILE_COUNT_ROWT 32   // count size of each row
#define FILE_COUNT_CLMT 64   // count size of each column
#define FILE_COUNT_NUM LOAD_NUM   // read #columns, #rows and #elements
#define FILE_COUNT_GRAPHNUM LOAD_GRAPHNUM   // read #vertices and #edges
#define FILE2_BUFSIZ 16384
#define FILE_COUNT_INT VEC_ID
#define FILE_COUNT_INTF VEC_IDF

// swap macro
template<typename T>
void SWAP_(T *a,T *b){ T stmp = *a; *a=*b; *b=stmp; }

/* macros for allocating memory with exiting if an error occurs */
#define free2(a)   do{if(a){free(a);(a)=NULL;}}while(0)

// free many pointers 
void mfree_(void *x, ...);

#define mfree(...)          mfree_(NULL, __VA_ARGS__, (void *)1)


// ==============================
// template For ARRAY 
// ==============================
template<typename T,typename Tz>
T ARY_MAX( T *f,Tz x, Tz y){

	T m = f[x];
	for(Tz i0 = x+1 ; i0 < y ; i0++){
		if( m < f[i0]) { 
			m = f[i0];
		}
	}
	return m;
}
template<typename T,typename Tz>
T ARY_MIN( T *f , Tz x, Tz y){

	T m = f[x];
	for(Tz i0 = x+1 ; i0 < y ; i0++){
		if( m > f[i0] ){
			m = f[i0];
		}
	}
	return m;
}
template<typename T,typename Tz>
T ARY_SUM( T *v , Tz x,Tz y)       
{
	T f = 0;
	for(Tz i = x ; i < y ; i++){
		f += v[i];
	}
	return f;
}


// ====================================================
// memmory allcate function
// ====================================================
template<typename T,typename Tz>
T* malloc2(T* f ,Tz b){
	if(!( f = (T*)malloc(sizeof(T)*b))){
		throw("memory allocation error : malloc2");
	}
	return f;
}

template<typename T,typename Tz>
T* calloc2(T* f ,Tz b){
	if(!( f = (T*)calloc(sizeof(T),b))){
		throw("memory allocation error : calloc2");
	}
	return f;
}
/*
template<typename T,typename Tz>
T* realloc2(T* f ,Tz b){

	if(!( f = (T*)realloc( f, sizeof(T)*(b) ))) {
		throw("memory allocation error : realloc2");
	}
	return f;
}

template<typename T,typename Tz>
T* realloci(T* f,Tz i){
	if( !( i & (i-1) )){
		f = realloc2( f, i*2+1 );
	}
	return f;
}

template<typename T ,typename TI>
T* reallocx(T *f, TI *end ,size_t i,T e){

	if( i >= *end ){
		size_t end2 = MAX((*end)*2+16,i+1);

		if(!( f = (T *) realloc( f , sizeof(T) * end2 ) ) ){
			fprintf(stderr,"memory allocation error: line %d (" LONGF " byte)\n",__LINE__,(LONG)(sizeof(T)*(end2)) );
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
			fprintf(stderr,"memory allocation error: line %d (" LONGF " byte)\n",__LINE__,(LONG)(sizeof(T)*(end2)) );
		}
		for(TI j= *end ; j<end2 ; j++ ){
			f[j]=j;
		}
		*end=MAX((*end)*2,(i)+1);
	}
	return f;

}

*/

// ====================================================
// sorting function
// ===================================================
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

	PERM *perm = new PERM[siz]; 
//	perm = malloc2(perm, siz);
	for(size_t i=0 ; i<siz; i++){ perm[i]=i; }
	qsort_perm__<T>(v, siz, perm, unit); 
	return perm;
}

template<typename T>
size_t bin_search_(T *v, T u, size_t siz, int unit){ 
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

// ====================================================
// vectorでいいようなきもするが独自ARAY一応つくる？
// 
// ====================================================
template<class T>
class VECARY{

	size_t _end;
	T * _v;
	
	public:
	
	VECARY():_end(0),_v(NULL){}

	~VECARY(){ 
		free(_v); 
		_v=NULL;
	}
	// 仮
	T* getp(){ return _v;}

	void resize(size_t sz){
		if(!( _v = (T*)realloc( _v, sizeof(T)*(sz) ))) {
			throw("memory allocation error : VECARY");
		}
		_end = sz;
	}

	bool empty(){ return _end==0; }

	T &operator[](size_t i){ return _v[i]; }
	

	void realloc2(size_t b){
		if(!( _v = (T*)realloc( _v, sizeof(T)*(b) ))) {
			throw("memory allocation error : VECARY realloc2");
		}
		_end = b;
		return;
	}

	void realloci(size_t i){
		if( !( i & (i-1) )){
			if(!( _v = (T*)realloc( _v, sizeof(T)*(i*2+1) ))) {
				throw("memory allocation error : VECARY realloci ");
			}
			_end = i*2+1; 
		}
	}

	void malloc2(size_t b){
		if(!( _v = (T*)malloc(sizeof(T)*b))){
			throw("memory allocation error : VECARY malloc2");
		}
		_end = b;
	}

	size_t reallocx( size_t end ,size_t i,T e){
		// _endのみでいけるはず
		if( i >= end ){

			size_t end2 = MAX((end)*2+16,i+1);

			if(!( _v = (T *) realloc(  _v , sizeof(T) * end2 ) ) ){
				fprintf(stderr,"memory allocation error: line %d (" LONGF " byte)\n",__LINE__,(LONG)(sizeof(T)*(end2)) );
			}
			for(size_t j= end ; j< end2  ; j++ ){
				_v[j]=e;
			}
			end=MAX((end)*2,(i)+1);
			_end = end;
		}
		return end;
	}

	size_t reallocSeq( size_t end ,size_t i){
	
		if( i >= end ){

			size_t end2 = MAX((end)*2+16,i+1);
			if(!(  _v= (T *)realloc( _v ,sizeof(T)*end2 ) ) ){
				fprintf(stderr,"memory allocation error: line %d (" LONGF " byte)\n",__LINE__,(LONG)(sizeof(T)*(end2)) );
			}
			for(size_t j= end ; j<end2 ; j++ ){
				_v[j]=j;
			}
			end=MAX((end)*2,(i)+1);
			_end = end;
		}
		return end;

	}

	PERM* qsort_perm(size_t size,int flg){
    return qsort_perm_<T>(_v, size, flg);
	}

	static void swap( VECARY<T> &a , VECARY<T> &b ){

		size_t t0 = a._end;
		T * t1 = a._v;
		a._end = b._end;
		a._v = b._v;
		b._end = t0;
		b._v = t1;
	}


	T min( size_t x, size_t y){
		T m = _v[x];
		for(size_t i0 = x+1 ; i0 < y ; i0++){
			if( m > _v[i0] ){
				m = _v[i0];
			}
		}
		return m;
	}

	T max( size_t x, size_t y){

		T m = _v[x];
		for(size_t i0 = x+1 ; i0 < y ; i0++){
			if( m < _v[i0]) { 
				m = _v[i0];
			}
		}
		return m;
	}
	
};

