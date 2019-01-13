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

#include <kgkmeans.h>
#include <kgDblN.h>
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

using namespace boost;

// -----------------------------------------------------------------------------
// コンストラクタ(モジュール名，バージョン登録)
// -----------------------------------------------------------------------------
kgKmeans::kgKmeans(void){

	_name    = "kgkmeans";
	_version = "###VERSION###";

	#include <help/en/kgkmeansHelp.h>
	_titleL = _title;
	_docL   = _doc;
	#ifdef JPN_FORMAT
		#include <help/jp/kgkmeansHelp.h>
	#endif

	_name    = "kmeans";
	_version = "1.0";
}

// -----------------------------------------------------------------------------
// 入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgKmeans::setArgs(void){
	// unknown parameter check
	_args.paramcheck("n=,f=,i=,o=,S=,d=,m=,a=,-null");

	// 入出力ファイルオープン
	_iFile.open(_args.toString("i=",false), _env,_nfn_i);
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);

	// f= 項目引数のセット
	vector<kgstr_t> vs_f = _args.toStringVector("f=",true);

	kgstr_t cnum = _args.toString("n=",true);
	_cCnt = atoi(cnum.c_str());

	kgstr_t dt = _args.toString("d=",false);
	if(dt.empty()){ _dType = 2; }
	else          { _dType = atoi(dt.c_str()); }
	if ( _dType < 0 || _dType > 2 ){
			throw kgError("parameter d= is 0-2");
	}

	kgstr_t mt = _args.toString("m=",false);
	if(mt.empty()){ _mcnt = 10; }
	else          { _mcnt = atoi(mt.c_str()); }

	// a= 追加項目名
	_addstr = _args.toString("a=",false);
	if(_addstr.empty()&& _nfn_o==false){
		throw kgError("parameter a= is mandatory");
	}

	_null=_args.toBool("-null");


	//乱数の種
	kgstr_t S_s = _args.toString("S=",false);
	if(S_s.empty())	{
		// 乱数の種生成（時間）
		posix_time::ptime now = posix_time::microsec_clock::local_time();
		posix_time::time_duration nowt = now.time_of_day();
		_seed = static_cast<unsigned long>(nowt.total_microseconds());
	}else	{ 
		_seed = static_cast<unsigned long>(aToSizeT(S_s.c_str()));
	}

	_iFile.read_header();
	_fField.set(vs_f, &_iFile,_fldByNum);
	
}

// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgKmeans::run(void) try {

	setArgs();

	// データ初期読込 & クラスタサイズ調整
	size_t rec = _dinfo.getDatInfo(_iFile,_fField);
	if( rec <_cCnt ) _cCnt = rec; 

	// 初期設定
  switch(_dType){
		// inputdata ,項目位置情報 , クラスタ数  , 乱数seed , サンプリング数 ,  サンプリングセット数

  	case 0: /*---------------------------- ランダム*/
			
	    _cCnt = _clusters.initRA(&_iFile , &_fField , &_dinfo , _cCnt,  _seed , 1, 100);

	    break;

  	case 1: /*---------------------------- Kaufman Approach*/

	    _cCnt = _clusters.initKA(&_iFile , &_fField , &_dinfo, _cCnt,  _seed , 1, 100);

	    break;

	  case 2: /*---------------------------- Bradley & Fayyad Approach*/
	  
	    _cCnt = _clusters.initBFbyRA(&_iFile , &_fField , &_dinfo, _cCnt,  _seed , _mcnt, 100);

	    break;
	  
	}

  // MAIN CONVERGENCE
  _clusters.convergent();


	// 結果出力
  _oFile.writeFldName(_iFile,_addstr);
	_iFile.seekTop();
	while(_iFile.read()!=EOF){

    int k=_clusters.nearestCid(_null);
    if(k!=-1){
			_oFile.writeFld(_iFile.getFld(),_iFile.fldSize(),k+1,false);
    }
    else{
			_oFile.writeFld(_iFile.fldSize(),_iFile.getFld(),false);
			_oFile.writeEol();
		}
	}

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

