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

#include "regptn.h"

using namespace kglib;
using namespace std;



bool RegPtn::le_lt(const RegPtn& other,bool eql){
	// プライオリティ(splitting rule)が低い方が勝ち
			 if( _val > other._val){ return true; }
	else if( _val < other._val){ return false; }

	//プライオリティが同じときは正規パターン長が長い方が勝ち*/
	else if( _ptn.size() < other._ptn.size()) { return true; }
	else if( _ptn.size() > other._ptn.size()) { return false; }

	// bgnRngがあればその数が小さい方が勝ち
	else if( _bgn != 0 && other._bgn == 0){ return true; }
	else if( _bgn == 0 && other._bgn != 0){ return false; }
	else if( _bgn != 0 && other._bgn != 0 && _bgn > other._bgn) { return true; }
	else if( _bgn != 0 && other._bgn != 0 && _bgn < other._bgn) { return false; }

	// endRngがあればその数が小さい方が勝ち
	else if( _end != 0 && other._end == 0) { return true; }
	else if( _end == 0 && other._end != 0) { return false; }
	else if( _end != 0 && other._end != 0 && _end > other._end){ return true; }
	else if( _end != 0 && other._end != 0 && _end < other._end){ return false; }

	// substring と subsequence ではsubstringの勝ち
	else if( _type != false and other._type == true){ return true; }
	else if( _type != true  and other._type == false){ return false; }
	else {
		for(size_t i=0;i<_ptn.size();i++){
			if( _ptn.at(i) > other._ptn.at(i) ) { return true; } 
			if( _ptn.at(i) < other._ptn.at(i) ) { return false; } 
		}
	}
	return eql;
}

void RegPtn::addPtn(int x){ _ptn.add(x);}

RegPtn* RegPtn::right(int i){ 
	return new RegPtn(_ptn.right(i),_type,_bgn,_end);
}
	
bool RegPtn::regCmpSeq(ValtypeVec &idx)
{
	if(idx.null()){return false;}
	int str_len = idx.size();
	int reg_len = _ptn.size();
	int str_cnt=0;
	int reg_cnt=0;
	while(str_cnt < str_len && reg_cnt < reg_len){
		int pos = idx.strchr(_ptn.at(reg_cnt),str_cnt);
		if(pos < 0){ return false; }
		str_cnt += pos;
		str_cnt++;
		reg_cnt++;
	}
	if (reg_len != reg_cnt){ return false;}
	// 先頭一致の指定がある場合
	if( _bgn>0 ){
		bool flg= false;
		for( int i=0 ; i<_bgn ; i++){
			if(_ptn.at(0) == idx.at(i)){
				flg = true;
				break;
			}
		}
		if(!flg) {return false;}
	}
	// 末尾一致一致の指定がある場合
	if( _end>0 ){
		bool flg= false;
		for( int i=0 ; i<_end ; i++){
			int pos = reg_len-i-1;
			if( pos<0 ) { break; }
			if( _ptn.at(reg_len-1) == idx.at(pos)){
				flg = true;
				break;
			}
		}
		if(!flg) {return false;}
	}
	return true;
}

//--------------------------------------------------------------------
//	パターンxにself.end文字内に終わるパターン(self.__ptn)があるか
//		受け取ったデータxが正規パターンに後方一致するか
//--------------------------------------------------------------------
bool RegPtn::atEndRng(ValtypeVec &idx)
{
	int x_len = idx.size();
	int y_len = _ptn.size();
	int start = x_len - _end - y_len + 1;
	int 	end = x_len - y_len + 1;

	if(start<0){ start = 0; }
	for(int i=start ; i<end ; i++){
		if( idx.strncmp(i,_ptn.val(),y_len) ){ return true;}
	}
	return false;
}

bool RegPtn::regCmpStr(ValtypeVec &idx)
{
	if(idx.null()){  return false;}
	int bgnpos = idx.strstr(_ptn.val());
	if(bgnpos<0)										{ return false;}
	if(_bgn>0 && bgnpos > _bgn )		{ return false;}
	if(_end>0 && atEndRng(idx)==0 )	{ return false;}
	return true;
}






