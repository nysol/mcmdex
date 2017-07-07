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
#include<map>
#include<kgmod.h>

namespace kglib {

class itemMap
{
	vector<kgstr_t>  _num2str;
	map<kgstr_t,int> _str2num;
	vector<int>      _num2cnt;

	bool _cntF;

	public:
		itemMap(bool cntf=false):_cntF(cntf){}

		void push(kgstr_t val);
		void push(char* val,char delim);
		int item2id(char *val)const;
		vector<int> item2id(char *val,char delim);

		kgstr_t id2item(int id) const { return _num2str.at(id);}
		int  count(int id) const { return _num2cnt.at(id);}


		int size() const { return _num2str.size();}
};

}