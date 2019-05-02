/*  graph library by array list
            12/Feb/2002    by Takeaki Uno
    homepage:   http://research.nii.ac.jp/~uno/index.html  */
/* This program is available for only academic use, basically.
   Anyone can modify this program, but he/she has to write down 
    the change of the modification on the top of the source code.
   Neither contact nor appointment to Takeaki Uno is needed.
   If one wants to re-distribute this code, please
    refer the newest code, and show the link to homepage of 
    Takeaki Uno, to notify the news about the codes for the users. */

#include"sgraph.hpp"
#include"vec.hpp"


/*  initialization  */
/*
void SGRAPH::alloc(QUEUE_ID nodes, size_t edge_num){

  if ( edge_num > 0 ){
    _edge.alloc(nodes, NULL, nodes, edge_num);
    if ( _flag&LOAD_EDGEW && (!ERROR_MES) ) _edge.alloc_weight ( NULL);
  }
}
*/

// use by lcm ,mace 
int SGRAPH::loadEDGE(int flag ,char* fname){

  _fname = fname;
  
  _flag = flag & (LOAD_ELE + LOAD_EDGEW + LOAD_EDGE + LOAD_RC_SAME + LOAD_ID1 + LOAD_NUM + LOAD_GRAPHNUM);
	_flag |= LOAD_RC_SAME;
  
	// _edge => SETFAMILY
  _edge.load(_flag,_fname);

  _perm = _edge.get_rperm(); 
  _edge.set_rperm(NULL);

  printMes("sgraph: %s ,#nodes %d ,#edges %zd ,#arcs %zd\n", _fname, _edge.get_t(), _edge.get_eles()/2,  _edge.get_eles());

	return 0;
}

char * SGRAPH::initOQ(QUEUE * OQ){

	QUEUE_INT *x;
	VEC_ID    *occ_t;

	// calloc2 ( occ_t, _edge.get_t()+2, EXIT);
	occ_t = new VEC_ID[_edge.get_t()+2]();

	for (VEC_ID iv=0 ; iv< _edge.get_t(); iv++){
		for ( x= _edge.get_vv(iv) ; *x < _edge.get_t() ; x++){ occ_t[*x]++; }
	}

	size_t OQMemSize = 0;
	for(VEC_ID i=0; i < _edge.get_t() ; i++){ OQMemSize += occ_t[i]; }

	char *OQp ;
	// malloc2 (OQp,(OQMemSize + (_edge.get_t()*2)+2)*(sizeof(QUEUE_INT)),  EXIT ); 
	OQp =  malloc2 (OQp,(OQMemSize + (_edge.get_t()*2)+2)*(sizeof(QUEUE_INT)) ); 

	char *cmn_pnt = OQp;

	for(VEC_ID i=0; i < _edge.get_t() ;i++){
		OQ[i].set_endv(occ_t[i],(QUEUE_ID *)cmn_pnt);
		cmn_pnt += (sizeof(QUEUE_INT)) * (occ_t[i]+(2));
	}
	delete [] occ_t;
	return OQp; 


}





