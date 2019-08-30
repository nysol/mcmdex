
#include"vec.hpp"

class OQueue{

	QUEUE * _oq;
	size_t _size; //必要ないかも
	
	public:

	OQueue(void):_oq(NULL),_size(0){}
	
	QUEUE_INT* start(size_t i){ return _oq[i].start(); }
	QUEUE_INT* begin(size_t i){ return _oq[i].begin(); }
	QUEUE_INT* end(size_t i)  { return _oq[i].end(); }
	QUEUE_INT * get_v(size_t i){ return _oq[i].get_v(); }
	QUEUE_INT get_v(size_t i,size_t j){ return _oq[i].get_v(j); }


	void clrMark(size_t i,char *mark)	{ _oq[i].clrMark(mark); }
	void endClr(size_t i)  { return _oq[i].endClr(); }
	void set_end(size_t i,int v){ _oq[i].set_end(v); }
	void set_sentinel(size_t i){ _oq[i].set_v( _oq[i].get_t(),INTHUGE);}
	void move(size_t i,size_t j){ return _oq[i].move(j); }

	void setfromT(SETFAMILY &T){
		QUEUE_INT M = T.get_clms();
		for (VEC_ID i=0 ; i< _oq[M].get_t() ; i++){ 
	  	VEC_ID e = _oq[M].get_v(i);
			for ( QUEUE_INT * x = T.get_vv(e) ; *x < M ; x++){ 
				_oq[*x].push_back(e);
			}
		}
	}

	// 仮
	QUEUE * getOQ(void){ return _oq; }

	void alloc(SETFAMILY &T){
		VEC_ID *p = T.counting();
		QUEUE_INT clm_max = p[0];
		size_t cmmsize = p[0];
		for(int cmm = 1 ; cmm < T.get_clms() ;cmm++ ){
			cmmsize += p[cmm];
			if( clm_max < p[cmm]){ clm_max = p[cmm]; }
		}
		
		_oq = new QUEUE[T.get_clms()+1]; 
		_size = T.get_clms()+1;

		QUEUE_INT *cmn_pnt;
		try{
			cmn_pnt = new QUEUE_INT[cmmsize+T.get_clms()+2]; //malloc2
		}catch(...){
			delete[] _oq;
			throw;
		}
		size_t posx=0;
		for(VEC_ID cmmv =0; cmmv < T.get_clms() ; cmmv++){
			_oq[cmmv].alloc( p[cmmv],cmn_pnt+posx );
			posx += p[cmmv]+1;
		}
		_oq[T.get_clms()].alloc( MAX(T.get_t(), clm_max));

		for(size_t i=0 ; i < T.get_clms()+1 ; i++ ){
			_oq[i].endClr();
		}
		_oq[T.get_clms()].initVprem(T.get_t());  //ARY_INIT_PERM
		delete [] p;
	}
	void prefin(size_t i ,QUEUE_INT * o){
	  _oq[i].add_t( _oq[i].get_v() - o);
  	_oq[i].set_v ( o);
  }

};
