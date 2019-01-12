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
#include "cluster4kmeans.h"
using namespace std;
using namespace kglib;
using namespace kgmod;

using namespace boost;

/*----------------------------------------------------------------------------*/
/* 数値間の距離を計算                                                         */
/*----------------------------------------------------------------------------*/
static inline double disNum(double x, double y){

  double sub;
  /*ユークリッド距離絶対値*/
  sub= x - y;

  if(sub<0) return sub*(-1);
  else      return sub;

}

static inline double norm(double num, double min, double rng){
	return (num-min)/rng;
}

//----------------------------------------------------------------------------
// 各項目の距離の平均を計算(NULLは計算に入れない)                             
//----------------------------------------------------------------------------
static inline double disAvg(vector<double> &disFld, int cnt)
{
  double sum=0;
  int j=0;
  int i;

  /*総和*/
  for(i=0;i<cnt;i++){
    sum+=disFld[i];
  }
  return sum/(double)cnt;
}
//----------------------------------------------------------------------------
// sListのk番目までに、sがあれば１を返す 
//----------------------------------------------------------------------------
static bool isInCluster(int s,vector<int> &sList, int k)
{

  for(int i=0; i<k; i++){
    if( s==sList[i] ) return(true);
  }
  return(false);
}

//----------------------------------------------------------------------------
// 入力データのsumary情報取得
// ( cnt ,sum ,min, max)
//----------------------------------------------------------------------------
void Clusters::getDataInfo(){

	_dinfo.init(_fCnt);

	while(_icsv->read()!=EOF){
		for(size_t i=0 ; i<_fCnt ; i++){ 
			char *str = _icsv->getVal(_ffld->num(i));
			if(*str!='\0'){
				_dinfo.vSet(i,atof(str));
			}
		}
		++_dinfo;
	}

	if(!_dinfo.valid()){
		throw kgError("value not found on some fields");
	}

	_dinfo.calc();

  if( _dinfo.Rec()<_cCnt ) _cCnt = _dinfo.Rec();
	
}

//----------------------------------------------------------------------------
// ファイルからrecCnt件サンプリングし、sample[]にセットする                   
//  mul, 何セット用意するか
//  recCnt   サンプリング行数
// NULLの場合avgセット
//----------------------------------------------------------------------------
void Clusters::csv2sample(
	vector<Sample> & samp,int recCnt, 
	boost::variate_generator< boost::mt19937,boost::uniform_int<>  > * rmod 
){ 
  /*サンプリング件数の調整*/
  if(recCnt < 100      ) recCnt=100;
  if(recCnt > 5000     ) recCnt=5000;
  if(recCnt > _dinfo.Rec()) recCnt=_dinfo.Rec();

  /*領域確保*/
  size_t mul = samp.size();
  vector<int> select(mul);    // 必要サンプリング数
  vector<int> remaining(mul); // データ行数残

  for(int i=0; i<mul; i++){
  	samp[i].setSize(recCnt,_fCnt);
    select[i]=recCnt;
    remaining[i]=_dinfo.Rec();
  }

	_icsv->seekTop();

	while(_icsv->read()!=EOF){
	
    for(int i=0; i<mul; i++){

      if(  (*rmod)()%remaining[i] < select[i] ){
        /*数値項目*/
				for(size_t j=0 ; j<_fCnt; j++){ 
					char *str = _icsv->getVal(_ffld->num(j));
          if(*str!='\0'){ 
	          samp[i].vSet(recCnt-select[i],j,atof(str));
        	}
        	else{
	          samp[i].vSet(recCnt-select[i],j,_dinfo.Avg(i));
        	}
        	
        }
        select[i]--;
      }
      remaining[i]--;
    }
  }
}

//----------------------------------------------------------------------------
// 与えられた行が最も近いクラスタ番号を返す                                    
//  クラスタが全てNULLの場合,もしくはデータ項目が全てNULLの場合,もしくは      
//  クラスタの項目とデータの項目がうまくNULLでかみあった場合は-1を返すことに  
//  なる。すなわち、近いクラスタは分からないということになる                 
//----------------------------------------------------------------------------
int Clusters::nearestCid(){

  vector<double> disFld(_fCnt);
	double a;
	double b;
  double distance=9999;
	int k=-1;

  for(int c=0; c<_cCnt; c++){

    int j=0;
    for(size_t i=0; i<_fCnt; i++){

    	if( _dinfo.Rng(i) == 0 ) continue;

      a=norm(_cluster[c].gGet(i), _dinfo.Min(i), _dinfo.Rng(i));

			char *str = _icsv->getVal(_ffld->num(i));
			
      if(*str=='\0') { b = _dinfo.Avg(i);}
      else        { b=atof(str);}

      b=norm(b,_dinfo.Min(i), _dinfo.Rng(i));
      disFld[j++] = disNum(a,b);
 
    }
 
    if(j==0){ continue;}

    //結合距離の計算
    double distanceTmp=disAvg(disFld,j);

    //距離が小さければ更新
    if(distanceTmp<distance){
      distance=distanceTmp;
      k=c;
    }
  }

  return(k);
}
//----------------------------------------------------------------------------
// 全データを各クラスタに振り分け、重心計算のための値を更新していく            
//----------------------------------------------------------------------------
void Clusters::convergent(){

	while(1){

		wrkReset();
		_icsv->seekTop();

		while(_icsv->read()!=EOF){
	    int k=nearestCid();
  	  if(k!=-1){
	  	  _cluster[k].incCnt(); //_クラスタに属するレコード数
      
      	for(int i=0; i<_fCnt; i++){
					char *s = _icsv->getVal(_ffld->num(i));
					if(*s=='\0'){ _cluster[k].acum(i , _dinfo.Avg(i)); }
	      	else{ _cluster[k].acum(i , atof(s)); }
      	}
    	}
  	}
  	if(!movCenter()) break;
  }
  
}

//----------------------------------------------------------------------------
// cluster と rno 番のサンプルとの距離                                 
//----------------------------------------------------------------------------

double Cluster::calDist(Sample& sample,size_t rno ,DataInfo & dinfo)
{

	vector<double> disFld(_cenNum.size());

  int j=0;

  for(int i=0; i<_cenNum.size(); i++){
  	if( _cenNum[i].null() || dinfo.Rng(i) ==0 ) continue;
    double a=norm( _cenNum[i].r()     , dinfo.Min(i), dinfo.Rng(i));
    double b=norm( sample.vGet(rno,i) , dinfo.Min(i), dinfo.Rng(i));
    disFld[j++]=disNum(a,b);
  }
  if(j==0){ return DBL_MAX;}

  /*結合距離の計算*/
  double sum=disAvg(disFld,j);

  return( sum );
}


//----------------------------------------------------------------------------
// 重心に最も近いレコード番号を取得 
//----------------------------------------------------------------------------
int Clusters::getCenInstance(Sample& sample,DataInfo &dinfo){

  Cluster clusterT;
  clusterT.setSize(sample.fCnt());
  
  for(int i=0; i< sample.Cnt(); i++){
    for(int j=0; j< sample.fCnt(); j++){
	    clusterT.acum( j ,sample.vGet(i,j));
    }
  }

  for(int j=0; j< sample.fCnt(); j++){
    if( clusterT.Cnt(j) !=0 ){
    	clusterT.gSet( j ,clusterT.Avg(j));
    }
  }

  int k=0;
  double disTmp,distance=DBL_MAX;

  for(int i=0 ; i<sample.Cnt() ; i++){
    disTmp=clusterT.calDist(sample,i,dinfo);
    if( distance > disTmp ){
      distance=disTmp;
      k=i;
    }
  }
  return(k);
}

//----------------------------------------------------------------------------
// m番目のサンプル と n番目のサンプルとの距離                                
//----------------------------------------------------------------------------
double Clusters::get_dji( Sample& sample,  DataInfo& dinfo, int m, int n )
{
	vector<double> disFld(sample.fCnt());
	
  int j=0;
  /*数値項目*/
  for(int i=0; i<sample.fCnt(); i++){
    double a=norm(sample.vGet(m,i), dinfo.Min(i), dinfo.Rng(i));
    double b=norm(sample.vGet(n,i), dinfo.Min(i), dinfo.Rng(i));
    disFld[j++]=disNum(a,b);
  }

  /*結合距離の計算*/
  return disAvg(disFld,j);
}

//----------------------------------------------------------------------------
// 現在登録されている各クラスタとサンプルjとの最短距離を返す 
//----------------------------------------------------------------------------
double Clusters::get_Dj(Sample& sample,DataInfo& dinfo,int k,int j)
{
  double dis;
  double disMin;
  int s_min=0;
  int s;

  disMin=DBL_MAX;

  for(s=0; s<k; s++){
    dis = _cluster[s].calDist(sample,j,dinfo);
    if(disMin>dis){
      disMin=dis;
    }
  }
  return disMin;
}


//----------------------------------------------------------------------------
//与えられた行が最も近いクラスタ番号を返す(sample用)                          
//  クラスタが全てNULLの場合,もしくはデータ項目が全てNULLの場合,もしくは     
//  クラスタの項目とデータの項目がうまくNULLでかみあった場合は-1を返すことに  
//  なる。すなわち、近いクラスタは分からないということになる                  
//----------------------------------------------------------------------------
int Clusters::nearestCid(Sample& sample,int rpos,DataInfo &dinfo){

  double distanceTmp;
  double distance=9999;

  int k=-1;

  vector<double> disFld(_fCnt);


  for(int c=0; c < _cluster.size(); c++){
    int j=0;

    /*数値項目*/
    for(int i=0; i< _fCnt; i++){
      double a=norm( _cluster[c].gGet(i) ,dinfo.Min(i),dinfo.Rng(i));
      double b=norm( sample.vGet(rpos,i) ,dinfo.Min(i),dinfo.Rng(i));
      disFld[j++]= disNum(a,b);
    }
    //カテゴリ項目

    //結合距離の計算
    distanceTmp=disAvg(disFld,j);
    //距離が小さければ更新
    if( distanceTmp < distance){
      distance=distanceTmp;
      k=c;
    }
  }

  return(k);
}




//----------------------------------------------------------------------------
// 各クラスタの重心を計算し、重心が変われば1変わらなければ0を返す             
//----------------------------------------------------------------------------
int Clusters::movCenter(){
  int mov=0; // 重心が動いたフラグ
  double newCen;
  int i,k;

  for(k=0; k<_cluster.size(); k++){
    for(i=0; i<_fCnt; i++){
    	if(_cluster[k].Cnt(i)!=0){
	    	newCen = _cluster[k].Avg(i);
        if( newCen != _cluster[k].gGet(i) ){
	        _cluster[k].gSet(i,newCen);
          mov=1;
        }
    	}
    }
  }
  return(mov);
}

 
//----------------------------------------------------------------------------
// サンプリングデータを各クラスタに振り分け、重心計算のための値を更新していく  
//----------------------------------------------------------------------------
void Clusters::convergentBySmp(Sample& sample,DataInfo &dinfo){

	while(1){

		wrkReset();
	
	  for(int smp=0; smp < sample.Cnt(); smp++){

	    int k=nearestCid(sample,smp,dinfo);
  	  if(k!=-1){
	  	  _cluster[k].incCnt(); //_クラスタに属するレコード数
      	for(int i=0; i<_fCnt; i++){
		      double v = sample.vGet(smp,i);
    		  _cluster[k].acum(i , v);
      	}
    	}
  	}
  	if(!movCenter()) break;
  }
}
//----------------------------------------------------------------------------
/*多分いらん
void Clusters::setClusterSmp(Sample& sample,DataInfo &dinfo){

	int k;
	wrkReset();

  for(int smp=0; smp < sample.Cnt(); smp++){

    k=nearestCid(sample,smp,dinfo);
    if(k==-1){ throw kgError("cluster NO -1"); }
		// -1の時どうする？
    _cluster[k].incCnt();

    for(int i=0; i<sample.fCnt(); i++){
      double v = sample.vGet(smp,i);
      _cluster[k].acum(i , v);
    }
	}
}
*/
 

// k番目のクラスタから最も離れたサンプルをクラスタの中心として登録する
// for BRADLEY & FAYYAD APPROACH
void Clusters::farthest(
  int k,
  Sample & sample, 
  DataInfo &dinfo)
{

  //Clusters cls; //新しいクラスタ
  double dis;
  double dis_max=-1;
  int i_max=1;
  int i;

  // start i_max の算出 
  for(i=0; i< sample.Cnt(); i++){

		dis= _cluster[k].calDist(sample,i,dinfo);

    if( dis > dis_max){
      dis_max=dis;
      i_max=i;
    }
  }

	_cluster[k].resetCalnum();	
  setSmp2Cluster(k,i_max,sample);

  return;
}

//----------------------------------------------------------------------------
// サンプルデータからcluster個数分ランダムに選択
//----------------------------------------------------------------------------
void Clusters::initRA_MAIN(
	Sample& org,
	boost::variate_generator< boost::mt19937,boost::uniform_int<>  > * rmod
){


 	size_t recCnt = _cCnt > org.Cnt() ? org.Cnt() : _cCnt;
  Sample newsamp;
  newsamp.setSize(recCnt,_fCnt);

  size_t remaining = org.Cnt();
  size_t select = recCnt;


  for(size_t i=0; i< org.Cnt(); i++){

    if(  (*rmod)()%remaining  < select ){
      for(int j=0; j< _fCnt; j++){
	      newsamp.vSet( recCnt-select,j, org.vGet(i,j));
      }
      select--;
    }
    remaining--;
  }

  for(size_t k=0; k<newsamp.Cnt(); k++){
    setSmp2Cluster(k,k,newsamp);
  }

}

//----------------------------------------------------------------------------
// ランダム初期化
//----------------------------------------------------------------------------
size_t Clusters::initRA( 
kgCSVfld *csv, kgArgFld *fld,
 size_t cCnt , unsigned long seed, 
 size_t mcnt, size_t rcnt)
{

	// 初期設定
	_ffld = fld;
	_cCnt = cCnt;
	_icsv = csv;
	_fCnt = fld->size();
	variate_generator< mt19937,uniform_int<> > randM( mt19937((uint64_t)seed),uniform_int<>(0,INT_MAX));


	getDataInfo();
	
	// サンプリング , clusrteサイズもチェック
	vector < Sample > sample0(mcnt);
	csv2sample(sample0,rcnt,&randM); 
  cResize(_cCnt,_fCnt);

  // サンプルデータからcluster個数分ランダムに選択
	initRA_MAIN(sample0[0],&randM);

  return _cCnt;
}

//----------------------------------------------------------------------------
// KA初期化     
// KAUFMAN APPROACH                                                           
// Reference :                                                                
// Lozano, "An empirical comparison of four initialization methods            
// for the K-Mean",p6.                                                        
//----------------------------------------------------------------------------
size_t Clusters::initKA(
kgCSVfld *csv, kgArgFld *fld,
 size_t cCnt , unsigned long seed, 
 size_t mcnt, size_t rcnt
){

	// 初期設定
	_ffld = fld;
	_cCnt = cCnt;
	_icsv = csv;
	_fCnt = fld->size();
	variate_generator< mt19937,uniform_int<> > randM( mt19937((uint64_t)seed),uniform_int<>(0,INT_MAX));


	getDataInfo();

	// サンプリング , clusrteサイズもチェック
	vector < Sample > sample0(mcnt);
	csv2sample(sample0,rcnt,&randM); 
  cResize(_cCnt,_fCnt);

  // シード番号を納める領域の確保
  vector<int> sList(_cCnt);

	int pos = 0 ; //固定値
  int k=0;
  int s;

  s=getCenInstance(sample0[0],_dinfo);
  sList[k]=s;
  setSmp2Cluster(k++,s,sample0[0]);

  double    Cji,Cji_max,val0=0;
  int i_max=0;

  do {
    Cji_max=-1;
    for(int i=0; i<sample0[0].Cnt(); i++){

      if( isInCluster(i,sList,k) ) continue;

      Cji=0;
      for(int j=0; j<sample0[0].Cnt(); j++){
        if(isInCluster(j,sList,k)) continue;
        if(i==j) continue;
        double tt = get_Dj(sample0[0],_dinfo,k,j) - get_dji(sample0[0],_dinfo,i,j);
        Cji = Cji + ( tt - val0 > 0 ? tt : val0);
      }

      if(Cji>Cji_max){
        Cji_max=Cji;
        i_max=i;
      }
      
    }
    sList[k]=i_max;
    setSmp2Cluster(k++,i_max,sample0[0]);

  } while( k < cCnt );

  return _cCnt;

}



/*----------------------------------------------------------------------------*/
/* BRADLEY & FAYYAD APPROACH                                                  */
/* Reference :                                                                */
/* Bradley, Fayyad, "Refining Initial Points for K-Means Clustering"          */
/* CMi : CM[i] */
/* CM  : smpCM */
/* FMi : FM[i] */
/*----------------------------------------------------------------------------*/
size_t Clusters::initBFbyRA(
	kgCSVfld *csv, kgArgFld *fld,
	size_t cCnt , unsigned long seed, 
	size_t mcnt, size_t rcnt
){

	// 初期設定
	_ffld = fld;
	_cCnt = cCnt;
	_icsv = csv;
	_fCnt = fld->size();
	variate_generator< mt19937,uniform_int<> > randM( mt19937((uint64_t)seed),uniform_int<>(0,INT_MAX));


	getDataInfo();
	
	// サンプリング , clusrteサイズもチェック
	vector < Sample > sample(mcnt);
	csv2sample(sample,rcnt,&randM); 


  int i,j,flg;
  int minFMi=0;
	double minFM;
  double tmpFM;

	vector < Clusters > CM(sample.size());

	do {
		// 新しいサンプルの作成 
  	for(int s=0; s< sample.size(); s++){
	  	//CM[s].initRA( cCnt, sample[s], rmod);
	  	CM[s].cResize(_cCnt,_fCnt);
			CM[s].initRA_MAIN( sample[s], &randM);

	    // CONVERGENCE (kmean-mod)
  	  for(i=0;i<2;i++){
    	  // k-mean 
				CM[s].convergentBySmp(sample[s],_dinfo);
	      // k-mean mod
  	    flg=0;
    	  for(j=0; j< _cCnt; j++){
      	  // 空のクラスタもしくはNULLのクラスタがあった
       		if( CM[s].recEmpty(j)){
        	  CM[s].farthest(j,sample[s],_dinfo);
          	flg=1; //空のクラスタがあったフラグ
          	break;
        	}
      	}
      	if(!flg) break;
      	//2回目の試行で空のクラスがあればNULLを返す
      	if(i==1 && flg){
        	_cCnt--;
        	break;
      	}
	    }
		}
		
 	}while(flg);

  // クラスタの重心をサンプルとして登録する
	vector < Clusters > FM(sample.size());
  Sample  smpCM;  

  //領域確保(smpCM)
  smpCM.setSize(sample.size() * _cCnt ,_fCnt );
	size_t pos=0;
  for(int s=0; s < sample.size(); s++){

    for(i=0; i< _cCnt; i++){
      for(j=0; j<_fCnt; j++){
	      smpCM.vSet( pos ,j,CM[s].gGet(i,j) );
      }
      pos++;
    }
  }

  // FM[i]の算出
  for(int s=0;  s< sample.size(); s++){

		FM[s].cResize(_cCnt,_fCnt);
    //CM[i]をFM[i]の初期値とする
    for(i=0; i< _cCnt; i++){
      for(j=0; j< _fCnt; j++){
      	FM[s].gSet(i,j,CM[s].gGet(i,j) );
      }
    }
    // CONVERGENCE
		FM[s].convergentBySmp(smpCM,_dinfo);
  }

  // DISTORTION(FM[i],smpCM)         
  // もっともsmpCMにfitするFM[i]を選ぶ
	minFM=DBL_MAX;
  for(int s=0; s< sample.size(); s++){

  	//FM[s].setClusterSmp(smpCM,_dinfo);// <=多分いらん

		tmpFM=FM[s].ttlAcum();
    if( minFM > tmpFM){
      minFM =tmpFM;
      minFMi=s;
    }
  }
	cResize(_cCnt,_fCnt);

  clscopy(FM[minFMi]);

  return _cCnt;
}




