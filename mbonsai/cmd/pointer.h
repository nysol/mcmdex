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
#include<vector>
//======================================================
// pointer
//======================================================
class Pointer
{
	size_t _size;
	vector<int>	_lNumber;
	vector<double> _lRatio;
public:
	Pointer(int sz):_size(sz){
		_lNumber.resize(0);
		_lRatio.resize(0);
	}

	void push(int no ,double ratio){
		_lNumber.push_back(no);
		_lRatio.push_back(ratio);
	}

	int atLnum(int i){ return _lNumber.at(i); }

	double atLratio(int i){ return _lRatio.at(i); }

	size_t size(){ return _size;}

	void show(void){
		for(int i=0 ; i<_size ; i++){
			cerr << "d " <<  atLnum(i) << " " <<atLratio(i) << " " << _lNumber.size() << " " << _size << endl; 
		}
	}
};		

