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

#include "flditem/numitem.h"

using namespace kglib ;

#define showIndent(indent) for(int ind=0; ind<indent; ind++) printf(" ");
void NumericItem::show(int indent){
	showIndent(indent);
	printf("data:\n");
	showIndent(indent+2);

	printf("%s: ",this->name().c_str() );
	for(size_t i=0 ; i<_trainingDat.size() ; i++ ){
		printf("%s,", _trainingDat[i].toStr().c_str());
  }
	printf("\n");
}

//----------------------------------------------------------------
// データ読み込み
//----------------------------------------------------------------
//void NumericItem::read(char** val,bool dset)
void NumericItem::read(char** val)
{
	if(*val[fldnum()]=='\0'){
		_trainingDat.push_back(ValtypeDbl(true));
//		if(dset){	_testDat.push_back(ValtypeDbl(true));}
  }else{
		_trainingDat.push_back(ValtypeDbl(atof(val[fldnum()])));
//		if(dset){	_testDat.push_back( ValtypeDbl(atof(val[fldnum()]) ) ); }
	}
}
//void NumericItem::readT(char** val){
//	if(*val[fldnum()]=='\0'){
// 		_testDat.push_back(ValtypeDbl(true));
//	}
//	else{
//		_testDat.push_back(ValtypeDbl(atof(val[fldnumT()]) ));
//	}
//}
