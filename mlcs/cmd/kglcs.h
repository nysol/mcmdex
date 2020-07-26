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

#include <string>
#include <vector>
#include <set>
#include <kgConfig.h>
#include <kgEnv.h>
#include <kgmodincludesort.h>
#include <kgError.h>
#include <kgArgs.h>
#include <kgArgFld.h>
#include <kgCSV.h>
#include <kgCSVout.h>

using namespace kglib;

namespace kgmod { ////////////////////////////////////////////// start namespace

class KgVectorList{
	vector< vector< vector<size_t> > > stock;
	vector< vector<size_t> > linklist;
	
	public:
		void push( size_t k , vector<size_t> dt){
			//cerr << "PUSH ====== RUN ======== START" << endl;
			//cerr << "K= " << k << endl;
			
			if (k >= stock.size()){ 
				stock.resize(k+1);
				linklist.resize(k+1);
			}
			stock[k].push_back(dt);
			if(k==0){ linklist[k].clear(); }
			else{     linklist[k] = linklist[k-1];}
			linklist[k].push_back(stock[k].size()-1);
			//show();
			//cerr << "PUSH ====== RUN ======== END" << endl;

		}

		vector< vector<size_t> > getLinkList(size_t k){

			vector< vector<size_t> > rtn;
			size_t pos=k;

			for ( vector<size_t>::reverse_iterator it=linklist[k].rbegin(); it!=linklist[k].rend();it++){
				//cerr << "ck " << pos << " " << *it << endl;
				rtn.push_back(stock[pos][*it]);
				pos--;
			}
			return rtn;
		}

		void show(){
			cerr << "--------" << endl;
			for(vector < vector< vector<size_t> > >::iterator it = stock.begin(); it != stock.end(); it++){
				for(vector< vector<size_t> >::iterator itt = it->begin(); itt != it->end(); itt++){
					for( vector<size_t>::iterator ittt = itt->begin(); ittt != itt->end(); ittt++){
						cerr << *ittt << " ";
					}
					cerr << endl;
				}
				cerr << "--------" << endl;
			}
			for(vector< vector<size_t> >::iterator it = linklist.begin(); it != linklist.end(); it++){
				for(vector<size_t>::iterator itt = it->begin(); itt != it->end(); itt++){
						cerr << *itt << " ";
				}
				cerr << endl;
			}
		}
};


class kgLcs : public kgModIncludeSort {

	// 引数
	kgArgFld _fField; // f=
	kgArgFld _kField; // k=
	kgCSVkey _iFile;  // i=
	kgCSVout _oFile;  // o=
	double _th;


	// 引数セット
	void setArgs(void);
//	wstring dolcs(wstring f1 ,wstring f2);
	vector<wstring> dolcs(const wchar_t *f1,size_t f1_sz ,const wchar_t *f2 ,size_t f2_sz);
	size_t replace_next_larger(vector<size_t> & enumd, size_t value, size_t last_index);
public:
  // コンストラクタ
	kgLcs(void);
	~kgLcs(void){}

	int run(void);

	// コマンド固有の公開メソッド
	size_t iRecNo(void) const { return _iFile.recNo();}
	size_t oRecNo(void) const { return _oFile.recNo();}

};

}
