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
namespace kglib {
// -------------------------------------------
//  double データ型
// -------------------------------------------
class ValtypeDbl
{
		double _val;
		bool   _null;

	public:
		ValtypeDbl(void)   : _val(-1),_null(true){}
		ValtypeDbl(double x)  : _val(x),_null(false){}
		ValtypeDbl(bool n) :_val(-1),_null(n) {}
		ValtypeDbl(double x,bool n) :_val(x),_null(n) {}

		// ------------------------------------------------------
		// アクセッサ
		// ------------------------------------------------------
		bool   null(void)    const { return _null;}
		double val(void)     const { return _val; }
		void   null(bool n)  { _null=n;}
		void   val(double x) { _val=x; }

		string toStr(void){ 
			ostringstream ss;
			//printf("dbl(null=%d):",_null);
			ss << _val;
			return ss.str();
		}

	};
}
