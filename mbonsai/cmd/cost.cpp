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
#include "cost.h"

using namespace kglib ;


void Cost::initSet(int sz){
	size=sz;
	_tbl.resize(sz);
	_actTtl.resize(sz);
	for(size_t i=0; i<sz; i++){ _tbl.at(i).resize(sz); }
	for(size_t i=0; i<sz; i++){
		for(size_t j=0; j<sz; j++){
			if(i==j){ _tbl.at(i).at(j)=0; }
			else    { _tbl.at(i).at(j)=1; }
		}
	}
}

void Cost::update(void){
	for(size_t i=0; i<size; i++){
		_actTtl.at(i)=0;
		for(size_t j=0; j<size; j++){
			_actTtl.at(i)+=_tbl.at(i).at(j);
		}
	}
}
