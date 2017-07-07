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

#include "flditem/catitem.h"

using namespace kglib ;


const int	CategoryItem::end[] ={2,4,8,16,32,64,128,256,512,1024};
const int	CategoryItem::rui[] ={1,2,4,8,16,32,64,128,256,512};
const int	CategoryItem::loop[]={512,256,128,64,32,16,8,4,2,1};

//----------------------------------------------------------------
// アイテム読み込み
// NULL値は登録しない
//----------------------------------------------------------------
void CategoryItem::init_read(char** val)
{
	if(*val[fldnum()]!='\0'){	_itemMap.push(val[fldnum()]);}
}
void CategoryItem::itemSet(vector<kgstr_t>& ilist)
{
	for(size_t i=0 ; i<ilist.size() ; i++){
		_itemMap.push(ilist[i]);
	}
}


//void CategoryItem::init_readT(char** val)
//{
//	if(*val=='\0'){ _itemMap.push(val[fldnumT()]);}
//}
//----------------------------------------------------------------
// データ読み込み
//----------------------------------------------------------------
//void CategoryItem::read(char** val,bool dset)
void CategoryItem::read(char** val)
{
	if(*val[fldnum()]=='\0'){
		_trainingDat.push_back(ValtypeInt(true));
//		if(dset){	_testDat.push_back(ValtypeInt(true));}
  }else{
  	int id = _itemMap.item2id(val[fldnum()]);
		_trainingDat.push_back(ValtypeInt(id));
//		if(dset){	_testDat.push_back(ValtypeInt(id));}
	}
}
//void CategoryItem::readT(char** val){
//	if(*val[fldnum()]=='\0'){
// 		_testDat.push_back(ValtypeInt(true));
// 	}
//	else{
//		int id = _itemMap.item2id(val[fldnumT()]);
//		_testDat.push_back(ValtypeInt(id));
// 	}

#define showIndent(indent) for(int ind=0; ind<indent; ind++) printf(" ");
void CategoryItem::show(int indent){
	showIndent(indent);
	printf("data:\n");

	showIndent(indent+2);
	printf("%s: ",this->name().c_str() );
	for(size_t i=0 ; i<_trainingDat.size() ; i++ ){
		printf("%s,", _trainingDat[i].toStr().c_str());
  }
	printf("\n");
}

