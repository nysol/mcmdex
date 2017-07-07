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
#include<sstream>
#include <cstdio>
#include "aiqueue.h"

namespace kglib {
// -------------------------------------------
//  vetor int データ型
// -------------------------------------------
class ValtypeVec
{
		vector<int> _val;
		bool _null;

	public:
		ValtypeVec(void)                 : _null(false){}
		ValtypeVec(vector<int> x)        : _val(x),_null(false){}
		ValtypeVec(bool n)               : _null(n){}
		ValtypeVec(vector<int> x,bool n) : _val(x),_null(n){}

		// ------------------------------------------------------
		// アクセッサ
		// ------------------------------------------------------
		bool        null(void)         const { return _null;}
		vector<int> val(void)          const { return _val; }
		void        null(bool n)       { _null=n;}
		void        val(vector<int> x) { _val=x; }

		void show(int indent=0)
		{ 
			for(int i=0; i<indent; i++) printf(" ");
			printf("vec(null=%d):",_null);
			for(size_t i=0; i<_val.size() ;i++){
				printf("%d",_val[i]);
			}
		}

		string toStr(void){ 
			ostringstream ss;
			printf("vec(null=%d):",_null);
			for(size_t i=0; i<_val.size() ;i++){
				ss << _val[i];
			}
			return ss.str();
		}

		// ------------------------------------------------------
		// vector操作
		// ------------------------------------------------------
		// データ追加
		// ------------------------------------------------------
		void add(int x)
		{ 
			if(_null){
				_val.clear();
				_null = true;
			}
			_val.push_back(x);
		}

		// ------------------------------------------------------
		//  指定位置から以降を抜粋(0〜)
		// ------------------------------------------------------
		vector<int> right(size_t pos)
		{ 
			vector<int> rtn;
			for(size_t i=pos ; i<_val.size() ;i++){
				rtn.push_back(_val[i]);
			}
			return rtn;
		}
		// ------------------------------------------------------
		// 指定位置参照(0〜) 
		// ------------------------------------------------------
		int at(int pos) const 
		{
			return _val.at(pos);
		} 
		// ------------------------------------------------------
		// 指定値の位置取得(start:検索開始位置,y:検索値)	
		// 見つからなければreturn -1 
		// ------------------------------------------------------
		int strchr(int sval,int spos=0)
		{
			for(int i=spos ;i < _val.size();i++){
				if(_val.at(i)==sval) { return i;} 
			}
			return -1;
		}
		// ------------------------------------------------------
		// 指定値列の位置取得(y:検索値)	
		// 見つからなければreturn -1 
		// ------------------------------------------------------
		int strstr(vector<int> svals)
		{
			if(svals.size()==0){ return 0;}
			int xpos=strchr(svals.at(0));
			while(xpos>=0){
				int pre_xpos=xpos;
				int spos=0;
				while(1){
					spos++;
					if(spos==svals.size()) { return pre_xpos; }
					xpos++;
					if( xpos >= _val.size() || _val.at(xpos)!=svals.at(spos) ){ break; }
				}
				xpos=strchr(svals.at(0),pre_xpos+1);
			}
			return xpos;		
		}


		int strncmp(int pos1 , vector<int> ptn,int n)
		{
			int pos2=0;
			int s1  =0;
			int s2  =0;
			do{
				if(n==0){ s1=0; s2=0; break;}
				if(pos1==_val.size()){ s1=0; }
				else                 { s1=_val.at(pos1); }
				if(pos2== ptn.size()){ s2=0;}
				else                 { s2= ptn.at(pos2); }
				if( (s1-s2) != 0) break;
				pos1++; pos2++; n--;
			}while(true);
			return s1-s2;
		}

	//------------------------------------------------------
	// aimapを使用してインデックス化
	// aimapはindexOfメソッドを持っていると想定
	//------------------------------------------------------
	ValtypeVec toIndex(AIndex *ai)
	{
		ValtypeVec rtn;
		if(_null){ return rtn;}
		else{
			for(size_t i=0 ; i<_val.size();i++ ){
				rtn.add(ai->idxOf(_val[i]));
			}
		}
		return rtn;
	}

	ValtypeVec toIndex(vector<size_t> &idx)
	{
		ValtypeVec rtn;
		if(_null){ return rtn;}
		else{
			for(size_t i=0 ; i<_val.size();i++ ){
				rtn.add(idx.at(_val[i]));
			}
		}
		return rtn;
	}


	int size() const  {
		if(_null) return 0;
		else			return _val.size();
	}
	string to_s() const{
		stringstream rtn;
		if(_null){ return rtn.str();}
		for(int i=0 ; i<_val.size() ; i++){
			rtn << _val[i];
		}
		return rtn.str();
	}

	bool operator<(const ValtypeVec& other){
		for(size_t i=0;i<_val.size()&&i<other._val.size();i++){
			if( _val.at(i) < other._val.at(i) ) { return true; } 
			if( _val.at(i) > other._val.at(i) ) { return false; } 
		}
		if(_val.size()<other._val.size()){ return true; }
		return false;
	}


};
}
