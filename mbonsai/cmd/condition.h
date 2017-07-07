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
#include <cmath>
#include <cfloat>
#include "flditem/clsitem.h"

#include "flditem/numitem.h"
#include "flditem/catitem.h"
#include "flditem/patitem.h"
#include "pointer.h"

#include "valtype/valtypevec.h"
#include "regptn.h"
#include "baseinfo.h"

namespace kglib {
// =================================================================
//	set_conditionでセットされる(mCnt,uCntはcalObjValCntでもセットされる)
//	__mCount	match件数+null件数
//	__uCount	unmatch件数+null件数
//	__nCount	null件数
//	__mCnt		クラスごとmatch件数(null比率込み)
//	__uCnt		クラスごとunmatch件数(null比率込み)
//	__nCnt		クラスごとnull件数(null比率込み)
// ------------------------------------------------
// __calCndCntでセットされる(mmCnt,uuCntは今回null分をmatch,unmatchに付加)
// __tCnt		クラスごと総件数　<= __mCnt+__uCnt+[__nCnt] (初期段階では__nCntは含まない)   
// __mRatio	match比率(null分込み) <=　__mCnt計/(__mCnt計+__uCnt計)
// __mmCnt		クラスごとmatch件数(今回分再計算null分込み) 
//							<= __mCnt +  (__mCnt/__tCnt)*__ncnt
// __uuCnt		クラスごとunmatch件数(今回分再計算null比率込み) 
//							<= __uCnt +  (__uCnt/__tCnt)*__ncnt
//	__mtCnt		match件数(null再計算)		<= __mmCnt計
//	__utCnt		unmatch件数(null再計算)	<= __uuCnt計 
//	__total		総件数 <= __tCnt計 
//	__mtShr		マッチシェア <= __mtCnt/__total
//	__utShr		アンマッチシェア <= __utCnt/__total
//	__mShrRN	マッチでのクラスごと比率<= __mmCnt/__mtCnt
//	__uShrRN	アンマッチでのクラスごと比率<= __uuCnt/__utCnt
//	__tShrRN	全体でのクラスごと比率<=__tCnt/__total
//	__mShrRC	マッチでのクラスごと比率(コスト考慮)
//							<= コスト*__mShrRN/(コスト*__mShrRN)計
//	__uShrRC	アンマッチでのクラスごと比率(コスト考慮)<= __uuCnt/__utCnt
//							<= コスト*__uShrRN/(コスト*__uShrRN)計
//	__tShrRC	全体でのクラスごと比率(コスト考慮)<=__tCnt/__total
//							<= コスト*__tShrRN/(コスト*__tShrRN)計
// ------------------------------------------------
// リンク
//	tShr		(=tShrRC)
//	mShr		(=mShrRC)
//	uShr		(=uShrRC)
// ------------------------------------------------
// splitBefor 枝が生えてない時のvalue <-小さいほうがいい
// splitAfter 枝が生えた時のvalue 　<-小さいほうがいい
// baseval	このconditionの基準値
// ------------------------------------------------
// +---------+--------+--------+---+--------+------+
// |condition|  cls0  |  cls1  |...|  clsN  | Total|
// +---------+--------+--------+---+--------+------+
// |    match| mCnt[0]| mCnt[1]|   | mCnt[N]| mtCnt|
// |         |mmCnt[0]|mmCnt[1]|   |mmCnt[N]|      | mmCnt=mCnt/(mCnt+uCnt)
// +---------+--------+--------+---+--------+------+
// |  unmatch| uCnt[0]| uCnt[1]|   | uCnt[N]| utCnt|
// |         |uuCnt[0]|uuCnt[1]|   |uuCnt[N]|      | uuCnt=uCnt/(mCnt+uCnt)
// +---------+--------+--------+---+--------+------+
// |     NULL| nCnt[0]| nCnt[1]|   | nCnt[N]|      |
// +---------+--------+--------+---+--------+------+
// |    Total| tCnt[0]| tCnt[1]|   | tCnt[N]| total| tCnt=mCnt+uCnt
// |         | tShr[0]| tShr[1]|   | tShr[N]|      |
// +---------+--------+--------+---+--------+------+
//
//
// +---------+-------+-------+---+-------+------+
// |condition| cls0  | cls1  |...| clsN  | Total|
// +---------+-------+-------+---+-------+------+
// |    match|mmCnt[0]|mmCnt[1]|   |mmCnt[N]| mtCnt|
// |         |mShr[0]|mShr[1]|   |mShr[N]| mtShr|
// +---------+-------+-------+---+-------+------+
// |  unmatch|uCnt[0]|uCnt[1]|   |uCnt[N]| utCnt|
// |         |uShr[0]|uShr[1]|   |uShr[N]| utShr|
// +---------+-------+-------+---+-------+------+
// |     NULL|nCnt[0]|nCnt[1]|   |nCnt[N]|      |
// +---------+-------+-------+---+-------+------+
// |    Total|tCnt[0]|tCnt[1]|   |tCnt[N]| total|
// |         |tShr[0]|tShr[1]|   |tShr[N]|      |
// +---------+-------+-------+---+-------+------+
//
// =================================================================

class Condition
{

	const ClsItem* _cls; //_cost = cost
	size_t _clsSz;
	vector<double> _mCnt;
	double _mtCnt;
	vector<double> _uCnt;
	double _utCnt;
	vector<double> _nCnt;

	vector<double> _tCnt;

	vector<double> _tShrRN; 
	vector<double> _tShrRC; 
	vector<double>* _tShr;

	double _total;

	vector<double> _mmCnt; 
	vector<double> _mShrRN;
	vector<double> _mShrRC;
	vector<double>* _mShr;


	vector<double> _uuCnt;
	vector<double> _uShrRN;
	vector<double> _uShrRC;
	vector<double>* _uShr;
	int _mCount;
	int _uCount;
	int _nCount;
	
	double _mRatio;
	double _mtShr;
	double _utShr;
	double _splitBefor;
	double _splitAfter; 
	double _splitgain;
	baseInfo _baseval; 



	static double divDef(double d1, double d2, double def){
 	 if(d2==0) return(def);
  	return(d1/d2);
	}

	static double log2(double val){
		if( val==0 ) { return 0.0; }
		else				 { return log(val)/log(2.0);}
	}

	void calentropy(void);
	void calCndCntSplit(void);
	void calCndCnt(void);

public:

	Condition(const ClsItem *cls):
		_cls(cls),_clsSz(_cls->itmSz()),_mCnt(_clsSz,0),_mtCnt(0),
		_uCnt(_clsSz,0),_utCnt(0),_nCnt(_clsSz,0),_tCnt(_clsSz,0),
		_tShrRN(_clsSz,0),_tShrRC(_clsSz,0),_total(0),_mmCnt(_clsSz,0),
		_mShrRN(_clsSz,0),_mShrRC(_clsSz,0),_uuCnt(_clsSz,0),
		_uShrRN(_clsSz,0),_uShrRC(_clsSz,0),
		_mCount(0),_uCount(0),_nCount(0),_mRatio(0),_mtShr(0),_utShr(0),
		_splitBefor(0),_splitAfter(DBL_MAX),_splitgain(DBL_MAX)
	{
		_tShr=&_tShrRC;
		_mShr=&_mShrRC;
		_uShr=&_uShrRC;
	}
	void reset(void);
	double calUpperBound(void);
	bool calObjValCnt(RegPtn* regptn,vector<ValtypeVec>& idxdat);
	void setCondition(Pointer& pointer , NumericItem* dCls);
	void setCondition(Pointer& pointer , CategoryItem* dCls);
	void setCondition(Pointer& pointer , PatternItem* dCls);
	void copy(Condition& cond);


	//アクセッサ
	vector<double> mCnt(){ return _mCnt;}
	double mCnt(int i){ return _mCnt[i];}

	vector<double> uCnt(){ return  _uCnt;}
	double uCnt(int i){ return _uCnt[i];}
	
	vector<double> nCnt() { return _nCnt;}
	double nCnt(int i) { return _nCnt[i];}

	vector<double> tCnt(){ return _tCnt;}
	double tCnt(int no) { return _tCnt[no];}

	double mtCnt(){ return _mtCnt;}
	double utCnt(){ return _utCnt;}

	vector<double> tShrRN() { return _tShrRN; }
	vector<double> tShrRC() { return _tShrRC; } 
	double total(){ return _total;}
	vector<double> mmCnt(){ return _mmCnt; }
	vector<double> mShrRN(){ return _mShrRN;}
	vector<double> mShrRC(){ return _mShrRC;}
	vector<double> uuCnt(){ return _uuCnt;}
	vector<double> uShrRN() { return _uShrRN;}
	vector<double> uShrRC() { return _uShrRC;}

	int mCount() { return _mCount;}
	int uCount() { return _uCount;}
	int nCount() { return _nCount;}

	double muCnt(int no){ return _mCnt[no] + _uCnt[no] ; }
	
	double mRatio() { return _mRatio;}
	double mtShr(){ return _mtShr;}
	double utShr() { return _utShr;}
	double splitBefor() { return _splitBefor;}
	double splitAfter() { return _splitAfter;} 
	double splitgain() { return _splitgain;}
	baseInfo baseVal() { return _baseval;}
	void baseVal(RegPtn* rp) { 	_baseval.set(rp); }
	void baseVal(vector<char> clist) { 	_baseval.set(clist); }
	void baseVal(double th) { 	_baseval.set(th); }



	double errCnt(int i){ 
		return _total * _tShr->at(i) ;
	}

	//----------------------------------------------------
	// match,unmatchのどちらかがnullの件数に等しいとTrue
	//  ->umathあるいはmatchがnullのみ
	//=---------------------------------------------------
	bool nullall(void){ 
		if(_nCount!=0){
			return _mCount==_nCount || _uCount==_nCount ;
		}
		return false;
	}
	
	//----------------------------------------------------
	// self.__splitAfter >= self.__splitBefor
	// 	改善しているかどうか(小さくなれば改善したことになる）
	//		改善がなければTrue
	//----------------------------------------------------
	bool splitCheck(void){
		return _splitAfter >= _splitBefor; 
	}

	//----------------------------------------------------
	// match　unmatchのどちらかが0でTrue
	//----------------------------------------------------
	bool zeroCheck(void){ return _utCnt == 0 || _mtCnt == 0; }

	//----------------------------------------------------
	// totalが指定値以下でTrue
	//----------------------------------------------------
	bool totalCheck(int minsize){ return _total < minsize; }


	double errgain(void) const { return _total * _splitgain; }

	int maxClassNo(void);

	void tcnt_set(map<string,int>& cntset);
	//totalセット
	void total_set(double total){_total=total;}	
	void mratio_set(double mratio){_mRatio=mratio;}	
	
	
	void nullDset(int nulrec,vector<double> &nullcnt);

	void add_mCount(int cnt) { _mCount+=cnt; } 
	void add_uCount(int cnt) { _uCount+=cnt; } 
	void add_nCount(int cnt) { _nCount+=cnt; } 
	void add_mCnt(int cls,double cnt) { _mCnt[cls]+=cnt; } 
	void add_uCnt(int cls,double cnt) { _uCnt[cls]+=cnt; } 
	void add_nCnt(int cls,double cnt) { _nCnt[cls]+=cnt; } 
	


};


}
