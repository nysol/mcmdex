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
#include <fstream>
#include "tree.h"

Node* Tree::makeTree(Pointer &pointer ,Node *pNode )
{
	fldItem *better_class=NULL;
	Node * now = new Node( _cls , _traCnt , pNode );

	// better_classにnow nodeを分割する最適項目(fldItem*型)が入る
	// conditionのsplitAfter(分割後の２つの節点のエントロピーの加重平均.これはgainに変更すべき)が最も大きい項目が最適項目
	better_class = _attr->setCondition(now,pointer,_cls);

	// クラス決定		
	now->dominantClass();
	// リーフチェック		
	if( now->leafCheck(_lFSz)){
		now->cal_rsERR();
		return now;
	}
	Pointer mptn(now->mCount()); // mCount: ルールにマッチしたサンプル数
	Pointer uptn(now->uCount());

	double mratio = now->mRatio(); // now nodeにおけるパターンにマッチするサンプルの割合 match/total at the node
	baseInfo baseval = now->baseVal(); // basevalには系列項目の場合、そのノードを分岐する最適なregexが格納されている(数値の場合は分割閾値、カテゴリの場合はカテゴリリスト)

	for(int i=0 ; i<pointer.size() ; i++){
		int lineNo = pointer.atLnum(i); // lineNoこのnodeに分割されたsanple番号
		if( better_class->null(lineNo) ){ // lineNoのsampleがnullならmratioに応じて、その1件をmatchとunmatchに分配する
			mptn.push(lineNo,pointer.atLratio(i)*mratio);
			uptn.push(lineNo,pointer.atLratio(i)*(1-mratio));
		}
		else{
			if( better_class->isMatch( baseval , lineNo)){ // lineNoの系列データがbasevalに登録されたregexにマッチするかどうか
				mptn.push(lineNo,pointer.atLratio(i));
			}
			else{
				uptn.push(lineNo,pointer.atLratio(i));
			}
		}
	}
	now->nodeSet( makeTree(mptn,now) ,'m' );
	now->nodeSet( makeTree(uptn,now) ,'u' );
	now->brotherSet();
	//resubstitution ERROR計算
	now->cal_rsERR();
	
	return now;
}

//------------------------------------------------------------------------------
// 決定木からクラス予測 (type=falseでテストデータで予測)
//------------------------------------------------------------------------------		
int Tree::predictTree(int recNo , vector<double>* clsW)
{
	int maxCls=0;
	double maxClsWeight=0;
	vector<double> clsWeight(_cls->itmSz(),0);
	vector<double>* vcls;

	// treeをたどり、クラス別の
	if(clsW){ vcls = clsW;}
	else{ vcls = &clsWeight;}

	_topNode->predictNode(vcls,recNo,1);	
	
	for(int i=0 ; i<_cls->itmSz(); i++){
		if( maxClsWeight<vcls->at(i) ){
			maxCls=i;
			maxClsWeight=vcls->at(i);
		}
	}
	return maxCls;	
}

//------------------------------------------------------------------------------
// 結果表データ set
//------------------------------------------------------------------------------		
void Tree::setClsTbl(void)
{
	clsTbl * clstbl; 
	clstbl=&_trnRsl;
	clstbl->clear();
	for(int i=0 ;i<_traCnt ;i++){
		int actualCls  = _cls->getVal(i);
		int predictCls = predictTree(i);
		clstbl->addcnt(actualCls,predictCls,1);
	}
}
//------------------------------------------------------------------------------
// 結果表データ set
//------------------------------------------------------------------------------		
void Tree::setClsTblFromTest(bool min)
{
	clsTbl * clstbl; 
	map<int,int> *predictD;
	if(min){ 
		clstbl=&_tstRsl_MIN;
		predictD =  _pred->predict_min();
	}
	else	 { 
		clstbl=&_tstRsl_1SE;
		predictD =  _pred->predict_se1();
	}
	
	clstbl->clear();
	for ( map<int,int>::iterator it = predictD->begin() ; it!=predictD->end(); it++){
		int actualCls  = _cls->getVal(it->first);
		int predictCls = it->second;
		clstbl->addcnt(actualCls,predictCls,1);
	}
}


//------------------------------------------------------------------------------
// リーフ数指定枝狩り
//------------------------------------------------------------------------------		
void Tree::pruneLFC(int leafC){
	priority_queue<Node*, vector<Node*>, ndSort> lpq;
	vector<Node*> nodelist;

	_topNode->setLPQ(&lpq);
	
	while( leafC>_lFNum ){
		nodelist.clear();
		if ( lpq.size()==0 ) break;
		Node* nnode = lpq.top(); 
		lpq.pop();
		while(1){
			nodelist.push_back(nnode);
			if ( lpq.size()==0 || nnode!=lpq.top()){ break;}
			nnode = lpq.top();
			lpq.pop();
		}
		for(int i=0; i<nodelist.size() ;i++ ){
			nodelist[i]->pruneOnNode();
			leafC--;
		}			
		for(int i=0; i<nodelist.size() ;i++ ){
			nodelist[i]->pushNewLeaf(&lpq);
		}			
	}
}
		
//------------------------------------------------------------------------------
// 枝狩り リーフ数指定の場合とそれ以外の場合(Error-based Pruning)で処理をかえる
//------------------------------------------------------------------------------		
void Tree::pruneTree(void){
	
	// 後のscore_rate計算のためにerrcntを計算しておく
	_topNode->setErrCnt(_CF);
	_topNode->setErrLeaf();
	// リーフ数指定(self.__lfc)による枝刈り
	if( _lFNum > 0 ) { pruneLFC(_topNode->leafCnt()); }
	else						 { _topNode->pruneEBP(_CF); }

	if ( _lFSz>0 )	{ _topNode->pruneByLSz(_lFSz   );}
	_topNode->pruneSameCls();
}
	
//------------------------------------------------------------------------------
// 決定木作成メインルーチン
//------------------------------------------------------------------------------
void Tree::decisionTree(void)
{
	Pointer pointer(_traCnt);
	for(int i=0 ; i<_traCnt ; i++){
		pointer.push(i,1);
	}
	_topNode = makeTree(pointer,NULL);
	pruneTree();
	setLevel();
	setClsTbl();
	_trnRsl.calClsTbl();
	_trnRsl.calStats();
}


void Tree::decisionTree(Pointer & pointer)
{
	_topNode = makeTree(pointer,NULL);
}
//------------------------------------------------------------------------------
// Tree情報出力
// typeによって出力内容を変える
//------------------------------------------------------------------------------			
void Tree::calinfoOut(kgstr_t fname , int type , bool evalu )
{
	if(evalu)	{  alphaPRB(_pred->evalu_alpha());}
	else		{ alphaPRB(_pred->se1_alpha());}
	// トレーニングでーたでのデータでの内容出力
	_deep=0;
	setLevel();
	setClsTbl();
	_trnRsl.calClsTbl();
	_trnRsl.calStats();

	//アウトプット(traning)
	ofstream ofp;
	ofp.open(fname.c_str());
	ofp << "[alphabet-index]" << endl;
	_attr->output(&ofp);
	ofp << endl;
	ofp << "[decision tree]" << endl;
	_topNode->output(0,'t',&ofp);
	ofp << endl;
	ofp << "numberOfLeaves=" << _lFcnt << endl;
	ofp << "deepestLevel==" << _deep << endl;
	ofp <<  endl;
	ofp << "[Confusion Matrix by Training]" << endl;
	_trnRsl.output(&ofp);

	//test
	if(type==1||type==2){//1SEでの結果
		clsTbl * clstbl;
		setClsTblFromTest(evalu);
		if(evalu) { clstbl=&_tstRsl_MIN; }
		else 			{ clstbl=&_tstRsl_1SE; }
		clstbl->calClsTbl();
		clstbl->calStats();
		ofp <<  endl;
//				 if( type==1){ ofp << "## estimation by test sample ##" << endl; }
//		else if( type==2){ ofp << "## estimation by cross validation ##" << endl; }
		ofp << "[Confusion Matrix by Estimation]" << endl;
		clstbl->output(&ofp);
		ofp <<  endl;
		ofp << "[Selected Alpha]" << endl;
		if(evalu) { ofp << "alpha: " << _pred->evalu_alpha() << endl; }
		else 			{ ofp << "alpha: " << _pred->se1_alpha() << endl; }
	}

}





//------------------------------------------------------------------------------
// Tree比較
//------------------------------------------------------------------------------			
bool Tree::better(Tree *other){
	if(other==NULL){ return true;}
	if( tcst() < other->tcst()){return true;}
	else if( tcst() > other->tcst()){return false;}
	else{
		if( lFcnt()<other->lFcnt() ){ return true;}
		else										    { return false;}
	}
}

bool Tree::betterPred(Tree *other){
	if(other==NULL){ return true;}
	if(_pred==NULL){ return false;}
	bool f=_pred->better(other->pred());
	return f;
}


//------------------------------------------------------------------------------
// socre計算
//------------------------------------------------------------------------------		
void Tree::calScore(void)
{
	double min = DBL_MAX;
	double max = -DBL_MAX;
	_topNode->calScore(&min,&max);
	if( max!=0 ){ _topNode->calRateScore(min,max);}
}

//------------------------------------------------------------------------------
// 実際には枝狩り
//------------------------------------------------------------------------------		
void Tree::cal_T1(void)
{
	_topNode->cal_T1();
}

void Tree::alphaPRB(double alpha){
	_topNode->alphaPRB(alpha);

}


//------------------------------------------------------------------------------
// アルファを求める
// それぞれのノードにアルファが設定される
// ルートノードのみになったら終了
// 最小値とノード一覧が取得される
//------------------------------------------------------------------------------		
vector<double> Tree::cal_alpha(void)
{
	vector<double> minVec;

	minVec.push_back(0);
	while(!_topNode->isleaf()){
		vector<Node *> nodelist;
		double min = DBL_MAX;
		_topNode->cal_alpha(&min ,&nodelist);
		minVec.push_back(min);
		// node仮削除
		for (size_t i=0 ; i < nodelist.size() ;i++ ){
			nodelist[i]->alphaset(min);
		}
	}
	// node復活(コピーの方がよければコピーに変更すること)
	_topNode->restoreNode();

	for(size_t i=0 ; i<minVec.size()-1; i++){
		minVec[i] = sqrt(minVec[i]*minVec[i+1]);
	}
	minVec[minVec.size()-1]=DBL_MAX;

	return minVec;	
}

map<double,int> Tree::lfcList(vector<double> alphaList)
{
	map<double,int> result;
	for(vector<double>::iterator it=alphaList.begin() ; it!=alphaList.end() ; it++){
		result[*it] = _topNode->lFcnt(*it);
	}
	return result;

}


//------------------------------------------------------------------------------
// Tree情報出力
//------------------------------------------------------------------------------			
void Tree::output(ostream* ofp){
	*ofp << "[alphabet-index]" << endl;
	_attr->output(ofp);

	*ofp << endl;
	*ofp << "[decision tree]" << endl;
	_topNode->output(0,'t',ofp);
	*ofp << endl;
	*ofp << "numberOfLeaves=" << _lFcnt << endl;
	*ofp << "deepestLevel==" << _deep << endl;

	*ofp << "[Confusion Matrix]" << endl;
	*ofp << "## TRAINING DATA ##" << endl;
	_trnRsl.output(ofp);

	if(_pred){
		*ofp << "## SELECTED ALPHA ##" << endl;
		*ofp << "min=" << _pred->min_alpha() << endl;
		*ofp << "SE1=" << _pred->se1_alpha() << endl;
	}

}

//------------------------------------------------------------------------------
// Model情報出力
//------------------------------------------------------------------------------			
void Tree::outMinfo(kgCSVout *out , int type , bool min){

	vector<kgstr_t> nf_a ;
	nf_a.push_back("nobs");
	nf_a.push_back("alpha");
	nf_a.push_back("accuracy");
	nf_a.push_back("totalCost");
	out->writeFldName(nf_a);

	out->writeInt(_traCnt, false );
	
	if(type==0){
		out->writeDbl(_pred->evalu_alpha(), false );
		out->writeDbl(_trnRsl.accRate(), false );
		out->writeDbl(_trnRsl.tcst(), true );	
	}
	else{
		if(min){
			out->writeDbl(_pred->min_alpha(), false );
			out->writeDbl(_tstRsl_MIN.accRate(), false );
			out->writeDbl(_tstRsl_MIN.tcst(), true );

		}
		else{
			out->writeDbl(_pred->se1_alpha(), false );
			out->writeDbl(_tstRsl_1SE.accRate(), false );
			out->writeDbl(_tstRsl_1SE.tcst(), true );
		}	
	}
}


// -----------------------------------------------------------------------------
// 時刻取得
// -----------------------------------------------------------------------------
string Tree::getTime(void)
{
	time_t t;
	struct tm *ltm;
	time(&t);             
	ltm = localtime(&t);  

	char buf[128];
	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d",
		ltm->tm_year+1900, ltm->tm_mon+1, ltm->tm_mday,
		ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	ostringstream ss;
	ss << buf;
	return ss.str();
}
// -----------------------------------------------------------------------------
// PMML出力
// -----------------------------------------------------------------------------
void Tree::writePMML(kgstr_t fname,bool noestimate){
	FILE *fp;
	fp  = fopen(fname.c_str(),"w");
	
	fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp,"<PMML version=\"4.0\">\n");
	fprintf(fp,"\t<Header copyright=\"KGMOD\">\n");
	fprintf(fp,"\t\t<Application name=\"mbonsai\" version=\"1.0\"/>\n");
	fprintf(fp,"\t\t<Timestamp>%s</Timestamp>\n" , getTime().c_str());
	fprintf(fp,"\t</Header>\n");
	
	fprintf(fp,"\t<DataDictionary numberOfFields=\"%d\">\n" , _attr->size()+1);
	_attr->dFldPrint(fp);
	fprintf(fp,"\t\t<DataField name=\"%s\" optype=\"categorical\" dataType=\"string\">\n",_cls->name().c_str() );
	for(int i=0 ; i<_cls->itmSz() ; i++ ){
		fprintf(fp,"\t\t\t<Value value=\"%s\"/>\n",_cls->id2item(i).c_str());
	}
	fprintf(fp,"\t\t</DataField>\n");
	fprintf(fp,"\t</DataDictionary>\n");
	fprintf(fp,"\t<TreeModel functionName=\"classification\" splitCharacteristic=\"binarySplit\">\n");
	if(noestimate){
		fprintf(fp,"\t\t<Extension extender=\"KGMOD\" name=\"alpha\" value=\"%f\"/>\n",_pred->evalu_alpha());	
	}else{
		fprintf(fp,"\t\t<Extension extender=\"KGMOD\" name=\"1SE alpha\" value=\"%f\"/>\n",_pred->se1_alpha());	
		fprintf(fp,"\t\t<Extension extender=\"KGMOD\" name=\"min alpha\" value=\"%f\"/>\n",_pred->min_alpha());	
	}
	fprintf(fp,"\t\t<MiningSchema>\n");
	_attr->infoPrint(fp);
	fprintf(fp,"\t\t\t<MiningField name=\"%s\" usageType=\"predicted\"/>\n" , _cls->name().c_str());
	fprintf(fp,"\t\t</MiningSchema>\n");
	_topNode->writePMML(fp);
	fprintf(fp,"\t</TreeModel>\n");
	fprintf(fp,"</PMML>\n");
	fclose(fp);
}

Node* Tree::setNode(xmlNodePtr  nodep ,Node *pNode ,bool setF )
{
	Node * now = new Node( _cls ,_traCnt, pNode );
	map<kgstr_t,kgstr_t> attr = kglibxml::get_attr(nodep);
	map<kgstr_t,int> cnt_set;

	now->setDominantClass(const_cast<char*>(attr["score"].c_str()));

	now->total_set(atof(attr["recordCount"].c_str()));

	bool match=true;
	xmlNodePtr nodetp = nodep->children;
	
	while(nodetp){
		if(xmlStrEqual(nodetp->name,(xmlChar *)"Node")){
			if(match){
				now->nodeSet( setNode(nodetp,now,match) ,'m' );
				match=false;
			}else{
				now->nodeSet( setNode(nodetp,now,match) ,'u' );	
				//martioセット
				now->cal_mratio();	
			}
		}
		else if(xmlStrEqual(nodetp->name,(xmlChar *)"ScoreDistribution")){
			map<kgstr_t,kgstr_t> attr_sc = kglibxml::get_attr(nodetp);
			cnt_set[attr_sc["value"]] = atoi(attr_sc["recordCount"].c_str());
		}
		else if(xmlStrEqual(nodetp->name,(xmlChar *)"Extension")){//ptn
			map<kgstr_t,kgstr_t> attr_e = kglibxml::get_attr(nodetp);
			map<kgstr_t,kgstr_t>::const_iterator itor = attr_e.find("name");
			if( itor!=attr_e.end() && (*itor).second == "complexity penalty"){
				now->alphaset( atof(attr_e["value"].c_str()),false);
			}
			else if( itor!=attr_e.end() && (*itor).second == "patternPredicate" &&setF){		
				xmlNodePtr nodettp = nodetp->children;
				while(nodettp){
					if(xmlStrEqual(nodettp->name,(xmlChar *)"SimplePredicate")){
						RegPtn *rp = new  RegPtn; 
						map<kgstr_t,kgstr_t> attr_pre = kglibxml::get_attr(nodettp);
						xmlNodePtr nodetttp = nodettp->children;
						while(nodetttp){
							if(xmlStrEqual(nodetttp->name,(xmlChar *)"index")){
								map<kgstr_t,kgstr_t> attr_idx = kglibxml::get_attr(nodetttp);
								rp->addPtn(atoi(attr_idx["value"].c_str()));
							}
							nodetttp = nodetttp->next;
						}
						pNode->setClass(_attr->findclass(attr_pre["field"]));
						pNode->baseVal(rp);
					}
					nodettp = nodettp->next;
				}
			}
		}
		else if(xmlStrEqual(nodetp->name,(xmlChar *)"SimplePredicate")&&setF){//num
			map<kgstr_t,kgstr_t> attr_e = kglibxml::get_attr(nodetp);
			pNode->setClass(_attr->findclass(attr_e["field"]));
			pNode->baseVal(atof(attr_e["value"].c_str()));
		}
		else if(xmlStrEqual(nodetp->name,(xmlChar *)"SimpleSetPredicate")&&setF){//cat
			map<kgstr_t,kgstr_t> attr_e =kglibxml::get_attr(nodetp);
			fldItem *cfld = _attr->findclass(attr_e["field"]);
			vector<char> catList(cfld->itmSz(),'U');
			xmlNodePtr nodettp = nodetp->children;
			while(nodettp){
				if(xmlStrEqual(nodettp->name,(xmlChar *)"Array")){
					map<kgstr_t,kgstr_t> attr_pre = kglibxml::get_attr(nodettp);
					string clist = string((char*)nodettp->children->content);
					vector<string> clists = splitToken(clist,' ');
					for(size_t i=0;i<clists.size();i++){
						if(!clists[i].compare(0,1,"\"")&&!clists[i].compare(clists[i].size()-1,1,"\"")){
							clists[i].erase(clists[i].begin());
							clists[i].erase(clists[i].end()-1);
						}
						catList[_attr->item2id(attr_e["field"],clists[i])] ='M';
					}
				}
				nodettp = nodettp->next;
			}
			//vector<char> catList(cfld->itmSz(),'U');
			//catList[_attr->item2id(attr_e["field"],attr_e["value"])] ='M';
			pNode->setClass(cfld);
			pNode->baseVal(catList);
		}
		else if(xmlStrEqual(nodetp->name,(xmlChar *)"CompoundPredicate")&&setF){//cat
			xmlNodePtr nodettp = nodetp->children;
			fldItem *cfld;
			vector<char> catList;
			while(nodettp){
				if(xmlStrEqual(nodettp->name,(xmlChar *)"SimpleSetPredicate")){
					map<kgstr_t,kgstr_t> attr_pre = kglibxml::get_attr(nodettp);
					cfld = _attr->findclass(attr_pre["field"]);
					if(catList.empty()){ catList.resize(cfld->itmSz(),'U'); }
					catList[_attr->item2id(attr_pre["field"],attr_pre["value"])] ='M';
				}
				nodettp = nodettp->next;
			}
			pNode->setClass(cfld);
			pNode->baseVal(catList);
		}
		nodetp = nodetp->next;
	}
	now->tcnt_set(cnt_set);
	return now;
}

void Tree::setTopNode(xmlNodePtr node){
	_topNode = setNode(node,NULL,false);
}