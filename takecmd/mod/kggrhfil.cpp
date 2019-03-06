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

int KGGRHFIL::read_param_iter (char *a, int *ff){
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
void KGGRHFIL::read_param (int argc, char *argv[]){

  int c=1;

  if ( argc<c+3 ){ help(); return; }

  _FS.set_edge_dir(2);
  
  if ( !strchr (argv[c], '_') ){ _FS.union_flag(SHOW_MESSAGE);   }
  if ( strchr (argv[c], '+') ) { _II.union_flag(ITEMSET_APPEND); }

  _FS.union_flag( read_param_iter (argv[c], _FS.getp_edge_dir()) );

  if ( strchr(argv[c], 'u') ){ _problem |= LOAD_EDGE; }
  if ( strchr(argv[c], 'E') ){ _problem |= LOAD_ELE; }
  if ( strchr(argv[c], 'N') ){ _problem |= LOAD_GRAPHNUM; }
  if ( strchr(argv[c], 'W') ){ _problem |= LOAD_EDGEW; }
  if ( strchr(argv[c], 'V') ){ _problem |= LOAD_ID1; }
  if ( strchr(argv[c], 'q') ){ _dir = 1; _FS.set_edge_dir(0); }
  if ( strchr(argv[c], '0') ){ _problem  |= FSTAR_INS_ROWID; }
  if ( strchr(argv[c], '9') ){ _problem  |= FSTAR_INS_ROWID_WEIGHT; }
  if ( strchr(argv[c], 'Z') ){ _problem2 |= GRHFIL_NORMALIZE; }
  if ( strchr(argv[c], '1') ){ _problem  |= LOAD_RM_DUP; }
  if ( _FS.get_edge_dir() == 2 )  { error ("one of B, D, d, u or U has to be given", EXIT);}

  c++;
  while ( argv[c][0] == '-' ){
    switch (argv[c][1]){
      case 't': _FS.set_deg_lb (atoi(argv[c+1]));
      break; case 'T': _FS.set_deg_ub ( atoi(argv[c+1]) );
      break; case 'i': _FS.set_in_lb ( atoi(argv[c+1]) );
      break; case 'I': _FS.set_in_ub ( atoi(argv[c+1]) );
      break; case 'o': _FS.set_out_lb ( atoi(argv[c+1]) );
      break; case 'O': _FS.set_out_ub ( atoi(argv[c+1]) );
      break; case 'r': _FS.set_w_lb ( atof(argv[c+1]) );
      break; case 'R': _FS.set_w_ub ( atof(argv[c+1]) );
      break; case 'X': _ratio = atof(argv[c+1]); _th = atof(argv[c+2]); c++;
      break; case 'x': _ratio = atof(argv[c+1]); _th = DOUBLEHUGE;
      break; case '1': _th2 = atof(argv[c+1]);  _problem2 |= GRHFIL_DISCRETIZE;
      break; case 'w': _FS.set_wfname ( argv[c+1] );
      break; case 'W': _weight_fname = argv[c+1];
      break; case 'p': _table_fname = argv[c+1];
      break; case ',': _FS.set_sep ( argv[c+1][0] );
      break; case 'Q': _table_fname = argv[c+1]; _dir =1;
      break; case 'd': 
      	if ( _FS2.get_fname() ){
          _th2 = atof(argv[c+1]);
          _root = 4;
     		} else {
          _FS2.set_fname ( argv[c+1]);
          _root = 3; _th2 = 1;
          _FS2.union_flag ( read_param_iter (&argv[c][2], _FS2.getp_edge_dir()) );
          if ( _FS2.get_edge_dir() == 2 ) _FS2.set_edge_dir ( _FS.get_edge_dir()); 
        }
      break; case 'm':
      	_FS2.set_fname ( argv[c+1] ); _root = 1;
        _FS2.union_flag ( read_param_iter (&argv[c][2], _FS2.getp_edge_dir()) );
      break; case 'M':
      	_FS2.set_fname ( argv[c+1] ); _root = 2;
        _FS2.union_flag ( read_param_iter (&argv[c][2], _FS2.getp_edge_dir()) );
      break; case 'n': _rows = atoi(argv[c+1]); 
      break; default: goto NEXT;
    }
    c += 2;
    if ( argc<c+2 ){ help(); return; }
  }
  
  NEXT:;
  _FS.set_fname ( argv[c] );
  _output_fname = argv[c+1];
  print_mesf (&_FS, "input-file %s, output-file %s\n", _FS.get_fname(), _output_fname);
  print_mesf (&_FS, "degree threshold: ");
  if ( _FS.get_deg_lb()>0 ) print_mesf (&_FS, FSTAR_INTF" <", _FS.get_deg_lb());
  if ( _FS.get_deg_lb()>0 || _FS.get_deg_ub()<FSTAR_INTHUGE) print_mesf (&_FS, " degree ");
  if ( _FS.get_deg_ub()<FSTAR_INTHUGE ) print_mesf (&_FS, "< "FSTAR_INTF"  ", _FS.get_deg_ub());

  if ( _FS.get_in_lb()>0 ) print_mesf (&_FS, FSTAR_INTF" <", _FS.get_in_lb());
  if ( _FS.get_in_lb()>0 || _FS.get_in_ub()<FSTAR_INTHUGE) print_mesf (&_FS, " in-degree ");
  if ( _FS.get_in_ub()<FSTAR_INTHUGE ) print_mesf (&_FS, "< "FSTAR_INTF"  ", _FS.get_in_ub());
  
  if ( _FS.get_out_lb()>0 ) print_mesf (&_FS, FSTAR_INTF" <", _FS.get_out_lb());
  if ( _FS.get_out_lb()>0 || _FS.get_out_ub()<FSTAR_INTHUGE) print_mesf (&_FS, " out-degree ");
  if ( _FS.get_out_ub()<FSTAR_INTHUGE ) print_mesf (&_FS, "< "FSTAR_INTF"  ", _FS.get_out_ub());
  print_mesf (&_FS, "\n");
  
  if ( _table_fname ) print_mesf (&_FS, "permutation-table-file %s\n", _table_fname);
  if ( _root > 0 ){ _FS.union_flag ( LOAD_INCSORT ); _FS2.union_flag ( LOAD_INCSORT); }

}

/* main routine */
int KGGRHFIL::run (int argc ,char* argv[]){

  FILE2 fp, fp2;
  LONG l, ll, x, xx;
  char i;
  WEIGHT w;

	read_param (argc, argv);

	_FS.set_out_node_num(_rows);

	//ARY_LOAD (_FS.get_table(), int, l, _table_fname, 1, EXIT);
  if ( _dir ) l = _FS.array_LOAD( _table_fname);
  
    // no transformation (just replace the numbers and separators)
  if ( _dir ){
    fp.open ( _FS.get_fname()   , "r");
    fp2.open(_output_fname, "w");
    do {
      i=0; x=0;
      do {
        l = fp.read_int ();
        if ( (FILE_err&4)==0 ){
          fp2.print_int ( _FS.get_table()? _FS.get_table(l): l, i);
          i = _II.get_separator();
          if ( (_FS.get_flag()&LOAD_EDGEW) && (((_FS.get_flag()&LOAD_ELE)&&x==1) || !(_FS.get_flag()&LOAD_ELE)) ){
            w = fp.read_double ();
            fp2.print_int ( w, i);
          }
          fp2.flush ();
        }
        x++;
      } while ( (FILE_err&3)==0 );
      fp2.puts ( "\n");
    } while ( (FILE_err&2)==0 );
    fp.close ();
    fp2.closew ();
    return (0); //別にする・simset用？
  }

  //PROBLEM_load (&PP);
  // _PP.load();
	preLOAD();

  if ( _ratio != 0 && _FS.get_edge_w() ){ // multiply & trancate
    FLOOP (l, 0, _FS.get_edge_num()){ 
      if ( _th == DOUBLEHUGE ){
        _FS.edge_w_pow (l, _ratio);
      } 
      else { 
      	_FS.edge_w_mul_min(l , _ratio,_th);
      }
    }
  }
  if ( (_problem2 & GRHFIL_NORMALIZE) && _FS.get_edge_w() ){
    FLOOP (l, 0, _FS.get_out_node_num()){
      w = 0.0;
      FLOOP (x, _FS.get_fstar(l), _FS.get_fstar(l+1)) w += _FS.edge_w_mul(x);
      w = sqrt (w);
      //FLOOP (x, _FS.get_fstar[l], _FS.get_fstar[l+1]) _FS._edge_w[x] /= w;
      FLOOP (x, _FS.get_fstar(l), _FS.get_fstar(l+1)) _FS.edge_w_div(x,w);
    }
  }
  if ( (_problem2 & GRHFIL_DISCRETIZE) && _FS.get_edge_w() ){
    xx = 0;
    FLOOP (l, 0, _FS.get_out_node_num()){
      ll = _FS.get_fstar(l); _FS.set_fstar(l, xx);
      FLOOP (x, ll, _FS.get_fstar(l+1)){
        if ( _FS.get_edge_w(x) >= _th2 ){
          _FS.set_edge( xx, _FS.get_edge(x));
          _FS.set_edge_w( xx , _FS.get_edge_w(x));
          xx++;
        }
      }
    }
    _FS.set_fstar(l, xx);
  }
  
  _FS.set_flag ( _problem); // +(FS->flag&LOAD_EDGE);
  if ( !_dir ) _FS.write_table_file (_table_fname);
  if ( _root ) internal_params.l1 = FSTAR::write_graph_operation (&_FS, &_FS2, _output_fname, _weight_fname, _root, _th2);
  else internal_params.l1 = _FS.write_graph (_output_fname, _weight_fname);
//	_PP.end();
  return (0);
}

int KGGRHFIL::mrun(int argc ,char* argv[]){
	return KGGRHFIL().run(argc,argv);
}

