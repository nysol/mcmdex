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
#include <ctime>
#include <kgCSVout.h>
#include <libxml/xpath.h>
#include "flditem.h"
#include "flditem/clsitem.h"
#include "node.h"
#include "pointer.h"
#include "attrvec.h"
#include "clstbl.h"
#include "kglibxmltool.h"
#include "predictdata.h"

namespace kglib {

class Tree{
	ClsItem* _cls;
	AttrVec* _attr;
 	double 	 _CF; // 枝刈り度,確信度( 'F='/100 )
	int      _lFSz;			// leafSize= リーフサイズ制限
	int      _lFNum;		// leafCnt= リーフ数制限
	Node*		 _topNode;
	int			 _lFcnt;
	int			 _deep;
	clsTbl	 _trnRsl;
	clsTbl	 _tstRsl_1SE;
	clsTbl	 _tstRsl_MIN;
	kgstr_t	 _oFname;
	
	PredictData * _pred;

	size_t _traCnt;

	//vector<double> _minVec; //アルファ一覧

	// tree作成
	Node* makeTree(Pointer &pointer ,Node *pNode );


	// 結果表データ set
	//void setClsTbl(bool type=true);
	void setClsTbl(void);
	void setClsTblFromTest(bool min=true);
	// リーフ数指定枝狩り
	void pruneLFC(int leafC);

	// 枝狩り リーフ数指定の場合とそれ以外の場合(Error-based Pruning)で処理をかえる
	void pruneTree(void);
	
	public:

 	Tree(void)
 		:_cls(NULL),_CF(0),_lFSz(0),_lFNum(0),_topNode(NULL),
 		 _lFcnt(0),_deep(0),_pred(NULL),_traCnt(0){}

 	Tree(ClsItem* cls,AttrVec* attr,size_t tracnt,double cf=0,
 					int lfm=0,int lfc=0,kgstr_t fn="")
	 		:_cls(cls),_attr(attr),_CF(cf),_lFSz(lfm),_lFNum(lfc),
 			 _topNode(NULL),_lFcnt(0),_deep(0),_trnRsl(cls),_tstRsl_1SE(cls),_tstRsl_MIN(cls),_oFname(fn),_pred(NULL),_traCnt(tracnt){}
 			 //_topNode(NULL),_lFcnt(0),_deep(0),_trnRsl(cls),_tstRsl(cls),_oFname(fn){}

	~Tree(){
		if(_topNode!=NULL){ delete _topNode;}
		if(_pred!=NULL){ delete _pred;}
	}

  // ツリー構造体の各種変数(レベル,最深レベル,リーフ数)のセット
	void setLevel(void){ _lFcnt = _topNode->setlevel(0,&_deep);}
	void setPred(PredictData *pred){  _pred = pred;}
	PredictData *pred(void){ return _pred;}

	void restore(void){	_topNode->restoreNode();}

	// 決定木作成メインルーチン
	void decisionTree(void);
	void decisionTree(Pointer &pointer);
	void calinfoOut(kgstr_t fname , int type , bool min=true );

	// T1生成
	void cal_T1(void);
	vector<double> cal_alpha(void);
	void alphaPRB(double);

	// 決定木からクラス予測 (type=falseでテストデータで予測)
	//int predictTree(int recNo,bool type=true);
	//int predictTree(int recNo);
	int predictTree(int recNo,vector<double>* clsW = NULL);
	
	void setTopNode(xmlNodePtr np);
	Node* setNode(xmlNodePtr np,Node* node,bool setf);

	
	double tcst(void) { return _trnRsl.tcst();}
//	int lFcnt(void)		{ return _lFcnt;}
	int lFcnt(double alpha=0)		{ return _topNode->lFcnt(alpha);}
	map<double,int> lfcList(vector<double> alphaList);
	// Tree比較
	bool better(Tree *other);
	bool betterPred(Tree *other);
  // socre計算
	void calScore(void);

	// Tree情報出力
	//void show(void);
	void output(ostream* ofp=&cout);
	void outMinfo(kgCSVout *out, int type , bool min=true);

	double errMin(void){ 
		if(_pred) { return _pred->errMin(); }
		else			{ return DBL_MAX;}
	}
	// 時刻取得
	string getTime(void);
	//PMML出力
	void writePMML(kgstr_t fname,bool noestimate = false);


};

}
