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



/*----------------------------------------------------------------------------*/
/* 数値間の距離を計算                                                         */
/*----------------------------------------------------------------------------*/
static inline double disNum(double x, double y){

  double sub;
  /*ユークリッド距離絶対値*/
  sub= x - y;

  if(sub<0) return sub*(-1);
  else      return sub;

	 /*ユークリッド距離２乗*/
  /*rsl.v.d = (x.v.d - y.v.d)*(x.v.d - y.v.d);*/

//  return(rsl);
}


static inline double norm(double num, double min, double rng){
	return (num-min)/rng;

}

/*----------------------------------------------------------------------------*/
/* 各項目の距離の平均を計算(NULLは計算に入れない)                             */
/*----------------------------------------------------------------------------*/
static inline double disAvg(vector<double> &disFld, int cnt){
  double sum=0;
  int j=0;
  int i;

  /*総和*/
  for(i=0;i<cnt;i++){
    sum+=disFld[i];
  }
  return sum/(double)cnt;
}

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
	_args.paramcheck("n=,f=,i=,o=,S=,d=,m=,a=");

	// 入出力ファイルオープン
	_iFile.open(_args.toString("i=",false), _env,_nfn_i);
  _oFile.open(_args.toString("o=",false), _env,_nfn_o);

	// f= 項目引数のセット
	vector<kgstr_t> vs_f = _args.toStringVector("f=",true);

	kgstr_t cnum = _args.toString("n=",true);
	_clusterCnt = atoi(cnum.c_str());

	kgstr_t dt = _args.toString("d=",false);
	if(dt.empty()){ _dType = 2; }
	else          { _dType = atoi(dt.c_str()); }
	if ( _dType < 0 || _dType > 2 ){
			throw kgError("parameter d= is 0-2");
	}



	kgstr_t mt = _args.toString("m=",false);
	if(mt.empty()){ _mcnt = 10; }
	else          { _mcnt = atoi(dt.c_str()); }

	// a= 追加項目名
	_addstr = _args.toString("a=",false);
	if(_addstr.empty()&& _nfn_o==false){
		throw kgError("parameter a= is mandatory");
	}


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
	_rand_mod.set( new variate_generator< mt19937,uniform_int<> >
			(mt19937((uint64_t)_seed),uniform_int<> (0,INT_MAX)) );

	_iFile.read_header();
	_fField.set(vs_f, &_iFile,_fldByNum);
	
}


void kgKmeans::setSmp2Cluster(
  int k,
	Sample  *sample,
  int s,
  Clusters *clusters
  ){

  int i;
  /*数値項目*/
  for(i=0; i<_fField.size(); i++){
  	clusters->set_cenNum(k,i,sample->getVal(s,i)) ;
  }
}

/*----------------------------------------------------------------------------*/
/*ランダム初期化                                                              */
/*----------------------------------------------------------------------------*/
void kgKmeans::initClusterRA(int pos ,Clusters* clusters){

  int k=0;                 /*クラスタ番号*/
  /* k個をランダムに選択*/
  // サンプルデータからcluster個数分サンプリング
  Sample newsamp;
	int recCnt = _clusterCnt;
  /*サンプリング件数の調整*/
  if(_clusterCnt > _sample[pos].getCnt()) recCnt=_sample[pos].getCnt();

  newsamp.setSize(recCnt,_fField.size());

  int remaining = _sample[pos].getCnt();
  int select = recCnt;



  for(int r=0; r<_sample[pos].getCnt(); r++){

    if( ((*_rand_mod.get())()%remaining) < select ){
      /*数値項目*/
      for(int j=0; j<_fField.size(); j++){
	      newsamp.setVal(recCnt-select,j,_sample[pos].getVal(r,j));
      }
      select--;
    }
    remaining--;
  }

  newsamp.setCnt(recCnt);


	clusters->resize(_clusterCnt,_fField.size());

  for(k=0; k<newsamp.getCnt(); k++){
    setSmp2Cluster(k,&newsamp,k,clusters);
  }
}
/*----------------------------------------------------------------------------*/
/* cluster と n 番名のサンプルとの距離                                 */
/*----------------------------------------------------------------------------*/
double kgKmeans::calDistanceClsSmp(Cluster &cluster, int n,int spos){

	vector<double> disFld(_fField.size());
  int j=0;
  /*数値項目*/
  for(int i=0; i<_fField.size(); i++){
  	if(cluster.is_cenNum(i) || _datainfo.getRng(i) ==0 ) continue;
    double a=norm(cluster.get_cenNum(i) ,_datainfo.getMin(i),_datainfo.getRng(i));
    double b=norm(_sample[spos].getVal(n,i),_datainfo.getMin(i),_datainfo.getRng(i));
    disFld[j++]=disNum(a,b);
  }
  if(j==0){ return DBL_MAX;}

  /*カテゴリ項目*/

  /*結合距離の計算*/
  double sum=disAvg(disFld,j);

  return( sum );
}
/*----------------------------------------------------------------------------*/
/* KAUFMAN APPROACH                                                           */
/* Reference :                                                                */
/* Lozano, "An empirical comparison of four initialization methods            */
/* for the K-Mean",p6.                                                        */
/*------------------------------_----------------------------------------------*/
/*重心に最も近いレコード番号を取得*/
int kgKmeans::getCenInstance(int pos){

  Cluster clusterT;
  clusterT.setSize(_fField.size());
  
  
  for(int i=0; i<_sample[pos].getCnt(); i++){
    for(int j=0; j<_fField.size(); j++){
    	// nullチェックいる？
      //if(! (*((sample->rec+i)->num+j)).nul ){
	    clusterT.addAccm(j,_sample[pos].getVal(i,j));
    }
  }

  for(int j=0; j<_fField.size(); j++){

    if( clusterT.getCalCnt(j) !=0 ){
    	clusterT.set_cenNum(j ,clusterT.getCalAvg(j));
    }
  }

  int k=0;
  //mssVinit(&distance,DBL);
  double distance=DBL_MAX;
  for(int i=0; i<_sample[pos].getCnt(); i++){
    double disTmp=calDistanceClsSmp(clusterT,i,pos);
    if( distance > disTmp ){
      distance=disTmp;
      k=i;
    }
  }

  return(k);
}
/*sListのk番目までに、sがあれば１を返す*/
static int isInCluster(int s,vector<int> &sList, int k){
  int i;
  for(i=0; i<k; i++){
    if( s==sList[i] ) return(1);
  }
  return(0);
}


/*----------------------------------------------------------------------------*/
/* m番目のサンプル と n番目のサンプルとの距離                                */
/*----------------------------------------------------------------------------*/
double kgKmeans::get_dji(Sample *sample, int m, int n ){


	vector<double> disFld(_fField.size());
	
  int j=0;
  /*数値項目*/
  for(int i=0; i<_fField.size(); i++){
    double a=norm(sample->getVal(m,i),_datainfo.getMin(i),_datainfo.getRng(i));
    double b=norm(sample->getVal(n,i),_datainfo.getMin(i),_datainfo.getRng(i));
    disFld[j++]=disNum(a,b);
  }

  /*結合距離の計算*/
  double sum=disAvg(disFld,j);

  return( sum );
}


/*現在登録されている各クラスタとサンプルjとの最短距離を返す */
double kgKmeans::get_Dj(
  int k,                  /*現在登録されているクラスタ数*/
  int j){

  double dis;
  double disMin;
  int s_min=0;
  int s;

  disMin=DBL_MAX;

  for(s=0; s<k; s++){
    dis=calDistanceClsSmp(_clusters.at(s),j,0);
    if(disMin>dis){
      disMin=dis;
    }
  }
  return(disMin);
}

/* メインルーチン (KA) */
void kgKmeans::initClusterKA(int pos ,Clusters* clusters){

  int k=0;
  int i,j,i_max=0;
  int s;

	clusters->resize(_clusterCnt,_fField.size());

  /*シード番号を納める領域の確保*/
  vector<int> sList(_clusterCnt);

  k=0;
  s=getCenInstance(pos);

  sList[k]=s;
  setSmp2Cluster(k++,&_sample[0],s,clusters);

  double    Cji,Cji_max,val0=0;

  while(1){
    Cji_max=-1;
    for(i=0; i<_sample[0].getCnt(); i++){
      if(isInCluster(i,sList,k)) continue;
      Cji=0;
      for(j=0; j<_sample[0].getCnt(); j++){
        if(isInCluster(j,sList,k)) continue;
        if(i==j) continue;
        double tt = get_Dj(k,j) - get_dji(&_sample[0],i,j);
        Cji = Cji + ( tt - val0 > 0 ? tt : val0);
      }
      if(Cji>Cji_max){
        Cji_max=Cji;
        i_max=i;
      }
      
    }

    sList[k]=i_max;
    setSmp2Cluster(k++,&_sample[0],i_max,clusters);
    if(k==_clusterCnt) break;
  }

  return;
}


//----------------------------------------------------------------------------
// ファイルからrecCnt件サンプリングし、sample[]にセットする                   
//  mul, 何セット用意するか
//  recCnt   サンプリング行数
//----------------------------------------------------------------------------
void kgKmeans::sampling(int mul,int recCnt)  
{ 
  /*サンプリング件数の調整*/
  if(recCnt < 100      ) recCnt=100;
  if(recCnt > 5000     ) recCnt=5000;
  if(recCnt > _datainfo.getCnt()) recCnt=_datainfo.getCnt();

	_sample.resize(mul);

  /*領域確保*/
  for(int i=0; i<mul; i++){
  	_sample[i].setSize(recCnt,_fField.size());
  }

	_iFile.seekTop();

  vector<int> select(mul);
  vector<int> remaining(mul);

  for(int i=0; i<mul; i++){//mul10以上でだめ
    select[i]=recCnt;
    remaining[i]=_datainfo.getCnt();
  }

	while(_iFile.read()!=EOF){
	
    for(int i=0; i<mul; i++){

      if( ((*_rand_mod.get())()%remaining[i]) < select[i] ){
        /*数値項目*/
				for(size_t j=0 ; j<_fField.size(); j++){ 
					char *str = _iFile.getVal(_fField.num(j));
          if(*str!='\0'){ 
	          _sample[i].setVal(recCnt-select[i],j,atof(str));
        	}
        	else{
	          _sample[i].setVal(recCnt-select[i],j,_datainfo.getAvg(i));
        	}
        	
        }
        select[i]--;
      }
      remaining[i]--;
    }
  }

  for(int i=0; i<mul; i++){ _sample[i].setCnt (recCnt); }
}



void kgKmeans::getDatInfo(){

	_datainfo.sizeSet(_fField.size());

	while(_iFile.read()!=EOF){
		for(size_t i=0 ; i<_fField.size(); i++){ 
			char *str = _iFile.getVal(_fField.num(i));
			if(*str!='\0'){
				double v = atof(str);
				_datainfo.info_set(i,v);
			}
		}
		_datainfo.cnt_inc();
	}

	if(!_datainfo.existAllData()){
		throw kgError("value not found on some fields");
	}
	_datainfo.cal_rng_avg();

  if(_datainfo.getCnt()<_clusterCnt) _clusterCnt = _datainfo.getCnt();
	
}

/*----------------------------------------------------------------------------*/
/* 各クラスタの重心を計算し、重心が変われば1変わらなければ0を返す             */
/*----------------------------------------------------------------------------*/
int kgKmeans::movCenter(Clusters *clusters){
  int mov=0; /*重心が動いたフラグ*/
  double newCen;
  int i,k;


  for(k=0; k<_clusterCnt; k++){
    /*数値項目*/
    for(i=0; i<_fField.size(); i++){
    	if(clusters->getCalCnt(k,i)!=0){
	    	newCen = clusters->getCalAvg(k,i);
        if( newCen != clusters->get_cenNum(k,i) ){
        	clusters->set_cenNum(k,i,newCen);
          mov=1;
        }
    	}
    }
  }
  return(mov);
}


/*----------------------------------------------------------------------------*/
/*与えられた行が最も近いクラスタ番号を返す                                    */
/*  クラスタが全てNULLの場合,もしくはデータ項目が全てNULLの場合,もしくは      */
/*  クラスタの項目とデータの項目がうまくNULLでかみあった場合は-1を返すことに  */
/*  なる。すなわち、近いクラスタは分からないということになる                  */
/*----------------------------------------------------------------------------*/
int kgKmeans::nearestCluster(){

  vector<double> disFld(2);
	double a;
	double b;
  double distance=9999;
	int k=-1;

  for(int c=0; c<_clusterCnt; c++){

    int j=0;

    /*数値項目*/
    for(size_t i=0; i<_fField.size(); i++){

    	if( _datainfo.getRng(i) == 0 ) continue;

      a=norm(_clusters.get_cenNum(c,i), _datainfo.getMin(i), _datainfo.getRng(i));

			char *str = _iFile.getVal(_fField.num(i));
			
      if(*str=='\0') { b = _datainfo.getAvg(i);}
      else        { b=atof(str);}

      b=norm(b,_datainfo.getMin(i), _datainfo.getRng(i));
      disFld[j++] = disNum(a,b);
 
    }
 
    if(j==0){ continue;}

    /*結合距離の計算*/
    double distanceTmp=disAvg(disFld,j);

    /*距離が小さければ更新*/
    if(distanceTmp<distance){
      distance=distanceTmp;
      k=c;
    }
  }

  return(k);
}

/*----------------------------------------------------------------------------*/
/*全データを各クラスタに振り分け、重心計算のための値を更新していく            */
/*----------------------------------------------------------------------------*/
void kgKmeans::setCluster(){
  int i,k;
  char *s;

	_clusters.reset();

	_iFile.seekTop();

	while(_iFile.read()!=EOF){

    k=nearestCluster();
    if(k!=-1){
      _clusters.incCnt(k); /*クラスタに属するレコード数*/
      /*数値項目*/
      for(i=0; i<_fField.size(); i++){
	      s = _iFile.getVal(_fField.num(i));
	      if(*s=='\0'){
		      _clusters.addAccm(k,i,_datainfo.getAvg(i));
	      }
	      else{
		      _clusters.addAccm(k,i,atof(s));
		    }
      }
    }
  }
}


/*----------------------------------------------------------------------------*/
/*与えられた行が最も近いクラスタ番号を返す(sample用)                          */
/*  クラスタが全てNULLの場合,もしくはデータ項目が全てNULLの場合,もしくは      */
/*  クラスタの項目とデータの項目がうまくNULLでかみあった場合は-1を返すことに  */
/*  なる。すなわち、近いクラスタは分からないということになる                  */
/*----------------------------------------------------------------------------*/
int kgKmeans::nearestClusterSmp(Clusters *clusters,Sample *sample,int rpos){
  double  distanceTmp;

  int  j,k;


  k=-1;
  double distance=9999;
  vector<double> disFld(_fField.size());


  for(int c=0; c<_clusterCnt; c++){
    j=0;

    /*数値項目*/
    for(int i=0; i< _fField.size(); i++){
      double a=norm( clusters->get_cenNum(c,i) ,_datainfo.getMin(i),_datainfo.getRng(i));
      double b=norm( sample->getVal(rpos,i),_datainfo.getMin(i),_datainfo.getRng(i));
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
//サンプリングデータを各クラスタに振り分け、重心計算のための値を更新していく  */
//----------------------------------------------------------------------------
void kgKmeans::setClusterSmp(Clusters *clusters,  Sample*  sample){

	int k;
	clusters->reset();

  for(int smp=0; smp< sample->getCnt(); smp++){
    k=nearestClusterSmp(clusters,sample,smp);
    clusters->incCnt(k); /*クラスタに属するレコード数*/

    /*数値項目*/
    for(int i=0; i<_fField.size(); i++){
      double v = sample->getVal(smp,i);
			clusters->addAccm(k,i ,v);
    }

  }
}


/*----------------------------------------------------------------------------*/
/* BRADLEY & FAYYAD APPROACH                                                  */
/* Reference :                                                                */
/* Bradley, Fayyad, "Refining Initial Points for K-Means Clustering"          */
/* CMi : CM[i] */
/* CM  : smpCM */
/* FMi : FM[i] */
/*----------------------------------------------------------------------------*/
/*k番目のクラスタから最も離れたサンプルをクラスタの中心として登録する*/
void kgKmeans::farthest(
  Clusters *clusters,
  int k,
  int spos){

  //Clusters cls; /*新しいクラスタ*/
  double dis;
  double dis_max=-1;
  int i_max=1;
  int i;

  /* start i_max の算出 */
  for(i=0; i<_sample[spos].getCnt(); i++){
    dis=calDistanceClsSmp(_clusters.at(k),i,spos);
    if( dis > dis_max){
      dis_max=dis;
      i_max=i;
    }
  }
	clusters->resetCalnum(k);

  setSmp2Cluster(k,&_sample[spos],i_max,clusters);
  return;
}
Clusters *kgKmeans::initClusterBF(){

  Sample  smpCM;  //新しいサンプル

  int i,j,flg;
  int minFMi=0;
	double minFM;
  double tmpFM;

	vector < Clusters > CM(_mcnt);
	vector < Clusters > FM(_mcnt);
	
  // 新しいサンプルの作成 
  for(int s=0; s<_mcnt; s++){
    // 領域確保(CM[s])
		CM[s].resize(_clusterCnt,_fField.size());

    // 入力データから初期値としてk個の重心を得る
    ///initClusterKA(s,&CM[s]);
    initClusterRA(s,&CM[s]);

    //showCluster(CM[s]);

    // CONVERGENCE (kmean-mod)
    for(i=0;i<2;i++){
      // k-mean 
      while(1){
        setClusterSmp(&CM[s],&_sample[s]);
        if(!movCenter(&CM[s]))break;
      }

      // k-mean mod
      flg=0;
      for(j=0; j<_clusterCnt; j++){
        // 空のクラスタもしくはNULLのクラスタがあった
        if( CM[s].getCnt(j)== 0){
          farthest(&CM[s],j,s);
          flg=1; //空のクラスタがあったフラグ
          break;
        }
      }
      if(!flg) break;
      //2回目の試行で空のクラスがあればNULLを返す
      if(i==1 && flg){
        return(NULL);
      }
    }
  }

  // クラスタの重心をサンプルとして登録する
  //領域確保(smpCM)
  smpCM.setSize(_mcnt * _clusterCnt ,_fField.size() );
  smpCM.setCnt(0);


  for(int s=0; s < _mcnt; s++){
    for(i=0; i<_clusterCnt; i++){
      for(j=0; j<_fField.size(); j++){
	      smpCM.setVal( smpCM.getCnt() ,j,CM[s].get_cenNum(i,j) );
      }
      smpCM.incCnt();
    }
  }

  // FM[i]の算出
  for(int s=0;  s<_mcnt; s++){

		FM[s].resize(_clusterCnt,_fField.size());
    //CM[i]をFM[i]の初期値とする
    //数値項目
    for(i=0; i<_clusterCnt; i++){
      for(j=0; j<_fField.size(); j++){
      	FM[s].set_cenNum(i,j,CM[s].get_cenNum(i,j) );
      }
    }
    // CONVERGENCE
    while(1){
      setClusterSmp(&FM[s],&smpCM);
      if(!movCenter(&FM[s]))break;
    }
  }

  // DISTORTION(FM[i],smpCM)         
  // もっともsmpCMにfitするFM[i]を選ぶ
	minFM=DBL_MAX;
  for(int s=0; s<_mcnt; s++){
    setClusterSmp(&FM[s], &smpCM);
		tmpFM=0;
    for(i=0; i<_clusterCnt; i++){
      //数値
      for(j=0; j<_fField.size(); j++){
        tmpFM += FM[s].getAccm(i,j);
      }
    }
    if( minFM > tmpFM){
      minFM =tmpFM;
      minFMi=s;
    }
  }
  _clusters = FM[minFMi];
  return &_clusters;
}
 

// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgKmeans::run(void) try {

	setArgs();

	getDatInfo();

  switch(_dType){
  	case 0: /*---------------------------- ランダム*/
			
  		sampling(1,100);
	    initClusterRA(0,&_clusters);
	    break;

  	case 1: /*---------------------------- Kaufman Approach*/
	    sampling(1,100);
    	initClusterKA(0,&_clusters);
	    break;

	  case 2: /*---------------------------- Bradley & Fayyad Approach*/
	    sampling(_mcnt,100);

    	while(1){
      	if(initClusterBF()==NULL) _clusterCnt--;
      	else              break;
    	}
	    break;
	  
	}

  while(1){
    setCluster();
    if(!movCenter(&_clusters))break;
  }
  
	_iFile.seekTop();

	// 項目名の出力
  _oFile.writeFldName(_iFile,_addstr);


	while(_iFile.read()!=EOF){

    int k=nearestCluster();
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

