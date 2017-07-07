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
#include "clstbl.h"

//------------------------------------------------------
// コスト正答率計算
//------------------------------------------------------
void clsTbl::calClsTbl(void)
{
	for(int i=0 ; i<_clsSz ; i++ ){
		for(int j=0 ; j<_clsSz ; j++ ){
			_cst[i][j] = _cnt[i][j] * _cls->getCost(i,j) ;
		}
	}
	// actualClass,predictedClassの合計をとる
	for(int i=0 ; i<_clsSz ; i++ ){
		for(int j=0 ; j<_clsSz ; j++ ){
			_atCnt[i] += _cnt[i][j];
			_ptCnt[i] += _cnt[j][i];
			_tCnt     += _cnt[i][j];
			_atCst[i] += _cst[i][j];
			_ptCst[i] += _cst[j][i];
			_tCst     += _cst[i][j];
		}
	}
	// 各種シェアの合計*/
	for(int i=0 ; i<_clsSz ; i++ ){
		for(int j=0 ; j<_clsSz ; j++ ){
			_aShr[i][j] = clsTbl::divDef(_cnt[i][j],_atCnt[i],0);
			_pShr[i][j] = clsTbl::divDef(_cnt[i][j],_ptCnt[j],0);
		}
		_atShr[i]=clsTbl::divDef(_atCnt[i],_tCnt,0);
		_ptShr[i]=clsTbl::divDef(_ptCnt[i],_tCnt,0);
	}
	for(int i=0 ; i<_clsSz ; i++ ){
		for(int j=0 ; j<_clsSz ; j++ ){
			if(i == j){ _accCnt += _cnt[i][j]; }
			else			{ _errCnt += _cnt[i][j]; }
		}
	}
	// 正解率、エラー率の計算
	_accRate = clsTbl::divDef(_accCnt,_tCnt,0);
	_errRate = clsTbl::divDef(_errCnt,_tCnt,0);
	///*幾何平均の計算*/
	_geoMean=1;
	for(int i=0 ; i<_clsSz ; i++ ){
		_geoMean *= _aShr[i][i];
	}
}

// モデル評価
void clsTbl::calStats(void){

	for(int i=0 ; i<_clsSz ; i++ ){
		_recall[i]    = clsTbl::divDef(_cnt[i][i],_atCnt[i],0);
		_precision[i] = clsTbl::divDef(_cnt[i][i],_ptCnt[i],0);
		_fpRate[i]    = clsTbl::divDef(_ptCnt[i]-_cnt[i][i],_tCnt-_atCnt[i],0);
		_f1[i]        = clsTbl::divDef(2*_recall[i]*_precision[i],_recall[i]+_precision[i],0);
		//_auc[i]       = calAuc();
	}
}

//------------------------------------------------------
// 結果output
//------------------------------------------------------
void clsTbl::output(ostream* ofp){
	*ofp << "## By count" << endl;
	*ofp << "         Predicted As ..." << endl;
	for(int i=0 ; i<_clsSz ; i++){
		*ofp << "\t" << _cls->id2item(i);
	}
	*ofp << "\tTotal" << endl;

	for(int i=0 ; i<_clsSz ; i++){
		*ofp << _cls->id2item(i);
		for(int j=0 ; j<_clsSz ; j++){
			*ofp << "\t" << _cnt[i][j];
		}
		*ofp << "\t" << _atCnt[i] << endl;
	}
	*ofp << "Total";
	for(int i=0 ; i<_clsSz ; i++){
		*ofp << "\t" << _ptCnt[i];
	}
	*ofp << "\t" << _tCnt << endl << endl;

	*ofp << "## By cost" << endl;
	*ofp << "         Predicted As ..." << endl;
	for(int i=0 ; i<_clsSz ; i++){
		*ofp << "\t" << _cls->id2item(i);
	}
	*ofp << "\tTotal" << endl;

	for(int i=0 ; i<_clsSz ; i++){
		*ofp << _cls->id2item(i);
		for(int j=0 ; j<_clsSz ; j++){
			*ofp << "\t" << _cst[i][j];
		}
		*ofp << "\t" << _atCst[i] << endl;
	}
	*ofp << "Total";
	for(int i=0 ; i<_clsSz ; i++){
		*ofp << "\t" << _ptCst[i];
	}
	*ofp << "\t" << _tCst << endl << endl;

	*ofp << "## Detailed accuracy by class" << endl;
	*ofp << "class,recall,precision,FPrate,F-measure" << endl;
	for(int i=0 ; i<_clsSz ; i++){
		*ofp << _cls->id2item(i) << "," << _recall[i] << "," << _precision[i] << "," << _fpRate[i] << "," << _f1[i] << endl;
	}

	*ofp << endl << "## Summary" << endl;
	*ofp << "accuracy=" << _accRate << endl;
	*ofp << "totalCost=" << _tCst <<endl;
}


//------------------------------------------------------
// show
//------------------------------------------------------
void clsTbl::show(void){
	cout << "## By count" << endl;
	cout << "         Predicted As ..." << endl;
	for(int i=0 ; i<_clsSz ; i++){
		cout << "\t" << _cls->id2item(i);
	}
	cout << "\tTotal" << endl;

	for(int i=0 ; i<_clsSz ; i++){
		cout << _cls->id2item(i);
		for(int j=0 ; j<_clsSz ; j++){
			cout << "\t" << _cnt[i][j];
		}
		cout << "\t" << _atCnt[i] << endl;
	}
	cout << "Total";
	for(int i=0 ; i<_clsSz ; i++){
		cout << "\t" << _ptCnt[i];
	}
	cout << "\t" << _tCnt << endl << endl;

	cout << "## By cost" << endl;
	cout << "         Predicted As ..." << endl;
	for(int i=0 ; i<_clsSz ; i++){
		cout << "\t" << _cls->id2item(i);
	}
	cout << "\tTotal" << endl;

	for(int i=0 ; i<_clsSz ; i++){
		cout << _cls->id2item(i);
		for(int j=0 ; j<_clsSz ; j++){
			cout << "\t" << _cst[i][j];
		}
		cout << "\t" << _atCst[i] << endl;
	}
	cout << "Total";
	for(int i=0 ; i<_clsSz ; i++){
		cout << "\t" << _ptCst[i];
	}
	cout << "\t" << _tCst << endl << endl;
	cout << endl << "## Summary" << endl;
	cout << "accuracy=" << _accRate << endl;
	cout << "totalCost=" << _tCst <<endl;
}


void clsTbl::clear(void){
	for(int i=0 ; i<_clsSz ;i++ ){
		for(int j=0 ; j<_clsSz ;j++ ){
			_cnt[i][j]=0;
			_aShr[i][j]=0;
			_pShr[i][j]=0;
			_cst[i][j]=0;
		}
		_atCnt[i]=0;
		_atShr[i]=0;
		_atCst[i]=0;
		_ptCnt[i]=0;
		_ptShr[i]=0;
		_ptCst[i]=0;
		_recall[i]=0;
		_precision[i]=0;
		_f1[i]=0;
		_fpRate[i]=0;
		_auc[i]=0;
	}
	_tCnt=0;
	_tCst=0.0;
	_accCnt=0;
	_errCnt=0;
	_accRate=0.0;
	_errRate=0.0;
	_geoMean=0.0;
}
