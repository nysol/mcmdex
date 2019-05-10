/* forward star type graph structure -- for quite large graphs */
/* 6/Nov/2008   Takeaki Uno  */

#pragma once


#include"stdlib2.hpp"
#include"file2.hpp"

//#define FSTAR_INT_LONG

#define FSTAR_REDUCE 33554432    // output table
#define FSTAR_CNT_IN 67108864
#define FSTAR_CNT_OUT 134217728
#define FSTAR_CNT_DEG_ONLY 268435456   // in_deg means usual degree

#define FSTAR_DEG_CHK 536870912
#define FSTAR_IN_CHK 1073741824
#define FSTAR_OUT_CHK 4194304
#define FSTAR_INS_ROWID 536870912  // insert row ID to the head of each line
#define FSTAR_INS_ROWID_WEIGHT 1073741824  // insert row ID to the head of each line

// undirected graph => CNT_DEG_ONLY, edge_dir = 0
// bipartite graph => BIPARTITE, edge_dir != 0
#ifdef FSTAR_INT_LONG
  #define FSTAR_INT LONG
  #define FSTAR_INTHUGE LONGHUGE
  #define FSTAR_INTF "%llu"
#else
  #define FSTAR_INT unsigned int
  #define FSTAR_INTHUGE UINTHUGE
  #define FSTAR_INTF "%u"
#endif


class FSTAR{

	// input file name, edge weight file name
  char *_fname, *_wfname;

  // separator for output file
  char _sep;         

	// flag
  int _flag;         

  FSTAR_INT *_edge;   // edge array
  FSTAR_INT *_fstar;  // starting position of edge list for each vertex
  FSTAR_INT *_in_deg, *_out_deg;    // in/out-degree of each vertex (can be NULL if not used)
  FSTAR_INT _node_num, _out_node_num, _in_node_num;  // #vertex and #1st/2nd vertices
  FSTAR_INT _edge_num, _edge_num_org, _reduced_node_num;  // #edges in file, in array
  FSTAR_INT _xmax, _ymax;  // maximum in 1st/2nd column
  FSTAR_INT *_table ;   // vertex permutation table 
  WEIGHT *_edge_w;   // edge weights

  int _edge_dir;
  
  FSTAR_INT _deg_lb, _in_lb, _out_lb, _deg_ub, _in_ub, _out_ub;  // bounds for degrees

  WEIGHT _w_lb, _w_ub; // bounds for edge weight

	char *_ERROR_MES;

		
	void inc_deg(FSTAR_INT x, FSTAR_INT y);//privateでOK?
		
	void sort_adjacent_node (int flag);
	LONG alloc_deg ();
	void calc_fstar ();

	void scan_file (FILE2 *fp);

	void read_file (FILE2 *fp, FILE2 *wfp);//privateでOK?

	void extract_subgraph();


	int  eval_edge ( FSTAR_INT x, FSTAR_INT y, WEIGHT w);


	//FILE *  open_write_file ( char *fname);//privateでOK?
		
	void write_graph_ID (OFILE2 &fp, OFILE2 &fp2, FSTAR_INT ID);//privateでOK?
	int  write_graph_item (FSTAR_INT x, FSTAR_INT y, WEIGHT w, OFILE2 &fp, OFILE2 &fp2, int *row, FSTAR_INT *prv);//privateでOK?

		
	void edge_w_pow(LONG l,double ratio){
		_edge_w[l] = pow (_edge_w[l], ratio);
	}

	void edge_w_mul_min(LONG l,double ratio,double th){
		_edge_w[l] *= ratio; 
		_edge_w[l] = (_edge_w[l]<th) ? _edge_w[l] : th ;
	}

	void edge_w_div(int x,double w){
		_edge_w[x] /= w;
	}

	double edge_w_mul(LONG l){ 
		return _edge_w[l]* _edge_w[l]; 
	}

	double eWeight(LONG l){ 
		if(_edge_w){
			return _edge_w[l];
		}
		return 0;
	}


	void writeHeadInfo(OFILE2 &fp);


	public:

		FSTAR():
			_fname(NULL),_wfname(NULL),_sep(' '),_flag(0),
			_edge(NULL),_fstar(NULL),_in_deg(NULL),_out_deg(NULL),
			_node_num(0),_out_node_num(0),_in_node_num(0),_edge_num(0),
			_edge_num_org(0),_reduced_node_num(0),_xmax(0),_ymax(0),
			_table(NULL),_edge_w(NULL),_edge_dir(0),
			_deg_lb(0),_in_lb(0),_out_lb(0),	
			_deg_ub(FSTAR_INTHUGE),_in_ub(FSTAR_INTHUGE),_out_ub(FSTAR_INTHUGE),
			_w_lb(-WEIGHTHUGE),_w_ub(WEIGHTHUGE),_ERROR_MES(NULL){}

		~FSTAR(){
			//mfree (_edge, _edge_w, _in_deg, _out_deg, _fstar, _table);
			delete [] _edge; 
			delete [] _edge_w; 
			delete [] _in_deg; 
			delete [] _out_deg; 
			delete [] _fstar;
			delete [] _table;

		}

	
		// Medset
		void setParams( int fsFlag, char *fname ,int edge_dir)
		{
			_flag   = fsFlag;
			_fname  = fname;
			_edge_dir = edge_dir;
		}

		// simset
		void setParams( char *fname)
		{
			_fname  = fname;
		}

		// grhfil
		void setParams( int fsFlag, char *fname ,int edge_dir,char sep,FSTAR_INT rows)
		{
			_flag   = fsFlag;
			_fname  = fname;
			_edge_dir = edge_dir;
			_sep = sep;
			_out_node_num = rows;
			
		}

		// grhfil
		void setParams( 
			int fsFlag, char *fname ,int edge_dir,char *wfname,
			FSTAR_INT deg_lb,FSTAR_INT deg_ub,
			FSTAR_INT in_lb,FSTAR_INT in_ub,
			FSTAR_INT out_lb,FSTAR_INT out_ub,
			WEIGHT w_lb , WEIGHT w_ub,
			char sep, FSTAR_INT rows)
		{
			_flag = fsFlag;
			_fname = fname;
			_edge_dir = edge_dir;
  		_wfname = wfname;
			_deg_lb = deg_lb;
			_deg_ub = deg_ub;
			_in_lb = in_lb;
			_in_ub = in_ub;
			_out_lb = out_lb;
			_out_ub = out_ub;
			_w_lb = w_lb;
			_w_ub = w_ub;
			_sep  = sep;
			_out_node_num = rows;
		}
		

		int adjust_edgeW(double ratio,double th,double th2,int norm , int discret )
		{
			LONG l,ll, x,xx;
  		WEIGHT w;

			if(!_edge_w) return 0;
				
			 // multiply & trancate

		  if ( ratio != 0 ){
		    //FLOOP (l, 0, _edge_num){ 
		    for(l=0;l<_edge_num;l++){
		      if ( th == DOUBLEHUGE ){
        		edge_w_pow (l, ratio);
      		}
      		else { 
      			edge_w_mul_min(l , ratio, th);
      		}
    		}
  		}

		  if ( norm ){
    		//FLOOP (l, 0, _out_node_num){
		    for(l=0;l<_out_node_num;l++){
		      w = 0.0;
		      //FLOOP (x, _fstar[l], _fstar[l+1]){
			    for(x=_fstar[l];x<_fstar[l+1];x++){
		      	w += edge_w_mul(x);
		      }
		      w = sqrt (w);
    			//FLOOP (x, _fstar[l], _fstar[l+1]){
			    for(x=_fstar[l];x<_fstar[l+1];x++){
    			 	edge_w_div(x,w);
    			}
		    }
  		}

		  if ( discret ){
  		  xx = 0;

		    for(l=0;l<_out_node_num;l++){
	      	ll = _fstar[l]; 
  	    	_fstar[l] =  xx;

			    for(x=ll;x<_fstar[l+1];x++){

    			  if ( _edge_w[x] >= th2 ){
    			  	_edge[xx] = _edge[x];
    			  	_edge_w[xx] = _edge_w[x];
							xx++;
    		  	}

      		}
		  	}
    		_fstar[l] = xx;
  		}
  		return 0;
		}

		int load();
		//int  get_flag(void){ return _flag;}
		void set_flag(int flag){ _flag=flag;}


		void printMes(char *frm ,...){

			if( _flag&1 ){
				va_list ap;
				va_start(ap,frm);
				vfprintf(stderr,frm,ap);
				va_end(ap);
			}
		}



		void write_table_file (char *fname);

		static LONG write_graph_operation (FSTAR *F1, FSTAR *F2, char *fname, char *fname2, int op, double th);
		LONG write_graph (char *fname, char *fname2);


		static FSTAR_INT maxNodeNum(FSTAR *F1, FSTAR *F2){
			FSTAR_INT mv = F1->_out_node_num;
			if(mv < F1->_in_node_num) { mv = F1->_in_node_num; }
			if(mv < F2->_out_node_num){ mv = F2->_out_node_num;}
			if(mv < F2->_in_node_num) { mv = F2->_in_node_num; }
			return mv;
		}
		
		FSTAR_INT get_in_node_num(void){return _in_node_num;}
		FSTAR_INT get_out_node_num(void){return _out_node_num;}
		FSTAR_INT get_node_num(void){return _node_num;}
		
		FSTAR_INT get_fstar(LONG l){ return _fstar[l];}
		FSTAR_INT get_edge(LONG l){ return _edge[l];}
} ;


