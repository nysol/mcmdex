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
#include "itemmap.h"
using namespace kglib;

// ------------------------------------------------------
//  アイテムの登録
// ------------------------------------------------------
void itemMap::push(kgstr_t val)
{
	map<kgstr_t,int>::const_iterator itor=_str2num.find(val);
	if(itor==_str2num.end()){
		_str2num[val] = _num2str.size();
		_num2str.push_back(val);
		if(_cntF){
			_num2cnt.resize(_num2str.size(),1);
		}
	}
	else{
		if(_cntF){ _num2cnt[itor->second]++;}
	}
}		

// ------------------------------------------------------
//  アイテムの登録(分割登録)
// ------------------------------------------------------
void itemMap::push(char* val,char delim)
{
	if(delim=='\0'){
		string str(val);
		for(size_t i=0 ;i < str.size();i++){
			push(str.substr(i,1));
		}
	}
	else{
		vector<char*> token = splitToken(val,delim);
		for(size_t i=0; i<token.size(); i++){
			push(token.at(i));
		}
	}
}

// ------------------------------------------------------
// アイテム名から内部IDを取得する
// ------------------------------------------------------
int itemMap::item2id(char *val)const
{
	map<kgstr_t,int>::const_iterator itor=_str2num.find(val);
	if(itor==_str2num.end()){
		throw kgError("internal error: invalid item value");
	}
	return itor->second;
}
// ------------------------------------------------------
// アイテム名から内部IDを取得する
// ------------------------------------------------------
vector<int> itemMap::item2id(char *val,char delim)
{
	vector<int>	rtn;
	if(delim=='\0'){
		string str(val);
		for(size_t i=0 ;i < str.size();i++){
			kgstr_t val = str.substr(i,1);
			map<kgstr_t,int>::const_iterator itor=_str2num.find(val);
			if(itor==_str2num.end()){
				throw kgError("internal error: invalid item value");
			}
			rtn.push_back(itor->second);
		}
	}
	else{
		vector<char*> token = splitToken(val,delim);
		for(size_t i=0; i<token.size(); i++){
			map<kgstr_t,int>::const_iterator itor=_str2num.find(val);
			if(itor==_str2num.end()){
				throw kgError("internal error: invalid item value");
			}
			rtn.push_back(itor->second);
		}
	}
	return rtn;
}

ostream& operator<<(ostream& out, const itemMap &vs)
{
	out << "=====itemmap======" << endl;;
	for(int i=0;i<vs.size();i++){
		out << vs.id2item(i) << endl;
	}	
	out << "==================" << endl;;
	return out;
};


