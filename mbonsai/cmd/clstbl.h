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
#include "flditem/clsitem.h"

class clsTbl
{

	static double divDef(double d1, double d2, double def){
 	 if(d2==0) return(def);
  	return(d1/d2);
	}
	
	ClsItem* _cls;
	int _clsSz;
	vector< vector<double> > _cnt;		//実際クラス:予測クラスの件数	
	vector< vector<double> > _aShr;		//実際クラス:予測クラスの件数/実際クラスごとの件数集計
	vector< vector<double> > _pShr;		//実際クラス:予測クラスの件数/予想クラスごとの件数集計
	vector< vector<double> > _cst;		//実際クラス:予測クラスの件数*コスト値	
	
	vector<double> _atCnt;	//実際クラスごとの件数集計 
	vector<double> _atShr;		//実際クラスごとの件数集計/全件数
	vector<double> _atCst;	//実際クラスごとのコスト集計 　

	vector<double> _recall;	   //クラスごとのrecall
	vector<double> _precision; //クラスごとのprecision
	vector<double> _f1;        //クラスごとのf1
	vector<double> _fpRate;    //クラスごとの偽陽性率
	vector<double> _auc;       //クラスごとのauc(area under the curve)
	
	vector<double> _ptCnt;	//予想クラスごとの件数集計
	vector<double> _ptShr;		//予想クラスごとの件数集計/全件数
	vector<double> _ptCst;	//予想クラスごとのコスト集計

	double _tCnt;	//全件数
	double _tCst;	//全コスト
	double _accCnt;	//正解件数(実際クラス=予測クラス)
	double _errCnt;	//エラー件数(実際クラス!=予測クラス)
	double _accRate;	// 正解件数/全件数
	double _errRate;	// エラー件数/全件数
	double _geoMean;	// 正解したクラスのaShrの乗算

	public:
	clsTbl(void):
		_cls(NULL),_clsSz(0),_tCnt(0),_tCst(0.0),_accCnt(0),
		_errCnt(0),_accRate(0.0),_errRate(0.0),_geoMean(0.0){}

	clsTbl(ClsItem* cls):
		 _cls(cls),_clsSz(cls->itmSz()), _cnt(_clsSz,vector<double>(_clsSz,0)) , _aShr(_clsSz,vector<double>(_clsSz,0)),
		_pShr(_clsSz,vector<double>(_clsSz,0)) , _cst(_clsSz,vector<double>(_clsSz,0)),
		_atCnt(_clsSz,0),_atShr(_clsSz,0),_atCst(_clsSz,0),
		_recall(_clsSz,0),_precision(_clsSz,0),_f1(_clsSz,0),_fpRate(_clsSz,0),_auc(_clsSz,0),
		_ptCnt(_clsSz,0),_ptShr(_clsSz,0),_ptCst(_clsSz,0),
		_tCnt(0),_tCst(0.0),_accCnt(0),_errCnt(0),
		_accRate(0.0),_errRate(0.0),_geoMean(0.0){}

	double tcst(void) { return _tCst;}
	void addcnt(int ac,int pr,double val){ _cnt[ac][pr]+=val;}

	double accRate(void){return _accRate;}
	// コスト正答率計算
	void calClsTbl(void);
	void calStats(void);

	// output
	void output(ostream* ofp=&cout);
	void show(void);
	void clear(void);

};
