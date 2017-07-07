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

#include "regptnque.h"
using namespace kglib;
using namespace std;

int RegPtnQueue::MaxRegLen=10;

RegPtn* RegPtnQueue::pop(void){
	RegPtn* rtn = _pqTop.top(); 
	_pqTop.pop();
	return rtn;
}
RegPtn* RegPtnQueue::popBest(void){ 
	RegPtn* rtn = _regBest.top(); 
	_regBest.pop();
	return rtn;
}

bool RegPtnQueue::isFBDN(RegPtn* regptn)
{
	if( regptn->bgn()!=0 || regptn->end()!=0 ){ return false; }
	bool rtn=false;
	for(size_t i=0 ; i<regptn->size() && !rtn ; i++){
		RegPtn* tmpreg =  regptn->right(i);
		if( _forbidden.find(tmpreg) !=  _forbidden.end() ) {
			rtn = true; 
		}
		delete tmpreg;
	}
	return rtn;
}

//--------------------------------------------------------------------
// ベストキューPUSH
// キューサイズサイズが超えた場合にはチェックしてからセットする
// キューからすべてとり出してから再度セットしているので、
// もっと効率の良い方法があると思う
//--------------------------------------------------------------------
bool RegPtnQueue::pushBest(RegPtn* regptn)
{
	if(_queMax<0){  // 候補パターン数上限がマイナスの場合は無条件にpush
		_regBest.push(new RegPtn(regptn,true));
		return true;
	}
	if(_regBest.size()<_queMax){  // 候補パターン数上限に達していなければ
		RegPtn * regptn_dup = new RegPtn(regptn,true);
		_regBest.push(regptn_dup);
		if( _maxVal==NULL || *regptn_dup < *_maxVal ){ _maxVal = regptn_dup; }
	}
	// 候補パターン数上限に達している場合
	else{
		// regptnの方が良いパターンの場合(maxValはqueの中で最も悪いパターン)
		// maxValより悪いパターンの場合はregptnは登録せずfalseを返す。
		if( *_maxVal < *regptn ){
			vector<RegPtn*> tmpque; 
			RegPtn* limptn;
			while( _regBest.size()>1 ){
				limptn = _regBest.top();
				tmpque.push_back(limptn);
				_regBest.pop();
			}
			RegPtn * regptn_dup = new RegPtn(regptn,true);
			if( *limptn < *regptn ){ _maxVal=limptn; }
			else{ _maxVal=regptn_dup; }
			RegPtn* dist_val = _regBest.top();
			_regBest.pop();
			delete dist_val;
			for(size_t i=0 ; i<tmpque.size() ; i++){
				_regBest.push(tmpque[i]);
			}
			_regBest.push(regptn_dup);
		}
		else{ 
			return false; 
		}
	}
	return true;
}


//--------------------------------------------------------------------
// conditionクラスで有効度を計算して
// 条件を満たせばregを登録する(regBest)
// 登録できたものはpqTopにも追加する（再度チェックするため）
//--------------------------------------------------------------------
void RegPtnQueue::pushReg(RegPtn* regptn,vector<ValtypeVec>& idxdat)
{
	if( isFBDN(regptn) ) { 
		delete regptn;
		return; 
	}
	_condition.reset();
	bool noMatch = _condition.calObjValCnt(regptn , idxdat);
	// unmatch
	if(noMatch){
		if( regptn->bgn()==0 and regptn->end()==0){
			_forbidden.insert(regptn);		
		}
		else{
			delete regptn;
		}
		return;
	}
	// マッチした物は優先度によって登録するか決定
	regptn->val(_condition.splitAfter());
	// クラスが2つDEないときは別途考慮
	regptn->ub (_condition.calUpperBound());

	// 現在のBest nに入るならばregBestにセット、入ったら一緒にキューにもセット
	if ( pushBest(regptn) ) {
		if(regptn->size() < RegPtnQueue::MaxRegLen){ _pqTop.push(regptn); }
		return ;
	}
	//BEST nに入らなかった
	if( _maxVal != NULL && regptn->ub() >  _maxVal->ub()){
		if( regptn->bgn()==0 and regptn->end()==0){ _forbidden.insert(regptn); }
		else{ delete regptn;}
		return;
	}
	if(regptn->size() > RegPtnQueue::MaxRegLen){ 
		delete regptn;
		return; 
	}
	_pqTop.push(regptn);
	return ;
}
