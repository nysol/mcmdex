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
#include <ostream>
#include <kgCSV.h>

#include "flditem.h"
#include "itemmap.h"
#include "valtype/valtypevec.h"
#include "flditem/clsitem.h"
#include "regptn.h"
#include "ptnParam.h"

namespace kglib {

class PatternItem : public fldItem
{
	vector<ValtypeVec> _trainingDat; // ValtypeVec:整数の可変長vector
	//vector<ValtypeVec> _testDat;
	vector<ValtypeVec> _idxDat;

	vector< vector<bool> > _attDat;
	vector<RegPtn*> _regTbl;
	vector<RegPtn*> _regTbl_better;
	vector<RegPtn*> _regTbl_best;

	ptnParam _param;
	itemMap _itemMap;
	int			_regCnt;
	AIndex  *_aindex;
	vector<size_t> _best_aindex;
	bool			_noidx; // インデックスサイズを0と指定した場合true
	
	void regclean(int type=0){
		if(type==1){
			for(size_t i=0; i< _regTbl_better.size();i++){
				delete _regTbl_better.at(i);
			}
			_regTbl_better.clear();		
			for(size_t i=0; i< _regTbl_best.size();i++){
				delete _regTbl_best.at(i);
			}
			_regTbl_best.clear();		
		}	
		for(size_t i=0; i< _regTbl.size();i++){
			delete _regTbl.at(i);
		}
		_regTbl.clear();
	}

public:
	PatternItem(void):_noidx(false){}
	PatternItem(ptnParam param) : _param(param),_noidx(false){}
	//PatternItem(int fldnum,int fldnumT,kgstr_t fldname ,ptnParam param,int regC):
	//							fldItem(fldnum,fldnumT,fldname), _param(param),_regCnt(regC),_aindex(NULL){}
	PatternItem(int fldnum,kgstr_t fldname ,ptnParam param,int regC):
								fldItem(fldnum,fldname), _param(param),_regCnt(regC),_aindex(NULL),_noidx(false){}

	~PatternItem(void){
		regclean(1);
		if(_aindex!=NULL){delete _aindex;}
	}
	
	int  begin(void)    const { return _param.begin();}
	int  end(void)      const { return _param.end();}
	int  indexLen(void) const { return _param.indexLen();}
	bool subseq  (void) const { return _param.isSubSeq();}
	bool ordered (void) const { return _param.isOrder();}
	size_t idxSz (void) const { return _param.indexSize();}
	size_t itmSz (void) const { return _itemMap.size();}
	bool   noidx(void)  const { return _noidx;}

	void noidxOn(void){
		_noidx=true;
		_param.setIndexSize(_itemMap.size());
	}

	AIndex * setAindex(boost::variate_generator< boost::mt19937,boost::uniform_int<>  > * rfunc){
		_aindex = new AIndex(idxSz() ,ordered(),itmSz(),_noidx,rfunc);
		return _aindex;
	}	
	void init_aindex(void){ _aindex->initial_set(); }	
	
	void betterSet(){
		_regTbl_better = _regTbl;
		_regTbl.clear();
	}
	void bestSet(){
		_regTbl_best = _regTbl_better;
		_regTbl_better.clear();
		_best_aindex = _aindex->getidx();
	}


	RegPtn* regInfo(int i){return _regTbl[i];}
	void init_read(char** val);
	void itemSet(vector<kgstr_t>& ilist);
	void idxSet(map<kgstr_t,int>& ilist);


	//void init_readT(char** val);
	//void read(char** val,bool dset);
	void read(char** val);
	//void readT(char** val);
	void make_idx(AIndex * ai);
	void make_regexp(const ClsItem* cls);
	vector< vector<bool> >& att(void){ return  _attDat;}

	kgstr_t id2item(int no){ return _itemMap.id2item(no); } 
	size_t id2bestAidx(int no){ return _best_aindex.at(no); } 


	//bool null(int pos,bool type=true){ 
	//	if(type){ return _trainingDat.at(pos).null(); }
	//	else		{ return _testDat.at(pos).null(); }
	//}
	bool null(int pos){ 
		return _trainingDat.at(pos).null();
	}

	//bool isMatch(baseInfo base,int pos,bool type=true);
	bool isMatch(baseInfo base,int pos);

	//bool chkMatch(baseInfo base,int pos,bool type=true);
	bool chkMatch(baseInfo base,int pos);

	// output 
	void printIdx(ostream* ofp=&cout);
	void condPrint(baseInfo base, ostream* ofp=&cout);
	void writePMML(FILE *fp,baseInfo  base ,int level=0,char trl='t');

	void show(int indent=0);

};
}
