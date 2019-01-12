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
//#include <kgmodincludesort.h>
#include <kgmod.h>
#include <kgError.h>
#include <kgArgs.h>
#include <kgArgFld.h>
#include <kgCSV.h>
#include <kgCSVout.h>
#include "datainfo.h"
#include "cluster4kmeans.h"
using namespace kglib;

namespace kgmod { ////////////////////////////////////////////// start namespace


class kgKmeans : public kgMod {

	// 引数
	kgCSVfld _iFile;  // i=
	kgCSVout _oFile;  // o=
	kgArgFld _fField; // f=

	int _cCnt; // n=
	int		_dType;	//d=
	int		_mcnt;	// m=
	unsigned long _seed; // S=
	kgstr_t		_addstr;	 //a=

	vector < Sample > _sample;
	
	Clusters _clusters;

	// 引数セット
	void setArgs(void);

	// read data
	//void getDataInfo(void);
	
	DataInfo _dinfo;

	
	
	//void samplingSmp(Sample *sample, int recCnt);
	void setSmp2Cluster(int k,Sample  *sample,int s,Clusters* cls);

	void sampling( int mul,int recCnt);
	int movCenter(Clusters *clusters);
	int nearestCluster();
	void setCluster();

  public:
  // コンストラクタ
	kgKmeans(void);
	~kgKmeans(void){}

	int run(void);

	// コマンド固有の公開メソッド
	size_t iRecNo(void) const { return _iFile.recNo();}
	size_t oRecNo(void) const { return _oFile.recNo();}

};

}