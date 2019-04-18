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
  //*B = INIT_BASE;
  //_dellist = B;
  _unit = unit;
  _block_siz = block_siz;
  _num = block_siz;
  _block_num = -1;

  //calloc2 (_base, 20, EXIT);
  _base = calloc2 (_base, 20);

  _block_end = 20;
}


/* increment the current memory block pointer and (re)allcate memory if necessary */
void *BASE::get_memory (int i){
  _num += i;
  if ( _num >= _block_siz ){  /* if reach to the end of base array */
    _num = i;  /* allocate one more base array, and increment the counter */
    _block_num++;

    //reallocx(_base, _block_end, _block_num, NULL, EXIT0);
		_base = reallocx<char *, size_t>(_base, &_block_end, _block_num, NULL);


    if ( _base[_block_num] == NULL ){
      //malloc2 (_base[_block_num], _block_siz*_unit, EXIT0);
    	_base[_block_num] = malloc2(_base[_block_num],  _block_siz*_unit);
    }
    return (_base[_block_num]);
  }
  return (_base[_block_num] + (_num-i)*_unit);
}




