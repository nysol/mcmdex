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
#pragma once

#include "stdlib2.hpp"

/* structure for base array */
#define BASE_UNIT 16
#define BASE_BLOCK 65536

class BASE {

  char **_base;
  
  int _block_siz;  // size of one block of memory
  int _block_num;  // currently using block
  int _unit;  // size of one unit memory
  int _num;   // current position in a block
  size_t _block_end;  // current end of the block
  void *_dellist;

	//void end ();


	public:
	BASE(void):
		_base(NULL),
		_block_siz(0),_block_num(0),
		_unit(0),_num(0),_block_end(-1),
		_dellist(NULL){}

	~BASE(void){

	  for(int i = 0 ; i < _block_end;i++){ 
		  delete[] _base[i];
		}
  	free2 (_base);
  	//delete _base;
	}


	void alloc (int unit, int block_siz);
	int get_num(){ return _num; }
	int get_block_num(){ return _block_num; }
	int get_block_siz(){ return _block_siz; }

	void set_num(int num ){ _num=num; }
	void set_block_num(int num){ _block_num = num; }


	/* increment the current memory block pointer and (re)allcate memory if necessary */
	void *get_memory (int i);
	char *get_base (int i){return _base[i];}

	void dec_block_num(){ _block_num--; }
};




