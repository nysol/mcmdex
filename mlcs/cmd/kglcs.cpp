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

#include <kglcs.h>
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
kgLcs::kgLcs(void){

	_name    = "kgcut";
	_version = "###VERSION###";

	#include <help/en/kglcsHelp.h>
	_titleL = _title;
	_docL   = _doc;
	#ifdef JPN_FORMAT
		#include <help/jp/kglcsHelp.h>
	#endif

	_name    = "kglcs";
	_version = "1.0";
}

// -----------------------------------------------------------------------------
// 入出力ファイルオープン
// -----------------------------------------------------------------------------
void kgLcs::setArgs(void){
	// unknown parameter check
	_args.paramcheck("k=,f=,i=,o=,-q");

	// 入出力ファイルオープン
	_iFile.open(_args.toString("i=",false), _env,_nfn_i);
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);

	// k= 項目引数のセット
	vector<kgstr_t> vs = _args.toStringVector("k=",false);

	// f= 項目引数のセット
	vector<kgstr_t> vs_f = _args.toStringVector("f=",true);

	if(vs_f.size() > 2){
		ostringstream ss;
		ss << "f= takes one or two field name.";	
		throw kgError(ss.str());
	}
	if(vs.size()!=0 && vs_f.size()!=1){
		ostringstream ss;
		ss << "f= takes only one field name when using k=.";	
		throw kgError(ss.str());
	}

	_iFile.read_header();

	bool seqflg = _args.toBool("-q");
	if(_nfn_i) { seqflg = true; }
	if(!seqflg) { sortingRun(&_iFile,vs);}

	_kField.set(vs, &_iFile,_fldByNum);
	_fField.set(vs_f, &_iFile,_fldByNum);
	
}

size_t kgLcs::replace_next_larger(vector<size_t> & enumd, size_t value, size_t last_index){
//	cerr << "req0" << endl;

	if( enumd.empty() || value > enumd.back() ){
		enumd.push_back(value);
		return enumd.size() - 1;
	}
//	cerr << "req1" << endl;
	if(last_index==KG_SIZE_MAX){ last_index =enumd.size(); }
//	cerr << "req2" << endl;

  size_t first_index = 0;
  while (last_index!=KG_SIZE_MAX && first_index <= last_index){
//	cerr << "req2-1" << endl;

  	size_t i = ( (first_index + last_index) >> 1);
    size_t found = enumd[i];
//	cerr << "req2-2 " <<  value<< " " << found<< endl;
    if (value == found)    { return KG_SIZE_MAX ;}
    else if(value > found) { first_index = i + 1; }
    else                   { last_index = i - 1; }
//	cerr << "req2-3" << endl;

  }
//	cerr << "req3" << endl;
  enumd[first_index] = value;
//	cerr << "req4" << endl;
  return first_index;
}


vector<wstring> kgLcs::dolcs(const wchar_t *f1,size_t f1_sz ,const wchar_t *f2 ,size_t f2_sz) try {

	size_t f1_st =0;
	size_t f2_st =0; 
	size_t f1_ed =f1_sz-1;
	size_t f2_ed =f2_sz-1; 
	vector<size_t> rtn(f1_sz,KG_SIZE_MAX);
	
	while ( (f1_st<=f1_ed) && (f2_st<=f2_ed) && (f1[f1_st]==f2[f2_st]) ){
		rtn[f1_st]=f2_st;
		f1_st++; f2_st++;
	}
  f2_st = f1_st;

	while ((f1_st<=f1_ed) && (f2_st<=f2_ed) && (f1[f1_ed]==f2[f2_ed]) && f1_ed!=KG_SIZE_MAX && f2_ed!=KG_SIZE_MAX ){
    rtn[f1_ed] = f2_ed;
		f1_ed--; f2_ed--;
	}
	map < wchar_t , vector<size_t> > _b_matches;
	if(f2_ed!=KG_SIZE_MAX){
		for(size_t i=f2_st; i<=f2_ed; i++){
			if(_b_matches.find(f2[i])==_b_matches.end()){
				_b_matches[f2[i]] = vector<size_t> (1,i);
			}
			else{
				_b_matches[f2[i]].push_back(i);
			}
		}
	}
/*
	cerr << f1_st << " " <<f1_ed << " "<< f2_st << " " <<f2_ed << endl;
	for(map< wchar_t , vector<size_t> >::iterator it = _b_matches.begin(); it != _b_matches.end(); it++) {
			wprintf(L"%C ",it->first);
			for(vector<size_t>::iterator itt = it->second.begin(); itt != it->second.end(); itt++){
				wprintf(L"%d ",*itt);
			}
			wprintf(L"\n");
	}			
*/	
	vector <size_t> thresh; 
	KgVectorList links; 

	if(f1_ed!=KG_SIZE_MAX){
		for(size_t i=f1_st; i<=f1_ed;i++){
			size_t k=KG_SIZE_MAX;	
			if( _b_matches.find(f1[i]) != _b_matches.end() ){
				for( vector<size_t>::reverse_iterator j = _b_matches[f1[i]].rbegin(); j!= _b_matches[f1[i]].rend();j++){
					if ((k!=KG_SIZE_MAX) && (thresh[k] > *j) && (thresh[k-1] < *j) ){
							thresh[k] = *j;
					}
					else{
						k = replace_next_larger(thresh, *j, k);
					}
					if(k!=KG_SIZE_MAX){
						vector<size_t> pre ;
						pre.push_back(i);
						pre.push_back(*j);
						links.push(k,pre);
					}
				}
			}
		}
	}
/*	
	links.show();
	cerr << "-------------"  << endl;
	for(vector<size_t>::iterator itt = thresh.begin(); itt != thresh.end(); itt++){
		cerr << *itt << " ";
	}
	cerr <<endl;
	cerr << "-------------"  << endl;
	cerr << "end " << thresh.size() << endl;
*/
	if (!thresh.empty()){
		vector< vector<size_t> > linklist = links.getLinkList(thresh.size()-1);
		for(vector< vector<size_t> >::iterator itt = linklist.begin(); itt != linklist.end(); itt++){
 	 		rtn[itt->at(0)] = itt->at(1);
		}
	}

/*
	for (vector<size_t>::iterator it =rtn.begin(); it!=rtn.end();it++){
		cerr << *it <<" ";
	}
	cerr << endl;
*/
	vector<wstring> wrtn(3);
	size_t pos=0;
	for(size_t i=0;i<rtn.size();i++){
		if(rtn[i]!=KG_SIZE_MAX){
			wrtn[0].push_back(f1[i]);
			for(size_t j=pos; j<rtn[i];j++){
				wrtn[2].push_back(f2[j]);
			} 
			pos = rtn[i]+1;
		}
		else{
			wrtn[1].push_back(f1[i]);
		}
	}

	for(size_t j=pos;j<f2_sz;j++){
		wrtn[2].push_back(f2[j]);
	}
/*
	cerr << "----- result S-----" << endl;
	cerr << toMbs(wrtn[0]) << " | " << toMbs(wrtn[1]) << " | "<< toMbs(wrtn[2]) << endl;
	cerr << "----- result E-----" << endl;
*/
	return wrtn;
	
}catch(kgError& err){
	cerr << "catch1 " << endl;
	errorEnd(err);
}catch(...){
	cerr << "catch2 " << endl;
	errorEnd();
}


// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgLcs::run(void) try {

	setArgs();

	// 入力ファイルにkey項目番号をセットする．
	_iFile.setKey(_kField.getNum());

	// 項目名の出力
  if(!_nfn_o){
		vector<kgstr_t> outfld;
		for(vector<kgstr_t>::size_type i=0; i<_fField.size(); i++){
		  outfld.push_back(_fField.name(i));
		}
		if(_fField.size()==1){
		 outfld.push_back(_fField.name(0)+"_pair");		
		}
		outfld.push_back("comSeq");
		outfld.push_back("comSeqSize");
		outfld.push_back("diff1");
		outfld.push_back("diff1Size");
		outfld.push_back("diff2");
		outfld.push_back("diff2Size");
		_oFile.writeFldNameCHK(outfld);
	}
	
	if(_kField.size()==0){
		while(_iFile.read()!=EOF && !(_iFile.status() & kgCSV::End )){
		 //cerr << "========= data  S =============" <<  endl;
		 //cerr <<  _iFile.getNewVal(_fField.num(0)) << " | " << _iFile.getNewVal(_fField.num(1)) << endl;
		 //cerr << "========= data  E =============" <<  endl;
			wstring f0 = toWcs(_iFile.getNewVal(_fField.num(0)) );
			wstring f1 = toWcs(_iFile.getNewVal(_fField.num(1)) );
			vector<wstring>rls = dolcs(f0.c_str(),f0.size(),f1.c_str(),f1.size());

			_oFile.writeStr(_iFile.getNewVal(_fField.num(0)),false);
			_oFile.writeStr(_iFile.getNewVal(_fField.num(1)),false);
			_oFile.writeStr(toMbs(rls[0]).c_str(),false);
			_oFile.writeSizeT(rls[0].size(),false);
			_oFile.writeStr(toMbs(rls[1]).c_str(),false);
			_oFile.writeSizeT(rls[1].size(),false);
			_oFile.writeStr(toMbs(rls[2]).c_str(),false);
			_oFile.writeSizeT(rls[2].size(),true);
		}
	}else{ //k=あり時f=は1"
		vector<wstring> fld_stock;
		while(_iFile.read()!=EOF ){
			if( _iFile.keybreak() ){
				// 組合出力(nCk で k=<行数のとき出力)
			  if( fld_stock.size() >= 2  ){
			  	for (size_t i=0 ;i<fld_stock.size() ;i++){
				  for (size_t j=i+1 ;j<fld_stock.size() ;j++){
						vector<wstring>rls = dolcs(fld_stock[i].c_str(),fld_stock[i].size(),fld_stock[j].c_str(),fld_stock[j].size());
						_oFile.writeStr(toMbs(fld_stock[i]).c_str(),false);
						_oFile.writeStr(toMbs(fld_stock[j]).c_str(),false);
						_oFile.writeStr(toMbs(rls[0]).c_str(),false);
						_oFile.writeSizeT(rls[0].size(),false);
						_oFile.writeStr(toMbs(rls[1]).c_str(),false);
						_oFile.writeSizeT(rls[1].size(),false);
						_oFile.writeStr(toMbs(rls[2]).c_str(),false);
						_oFile.writeSizeT(rls[2].size(),true);
				  }}
			  } 
				if((_iFile.status() & kgCSV::End )) break;
				fld_stock.clear();
			}
			fld_stock.push_back( toWcs(_iFile.getNewVal(_fField.num(0))));
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

