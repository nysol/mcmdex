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
#include "itemmap.h"
#include "valtype/valtypeint.h"
#include "pointer.h"

namespace kglib {

class CategoryItem : public fldItem
{
	vector<ValtypeInt> _trainingDat;
	vector<ValtypeInt> _testDat;
	itemMap _itemMap;

	void setCatCombi(void){
		for(int i=0 ; i < 1024 ; i++){
			for(int j=0 ; j < 10 ; j++){ catcombi[i][j]='U'; }
		}
		for(int i=0 ; i<10 ; i++ ){
			int cnt=0;
			for(int j=0 ; j < loop[i] ; j++){
				for(int k=0 ; k < rui[i] ; k++){
					catcombi[cnt][i]='M';
					cnt++;
				}
				for(int k=0 ; k < rui[i] ; k++){
					catcombi[cnt][i]='U';
					cnt++;
				}
			}
		}
	}

public:
	static const int end[];
	static const int rui[];
	static const int loop[];
	char catcombi[1024][10];


	CategoryItem(void){ setCatCombi();}
	//CategoryItem(int fldnum,int fldnumT,kgstr_t fldname):fldItem(fldnum,fldnumT,fldname){ setCatCombi();}
	CategoryItem(int fldnum,kgstr_t fldname):fldItem(fldnum,fldname){ setCatCombi();}

	void init_read(char** val);
	void itemSet(vector<kgstr_t>& ilist);

	//void init_readT(char** val);

	//void read(char** val,bool dset);
	void read(char** val);
	//void readT(char** val);

	kgstr_t id2item(int no){ return _itemMap.id2item(no); } 
	int item2id(char* str) const{ return _itemMap.item2id(str); } 

	size_t itmSz (void)const { return _itemMap.size();}

//	int getVal(int x, bool type=true)  const  { 
//		if(type){ return _trainingDat[x].val(); }
//		else 		{ return _testDat[x].val(); }
//	}
	int getVal(int x)  const  { 
		return _trainingDat[x].val();
	}
//	bool null(int pos,bool type=true){ 
//		if(type){ return _trainingDat.at(pos).null(); }
//		else		{ return _testDat.at(pos).null(); }
//	}

	bool null(int pos){ 
		return _trainingDat.at(pos).null();
	}


/*	bool isMatch(baseInfo base,int pos,bool type=true){ 
		return base.c().at(getVal(pos,type)) == 'M';
	}
	bool chkMatch(baseInfo base,int pos,bool type){
		return isMatch( base, pos, type);
	}
	*/
	bool isMatch(baseInfo base,int pos){ 
		return base.c().at(getVal(pos)) == 'M';
	}
	bool chkMatch(baseInfo base,int pos){
		return isMatch( base, pos);
	}



	void condPrint(baseInfo base, ostream* ofp=&cout){

		*ofp << "if($" << name() << " is in {";
	
		vector<char> lst = base.c();
		for(int i=0 ; i<lst.size() ; i++){
			if(lst[i]=='M'){ 
				*ofp << _itemMap.id2item(i) << ",";
			}
		}
		*ofp << "}" << endl;
	}

	void writePMML(FILE *fp,baseInfo  base ,int level=0,char trl='t')
	{
		// catlistでマッチするものが１つかそれ以上の場合出力形式がかわる		
		vector<kgstr_t> nList ;

		vector<char> lst = base.c();
		for(int i=0 ; i<lst.size() ; i++){
			if(lst[i]=='M'){ nList.push_back( id2item(i) ); }
		}
		tabPrint(fp,level+2);
		if(trl=='m'){ fprintf(fp,"<SimpleSetPredicate field=\"%s\" booleanOperator=\"isIn\">\n",name().c_str()); }
		else				{ fprintf(fp,"<SimpleSetPredicate field=\"%s\" booleanOperator=\"isNotIn\">\n",name().c_str()); }

		tabPrint(fp,level+3);
    fprintf(fp,"<Array n=\"%lu\" type=\"string\">",nList.size());

		for(int i=0 ; i<nList.size() ; i++ ){
			if(i==0){ fprintf(fp,"\"%s\"",nList[i].c_str()); }
			else    { fprintf(fp," \"%s\"",nList[i].c_str()); }
		}
    fprintf(fp,"</Array>\n");
		tabPrint(fp,level+2);
		fprintf(fp,"</SimpleSetPredicate>\n"); 
	}
	
	void show(int indent=0);

	};

}

