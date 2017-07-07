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
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cfloat>
#include <csignal>

#include <kgedist.h>
#include <kgMessage.h>
#include <kgCSVout.h>
#include <kgError.h>
#include <kgMethod.h>
#include <kgArgs.h>
#include <kgVal.h>
#include <kgConfig.h>

using namespace std;
using namespace kglib;
using namespace kgmod;

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録)
// -----------------------------------------------------------------------------
kgEdist::kgEdist(void){

	#ifdef ENG_FORMAT
		#include <help/en/kgedistHelp.h>
	#else
		#include <help/jp/kgedistHelp.h>
	#endif

	_name    = "kgedist";
	_version = "1.0";
	itemNo_ = 1; // itemNo_=0は未登録情報用
	itemVec_.push_back("notRegistered"); // 最初の要素はdummyの文字列を入れておく
}

// -----------------------------------------------------------------------------
// 入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgEdist::setArgs(void){
	// unknown parameter check
	_args.paramcheck("k=,s=,f=,w=,i=,o=,O=,-norm");

	// 入力ファイルオープン
	_iFile.open(_args.toString("i=",false), _env,_nfn_i);

	// 出力ファイル名取得
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);
  _oFile.setPrecision(_precision);

	// 入力ファイルから項目名行を読み込む
	_iFile.read_header();

	// k= 項目引数のセット
	vector<kgstr_t> vs_k = _args.toStringVector("k=",true);
	_kField.set(vs_k, &_iFile, _fldByNum);

	// s= 項目引数のセット	
	vector<kgstr_t> vs_s = _args.toStringVector("s=",true);
	_sField.set(vs_s, &_iFile,_fldByNum);
	if(_sField.size()!=1){
		throw kgError("s= takes just one field name.");
	}
	
	// f= 項目引数のセット
	vector<kgstr_t> vs_f = _args.toStringVector("f=",true);
	_fField.set(vs_f, &_iFile,_fldByNum);
	if(_fField.size()>1){
		ostringstream ss;
		ss << "f= takes just one field name.";	
		throw kgError(ss.str());
	}

	// f= 項目引数のセット
	vector<kgstr_t> vs_w = _args.toStringVector("w=",false);
	if(vs_w.size()>0){
		_wField.set(vs_w, &_iFile,_fldByNum);
		if(_wField.size()>1){
			ostringstream ss;
			ss << "w= takes just one field name.";	
			throw kgError(ss.str());
		}
	}

	// a=　新項目名セット
	_vField = _args.toStringVector("a=",false);
	if(_vField.empty()){
		_vField.push_back("fld1");
		_vField.push_back("fld2");
	}else if(_vField.size()!=2){
		ostringstream ss;
		ss << "paraeter a= takes two field names or ommit them all.";
		throw kgError(ss.str());
	}

	// -norm 規格化フラグ
	normFlag_ = _args.toBool("-norm");
}

const char* kgEdist::int2str(size_t no){
	const char* ret=0;
	if(no<itemNo_){
		ret=itemVec_[no].c_str();
	}
	return ret;
}

size_t kgEdist::str2int(char* str){
	size_t ret=itemMap_[str];
	if(ret==0){ // strが未登録時
		ret=itemNo_++;
		itemMap_[str]=ret;
		itemVec_.push_back(str);
	}
	return ret;
}

void kgEdist::show(vector<vector<map<size_t,double> > > articles){
	for(vector<vector<map<size_t,double> > >::size_type i=0; i<articles.size(); i++){
		printf("########\n");
		for(vector<map<size_t,double> >::size_type j=0; j<articles.at(i).size(); j++){
			printf("--------\n");
			for(map<size_t,double>::iterator k=articles.at(i).at(j).begin(); k!=articles.at(i).at(j).end(); k++){
				cerr << k->first << "(" << int2str(k->first) << "," << k->second << ")" << endl;
			}
		}
	}
}

// 集合r,sのintersectionの数を返す。
double kgEdist::countCap(map<size_t,double> r, map<size_t,double> s){
	double interSectionCount=0;
	map<size_t,double>::iterator rIter=r.begin();
	map<size_t,double>::iterator sIter=s.begin();
	while(rIter!=r.end() && sIter!=s.end()){
		if(rIter->first < sIter->first){
			rIter++;
			continue;
		}
		if(rIter->first > sIter->first){
			sIter++;
			continue;
		}
		interSectionCount+=1;
		rIter++;
		sIter++;
	}
	return interSectionCount;
}

double kgEdist::setSize(map<size_t,double>& a){
	double size=0.0;
	map<size_t,double>::iterator iter;
	for(iter=a.begin(); iter!=a.end(); iter++){
		size+=iter->second;
	}
	return size;
}

double kgEdist::distSet(map<size_t,double> r, map<size_t,double> s, double aCost, double bCost){
	double cap = countCap(r,s);
	double cup = setSize(r)+setSize(s) - cap;
	return (aCost+bCost)*(1.0-cap/cup);
}

#define Address(i,j) ((i)*(b.size()+1)+(j))
double kgEdist::calEditDist(vector<map<size_t,double> > a, vector<map<size_t,double> > b){
	// 削除、挿入のコスト計算(長さによる規格化)
	double aCount=0.0;
	for(size_t i=0; i<a.size(); i++){
		aCount+=setSize(a[i]);
	}
	double bCount=0.0;
	for(size_t i=0; i<b.size(); i++){
		bCount+=setSize(b[i]);
	}

	// DP距離行列の初期化
	vector<double> dist( (a.size()+1)*(b.size()+1), 0);
	vector<double> aCost(a.size()+1, 0);
	vector<double> bCost(b.size()+1, 0);
//cerr << a.size() << endl;
//cerr << aCount << endl;
  // 長さ0の文字列と長さiの文字列の距離はi
	for(size_t i=1; i<=a.size(); i++){
		if(normFlag_){
			aCost[i]=static_cast<double>(setSize(a[i-1]))/aCount*0.5;
		}else{
			aCost[i]=static_cast<double>(setSize(a[i-1]));
		}
//printf("aCost[%d]=%g\n",i,aCost[i]);
//printf("address(%d,0)=%d size-%d\n",i,Address(i,0),(a.size()+1)*(b.size()+1));
		dist.at(Address(i,0))=dist.at(Address(i-1,0))+aCost[i];
	}
	for(size_t i=1; i<=b.size(); i++){
		if(normFlag_){
			bCost[i]=static_cast<double>(b[i-1].size())/bCount*0.5;
		}else{
			bCost[i]=static_cast<double>(b[i-1].size());
		}
//printf("bCost[%d]=%g\n",i,bCost[i]);
//printf("address(0,%d)=%d size=%d\n",i,Address(0,i),(a.size()+1)*(b.size()+1));
		dist.at(Address(0,i))=dist.at(Address(0,i-1))+bCost[i];
	}

/*
printf("------ aSize=%d, bSize=%d\n",a.size(),b.size());
for(size_t i=0; i<=a.size(); i++){
printf("i=%d\t",i);
for(size_t j=0; j<=b.size(); j++){
printf("%g\t",dist.at(Address(i,j)));
}
printf("\n");
}
printf("------\n");
*/
	// DP実行
	for(size_t i=1; i<=a.size(); i++){
		for(size_t j=1; j<=b.size(); j++){
			double cost=distSet(a[i-1],b[j-1],aCost[i],bCost[j]);
			double ins = dist.at(Address(i-1,j  )) + aCost[i];
			double del = dist.at(Address(i  ,j-1)) + bCost[j];
			double rep = dist.at(Address(i-1,j-1)) + cost;
			double min = ins<del ? ins : del;
			       min = min<rep ? min : rep;
//printf("i=%d,j=%d ins=%g del=%g rep=%g min=%g\n",i,j,ins,del,rep,min);
			dist.at(Address(i,j))=min;
		}
	}
	double sim=dist.at(Address(a.size(),b.size()));

	return sim;
}

// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgEdist::run(void) try {

	setArgs();

	// 入力ファイルにkey項目番号をセットする．
	_iFile.setKey(_kField.getNum());
	//vector<int>* key=_iFile.getKey();

	// 項目名の出力
  if(!_nfn_o){
		vector<kgstr_t> outfld;
		outfld.push_back("key1");
		outfld.push_back("key2");
		outfld.push_back("distance");
		_oFile.writeFldNameCHK(outfld);
	}

	map<size_t,double> itemset; // アイテム集合(第二要素は重み)
	vector<map<size_t,double> > seq; // アイテム集合シーケンス
	vector<vector<map<size_t,double> > > articles; // シーケンス集合

	// データを内部構造に格納
	size_t count=0;
	while(true){
		int eof=_iFile.read();
		if(eof!=1){
			aidVec_.push_back(_iFile.getOldVal(_kField.num(0))); // articleの番号-aid対応表を登録
			seq.push_back(itemset);
			articles.push_back(seq);
			break;
		}

		char* f_str=_iFile.getNewVal(_fField.num(0));
		double weight = 1.0;
		if(_wField.csv()!=0){
			char* w_str=_iFile.getNewVal(_wField.num(0));
			weight=atof(w_str);
		}
		char* new_s_str=_iFile.getNewVal(_sField.num(0));
		char* old_s_str=_iFile.getOldVal(_sField.num(0));
		char* new_k_str=_iFile.getNewVal(_kField.num(0));
		char* old_k_str=_iFile.getOldVal(_kField.num(0));
		if(*f_str=='\0' && *new_s_str=='\0'){
			continue;
		}

		bool seqBreak= (0!=strcmp(new_s_str,old_s_str) && count>0 );
		bool keyBreak= (0!=strcmp(new_k_str,old_k_str) && count>0 );
		if(keyBreak){
			aidVec_.push_back(old_k_str); // articleの番号-aid対応表を登録
		}
		// itemset(s=項目)の追加登録
		if( seqBreak || keyBreak ){
			seq.push_back(itemset);
			itemset.clear();
		}

		// シーケンス(k=項目)の追加登録
		if( keyBreak ){
			articles.push_back(seq);
			seq.clear();
		}

		size_t item=str2int(f_str);
		itemset[item]=weight;
		count+=1;
	}
	size_t totalCombi = aidVec_.size()*(aidVec_.size()-1)/2;
	count=0;
	kgMsg msg(kgMsg::MSG, _env);
	for(vector<vector<map<size_t,double> > >::size_type i=0; i<articles.size(); i++){
		for(vector<vector<map<size_t,double> > >::size_type j=i+1; j<articles.size(); j++){
			if(count%1000==0 && count>=1000){
				ostringstream ss;
				ss << count << "/" << totalCombi << " done.";
				msg.output(_name,ss.str());
			}
			double dist=calEditDist(articles.at(i),articles.at(j));
			_oFile.writeStr(aidVec_[i].c_str(),false);
			_oFile.writeStr(aidVec_[j].c_str(),false);
			_oFile.writeDbl(dist,true);
			count++;
		}
	}

	// 終了処理(メッセージ出力,thread pipe終了通知)
	th_cancel();
	_iFile.close();
	_oFile.close();

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

