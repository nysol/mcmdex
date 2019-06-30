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
#include"stdlib2.hpp"
//#include"filecount.hpp"

class OFILE2 {   
	FILE *_fp;
	char *_fname;

	public:
	OFILE2():_fname(NULL),_fp(NULL){}

	OFILE2(char* fname):_fname(fname),_fp(NULL){
		
		if(_fname){
			if(!(_fp=fopen(_fname,"w"))){
				throw("file open error");
			}
		}
	}
	OFILE2(FILE *fp):_fname(NULL),_fp(fp){}

	~OFILE2(){
		if(_fp){ fclose(_fp);}
	}
	void open(char* fname){
		_fname = fname;
		if(fname){
			if(!(_fp=fopen(_fname,"w"))){
				throw("file open error");
			}
		}
		else{
			printf("filenotfound \n");
		}
	}

	bool exist(){ return (_fp != NULL);}

	void print(char *frm ,...){
		va_list ap;
		va_start(ap,frm);
		vfprintf(_fp,frm,ap);	
		va_end(ap);
	}

	void putch(char c){ fputc(c, _fp); }

	void print(double f){
		char s[200];
		size_t i;
		i = sprintf (s, "%f", f);
		while ( s[i-1] == '0' ) i--;
		if ( s[i-1] == '.' ) i--;
		s[i] = 0;
		fprintf(_fp, "%s" , s); //fprintf(_fp, s); //warning
	}

	void print(int f){
		fprintf(_fp,"%d",f);
	}


};

class FILE2{ // in ,out 完全分ける？ 

	FILE *_fp;
	char *_buf_org, *_buf, *_buf_end;   // head/current/tail of buffer
	int _FILE_err;

	/*  
		signals  
		000 : 0: for normal termination
		001 : 1: read a number, then encountered a newline,
		010 : 2: read a number, then encountered the end-of-file
		101 : 5: read no number, and encountered a newline
		110 : 6: read no number, and encountered the end-of-file

		011 : 3
		100 : 4
	*/

	void _fopen2(char *filename ,char *mode){
		if(!(_fp=fopen(filename,mode))){ throw("file open error\n"); }
	}
	
	void _fclose2(){
		if(_fp){ fclose(_fp); _fp=NULL; }
	}

	template<typename T>
	void _ARY_Read(T *f,size_t num);

	template <>
	void _ARY_Read<double>(double *f,size_t num);
	

	public :

	FILE2():
		_fp(NULL),_buf_org(NULL),_buf(NULL)
		,_buf_end(NULL),_FILE_err(0){}

	FILE2(char *fname,char *rw="r"){

		if(fname){ 
			_fopen2( fname,rw );
			_buf_org = new char [FILE2_BUFSIZ+1];
			_buf=_buf_org;
			_buf_end=_buf_org-1;
			*_buf=0;
		}
		else { 
			_fp=NULL;
			_buf_org = NULL;
			_buf = NULL;
			_buf_end = NULL;
			_FILE_err = 0;
		}
	}

	~FILE2(){
  	_fclose2();
		delete [] _buf_org;
		_buf_org = NULL;
	  _buf = _buf_end = 0;
		
	}

	bool Eof   (void)  { return (_FILE_err&2);     } // 2,6   <=eof
	bool NotEof(void)  { return (_FILE_err&2)==0 ; } // 0,1,5 <=noteof
	bool NL(void)      { return (_FILE_err&1);     } // 1,5 newline
	bool Null(void)    { return (_FILE_err&4);     } // 5,6   << read no number
	bool NotEol(void)  { return (_FILE_err&3)==0 ; } // 0 ,(4?)
	bool NotNull(void) { return (_FILE_err&4)==0 ; } // 0,1,2 << read a number or normal termination
	bool EolOrEof(void){ return (_FILE_err&3);     } // 1,2,5,6

		

	void open(char *fname,char *rw) 
	{
		if(fname){ _fopen2( fname,rw );}
		else { _fp=NULL;}

		//_buf_org = malloc2(_buf_org,FILE2_BUFSIZ+1);
		_buf_org = new char [FILE2_BUFSIZ+1];
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		*_buf=0;
	}

	void open(FILE *a) {
		_fp=a;
		_buf_org = new char [FILE2_BUFSIZ+1]; //malloc2
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		*_buf=0;
	}

	void close (){
  	_fclose2();
		delete [] _buf_org; //free2
		_buf_org = NULL;
	  _buf = _buf_end = 0;
	}

	void clear(void){ 
		delete [] _buf_org;
		_buf_org = NULL;
	}

	void reset (){
		_buf = _buf_org;
		_buf_end = _buf_org-1;
		fseek (_fp, 0, SEEK_SET);
	}

	bool exist_buf(){ return _buf!=NULL; }
	bool exist()    { return _fp!=NULL; }

	size_t ARY_Scan_INT(void);
	size_t ARY_Scan_DBL(void);

	template<typename T>
	void VARY_Read(VECARY<T> &vec,size_t num);

	template <>
	void VARY_Read<double>(VECARY<double> &vec,size_t num);

	int getc();
	FILE_LONG read_int ();
	double read_double ();
	WEIGHT read_WEIGHT ();

	int  read_pair( LONG *x, LONG *y, WEIGHT *w, int flag);
	int  read_pair( LONG *x, LONG *y );

	int  read_item(FILE2 *wfp, LONG *x, LONG *y, WEIGHT *w, int fc, int flag);
	int  read_item(LONG *x, LONG *y, int flag);

	void read_until_newline ();

	// write case
	void closew();

	bool needFlush(void){ return ( _buf-_buf_org ) > FILE2_BUFSIZ/2 ; }
	void flush_last(void);
	void flush_(void);

	void putch(char c){
	  *_buf = c;
  	_buf++;
	}

	void puts(char *s){
	  while ( *s != 0 ){
  	  *_buf = *s;
			s++;
    	_buf++;
	  }
	}

	void print_int( LONG n, char c);
	void print_real(double n, int len, char c);
	void print_WEIGHT( WEIGHT w, int len, char c);
	void flush ();

	// 以下 class method
	static FILE2 * makeMultiFp(int size,FILE2 &a);

	static int ARY_Load(int *f,char* fname);
	static int ARY_Load(long long *f,char* fname);
	static int ARY_Load(unsigned int *f,char* fname);
	static int ARY_Load(double *f,char* fname);

	static void ARY_Write(char* fname, int *p ,size_t size);
	static void copy(char *f1, char *f2); //simsetでいる

};

/*
	// FILE2_print_mpintとかを使うなら復活させる
	char _bit; 

	fileerrのチェック方法をdで帰る場合は復活させる
	size_t ARY_Scan_INT(int d);
	size_t ARY_Scan_DBL(int d);
	static int ARY_Load(int *f,char* fname,int d);
	static int ARY_Load(long long *f,char* fname,int d);
	static int ARY_Load(unsigned int *f,char* fname,int d);
	static int ARY_Load(double *f,char* fname,int d);
*/


