/* GRHFIL: convert graph format */
/* 2004 Takeaki Uno */
/* matdraw */

// #define FSTAR_INT unsigned int
// internal_params.l1 :  #edges written to the output file

#define GRHFIL_INS_ROWID 1024
#define GRHFIL_NORMALIZE 65536
#define GRHFIL_DISCRETIZE 131072


#include <math.h>
#include "kggrhfil.hpp"
#define WEIGHT_DOUBLE


/* error routine */

void KGGRHFIL::help(void){

  print_err ("graph filtering: transform/convert/extract graph/subgraph\n\
grhfil dDUBeEq [options] input-file output-file\n\
%%:show progress, _:no message, +:write solutions in append mode\n\
d:directed graph (x->y), D:directed graph with reverse direction (x<-y)\n\
U,u:undirected graph (u:edge for both direction), B:bipartite graph, e,E:read/write file as edge list\n\
s,S:sort vertex adjacent list in increasing/decreasing order\n\
n,N:read/write the number of vertices and edges written in/at 1st line of the file\n\
v,V:node ID in read/write file starts from 1, q:non-transform mode (valid with -P option)\n\
0:insert vertex ID as the first entry, for each vertex (adjacency list mode)\n\
9:give weight 1 to each vertex ID (with 0)\n\
Z:normalize the length to 1 (in weighted case)\n\
w,W:read/write edge weights in the graph file\n\
1:unify consecutive two same numbers into one\n\
[options]\n\
-t,T [num]: remove vertices with degree smaller/larger then [num]\n\
-i,I [num]: remove vertices with in-degree smaller/larger then [num]\n\
-o,O [num]: remove vertices with out-degree smaller/larger then [num]\n\
-r,R [num]: remove edges with weights smaller/larger then [num]\n\
-n [num]: specify #nodes\n\
-X [num1] [num2]: multiply each weight by [num1] and trancate by [num2]\n\
-x [num]: power each weight by [num] and normalize to unit length\n\
-1 [num]: values below [num] will be 0 and will be 1 otherwise\n\
-w,W [filename]: weight file to read/write\n\
-d [filename]: take difference with graph of [filename] (2nd -d: specify the threshold value)\n\
-m,M [filename]: take intersection/union with graph of [filename]\n\
-p [filename]: permute the vertex ID to coutinuous numbering and output the permutation table to the file\n\
-, [char]:give the separator of the numbers in the output\n\
-Q [filename]: permute the numbers in the file according to the table \n");
  EXIT;
}

int KGGRHFIL::setArgs_iter (char *a, int *ff){
  int f=0;
  *ff = 2;
  if ( strchr(a, 'u') ){ f |= LOAD_EDGE; *ff = 0; }
  if ( strchr(a, 'U') ) *ff = 0;
  if ( strchr(a, 'B') ){ f |= LOAD_BIPARTITE; *ff = 1; }
  if ( strchr(a, 'D') ){ *ff = -1; f |= LOAD_TPOSE; }
  if ( strchr(a, 'd') ) *ff = 1;
  if ( strchr(a, 'e') ) f |= LOAD_ELE;
  if ( strchr(a, 's') ) f |= LOAD_INCSORT;
  if ( strchr(a, 'S') ) f |= LOAD_DECSORT;
  if ( strchr(a, 'n') ) f |= LOAD_GRAPHNUM;
  if ( strchr(a, 'w') ) f |= LOAD_EDGEW;
  if ( strchr(a, 'v') ) f |= LOAD_ID1;
  return (f);
}

/* read filenames from command line parameters */
int KGGRHFIL::setArgs(int argc, char *argv[]){

  int c=1;

  if ( argc<c+3 ){ help(); return 1; }

	_edge_dir = 2;//_FS.set_edge_dir(2);  

  if ( !strchr (argv[c], '_') ){ _fsFlag |= SHOW_MESSAGE;   }
	//これ使いたい場合は別途考える
//  if ( strchr (argv[c], '+') ) { _iFlag  |= ITEMSET_APPEND; }

	_fsFlag  |= setArgs_iter(argv[c], &_edge_dir);
  // _FS.union_flag( read_param_iter (argv[c], _FS.getp_edge_dir()) );

  if ( strchr(argv[c], 'u') ){ _problem |= LOAD_EDGE; }
  if ( strchr(argv[c], 'E') ){ _problem |= LOAD_ELE; }
  if ( strchr(argv[c], 'N') ){ _problem |= LOAD_GRAPHNUM; }
  if ( strchr(argv[c], 'W') ){ _problem |= LOAD_EDGEW; }
  if ( strchr(argv[c], 'V') ){ _problem |= LOAD_ID1; }
  if ( strchr(argv[c], 'q') ){ _dir = 1; _edge_dir=0; }
  if ( strchr(argv[c], '0') ){ _problem  |= FSTAR_INS_ROWID; }
  if ( strchr(argv[c], '9') ){ _problem  |= FSTAR_INS_ROWID_WEIGHT; }
  if ( strchr(argv[c], 'Z') ){ _problem2 |= GRHFIL_NORMALIZE; }
  if ( strchr(argv[c], '1') ){ _problem  |= LOAD_RM_DUP ; }
  if ( _edge_dir == 2 ){
		fprintf(stderr,"one of B, D, d, u or U has to be given\n");
  	return 1; 
  }

  c++;

  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){
      case 't': 
      	_deg_lb = atoi(argv[c+1]);
      break; case 'T': 
      	_deg_ub = atoi(argv[c+1]);

      break; case 'i':
      	_in_lb = atoi(argv[c+1]);

      break; case 'I': 
      	_in_ub = atoi(argv[c+1]);

      break; case 'o': 
      	_out_lb = atoi(argv[c+1]);

      break; case 'O': 
      	_out_ub = atoi(argv[c+1]);

      break; case 'r': 
      	_w_lb = atof(argv[c+1]);
      	
      break; case 'R': 
      	_w_ub = atof(argv[c+1]);
      
      break; case 'X': 
      	_ratio = atof(argv[c+1]); 
      	_th = atof(argv[c+2]); 
      	c++;
      	
      break; case 'x': 
      	_ratio = atof(argv[c+1]); 
      	_th = DOUBLEHUGE;

      break; case '1': 
      	_th2 = atof(argv[c+1]);  
      	_problem2 |= GRHFIL_DISCRETIZE;

      break; case 'w': 
      	_wfname = argv[c+1];

      break; case 'W': 
      	_weight_fname = argv[c+1];
      	
      break; case 'p': 
      	_table_fname = argv[c+1];
      	
      break; case ',': 
      	_sep = argv[c+1][0] ;

      break; case 'Q': 
      	_table_fname = argv[c+1]; 
      	_dir =1;


      break; case 'd': 
      	if ( _fname2 ){
          _th2 = atof(argv[c+1]);
          _root = 4;
     		} 
     		else {
	     		_fname2 = argv[c+1];
          _root = 3; 
          _th2 = 1;
          _fsFlag2 |= setArgs_iter (&argv[c][2], &_edge_dir2);
          if ( _edge_dir2 == 2 ) _edge_dir2 = _edge_dir;
        }

      break; case 'm':
	      _fname2 = argv[c+1];
	      _root = 1;
        _fsFlag2 |= setArgs_iter (&argv[c][2], &_edge_dir2);
        // _edge_dir2 チェックしなくていい？

      break; case 'M':
	      _fname2 = argv[c+1];
      	_root = 2;
      	_fsFlag2 |= setArgs_iter (&argv[c][2], &_edge_dir2);
        // _edge_dir2 チェックしなくていい？

      break; case 'n': 
      	_rows = atoi(argv[c+1]); 

      break; default: goto NEXT;
    }
    c += 2;
    if ( argc<c+2 ){ help(); return 1; }
  }
  
  NEXT:;
  _fname = argv[c];
  _output_fname = argv[c+1];

	_FS.printMes("input-file %s, output-file %s\n", _fname, _output_fname);
	_FS.printMes("degree threshold: ");

  if ( _deg_lb>0 ) _FS.printMes(FSTAR_INTF" <", _deg_lb);
  if ( _deg_lb>0 || _deg_ub < FSTAR_INTHUGE) _FS.printMes(" degree ");
  if ( _deg_ub < FSTAR_INTHUGE ) _FS.printMes("< "FSTAR_INTF"  ", _deg_ub);

  if ( _in_lb > 0 ) _FS.printMes( FSTAR_INTF" <", _in_lb);
  if ( _in_lb > 0 || _in_ub <FSTAR_INTHUGE) _FS.printMes( " in-degree ");
  if ( _in_ub < FSTAR_INTHUGE ) _FS.printMes("< "FSTAR_INTF"  ", _in_ub);
  
  if ( _out_lb > 0 ) _FS.printMes( FSTAR_INTF" <", _out_lb);
  if ( _out_lb > 0 || _out_ub < FSTAR_INTHUGE)_FS.printMes(" out-degree ");
  if ( _out_ub < FSTAR_INTHUGE ) _FS.printMes( "< "FSTAR_INTF"  ", _out_ub);

  _FS.printMes("\n");
  
  if ( _table_fname )_FS.printMes("permutation-table-file %s\n", _table_fname);



/*
  print_mesf (&_FS, "input-file %s, output-file %s\n", _fname, _output_fname);
  print_mesf (&_FS, "degree threshold: ");

  if ( _deg_lb>0 ) print_mesf (&_FS, FSTAR_INTF" <", _deg_lb);
  if ( _deg_lb>0 || _deg_ub < FSTAR_INTHUGE) print_mesf (&_FS, " degree ");
  if ( _deg_ub < FSTAR_INTHUGE ) print_mesf (&_FS, "< "FSTAR_INTF"  ", _deg_ub);

  if ( _in_lb > 0 ) print_mesf (&_FS, FSTAR_INTF" <", _in_lb);
  if ( _in_lb > 0 || _in_ub <FSTAR_INTHUGE) print_mesf (&_FS, " in-degree ");
  if ( _in_ub < FSTAR_INTHUGE ) print_mesf (&_FS, "< "FSTAR_INTF"  ", _in_ub);
  
  if ( _out_lb > 0 ) print_mesf (&_FS, FSTAR_INTF" <", _out_lb);
  if ( _out_lb > 0 || _out_ub < FSTAR_INTHUGE) print_mesf (&_FS, " out-degree ");
  if ( _out_ub < FSTAR_INTHUGE ) print_mesf (&_FS, "< "FSTAR_INTF"  ", _out_ub);

  print_mesf (&_FS, "\n");
  
  if ( _table_fname ) print_mesf (&_FS, "permutation-table-file %s\n", _table_fname);
*/
	if ( _root > 0 ){ 
	  _fsFlag |= LOAD_INCSORT ;
	  _fsFlag2|= LOAD_INCSORT ;
	}
	return 0;
}

// non-transform mode
// _fname , _output_fname, _table_fname
int KGGRHFIL::replaceDATA()
{
	char i;
	LONG l,x;
	FILE2 ifp, ofp;
	WEIGHT w;
	FSTAR_INT *table=NULL;
	
	if(_table_fname){
		FILE2::ARY_Load( table,_table_fname,1);
	}
	
	ifp.open( _fname , "r");
  ofp.open(_output_fname, "w");
		
	do{
		i=0; x=0;
		do {
			l = ifp.read_int ();

			//if ( (FILE_err&4)==0 ){
			
			if ( ifp.readOK() ){

				ofp.print_int( table ? table[l]: l , i);
				i = _sep;

				if ( (_fsFlag&LOAD_EDGEW) && (((_fsFlag&LOAD_ELE)&&x==1) || !(_fsFlag&LOAD_ELE)) ){
					w = ifp.read_double();
					ofp.print_int ( w, i);
				}
				ofp.flush ();
			}

			x++;

		//} while ( (FILE_err&3)==0 );
		} while ( ifp.remain());

		ofp.puts ( "\n");

	//} while ( (FILE_err&2)==0 );
	} while ( ifp.eof());

	ifp.close ();
	ofp.closew ();
			
	return 0;
			
}


/* main routine */
int KGGRHFIL::run (int argc ,char* argv[]){

  FILE2 fp, fp2;
  LONG l, ll, x, xx;
  char i;
  WEIGHT w;


	if ( setArgs(argc, argv) ){ return 1; }

	//ARY_LOAD (_FS.get_table(), int, l, _table_fname, 1, EXIT);
  // no transformation (just replace the numbers and separators)
  //これ別のほうがいい？
  if ( _dir ) {
		replaceDATA();
  }
  else{

	  _FS.setParams(
  		_fsFlag,_fname,_edge_dir,_wfname,
			_deg_lb,_deg_ub,_in_lb,_in_ub,
			_out_lb,_out_ub,_w_lb,_w_ub,
			_sep,_rows
		);

	  _FS2.setParams(_fsFlag2,_fname2,_edge_dir2);

		if( _FS.load() ) return 1;
		if( _fname2 ){ if( _FS2.load() ) return 1; }
		_FS.adjust_edgeW(_ratio,_th,_th2,_problem2 & GRHFIL_NORMALIZE,_problem2 & GRHFIL_DISCRETIZE);	
		_FS.set_flag(_problem); // +(FS->flag&LOAD_EDGE); //なぜここで

		if ( _table_fname ) _FS.write_table_file (_table_fname); 

	  if ( _root ){
			_ip_l1 = FSTAR::write_graph_operation (
  		 	&_FS, &_FS2, 
	  	 	_output_fname, _weight_fname,
  		 	_root, _th2
  	 	);
	  }
  	else{
  		 _ip_l1 = _FS.write_graph ( _output_fname, _weight_fname );
	  }
	}

  return 0;
}

int KGGRHFIL::mrun(int argc ,char* argv[]){
	return KGGRHFIL().run(argc,argv);
}

