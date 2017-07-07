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

#include "flditem.h"
#include "valtype/valtypedbl.h"

namespace kglib {

class NumericItem : public fldItem
{
	vector<ValtypeDbl> _trainingDat;
	//vector<ValtypeDbl> _testDat;

public:
	NumericItem(void){}
	//NumericItem(int fldnum,int fldnumT,kgstr_t fldname):fldItem(fldnum,fldnumT,fldname){}
	NumericItem(int fldnum,kgstr_t fldname):fldItem(fldnum,fldname){}

	//----------------------------------------------------------------
	// データ読み込み
	//----------------------------------------------------------------
	//void read(char** val,bool dset);
	void read(char** val);
	//void readT(char** val);
	//bool null(int pos,bool type=true){ 
	//	if(type){ return _trainingDat[pos].null(); }
	//	else		{ return _testDat[pos].null(); }
	//}

	bool null(int pos){ return _trainingDat[pos].null(); }

	//double getVal(int x ,bool type=true)  const  { 
	double getVal(int x)  const  { 
		//if(type){ return _trainingDat[x].val(); }
		//else 		{ return _testDat[x].val(); }
		return _trainingDat[x].val();
	}

	//bool isMatch(baseInfo base,int pos,bool type=true){ 
	bool isMatch(baseInfo base,int pos){ 
		//return getVal(pos,type) <= base.d() ;
		return getVal(pos) <= base.d() ;
	}

	//bool chkMatch(baseInfo base,int pos,bool type){ 
	bool chkMatch(baseInfo base,int pos){ 
		//return isMatch( base, pos , type);
		return isMatch( base, pos);
	}

	void condPrint(baseInfo base , ostream* ofp=&cout){
		*ofp << "if($" << name() << " <= " << base.d() << " )" << endl;
//		printf( "if($%s <= %g )\n" , name().c_str() , base.d() );
	}

	void writePMML(FILE *fp,baseInfo  base ,int level=0,char trl='t'){
		tabPrint(fp,level+2);
		if( trl=='m'){
			fprintf(fp, "<SimplePredicate field=\"%s\" operator=\"lessOrEqual\" value=\"%g\"/>\n" 
								, name().c_str() , base.d());
		}
		else{	
			fprintf(fp, "<SimplePredicate field=\"%s\" operator=\"greaterThan\" value=\"%g\"/>\n" 
								,name().c_str() , base.d());
		}
	}

	void show(int indent=0);

};
}
