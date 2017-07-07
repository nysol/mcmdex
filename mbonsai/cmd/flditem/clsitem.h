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
#include <kgCSV.h>
#include <boost/random.hpp>

#include "flditem.h"
#include "itemmap.h"
#include "valtype/valtypeint.h"
#include "cost.h"

namespace kglib {

class ClsItem : public fldItem
{
	vector<ValtypeInt> _trainingDat;
	vector< vector<int> >  _separate;

	itemMap _itemMap;
	Cost		_cost;

public:
	ClsItem(void):_itemMap(true){}
	//ClsItem(int fldnum,int fldnumT,kgstr_t fldname):fldItem(fldnum,fldnumT,fldname){}
	ClsItem(int fldnum,kgstr_t fldname):fldItem(fldnum,fldname),_itemMap(true){}
	
	void costSet(kgCSVfld &cfile);
	void init_read(char** val);
	void read(char** val);


	void itemSet(vector<kgstr_t>& ilist);


	int getVal(int x) const { return _trainingDat[x].val();}

	size_t itmSz (void) const { return _itemMap.size();}
	double getTTLCost(int no)const;
	double getCost(int i,int j)const{ return _cost.get(i,j); }
	size_t traSz(void){ return _trainingDat.size(); }

	kgstr_t id2item(int no){ return _itemMap.id2item(no); } 
	int item2id(char* str) const{ return _itemMap.item2id(str); } 
	void dataSplit(int type,double sep,size_t seed);
	int getsplitData(int i,int j){ return  _separate.at(i).at(j);}
	int getsplitSize(int i){ return  _separate.at(i).size();}

};
}
