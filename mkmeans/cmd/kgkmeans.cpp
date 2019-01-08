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
// sListのk番目までに、sがあれば１を返す 
static bool isInCluster(int s,vector<int> &sList, int k){

  for(int i=0; i<k; i++){
    if( s==sList[i] ) return(true);
  }
  return(false);
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

double Clusters::calDist(Cluster &cluster,Sample& sample,size_t rno ,DataInfo & dinfo){

	vector<double> disFld(sample.Cnt());

  int j=0;

  for(int i=0; i<sample.fCnt(); i++){
  	if( cluster.gExist(i) || dinfo.Rng(i) ==0 ) continue;
    double a=norm( cluster.gGet(i)  , dinfo.Min(i), dinfo.Rng(i));
    double b=norm( sample.vGet(rno,i) , dinfo.Min(i), dinfo.Rng(i));
    disFld[j++]=disNum(a,b);
  }
  if(j==0){ return DBL_MAX;}

  /*カテゴリ項目*/

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
    disTmp=calDist(clusterT,sample,i,dinfo);
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
double Clusters::get_dji(
	Sample& sample, 
  DataInfo& dinfo,
	int m, int n 
){


	vector<double> disFld(sample.fCnt());
	
  int j=0;
  /*数値項目*/
  for(int i=0; i<sample.fCnt(); i++){
    double a=norm(sample.vGet(m,i), dinfo.Min(i), dinfo.Rng(i));
    double b=norm(sample.vGet(n,i), dinfo.Min(i), dinfo.Rng(i));
    disFld[j++]=disNum(a,b);
  }

  /*結合距離の計算*/
  double sum=disAvg(disFld,j);

  return( sum );
}

//----------------------------------------------------------------------------
// 現在登録されている各クラスタとサンプルjとの最短距離を返す 
//----------------------------------------------------------------------------
double Clusters::get_Dj(
	Sample& sample,
  DataInfo& dinfo,
  int k,int j){

  double dis;
  double disMin;
  int s_min=0;
  int s;

  disMin=DBL_MAX;

  for(s=0; s<k; s++){
    dis=calDist(_cluster[s],sample,j,dinfo);
    if(disMin>dis){
      disMin=dis;
    }
  }
  return(disMin);
}

//----------------------------------------------------------------------------
// KA初期化     
// KAUFMAN APPROACH                                                           
// Reference :                                                                
// Lozano, "An empirical comparison of four initialization methods            
// for the K-Mean",p6.                                                        
//----------------------------------------------------------------------------
void Clusters::initKA(
	size_t cCnt ,
	Sample & sample,
	DataInfo &dinfo
){

	int pos = 0 ; //固定値
  int k=0;
  int s;
	
	cResize(cCnt,sample.fCnt());

  // シード番号を納める領域の確保
  vector<int> sList(cCnt);

  k=0;

  s=getCenInstance(sample,dinfo);
  sList[k]=s;
  setSmp2Cluster(k++,s,sample);

  double    Cji,Cji_max,val0=0;
  int i_max=0;

  do {
    Cji_max=-1;
    for(int i=0; i<sample.Cnt(); i++){

      if( isInCluster(i,sList,k) ) continue;

      Cji=0;
      for(int j=0; j<sample.Cnt(); j++){
        if(isInCluster(j,sList,k)) continue;
        if(i==j) continue;
        double tt = get_Dj(sample,dinfo,k,j) - get_dji(sample,dinfo,i,j);
        Cji = Cji + ( tt - val0 > 0 ? tt : val0);
      }

      if(Cji>Cji_max){
        Cji_max=Cji;
        i_max=i;
      }
      
    }
    sList[k]=i_max;
    setSmp2Cluster(k++,i_max,sample);

  } while( k < cCnt );

  return;
}
//----------------------------------------------------------------------------
// ランダム初期化                                                              
//----------------------------------------------------------------------------
void Clusters::initRA(
	size_t cCnt ,  Sample & sample,
	boost::variate_generator< boost::mt19937,boost::uniform_int<>  > * rmod
){

	int pos=0;
  int k=0;                 //クラスタ番号

  cResize(cCnt,sample.fCnt());

  // サンプルデータからcluster個数分ランダムに選択
  Sample newsamp;
	int recCnt = cCnt;
  if( cCnt > sample.Cnt()) recCnt= sample.Cnt();

  newsamp.setSize(recCnt,sample.fCnt());

  int remaining = sample.Cnt();
  int select = recCnt;

  for(int r=0; r< sample.Cnt(); r++){

    if(  (*rmod)()%remaining  < select ){
      for(int j=0; j< sample.fCnt(); j++){
	      newsamp.vSet( recCnt-select,j, sample.vGet(r,j));
      }
      select--;
    }
    remaining--;
  }

  for(k=0; k<newsamp.Cnt(); k++){
    setSmp2Cluster(k,k,newsamp);
  }
}

//----------------------------------------------------------------------------
// ファイルからrecCnt件サンプリングし、sample[]にセットする                   
//  mul, 何セット用意するか
//  recCnt   サンプリング行数
// NULLの場合avgセット
//----------------------------------------------------------------------------
void kgKmeans::sampling(int mul,int recCnt)  
{ 
  /*サンプリング件数の調整*/
  if(recCnt < 100      ) recCnt=100;
  if(recCnt > 5000     ) recCnt=5000;
  if(recCnt > _dinfo.Rec()) recCnt=_dinfo.Rec();

  /*領域確保*/
  vector<int> select(mul);    // 必要サンプリング数
  vector<int> remaining(mul); // データ行数残

	_sample.resize(mul);

  for(int i=0; i<mul; i++){
  	_sample[i].setSize(recCnt,_fField.size());
    select[i]=recCnt;
    remaining[i]=_dinfo.Rec();
  }

	_iFile.seekTop();

	while(_iFile.read()!=EOF){
	
    for(int i=0; i<mul; i++){

      if( ((*_rand_mod.get())()%remaining[i]) < select[i] ){
        /*数値項目*/
				for(size_t j=0 ; j<_fField.size(); j++){ 
					char *str = _iFile.getVal(_fField.num(j));
          if(*str!='\0'){ 
	          _sample[i].vSet(recCnt-select[i],j,atof(str));
        	}
        	else{
	          _sample[i].vSet(recCnt-select[i],j,_dinfo.Avg(i));
        	}
        	
        }
        select[i]--;
      }
      remaining[i]--;
    }
  }
}


// 入力データのsumary情報取得
// ( cnt ,sum ,min, max)
void kgKmeans::getDataInfo(){

	_dinfo.init(_fField.size());

	while(_iFile.read()!=EOF){
		for(size_t i=0 ; i<_fField.size(); i++){ 
			char *str = _iFile.getVal(_fField.num(i));
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

  if(_dinfo.Rec()<_clusterCnt) _clusterCnt = _dinfo.Rec();
	
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

  vector<double> disFld(_fField.size());
	double a;
	double b;
  double distance=9999;
	int k=-1;

  for(int c=0; c<_clusterCnt; c++){

    int j=0;

    /*数値項目*/
    for(size_t i=0; i<_fField.size(); i++){

    	if( _dinfo.Rng(i) == 0 ) continue;

      a=norm(_clusters.get_cenNum(c,i), _dinfo.Min(i), _dinfo.Rng(i));

			char *str = _iFile.getVal(_fField.num(i));
			
      if(*str=='\0') { b = _dinfo.Avg(i);}
      else        { b=atof(str);}

      b=norm(b,_dinfo.Min(i), _dinfo.Rng(i));
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

	_clusters.wrkReset();

	_iFile.seekTop();

	while(_iFile.read()!=EOF){

    k=nearestCluster();
    if(k!=-1){
      _clusters.incCnt(k); /*クラスタに属するレコード数*/
      /*数値項目*/
      for(i=0; i<_fField.size(); i++){
	      s = _iFile.getVal(_fField.num(i));
	      if(*s=='\0'){
		      _clusters.addAccm(k,i,_dinfo.Avg(i));
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
int Clusters::nearestClusterSmp(Sample& sample,int rpos,DataInfo &dinfo){

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
//サンプリングデータを各クラスタに振り分け、重心計算のための値を更新していく  */
//----------------------------------------------------------------------------
void Clusters::setClusterSmp(Sample& sample,DataInfo &dinfo){

	int k;
	wrkReset();

  for(int smp=0; smp < sample.Cnt(); smp++){

    k=nearestClusterSmp(sample,smp,dinfo);
    if(k==-1){ throw kgError("cluster NO -1"); }
		// -1の時どうする？
    _cluster[k].incCnt();

    /*数値項目*/
    for(int i=0; i<sample.fCnt(); i++){
      double v = sample.vGet(smp,i);
      _cluster[k].acum(i , v);
    }
	}
}
//----------------------------------------------------------------------------
// 各クラスタの重心を計算し、重心が変われば1変わらなければ0を返す             
//----------------------------------------------------------------------------
int Clusters::movCenter(){
  int mov=0; /*重心が動いたフラグ*/
  double newCen;
  int i,k;

  for(k=0; k<_cluster.size(); k++){
    /*数値項目*/
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

/*----------------------------------------------------------------------------*/
/* cluster と n 番名のサンプルとの距離                                 */
/*----------------------------------------------------------------------------*/
double Clusters::calDistanceClsSmp(int k, int n,  Sample & sample, DataInfo &dinfo){

	vector<double> disFld(_fCnt);
  int j=0;
  // 数値項目
  for(int i=0; i<_fCnt; i++){
  	if(_cluster[k].gExist(i) || dinfo.Rng(i) ==0 ) continue;
    double a=norm(_cluster[k].gGet(i),dinfo.Min(i),dinfo.Rng(i));
    double b=norm(sample.vGet(n,i),dinfo.Min(i),dinfo.Rng(i));
    disFld[j++]=disNum(a,b);
  }
  if(j==0){ return DBL_MAX;}

  // カテゴリ項目

  // 結合距離の計算
  double sum=disAvg(disFld,j);

  return( sum );
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
void Clusters::farthest(
  int k,
  Sample & sample, 
  DataInfo &dinfo
  ){

  //Clusters cls; //新しいクラスタ
  double dis;
  double dis_max=-1;
  int i_max=1;
  int i;

  // start i_max の算出 
  for(i=0; i< sample.Cnt(); i++){

    dis=calDistanceClsSmp(k,i,sample,dinfo);

    if( dis > dis_max){
      dis_max=dis;
      i_max=i;
    }
  }

	resetCalnum(k);
  setSmp2Cluster(k,i_max,sample);
  return;
}

Clusters* Clusters::initBFbyRA(
	size_t cCnt , vector < Sample > & sample, DataInfo &dinfo,
	boost::variate_generator< boost::mt19937,boost::uniform_int<>  > * rmod
){
  Sample  smpCM;  //新しいサンプル

  int i,j,flg;
  int minFMi=0;
	double minFM;
  double tmpFM;


  cResize(cCnt,sample[0].fCnt());

	vector < Clusters > CM(sample.size());
	vector < Clusters > FM(sample.size());
		
  // 新しいサンプルの作成 
  for(int s=0; s< sample.size(); s++){
	  CM[s].initRA( cCnt, sample[s], rmod);

    // CONVERGENCE (kmean-mod)
    for(i=0;i<2;i++){
      // k-mean 
      while(1){
        CM[s].setClusterSmp(sample[s],dinfo);
        if(!CM[s].movCenter())break;
      }

      // k-mean mod
      flg=0;
      for(j=0; j< cCnt; j++){
        // 空のクラスタもしくはNULLのクラスタがあった
        if( CM[s].getCnt(j)== 0){
          CM[s].farthest(j,sample[s],dinfo);
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
  smpCM.setSize(sample.size() * cCnt ,_fCnt );
	size_t pos=0;
  for(int s=0; s < sample.size(); s++){

    for(i=0; i< cCnt; i++){
      for(j=0; j<_fCnt; j++){
	      smpCM.vSet( pos ,j,CM[s].get_cenNum(i,j) );
      }
      pos++;
    }
  }


  // FM[i]の算出
  for(int s=0;  s< sample.size(); s++){

		FM[s].resize(cCnt,_fCnt);
    //CM[i]をFM[i]の初期値とする
    for(i=0; i< cCnt; i++){
      for(j=0; j< _fCnt; j++){
      	FM[s].set_cenNum(i,j,CM[s].get_cenNum(i,j) );
      }
    }
    // CONVERGENCE
    while(1){
      FM[s].setClusterSmp(smpCM,dinfo);
			if(!FM[s].movCenter())break;	

    }
  }

  // DISTORTION(FM[i],smpCM)         
  // もっともsmpCMにfitするFM[i]を選ぶ
	minFM=DBL_MAX;
  for(int s=0; s< sample.size(); s++){
  	FM[s].setClusterSmp(smpCM,dinfo);
		tmpFM=0;
    for(i=0; i< cCnt; i++){
      //数値
      for(j=0; j<_fCnt; j++){
        tmpFM += FM[s].getAccm(i,j);
      }
    }
    if( minFM > tmpFM){
      minFM =tmpFM;
      minFMi=s;
    }
  }

  *this = FM[minFMi];

  return this;
}

// -----------------------------------------------------------------------------
// 実行
// -----------------------------------------------------------------------------
int kgKmeans::run(void) try {

	// 
	setArgs();
	// 入力データ情報の獲得 
	getDataInfo();



  switch(_dType){
  	case 0: /*---------------------------- ランダム*/
			
  		sampling(1,100);
	    _clusters.initRA( _clusterCnt, _sample[0], _rand_mod.get());

	    break;

  	case 1: /*---------------------------- Kaufman Approach*/

	    sampling(1,100);
	    _clusters.initKA( _clusterCnt, _sample[0], _dinfo);

	    break;

	  case 2: /*---------------------------- Bradley & Fayyad Approach*/
	    sampling(_mcnt,100);

    	while(1){
				if(_clusters.initBFbyRA( _clusterCnt, _sample,  _dinfo , _rand_mod.get())==NULL){
	  			_clusterCnt--;
	  		}
	  		else{
		  		break;
		  	}
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

