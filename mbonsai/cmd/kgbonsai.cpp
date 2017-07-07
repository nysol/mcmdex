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
// kgcut.cpp 項目の選択クラス
// =============================================================================
#include <cstdio>
#include <vector>
#include <fstream>
#include <kgbonsai.h>
#include <kgError.h>
#include <kgConfig.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include "aiqueue.h"
#include "tree.h"
#include "pointer.h"

using namespace std;
using namespace kglib;
using namespace kgmod;
using namespace boost;


//=NOT USE=================================================================================
// -----------------------------------------------------------------------------
// 分類木作成(旧バージョン 未使用)
// -----------------------------------------------------------------------------
void kgBonsai::makeTree_C(void)
{
	Tree *bestTree = NULL;

	AIQueue aiqueue;
	_attrVec.setAiQue(aiqueue); // 全_patitemの初期化

	for(int l=0;l<_seachC;l++){
		Tree *betterTree = NULL;
		bool improved;
		_attrVec.prerun(); // 全_patitemにアルファベットインデックスをセットする
		while(1){
			improved = false;
			aiqueue.updNextIndex(); // local searchにて次のインデックスをセットする
			while(1){ //index loop
				_attrVec.showAlphabet();
				cerr << "= ";
				aiqueue.showIndex();
				//index更新＆データ作成
				_attrVec.updAique(aiqueue,&_clsitem);
				// tree
				Tree * tree = new Tree(&_clsitem,&_attrVec,_clsitem.traSz(),_confid,_lFsize,_lFcnt,_oPath);
				tree->decisionTree();
				if( tree->better(betterTree) ){
					improved=true;
					if(betterTree!=NULL){ delete betterTree;}
					betterTree = tree;
					_attrVec.betterSet();
					aiqueue.stockBetterIndx();
					cerr << " *improved(" << betterTree->tcst() << "," << betterTree->lFcnt() << ")";
				}
				else{
					if(tree){ delete tree;}
				}
				cerr << endl;
				if(!aiqueue.updNextIndex()){ break;}
			}
			if(!improved || aiqueue.empty()){ break;}
			else{ aiqueue.setBetterIndx(); }
		}
		if(betterTree->better(bestTree)){ 
			if(bestTree!=NULL){ delete bestTree;}
			bestTree = betterTree; 
			_attrVec.bestSet();
		}
		aiqueue.reset();
	}
	bestTree->calScore();

	ofstream ofp;
	ofp.open((_oPath+"/model.txt").c_str());
	bestTree->output(&ofp);
	bestTree->writePMML(_oPath+"/model.pmml");
	if(bestTree!=NULL){delete bestTree;}
}
//=NOT USE===============================================================================

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録,パラメータ)
// -----------------------------------------------------------------------------
kgBonsai::kgBonsai(void)
{
	#ifdef ENG_FORMAT
		#include <help/en/kgbonsaiHelp.h>
	#else
		#include <help/jp/kgbonsaiHelp.h>
	#endif
	_name    = "kgbonsai";
	_version = "1.0";
	_tree = NULL;
	_delim = '\0';
	_regCandC = 30;
	_seachC = 1;
	_lFcnt = -1;
	_lFsize = -1;
}

// -----------------------------------------------------------------------------
// 引数処理(モデル作成用)
// -----------------------------------------------------------------------------
void kgBonsai::setArgs(void)
{
	// パラメータチェック
	_args.paramcheck("p=,n=,d=,c=,i=,I=,O=,delim=,cost=,seed=,cand=,iter=,leafSize=,ts=,cv=,alpha=");

	// トレーニングデータオープン
	_iFile.open(_args.toString("i=",true), _env,_nfn_i);
	_iFile.read_header();
	
	// c,n,d,p= 各種項目引数のセット
	// p=pat:3:true:true:3:3:5
	// 左から   項目名: インデックスサイズ:subsequenceかどうか:(true/false)
	// ordered itemかどうか:(true/false) 先頭一致: 末尾一致,最大index長
	kgstr_t cls_a = _args.toString("c=",true);
	vector<kgstr_t> num_a = _args.toStringVector("n=",false);
	vector<kgstr_t> cat_a = _args.toStringVector("d=",false);
	vector<vector<kgstr_t> > pat_a = _args.toStringVecVec("p=",':',7,false);
	_clsField.set(cls_a, &_iFile, _fldByNum);
	_numField.set(num_a, &_iFile, _fldByNum);
	_catField.set(cat_a, &_iFile, _fldByNum);
	_patField.set(pat_a, &_iFile, _fldByNum);

	// delim= デリミタ
	kgstr_t d = _args.toString("delim=",false);
	if(!d.empty()){ _delim = d.at(0);}
	
	// クラス設定
	_ptn_params = ptnParams(pat_a,_delim);

	// 出力ディレクトリ名所得
	// 
	_oPath = _args.toString("O=",true);
	if(!boost::filesystem::is_directory(_oPath)){
		// ディレクトリ作成
		filesystem::path dirname=filesystem::path(_oPath);
		create_directories(dirname);
	}

	// コスト設定
	kgstr_t _cFname = _args.toString("cost=",false);
	if(!_cFname.empty()){
		_cFile.open(_cFname, _env, false);
		_cFile.read_header();
	}

	// seed= 乱数シード
	kgstr_t S_s = _args.toString("seed=",false);
	if(S_s.empty())	{
		// 乱数の種生成（時間）
		posix_time::ptime now = posix_time::microsec_clock::local_time();
		posix_time::time_duration nowt = now.time_of_day();
		_seed = nowt.total_microseconds();
	}
	else	{ _seed = aToSizeT(S_s.c_str());}

	_attrVec.setSeed(_seed);
	
	// cand= 候補正規パターン数 (1-256)
	kgstr_t candSize_s = _args.toString("cand=",false);
	if(!candSize_s.empty()){ _regCandC = atoi(candSize_s.c_str());}
	if(_regCandC>256) { _regCandC = 256; }
	if(_regCandC<1)		{ _regCandC = 1; }

	// iter= パターン項目が有る場合サーチ回数(1-5)
	kgstr_t tryCount_s = _args.toString("iter=",false);
	if(!tryCount_s.empty()){ _seachC = atoi(tryCount_s.c_str());}
	if(_seachC>5) { _seachC = 5; }
	if(_seachC<1) { _seachC = 1; }
	if(_patField.size()==0){ _seachC = 1; }

	// leafSize=　-1は指定無し
	kgstr_t lfs_s = _args.toString("leafSize=",false);
	if(!lfs_s.empty()) { _lFsize = atoi(lfs_s.c_str());}

	//分類対象クラス設定
	_clsitem.fldnum( _clsField.num(0),_clsField.name(0) );
	
	int _flditemC = _numField.size() + _catField.size() + _patField.size(); 
	if(_flditemC==0){ throw kgError("parameter p= or d= or n= is mandatory"); }
	
	for(size_t i=0 ; i<_catField.size(); i++){ 
		_attrVec.push( new CategoryItem(_catField.num(i),_catField.name(i)) );
	}
	for(size_t i=0 ; i<_numField.size(); i++){ 
		_attrVec.push( new NumericItem(_numField.num(i),_numField.name(i)) );
	}
	for(size_t i=0 ; i<_patField.size(); i++){ 
		_attrVec.push( new PatternItem(_patField.num(i),_patField.name(i),_ptn_params.at(i),_regCandC) );
	}

	//評価方法セット
	if(_args.find("ts=")){
		kgstr_t ts_s = _args.toString("ts=",true);
		if(!ts_s.empty()){ _estimation=1; _separate = atof(ts_s.c_str()); }
		else { _estimation=1; _separate = 1.0/3.0; }
	}
	else if(_args.find("cv=")){
		kgstr_t cv_s = _args.toString("cv=",true);
		if(!cv_s.empty()){ _estimation=2; _separate = atoi(cv_s.c_str());}	
		else { _estimation=2; _separate = 10; }
	}
	else{ _estimation=0; _separate=1; _alpha = 0.01; }
	kgstr_t al_s = _args.toString("alpha=",false);
	if(!al_s.empty()) { _alpha = atof(al_s.c_str());}

}

// -----------------------------------------------------------------------------
// 引数処理(予測モード用)
// ファイル関連のみ。その他のパラメータはreadModel後に設定する。
// -----------------------------------------------------------------------------
void kgBonsai::setArgsPredict1(void)
{
	// パラメータチェック
	_args.paramcheck("i=,I=,o=,-predict,alpha=,delim=");

	_tFile.open(_args.toString("i=",true), _env,_nfn_i); // 未知データオープン
	_tFile.read_header();
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);// 予測結果出力ファイル
  _mPath=_args.toString("I=",true);                    // モデルパス

	// delim= デリミタ
	kgstr_t d = _args.toString("delim=",false);
	if(d.empty()) _delim = '\0';
	else          _delim = d.at(0);

	_alphaS = _args.toString("alpha=",false);
			 if(_alphaS.empty()){ _alphaT=0;} 
	else if( _alphaS == "min"){ _alphaT=2; } 
	else if( _alphaS == "1se"){ _alphaT=3; } 
	else { 
		_alphaT = 1; 
		_alpha  = atof(_alphaS.c_str());
	}
}
// -----------------------------------------------------------------------------
// 決定木モデル(PMML)の読み込み
// -----------------------------------------------------------------------------
void kgBonsai::readModel(void){

	kgstr_t mFile=_mPath+"/model.pmml";
	kgstr_t cls_a; 
	vector<kgstr_t> num_a ;
	vector<kgstr_t> cat_a;
	vector< vector<kgstr_t> > pat_a(7);


	/* Read document */
	// ここでファイルがなければ、warningメッセージが出力されるが、catchできない。
	// I/O warning : failed to load external entity "./outdat/man11/model.pmml"
	xmlDocPtr doc = xmlParseFile(mFile.c_str());
	if (!doc) { throw kgError("failed to open a PMML file"); }

	// XPATH set
	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	if (!ctx) { throw kgError("context error"); }
    
	// 項目情報(DataDictionary)と 項目情報(MiningSchema)によりくらす決定
	map<kgstr_t,vector<kgstr_t> > dic_info;
	map<kgstr_t,map<kgstr_t,int> > schm_indx;
	
	xmlXPathObjectPtr xpfcobj = xmlXPathEvalExpression((xmlChar *)"/PMML/DataDictionary/*", ctx);
	if (!xpfcobj) { throw kgError("context error"); }
	xmlNodeSetPtr nodes_fc = xpfcobj->nodesetval;
	int size_fc = (nodes_fc) ? nodes_fc->nodeNr : 0;
	for (int i = 0; i < size_fc; ++i) {
		xmlNodePtr node_tfc = xmlXPathNodeSetItem(nodes_fc, i);
		if(!xmlStrEqual(node_tfc->name,(xmlChar *)"DataField")){ throw kgError("context error"); }
		map<kgstr_t,kgstr_t> attrlist = kglibxml::get_attr(node_tfc);
		map<kgstr_t,kgstr_t>::const_iterator itor = attrlist.find("dataType");
		if( itor!=attrlist.end() && (*itor).second == "string" ){
			vector<kgstr_t> vlist;
			xmlNodePtr vchild = node_tfc->children;
			while(vchild){
				if(xmlStrEqual(vchild->name,(xmlChar *)"Value")){
					xmlAttrPtr attr_tfc_v = vchild->properties;
					while(attr_tfc_v){
						if(xmlStrEqual(attr_tfc_v->name,(xmlChar *)"value")){
							vlist.push_back((char*)attr_tfc_v->children->content);
						}
						attr_tfc_v = attr_tfc_v->next;
					}
				}
				vchild = vchild->next;
			} 
			dic_info[attrlist["name"]] = vlist;
		}
		else{
			vector<kgstr_t> dmy;
			dic_info[(char*)node_tfc->name] = dmy;
		}
	}
	if(xpfcobj){xmlXPathFreeObject(xpfcobj);}
	
	// alpha情報取得
	// 項目情報(MiningSchema)取得
	if(_alphaT != 1){
		xmlXPathObjectPtr alobj = xmlXPathEvalExpression((xmlChar *)"/PMML/TreeModel/Extension", ctx);
		if (alobj){
			double al_1se,al_min , al;
			al_1se = al_min = al = -1;
			xmlNodeSetPtr nodes_al = alobj->nodesetval;
			int size_al = (nodes_al) ? nodes_al->nodeNr : 0;
			for (int i = 0; i < size_al; ++i) {
				xmlNodePtr node_al = xmlXPathNodeSetItem(nodes_al, i);
				map<kgstr_t,kgstr_t> attrlist_a = kglibxml::get_attr(node_al);
				if(attrlist_a.find("name")!=attrlist_a.end()){
					if(attrlist_a["name"]=="1SE alpha"){
						 al_1se = atof(attrlist_a["value"].c_str());
					}
					else if(attrlist_a["name"]=="min alpha"){
						 al_min = atof(attrlist_a["value"].c_str());
					}
					else if(attrlist_a["name"]=="alpha"){
						 al     = atof(attrlist_a["value"].c_str());
					}
				}
			}
			     if(_alphaT==2) { _alpha = al_min; }
			else if(_alphaT==3) { _alpha = al_1se; }
			else if(_alphaT==0) { 
				if(al==-1){ _alpha = al_min; }
				else			{ _alpha = al;		 } 
			}

		}
		if(alobj){xmlXPathFreeObject(alobj);}
		if(_alpha==-1){throw kgError("can not use alpha=min or alpha=1se in this model"); }
	}

	// 項目情報(MiningSchema)取得
	xmlXPathObjectPtr msobj = xmlXPathEvalExpression((xmlChar *)"/PMML/TreeModel/MiningSchema/*", ctx);
	if (!msobj) { throw kgError("context error"); }
	xmlNodeSetPtr nodes_ms = msobj->nodesetval;
	int size_ms = (nodes_ms) ? nodes_ms->nodeNr : 0;
	for (int i = 0; i < size_ms; ++i) {
		xmlNodePtr node_ms = xmlXPathNodeSetItem(nodes_ms, i);
		map<kgstr_t,kgstr_t> attrlist = kglibxml::get_attr(node_ms);
		map<kgstr_t,kgstr_t>::const_iterator itor = attrlist.find("missingValueTreatment");
		if( itor!=attrlist.end() && (*itor).second == "asMean" ){
			//cat=1 or num=2 or pat =3
			if(dic_info[attrlist["name"]].size()==0){ 
				num_a.push_back(attrlist["name"]);
			}
			else {
				cat_a.push_back(attrlist["name"]);
			}
		}
		else{
			map<kgstr_t,kgstr_t>::const_iterator itorx = attrlist.find("usageType");
			if( itorx!=attrlist.end() && (*itorx).second == "predicted" ){
				cls_a=attrlist["name"];
			}
			else{//pat index処理
				kgstr_t p_fldname=attrlist["name"];
				map<kgstr_t,int> i_info;
				kgstr_t indexSize,order,subsequence,indexLen,head,tail;
				xmlNodePtr mvchild = node_ms->children;
				while(mvchild){
					if(xmlStrEqual(mvchild->name,(xmlChar *)"Extension")){
						map<kgstr_t,kgstr_t> ex_attr = kglibxml::get_attr(mvchild);
						if(ex_attr["name"]=="alphabetIndex"){
							xmlNodePtr mvvchild = mvchild->children;
							while(mvvchild){
								if(xmlStrEqual(mvvchild->name,(xmlChar *)"alphabetIndex")){
									map<kgstr_t,kgstr_t> attrlistvvc = kglibxml::get_attr(mvvchild);
									i_info[attrlistvvc["alphabet"]] = atoi(attrlistvvc["index"].c_str());
								}
								mvvchild = mvvchild->next;
							}
						}
						else if(ex_attr["name"]=="indexSize"){ indexSize = ex_attr["value"]; }
						else if(ex_attr["name"]=="order"){ order = ex_attr["value"];}
						else if(ex_attr["name"]=="subsequence"){ subsequence = ex_attr["value"];}
						else if(ex_attr["name"]=="indexLen"){ indexLen = ex_attr["value"];}
						else if(ex_attr["name"]=="head"){ head = ex_attr["value"];}
						else if(ex_attr["name"]=="tail"){ tail = ex_attr["value"];}
					}
					mvchild = mvchild->next;
				} 
				//パラメータ文字列生成
				pat_a[0].push_back(p_fldname);
				pat_a[1].push_back(indexSize);
				pat_a[2].push_back(order);
				pat_a[3].push_back(subsequence);
				pat_a[4].push_back(head);
				pat_a[5].push_back(tail);
				pat_a[6].push_back(indexLen);
				schm_indx[attrlist["name"]] = i_info;
			}
		}
	}
	if(msobj){xmlXPathFreeObject(msobj);}
	
	//_clsField.set(cls_a, &_tFile, _fldByNum);
	_numField.set(num_a, &_tFile, _fldByNum);
	_catField.set(cat_a, &_tFile, _fldByNum);
	_patField.set(pat_a, &_tFile, _fldByNum);
	_ptn_params = ptnParams(pat_a , _delim);

	_clsitem.fldnum(0,cls_a);
	
	int _flditemC = _numField.size() + _catField.size() + _patField.size(); 
	if(_flditemC==0){ throw kgError("parameter p= or d= or n= is mandatory"); }
	
	for(size_t i=0 ; i<_catField.size(); i++){ 
		_attrVec.push( new CategoryItem(_catField.num(i),_catField.name(i)) );
	}
	for(size_t i=0 ; i<_numField.size(); i++){ 
		_attrVec.push( new NumericItem(_numField.num(i),_numField.name(i)) );
	}
	for(size_t i=0 ; i<_patField.size(); i++){ 
		_attrVec.push( new PatternItem(_patField.num(i),_patField.name(i),_ptn_params.at(i),_regCandC) );
	}

	// アイテム情報セット data_set
	//map<kgstr_t,vector<kgstr_t> > dic_info;データ名一覧
	//map<kgstr_t,map<kgstr_t,int> > schm_indx;データ名index
	_clsitem.itemSet(dic_info[cls_a]);
	_attrVec.itemSet(dic_info);
	_attrVec.idxSet(schm_indx);


	_tree = new Tree(&_clsitem,&_attrVec,_clsitem.traSz());

	// Node　パース
	xmlXPathObjectPtr nobj = xmlXPathEvalExpression((xmlChar *)"/PMML/TreeModel/Node", ctx);
	if (!nobj) { throw kgError("context error"); }
	xmlNodeSetPtr nodes_nd = nobj->nodesetval;
	int size_nd = (nodes_nd) ? nodes_nd->nodeNr : 0;
	
	for (int i = 0; i < size_nd; ++i) {
		xmlNodePtr node_nd = xmlXPathNodeSetItem(nodes_nd, i);
		if(!xmlStrEqual(node_nd->name,(xmlChar *)"Node")){ continue;}
		_tree->setTopNode(node_nd);
	}
	if(nobj){xmlXPathFreeObject(nobj);}


	// LIBXMLメモリクリーン
	if(ctx){ xmlXPathFreeContext(ctx);}
	if(doc){ xmlFreeDoc(doc); }

}
// -----------------------------------------------------------------------------
// データ読み込み(モデル作成用)
// -----------------------------------------------------------------------------
void kgBonsai::dataSet(void)
{
	// 初期読み込み(アイテム読み込み)
	while(_iFile.read()!=EOF){
		char **lin =  _iFile.getFld();
		_clsitem.init_read(lin);
		_attrVec.init_read(lin);
	}
	// indexサイズの調整: 0指定の場合にalphabetサイズにあわせる
	_attrVec.adjustIndexSize();

	// データ読み込み(アイテム読み込み)
	_iFile.seekTop();
	while(_iFile.read()!=EOF){
		char **lin =  _iFile.getFld();
		_clsitem.read(lin);
		_attrVec.read(lin);
	}
	//コストセット
	_clsitem.costSet(_cFile);
	//データ分割
	if(_estimation!=0){
		_clsitem.dataSplit(_estimation,_separate,_seed);
	}	
}
// -----------------------------------------------------------------------------
// データ読み込み（予測モード用）
// -----------------------------------------------------------------------------
void kgBonsai::dataSetPredict(void)
{
	while(_tFile.read()!=EOF){
		char **lin =  _tFile.getFld();
		_attrVec.read(lin);
	}
}
// -----------------------------------------------------------------------------
// データ予測（予測モード用）
// -----------------------------------------------------------------------------
void kgBonsai::predict(void)
{
	_tFile.seekTop();
	vector<kgstr_t> nf ;
	nf.push_back("predict");
	for(int i=0 ; i < _clsitem.itmSz() ;i++){
		nf.push_back(_clsitem.id2item(i));
	}
	_oFile.writeFldName(_tFile,nf);

	int incnt=0;
	while(_tFile.read()!=EOF){
		for(size_t i=0; i<_tFile.fldSize(); i++){
			_oFile.writeStr(_tFile.getVal(i), false );
		}
		vector<double> clsWeight(_clsitem.itmSz(),0);
		_oFile.writeStr(_clsitem.id2item(_tree->predictTree(incnt,&clsWeight)).c_str(), false);
		for(size_t i=0; i < clsWeight.size(); i++){
			_oFile.writeDbl(clsWeight[i], i==clsWeight.size()-1);
		}
		incnt++;
	}
}
// -----------------------------------------------------------------------------
// ツリー作成:RUN(モデル作成用)
// -----------------------------------------------------------------------------
Tree * kgBonsai::makeMaxTree(Pointer &pointer)
{
	Tree * tree = new Tree(&_clsitem,&_attrVec,pointer.size(),_confid,_lFsize,_lFcnt,_oPath);
	//最大木＆アルファを求めてアルファの一覧を取得する。
	tree->decisionTree(pointer);
	tree->cal_T1();
	return tree;

}
// -----------------------------------------------------------------------------
// ツリー作成:データセット(モデル作成用)
// -----------------------------------------------------------------------------
Tree * kgBonsai::makeTreeSub(void)
{
	// データセット
	Pointer pointer(_clsitem.traSz());
	for(int i=0 ; i<_clsitem.traSz() ; i++){ pointer.push(i,1); }
	//最大木&T1を求める（最大木からNodeを実際に削除する）
	return makeMaxTree(pointer);
}
// -----------------------------------------------------------------------------
// ツリー作成:index LOOP(モデル作成用)
// -----------------------------------------------------------------------------
Tree* kgBonsai::makeTreeCore(void)
{
	Tree *betterTree = NULL;
	bool improved;
	_attrVec.prerun(); // 全_patitemにアルファベットインデックスをセットする
	while(1){
		improved = false;
		if(!_aiqueue.updNextIndex()&& betterTree != NULL){ break;} // local searchにて次のインデックスをセットする
		while(1){ //index loop
			_attrVec.showAlphabet(); cerr << "= ";
			_aiqueue.showIndex();
			//index更新＆データ作成
			_attrVec.updAique(_aiqueue,&_clsitem);

			// tree作成
			Tree * tree = makeTreeSub();
			vector<double> alphaList =  tree->cal_alpha();
			tree->setPred(runEstimate(tree,alphaList));

			if( tree->betterPred(betterTree) ){
				improved=true;
				if(betterTree!=NULL){ delete betterTree;}
				betterTree = tree;
				_attrVec.betterSet();
				_aiqueue.stockBetterIndx();
				cerr << " *improved(errev:" << betterTree->pred()->evaluation();
				cerr << " *improved(errMin:" << betterTree->errMin() << ",leaf:" << betterTree->lFcnt() << ")";
			}
			else{
				if(tree){ delete tree;}
			}
			cerr << endl;
			if(!_aiqueue.updNextIndex()){ break;}
		}

		if(!improved || _aiqueue.empty()){ break;}
		else{ 
			_aiqueue.setBetterIndx(); 
		}
	}
	return betterTree; 
}
// -----------------------------------------------------------------------------
// ツリー作成::サーチ回数LOOP(モデル作成用)
// -----------------------------------------------------------------------------
void kgBonsai::makeTree(void)
{
	// 全_patitemの初期化
	_attrVec.setAiQue(_aiqueue); 

	Tree *bestTree = NULL;

	//indexの初期値を変えて繰り返し実行
	for(int l=0;l<_seachC;l++){
		//tree生成
		Tree * tree_rls = makeTreeCore();

		if(tree_rls->betterPred(bestTree)){ 
			if(bestTree!=NULL){ delete bestTree;}
			bestTree = tree_rls; 
			_attrVec.bestSet();
		}
		_aiqueue.reset();
	}
	_tree =  bestTree;
	return ;
}
// -----------------------------------------------------------------------------
// パラメータ出力(モデル作成用)
// -----------------------------------------------------------------------------
void kgBonsai::outputParam(void)
{
	kgCSVout out;
 	out.open(_oPath+"/param.csv", _env,_nfn_o);

	vector<kgstr_t> nf ;
	nf.push_back("key");
	nf.push_back("value");
	out.writeFldName(nf);

	for (set<string>::iterator it = _args.begin_avail();it!= _args.end_avail();it++){
		out.writeStr((*it).c_str(),false);
		kgstr_t v = _args.get(*it);
		if(v==""){
			     if( *it == "cand=")    { out.writeStr("30",true);}
			else if( *it == "leafSize="){ out.writeStr("-1",true);}
			else if( *it == "iter=")    { out.writeStr("1",true);}
			else if( *it == "alpha=")   { out.writeStr("0.01",true);}
			else if( *it == "ts=" && _args.find("ts=")){ out.writeDbl(1.0/3.0,true); }
			else if( *it == "cv=" && _args.find("cv=")){ out.writeStr("10",true); }
			else                        { out.writeStr(v.c_str(),true);} 
		}
		else{
			out.writeStr(v.c_str(),true);	
		}
	}
}
// -----------------------------------------------------------------------------
// 予測出力:項目名(モデル作成用)
// -----------------------------------------------------------------------------
void kgBonsai::writeFldName_PD(kgCSVout & out)
{
	vector<kgstr_t> nf ;
	nf.push_back("predict");
	for(int i=0 ; i < _clsitem.itmSz() ;i++){
		nf.push_back(_clsitem.id2item(i));
	}
	out.writeFldName(_iFile,nf);
}
// -----------------------------------------------------------------------------
// 予測出力:データ(モデル作成用)
// -----------------------------------------------------------------------------
void kgBonsai::writeFld_PD(kgCSVout & out,int id, vector<double>* clsWeight)
{
	for(size_t i=0; i<_iFile.fldSize(); i++){
		out.writeStr(_iFile.getVal(i), false );
	}
	out.writeStr(_clsitem.id2item(id).c_str(), false);
	for(size_t i=0; i < clsWeight->size(); i++){
		out.writeDbl(clsWeight->at(i), i==clsWeight->size()-1);
	}
}
// -----------------------------------------------------------------------------
// 結果出力(// _treeに最終的なtreeがセットされている)
// -----------------------------------------------------------------------------
void kgBonsai::resultOut(void)
{
	if(_tree==NULL){ return ;}

	// パラメータ出力
	outputParam();
	
	// PMML出力はT1を出力
	_tree->restore();
	_tree->writePMML(_oPath+"/model.pmml",_estimation==0);

	// 予測データ出力
	PredictData *pred = _tree->pred();

	kgCSVout _oFile_alpha;
 	_oFile_alpha.open(_oPath+"/alpha_list.csv", _env,_nfn_o);// アルファリスト
	pred->outInfo(&_oFile_alpha);

	if(_estimation==0){ 
		kgCSVout oFile_m;
		kgCSVout oFile_p;
 		oFile_m.open(_oPath+"/model_info.csv", _env,_nfn_o);
 		oFile_p.open(_oPath+"/predict.csv"  , _env,_nfn_o);

		_tree->calinfoOut(_oPath+"/model.txt",_estimation);
		_tree->outMinfo(&oFile_m,_estimation);

		writeFldName_PD(oFile_p);
		int incnt=-1;
		_iFile.seekTop();  
		while(_iFile.read()!=EOF){
			incnt++;
			if(!pred->find_line(incnt)){ continue;}
			writeFld_PD( oFile_p , pred->predict_min(incnt) , pred->weight_min(incnt) );
		}
	}
	else if(_estimation==1||_estimation==2)
	{
		kgCSVout oFile_m_min;
		kgCSVout oFile_m_1se;
		kgCSVout oFile_min;
		kgCSVout oFile_se1;
 		oFile_m_min.open(_oPath+"/model_info_min.csv", _env,_nfn_o);
 		oFile_m_1se.open(_oPath+"/model_info_1se.csv", _env,_nfn_o);
 		oFile_min.open(_oPath+"/predict_min.csv", _env,_nfn_o);
 		oFile_se1.open(_oPath+"/predict_1se.csv", _env,_nfn_o);

		_tree->calinfoOut(_oPath+"/model_min.txt",_estimation);
		_tree->calinfoOut(_oPath+"/model_1se.txt",_estimation,false);

		_tree->outMinfo( &oFile_m_min,_estimation);
		_tree->outMinfo( &oFile_m_1se,_estimation,false);

		int incnt=-1;
		writeFldName_PD(oFile_min);
		writeFldName_PD(oFile_se1);
		_iFile.seekTop();
		while(_iFile.read()!=EOF){
			incnt++;
			if(!pred->find_line(incnt)){ continue;}
			writeFld_PD( oFile_min , pred->predict_min(incnt) , pred->weight_min(incnt));
			writeFld_PD( oFile_se1 , pred->predict_se1(incnt) , pred->weight_se1(incnt));
		}
	}

}
// =============================================================================
// ツリー評価関連
// =============================================================================
// -----------------------------------------------------------------------------
// ツリー評価（テストサンプル）
// -----------------------------------------------------------------------------
PredictData * kgBonsai::runTestSample(Tree * orgTree ,vector<double> & alphaList)
{
	int psize=_clsitem.getsplitSize(0);
	PredictData * preData 
		= new PredictData(
				alphaList,orgTree->lfcList(alphaList),_clsitem.getsplitSize(1),_clsitem.itmSz()
			);

	Pointer pointer(psize);	
	for(int i=0;i<psize;i++){ pointer.push(_clsitem.getsplitData(0,i),1); }
	
	Tree * tree = makeMaxTree(pointer);
	tree->cal_alpha();
	
	for(vector<double>::iterator it=alphaList.begin();it!=alphaList.end();it++){
		tree->alphaPRB(*it);
		for(int is=0 ; is<_clsitem.getsplitSize(1) ;is++){
			vector<double> clsW( _clsitem.itmSz(),0);
			int linno =  _clsitem.getsplitData(1,is);
			int predict = tree->predictTree( linno , &clsW );
			preData->infoSet(*it , linno , clsW , predict , predict!= _clsitem.getVal(linno) );
		}
	}
	//最小エラー決定
	preData->cal_min();

	return preData;
	
}
// -----------------------------------------------------------------------------
// ツリー評価（クロスバリデーション）
// -----------------------------------------------------------------------------
PredictData * kgBonsai::runCrossValidation(Tree * orgTree ,vector<double> & alphaList)
{
	PredictData * preData 
		= new PredictData(
				alphaList,orgTree->lfcList(alphaList),_clsitem.traSz(),_clsitem.itmSz()
			);

	for (int i=0 ; i < int(_separate) ; i++){
		int psize=_clsitem.traSz() -_clsitem.getsplitSize(i);
		Pointer pointer_cv(psize);
		for (int j=0 ; j < int(_separate) ; j++){
			if(i!=j){ 
				for(int k=0;k<_clsitem.getsplitSize(j);k++){
					pointer_cv.push(_clsitem.getsplitData(j,k),1);
				}
			}
		}
		Tree * tree = makeMaxTree(pointer_cv);
		tree->cal_alpha();

		for(vector<double>::iterator it=alphaList.begin();it!=alphaList.end();it++){
			tree->alphaPRB(*it);
			for(int is=0 ; is<_clsitem.getsplitSize(i) ;is++){
				vector<double> clsW( _clsitem.itmSz(),0);
				int linno =  _clsitem.getsplitData(i,is);
				int predict = tree->predictTree( linno , &clsW );
				preData->infoSet(*it , linno , clsW , predict, predict!= _clsitem.getVal(linno) );
			}
		}
	}
	//最小エラー決定
	preData->cal_min();

	return preData;
}
// -----------------------------------------------------------------------------
// ツリー評価（トレーニングデータ）
// -----------------------------------------------------------------------------
PredictData * kgBonsai::runTraning(Tree * orgTree,vector<double> & alphaList )
{
	PredictData * preData 
		= new PredictData(
				alphaList,orgTree->lfcList(alphaList),_clsitem.traSz(),_clsitem.itmSz()
			);
	
	for(vector<double>::iterator it=alphaList.begin();it!=alphaList.end();it++){
		orgTree->alphaPRB(*it);
		for(int linno=0;linno<_clsitem.traSz();linno++){
			vector<double> clsW( _clsitem.itmSz(),0);
			int predict = orgTree->predictTree( linno , &clsW );
			preData->infoSet(*it , linno , clsW , predict, predict!= _clsitem.getVal(linno) );
		}
	}
	preData->cal_min();
	preData->set_evaluation(_alpha);
	
	return preData;
}
// -----------------------------------------------------------------------------
// ツリー評価（テストサンプルORクロスバリデーションORトレーニングデータ）
// -----------------------------------------------------------------------------
PredictData * kgBonsai::runEstimate(Tree * orgTree ,vector<double> & alphaList)
{
	     if(_estimation==1){ return runTestSample(orgTree,alphaList); }
	else if(_estimation==2){ return runCrossValidation(orgTree,alphaList); }
	return runTraning(orgTree,alphaList);

}
// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgBonsai::run(void) try 
{
	if(_args.toBool("-predict")){
		// 予測モード
		setArgsPredict1(); 
		readModel();
		dataSetPredict();
		_tree->alphaPRB(_alpha);
		predict();
	}else{
		// モデル作成モード
		setArgs(); 
		dataSet();
		makeTree();
		resultOut();
		
	}
	successEnd();
	return 0;

// 例外catcher
}catch(kgError& err){
	errorEnd(err);
	return 1;
}catch(...){
	errorEnd();
	return 1;
}


int main(int argc, const char *argv[]) try{

  kgEnv    env;
  kgBonsai kgmod;
  
  for(int i=1; i<argc; i++){
    if(0==strcmp("--help",argv[i]) || 0==strcmp("-help",argv[i])){
      cout << kgmod.doc() << endl;
	    exit(0);
    }
		if(0==strcmp("--version",argv[i]) || 0==strcmp("-version",argv[i])){
			cout << kgmod.lver() << endl;
			exit(0);
    }
  }

  kgmod.init(argc, argv, &env);
  return kgmod.run();

}catch(...){
  return 1;
}


