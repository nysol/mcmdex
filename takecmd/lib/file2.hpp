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

	void putc(char c){ fputc(c, _fp); }

	void print(double f){
		char s[200];
		size_t i;
		i = sprintf (s, "%f", f);
		while ( s[i-1] == '0' ) i--;
		if ( s[i-1] == '.' ) i--;
		s[i] = 0;
		//fprintf(_fp, s); //warning
		fprintf(_fp, "%s" , s);
	}

	void print(int f){
		fprintf(_fp,"%d",f);
	}


};

class FILE2 {   

	FILE *_fp;
	char *_buf_org, *_buf, *_buf_end;   // head/current/tail of buffer
	char _bit;
	int _FILE_err;

	/*  
		signals  
		0: for normal termination
		1: read a number, then encountered a newline,
		2: read a number, then encountered the end-of-file
		5: read no number, and encountered a newline
		6: read no number, and encountered the end-of-file
	*/


	void _fopen2(char *filename ,char *mode){
		if(!(_fp=fopen(filename,mode))){
			throw("file open error\n");
		}
	}
	void _fclose2(){
		if(_fp){
			fclose(_fp);
			_fp=NULL;
		}
		
	}


	public :

	bool readOK(void){ return (_FILE_err&4)==0 ; } 
	bool remain(void){ return (_FILE_err&3)==0 ; }
	bool eof   (void){ return (_FILE_err&2)==0 ; } //<=noteof

	bool getOK1(void){ return (_FILE_err&1); }
	bool eofx  (void){ return (_FILE_err&2) ; } //<=noteof
	bool getOK(void) { return (_FILE_err&3); }
	bool readNG(void){ return (_FILE_err&4); }

	FILE2():
		_fp(NULL),_buf_org(NULL),_buf(NULL)
		,_buf_end(NULL),_bit(0),_FILE_err(0){}
		
	void open(char *fname,char *rw) 
	{
		if(fname){ _fopen2( fname,rw );}
		else { _fp=NULL;}
		_buf_org = malloc2(_buf_org,FILE2_BUFSIZ+1);
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		_bit=0;
		*_buf=0;
	}

	void open(FILE *a) {
		_fp=a;
		//_buf_org = malloc2(_buf_org,FILE2_BUFSIZ+1);
		_buf_org = new char [FILE2_BUFSIZ+1]; //malloc2(_buf_org,FILE2_BUFSIZ+1);
		_buf=_buf_org;
		_buf_end=_buf_org-1;
		_bit=0;
		*_buf=0;
	}


	bool exist_buf(){ return _buf!=NULL; }
	bool exist(){ return _fp!=NULL; }

	void flush_last (void);
	void flush_ (void);
	void clear(void){ 
		delete [] _buf_org;
		_buf_org = NULL;
		//free2 (_buf_org);
	}

	bool needFlush(void){ return ( _buf-_buf_org ) > FILE2_BUFSIZ/2 ; }
	
	
	static FILE2 * makeMultiFp(int size,FILE2 &a){
		FILE2 *mfp = new FILE2[size];
		for(int i=0;i<size;i++){
			mfp[i].open(a._fp);
		}
		return mfp;
	}


	static int ARY_Load(int *f,char* fname,int d){
		FILE2 cmn;
		int num;
		cmn.open(fname,"r");
		num = cmn.ARY_Scan_INT(d);

		f = new int[num+1];

		cmn.reset();
		cmn.ARY_Read(f,num);
		cmn.close();
		return num;
	}
		
	static int ARY_Load(long long *f,char* fname,int d){
		FILE2 cmn;
		int num;

		cmn.open(fname,"r");
		num = cmn.ARY_Scan_INT(d);

		//f = malloc2(f,(num)+1);
		f = new long long[num+1];

		cmn.reset();
		cmn.ARY_Read(f,num);
		cmn.close();
		return num;

	}

	static int ARY_Load(unsigned int *f,char* fname,int d){
			
		FILE2 cmn;
		int num;
		
		cmn.open(fname,"r");
		num = cmn.ARY_Scan_INT(d);
		
		//f = malloc2(f,(num)+1);
		f = new unsigned int[num+1];
		cmn.reset();
		cmn.ARY_Read(f,num);
		cmn.close();
		return num;
	}

	static int ARY_Load(double *f,char* fname,int d){

		FILE2 cmn;
		int num;

		cmn.open(fname,"r");
		num = cmn.ARY_Scan_DBL(d);
		f = new double[num+1];
		cmn.reset();
		cmn.ARY_Read(f,num);
		cmn.close();
		return num;
	}

	static void ARY_Write(char* fname, int *p ,size_t size){

  	OFILE2 fp(fname);
		
		for( size_t i=0 ; i< size ;i++){
			fp.print( "%d " ,p[i]);
		}
		fp.putc('\n');
	}
	/*simsetでいる？
	static void copy(char *f1, char *f2){

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
	}*/

	size_t ARY_Scan_INT(int d){

		size_t num=0;

		do{

			do{ read_int(); } while((_FILE_err&((d)*5))==5);
	
			if(RANGE(5+(int)(d),_FILE_err,6))break;
	
			(num)++;

		}while((_FILE_err&(3-(int)(d)))==0);
	
		return num;
	}

	size_t ARY_Scan_DBL(int d){

		size_t num=0;

		do{

			do{ read_double(); } while((_FILE_err&((d)*5))==5);
	
			if(RANGE(5+(int)(d),_FILE_err,6))break;
	
			(num)++;

		}while((_FILE_err&(3-(int)(d)))==0);
	
		return num;
	}

	void reset (){
		_buf = _buf_org;
		_buf_end = _buf_org-1;
		fseek (_fp, 0, SEEK_SET);
	}

	template<typename T>
	void ARY_Read(T *f,size_t num) {

		for (size_t i=0 ; i < num  ; i++){

 			do{
 				f[i]= read_int();
	 		}while((_FILE_err&6)==4);

 			if(_FILE_err&2)break;
	 	}
	}
	template <>
	void ARY_Read<double>(double *f,size_t num) {

		for (size_t i=0 ; i < num  ; i++){
			do{
				f[i]=read_double();
			}while((_FILE_err&6)==4);

		 	if(_FILE_err&2)break;
	 	}
	}

	template<typename T>
	void VARY_Read(VECARY<T> &vec,size_t num) {

		for (size_t i=0 ; i < num  ; i++){
			do{
				vec[i]=read_int();
			}while((_FILE_err&6)==4);

		 	if(_FILE_err&2)break;
	 	}
	}


	template <>
	void VARY_Read<double>(VECARY<double> &vec,size_t num) {

		for (size_t i=0 ; i < num  ; i++){
			do{
				vec[i]=read_double();
			}while((_FILE_err&6)==4);

		 	if(_FILE_err&2)break;
	 	}
	}



	void close (){
  	_fclose2();
 		//free2 (_buf_org);
		delete [] _buf_org;
		_buf_org = NULL;


	  _buf = _buf_end = 0;
	}

	FILE_LONG read_int ();
	double read_double ();
	WEIGHT read_WEIGHT ();


	bool exist_fp(){return _fp!=NULL;}

	int getc();

	void putc ( char c){
	  *(_buf) = c;
  	_buf++;
	}
	void puts ( char *s){
	  while ( *s != 0 ){
  	  *(_buf) = *s;
			s++;
    	_buf++;
	  }
	}
	void read_until_newline ();
	int  read_pair ( LONG *x, LONG *y, WEIGHT *w, int flag);
	int  read_item (FILE2 *wfp, LONG *x, LONG *y, WEIGHT *w, int fc, int flag);
	void closew();
	void print_int ( LONG n, char c);
	void print_real (double n, int len, char c);
	void print_WEIGHT ( WEIGHT w, int len, char c);
	void flush ();

};


