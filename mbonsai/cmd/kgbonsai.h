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
// =============================================================================
// kgbonsai.h 項目の選択クラス
// =============================================================================
#pragma once
#include <kgmod.h>
#include <kgError.h>
#include <kgArgFld.h>
#include <kgCSV.h>
#include <kgCSVout.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include "kglibxmltool.h"
#include "ptnParam.h"
#include "flditem/clsitem.h" 
#include "attrvec.h"
#include "tree.h"
#include "aiqueue.h"
#include "predictdata.h"
using namespace kglib;

namespace kgmod { ////////////////////////////////////////////// start namespace

class kgBonsai : public kgMod
{

	double   _alpha;		// alpha= (モデル作成,予測共通)
	kgstr_t	 _alphaS;   // alpha= (予測モード)
	int 		 _alphaT;   // 0:未指定,1:double,2:min,3:1se

	// 引数
	kgArgFld _clsField;		// c= クラス項目
	kgArgFld _numField;		// n= 数値項目
	kgArgFld _catField;		// d= カテゴリ項目
	kgArgFld _patField;		// p= アイテムシーケンス

	kgCSVfld _iFile;		// i= 訓練データファイル名
	kgCSVfld _cFile;		// cost= コストファイル名
	kgstr_t  _oPath;		// O= 出力先ディレクトリ名

	kgCSVfld _tFile;		// i= 未知データファイル名 (予測モード)
  kgstr_t  _mPath;    // I= モデルディレクトリ (予測モード)
  kgCSVout _oFile;    // o= 出力ファイル名 (予測モード)


	size_t	 _seed;		   // seed= 乱数シード
	int			 _regCandC;	 // cand= 正規パターン候補上限
	int			 _seachC;		 // iter= 検索回数

	int      _lFsize;		// leafSize= リーフサイズ

	char		 _delim;    //delim=(モデル作成,予測共通)


	int  _estimation; // 1:テストサンプル法(ts=),2:交差検証法(cv=)
	double _separate;

	Tree *_tree;

	
	// 分類用クラス
	ClsItem _clsitem;
	AttrVec _attrVec;
	
	AIQueue _aiqueue; //alphabet index que

	ptnParams _ptn_params; //パターンパラメータ

	//　モデル作成モード関係(前処理)
	void setArgs(void);     // 引数セット
	void dataSet(void);     // データセット
	void outputParam(void); // パラメータ出力
	void resultOut(void);		// 結果出力

	//　モデル作成関係(tree作成)
	Tree* makeMaxTree(Pointer &pointer);
	Tree* makeTreeSub(void);
	Tree* makeTreeCore(void); 
	void  makeTree(void);

	// tree評価
	PredictData * runTraning(Tree * orgTree,vector<double> & alphaList);
	PredictData * runTestSample(Tree * orgTree ,vector<double> & alphaList);
	PredictData * runCrossValidation(Tree * orgTree ,vector<double> & alphaList);
	PredictData * runEstimate(Tree * orgTree ,vector<double> & alphaList);

	// 予測関係モード(前処理)
	void setArgsPredict1(void);
	void readModel(void);
	void dataSetPredict(void);
	void predict(void);

	//予測アウトプット
	void writeFldName_PD(kgCSVout & out);
	void writeFld_PD(kgCSVout & out , int id , vector<double>* clsWeight);

	// 旧バージョン用
	double   _confid;		// 未使用 旧バージョン用
	int      _lFcnt;		// leafSize= リーフサイズ
	void makeTree_C(void);



public:
	// コンストラクタ
	kgBonsai(void);
	~kgBonsai(void){
		if(_tree!=NULL){delete _tree;}
	}

	// 処理行数取得メソッド
	size_t iRecNo(void) const { return _iFile.recNo(); }

	//実行メソッド
	int run(void);

};

}
