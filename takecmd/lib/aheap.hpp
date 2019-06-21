/*
    array-based simple heap (fixed size)
            12/Apr/2001   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

/* bench mark
  PentiumIII 500MHz, Memory 256MB Linux
  values 0-1,000,000 : set & del & get  1,000,000 times
  2.55sec

  # rotation  == 1/5 per 1 set/del

   *** simple array *** 
   value 0-1,000,000 set & set & set   1,000,000 times,
   0.88sec 
  */
#pragma once

#include"stdlib2.hpp"

#define AHEAP_KEY_DOUBLE

#ifdef AHEAP_KEY_DOUBLE
	#define AHEAP_KEY  double
	#define AHEAP_KEYHUGE DOUBLEHUGE
	#define AHEAP_KEYF "%f"
#else
	#define AHEAP_KEY  int
	#define AHEAP_KEYHUGE INTHUGE
	#define AHEAP_KEYF "%d"
#endif

#define AHEAP_ID int
#define AHEAP_ID_HUGE INTHUGE
#define AHEAP_IDF "%d"

class AHEAP {

	
	/* update the ancestor of node i */
	void update(AHEAP_ID i){
  	AHEAP_ID j;
  	AHEAP_KEY a = _v[i];
  	while ( i>0 ){
    	j = i - 1 + (i%2)*2;   /* j = the sibling of i */
    	i = (i-1) / 2;
    	if ( _v[j] < a ) a = _v[j];
    	if ( a == _v[i] ) break;
		  _v[i] = a;
  	}
	}


	AHEAP_ID IDX (AHEAP_ID i){ return (i+1-_base)%_end; }
	AHEAP_ID LEAF(AHEAP_ID i){ return (i+_base)%_end + _end-1;}

  AHEAP_KEY *_v;       /* array for heap key */
  AHEAP_ID _end;       /* the number of maximum elements */
  AHEAP_ID _base;      /* the constant for set 0 to the leftmost leaf */

	public:

		AHEAP(void):_v(NULL),_end(0),_base(0){}

		AHEAP(AHEAP_ID num){

			if ( num>0 ) {
				_v = new AHEAP_KEY[num*2];
				//_v = malloc2 (_v, num*2);
			}

  		_end = num;
			for(size_t i =0 ;i<num*2;i++){ 
				_v[i] = AHEAP_KEYHUGE; 
			}

		  AHEAP_ID i;
		  for (i=0 ; i<num-1 ; i=i*2+1);

		  _base = i - num + 1;
		
		}

		~AHEAP(){ 
			delete [] _v; 
			//free2 (_v);  
		}

		// アクセッサ
		AHEAP_ID end(void){ return _end;}
		AHEAP_ID base(void){ return _base;}

		void end(AHEAP_ID end){ _end=end;}
		void base(AHEAP_ID base){ _base=base;}




		AHEAP_KEY H(AHEAP_ID i){ 
			return _v[(i+_base)%_end+_end-1];
		}

		AHEAP_ID findmin_head (){ 
		  if ( _end <= 0 ) return (-1);
		  AHEAP_ID i = 0;
		  while ( i < _end-1 ){
		    if ( _v[i*2+1] == _v[i] ){ i = i*2+1; }
    		else                     { i = i*2+2; }
  		}
	  	return IDX(i);
		}
		
		void alloc (AHEAP_ID num){
  		AHEAP_ID i;
  		if ( num>0 ){
				//_v = malloc2 (_v, num*2);
				_v = new AHEAP_KEY[num*2];

			}
  		_end = num;

			for(size_t i =0 ;i<num*2;i++){ 
				_v[i] = AHEAP_KEYHUGE; 
			}
  		
  		
  		for (i=0 ; i<num-1 ; i=i*2+1);
  		_base = i - num + 1;
		}
		/* change the key of node i to a /Add a to the key of node i, and update heap H */
		void chg (AHEAP_ID i, AHEAP_KEY a){
		  i = LEAF (i);
  		_v[i] = a;
  		update (i);
		}

		void alloc(AHEAP_ID siz,AHEAP_KEY val){
			LONG i;
			alloc(siz);
      //FLOOP (i, 0, siz){
			for(int i = 0 ; i<siz ; i++){
      	chg( (AHEAP_ID)i, val);
      }
		
		}

		/* print heap keys according to the structure of the heap */
		void print (){
		  AHEAP_ID i, j=1;
		  printf("hend %d\n",_end);
  		while ( j<=_end*2-1 ){
    		//FLOOP (i, j-1, MIN(j, _end)*2-1) {
				for(i = j-1 ; i <  MIN(j, _end)*2-1 ; i++){
    			printf (AHEAP_KEYF ",%d", _v[i] ,i);
    		}
    		printf ("\n");
    		j = j*2;
  		}
		}

} ;





