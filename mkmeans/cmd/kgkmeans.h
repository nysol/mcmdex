/* ////////// LICENSE INFO ////////////////////

 * Copyright (C) 2013 by NYSOL CORPORATION
 *
 * Unless you have received this program directly from NYSOL pursuant
 * to the terms of a commercial license agreement with NYSOL, then
 * this program is licensed to you under the terms of the GNU Affero General
 * Public License (AGPL) as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF 
 * NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Please refer to the AGPL (http://www.gnu.org/licenses/agpl-3.0.txt)
 * for more details.

 ////////// LICENSE INFO ////////////////////*/
#pragma once

#include <string>
#include <vector>
#include <set>
#include <kgConfig.h>
#include <kgEnv.h>
//#include <kgmodincludesort.h>
#include <kgmod.h>
#include <kgError.h>
#include <kgArgs.h>
#include <kgArgFld.h>
#include <kgCSV.h>
#include <kgCSVout.h>

using namespace kglib;

namespace kgmod { ////////////////////////////////////////////// start namespace

/*入力データ関連*/
class DataInfo {
  size_t  _cnt;    /*データ行数*/
  size_t  _fldsize;    /*項目サイズ*/
  vector<double> _maxNum; /*数値項目の最大値*/
  vector<double> _minNum; /*数値項目の最小値*/
  vector<double> _rngNum; /*数値項目の最大値-最小値*/
  vector<double> _sumNum; /*数値項目の合計値*/
  vector<int>    _cntNum; /*数値項目の件数(nullを省く)*/
  vector<double> _avgNum; /*数値項目の平均値*/
  public:
		DataInfo():_cnt(0){}
		
	size_t getCnt(void){ return _cnt;}
	void sizeSet(size_t size){
		_cnt=0;
		_fldsize = size;
		_maxNum.clear();_maxNum.resize(size,-DBL_MAX); /*数値項目の最大値*/
  	_minNum.clear();_minNum.resize(size,DBL_MAX); /*数値項目の最小値*/
  	_rngNum.clear();_rngNum.resize(size,0); /*数値項目の最大値-最小値*/
  	_sumNum.clear();_sumNum.resize(size,0); /*数値項目の合計値*/
  	_cntNum.clear();_cntNum.resize(size,0); /*数値項目の件数(nullを省く)*/
  	_avgNum.clear();_avgNum.resize(size,0); /*数値項目の平均値*/
	}
	
	double getAvg(size_t i){return _avgNum[i];}
	double getRng(size_t i){return _rngNum[i];}
	double getMin(size_t i){return _minNum[i];}

	
	void info_set(size_t i,double v) {
		_cntNum[i]++;
		_sumNum[i]+=v;
		if ( _maxNum[i] < v ) { _maxNum[i] = v;  } 
		if ( _minNum[i] > v ) { _minNum[i] = v;  } 
	}
	void cal_rng_avg(void) { // range , avg

		for(size_t i=0;i<_fldsize;i++){
			_rngNum[i] = _maxNum[i] - _minNum[i];
			_avgNum[i] = _sumNum[i] / _cntNum[i];
		}
	}
	void cnt_inc(){ _cnt++; }

	bool existAllData(){

		if(_fldsize==0){ return false;}

		for(size_t i=0;i<_fldsize;i++){
			if(_cntNum[i]==0) return false;
		}

		return true;
	}
	
	// debug
	void print(){
		cerr << "line " << _cnt << endl;
		for (size_t i=0;i<_fldsize;i++){
			cerr << " min:" << _minNum[i]; 
			cerr << " max:" << _maxNum[i]; 
			cerr << " rng:" << _rngNum[i];
			cerr << " sum:" << _sumNum[i];
			cerr << " cnt:" << _cntNum[i];
			cerr << " avg:" << _avgNum[i];
			cerr << endl;
		}

	
	}
	
	
};

class CalNum {
  double _accum;
  int   _cnt;
  public:
  void clear(){
  	_cnt=0;
  	_accum=0;
  }
  void add(double val){
	  _accum += val;
  	_cnt ++;
  }
  int getCnt(void){ return _cnt;}
  double getAvg(void){ return _accum/_cnt;}
  double getAccm(void){ return _accum;}

};

class Cluster {

  int _cnt;               /*クラスタに属するレコード数*/
  vector<CalNum> _calNum; /*重心の計算用*/
  vector<kgVal>  _cenNum; /*重心*/

	public:

	void setSize(size_t fcnt){
		_calNum.resize(fcnt);
		_cenNum.resize(fcnt,kgVal('N'));
	}

	void set_cenNum(int i ,double v){ _cenNum[i].r(v);}

	double get_cenNum(int i ){ return _cenNum[i].r();}
	double is_cenNum(int i ){ return _cenNum[i].null();}

	void resetCnt(){_cnt=0;}
	void incCnt(){_cnt++;}

	void resetCalnum(){
		for(size_t i=0 ; i<_calNum.size();i++){
			_calNum[i].clear();
		}
	}
	
	void addAccm(int i ,double v){
  	_calNum[i].add(v);
  }
  int getCalCnt(size_t i){ return _calNum[i].getCnt();} 
	double getCalAvg(size_t i){ return _calNum[i].getAvg();} 
	int getCnt(){ return _cnt;}

	double getAccm(int i ){
  	return _calNum[i].getAccm();
  }

};

class Clusters {

	vector < Cluster > _cluster;

	public:

	void resize(int clsCnt,int fCnt){
		_cluster.resize(clsCnt);
		for(int i=0;i<clsCnt;i++){
	  	_cluster[i].setSize(fCnt);
	  }
	}
	Cluster& at(int i){ return _cluster[i]; }

	void reset(){
		for(int i=0; i<_cluster.size();i++){
	    _cluster[i].resetCnt();
  	  _cluster[i].resetCalnum();
		}
	}
	void resetCalnum(int i){
  	  _cluster[i].resetCalnum();
  }


	void set_cenNum(int k,int i,double v){ _cluster[k].set_cenNum(i,v); }
	int getCalCnt(int k,int i)    { return _cluster[k].getCalCnt(i); }
	double getCalAvg (int k,int i){ return _cluster[k].getCalAvg(i); }
	double get_cenNum(int k,int i){ return _cluster[k].get_cenNum(i);}
	double getAccm (int k,int i)  { return _cluster[k].getAccm(i); }

	void addAccm(int k,int i ,double v){
  	_cluster[k].addAccm(i , v);
  }


	void incCnt(int k){ _cluster[k].incCnt(); }
	int getCnt(int k){ return _cluster[k].getCnt(); }
};


class Sample {
	vector < vector<kgVal> > _SmpRec;
  int _recCnt;
  //int _numFldCnt;
  
	public:
	void setSize(size_t reccnt,size_t fcnt ){
		_SmpRec.resize(reccnt);
		for(size_t i=0;i<reccnt;i++){
			_SmpRec[i].resize(fcnt,kgVal('N'));
		}
	}
	void setVal(int i,int j,double v){ _SmpRec[i][j].r(v);}
	double getVal(int i,int j){ return _SmpRec[i][j].r(); }
	int getCnt(){ return _recCnt; }
	void setCnt(int v){ _recCnt=v; }
	//void setFCnt(int v){ _numFldCnt=v; }
	void incCnt(){_recCnt++;}

};

class kgKmeans : public kgMod {

	// 引数
	kgCSVfld _iFile;  // i=
	kgCSVout _oFile;  // o=

	int _clusterCnt; // n=
	kgArgFld _fField; // f=
	unsigned long _seed;			 // S=
	kgstr_t		_addstr;	//a=
	int		_dType;	//d=
	int		_mcnt;	// m=

	vector < Sample > _sample;
	//vector < Cluster > _cluster;
	
	Clusters _clusters;

	kgAutoPtr1<boost::variate_generator< boost::mt19937,boost::uniform_int<>  > > _rand_mod;	

	DataInfo _datainfo;
	// 引数セット
	void setArgs(void);
	void getDatInfo(void);
	//void samplingSmp(Sample *sample, int recCnt);
	void setSmp2Cluster(int k,Sample  *sample,int s,Clusters* cls);
	void initClusterRA(int i,Clusters* cls);
	void initClusterKA(int i,Clusters* cls);
	Clusters* initClusterBF();
	void sampling( int mul,int recCnt);
	int movCenter(Clusters *clusters);
	int nearestCluster();
	void setCluster();
	double calDistanceClsSmp(Cluster &cluster, int n,int spos);
	int getCenInstance(int pos);
	double get_dji(Sample *sample, int m, int n );
	double get_Dj(int k,int j);


	void farthest(Clusters *clusters,int k,int spos);
	void setClusterSmp(Clusters *clusters,  Sample*  sample);
	int nearestClusterSmp(Clusters *clusters,Sample *sample,int rpos);

  public:
  // コンストラクタ
	kgKmeans(void);
	~kgKmeans(void){}

	int run(void);

	// コマンド固有の公開メソッド
	size_t iRecNo(void) const { return _iFile.recNo();}
	size_t oRecNo(void) const { return _oFile.recNo();}

};

}