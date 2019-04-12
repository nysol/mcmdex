/* take the intersection of each cluster */
/* 8/Nov/2008   Takeaki Uno  */

#include"fstar.hpp"
#include"stdlib2.hpp"
#include"kgmedset.hpp"
#include"problem.hpp"

/* get the ID of belonging group, and flatten the ID tree */
UNIONFIND_ID UNIONFIND_getID (UNIONFIND_ID v, UNIONFIND_ID *ID){
  UNIONFIND_ID vv = v, vvv;
  while (v != ID[v]) v = ID[v];  // trace parents until the root (ID[v] = v, if v is a root)
  while (vv != ID[vv]){ vvv = vv; vv = ID[vv]; ID[vvv] = v; }
  return (v);
}

    // maintain ID and list to representing the sets simultaneously
void UNIONFIND_unify_set (UNIONFIND_ID u, UNIONFIND_ID v, UNIONFIND_ID *ID, UNIONFIND_ID *set){
  UNIONFIND_ID z;
  v = UNIONFIND_getID (v, ID); // compute ID of v 
  u = UNIONFIND_getID (u, ID); // compute ID of u 
  if ( u != v ){
    if ( set[u] == u ){ set[u] = v; ID[v] = u; } // attach u as the head of the list of v
    else if ( set[v] == v ){ set[v] = u; ID[u] = v; }  // attach v as the head of the list of u
    else {
      for (z=v; set[z]!=z ; z=set[z]);  // find the last in the list of v
      set[z] = set[u]; set[u] = v; // insert the list of v to list of u
      ID[v] = u; // set ID of (ID of v) to (ID of u)
    }
  }
}



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


/* output clusters to the output file */
void KGMEDSET::print_clusters (
	FSTAR_INT *mark, FSTAR_INT *set, FSTAR_INT xmax)
{

  FSTAR_INT i, x, c;
  
  FLOOP (i, 0, xmax){

    if ( mark[i] != i ) continue;
    c = 0; x = i;

    while (1){
      c++;
      if ( set[x] == x ) break;
      x = set[x];
    }

    if ( c < _num ) continue;

    x = i;
    while (1){
      fprintf (_ofp, ""FSTAR_INTF" ", x);
      if ( set[x] == x ) break;
      x = set[x];
    }
    fputs ("\n", _ofp);
  }

}


/* read file, output the histogram of each line */
void KGMEDSET::read_file(){

  FSTAR_INT *cnt, *que, t, s, i, x;

  calloc2 (cnt, _FS.get_in_node_num(), EXIT);
  calloc2 (que, _FS.get_in_node_num()*2, goto END);
  
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

      FLOOP (i, _FS.get_fstar(x), _FS.get_fstar(x+1)){
        if ( cnt[_FS.get_edge(i)]++ == 0 ){ 
        	que[t*2+1] = _FS.get_edge(i); 
        	t++; 
        }
      }
      s++;
    } while ( (FILE_err&3)==0 );

    if ( _problem & MEDSET_ALLNUM ){

      FLOOP (i, 0, _FS.get_in_node_num()){
        fprintf (_ofp, "%.2f ", ((double)cnt[i])/(double)s);
        cnt[i] = 0;
      }
      fprintf (_ofp, "\n");
      continue;

    }

    if ( s>0 ){

      FLOOP (i, 0, t){ 
      	que[i*2] = cnt[que[i*2+1]]; cnt[que[i*2+1]] = 0; 
      }

      qsort_<FSTAR_INT>(que, t, (_problem&MEDSET_BELOW?1:-1)*((int)sizeof(FSTAR_INT))*2);

      FLOOP (i, 0, t){
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

/* read file, output the histogram of each line */
void KGMEDSET::cc_clustering (){

  FSTAR_INT *pnt=NULL, end1=0, end2=0, xmax=0, *mark=NULL, *set=NULL;

  LONG x, y;

  // merge the connponents to be connected by using spray tree
  do {
    if ( _fp.read_pair ( &x, &y, NULL, 0) ) continue;

    ENMAX (xmax, MAX(x, y)+1);

    //reallocx (mark, end1, xmax, common_size_t, EXIT);
    mark = reallocx (mark, &end1, xmax);

    //reallocx (set, end2, xmax, common_size_t, EXIT);
    set = reallocx (set, &end2, xmax);

    UNIONFIND_unify_set (x, y, (UNIONFIND_ID *)mark, (UNIONFIND_ID *)set);

  } while ( (FILE_err&2)==0 );

  print_clusters ( mark, set, xmax);

  END:;
  mfree (mark, set);

}


/* clustering the nodes by finding independent set */
/* cnt: cluster siz, if v is representative, and #vertices covering v, if v isn't representative */
void KGMEDSET::ind_clustering (){

  FSTAR_INT *pnt=NULL, flag,  xmax=0, *mark=NULL, *set=NULL, *cnt=NULL;
	size_t end1=0, end2=0, end3=0;

  LONG x, y, yy;

    // merge the connponents to be connected by using spray tree
  do {

    flag = 0;

    do {

      if ( _fp.read_pair ( &x, &y, NULL, 0) ) continue;

      ENMAX (xmax, MAX(x, y)+1);

      //reallocx (mark, end1, xmax, common_size_t, EXIT);
      mark = reallocx<FSTAR_INT>(mark, &end1, xmax);

      //reallocx (set, end2, xmax, common_size_t, EXIT);
			set = reallocx<FSTAR_INT>(set, &end2, xmax);

      //reallocx (cnt, end3, xmax, 0, EXIT);
      cnt = reallocx<FSTAR_INT>(cnt, &end3, xmax, 0);

      if ( cnt[x] < cnt[y] ) SWAP_<LONG> (&x, &y);

      if ( mark[x] == x && mark[y] == y ){

        if ( set[x] == x && !(set[y]== y && cnt[y]>0) ){ 
        	UNIONFIND_unify_set (y, x, (UNIONFIND_ID *)mark, (UNIONFIND_ID *)set); 
        	cnt[y]++; 
        	cnt[x] = 1; 
        	flag = 1; 
        }
        else {
          do {
            yy = set[y];
            set[y] = y;
            y = yy;
            mark[y] = y;
            cnt[y]--;
          } while (y != set[y]);
        }
        if ( set[y] == y ){ 
        	UNIONFIND_unify_set (x, y, (UNIONFIND_ID *)mark, (UNIONFIND_ID *)set); 
        	cnt[x]++; 
        	cnt[y] = 1; 
        	flag = 1;
        }
      }

      if ( mark[x] == x ){ cnt[y]++; }
      else if ( mark[y] == y ){ cnt[x]++; }

    } while ( (FILE_err&2)==0 );

  } while (flag);

  print_clusters (mark, set, xmax);

  END:;

  mfree (mark, set, cnt);

}


/*******************************************************************/
int KGMEDSET::run (int argc, char *argv[]){

  if( setArgs ( argc, argv) ) return 1;

	_fsFlag |= LOAD_BIPARTITE;
  _FS.setParams(_fsFlag,_set_fname,1);

  if ( _set_fname ){ 
  	if ( _FS.load() ) return 1;
  }
  print_mesf (&_FS, "medset: cluster-file= %s set-file= %s threshold= %f output-file= %s\n", _input_fname, _set_fname, _th, _output_fname);


  _fp.open( _input_fname, "r");
  fopen2(_ofp, _output_fname, "w", goto END);

  if ( _ERROR_MES ){ goto END ;}

  if ( _problem & MEDSET_CC ) cc_clustering();
  else if ( _problem & MEDSET_IND ) ind_clustering ();
  else read_file ();

  END:;

  _fp.close();
  fclose2 (_ofp);

//  PROBLEM_end (&PP);
  return (ERROR_MES?1:0);
}
int KGMEDSET::mrun (int argc, char *argv[]){
	return KGMEDSET().run(argc,argv);
}


