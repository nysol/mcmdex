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
#include <kgmod.h>
#include <kgError.h>
#include <kgArgs.h>
#include <kgArgFld.h>
#include <kgCSV.h>
#include <kgCSVout.h>
#include <kgmodincludesort.h>

using namespace kglib;

namespace kgmod { ////////////////////////////////////////////// start namespace

class kgEdist : public kgModIncludeSort {

	// 引数
	kgArgFld _kField; // k=
	kgArgFld _sField; // s=
	kgArgFld _fField; // f=
	kgArgFld _wField; // w=
	kgCSVkey _iFile;  // i=
	kgCSVout _oFile;  // o=
	bool     normFlag_; // -norm
	vector<kgstr_t> _vField;//a=

	size_t itemNo_;
  map<string, size_t> itemMap_; // itemName -> int
	vector<string>      itemVec_; // int -> itemName

	size_t aidNo_;
	vector<string>      aidVec_; // aidの配列要素番号 -> aid

	void setArgs(void);
	size_t str2int(char* str);
	const char* int2str(size_t);

	double countCap(std::map<size_t,double> r, std::map<size_t,double> s);
	double distSet(std::map<size_t,double> r, std::map<size_t,double> s, double aCost, double bCost);
	double setSize(map<size_t,double>& a);
	double calEditDist(vector<std::map<size_t,double> > article1, vector<std::map<size_t,double> > article2);
	void show(vector<vector<std::map<size_t,double> > > articles);

public:
  // コンストラクタ
	kgEdist(void);
	~kgEdist(void){}

	int run(void);

	// コマンド固有の公開メソッド
	size_t iRecNo(void) const { return _iFile.recNo(); }
	size_t oRecNo(void) const { return _oFile.recNo(); }

};

}
