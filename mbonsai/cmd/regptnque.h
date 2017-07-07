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
#include <queue>
#include "regptn.h"
#include "flditem/clsitem.h"
#include "condition.h"

namespace kglib {

struct rpSort{
	bool operator()(RegPtn* q1,RegPtn* q2) const {
		return *q1<*q2 ;
	}
};

struct vecSort{
	bool operator()(RegPtn* q1,RegPtn* q2) const {
		return q1->ptn()<q2->ptn() ;
	}
};


// 正規パターン探索キュー（二つのプライオリティキューをもつ探索キュー)
class RegPtnQueue {
	// pqTop   : 候補となる正規表現を入れておくpriority queue
	// regBest : 候補となるn個の正規patternを、svの低い順に蓄える
	// fobidden: 改善の見込みのない正規patternを蓄えておくハッシュ
	priority_queue<RegPtn*, vector<RegPtn*>, rpSort> _regBest;
	priority_queue<RegPtn*, vector<RegPtn*>, rpSort> _pqTop;
	set<RegPtn*,vecSort> _forbidden;

	int _queMax; // 候補パターン最大数(cand=パラメータ)
	Condition _condition;

	RegPtn* _maxVal;
	
	static int MaxRegLen;

public:
	RegPtnQueue(int max,const ClsItem* cls):_queMax(max),_condition(cls),_maxVal(NULL){}
	~RegPtnQueue(void){
		for (set<RegPtn*,vecSort>::iterator it=_forbidden.begin(); it!=_forbidden.end(); ++it){
			delete *it;
		}
		_forbidden.clear();
	}

	RegPtn* top(void){ return _pqTop.top();}
	RegPtn* topBest(void){ return _regBest.top();}

	bool empty(void)		 { return  _pqTop.empty();}

	RegPtn* pop(void);
	RegPtn* popBest(void);
	bool isFBDN(RegPtn* regptn);
	
	bool pushBest(RegPtn* regptn);
	void pushReg(RegPtn* regptn,vector<ValtypeVec>& idxdat);
	
	bool isUnderMax(RegPtn* reg){
		return 	_regBest.size()<_queMax || reg->ub() <= _maxVal->ub(); 
	}


	size_t sizex(void){ return _pqTop.size();}

	size_t size(void){ return _regBest.size();}
};

}
