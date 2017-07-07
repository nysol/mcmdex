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

using namespace std;

namespace kglib {

class Cost
{
	vector< vector<double> > _tbl;
	vector<double> _actTtl;
	int size ;
	
	public:
	
	Cost(void){};
	
	void initSet(int sz);
	void update(void);

	void set(int fr ,int to ,double val){ _tbl.at(fr).at(to)=val; }
	double getTTL(int i) const { return _actTtl.at(i); }
	double get(int i,int j) const { return _tbl[i][j]; }

};

}