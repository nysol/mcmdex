/*
    blocked memory allocation library
            12/Mar/2002   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#include"base.hpp"


/*  initialization, and allocate memory for header */
void BASE::alloc (int unit, int block_siz){

  _unit = unit;
  _block_siz = block_siz;
  _num = block_siz;
  _block_num = -1;

  //calloc2 (_base, 20, EXIT);
	if(!( _base = (char **)calloc(sizeof(char *),20))){
		throw("memory allocation error : calloc2");
	}
  _block_end = 20;

}


/* increment the current memory block pointer and (re)allcate memory if necessary */
void *BASE::get_memory (int i){
  _num += i;
  if ( _num >= _block_siz ){  /* if reach to the end of base array */
    _num = i;  /* allocate one more base array, and increment the counter */
    _block_num++;
		// reallocx
		if( _block_num >= _block_end ){
			size_t end2 = MAX((_block_end)*2+16,_block_num+1);

			if(!( _base = (char**) realloc( _base , sizeof(char*) * end2 ) ) ){
				fprintf(stderr,"memory allocation error: line %d (" LONGF " byte)\n",__LINE__,(LONG)(sizeof(char*)*(end2)) );
			}
			for(size_t j= _block_end ; j< end2  ; j++ ){
				_base[j]=NULL;
			}
			_block_end=MAX((_block_end)*2,(_block_num)+1);
		}
    if ( _base[_block_num] == NULL ){
    	//_base[_block_num] = malloc2(_base[_block_num],  _block_siz*_unit);
    	_base[_block_num] = new char[_block_siz*_unit];


    }
    return (_base[_block_num]);
  }
  return (_base[_block_num] + (_num-i)*_unit);
}




