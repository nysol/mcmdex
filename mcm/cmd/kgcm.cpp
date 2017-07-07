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
#include <vector>
#include <csignal>
#include <kgcm.h>
#include <kgCSVout.h>
#include <kgError.h>
#include <kgMethod.h>
#include <kgArgs.h>
#include <kgConfig.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace kglib;
using namespace kgmod;
using namespace boost;

class strsort{
	public:
		bool operator()(char* t1,char* t2) const {
			return strcmp(t1,t2) < 0 ;			
		}
};

class compStr{
	public:
		bool operator()(const string& s1,const string& s2) const {
			return s1 < s2;
		}
};

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録,パラメータ)
// -----------------------------------------------------------------------------
kgCm::kgCm(void){
	#ifdef ENG_FORMAT
		#include <help/en/kgcmHelp.h>
	#else
		#include <help/jp/kgcmHelp.h>
	#endif
	_name    = "kgcm";
	_version = "1.0";
}

// -----------------------------------------------------------------------------
// 入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgCm::setArgs(void){
	// unknown parameter check
	_args.paramcheck("ac=,pc=,O=,i=");

	// i= 入力ファイルオープン
	_iFile.open(_args.toString("i=",false), _env, _nfn_i);

	// 入力ファイルから項目名行を読み込む
	_iFile.read_header();

	// o= 出力ファイルオープン
	kgstr_t s_o = _args.toString("O=",true);
	filesystem::path dirname=filesystem::path(s_o);
	create_directories(dirname);

	kgstr_t sum_name 		= s_o + "/summary.csv";
	kgstr_t class_name	= s_o + "/class.csv";
	kgstr_t matrix_name	= s_o + "/confMatrix.txt";
	_oFile_s.open(sum_name   , _env, _nfn_o);
	_oFile_c.open(class_name , _env, _nfn_o);
	_oFile_m.open(matrix_name, _env, _nfn_o);

	// c= 項目引数のセット
	kgstr_t s_c = _args.toString("ac=",true);
	_cField.set(s_c, &_iFile, _fldByNum);

	// pred= 項目引数のセット
	kgstr_t s_f = _args.toString("pc=",true);
	_pField.set(s_f, &_iFile, _fldByNum);

}

#define MaxClassSize 50
namespace {
	// クラス名-配列番号対応表からクラス名strに対応する配列番号を返す.
	// クラス名が登録されていなければ登録する．
	int getClassNum( map<string,int,compStr>& itemtable, string& str){
		int c_no;
		map<string,int,compStr>::iterator i;
		i = itemtable.find(str);
		if( i == itemtable.end() ){
			c_no=itemtable.size();

			// クラス数上限を超えればエラー
			if(c_no>=MaxClassSize){
				ostringstream ss;
				ss << "exceeded maximum class size (" << MaxClassSize << ")";
				kgError err(ss.str());
				throw err;
			}

			itemtable[str] = c_no;
		}else{
			c_no=i->second;			
		}
		return c_no;
	}
}

// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgCm::run(void) try {
	setArgs();

	// クラス文字列-配列番号対応表.
	map<string,int,compStr> itemtable;
	// 実クラス×予測クラスの件数を格納する二次元配列(クラス数上限制限あり)
	vector< vector<int> > vtable(MaxClassSize,vector<int>(MaxClassSize));
	// 予測不能件数を格納する配列
	vector<int> unpred(MaxClassSize);

	int totalcnt=0; // 行数カウント

	// データ読み込み
	while( EOF != _iFile.read() ){

		// 実クラスがnullであれば無視 (追加byHam)
		string acstr = _iFile.getVal(_cField.num(0));
		if(acstr.empty()){
			continue;
		}
		// 実クラスの配列番号を取得
		int ac_no=getClassNum(itemtable,acstr);

		// 予測クラスがnullであれば予測不能として扱う (追加byHam)
		string pcstr = _iFile.getVal(_pField.num(0));
		if(pcstr.empty()){
			unpred[ac_no]++;
			continue;
		}
		// 予測クラスの配列番号を取得
		int pc_no=getClassNum(itemtable,pcstr);

		// カウントアップ
		vtable.at(ac_no).at(pc_no)++;
		totalcnt++;
	}

	//アイテムテーブルをvectorへ変換
	int classSize=itemtable.size();  // クラスの種類数
	vector<string> itemvtable(classSize);	
	map<string,int,compStr>::iterator i;
	for(i=itemtable.begin();i!=itemtable.end();i++){
		itemvtable[i->second]=i->first;
	}

	// ---------------------------------------------------
	// 出力1(confMatrix.txt)
	//     a  b
	//  a 10  2
	//  b  3 18
	//
	//  a:クラス名1
	//  b:クラス名2
	// ---------------------------------------------------
	if(classSize>0){
		for(int i = 0;i<classSize;i++){
			_oFile_m.writeStr("\t");			
			_oFile_m.writeInt2alpha(i+1);
		}
		_oFile_m.writeStr("\tu");			
		_oFile_m.writeEolNC();	

		for(int i = 0;i<classSize;i++){
			_oFile_m.writeInt2alpha(i+1);
			for(int j = 0;j<classSize;j++){
				_oFile_m.writeStr("\t");			
				_oFile_m.writeInt(vtable[i][j]);
			}		
			_oFile_m.writeStr("\t");			
			_oFile_m.writeInt(unpred[i]);
			_oFile_m.writeEolNC();	
		}
		_oFile_m.writeEolNC();	

		// a,bに対応するクラス名の表示
		for(int i = 0;i<classSize;i++){
			_oFile_m.writeInt2alpha(i+1);
			_oFile_m.writeStr(":");
			_oFile_m.writeStr(itemvtable[i].c_str());
			_oFile_m.writeEolNC();
		}
		_oFile_m.writeStr("u:unpredictable");
	}

	// ---------------------------------------------------
	// 出力2(summary.csv)
	// evaluation,value
	// accuracy,0.9185130478
	// error rate,0.08148695224
	// total records,8124
	// unpredictable records,0
	// ---------------------------------------------------
	int ac_cnt=0; // 正答件数
	int er_cnt=0; // 誤答件数
	int up_cnt=0; // 予測不能件数
	for(int i = 0;i<classSize;i++){
		for(int j = 0;j<classSize;j++){
			if(i==j)	{ ac_cnt = ac_cnt + vtable[i][j];}
			else			{ er_cnt = er_cnt + vtable[i][j];} 
		}
		up_cnt += unpred[i];
	}
	vector<kgstr_t> vs1;
	vs1.push_back("evaluation");
	vs1.push_back("value");
	_oFile_s.writeFldName(vs1);
	_oFile_s.writeStr("accuracy",false);
	if(totalcnt!=0)
	_oFile_s.writeDbl((double)ac_cnt/(double)totalcnt);
	_oFile_s.writeEolNC();
	_oFile_s.writeStr("error rate",false);			
	if(totalcnt!=0)
	_oFile_s.writeDbl((double)er_cnt/(double)totalcnt);
	_oFile_s.writeEolNC();	
	_oFile_s.writeStr("total records",false);
	_oFile_s.writeDbl(totalcnt);
	_oFile_s.writeEolNC();	
	_oFile_s.writeStr("unpredictable records",false);			
	_oFile_s.writeDbl(up_cnt);
	_oFile_s.writeEolNC();
	
	// ---------------------------------------------------
	// 出力3(class.csv)
	// actClass,TP,FN,FP,TN,upCnt,upRate,recall,precision,f1
	// neg,3747,169,493,3715,0.9568437181,0.8837264151,0.9188327612
	// pos,3715,493,169,3747,0.8828422053,0.9564881565,0.9181908057
	// ---------------------------------------------------
	vector<kgstr_t> vs2;
	vs2.push_back(_cField.name(0));
	vs2.push_back("TP");
	vs2.push_back("FN");
	vs2.push_back("FP");
	vs2.push_back("TN");
	vs2.push_back("upCnt");
	vs2.push_back("upRate");
	vs2.push_back("recall");
	vs2.push_back("precision");
	vs2.push_back("f1");
	_oFile_c.writeFldName(vs2);
	
	for(int i=0;i<classSize;i++){
		int tp=0;
		int fp=0;
		int fn=0;
		int tn=0;
		for(int j=0;j<classSize;j++){
			for(int k=0;k<classSize;k++){
				if(i==j&&i==k){ tp += vtable[j][k]; }
				else if(i==j) { fn += vtable[j][k]; }
				else if(i==k) { fp += vtable[j][k]; }
				else          { tn += vtable[j][k]; }
			}
		}
		_oFile_c.writeStr(itemvtable[i].c_str(),false);
		_oFile_c.writeInt(tp,false);
		_oFile_c.writeInt(fn,false);
		_oFile_c.writeInt(fp,false);
		_oFile_c.writeInt(tn,false);
		_oFile_c.writeInt(unpred[i],false);

		_oFile_c.writeDbl(
			 static_cast<double>(unpred[i])/
			(static_cast<double>(unpred[i])+tp+fn+fp+tn),false);

		double recall=0;
		double precision=0;
		if( (tp+fn) != 0 ){
			recall = (double)tp/(double)(tp+fn);
			_oFile_c.writeDbl(recall,false);
		}else{
			_oFile_c.writeStr("",false);
		}

		if( (tp+fp) !=0){
			precision = (double)tp/(double)(tp+fp);
			_oFile_c.writeDbl(precision,false);
		}else{
			_oFile_c.writeStr("",false);
		}

		if( (tp+fn) != 0 && (tp+fp) !=0 && (recall+precision)!=0 ){
			_oFile_c.writeDbl(recall*precision*2/(recall+precision),true);
		}else{
			_oFile_c.writeStr("",true);
		}
	}

	_iFile.close();

	_oFile_m.close();
	_oFile_c.close();
	_oFile_s.close();

	// 終了処理(メッセージ出力,thread pipe終了通知)
	successEnd();
	return 0;

// 例外catcher
}catch(kgError& err){
	errorEnd(err);
}catch(...){
	errorEnd();
}
