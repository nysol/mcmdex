/* take the intersection of each cluster */
/* 8/Nov/2008   Takeaki Uno  */

#include"fstar.hpp"
#include"stdlib2.hpp"
#include"kgmedset.hpp"
#include"problem.hpp"


void KGMEDSET::help(){

  print_err ("medset: compute the intersection of each set of sets\n\
usage: medset [-HRTIitl] cluster-filename set-filename threshold output-filename\n\
if threshold is negative, output the items whose frequencies are no more than the threshold\n\
-%%: show progress, -_: no message\n\
-H: do not use histgram, just output the items\n\
-R: do not output singleton clusters\n\
-V: output ratio of appearances of all items\n\
-T: clustering by connected component (read edge type file)\n\
-I: find an independent set, and clustering by using the vertices in it as seeds (read edge type files)\n\
-i: output for each item, ratio of records including the item\n\
-t: transpose the input database, (transaction will be item, and vice varsa)\n\
-l [num]: output clusters of size at least [num]\n\
# the 1st letter of input-filename cannot be '-'.\n");
  EXIT;
}

/* read commands and options from command line */
int KGMEDSET::setArgs (int argc, char *argv[]){

  int c=1;

  if ( argc < c+3 ){ help (); return 1; }
  _dir = 1; 
  _fsFlag |= SHOW_MESSAGE;

  while ( argv[c][0] == '-' ){

    if ( argc<c+3 ){ help(); return 1; }

    switch ( argv[c][1] ){
      case 't': _fsFlag |= LOAD_TPOSE;
      break; case '_': _fsFlag -= SHOW_MESSAGE;   // connected component clustering
      break; case '%': _fsFlag |= SHOW_PROGRESS;   // connected component clustering
      break; case 'T': _problem |= MEDSET_CC;   // connected component clustering
      break; case 'I': _problem |= MEDSET_IND;   // independent set clustering
      break; case 'H': _problem |= MEDSET_NO_HIST;   // do not use histgram
      break; case 'V': _problem |= MEDSET_ALLNUM;   // output appearance ratio for all
      break; case 'l': _num = atoi(argv[c+1]); c++;   // minimum cluster size to be output
      break; case 'i': _problem |= MEDSET_RATIO;   // output included-ratio of items
//      break; case 'c': _deg = atoi(argv[c+1]); c++;   // least degree
   }
    c++;
  }
  
  _input_fname = argv[c];

  if ( !(_problem & (MEDSET_CC+MEDSET_IND))) _set_fname = argv[c+1];

  _th = atof(argv[c+2]);

  if ( _th < 0 ){ _th = -_th; _problem |= MEDSET_BELOW; }  // output less frequency items

  _output_fname = argv[c+3];

	return 0;
}


/* read file, output the histogram of each line */
void KGMEDSET::read_file(){

  FSTAR_INT *cnt, *que, t, s, i, x;

  cnt = calloc2 (cnt, _FS.get_in_node_num());
  que = calloc2 (que, _FS.get_in_node_num()*2);

  do {

    s = t = 0;
    // count #out-going edges for each vertex
    do {   
      x = (FSTAR_INT)_fp.read_int();

      if ( FILE_err&4 ) break;

      if ( x<0 || x >= _FS.get_out_node_num() ){
        print_err ("set ID out of bound "FSTAR_INTF">"FSTAR_INTF"\n", x, _FS.get_out_node_num());
        exit(0);
      }

			for(i=_FS.get_fstar(x);i<_FS.get_fstar(x+1);i++){

        if ( cnt[_FS.get_edge(i)]++ == 0 ){ 
        	que[t*2+1] = _FS.get_edge(i); 
        	t++; 
        }
      }
      s++;
    } while ( (FILE_err&3)==0 );

    if ( _problem & MEDSET_ALLNUM ){

			for(i=0;i< _FS.get_in_node_num();i++){

        fprintf (_ofp, "%.2f ", ((double)cnt[i])/(double)s);
        cnt[i] = 0;
      }
      fprintf (_ofp, "\n");
      continue;

    }

    if ( s>0 ){

			for(i=0;i< t;i++){
      	que[i*2] = cnt[que[i*2+1]]; cnt[que[i*2+1]] = 0; 
      }

      qsort_<FSTAR_INT>(que, t, (_problem&MEDSET_BELOW?1:-1)*((int)sizeof(FSTAR_INT))*2);

			for(i=0;i< t;i++){
        if ( _problem & MEDSET_BELOW ){
           if ( ((double)que[i*2])/(double)s > _th ) break;
        } 
        else if ( ((double)que[i*2])/(double)s < _th ) break;

        if ( _problem & MEDSET_NO_HIST ) fprintf (_ofp, ""FSTAR_INTF" ", que[i*2+1]);
        else if ( _problem & MEDSET_RATIO ) fprintf (_ofp, "("FSTAR_INTF":%.2f) ", que[i*2+1], ((double)que[i*2])/(double)s);
        else fprintf (_ofp, ""FSTAR_INTF" ", que[i*2+1]);
      }
    }
    fprintf (_ofp, "\n");

  } while ( (FILE_err&2)==0 );

  END:;
  mfree (cnt, que);
}



/*******************************************************************/
int KGMEDSET::run (int argc, char *argv[]){

  if( setArgs ( argc, argv) ) return 1;

  _fp.open( _input_fname, "r");
  fopen2(_ofp, _output_fname, "w", goto END);


	// うえ２つは別にしたほうが
  if ( _problem & MEDSET_CC ){
		/* read file, output the histogram of each line */
		kgClusterForCC ccc;
		ccc.read(&_fp);
		ccc.print(_ofp,_num);
  }
  else if ( _problem & MEDSET_IND ){
		kgClusterForCC ccc;
		ccc.readWithCnt(&_fp);
		ccc.print(_ofp,_num);
  }
  else{
		_fsFlag |= LOAD_BIPARTITE;
  	_FS.setParams(_fsFlag,_set_fname,1);
  	if ( _FS.load() ) return 1;
		print_mesf (&_FS, "medset: cluster-file= %s set-file= %s threshold= %f output-file= %s\n", _input_fname, _set_fname, _th, _output_fname);
  	read_file();
  }

  END:;

  _fp.close();
  fclose2 (_ofp);

  return (_ERROR_MES?1:0);
}
int KGMEDSET::mrun (int argc, char *argv[]){
	return KGMEDSET().run(argc,argv);
}


