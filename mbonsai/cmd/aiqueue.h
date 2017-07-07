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
#include <vector>
#include <boost/random.hpp>

using namespace boost;
using namespace std;

namespace kglib {
class AIndex {

	vector<size_t> _a2i; // vectorのポジションはalphabet番号
	size_t _indexSz;
	bool   _ordered;
	bool   _updated;     // indexが更新されたときonになり、データに反映されるとoffになる。
	bool   _noidx;
	boost::variate_generator< boost::mt19937,boost::uniform_int<> > *_generator;

	// ----------------------------------------------------------------
	// インデックスのサイズ分の種類を全て用いてるインデックスかチェック
	// ----------------------------------------------------------------
	bool valid_init();

public:

	AIndex(size_t idx_size,bool order,size_t item_size,bool noidx,
					boost::variate_generator< boost::mt19937,boost::uniform_int<> > * rfunc) : 
		_indexSz(idx_size),_ordered(order), _updated(false), _noidx(noidx), _generator(rfunc)
	{
		_a2i.resize(item_size,0);
	}
	size_t idxSz(void)  { return _indexSz;}
	size_t itmSz(void)  { return _a2i.size();}
	bool ordered(void)  { return _ordered; }
	bool updated(void)  { return _updated; }
	bool noidx(void)    { return _noidx; }
	void updated(bool n){ _updated=n; }

	vector<size_t> getidx(void){ return _a2i;}

	bool valid(void);
	void initial_set(void);
	void setIdx(size_t no , size_t idx);
	
	//--------------------------------------------------------------------
	//	アイテム番号noのインデックス値参照
	//--------------------------------------------------------------------
	size_t idxOf(size_t no){ return _a2i.at(no); }

	size_t total_idxcnt(void);
	void show();

};


class AIQueue {
	vector<AIndex*> _ai;

	size_t _AI_nowNo;     // 変更中aindex No.
	size_t _AI_nowItmPos; // 変更中aindex内位置
	size_t _AI_nowItmOrg; // 変更前index値

	size_t _bestIndex;			// 最適変更中aindex No.
	size_t _bestPos;  // 最適aindex内変更位置
	size_t _bestValue;

	bool   _first_idx;			// index更新初回フラグ
	size_t _fin_idxcnt;			//　現在チェック済みindex件数
	size_t _ttl_idx;				// 予測トータルindex件数

	//------------------------------------------------------
	// ・更新中aindex取得
	// ・更新中インデックス値取得
	// ・インデックス値更新
	//------------------------------------------------------
	AIndex* targetAI(void) { return _ai.at(_AI_nowNo); }
	size_t  targetIdx(void){ return targetAI()->idxOf(_AI_nowItmPos); }
	void updateIdx(size_t idx) { targetAI()->setIdx(_AI_nowItmPos,idx); }

	bool roundupIndex(void);
	bool roundupIndex_order(void);
	bool moveToNextAlphabet(void);
	bool updNextIndexSub(void);

public:
	AIQueue(void) :  
		_AI_nowNo(0), _AI_nowItmPos(0), _AI_nowItmOrg(1), 
		_bestIndex(0), _bestPos(0), _bestValue(0),
		_first_idx(true),_fin_idxcnt(0) ,_ttl_idx(1)
		{}

	void reset(){
		_AI_nowNo=0;
		_AI_nowItmPos=0;
		_AI_nowItmOrg=1; 
		_bestIndex=0;
		_bestPos=0;
		_bestValue=0;
		_first_idx=true;
		_fin_idxcnt=0;
		_ttl_idx=1;
	}
	bool empty(void){ return _ai.size()==0 ; }
	
	void add(AIndex* ai)  { _ai.push_back(ai);}

	AIndex* get(size_t no){ return _ai.at(no);}
	
	bool updNextIndex(void);
	void stockBetterIndx(void);
	void setBetterIndx(void);
	void showIndex(void){
		for(int i=0 ; i<_ai.size();i++){
			_ai.at(i)->show();
			cerr << " ";
		}
	}	


};

}
