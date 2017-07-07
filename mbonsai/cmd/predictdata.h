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
#include <kgCSVout.h>
#include<vector>
#include<map>
//======================================================
// PredictData
// 行番号とクラスごと予測値
//======================================================
class PredictLineData
{
	size_t _size;
	size_t _clsSz;
	map< int,vector<double> >  _data;
	map< int,int >  _predict;

public:
	PredictLineData(){}
	PredictLineData(int sz,int clsSz):
		_size(sz),_clsSz(clsSz){}

	void set(size_t linNo ,vector<double> &data,int predict) {
		 _data[linNo] = data; 
		 _predict[linNo] = predict;
	}
	bool find(int lin){
		return _predict.find(lin) != _predict.end();
	}
	int get_predict(int lin){return _predict[lin];}
	map< int,int > * get_predict(void){return &_predict;}

	vector<double>* get_weight(int lin){return &_data[lin];}

};		
//======================================================
// アルファごとの予測情報
//======================================================
class PredictData
{
	size_t _size;
	size_t _clsSz;
	map<double,PredictLineData> _pdata; //LINE情報
	map<double,int> _edata;             //エラー情報
	map<double,int> _ldata;             //リーフ情報
	map<double,double> _erate;             //エラー情報
	double _err_min;
	double _SE1;
	double _errlimit;

	double _evaluation;
	double _evalu_alpha;

	double _min_alpha;
	double _SE1_alpha;


public:
	PredictData(vector<double> & alphaList,map<double,int> lfcList, int sz,int clsSz){
		_size=sz;
		_clsSz=clsSz;
		_err_min = DBL_MAX;
		_errlimit = DBL_MAX;
		_min_alpha= DBL_MAX;
		_SE1_alpha= DBL_MAX;
		_evalu_alpha= DBL_MAX;
		_evaluation= DBL_MAX;
		_SE1 = 0;
		for( vector<double>::iterator it=alphaList.begin();it!=alphaList.end();it++){
			_pdata[*it] = PredictLineData(sz,clsSz);
			_edata[*it] = 0;
			_ldata[*it] = lfcList[*it];
		}	
	}
	void infoSet(double alpha , size_t linNo ,vector<double>& data,int predict,bool errFlg) {
		// cerr << "aax " << alpha << endl;
		_pdata[alpha].set(linNo,data,predict);
		if(errFlg){ _edata[alpha]++; }
	}
	void cal_min(void)
	{
		for( map<double,int>::iterator it = _edata.begin(); it != _edata.end(); it++) {
			// cerr << "aa " << it->first << endl;
			_erate[it->first] = (double)it->second / (double)_size;
			if( _err_min > _erate[it->first]){
				_err_min = _erate[it->first];
				_min_alpha = it->first;
			}
		}
		_SE1 = sqrt(_err_min*(1-_err_min)/(double)_size);
		_errlimit = _SE1 + _err_min;
		// errlimit以下でリーフの少ないものを選択
		int maxleaf = INT_MAX;
		for (map<double,double>::iterator it = _erate.begin(); it != _erate.end(); it++) {
			if( it->second > _errlimit ){ continue;}
			if(_ldata[it->first] < maxleaf){
				maxleaf = _ldata[it->first];
				_SE1_alpha=it->first;
			}
		}
		_evaluation = _err_min;
		_evalu_alpha = _min_alpha;
	}
	
	double evaluation(void){ return _evaluation;}
	void set_evaluation(double alpha){
		//alphaを超えた最初の値をセット
		_evaluation = DBL_MAX;
		_evalu_alpha = alpha;
		for (map<double,double>::iterator it = _erate.begin(); it != _erate.end(); it++) {
			if(it->first < alpha ){ continue; }
			_evaluation = it->second;
			break;  
		}
	}
	double errMin(void){ return _err_min;}

	bool better(PredictData *base){
		if(base==NULL){return true;}
		if(_evaluation<base->evaluation()) { return true;}
		return false;
	
	}

	double min_alpha(void){return _min_alpha;}
	double se1_alpha(void){return _SE1_alpha;}
	double evalu_alpha(void){return _evalu_alpha;}

	bool find_line(int incnt)	{  return _pdata[_min_alpha].find(incnt);	}

	int predict_min(int lin){
		return _pdata[_min_alpha].get_predict(lin);
	}
	int predict_se1(int lin){
		return _pdata[_SE1_alpha].get_predict(lin);
	}

	map< int,int > * predict_se1(){
		return _pdata[_SE1_alpha].get_predict();
	}

	map< int,int > * predict_min(){
		return _pdata[_min_alpha].get_predict();
	}



	vector<double>* weight_min(int lin){
		return _pdata[_min_alpha].get_weight(lin);
	}
	vector<double>* weight_se1(int lin){
		return _pdata[_SE1_alpha].get_weight(lin);
	}

	void outInfo(kgCSVout *out){

		vector<kgstr_t> nf_a ;
		nf_a.push_back("alpha");
		nf_a.push_back("leafSize");
		nf_a.push_back("errorRate");
		nf_a.push_back("SE");
		nf_a.push_back("up");
		nf_a.push_back("lo");
		out->writeFldName(nf_a);

		for (map<double,double>::iterator it = _erate.begin(); it != _erate.end(); it++) {
			double se1 = sqrt(it->second*(1-it->second)/(double)_size);
			out->writeDbl(it->first, false );
			out->writeInt(_ldata[it->first], false );
			out->writeDbl(it->second, false );
			out->writeDbl(se1, false );
			out->writeDbl(it->second + se1, false );
			out->writeDbl(it->second - se1, true );
		}
	
	}


};		

