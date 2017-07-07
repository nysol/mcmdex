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
#include "aiqueue.h"

using namespace boost;
using namespace kglib;

bool AIndex::valid_init(){
	vector<bool> chk(_indexSz+1,false);
	for(size_t i=0 ; i< _a2i.size() ; i++){ 
		chk[_a2i.at(i)]=true; 
	}
	for(size_t i=1 ; i<=_indexSz ; i++){ 
		if(chk.at(i)) continue;
		return false;
	}
	return true;
}
// ----------------------------------------------------------------
// すべてが同じでないインデックスかチェック
// ----------------------------------------------------------------
bool AIndex::valid(void){
	size_t chr = _a2i.at(0);
	for(size_t i=1 ; i < _a2i.size() ; i++){
		if(chr != _a2i.at(i)){ return true; }
	}
	return false;
}

//--------------------------------------------------------------------
// インデックスの初期セット
// インデックスサイズが0の場合、アイテム数と同じと考える 
// 数値パターンは12223444のようにidxが作成される
// 文字パターンは14312312のようにidxが作成される
//--------------------------------------------------------------------
void AIndex::initial_set(){
//printf("xxxxxxxxxxxxxxxxxxxxindexSz=%lu noidx=%d\n",_indexSz,_noidx);

	//if(_indexSz == 0){
	if(_noidx){
//		_indexSz = _a2i.size();
		for(size_t i=0; i< _a2i.size() ; i++ ){
			_a2i[i]=i+1;
		}
	}
	else if(_ordered){//数値パターン
		size_t num_cnt=0;
		while(num_cnt<_indexSz-1){
			int digits = (*_generator)()/(INT_MAX/(_a2i.size()-1))+1;
			if ( _a2i.at(digits) !=0 ){ continue;}
			_a2i[digits]=9999;
			num_cnt++;
		}
		size_t num = 1;
		for(size_t i=0;i<_a2i.size();i++){
			if(_a2i.at(i)==9999){ num++; }
			_a2i[i]=num;
		}
	}
	else{//文字パターン
		int unit = INT_MAX/_indexSz;
		while(1){
			for(size_t i=0;i<_a2i.size();i++){
				_a2i[i]=1+ (*_generator)()/unit;
			}
			if(valid_init()){ break; }
		}
	}
	_updated=true;
}

//--------------------------------------------------------------------
//	予測インデックストータル件数取得
//--------------------------------------------------------------------
size_t AIndex::total_idxcnt(void){
	if(_ordered){ return _a2i.size(); }
	return _a2i.size()*(_indexSz-1);
}

void AIndex::show(){
	for(int i=0 ; i<_a2i.size();i++){ 
		cerr << _a2i[i];
	}
	//cerr << endl;
}


//--------------------------------------------------------------------
//	アイテム番号noにインデックスiをセット(&変更フラグをセット)
//--------------------------------------------------------------------
void AIndex::setIdx(size_t no , size_t idx){
	_a2i[no]=idx;
	_updated=true;
}

//------------------------------------------------------
// targetインデックスを更新する。
// インデックス値を１ずつupしていく
// 一周した場合にはtrueを返す。（文字タイプ用）
//------------------------------------------------------
bool AIQueue::roundupIndex(void){
	bool carryOver = false;
	size_t newidx = targetIdx()+1 ;
	if(newidx > targetAI()->idxSz()){ newidx=1;} // 初期値に
	if(newidx==_AI_nowItmOrg) {  carryOver=true; }
	updateIdx(newidx);
	return carryOver;
}

//------------------------------------------------------
// targetインデックスを更新する。
// 前後のインデックス値を見て、
// オリジナルの値と違えば更新する（数字タイプ用）
// 123 -> 133 ,113
//------------------------------------------------------
bool AIQueue::roundupIndex_order(void)
{
	bool carryOver = false;
	// 前Index値
	size_t prv = _AI_nowItmOrg;
	if(_AI_nowItmPos!=0){ 
		prv = targetAI()->idxOf(_AI_nowItmPos-1);
	}

	// 次Index値
	size_t next = _AI_nowItmOrg;
	if(_AI_nowItmPos+1 < targetAI()->itmSz()){ 
		next = targetAI()->idxOf(_AI_nowItmPos+1);
	}

	// 111の場合	
	if(prv == next){ carryOver = true; }
	// 112の場合	
	else if (prv == _AI_nowItmOrg){
		if( _AI_nowItmOrg == targetIdx()){
			_fin_idxcnt++; 
			updateIdx(next);
		}
		else{
			updateIdx(_AI_nowItmOrg);
			carryOver= true;
		}
	}
	// 122の場合
	else if( next == _AI_nowItmOrg){
		if(_AI_nowItmOrg == targetIdx()) {
			_fin_idxcnt++;
			updateIdx(prv);
		}
		else{
			updateIdx(_AI_nowItmOrg);
			carryOver= true;
		}
	}
	// 123の場合
	else{
		if(_AI_nowItmOrg == targetIdx() ){
			updateIdx(next);
		}
		else if(_AI_nowItmOrg==next){
			updateIdx(prv);
		}
		else{
			updateIdx(_AI_nowItmOrg);
			carryOver= true;
		}
	}
	return carryOver;
}

//------------------------------------------------------
// 更新対象のアルファベットを次の位置に移動させる。
// targetAIno_, targetAlphabet_, targetOrgIndex_を更新する。
// 終端であればtrueを返す。
//------------------------------------------------------
bool AIQueue::moveToNextAlphabet(void){
	bool isTerminal= false;
	_AI_nowItmPos++; 
	// alphabetの終端に達していなければ、オリジナルのalphabetを覚えておく
	if(_AI_nowItmPos < targetAI()->itmSz()){
		_AI_nowItmOrg = targetIdx();
	}
	// alphabetの終端に達していれば
	else{
		_AI_nowItmPos = 0;             // alphabetは0に戻し
		_AI_nowNo++;                   // 次のAIに移動する
		// AIが終端でなければ
		if(_AI_nowNo < _ai.size()){
			_AI_nowItmOrg = targetIdx() ; // オリジナルのalphabetを覚えておく
		}
		// AIが終端であれば
		else{
			_AI_nowNo = 0;                // 0クリアして
			_AI_nowItmOrg = 0;
			isTerminal = true;            // isTerminalフラグをonに
		}
	}
	return isTerminal;
}

//------------------------------------------------------
// ローカルサーチにて次のアルファベットインデックスを生成する。
// ローカルの変更がすべて終わればfalseを返す。
//------------------------------------------------------
bool AIQueue::updNextIndexSub(void)
{
	bool notTerminal = true;
	bool carryOver;

	if(targetAI()->noidx()) return false;

	if(targetAI()->ordered()){
		carryOver = roundupIndex_order();
	}
	else{
		carryOver = roundupIndex();
	}
	if(carryOver){
		if( moveToNextAlphabet()){ notTerminal=false;}
		else                     { notTerminal=updNextIndexSub();}
	}
	return notTerminal;
}

//------------------------------------------------------
// インデックス更新
//------------------------------------------------------
bool AIQueue::updNextIndex(void)
{
 	_fin_idxcnt++;
	if(_ai.size()==0){ return false;}
		
	bool notTerminal=true;
	if( _first_idx){
		_AI_nowItmOrg = targetIdx();
		_first_idx=false;
	}
	else{
		while(notTerminal){
			notTerminal = updNextIndexSub();
			if(targetAI()->valid()){ break;}
			_fin_idxcnt ++;
		}
	}
	return notTerminal;
}

//------------------------------------------------------
// ベターインデックス保持
//------------------------------------------------------
void AIQueue::stockBetterIndx(void){
	if( _ai.size()==0 ) { return ;}
	_bestIndex = _AI_nowNo;
	_bestPos   = _AI_nowItmPos;
	_bestValue = targetIdx();
}

void AIQueue::setBetterIndx(void){
	_ai[_bestIndex]->setIdx(_bestPos,_bestValue);
	_AI_nowNo = 0; 
	_AI_nowItmPos = 0;
	_bestIndex		= 0;
	_bestPos		  = 0;
	_bestValue		= 0;
	_fin_idxcnt   = 0;
	_ttl_idx      = 0;
	_AI_nowItmOrg = targetIdx();
	for(int i=0 ; i<_ai.size() ; i++ ){
		_ttl_idx += _ai[i]->total_idxcnt();
	}
}
