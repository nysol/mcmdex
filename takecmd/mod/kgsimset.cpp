/************************************************************************/
/* simstr.c: find frequently appearing string patterns from a string, by 
   collecting frequently appearing short substrings and extend it unless the 
   "voting" does not succeed (the mojority letter in a position of them is
   not determined) */
   
#include <unistd.h>

#include "kgsimset.hpp"
#include "kggrhfil.hpp"
#include "kgsspc.hpp"
#include "kgmedset.hpp"
#include "kgmace.hpp"


//_REMAIN_DIFFS_は除去

/* remove many files */
void mremove_ (char *x, ...){
  va_list argp;
  char *a;
	char common_comm[1024];
  va_start (argp, x);
  while((a = va_arg(argp, char *))){
    sprintf (common_comm, "%s%s", x, a);
    remove (common_comm);
  }
  va_end (argp);
}


// remove a file on the specified directory
#define MREMOV(dir,...) mremove_(dir, __VA_ARGS__, NULL, NULL)

void KGSIMSET::_error(void){

  fprintf(stderr,"simset ISCMOt [options] similarity-graph-filename similarity-threshold degree-threshold output-filename\n\
%%:show progress, _:no message, +:write solutions in append mode, =:do not remove temporal files\n\
@:do not execute data polishing, E:read edge list file\n\
i:set similarity measure to the ratio of one is included in the other\n\
I:set similarity measure to the ratio of both are included in the other\n\
S:set similarity measure to |A\\cap B|/max(|A|,|B|)\n\
s:set similarity measure to |A\\cap B|/min(|A|,|B|)\n\
C:set similarity measure to the cosign distance, the inner product of the normalized characteristic vectors\n\
T:set similarity measure to the intersection size, i.e., |A\\cap B|\n\
R:(recemblance) set similarity measure to |A\\cap B|/|A\\cup B|\n\
P(PMI): set similarity measure to log (|A\\capB|*|all| / (|A|*|B|)) where |all| is the number of all items\n\
M:output intersection of each clique, instead of IDs of its members\n\
v (with M): output ratio of records, including each item\n\
m:do not remove edges in the data polishing phase\n\
O:repeatedly similarity clustering until convergence\n\
Y:take intersection of original graph and polished graph, instead of clique mining\n\
1:do not remove the same items in a record (with -G)\n\
W:load weight of each element\n\
t:transpose the input database, so that each line will be considered as a record\n\
\n[options]\n\
-G [similarity] [threshold]:use [similarity] of [threshold] in the first phase (file is regarded as a transaction database)\n\
-k [threshold]:find only k-best for each record in -G option\n\
-M [num]:merge similar cliques of similarity in [num] of recemblance (changes to 'x' by giving '-Mx')\n\
-m [num]:take independently cliques from similar cliques of similarity in [num] of recemblance, and merge the neighbors of each independent clique ('recemblance' changes to 'x' by giving '-Mx')\n\
-v [num]:specify majority threshold (default=0.5) (if negative is given, items of frequency no more than -[num] are output)\n\
-u [num]:ignore vertices of degree more than [num]\n\
-l [num]:ignore vertices of degree less than [num]\n\
-U [num]:ignore transactions of size more than [num] (with -G)\n\
-L [num]:ignore transactions of size less than [num] (with -G)\n\
-I [num]:ignore items of frequency more than [num] (with -G)\n\
-i [num]:ignore items of frequency less than [num] (with -G)\n\
 (-II,-ii [ratio]:give thresholds by the ratio of #ransactions/#items)\n\
-T,t [num]:ignore pairs whose intersection size is less than [num] (T for first phase with -G option, and t for polishing)\n\
-O [num]:specify the number of repetitions\n\
-9 [num]:shrink records of similarity more than [num]\n\
-X [num]:multiply the weight by [num] (and trancate by 1, for C command)\n\
-x [num]:power the weight by [num] (and normalize, for C command)\n\
-y [num]:at last, remove edges with weight less than [num]\n\
-w [filename]:load weight of elements from the file\n\
-! [num]:use multicores of [num] (compile by 'make multicore')\n\
-W [dir]:specify the working directory (folder). The last letter of the directory has to be '/' ('\\')\n\
-, [char]:give the separator of the numbers in the output\n\
-Q [filename]:replace the output numbers according to the permutation table given by [filename]\n\
# the 1st letter of input-filename cannot be '-'.\n\
if similarity-threshold is 0, skip the similarity graph construction phase\n");
	EXIT;

}


/***********************************************************************/
/*  read parameters given by command line  */
/***********************************************************************/
void KGSIMSET::read_param(int argc, char *argv[]){
 int c=1;
  _prog[0] = 0; _sep[0] = 0;
		
  if ( argc<c+5 ){ _error (); return; }
  if ( strchr (argv[c], '_') ){ strcat (_prog, "_"); _mes = 0; _mes2 = " -_";}
  if ( strchr (argv[c], '%') ) strcat (_prog, "%");
  if ( strchr (argv[c], '+') ) _append = 1;
  if ( strchr (argv[c], '=') ) _leave_tmp_files = 1;
  if ( strchr (argv[c], '@') ) _repeat = -1;

  if ( strchr (argv[1], 'I') ) _com = 'I';
  else if ( strchr (argv[1], 'i') ) _com = 'i';
  else if ( strchr (argv[1], 'C') ){_com = 'C'; _f = "f"; }
  else if ( strchr (argv[1], 'T') ) _com = 'T';
  else if ( strchr (argv[1], 'S') ) _com = 'S';
  else if ( strchr (argv[1], 'R') ) _com = 'R';
  else if ( strchr (argv[1], 's') ) _com = 's';

  if ( strchr (argv[1], 'M') ) _intersection = 1;
  if ( strchr (argv[1], 'm') ) _no_remove = 1;
  if ( strchr (argv[1], 'O') ) _repeat = 10000000;
  if ( strchr (argv[c], 't') ) _tpose = 't';
  if ( strchr (argv[c], 'H') ) _hist = 1;
  if ( strchr (argv[c], 'v') ) _ratio = 1;
  if ( strchr (argv[c], 'E') ) _edge = 'E';
  if ( strchr (argv[c], 'Y') ) _intgraph = 1;
  if ( strchr (argv[c], 'W') ) _itemweight = "w";
  if ( strchr (argv[c], '1') ) _rm_dup = 1;
	//  SIMSET_com1 = SIMSET_com;
  c++;
    
  _item_lb[0] = _item_lb[0] = 0;
  _trsact_ub[0] = _trsact_lb[0] = 0;

  while ( argv[c][0] == '-' ){
    if ( argc<c+5 ){ _error (); return; }
    switch ( argv[c][1] ){
      case 'G':
      	if ( !strchr ("IiCTSsR", argv[c+1][0]) ){
          fprintf(stderr,"unknown similarity measure\n");
					exit(1);
        }
        if ( (_th1 = atof(argv[c+2])) <= 0 ){
          fprintf(stderr,"the majority threshold has to be positive : %g\n", atof(argv[c+1]) );
          exit(1);
      	}
        _com1 = argv[c+1][0]; c++; 

      break; case 'v':
      	_vote_th = atof(argv[c+1]);

      break; case 'm': 
      	if ( (_th3 = atof(argv[c+1])) <= 0 ){
          fprintf(stderr,"the independent set threshold has to be positive : %g\n", atof(argv[c+1]) );
          exit(1);
        }
        if ( argv[c][2] ){ _com2 = argv[c][2]; }
        else { _com2 = 'R'; }

      break; case 'M': 
      	if ( (_th2 = atof(argv[c+1])) <= 0 ){
					fprintf(stderr,"the merge threshold has to be positive : %g\n", atof(argv[c+1]) );
					exit(1);
				}
				if ( argv[c][2] ){ _com2 = argv[c][2];}
				else { _com2 = 'R';}

      break; case '9':
      	if ( (_th4 = atof(argv[c+1])) <= 0 ){
					fprintf(stderr,"the unification threshold has to be positive : %g\n", atof(argv[c+1]) );
					exit(1);
        }
      break; case 'k': 
      	if ( (_thk = atoi(argv[c+1])) <= 0 ){
					fprintf(stderr,"the k-best threshold has to be positive : %g\n", atof(argv[c+1]) );
					exit(1);
        }
      break; case 'u': 
      	_deg_ub = atoi(argv[c+1]);

      break; case 'l': 
      	_deg_lb = atoi(argv[c+1]);
      	
      break; case 'U': 
      	sprintf (_trsact_ub, " -u %d", atoi(argv[c+1]));
      	
      break; case 'L': 
      	sprintf (_trsact_lb, " -l %d", atoi(argv[c+1]));
      	
      break; case 'I': 
      	sprintf (_item_ub, " -U%s %f", argv[c][2]=='I'?"U":"", atof(argv[c+1]));
      	
      break; case 'i': 
      	sprintf (_item_lb, " -L%s %f", argv[c][2]=='i'?"L":"", atof(argv[c+1]));
      	
      break; case 'T': 
      	_ignore = atoi(argv[c+1]);

      break; case 't': 
      	_ignore2 = atoi(argv[c+1]);

      break; case 'O': 
      	_repeat = atoi(argv[c+1]);
      break; case 'X': 
      	if ( (_multiply = atof(argv[c+1])) <= 0 ){
					fprintf(stderr,"the factor has to be positive : %g\n", atof(argv[c+1]) );
					exit(1);
        }
      break; case 'x': 
      	if ( (_power = atof(argv[c+1])) == 0 ){
					fprintf(stderr,"the factor has to be non zero : %g\n", atof(argv[c+1]) );
					exit(1);
        }
      break; case 'y': 
      	if ( (_cut = atof(argv[c+1])) == 0 ){
					fprintf(stderr,"the threshold has to be positive : %g\n", atof(argv[c+1]) );
					exit(1);
        }
      break; case 'w': 
      	_itemweight_file = argv[c+1];
      break; case '!': 
      	if ( (_cores = atoi(argv[c+1])) <= 0 ){
					fprintf(stderr,"the number of cores has to be positive : %d\n", atoi(argv[c+1]) );
					exit(1);
        }
      break; case 'W': 
      	_workdir = argv[c+1];

      break; case ',':
      	 _sep[0] = '-'; 
      	 _sep[1] = argv[c+1][0]; 
      	 _sep[2] = 32; 
      	 _sep[3] = 0;

      break; case 'Q': 
      	_outperm_fname = argv[c+1];

      break; default: goto NEXT;
   }
    c += 2;
  }
  NEXT:;
  _infname = argv[c];    // input file name
  _th = atof(argv[c+1]);  // similarity threshold
  _lb = atoi(argv[c+2]);  // threshold for cluster size
  _outfname = argv[c+3];  // output file name

  if ( (c = strlen(_infname) + strlen(_workdir)) > 800 ){
		fprintf(stderr,"too long filename/workdir : %d\n", c );
		exit(1);
	}
}

/*****************************************************************************/
/* unify the similar records, by deleting them except for one representative */
/*****************************************************************************/
// flag= 0:cluster, 1:graph, 2:for mace
void KGSIMSET::unify (char *fname, char *fname2, int flag){  

	FSTAR FF;
  char c;
  IFILE2 fp;
  OFILE2 wfp;
  FSTAR_INT i, j, e, z;
  char *buf;
  LONG x, y;
  
  if ( flag ){
    if ( !_unifind.empty() ){
	    _unifind.alloc(_nodes);
	  }
    fp.open ( fname2);
    do {
      if ( fp.read_pair( &x, &y, NULL, 0) ) continue;
      _unifind.unify_set(x,y);

    } while ( fp.NotEof() );
    fp.close();
  }

	FF.setParams(fname);
	FF.load();


  wfp.open(fname);

	for(i=0;i<FF.get_node_num() ; i++){
    if ( flag==0 || _unifind.mark(i) == i ){
      if ( flag == 0 ){
        z = 0; 
        for(j=FF.get_fstar(i);j<FF.get_fstar(i+1);j++){
          e = FF.get_edge(j);
          if ( _unifind.mark(e) != e ) continue;
          while (1){
            z++;
            if ( _unifind.set(e) == e ) break;
            e = _unifind.set(e);
          }
        }
        if ( z < _lb ) continue;
      }
      c = 0;
      for(j=FF.get_fstar(i);j<FF.get_fstar(i+1);j++){
        e = FF.get_edge(j);
        if ( _unifind.mark(e) != e ) continue;
        while (1){
	        wfp.print_int(e, c);
	        wfp.flush();
          c = ' ';
          if ( flag == 2 || _unifind.set(e) == e ) break;
          e = _unifind.set(e);
        }
      }
    }
    wfp.puts("\n");
	  wfp.flush();
  }
  wfp.close();

}

/*************************************************************************/
/* main function of SIMSET */
/*************************************************************************/
int KGSIMSET::run (int argc ,char* argv[]){

	read_param (argc, argv);

  //char *W = _workdir, 
  int flag=1, count=0, unify_flag = 0, break_flag = 0;
  size_t siz=0, sizz = 0, diff=0, difff=0;

  char s1[1024], s2[1024], s3[1024], s4[1024], s5[1024];
  char s6[1024], s7[1024], s8[1024], s9[1024];
  char sk[20];
	char *s10 = " -d 0.001"; 
	char *f1  = "__tmp_out0__"; 
	char *f2  = "__tmp_out1__" ;
	char *fn  = f1;
	char ff  = 0;
	std::vector<LONG> sspcrtn;
	
  sprintf (s1, "%s__tmp__", _workdir);  // write commands to string variables
  sprintf (s2, "%s__tmp_out2__", _workdir);

  s5[0] = s6[0] = 0; 
  if ( _th4 > 0 ){
    sprintf (s5, " -9 %f %s__tmp_unify__", _th4, _workdir); 
    sprintf (s6, "%s__tmp_unify__", _workdir); 
  }

  FILE2::copy(_infname, s1);  // copy the input file to temporary file

  s7[0] = s8[0] = 0;

  if ( _itemweight_file ){
    sprintf (s7, " -W %s", _itemweight_file);
    if ( _com == 'C' ){ sprintf (s8, " -W %s__tmp_weight__", _workdir); } // inpro
  }

  s9[0]=0; 
  if ( _multiply > 0 )      sprintf (s9, " -X %f" , _multiply);
  else if ( _power != 0.0 ) sprintf (s9, "Z -x %f", _power);

  if ( _com != 'C' ) s10 = ""; // not inpro

  sk[0]=0; 
  if ( _thk ) sprintf (sk, " -k %d", _thk);

  // transaction comparison phase: similarity graph construction for transaction database
  if ( !_com1 ){
  	sprintf(
  		_cmn_comm, "grhfil %su09%c%s%s%s \"%s\" \"%s__tmp__\"",
			_prog, _edge=='E'?'e':' ', _itemweight, s7, s8, _infname, _workdir
		);

		comm_str_decompose();
	  KGGRHFIL::mrun(_cmn_argc,_cmn_argv);

    goto GRAPH_POLISHING;

  }


  // for giving -T option for SSPC
  s3[0] = 0; 
  if ( _ignore > 0 ) sprintf (s3, " -T %d", _ignore);
  // find similar pairs
  if ( _edge=='E' ){
  	sprintf(
  		_cmn_comm,"grhfil ed%s%s%s \"%s\" \"%s__tmp_edge__\"",
      _itemweight, s7, s8, _infname, _workdir
    );

		comm_str_decompose();
	  KGGRHFIL::mrun(_cmn_argc,_cmn_argv);

    sprintf (s4, "%s__tmp_edge__", _workdir);
    _infname = s4;
  }


  // similarity graph construction phase
  sprintf(
	  _cmn_comm,"sspc 0%s%c%s%s%s%s%s%s%s%s%s%s -M %d \"%s\" %f \"%s__tmp_out__\"",
    _prog, _com1, _rm_dup?"":"1", _tpose=='t'?"t":"", s3, s5, s8, sk,
    _item_lb, _item_ub, _trsact_lb, _trsact_ub, _cores, _infname, _th1,  _workdir
  );
	comm_str_decompose();
	sspcrtn =  KGSSPC::mrun(_cmn_argc,_cmn_argv);

  unify_flag = 1; 
  _nodes = sspcrtn[1];// internal_params.l3; //  stdlib2.cppにある

  // convert edge type to list type
  sprintf(
	  _cmn_comm,"grhfil %sue91%s%s -n " UNIONFIND_IDF " \"%s__tmp_out__\" \"%s__tmp__\"",
    _prog, _itemweight, s8, _nodes, _workdir, _workdir
  );
	comm_str_decompose();
	KGGRHFIL::mrun(_cmn_argc,_cmn_argv);
  
  // phase 2: data polishing
  GRAPH_POLISHING:;

  if ( _intgraph ){ 
	  sprintf(
		  _cmn_comm,"grhfil %su \"%s__tmp__\" \"%s__tmp_org__\"",
      _prog, _infname, _workdir
    );
		comm_str_decompose();
		KGGRHFIL::mrun(_cmn_argc,_cmn_argv);
  }

  if ( _repeat < 0 ) goto FIND_CLIQUE;   // clique enumeration without polishing

  s3[0] = 0; 
  if ( _ignore2 > 0 ) sprintf (s3, " -T %d", _ignore2);
  
  do {  // data polishing loop

    count ++;
    if ( _mes ){	
    	fprintf(stderr,"%dth-iter\n", count);
    }
    if ( _th4 > 0 && unify_flag ) unify (s1, s6, 1);

		// neighbor similarity comparison
		sprintf(
			_cmn_comm,"sspc 0%s%c%s%s%s%s -l %d -u %d -M %d \"%s__tmp__\" %f \"%s%s\"",
      _prog, _com, _f, s3, s5, s8,
      _deg_lb, _deg_ub, _cores, _workdir, _th, _workdir, fn
    );
		comm_str_decompose();
		std::vector<LONG> sspcrtn = KGSSPC::mrun(_cmn_argc,_cmn_argv);

    unify_flag = 1; 
    _nodes = sspcrtn[1];//internal_params.l3;
    siz = sspcrtn[0];// internal_params.l1;
    if ( siz == 0 ){ 
    	fprintf(stderr,"no similar pair exists\n");
    	break; 
    }

    if ( _com != 'C' && siz == sizz && siz > 0 ){
			sprintf(
				_cmn_comm,"grhfil %sdEe%s -dde%s \"%s__tmp_out0__\"%s \"%s__tmp_out1__\" \"%s__tmp_diff%d__\"", 
        _prog, _com=='C'?"w":"", _com=='C'?"w":"", _workdir, s10, _workdir, _workdir, ff			
			);
    	// if  _REMAIN_DIFFS_ SIMSET_prog, SIMSET_com=='C'?"w":"", SIMSET_com=='C'?"w":"", W, s10, W, W, count
			comm_str_decompose();
			LONG rtn3 = KGGRHFIL::mrun(_cmn_argc,_cmn_argv);


      if ( rtn3  == 0 ) break_flag = 1;  // internal_params.l1 no difference
      if ( _mes ) fprintf (stderr, "#diff edges %lld\n", rtn3 ); //internal_params.l1
      
      difff = rtn3; //internal_params.l1;
      if ( rtn3 == diff && break_flag == 0 ){ //internal_params.l1

				sprintf(
					_cmn_comm , "grhfil dEe%s -dde%s \"%s__tmp_diff%d__\"%s \"%s__tmp_diff%d__\" \"%s__tmp_diff__\"", 
    	      _com=='C'?"w":"", _com=='C'?"w":"", _workdir, (ff+1)%2, s10, _workdir, ff, _workdir
      	);
				comm_str_decompose();
				LONG rtn4 = KGGRHFIL::mrun(_cmn_argc,_cmn_argv);

        if ( rtn4 == 0 ) break_flag = 1;  // no difference internal_params.l1
        if ( _mes ) fprintf (stderr, "#diff-diff edges %lld\n", rtn4 );//internal_params.l1
      }
      diff = difff; 
      ff = (ff+1)%2;
    }
    else{
	    diff = -1;
	  }
    sizz = siz;

    if ( _com == 'C' ){ sprintf (s8, " -W %s__tmp_weight__", _workdir); } // inpro
		
		sprintf(
			_cmn_comm , "grhfil %s%sue91%s%s -n " UNIONFIND_IDF " \"%s%s\" \"%s__tmp__\"",
      _com=='C'?"w":"", _prog, s9, s8, _nodes, _workdir, fn, _workdir
    );
		comm_str_decompose();
		KGGRHFIL::mrun(_cmn_argc,_cmn_argv);

    if ( _no_remove == 1 ){
			sprintf(
				_cmn_comm , "grhfil %sue -M %s \"%s__tmp__\" \"%s__tmp_out2__\"", 
				_prog, _infname, _workdir, _workdir
			);
    	comm_str_decompose();
			KGGRHFIL::mrun(_cmn_argc,_cmn_argv);
      rename (s2, s1);
    }
    if ( break_flag || !_repeat || _repeat == count || count>=5000 ) break;      // repeat until maximi iterations
    fn = fn == f1? f2: f1; // alternate the result filename of sspc

	}while (1);

  if ( _com == 'C' ){ // not inpro
		sprintf(
			_cmn_comm ,"grhfil %sd -1 %f -w \"%s__tmp_weight__\" \"%s__tmp__\" \"%s__tmp_out2__\"",
      _prog, _cut, _workdir, _workdir, _workdir
		);
    comm_str_decompose();
		KGGRHFIL::mrun(_cmn_argc,_cmn_argv);

    sprintf (s8, "%s__tmp_out2__", _workdir);
    sprintf (s7, "%s__tmp__", _workdir);
    unlink(s7);
    rename(s8, s7);
  }
  
  if ( _intgraph ){ 
		sprintf(
			_cmn_comm ,"grhfil %su0 -m \"%s__tmp__\" \"%s\"",
      _prog, _workdir, _outfname
		);
    comm_str_decompose();
		KGGRHFIL::mrun(_cmn_argc,_cmn_argv);
    goto END;
  }

  // phase3: clinue enumeration
  FIND_CLIQUE:;
  if ( _th4 > 0 && unify_flag ) unify (s1, s6, 2);

	sprintf(
		_cmn_comm ,"mace %sM %s-l %d \"%s__tmp__\" \"%s__tmp_out2__\"",
     _prog, _sep, _th4>0?1:_lb, _workdir, _workdir
	);
  comm_str_decompose();
	KGMACE::mrun(_cmn_argc,_cmn_argv);

  if ( _th4 > 0 && !_unifind.empty() ){ unify (s2, NULL, 0);}

  // phase4: merge similar cliques
  if ( _th2 > 0.0 || _th3 > 0.0 ){
		sprintf(
			_cmn_comm ,"sspc %s%c %s-M %d \"%s__tmp_out2__\" %f \"%s__tmp2__\"",
    	_prog, _com2, _sep, _cores, _workdir, _th2+_th3, _workdir
		);
		comm_str_decompose();
		KGSSPC::mrun(_cmn_argc,_cmn_argv);

		sprintf(
			_cmn_comm ,"medset%s%s -l %d -%c \"%s__tmp2__\" \"%s__tmp_out2__\" 1 \"%s__tmp2__\"",
    	_mes2, _tpose=='t'?" -t":"", _lb, _th2>0.0? 'T': 'I', _workdir, _workdir, _workdir
		);
		comm_str_decompose();
		KGMEDSET::mrun(_cmn_argc,_cmn_argv);

    sprintf (_cmn_comm, "%s__tmp2__", _workdir);
    rename (_cmn_comm, s2);
  }

	// phase5: take intersection
  if ( _intersection ){
  	sprintf(
			_cmn_comm , "medset%s %s%s%s-l %d %s\"%s__tmp_out2__\" \"%s\" %f \"%s\"",
      _mes2, _sep, _tpose=='t'?"-t ":"", _ratio?"-i ": "", _lb,
      _hist?"-H ":"", _workdir, _infname, _vote_th, _outfname
    );
		comm_str_decompose();
		KGMEDSET::mrun(_cmn_argc,_cmn_argv);
	}
  else{
	  rename (s2, _outfname);
  }
  
  flag = 0;
  END:;
  if ( !_leave_tmp_files ){
  	 MREMOV (
  	 	_workdir, "__tmp__", "__tmp2__", "__tmp_out__", 
  	 	"__tmp_out0__", "__tmp_out1__", "__tmp_out2__",
  	 	"__tmp_weight__", "__tmp_edge__", "__tmp_org__", 
  	 	"__tmp_unify__", "__tmp_diff__", "__tmp_diff0__", "__tmp_diff1__"
  	 );
  }

  return 0;

}
