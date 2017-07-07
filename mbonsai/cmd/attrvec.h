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

#include "flditem/clsitem.h" 
#include "flditem/catitem.h" 
#include "flditem/numitem.h"
#include "flditem/patitem.h"
#include "flditem.h"
#include "aiqueue.h"
#include "node.h"
#include "condition.h"
#include "pointer.h"

namespace kglib {

class AttrVec
{
	vector<CategoryItem *> _catitem;
	vector<NumericItem  *> _numitem;
	vector<PatternItem  *> _patitem;
	int	_attritemC;

	kgAutoPtr1<boost::variate_generator< boost::mt19937,boost::uniform_int<>  > > _rand_mod;


	public:
	AttrVec(void):_attritemC(0){}
	~AttrVec(void){
		for(size_t i=0;i<_catitem.size();i++){ delete _catitem[i];}
		for(size_t i=0;i<_numitem.size();i++){ delete _numitem[i];}
		for(size_t i=0;i<_patitem.size();i++){ delete _patitem[i];}
	}
	int size(void){ return _attritemC; }
	void setSeed(size_t seed){ 
		_rand_mod.set( new variate_generator< mt19937,uniform_int<> >
				(mt19937((uint64_t)seed),uniform_int<> (0,INT_MAX)) );
	}

	// データセット
	void push(CategoryItem * fld);
	void push(NumericItem * fld);
	void push(PatternItem * fld);

	// データ読み込み
	void init_read(char **lin);
	void itemSet(map<kgstr_t,vector<kgstr_t> > & ilist);
	void idxSet(map<kgstr_t,map<kgstr_t,int> > & ilist);
	int item2id(kgstr_t name ,kgstr_t idname);
	fldItem * findclass(string name);

	//void init_readT(char **lin);
	//void read(char **lin,bool dset);
	void read(char **lin);
	//void readT(char **lin);
	void adjustIndexSize(void);

	void prerun(void);
	
	// Alphabet index queue関係
	void setAiQue(AIQueue &aique);
	void updAique(AIQueue &aique,ClsItem* _cls);

	// condition set
	fldItem * setCondition(Node* node,Pointer& pointer,ClsItem* _cls);
	void betterSet(void){
		for(size_t i=0;i<_patitem.size();i++){ _patitem[i]->betterSet();}
	}
	void bestSet(void){
		for(size_t i=0;i<_patitem.size();i++){ _patitem[i]->bestSet();}
	}
	// output
	void output(ostream* ofp=&cout);
	void dFldPrint(FILE* fp);
	void infoPrint(FILE* fp);

	void show(int indent=0);
	void showAlphabet(void);
};
}
