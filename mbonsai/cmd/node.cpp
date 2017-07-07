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
#include "node.h"

using namespace kglib ;

const double Node::_Val[] = {  0,  0.001, 0.005, 0.01, 0.05, 0.10, 0.20, 0.40, 1.00};
const double Node::_Dev[] = {100.0,3.09,  2.58,  2.33, 1.65, 1.28, 0.84, 0.25, 0.00};
const int	Node::_ValLMT = 9;
const int	Node::_DevLMT = 9;
int	Node::_outcnt = 0;

//--------------------------------------------------------------------
// 枝苅り用 予測分類誤差計算(refered from C4.5)
//--------------------------------------------------------------------
double Node::calEBPerr(double NN,double e ,double cf)
{
	double coeff=0;
	int pos=1;
	
	for(int i=0 ; i<_ValLMT ; i++){
		if(cf<=_Val[i]) {
			pos=i;
			break;
		}
	}

	coeff = _Dev[pos-1]  
					+ (_Dev[pos]-_Dev[pos-1]) * (cf-_Dev[pos-1]) 
					/ (_Val[pos]-_Val[pos-1]);
	coeff=coeff * coeff;

	if( e < 1E-6 ){
		if(cf==0)	{ return NN;}
		else			{ return NN * ( 1 - exp( log(cf)/NN ) ) ;}
	}
	else if(e < 0.9999){
		double val0 =0;
		if(cf== 0)	{ val0= NN; }
		else				{ val0= NN * ( 1 - exp( log(cf)/NN ) );}
		return val0 + e * ( calEBPerr(NN,1.0,cf) - val0 ); 
	}
	else if ( e+0.5>=NN ){
		return 0.67 * (NN-e);
	}
	else{
		return NN 
					* (( e + 0.5 + coeff/2 + sqrt( coeff* ( (e+0.5) * (1-(e+0.5)/NN) +coeff/4 ) ) ) 
					/ (NN+coeff) ) - e; 
	}
	return NN;
}

//--------------------------------------------------------------------
//		リーフサイズ集計
//--------------------------------------------------------------------
int Node::leafCnt(void){
	int cnt=0;
	if( 0==_nodeType || _pruned ){ return 1; }
	cnt  = _uNode->leafCnt();
	cnt += _mNode->leafCnt();
	return cnt;
}

//--------------------------------------------------------------------
//	各ノードのエラー数を予測分類誤差で計算する。
//--------------------------------------------------------------------
void Node::setErrCnt(double cf)
{
	for(size_t i=0 ; i <_cls->itmSz() ; i++){
		if( i!=_clsNo ){ _errCnt += _cond.errCnt(i); }
	}
	_errNode = calEBPerr(_cond.total(),_errCnt,cf) + _errCnt;
	//リーフならばリターン
	if( 0 == _nodeType ){ return; }
	_uNode->setErrCnt(cf);
	_mNode->setErrCnt(cf);
}

//--------------------------------------------------------------------
// resubstitution ERROR計算
//--------------------------------------------------------------------
void Node::cal_rsERR(void){

	double errCnt=0;		// このノードのエラー数
	for(size_t i=0 ; i <_cls->itmSz() ; i++){
		if( i!=_clsNo ){ errCnt += _cond.errCnt(i); }
	}
	_reERR = (errCnt/_cond.total())*(_cond.total()/_traCnt);
}

//--------------------------------------------------------------------
//	各リーフのエラー数を予測分類誤差で計算する。
//--------------------------------------------------------------------
double Node::setErrLeaf(void)
{
	//:#ノードの場合はリーフとノードのerrを同じにしておく
	if( 0 == _nodeType ){
		_errLeaf = _errNode;
		_score = _errNode - _errLeaf ;

		return _errNode;
	}
	_errLeaf = _uNode->setErrLeaf() + _mNode->setErrLeaf();
	_score = _errNode - _errLeaf ;
	return _errLeaf;
}

//--------------------------------------------------------------------
//  予測分類誤差による枝苅り実行
//--------------------------------------------------------------------
void Node::pruneEBP(double cf)
{
	if( 0==_nodeType ){ return; } 
	if( _errLeaf>_errNode ){
		_pruned=true;
		return;
	}
	_uNode->pruneEBP(cf);
	_mNode->pruneEBP(cf);
	return ;
}

//--------------------------------------------------------------------
// ノード内最小ケース数(optMIN.val)に満たないノードは枝刈り			
//--------------------------------------------------------------------
void Node::pruneByNSz(int mincase)
{
	if( 0==_nodeType ||  _pruned ){ return;}
	if( 1==_uNode->nodeType() and !_uNode->pruned() ){
		_uNode->pruneByNSz( mincase );
	}
	if( 1==_mNode->nodeType() and !_mNode->pruned() ){
		_mNode->pruneByNSz( mincase );
	}
	// 子が両方ともリーフもしくはPrunedNodeの場合、両方とも同じクラスなら枝苅り*/
	if(_cond.total()<mincase){ _pruned=true; }
	return;
}
//--------------------------------------------------------------------
// * リーフ最小ケース数()に満たないノードは枝刈り
//--------------------------------------------------------------------
void Node::pruneByLSz(int minLSz)
{
	if( 0==_nodeType || _pruned ){ return; }
	if( _uNode->total() < minLSz ){ 
		_pruned=true;
		return;
	}
	else{
		_uNode->pruneByLSz(minLSz);
	}

	if( _mNode->total() < minLSz ){
		_pruned=true;
		return;
	}
	else{
		_mNode->pruneByLSz(minLSz);
	}
}
//--------------------------------------------------------------------
// 枝苅り 同じクラスを予測する二の葉を持つノードは枝苅り
//--------------------------------------------------------------------
void Node::pruneSameCls(void){
	if( 0==_nodeType ){ return; }
	
	if( 1==_uNode->nodeType() && !_uNode->pruned() ){ _uNode->pruneSameCls(); }
	if( 1==_mNode->nodeType() && !_mNode->pruned() ){ _mNode->pruneSameCls(); }

  // 子が両方ともリーフもしくはPrunedNodeの場合、両方とも同じクラスなら枝苅り
	if( (0==_uNode->nodeType() || _uNode->pruned()) 
			&& ( 0==_mNode->nodeType() || _mNode->pruned() )
			&& ( _uNode->clsNo() == _mNode->clsNo() ) ){
		_pruned=true;
	}
	return;
}

//--------------------------------------------------------------------
//		枝狩り実行	
//--------------------------------------------------------------------
void Node::pruneOnNode(void)
{
	_pruned = true;
	if( _brother == NULL || _brother->pruned() || _brother->nodeType()==0 ){
		_parentWillBePushed=true;
	}
	else{
		_parentWillBePushed=false;
	}
}

//--------------------------------------------------------------------
//		新たな候補挿入	
//--------------------------------------------------------------------
void Node::pushNewLeaf(priority_queue<Node*, vector<Node*>, ndSort> *lpq)
{
	if( _parentWillBePushed ){ lpq->push(_parent); }
}


//--------------------------------------------------------------------
//		ノードの深さレベルセット
//--------------------------------------------------------------------
int Node::setlevel(int lvl ,int *dp)
{
	_level=lvl;
	if( _level > *dp ){ *dp=_level;}  
	// リーフならばリターン
	if( 0==_nodeType || _pruned){ return 1;}

	int leaf_u = _uNode->setlevel(lvl+1,dp);
	int leaf_m = _mNode->setlevel(lvl+1,dp);
		
	return leaf_u + leaf_m;
}


//--------------------------------------------------------------------
//		ノードの深さレベルセット
//--------------------------------------------------------------------
/*
int Node::lFcnt(void)
{
	// リーフならばリターン
	if( 0==_nodeType || _pruned){ return 1;}

	int leaf_u = _uNode->lFcnt();
	int leaf_m = _mNode->lFcnt();
		
	return leaf_u + leaf_m;
}
*/

int Node::lFcnt(double alpha){
	if( 0==_nodeType || _pruned )    { return 1; }
	if( _alpha!=0 && alpha > _alpha ){ return 1; }
	int leaf_u = _uNode->lFcnt(alpha);
  int leaf_m = _mNode->lFcnt(alpha);
	return leaf_u + leaf_m;
}


//--------------------------------------------------------------------
// ノード予測
// clsWeightに結果をセットしている。
//--------------------------------------------------------------------
void Node::predictNode( vector<double> *clsWeight, int recNo,double weight)
{
	// リーフに到達すれば、weightをclsWeightテーブルにセットする。
	if( 0==_nodeType || -1==_nodeType  || _pruned){
		for(size_t i=0 ; i <_cls->itmSz() ; i++){
			clsWeight->at(i) = _cond.tCnt(i)/_cond.total();
		}

		return ;
	}
	// 現在ノードの条件項目がデータ上でNULLの場合は 件数によるweightをかけて再帰処理*/
	if( _attr->null(recNo) ){
		double mWeight = weight * _cond.mRatio();
		double uWeight = weight * ( 1-_cond.mRatio() );
		_mNode->predictNode( clsWeight ,recNo ,mWeight);
		_uNode->predictNode( clsWeight ,recNo ,uWeight);
	}
	else{
		if( _attr->chkMatch(_cond.baseVal(),recNo)){ // _cond.baseVal: 分岐ルールのしきい値
			_mNode->predictNode(clsWeight,recNo,weight);
		}
		else{
			_uNode->predictNode(clsWeight,recNo,weight);
		}
	}
	return ;
}

int Node::leafInfo(double *leaferr){

	int cnt=0;
	if( 0==_nodeType || _pruned ){ 
		*leaferr = *leaferr + _reERR;
		return 1; 
	}
	cnt  = _uNode->leafInfo(leaferr);
	cnt += _mNode->leafInfo(leaferr);
	return cnt;
}

double Node::cal_alphasub(void){
	
	double leaferr=0;
	double leafNum =	leafInfo(&leaferr);
//	cerr << "a " << _reERR << " "<< leaferr << " " << (leafNum-1.0) << " " << (_reERR-leaferr)/(leafNum-1.0) << endl;
	return (_reERR-leaferr)/(leafNum-1.0);
	
}

// 全ノードでのアルファを求める[g(t)を求める]
void Node::cal_alpha(double *min ,vector<Node*> *nodelist){
	if(_uNode!=NULL && 1==_uNode->nodeType() && !_uNode->pruned()){
		_uNode->cal_alpha(min,nodelist);
	}
	if (_mNode!=NULL && 1==_mNode->nodeType() && !_mNode->pruned()){
		_mNode->cal_alpha(min,nodelist);
	}
	//このノードでアルファ計算
	double res =  cal_alphasub();
	if(abs(res-*min)< 1.0e-10 ){
		nodelist->push_back(this);
	}
	else if(res<*min){
		*min=res;
		nodelist->clear();
		nodelist->push_back(this);
	}

}

void Node::setLPQ(priority_queue<Node*, vector<Node*>, ndSort> *lpq)
{
	if( 0==_nodeType){ return; }
	if( ( _uNode->nodeType() || _uNode->pruned() ) &&
			( _mNode->nodeType() || _mNode->pruned() )	){
		lpq->push(this);
	}
	_uNode->setLPQ(lpq);
	_mNode->setLPQ(lpq);
}

bool Node::updCondition(Condition &cond,fldItem * attr)
{
	if( _cond.splitAfter() > cond.splitAfter()){
		_cond.copy(cond);
		_attr=attr;
		return true;
	}
		return false;
}

bool Node::leafCheck(int minleaf)
{
	//all NULL
	if( _cond.nullall() ){
		_nodeType=0;
		return true;
	}

	//改善なし
	if( _cond.splitCheck() ){
		_nodeType=0;
		return true;
	}

	//ゼロチェック
	if( _cond.zeroCheck() ){
		_nodeType=0;
		return true;
	}

	if( _cond.totalCheck(minleaf) ){
		_nodeType=0;
		return true;
	}
	
	_nodeType=1;
	return false;
}






