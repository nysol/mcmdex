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
#include <kgArgFld.h>
#include <kgCSV.h>

namespace kgmod { ////////////////////////////////////////////// start namespace

// 入力データ情報
class DataInfo {

  size_t  _rec;   // データ行数
  size_t  _fCnt;  // 項目数
  vector<double> _maxNum; // 最大値
  vector<double> _minNum; // 最小値
  vector<double> _rngNum; // 最大値-最小値
  vector<double> _sumNum; // 合計値 
  vector<int>    _cntNum; // 件数(nullを省く) 
  vector<double> _avgNum; // 平均値


	void init(size_t size){
		_rec=0;
		_fCnt = size ;
		_maxNum.clear();_maxNum.resize(size,-DBL_MAX); 
  	_minNum.clear();_minNum.resize(size,DBL_MAX); 
  	_rngNum.clear();_rngNum.resize(size,0); 
  	_sumNum.clear();_sumNum.resize(size,0); 
  	_cntNum.clear();_cntNum.resize(size,0); 
  	_avgNum.clear();_avgNum.resize(size,0); 
	}
	
	void vSet(size_t i,double v) {
		_cntNum[i]++;
		_sumNum[i]+=v;
		if ( _maxNum[i] < v ) { _maxNum[i] = v;  } 
		if ( _minNum[i] > v ) { _minNum[i] = v;  } 
	}

	// 全てNULLはNG
	bool valid(){

		for(size_t i=0;i<_cntNum.size();i++){
			if(_cntNum[i]==0) return false;
		}
		return true;
	}

	// range , avg
	void calc(void) { 

		for(size_t i=0;i<_avgNum.size();i++){
			_rngNum[i] = _maxNum[i] - _minNum[i];
			_avgNum[i] = _sumNum[i] / _cntNum[i];
		}

	}



  public:
		DataInfo():_rec(0){}
		

	size_t getDatInfo( kgCSVfld& icsv,kgArgFld& fField){

		init(fField.size());
	
		while( icsv.read()!=EOF ){

			for(size_t i=0 ; i<_fCnt ; i++){ 

				char *str = icsv.getVal(fField.num(i));
				if(*str!='\0'){ vSet(i,atof(str)); }

			}
			_rec++;
		}

		if(!valid()){ throw kgError("value not found on some fields"); }

		calc();

		return _rec;
		
	}
	
	
	size_t Rec(void){ return _rec;}
	double Avg(size_t i){return _avgNum[i];}
	double Rng(size_t i){return _rngNum[i];}
	double Min(size_t i){return _minNum[i];}


	// debug
	void print(){
		cerr << "rec " << _rec << endl;
		for (size_t i=0;i<_minNum.size();i++){
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

}