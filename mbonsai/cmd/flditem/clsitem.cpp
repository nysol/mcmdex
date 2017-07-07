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

#include "flditem/clsitem.h"

using namespace kglib ;

void ClsItem::costSet(kgCSVfld &cfile){
	_cost.initSet(_itemMap.size());
	if(cfile.opened()){
		while(cfile.read()!=EOF){
			int fr  = _itemMap.item2id(cfile.getVal(0));
			int to  = _itemMap.item2id(cfile.getVal(1));
			double val =  atof(cfile.getVal(2));
			_cost.set(fr,to,val);
		}
	}
	_cost.update();
}
//----------------------------------------------------------------
// アイテム読み込み
// NULL値は登録しない
//----------------------------------------------------------------
void ClsItem::init_read(char** val)
{
	if(*val[fldnum()]=='\0'){ throw kgError("cls field cannot have a NULL value"); }
	else					          {	_itemMap.push(val[fldnum()]);}
}

void ClsItem::itemSet(vector<kgstr_t> &ilist){
	for(size_t i=0 ; i<ilist.size() ; i++){
		_itemMap.push(ilist[i]);
	}
}
//----------------------------------------------------------------
// データ読み込み
//----------------------------------------------------------------
void ClsItem::read(char** val)
{
	if(*val[fldnum()]=='\0'){
		_trainingDat.push_back(ValtypeInt(true));
	}else{
		int id = _itemMap.item2id(val[fldnum()]);
		_trainingDat.push_back(ValtypeInt(id));
	}
}

void ClsItem::dataSplit(int type,double sep,size_t seed){

	int sepcnt=2;
	if(type==2){ sepcnt=int(sep);}

	//分割する件数を決める
	vector< vector<int> > sepdist(itmSz() ,vector<int>(sepcnt) );
	vector< vector<int> > totaldist(itmSz() ,vector<int>(sepcnt) );
	if(type==1){//1:テストサンプル法
		for(size_t i=0 ; i<itmSz();i++){
			int tcnt = int(_itemMap.count(i)*sep);
			if(tcnt==0) { throw kgError("can't separate since data is small "); }
			sepdist.at(i).at(1)=tcnt;
			sepdist.at(i).at(0)=_itemMap.count(i)-tcnt;
			totaldist.at(i).at(0)= _itemMap.count(i);
			totaldist.at(i).at(1)= tcnt;
		}
	}
	else{
		for(size_t i=0 ; i<itmSz();i++){
			int div = _itemMap.count(i)/sepcnt;
			int sup = _itemMap.count(i)%sepcnt;
			if(div==0) { throw kgError("can't separate since data is small "); }
			for(size_t j=0 ; j<sepcnt;j++){
				if(j<sup) { sepdist.at(i).at(j)= div+1; }
				else			{ sepdist.at(i).at(j)= div; }		
				}
		}
		for(size_t i=0 ; i<itmSz();i++){
			int total = 0;
			for(size_t j=0 ; j<sepcnt;j++){
				totaldist.at(i).at(j)= _itemMap.count(i)-total;
				total += sepdist.at(i).at(j);
			}
		}
	}
	//乱数生成エンジン
	uniform_int<> dst(0,INT_MAX);
	variate_generator< mt19937,uniform_int<> > rand_m(mt19937((uint32_t)seed),dst); 

	// 分割リスト生成
	_separate.resize(sepcnt);	

	// 分割
	for(size_t i=0 ;i<traSz();i++){
		int cls = getVal(i);
		bool output = false;
		for(size_t j=0 ;j<sepcnt-1;j++){
			if( (rand_m()%totaldist.at(cls).at(j))< sepdist.at(cls).at(j)){
				_separate.at(j).push_back(i);
				output =true;
				totaldist.at(cls).at(j)--;
				sepdist.at(cls).at(j)--;
				break;
			}		
			totaldist.at(cls).at(j)--;
		}
		if(!output){
			_separate.at(sepcnt-1).push_back(i);
		
		}
	}
}


//コスト関係
double ClsItem::getTTLCost(int no) const{
	return _cost.getTTL(no);
}
