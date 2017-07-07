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
#include <queue>
#include <cstdio>
#include "flditem.h"
#include "flditem/clsitem.h"
#include "condition.h"
namespace kglib {

struct ndSort;

class Node
{
	static const double  _Val[];
	static const int     _ValLMT;
	static const double  _Dev[];
	static const int     _DevLMT;
	static int 			_outcnt;

	ClsItem* _cls;
	Condition _cond;

	Node* _parent;
	Node* _uNode;
	Node* _mNode;
	Node* _brother;

	bool  _pruned;
	int 	_level;

	fldItem * _attr;
	int _clsNo;    // dominat class
	int _nodeType; // 0:Leaf , 1:Node ,-1:未定義
	double _errNode;	// このノードのエラー数
	double _errLeaf;	// このノード以下のエラー数
	double _errCnt;		// このノードのエラー数
	bool _parentWillBePushed;
	double _score;					// 枝狩り時チェックした自ノードと子のノードの差
	double _scoreRate;	// 枝狩り時チェックした自ノードと子のノードの差*/
	double _reERR; //resubstitution ERROR
	double _alpha; //アルファ
	size_t _traCnt;


	//	self.__mulCls=0			#/*最大件数をもつClassが複数あるかどうかフラッグ*/
	//	self.__costNode=0		#/*このノードのコスト*/
	//	self.__costLeaf=0		#/*このノード以下のコスト*/

	void brother(Node* node){_brother=node;}
	bool pruned(void){return _pruned;}

	
	int nodeType(void){return _nodeType;}
	double reERR(void){return _reERR;} //resubstitution ERROR


	//--------------------------------------------------------------------
	// 枝苅り用 予測分類誤差計算(refered from C4.5)
	//--------------------------------------------------------------------
	double calEBPerr(double NN,double e ,double cf);


	public:
	void alphaset(double alpha,bool pruned = true){
		_alpha  = alpha;
		_pruned = pruned;
	}
	
	Node(ClsItem* cls,size_t tracnt ,Node *parent):
		_cls(cls),_cond(_cls),_parent(parent),_uNode(NULL),_mNode(NULL),_brother(NULL),
		_pruned(false),_level(0),_attr(NULL),_clsNo(-1),_nodeType(0),
		_errNode(0),_errLeaf(0),_errCnt(0),_score(0),_scoreRate(1),_alpha(0),_traCnt(tracnt)
		{}
	~Node(){
		if(_uNode!=NULL){ delete _uNode;}
		if(_mNode!=NULL){ delete _mNode;}
	}
	void setClass(fldItem * attr){_attr=attr;}
	void total_set(double total){ _cond.total_set(total);}
	void cal_mratio(void){
		_cond.mratio_set(_mNode->total()/total());
	}
	void tcnt_set(map<kgstr_t,int>& tcntset ){ _cond.tcnt_set(tcntset);}
	
	bool isleaf(void){
		if ( 0==nodeType() || pruned()) { return true ;}
		return false;
	
	}
	// 両方ともリーフならtrue
	bool childrenLeaf(void){
	
		if ( 0==nodeType() || pruned()) { return true ;}
		
		if( ( _uNode == NULL || 0==_uNode->nodeType() || _uNode->pruned() ) 
			&& ( _mNode == NULL || 0==_mNode->nodeType() || _mNode->pruned()) )
		{
			return true;
		}
		return false;
	}
	
	int leafInfo(double *leaferr);
	double cal_alphasub(void);
	void cal_alpha(double *min ,vector<Node*> *nodelist);

	
	//--------------------------------------------------------------------
	//		リーフサイズ集計
	//--------------------------------------------------------------------
	int leafCnt(void);

	//--------------------------------------------------------------------
	//	各ノードのエラー数を予測分類誤差で計算する。
	//--------------------------------------------------------------------
	void setErrCnt(double cf);

	//--------------------------------------------------------------------
	//	各リーフのエラー数を予測分類誤差で計算する。
	//--------------------------------------------------------------------
	double setErrLeaf(void);
	
	//--------------------------------------------------------------------
	//  予測分類誤差による枝苅り実行
	//--------------------------------------------------------------------
	void pruneEBP(double cf);
	
	//--------------------------------------------------------------------
	// ノード内最小ケース数(optMIN.val)に満たないノードは枝刈り			
	//--------------------------------------------------------------------
	void pruneByNSz(int mincase);

	//--------------------------------------------------------------------
	// * リーフ最小ケース数()に満たないノードは枝刈り
	//--------------------------------------------------------------------
	void pruneByLSz(int minLSz);


	//--------------------------------------------------------------------
	// 枝苅り 同じクラスを予測する二の葉を持つノードは枝苅り
	//--------------------------------------------------------------------
	void pruneSameCls(void);
	
	//--------------------------------------------------------------------
	//		枝狩り実行	
	//--------------------------------------------------------------------
	void pruneOnNode(void);

	//--------------------------------------------------------------------
	//		新たな候補挿入	
	//--------------------------------------------------------------------
	void pushNewLeaf(priority_queue<Node*, vector<Node*>, ndSort> *lpq);

	//--------------------------------------------------------------------
	//		ノードの深さレベルセット
	//--------------------------------------------------------------------
	int setlevel(int lvl ,int *dp);
	int lFcnt(double alpha);
	void setLPQ(priority_queue<Node*, vector<Node*>, ndSort> *lpq);
		
		
	bool updCondition(Condition &cond,fldItem * attr);

	void dominantClass(void){ 
		_clsNo = _cond.maxClassNo(); 
	}

	void setDominantClass(char* clsname){ 
		_clsNo = _cls->item2id(clsname);
	}
	
	bool leafCheck(int minleaf);

	void nodeSet(Node *node, char type)
	{
				 if( type == 'm' ){ _mNode=node; }
		else if( type == 'u' ){ _uNode=node; }
		return ;	
	}
	void brotherSet(void){
		_mNode->brother(_uNode);
		_uNode->brother(_mNode);
	}

	void predictNode( vector<double> *clsWeight, int recNo,double weight);


	int mCount(void){ return _cond.mCount();}
	int uCount(void){ return _cond.uCount();}
	double total(void){ return _cond.total();}


	double mRatio(void){  return _cond.mRatio();}
	baseInfo baseVal(void){  return  _cond.baseVal();}
	double errgain(void) const{ return _cond.errgain();} 
	int clsNo(void){ return _clsNo;} 

	void baseVal(RegPtn* rp) { 	_cond.baseVal(rp); _nodeType=1;}
	void baseVal(vector<char> clist) { 	_cond.baseVal(clist);  _nodeType=1;}
	void baseVal(double th) { _cond.baseVal(th); _nodeType=1; }

	void baseVal(kgstr_t cstr) { _nodeType=1; }
	void baseVal(vector<kgstr_t> clist) {  _nodeType=1; }


	//--------------------------------------------------------------------
	// 枝狩りスコア関係
	//--------------------------------------------------------------------
	//	スコアの最小、最大を求める
	//--------------------------------------------------------------------
	void calScore(double *min,double *max)
	{
		if( 0==_nodeType || _pruned ){ return ;}
		if( _score<*min ) { *min = _score; }
		if( _score>*max ) { *max = _score; }
		_uNode->calScore(min,max);
		_mNode->calScore(min,max);
	}

	void cal_T1	(void){
		if( 0==_nodeType || _pruned ){ return ;}
		_uNode->cal_T1();
		_mNode->cal_T1();
		if( (0==_uNode->nodeType() || _uNode->pruned() ) 
			&& (0==_mNode->nodeType() || _mNode->pruned()) )
		{
			if(abs(_reERR -(_uNode->reERR() + _mNode->reERR()))< 1.0e-10 ){
				delete _uNode; _uNode=NULL;
				delete _mNode; _mNode=NULL;
				_nodeType=0;
			}
		}
	}

	void alphaPRB(double alpha){
		//cerr << "a=" << alpha << " _a=" <<  _alpha << " nd=" << _nodeType << " prb="  << _pruned << endl;
		if( 0==_nodeType || _pruned ){ return ;}
		if( _alpha!=0 && alpha > _alpha ){
			_pruned=true;	
			return;
		}
		_uNode->alphaPRB(alpha);
  	_mNode->alphaPRB(alpha);
  	return ;
	}

	void restoreNode(void){
		if(_uNode!=NULL){ _uNode->restoreNode();}
		if(_mNode!=NULL){ _mNode->restoreNode();}
		_pruned = false;
	}


	void cal_rsERR(void);

	//--------------------------------------------------------------------
	//		score_rate min〜maxを１としてscoreを0-1の値に変換
	//--------------------------------------------------------------------
	void calRateScore(double min,double max){
		if(min == max){ _scoreRate=0; }
		else{ _scoreRate = (_score-min)/(max-min); }
		if( 0==_nodeType || _pruned ){ return; }
		_uNode->calRateScore(min,max);
		_mNode->calRateScore(min,max);
	}


	bool operator<(const Node& other){
		return errgain() < other.errgain();
	}
	bool operator==(const Node& other){
		return errgain() == other.errgain();
	}



	//--------------------------------------------------------------------
	// 決定木のテキスト形式出力
	// if($ブランド系列 has 22)
	//   then $離反=yes (hit/sup)=(4/4)
	//     else $離反=no (hit/sup)=(4/4)
	//--------------------------------------------------------------------
	void output(int level,char trl,ostream* ofp=&cout)
	{
		for(int i=0 ; i<level ; i++){ *ofp << "  "; }
		if(trl=='m')     { *ofp << "then ";}	
		else if(trl=='u'){ *ofp << "else ";}	
		// leaf
		if( _nodeType==0 || _pruned ){
			double hitCnt = _cond.tCnt(_clsNo);
			double supCnt = _cond.total();
			*ofp << "$" << _cls->name() << "=" << _cls->id2item(_clsNo) << " (hit/sup)=(" << hitCnt << "/" << supCnt <<  ")" << endl;
			//printf("$%s=%s (hit/sup)=(%g/%g)\n",
			//				_cls->name().c_str(),_cls->id2item(_clsNo).c_str() ,hitCnt,supCnt);
			return;
		}		
		if( _nodeType==1 ){ _attr->condPrint(baseVal(),ofp); }
		if( _mNode!=NULL ){ _mNode->output(level+1,'m',ofp); }
		if( _uNode!=NULL ){ _uNode->output(level+1,'u',ofp); }
	}

	//--------------------------------------------------------------------
	// 決定木表示(デバッグ用)
	// if($ブランド系列 has 22)
	//   then $離反=yes (hit/sup)=(4/4)
	//     else $離反=no (hit/sup)=(4/4)
	//--------------------------------------------------------------------
	//#define showIndent(indent) for(int ind=0; ind<indent; ind++) printf(" ");
	//void show(int level,char trl)
	//{
	//	showIndent(level);
//		printf("node: trl=%c nodeType=%d pruned=%d\n",trl,_nodeType,_pruned);
//		_cond.show(level+2);
//
		// leaf
//		if( _nodeType==0 ) return;

//		if( _mNode!=NULL ){ _mNode->show(level+1,'m'); }
//		if( _uNode!=NULL ){ _uNode->show(level+1,'u'); }
//	}
		
	void tabPrint(FILE* fp,int level){ 
		for(int i=0 ; i<level ; i++){ fprintf(fp,"\t"); }
	}
	
	void writeSubPMML(FILE *fp ,int level=0,char trl='t')
	{
		if( _nodeType==0 || _pruned ){ return; }
		_attr->writePMML(fp ,_cond.baseVal(),level,trl);	
	}


	void writePMML(FILE *fp ,int level=0,char trl='t')
	{
		tabPrint(fp,level+2);
		fprintf(fp,"<Node id=\"%d\" score=\"%s\" recordCount=\"%g\" >\n" ,
						_outcnt,_cls->id2item(_clsNo).c_str(),_cond.total() );
//		tabPrint(fp,level+3);
//		fprintf(fp,"<Extension extender=\"KGMOD\" name=\"pruning\" value=\"%f\"/>\n",_scoreRate); 
		tabPrint(fp,level+3);
		fprintf(fp,"<Extension extender=\"KGMOD\" name=\"complexity penalty\" value=\"%f\"/>\n",_alpha);
		_outcnt++;
		if( trl=='t' ){
			tabPrint(fp,level+3);
			fprintf(fp, "<True/>\n" );
		}
		else{
			_parent->writeSubPMML(fp,level+1,trl);
		}
		for(int i=0 ; i<_cls->itmSz() ; i++){
			tabPrint(fp,level+3);
			fprintf(fp,"<ScoreDistribution value=\"%s\" recordCount=\"%g\"/>\n" ,
				_cls->id2item(i).c_str() , _cond.muCnt(i) ) ;
		}
		
		if( 0==_nodeType || _pruned ){
			tabPrint(fp,level+2);
			fprintf(fp,"</Node>\n");
			return;
		}
		if( _mNode!=NULL ){
			_mNode->writePMML(fp,level+1,'m');
		}
		if( _uNode!=NULL ){
			_uNode->writePMML(fp,level+1,'u');
		}
		tabPrint(fp,level+2);
		fprintf(fp, "</Node>\n");

	}


};

struct ndSort{
	bool operator()(Node* q1,Node* q2) const {
		return *q1 < *q2;
	}
};

}

