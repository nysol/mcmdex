namespace kglib ////////////////////////////////////////////// namespace start
{
class kgDblN
{
	bool _null;
	double _v;

	public:

	kgDblN():_null(true),_v(0){}
	kgDblN(double v):_null(false),_v(v){}

	kgDblN& operator+=(const kgDblN& val)
	{
		if(_null || val._null){ 
			_null = true; _v=0;
		}
		else{
			_v += val._v;
		}
		return *this;
	}
	friend kgDblN operator+(kgDblN val1, const kgDblN& val2){
		val1 += val2;
		return val1;
	}
	
	void print(){
		if(_null){
			cerr << "NULL" << endl;
		}
		else{
			cerr << _v << endl;
		}
	}
		
};

}
