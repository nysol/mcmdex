/*  Common problem input/output routines /structure
            25/Nov/2007   by Takeaki Uno  e-mail:uno@nii.jp, 
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

/***************************************************/
#pragma once

/*****  parameters for PROBLEM initialization, given to flag  *****/

#define PROBLEM_FREQSET 1
#define PROBLEM_MAXIMAL 2
#define PROBLEM_CLOSED 4

#define PROBLEM_NORMALIZE   32  // print density threshold

#define PROBLEM_ITEMARY 128 // alloc itemary
#define PROBLEM_ITEMJUMP 256 // alloc itemjump
#define PROBLEM_ITEMCAND 2048 // alloc itemcand
#define PROBLEM_VECCHR 262144  //alloc vecchr
#define PROBLEM_OCC_T 524288 // alloc occ_t
#define PROBLEM_SHIFT 1048576  // allocate shift
#define PROBLEM_OCC_W 2097152  // weight/positive-weight sum for items

#define PROBLEM_EX_MAXIMAL 8
#define PROBLEM_EX_CLOSED 16

#define PROBLEM_OCC_PW 4194304  // weight/positive-weight sum for items
#define PROBLEM_OCC_W2 8388608  // weight/positive-weight sum for items
