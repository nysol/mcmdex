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

#include "flditem/patitem.h"
#include "regptnque.h"

using namespace kglib ;

//----------------
// 初期読み込みアイテム読み込み
// NULL値は登録しない
//----------------
void PatternItem::init_read(char** val)
{
	if(*val[fldnum()]!='\0'){	_itemMap.push(val[fldnum()],_param.delim());}
}

void PatternItem::itemSet(vector<kgstr_t>& ilist)
{
	for(size_t i=0 ; i<ilist.size() ; i++){
		_itemMap.push(ilist[i]);
	}
}

void PatternItem::idxSet(map<kgstr_t,int>& ilist){
	_best_aindex.resize(ilist.size());
	for ( map<kgstr_t,int>::iterator it = ilist.begin();it != ilist.end();it++){
		int id = _itemMap.item2id(const_cast<char *>((*it).first.c_str()));
		_best_aindex[id] = (*it).second;
	}	
}

//void PatternItem::init_readT(char** val)
//{
//	if(*val[fldnumT()]=='\0'){ _itemMap.push(val[fldnumT()],_param.delim());}
//}
//----------------------------------------------------------------
// データ読み込み
//----------------------------------------------------------------
//void PatternItem::read(char** val,bool dset)
void PatternItem::read(char** val)
{
	if(*val[fldnum()]=='\0'){
		_trainingDat.push_back(ValtypeVec(true));
//		if(dset){	_testDat.push_back(ValtypeVec(true));}
  }else{
  	vector<int> ids = _itemMap.item2id(val[fldnum()],_param.delim());
		_trainingDat.push_back(ValtypeVec(ids));
//		if(dset){	_testDat.push_back(ValtypeVec(ids));}
	}
}
//void PatternItem::readT(char** val){
//	if(*val[fldnum()]=='\0'){
// 		_testDat.push_back(ValtypeVec(true));
// 	}
//	else{
//  	vector<int> ids = _itemMap.item2id(val[fldnumT()],_param.delim());
//		_testDat.push_back(ValtypeVec(ids));
//	}
//}
void PatternItem::make_idx(AIndex * ai){
	_idxDat.resize(_trainingDat.size());
	for(size_t i=0 ; i< _trainingDat.size();i++){
		_idxDat[i] = _trainingDat.at(i).toIndex(ai);
	}
}
//------------------------------------------------------
// 正規パターンを作成し、
// 全データを使用した場合での成績の良い正規パターンに絞り込む
// 絞り込んだ正規パターンに対してmatch,unmatchを求める
// __reg_tblに絞り込んだ正規パターン
//	__attに正規パターン、データごとmatch,unmatchを格納
//------------------------------------------------------
void PatternItem::make_regexp(const ClsItem* cls)
{
//printf("xxmake_regex _idxSz=%lu\n",idxSz());
//printf("xxmake_regex _itmSz=%lu\n",itmSz());

	RegPtnQueue regQue(_regCnt,cls);		
	//一文字ずつの正規パターンをセット
	for (size_t i=0 ; i<idxSz() ; i++){
		vector<int> n_ptn(1,i+1);
		// _idxDat: sample別index sequence
		regQue.pushReg( new RegPtn(n_ptn,subseq()) , _idxDat);
//fprintf(stderr,"[%lu] que.size=%lu\n",i,regQue.size());
//for(size_t j=0; j<n_ptn.size(); j++){
//fprintf(stderr,"[%lu,%lu] ",i,j);
//fprintf(stderr,"%d ",n_ptn[j]);
//}
//fprintf(stderr,"\n");
		// 先頭一致
		for (size_t j=1 ; j<begin()+1 ; j++){
			regQue.pushReg( new RegPtn(n_ptn,subseq(),j) , _idxDat);
		}
    // 末尾一致*/
		for (size_t j=1 ; j<end()+1 ; j++){
			regQue.pushReg( new RegPtn(n_ptn,subseq(),0,j) , _idxDat);
		}
	}

//fprintf(stderr,"idxSz=%lu,que.size=%lu\n",idxSz(),regQue.size());
//regQue.top()->show();

	// 複数の正規パターンをセット
	while(!regQue.empty()){
		RegPtn *val = regQue.pop();
//fprintf(stderr,"xx1\n");
//val->show();
		if(regQue.isUnderMax(val) && val->size()<indexLen()){ // 候補数上限を超えていない
			for (size_t i=0 ; i<idxSz() ; i++){
				RegPtn * newptn =  new RegPtn(val);
				newptn->addPtn(i+1);
				regQue.pushReg( newptn , _idxDat );
			}
		}
		delete val;
	}
//fprintf(stderr,"idxSz=%lu,que.size=%lu\n",idxSz(),regQue.size());

	// match,unmatch情報セット
	size_t bestSz=regQue.size();
//printf("xxmake_regex _bestSz=%lu\n",bestSz);
	_attDat.clear();
	regclean();
	_regTbl.clear();
	if(bestSz>0){  _attDat.resize(bestSz); }
//printf("xxmake_regex _attDat.size=%lu\n",_attDat.size());
	for (size_t i=0 ;i<bestSz ; i++){
		RegPtn *val = regQue.popBest();
		_regTbl.push_back(val);
		for (size_t j=0 ;j<_idxDat.size() ; j++){
			_attDat[i].push_back(val->regCmp(_idxDat[j]));
		}
	}
}

//bool PatternItem::isMatch(baseInfo base,int pos,bool type)
bool PatternItem::isMatch(baseInfo base,int pos)
{
	ValtypeVec indx;	
//	if(type){ indx = _trainingDat[pos].toIndex(_aindex); }
//	else		{ indx = _testDat[pos].toIndex(_aindex); }
	indx = _trainingDat[pos].toIndex(_aindex);
	return base.reg()->regCmp(indx); 
}

//bool PatternItem::chkMatch(baseInfo base,int pos,bool type)
bool PatternItem::chkMatch(baseInfo base,int pos)
{
	ValtypeVec indx;	
//	if(type){ indx = _trainingDat[pos].toIndex(_aindex); }
//	else		{ indx = _testDat[pos].toIndex(_aindex); }
	if(_aindex==NULL){ //predictもーど
		indx = _trainingDat[pos].toIndex(_best_aindex);
	}
	else{
		indx = _trainingDat[pos].toIndex(_aindex);
	}	
	return base.reg()->regCmp(indx); 
}
void PatternItem::printIdx(ostream* ofp)
{
	for(int i=0 ; i< idxSz() ; i++ ){
		*ofp << "Index[" << i+1 << "]={";
		bool firstw=true;
		for(int j=0 ;j<_best_aindex.size() ; j++){
			if( i+1 == _best_aindex.at(j)){
				if(firstw){ firstw=false; }
				else			{ *ofp << ",";  }
				*ofp << _itemMap.id2item(j);
			}
		}
		*ofp << "}" << endl;
	}
}

void PatternItem::condPrint(baseInfo base, ostream* ofp)
{
	//printf( "if($%s has %s)\n" , name().c_str() , base.reg()->to_s().c_str() );
	*ofp << "if($" << name() << " has " << base.reg()->to_s()  << ")" << endl;
}

void PatternItem::writePMML(FILE *fp,baseInfo  base ,int level,char trl)
{
	RegPtn *reg = base.reg();
	string val;
	string op;

	if( reg->type() ){
		if( trl == 'm' ){ val= "subsequence"; op = "contain"; }
		else						{ val= "subsequence"; op = "notcontain"; }
	}
	else{
		if( trl == 'm' ){ val= "substring"; op = "contain"; }
		else						{ val= "substring"; op = "notcontain";}
	}
	tabPrint(fp,level+2);
	fprintf(fp,"<Extension extender=\"KGMOD\" name=\"patternPredicate\" value=\"%s\">\n" , 
						val.c_str() );
	tabPrint(fp,level+3);
	fprintf(fp, "<SimplePredicate field=\"%s\" operator=\"%s\">\n" , 
						name().c_str() , op.c_str());

	for(int i=0 ;i<reg->size() ; i++){
		tabPrint(fp,level+4);
		fprintf(fp,"<index seqNo=\"%d\" value=\"%d\"/>\n" , i+1,reg->ptnAt(i) );
	}

	tabPrint(fp,level+3);
	fprintf(fp, "</SimplePredicate>\n");

	tabPrint(fp,level+2);
	fprintf(fp, "</Extension>\n");

}

#define showIndent(indent) for(int ind=0; ind<indent; ind++) printf(" ");
void PatternItem::show(int indent){
	showIndent(indent);
	printf("%s: (alpha: ",this->name().c_str() );
	for(int i=0 ; i<this->itmSz() ; i++ ){ 
		printf("%s,", this->id2item(i).c_str());
	}
	printf(")(index:");
	for(int i=0 ; i<this->itmSz() ; i++ ){ printf("%lu,", this->id2bestAidx(i) );}
	printf(")\n");

	indent+=2;
	showIndent(indent);
	printf("data by alphabet:\n");
	for(size_t i=0 ; i<_trainingDat.size() ; i++ ){
		showIndent(indent+2);
		printf("%s\n", _trainingDat[i].toStr().c_str());
	}

	showIndent(indent);
	printf("data by index:\n");
	for(size_t i=0 ; i<_idxDat.size() ; i++ ){
		showIndent(indent+2);
		printf("%s\n", _idxDat[i].toStr().c_str());
	}

	showIndent(indent);
	printf("data by regex:\n");
	for(size_t i=0 ; i<_attDat.size() ; i++ ){
		showIndent(indent+2);
		printf("%s: ", _regTbl.at(i)->toStr().c_str());
		for(size_t j=0 ; j<_attDat.at(i).size() ; j++ ){
			printf("%d",(int)_attDat.at(i).at(j));
		}
		printf("\n");
	}

}



