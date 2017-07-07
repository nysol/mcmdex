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
#include "regptn.h"

namespace kglib {

class baseInfo
{
	double            _baseval_d; 
	std::vector<char> _baseval_c;
	int               _baseval_i;
	RegPtn*           _baseval_r;

	public:
	baseInfo(void){}

	void set(double val){ _baseval_d = val;}
	void set(std::vector<char> val){ _baseval_c = val;}
	void set(int val){ _baseval_i = val;}
	void set(RegPtn* val){ _baseval_r = val;}

	double			 d(void){ return _baseval_d;}  
	std::vector<char> c(void){ return _baseval_c;}  
	int					 i(void){ return _baseval_i;}  
	RegPtn*			 reg(void){ return _baseval_r;}  
};

}
