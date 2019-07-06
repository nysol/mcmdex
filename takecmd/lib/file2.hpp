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

class FILE2{ // in ,out 完全分ける？ 


	protected:

	FILE *_fp;
	char *_fname;
	char *_buf_org, *_buf, *_buf_end;   // head/current/tail of buffer

	void _fopen2(char *filename ,const char *mode){
		if(!(_fp=fopen(filename,mode))){ throw("file open error\n"); }
	}
	
	void _fclose2(){
		if(_fp){  
			fclose(_fp); 
			_fp=NULL; 
		}
	}

	FILE2(void):
		_fname(NULL),_fp(NULL),_buf_org(NULL),_buf(NULL)
		,_buf_end(NULL){}

	FILE2(char* fname,const char *aw){

		_fname = fname;

		if(_fname){ 
			_fopen2(_fname , aw );
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
		}
	}

	FILE2(FILE *a) {
		_fp=a;
		_buf_org = new char [FILE2_BUFSIZ+1]; //malloc2
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		*_buf=0;
	}


	~FILE2(void){
  	_fclose2();
		delete [] _buf_org;
		_buf_org = NULL;
	  _buf = _buf_end = 0;
	}

	public :
	
	void open(char *fname,const char *rw) 
	{
		_fname = fname;

		if(fname){ _fopen2( fname,rw );}
		else { _fp=NULL;}
		
		if( _buf_org ){
			delete [] _buf_org;
			_buf_org = NULL;
		} 
		_buf_org = new char [FILE2_BUFSIZ+1];
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		*_buf=0;
	}

	void open(FILE *a) {
		_fp=a;
		if( _buf_org ){
			delete [] _buf_org;
			_buf_org = NULL;
		} 
		_buf_org = new char [FILE2_BUFSIZ+1]; //malloc2
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		*_buf=0;
	}

	void clear(void){ 
		delete [] _buf_org;
		_buf_org = NULL;
		_buf = NULL;
		_buf_end = NULL;
	}

	void close(){
  	_fclose2();
  	delete [] _buf_org;
		_buf_org = NULL;
		_buf = NULL;
		_buf_end = NULL;
	}

	bool exist_buf(){ return _buf!=NULL; }
	bool exist()    { return _fp!=NULL; }
	
	static void copy(char *f1, char *f2); //simsetでいる

};



class OFILE2: public FILE2 {   

	public:

	OFILE2(){}

	OFILE2(char* fname,const char *aw="w"):FILE2(fname,aw){}

	OFILE2(FILE *fp):FILE2(fp){}

	void open(char *fname){ FILE2::open(fname,"w"); }
	void openA(char *fname){ FILE2::open(fname,"a"); }
	void open(FILE *fp){ FILE2::open(fp); }

	void print(char *frm ,...){
		va_list ap;
		va_start(ap,frm);
		vfprintf(_fp,frm,ap);	
		va_end(ap);
	}

	// write case
	void close(){
	  flush_last();
  	_fclose2();
  	delete [] _buf_org;
  	_buf_org = NULL;
  	_buf = _buf_end = 0;
	}

	void clearbuf(){
	  flush_last();
  	_fp=NULL;
  	delete [] _buf_org;
  	_buf_org = NULL;
  	_buf = _buf_end = 0;
	}

	
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

	//void putch(char c){ fputc(c, _fp); }

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

	void print_int( LONG n, char c);
	void print_real(double n, int len, char c);
	void print_WEIGHT( WEIGHT w, int len, char c);
	void flush ();
	// 以下 class method
	static OFILE2 * makeMultiFp(int size,OFILE2 &a);
	static void ARY_Write(char* fname, int *p ,size_t size);
};





class IFILE2 : public FILE2{ // in ,out 完全分ける？ 

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
	void _ARY_Read(int *f,size_t num);
	void _ARY_Read(long long *f,size_t num);
	void _ARY_Read(unsigned int *f,size_t num);
	void _ARY_Read(double *f,size_t num);


	public :

	IFILE2():_FILE_err(0){}
	IFILE2(char *fname):_FILE_err(0),FILE2(fname,"r"){}

	void open(char *fname){ FILE2::open(fname,"r"); }

	
	bool Eof   (void)  { return (_FILE_err&2);     } // 2,6   <=eof
	bool NotEof(void)  { return (_FILE_err&2)==0 ; } // 0,1,5 <=noteof
	bool NL(void)      { return (_FILE_err&1);     } // 1,5 newline
	bool Null(void)    { return (_FILE_err&4);     } // 5,6   << read no number
	bool NotEol(void)  { return (_FILE_err&3)==0 ; } // 0 ,(4?)
	bool NotNull(void) { return (_FILE_err&4)==0 ; } // 0,1,2 << read a number or normal termination
	bool EolOrEof(void){ return (_FILE_err&3);     } // 1,2,5,6

	void reset(){
		_buf = _buf_org;
		_buf_end = _buf_org-1;
		fseek(_fp, 0, SEEK_SET);
	}

	size_t ARY_Scan_INT(void);
	size_t ARY_Scan_DBL(void);

	int getc();
	FILE_LONG read_int ();
	double read_double ();
	WEIGHT read_WEIGHT ();

	int  read_pair( LONG *x, LONG *y, WEIGHT *w, int flag);
	int  read_pair( LONG *x, LONG *y );

	int  read_item(IFILE2 *wfp, LONG *x, LONG *y, WEIGHT *w, int fc, int flag);
	int  read_item(LONG *x, LONG *y, int flag);

	void read_until_newline ();

	void VARY_Read(VECARY<WEIGHT> &vec,size_t num);


	// 以下 class method

	static int ARY_Load(int *f,char* fname);
	static int ARY_Load(long long *f,char* fname);
	static int ARY_Load(unsigned int *f,char* fname);
	static int ARY_Load(double *f,char* fname);


};

/*
	template<typename T>
	void _ARY_Read(T *f,size_t num);

	template <>
	void _ARY_Read<double>(double *f,size_t num);

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


