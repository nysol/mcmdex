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
#include <kgmod.h>
#include <cstdio>
#include "baseinfo.h"
namespace kglib {

class fldItem {
	int _fldnum;
	kgstr_t _fldname;

	public:
		fldItem(void):_fldnum(0){}
		fldItem(int fldnum,kgstr_t fldname):_fldnum(fldnum),_fldname(fldname){}
		virtual ~fldItem(){}
		
		kgstr_t name(void){ return _fldname;}
		void tabPrint(FILE* fp,int level){ 
			for(int i=0 ; i<level ; i++){ fprintf(fp,"\t"); }
		}
		
		virtual void init_read(char** val){};
		virtual void read(char** val){};
		virtual void show(int indent){};
		virtual size_t itmSz(void)const{ return 0; }

		virtual int fldnum(void) { return _fldnum; }
		virtual void fldnum(int fn1) { _fldnum=fn1;}
		virtual void fldnum(int fn1,kgstr_t fldname) { _fldnum=fn1; _fldname=fldname;}
		
		virtual bool null(int pos){ return true;}	
		virtual bool isMatch(baseInfo  baseval,int pos){return true;}	
		virtual bool chkMatch(baseInfo  baseval,int pos){return true;}	
		virtual void condPrint(baseInfo  baseval, ostream* ofp){};
		virtual void writePMML(FILE *fp,baseInfo  baseval ,int level,char trl){}
};
}
