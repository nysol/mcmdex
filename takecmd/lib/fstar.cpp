/* forward star type graph structure -- for quite large graphs */
/* 6/Nov/2008   Takeaki Uno  */

#include"fstar.hpp"
#include"stdlib2.hpp"
#include"filecount.hpp"

/* increment degrees */
void FSTAR::inc_deg (FSTAR_INT x, FSTAR_INT y)
{
  if ( _out_deg ) _out_deg[x]++;
  if ( _out_deg && (_flag&LOAD_EDGE) ) _out_deg[y]++;
  if ( _in_deg ) _in_deg[y]++;
  if ( ((_flag & FSTAR_CNT_DEG_ONLY) || _edge_dir==0) && _in_deg ) _in_deg[x]++;
}

/* 
	remove edges adjacent to 
	small/large degree vertices 
*/
void FSTAR::sort_adjacent_node (int flag){

  FSTAR_INT x, d=0, y, s = sizeof(FSTAR_INT)+sizeof(WEIGHT);
  FSTAR_INT *p;  //char *p; //なぜchar*
  
  if ( _edge_w ){

    // sort with weight; 
    // make array of (ID,weight) and sort it
		for(x=0;x<_out_node_num;x++){
    	ENMAX (d, _fstar[x+1]-_fstar[x]);
    }

		//ここサイズおかしくなるはず
    //p = malloc2 (p, d * (sizeof(FSTAR_INT)+sizeof(WEIGHT))); 
    p = new FSTAR_INT [d * (sizeof(FSTAR_INT)+sizeof(WEIGHT))];

		for(x=0;x<_out_node_num;x++){

			for(y=0;y<_fstar[x+1]-_fstar[x];y++){
        *((FSTAR_INT *)(&p[y*s])) = _edge[y+_fstar[x]];
        *((WEIGHT *)(&p[y*s+sizeof(FSTAR_INT)])) = _edge_w[y+_fstar[x]];
      }
      //ここだいじょうぶ？
      qsort_<FSTAR_INT> (p, _fstar[x+1]-_fstar[x], flag*s); 

			for(y=0;y<_fstar[x+1]-_fstar[x];y++){
        _edge[y+_fstar[x]]   = *((FSTAR_INT *)(&p[y*s]));
        _edge_w[y+_fstar[x]] = *((WEIGHT *)(&p[y*s+sizeof(FSTAR_INT)]));
      }
    }
    delete [] p ;     //free2 (p);

  }
  else {
		for(x=0;x<_out_node_num;x++){
			qsort_<FSTAR_INT> (&_edge[_fstar[x]], _fstar[x+1]-_fstar[x], flag);
    }
  }
}

/* 
	compute node_num's and allocate arrays for degree 
*/
LONG FSTAR::alloc_deg (){

  LONG i = _out_node_num, j;
  _out_node_num = _xmax;
  _in_node_num = _ymax;

  j = _out_node_num; 
  ENMAX (_out_node_num, i);
  _node_num = MAX (_out_node_num, _in_node_num);
  
  if ( _edge_dir == 0 ){
  	_in_node_num = _out_node_num = _node_num;
  }

  _fstar = new FSTAR_INT[_out_node_num+2](); // calloc2

  if ( _flag & LOAD_EDGE ) return j;

  if ( _flag & FSTAR_CNT_IN ){
    if ( _flag & FSTAR_CNT_DEG_ONLY ){
      _in_deg = new FSTAR_INT[_node_num+2](); //calloc2 
    } else {
      _in_deg = new FSTAR_INT[ _in_node_num+2](); // calloc2
    }
  }

  if ( _flag & FSTAR_CNT_OUT ){
    _out_deg = new FSTAR_INT[_out_node_num+2](); // calloc2
  }

  return j;

}

/* 
	compute fstar from outdegree stored in fstar itself, 
	and allocate edge array 
*/
void FSTAR::calc_fstar (){

  FSTAR_INT i, j=0, jj;

  for(i=0;i<_out_node_num;i++){
    jj = j + _fstar[i];
    _fstar[i] = j;
    j = jj;
  }

  _fstar[i] = _edge_num = j;
  _edge = new FSTAR_INT[_edge_num +2]; // malloc2
  
	for(size_t i =0 ;i<_edge_num +2 ;i++){ _edge[i] = _node_num+1; }

  if ( (_flag&LOAD_EDGEW) || _wfname ) {
  	_edge_w = new WEIGHT[_edge_num +2]; //malloc2
  }

}

/* return 1 if edge (x,y) is valid */
/* head != 0 means x is the origin, otherwise x is destination */
int FSTAR::eval_edge (FSTAR_INT x, FSTAR_INT y, WEIGHT w){

  if ( x>_node_num || y>_node_num || x<0 || y<0 ) return (0);

  if ( ((_flag&LOAD_EDGEW) || _wfname) && !RANGE (_w_lb, w, _w_ub) ) return (0);

  if ( _flag & LOAD_EDGE ){

    if ( _fstar[_out_node_num] ){
      if ( !RANGE (_deg_lb, _fstar[x+1]-_fstar[x], _deg_ub) ) return (0);
      if ( !RANGE (_deg_lb, _fstar[y+1]-_fstar[y], _deg_ub) ) return (0);
    }
    else {
      if ( !RANGE (_deg_lb, _fstar[x], _deg_ub) ) return (0);
      if ( !RANGE (_deg_lb, _fstar[y], _deg_ub) ) return (0);
    }
    return (1);
  }

  if ( _out_deg && (_flag&FSTAR_OUT_CHK)){
    if ( !RANGE (_out_lb, _out_deg[y], _out_ub) ) return (0);
    if ( (_flag&LOAD_BIPARTITE)==0
          && !RANGE(_out_lb, _out_deg[x], _out_ub) ) return (0);
  }
  if ( _in_deg && _out_deg && (_flag&LOAD_BIPARTITE)==0 && (_flag&FSTAR_DEG_CHK)){

    if ( !RANGE (_deg_lb, _in_deg[x]+_out_deg[x], _deg_ub) ) return (0);
    if ( !RANGE (_deg_lb, _in_deg[y]+_out_deg[y], _deg_ub) ) return (0);
  }
  
  if ( ((_flag & FSTAR_CNT_DEG_ONLY) || _edge_dir==0) && _in_deg && (_flag&FSTAR_DEG_CHK)){

    if ( !RANGE (_deg_lb, _in_deg[x], _deg_ub) ) return (0);
    if ( !RANGE (_deg_lb, _in_deg[y], _deg_ub) ) return (0);
  } else if ( _in_deg && (_flag&FSTAR_IN_CHK) && !(_flag&FSTAR_CNT_DEG_ONLY) ){
    if ( (_flag&LOAD_BIPARTITE)==0 ){
      if ( !RANGE (_in_lb, _in_deg[y], _in_ub) ) return (0);
    } else { if ( !RANGE (_in_lb, _in_deg[x], _in_ub) ) return (0); }
  }

  return (1);
}

/* scan the file and count the degree, for edge listed file */
/* if F->out_node_num is set ot a number larger than #nodes, set the node number to it, so that isolated vertices will be attached to the last */
void FSTAR::_scan_file(FILE2 *fp){

  LONG i, j;

  FILE_COUNT C ;

  // count #pairs
  C.countFS (
  	fp,
  	(_flag&(LOAD_ELE+LOAD_TPOSE+LOAD_GRAPHNUM+LOAD_EDGE)) | 
  	FILE_COUNT_ROWT | ((_flag & FSTAR_CNT_IN)? FILE_COUNT_CLMT: 0),
  	(_flag&LOAD_EDGEW)?1:0 
  );

  _xmax = C.get_rows(); 
  _ymax = C.get_clms(); 
  _edge_num_org = C.get_eles();

  j = alloc_deg ();

  for(i=0;i<j;i++){
    _fstar[i] = C.get_rowt(i);
    if ( _out_deg ) _out_deg[i] = C.get_rowt(i);
  }
  for(i=0;i<C.get_clms();i++){
  	if ( _in_deg ) _in_deg[i] = C.get_clmt(i);
  }
}

/* load data from file to memory with allocation, (after the scan for counting) */
void FSTAR::read_file(FILE2 *fp, FILE2 *wfp){

  LONG i, x, y;
  int fc=0, FILE_err_=0;
  int flag = (_flag & (FSTAR_DEG_CHK+FSTAR_IN_CHK+FSTAR_OUT_CHK)), phase;
  //double w;
  WEIGHT w;
  if ( wfp ) wfp->reset ();

  for (phase=flag?1:2 ; phase < 3 ; phase++){
    i=0;
    fp->reset ();
    if ( _flag&(LOAD_GRAPHNUM) ) fp->read_until_newline();
    if ( phase == 2 ) calc_fstar ();
    do {
			//////////////
      if ( _flag&LOAD_ELE ){
        if ( fp->read_pair ( &x, &y, &w, _flag) ) continue;
      } else {
        x = i+((_flag&LOAD_ID1)?1:0);  // modified 5/2018 for v command in grhfil
        FILE_err_ = (FSTAR_INT)fp->read_item ( wfp, &x, &y, &w, fc, _flag);
				if (fp->Null()) goto LOOP_END; //FILE_err&4 
      }
			/////////////

      if ( !flag || eval_edge (x, y, w) ){
        if ( phase == 2 ){
          if ( _fstar[x+1]>_fstar[x] && (!(_flag & LOAD_EDGE) || _fstar[y+1]>_fstar[y])){
            if ( _edge_w ) _edge_w[_fstar[x]] = w;
            _edge[_fstar[x]++] = y;
            if ( (_flag & LOAD_EDGE) && x != y ){  // undirected && not selfloop
              if ( _edge_w ) _edge_w[_fstar[y]] = w;
              _edge[_fstar[y]++] = x;
            }
          }
        } else if (_flag & LOAD_EDGE){
        		// 最初からコメントアウトされてた
						//          F->fstar[x]--;
						//          if ( (F->flag & LOAD_EDGE) && x != y ) F->fstar[y]--;   // undirected && not selfloop
        }
      }

			//////////////
      if ( !(_flag&LOAD_ELE) ){
        fc = 0;
        //if ( FILE_err&3 ){
        if ( fp->EolOrEof() ){
          LOOP_END:;
          i++;
          fc = FILE_err_? 0: 1; FILE_err_=0; // even if next weight is not written, it is the rest of the previous line
        }
      }
			/////////////////////
    } 
    while ( fp->NotEof() );// !(FILE_err&2)
  }
  _fstar[_out_node_num+1] = 0;

  //BLOOP (i, _out_node_num, 0) {
  for(i=_out_node_num;(i--)>0;){
  	_fstar[i+1] = _fstar[i];
  }
  _fstar[0] = 0;
}

/* remove edges adjacent to small/large degree vertices */
void FSTAR::extract_subgraph()
{

  FSTAR_INT x, y, ii, i, nodes=0;
  int flag = 0;
  WEIGHT w=0;

  do { // count #pairs
    nodes = 0;
    if ( _in_deg ||_out_deg ){

      // re-count in/out degrees
      if ( _in_deg ) {
				for(size_t i =0 ;i< _in_node_num ;i++){ _in_deg[i] = 0; }
      }
      if ( _out_deg ){
				for(size_t i =0 ;i< _out_node_num ;i++){ _out_deg[i]= 0; }
      }
      x=0; 

    	for(i=0;i<_edge_num;i++){
        while ( i == _fstar[x+1] ){ x++; }
        if ( _edge[i] < _node_num ) inc_deg (x, _edge[i]);
      }
    }
    
      // re-remove out-bounded degree vertices
    ii=x=0; 

    for(i=0;i<_edge_num;i++){

      while ( i == _fstar[x+1] ){ 
      	_fstar[x+1] = ii; 
      	x++; 
      	flag = 0; 
      }

      y = _edge[i];

      if ( _edge_w ) w = _edge_w[i];

      if ( eval_edge ( x, y, w) ){

        if ( _edge_w ) _edge_w[ii] = _edge_w[i];
        _edge[ii++] = y;
        if ( flag==0 ){ nodes++; flag = 1; }
      }

    }

    while (x < _out_node_num) _fstar[++x] = ii;

    _edge_num = ii;
    printMes("iterative scan: #nodes="FSTAR_INTF", #edges = "FSTAR_INTF"\n", nodes, i);

  } while ( ii < i );
  
}

/* 
	load graph from file 
	kggrhfil
*/
int FSTAR::load(){

  FILE2 fp , wfp;

  if ( _in_lb >0 || _in_ub <FSTAR_INTHUGE ){
  	 _flag |= (FSTAR_IN_CHK+FSTAR_CNT_IN);
  }

  if ( _out_lb >0 || _out_ub <FSTAR_INTHUGE ){
  	_flag |= (FSTAR_OUT_CHK+FSTAR_CNT_OUT);
  }

  if ( _deg_lb >0 || _deg_ub <FSTAR_INTHUGE ){

    _flag |= FSTAR_DEG_CHK+FSTAR_CNT_IN;
    
    if ( (_flag&(FSTAR_OUT_CHK+FSTAR_IN_CHK)) ==0 ) {
    	_flag |= FSTAR_CNT_DEG_ONLY;
    }
    else{
    	_flag |= FSTAR_CNT_OUT;
    }
  }

  fp.open ( _fname, "r");
  if ( _wfname ) wfp.open ( _wfname, "r");

  _scan_file(&fp);
  printMes("first & second scan end: %s\n", _fname);

  read_file( &fp, _wfname? &wfp: NULL);

  fp.close ();
  if ( _wfname ) wfp.close ();

  if (_ERROR_MES) EXIT;

  printMes("file read end: %s\n", _fname);

  extract_subgraph();

  if ( (_flag&LOAD_INCSORT)|| (_flag&LOAD_DECSORT) ) {
     sort_adjacent_node ((_flag&LOAD_DECSORT)?-1:1);
  }

	printMes("forwardstar graph: %s ,#nodes "FSTAR_INTF"("FSTAR_INTF","FSTAR_INTF") ,#edges "FSTAR_INTF"\n", _fname, _node_num, _in_node_num, _out_node_num, _edge_num);

	return 0;
}



/* load data from file to memory with allocation, (after the scan for counting) */
void FSTAR::read_fileMED(FILE2 *fp ){

  LONG i=0; 
  LONG x, y;

  fp->reset ();

  calc_fstar ();

	do{
		
		x = i;

		fp->read_item ( &x, &y, _flag);

		if (fp->Null()) goto LOOP_END; //( FILE_err&4 )

    if ( _fstar[x+1]>_fstar[x] ){  _edge[_fstar[x]++] = y; }

		if ( fp->EolOrEof() ){ //( FILE_err&3 )
			LOOP_END:;
			i++;
    }

	} 
	while ( fp->NotEof() );

  _fstar[_out_node_num+1] = 0;

  for(i=_out_node_num;(i--)>0;){   
  	_fstar[i+1] = _fstar[i];
  }
  _fstar[0] = 0;
  
}

/* load graph from file */
// load fstar for medset
// int fsFlag (LOAD_TPOSE,LOAD_BIPARTITE), char *fname ,int edge_dir
// FILE_COUNT_ROWT |  ( LOAD_TPOSE )
int FSTAR::loadMed(){

  LONG i, j;
  FILE_COUNT C ;

	FILE2 fp(_fname);
  C.countFST(&fp, _flag&LOAD_TPOSE);

  _xmax = C.get_rows();
  _ymax = C.get_clms(); 

  _edge_num_org = C.get_eles();

  //j = alloc_deg ();
  {

	  _out_node_num = _xmax;
  	_in_node_num  = _ymax;

	  //j = _out_node_num; 
  	
  	_node_num = MAX (_out_node_num, _in_node_num);

	  _fstar = new FSTAR_INT[_out_node_num+2](); // calloc2
	
	}

  for(i=0 ; i<_out_node_num; i++){ _fstar[i] = C.get_rowt(i); }
  
	/*
  for(i=0;i<j;i++){
    _fstar[i] = C.get_rowt(i);
  }
  */

  printMes("first & second scan end: %s\n", _fname);

  read_fileMED(&fp);

  printMes("file read end: %s\n", _fname);

  extract_subgraph();

	printMes("forwardstar graph: %s ,#nodes "FSTAR_INTF"("FSTAR_INTF","FSTAR_INTF") ,#edges "FSTAR_INTF"\n", _fname, _node_num, _in_node_num, _out_node_num, _edge_num);

	return 0;
}

void FSTAR::writeHeadInfo(OFILE2 &fp){

	if ( _flag&(LOAD_GRAPHNUM) ){
		fp.print(FSTAR_INTF"%c", _node_num, _sep);
		fp.print(FSTAR_INTF"\n", _edge_num/((_flag&LOAD_EDGE)?2:1));
  }

}

/* write row ID on the top of the line */
void FSTAR::write_graph_ID (OFILE2 &fp,OFILE2 &fp2, FSTAR_INT ID){

  // for the case "first element is row ID
  if ( _flag & FSTAR_INS_ROWID){

    if ( _flag & LOAD_ELE ){
      fp.print( FSTAR_INTF"%c"FSTAR_INTF, ID, _sep, ID);
      if ( (_flag & LOAD_EDGE) && (_flag & FSTAR_INS_ROWID_WEIGHT) ){ 
	      fp.print("%c1", _sep);
      }
      fp.print("\n");
    }
    else{
      fp.print( FSTAR_INTF"%c", ID, _sep);
      if ( fp2.exist() && (_flag & FSTAR_INS_ROWID_WEIGHT) ){
				fp2.print("1%c", _sep);
      }
      if ( (_flag & LOAD_EDGEW) && (_flag & FSTAR_INS_ROWID_WEIGHT)){ 
      	fp.print("1%c", _sep); 
      }
    }
  }
}

/* write an edge */
int FSTAR::write_graph_item(
	FSTAR_INT x, FSTAR_INT y, WEIGHT w, 
	OFILE2 &fp, OFILE2 &fp2, int *row, 
	FSTAR_INT *prv
){

  if ( !(_flag&(LOAD_BIPARTITE+LOAD_EDGE)) && _edge_dir==0 && x>y ) return 1;

     // ID permutation 
  if ( _table ){
    x = _table[x];
    if ( !(_flag & LOAD_BIPARTITE) ){
      if ( (y=_table[y]) == _out_node_num ) return 1;
    }
  }
  if ( _flag & LOAD_ID1 ) y++;

       // write an element
  if ( !(_flag&LOAD_RM_DUP) || *row == 0 || *prv != y ){
    if ( _flag & LOAD_ELE ){

      fp.print(FSTAR_INTF"%c"FSTAR_INTF, x, _sep, y);
      if (_flag & LOAD_EDGEW){ 
      	fp.putch(_sep); 
      	fp.print(w); 
      }
      fp.print("\n");

    } else {

      fp.print( FSTAR_INTF"%c", y, _sep);
      if ( fp2.exist() ){ 
      	fp2.print(w); 
				fp2.putch(_sep);  
      }
      if ( _flag&LOAD_EDGEW ){ 
				fp.print(w);
				fp.putch(_sep);
      }
    }
    *row = 1;
  }

  // remember the previous element
  *prv = y;  

  return 0;
}


/* write induced graph and the number convert table; needs accumerated node_deg, edge */
LONG FSTAR::write_graph (char *fname, char *fname2){

  FSTAR_INT i=0, x=0, y, flag = 0, c=0, prv=0;
  int row=0, IDf=0;
  LONG cnt=0;
  
  OFILE2 fp(fname);
  OFILE2 fp2(fname2);

	writeHeadInfo(fp);

  while (i <= _edge_num+1){

    while ( i == _fstar[x+1]){

      if ( !_table || flag ){
        if ( IDf == 0 ) write_graph_ID ( fp, fp2, c);
        flag = 0; c++; row=0; IDf = 0;
        if ( !(_flag & LOAD_ELE) ){
          fp.print("\n");
          if ( fp2.exist() ) { fp2.print("\n"); }
        }
      }
      if ( ++x >= _out_node_num ) return cnt;
    }
    y = _edge[i];

    if ( IDf == 0 ){
      if ( _out_deg && (_out_node_num ==0 || x < _out_node_num) ) flag += _out_deg[x];
      if ( _in_deg && (_in_node_num ==0 || x < _in_node_num) ) flag += _in_deg[x];
      if ( (!_out_deg && !_in_deg) && (_flag&LOAD_EDGE) ) flag = _fstar[x+1] - _fstar[x];
      if ( !_table || flag ) write_graph_ID ( fp, fp2, c);
      IDf = 1;
    }

    write_graph_item ( x, y, _edge_w? _edge_w[i]: 0, fp, fp2, &row, &prv);
    cnt++; i++;
  }
  if ( !(_flag & LOAD_ELE) ){
    fp.print("\n");
    if ( fp2.exist() ) fp2.print("\n");
  }
  return cnt;

}

/*
	 write induced graph and the number convert table;
	needs accumerated node_deg, edge */
LONG FSTAR::write_graph_operation (
	FSTAR *F1, FSTAR *F2, 
	char *fname, char *fname2,  //	<= output weight
	int op, double th
){


	OFILE2 fp(fname);
	OFILE2 fp2(fname2);
	
	FSTAR_INT y, y1, y2;
  FSTAR_INT i=0, j=0, x=0, prv=0;
  
	FSTAR_INT ee = FSTAR::maxNodeNum(F1 ,F2);

  int row2=0;
  LONG cnt=0;
  WEIGHT w=0, w1, w2;

	bool end1,end2;

	F1->writeHeadInfo(fp);
 
  F1->write_graph_ID(fp, fp2, 0);

	end1 = end2 = false;

  while (i <= F1->_edge_num+1 && j <= F2->_edge_num+1){

    while ( (end1 || i == F1->_fstar[x+1]) && (end2 || j == F2->_fstar[x+1])){
      row2 = 0;
      if ( !(F1->_flag & LOAD_ELE) ){
        fp.print("\n");
        if ( fp2.exist() ) fp2.print("\n");
      }
      x++;
      if ( x >= F1->_out_node_num ) end1 = true;
      if ( x >= F2->_out_node_num ) end2 = true;
      if ( end1 && end2 ) {
      	return cnt;
      }
      F1->write_graph_ID ( fp, fp2, x);
    }

    y1 = (!end1 && i<F1->_edge_num+1 && i < F1->_fstar[x+1])? F1->_edge[i]: ee;
    y2 = (!end2 && j<F2->_edge_num+1 && j < F2->_fstar[x+1])? F2->_edge[j]: ee;

    if ( y1 < y2 ){
      if ( op == 1 ){ i++; continue; } // intersection
      y = y1;
			w = F1->eWeight(i);
      i++;
    }
    else if ( y1 > y2){
      if ( op == 1 ){ j++; continue; } // intersection
      y = y2;
      w = F2->eWeight(j);
      j++;
    }
    else {
      if ( op == 3 ){ i++; j++; continue; } // symmetric difference
      y = y1;
      w1 = F1->eWeight(i);
      w2 = F2->eWeight(j);
      i++; j++;
      if ( op == 1 ) w = MIN(w1, w2); // weighted intersection
      if ( op == 2 ) w = MAX(w1, w2); // weighted union
      if ( op == 4 ) w = w1 - w2;     // difference
    }
    if ( op == 4 && (w=abs(w)) < th ) continue;  // difference
    
    F1->write_graph_item ( x, y, w, fp, fp2, &row2, &prv);
    cnt++;

  }

  if ( !(F1->_flag & LOAD_ELE) ){
    fp.print("\n");
    if ( fp2.exist() ) fp2.print("\n");
  }

  return cnt;
}


/* make vertex permutation table and write to table-file */
void FSTAR::write_table_file (char *fname){

	FSTAR_INT i, flag;
  OFILE2 fp(fname);
  
  //_table = calloc2(_table, _node_num);
  _table = new FSTAR_INT[ _node_num]();

  _reduced_node_num = 0;

  for(i=0;i<_out_node_num;i++){

    flag = 0;
    if ( _out_deg ) flag += _out_deg[i];
    if ( _in_deg && (_in_node_num==0 || i < _in_node_num) ) flag += _in_deg[i];
    if ( flag ){
      fp.print(FSTAR_INTF"\n", (_flag&LOAD_ID1)?i+1:i );
      _table[i] = _reduced_node_num;
      _reduced_node_num++;
    }
    else{
    	 _table[i] = _out_node_num;
    }
  }

}



