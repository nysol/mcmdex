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
#include "valtype/valtypevec.h"

namespace kglib {

class RegPtn 
{
	ValtypeVec _ptn;
	bool	 _type; // true:subseq,false:substring 
	int 	 _bgn;
	int 	 _end;
	double _val;
	double _ub;
	
	bool le_lt(const RegPtn& other,bool eql);	
	bool regCmpSeq(ValtypeVec &idx);
	bool regCmpStr(ValtypeVec &idx);
	bool atEndRng(ValtypeVec &idx);


	public:
	RegPtn(void):_type(false),_bgn(0),_end(0),_val(0),_ub(0){}
	RegPtn(ValtypeVec ptn,bool type=false,
				int bgn=0,int end=0,double val=0,double ub=0)
		:_ptn(ptn),_type(type),_bgn(bgn),_end(end),_val(val),_ub(ub){}

	RegPtn(RegPtn *regptn ,bool deep=false)
		:_ptn(regptn->_ptn),_type(regptn->_type),_bgn(regptn->_bgn),_end(regptn->_end){
		if(deep){_val=regptn->_val; _ub=regptn->_ub;}
		else{ _val=0; _ub=0;}	
	}

	
	//アクセッサ
	size_t size(void){ return _ptn.size();}
	void val(double v){ _val = v;}
	void ub (double v){ _ub  = v;}

	bool type(void){ return _type; }

	int bgn(void){ return _bgn; }
	int end(void){ return _end; }
	double ub (void){ return _ub;}
	double val(void){ return _val;}
	ValtypeVec ptn(void){ return _ptn;}
	int ptnAt(int i){ return _ptn.at(i); } 
	
	bool isMatch(ValtypeVec &idx){ return regCmp(idx); }
	bool regCmp(ValtypeVec &idx){
		bool rtn;
		if(_type)	{ rtn = regCmpSeq(idx); }
		else			{ rtn = regCmpStr(idx); }
		return rtn;
	}

	bool operator<=(const RegPtn& other){
		return le_lt(other,true);
	}
	bool operator<(const RegPtn& other){
		return le_lt(other,false);
	}
	void addPtn(int x);
	RegPtn *right(int i);
	string to_s() const { return _ptn.to_s();} 

	//void show(void){
	//	fprintf(stderr,"Regptn:");
	//	_ptn.show();
	//	fprintf(stderr,"_type=%d, _bgn=%d, _end=%d, _val=%g, _ub=%g",_type,_bgn,_end,_val,_ub);
	//}

	string toStr(void){
		ostringstream ss;
		ss << _ptn.toStr();
		ss << "(";
		ss << _type << ",";
		ss << _bgn  << ",";
		ss << _end << ",";
		ss << _val << ",";
		ss << _ub << ")";
		return ss.str();
	}
};





}
