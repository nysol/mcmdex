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
#include <cfloat>
#include <csignal>
#include <kgroc.h>
#include <kgCSVout.h>
#include <kgError.h>
#include <kgMethod.h>
#include <kgArgs.h>
#include <kgConfig.h>

using namespace std;
using namespace kglib;
using namespace kgmod;

class strsort{
	public:
		bool operator()(char* t1,char* t2){
			return strcmp(t1,t2) < 0 ;			
		}
};

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録,パラメータ)
// -----------------------------------------------------------------------------
kgRoc::kgRoc(void){

	#ifdef ENG_FORMAT
		#include <help/en/kgrocHelp.h>
	#else
		#include <help/jp/kgrocHelp.h>
	#endif
	_name    = "kgroc";
	_version = "1.0";
}

// -----------------------------------------------------------------------------
// 入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgRoc::setArgs(void){
	// unknown parameter check
	_args.paramcheck("ac=,pc=,s=,o=,i=");

	// i= 入力ファイルオープン
	_iFile.open(_args.toString("i=",true), _env, _nfn_i);

	// o= 出力ファイルオープン
  _oFile.open(_args.toString("o=",false), _env, _nfn_o);

	// 入力ファイルから項目名行を読み込む
	_iFile.read_header();

	// ac= 項目引数のセット
	{
		kgstr_t str=_args.toString("ac=",true);
		_acField.set(str, &_iFile, _fldByNum);
	}

	// pc= 項目引数のセット
	{
		kgstr_t str=_args.toString("pc=",true);
		_pcField.set(str, &_iFile, _fldByNum);
	}

	// s= 項目引数のセット
	{
		kgstr_t str=_args.toString("s=",true);
		_sField.set(str, &_iFile, _fldByNum);
	}
}

namespace {
	void writeROC(kgCSVout& oFile, string& targetc, double score, double TP,double FN,double FP,double TN){

		oFile.writeStr( targetc.c_str()        ,false); // クラス名
		oFile.writeDbl( score                  ,false); // クラス名
		if(TP+FN==0) oFile.writeDbl( 0         ,false); // recall
		else         oFile.writeDbl( TP/(TP+FN),false); 
		if(FP+TN==0) oFile.writeDbl( 0         ,false); // FP rate
		else         oFile.writeDbl( FP/(FP+TN),false);
		if(TP+FP==0) oFile.writeDbl( 0         ,false); // precision
		else         oFile.writeDbl( TP/(TP+FP),false);
		oFile.writeDbl( (TP+FP)/(TP+FN+FP+TN)  ,false);  // posRate
		oFile.writeDbl( FP                     ,true ); // TP
	}

}


// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgRoc::run(void) try {

	setArgs();

	// 項目名出力
	if(! _nfn_o){
		_oFile.writeStr(_acField.name(0).c_str(), false);
		_oFile.writeStr(_sField.name(0).c_str() , false);
		_oFile.writeStr("recall",false);
		_oFile.writeStr("FPrate",false);
		_oFile.writeStr("precision",false);
		_oFile.writeStr("posRate",false);
		_oFile.writeStr("TP");
  	_oFile.writeEolNC();
	}

	// 実クラスの件数カウント
	_iFile.setKey(_pcField.getNum());
	map<string,int> clsCnt;
	int total=0;
	string old;
	while( EOF != _iFile.read() ){
		if( _iFile.keybreak() ) break;
		string prdc=_iFile.getNewVal(_pcField.num(0));
		string actc=_iFile.getNewVal(_acField.num(0));
		clsCnt[actc]++;
		total++;
	}

	// 入力ファイルにkey項目番号をセットする．
	vector<int> keyNum;
	vector<int> kn1 = _pcField.getNum();
	for(size_t i=0; i< kn1.size(); i++){
		keyNum.push_back(kn1.at(i));
	}
	vector<int> kn2 = _sField.getNum();
	for(size_t i=0; i< kn2.size(); i++){
		keyNum.push_back(kn2.at(i));
	}
	_iFile.setKey(keyNum);


	// 入力データ(predc+score降順でsortされていることが前提)
	// actc,prdc,score
	// a,a,5
	// a,a,4
	// b,a,3.5
	// a,a,3
	// b,a,2
	// b,a,1
	// b,b,9
	// b,b,8
	// a,b,7
	// b,b,6.5
	// a,b,6
	// a,b,5

	// 初期値
	// TP    FN clsCnt[targetc]
	// FP    TN clsCnt[nonTargetc]
	//  0 total total
	_iFile.seekTop();
	double score=0,TP=0,FN=0,FP=0,TN=0;
	string targetc;
	string prdc;
	string actc;
	while(_iFile.read()!=EOF){


		//keybreakしたら出力
		if( _iFile.keybreak() ){

			// 全てNegと予測した時の出力
			writeROC(_oFile,targetc,score,TP,FN,FP,TN);
		}

		prdc=_iFile.getNewVal(_pcField.num(0));

		// 次の対象クラス(ROCの対象となるクラス)
		if( prdc!=targetc && !(_iFile.status() & kgCSV::End) ){
			targetc=prdc;

			// TP,FP,FN,TNの初期化
			FN=clsCnt[targetc]; TN=total-FN;
			TP=FP=0;
			score=0;

			// 全てNegと予測した時の出力
			writeROC(_oFile,targetc,DBL_MAX,TP,FN,FP,TN);
		}

		score=atof( _iFile.getNewVal(_sField.num(0)));
		actc= _iFile.getNewVal(_acField.num(0));

		if(actc==targetc){ TP++; FN--;}
		else             { FP++; TN--;}
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
