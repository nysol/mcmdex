//#include "trsact.hpp"
//#include "problem.hpp"
//#include "itemset.hpp"
#pragma once

#include "stdlib2.hpp"
#include "unionfind.hpp"
#define WEIGHT_DOUBLE


class KGSIMSET{

	int   _cmn_argc;

	double _th;    // threshold for data polishing
	double _th1;   // threshold for the transaction comparison phase
	double _th2;   // threshold for clique unification
	double _th3;   // threshold for taking clique from clique connected component (connected by similarity measure)
	double _th4;   // threshold for unification of records
	int    _thk;   // threshold for the k-best transaction comparison phase


	char _sep[4];
	int _deg_ub, _deg_lb;   // ignore vertices of degree more/less than this, in data polishing
	char _item_ub[20], _item_lb[20];   // ignore items of frequency more/less than this, in transaction comparison phase
	char _trsact_ub[20], _trsact_lb[20];  // ignore transactions of size more/less than this, in transaction comparison phase
	int _lb;  // ignore clusters of size less than this

	char *_infname, *_outfname, *_outperm_fname, *_workdir;  // input/output/permutation/ file names
	int _no_remove;  // do not remove edges in "data polishing", if 1
	int _repeat;  // #repetition of "data polishing"
	int _intersection;      // take intersection of each cluster
	int _intgraph;      // take intersection of original graph and polished graph
	int _ignore, _ignore2;      // ignore pairs with intersection size of less than this (first phase/second phase)
	char _tpose;   // transpose the input? (only when -T option is given)
	int _mes;     // output messages, if 1
	int _append;  // append the output to the output file, if 1
	int _leave_tmp_files;  // do not delete the temporary files, if 1
	char _edge, _prog[10], *_mes2 ;    // tmp variables for commands
	char _com, _com1, _com2;  // similarity measure; for polishing, transaction comparison, and clique clustering
	char _rm_dup; // do not remove duplicates when 1
	int _hist;       // output histogram? (frequency is output to each item)
	double _vote_th;  // threshold for taking the intersection of each cluster; items of frequency less than this will not be output
	int _ratio;   // output the original frequency of each item, in intersection mode
	char *_itemweight ;  // load itemweitht from ele-file if 1
	char *_itemweight_file;  // load itemweight from this file
	char *_f;  // for spefifying f command
	double _multiply, _power , _cut ;
	int _cores ;  // multi-core mode

	void read_param(int argc, char* argv[]);
	void _error(void);

	char  _cmn_comm[1024];
	char* _cmn_argv[100];


	// number of nodes in the graph to be posihed
	// mark for union finding (grouping)
	UNIONFIND _unifind;
	UNIONFIND_ID _nodes;  

	// decompose the string by separator, and set v[i] to each resulted string.
	//  consecutive separators are regarded as one separator.
	// string s has to have end mark 0 at the end 
	// original string s will be written, so that each separator will be end mark 0 
	// at most [num] strings will be generated 
	void comm_str_decompose (char sep=' ', int max=100){

		int i=0, dq;
		char *ss = _cmn_comm;
		printf("%s\n",_cmn_comm);
		do {
			dq = 0;
			while (*ss == sep) ss++;
			if ( *ss == 0 ) break;
			if ( *ss == '\"' ){ dq = 1; _cmn_argv[i++] = ++ss; }
			else _cmn_argv[i++] = ss;
			while (*ss != sep || dq){
				if ( dq==0 && *ss == 0 ) break;
				if ( dq && *ss == '\"' ) break;
				ss++;
			}
			if ( *ss == 0 ) break;
			*(ss++) = 0;
			
		} while ( i<max);
		_cmn_argc = i;
		return ;
	}
	
	void unify (char *fname, char *fname2, int flag);

	public :

	KGSIMSET(void):
		_th(0.0),_th1(0.0),_th2(0.0),_th3(0.0),_th4(0.0),_thk(0),
		_nodes(0),_deg_ub(INTHUGE),_deg_lb(0),_outperm_fname(NULL),
		_no_remove(0),_repeat(0),_intersection(0),_intgraph(0),
		_ignore(0),_ignore2(0),_tpose(' '),_mes(1),_append(0),
		_leave_tmp_files(0),_edge(' '),_mes2(""),
		_com1(0),_rm_dup(0),_hist(0),_vote_th(0.5),_ratio(0),
		_itemweight(""),_itemweight_file(NULL),_f(""),
		_multiply(0),_power(0),_cut(0.0),_cores(1),_workdir("")
	{	}

	int run(int argc ,char* argv[]);
	
};