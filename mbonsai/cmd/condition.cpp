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
#include <algorithm>
#include "condition.h"

using namespace kglib;


//----------------------------------------------------
//	エントロピー値計算
//----------------------------------------------------
void Condition::calentropy(void)
{
	double mEnt=0.0;
	double uEnt=0.0;
	double tEnt=0.0;

	for(size_t i=0 ; i<_clsSz ; i++){
//		cerr << "pre " << i << " | " <<  _mShr->at(i) << " " << _mShrRC[i] << " " <<_uShr->at(i) << " " << _uShrRC[i] << endl;
//		cerr << "calent pre " <<  _mShr->at(i) << " " << _uShr->at(i) << " " << _tShr->at(i) << endl;

		mEnt += _mShr->at(i) * Condition::log2(_mShr->at(i)) * -1.0;
		uEnt += _uShr->at(i) * Condition::log2(_uShr->at(i)) * -1.0;
		tEnt += _tShr->at(i) * Condition::log2(_tShr->at(i)) * -1.0;
	}
//	cerr << "calent " <<  _mtShr << " " << mEnt << " " <<  _utShr << " " << uEnt << endl;
	_splitBefor = tEnt;
	_splitAfter = _mtShr * mEnt + _utShr * uEnt;
	_splitgain	= _splitBefor - _splitAfter ;
}

//----------------------------------------------------
//	conditonの優劣の基準になる値の計算
//	現状はエントロピー値を使用。
//----------------------------------------------------
void Condition::calCndCntSplit(void)
{
	calentropy(); 
}

//----------------------------------------------------
//	condition値計算
// トータル、シェアの計算
//----------------------------------------------------
void Condition::calCndCnt(void)
{
	double uCost=0;
	double mCost=0;
	double tCost=0;
	double total=0;
	double mTotal=0;
//	cerr << "cal 0-0 " << this << endl;
//	cerr << "cal 0-1 " << &_tCnt[0] << endl;
//	cerr << "cal 0-2 "  << _clsSz << endl;
//	cerr << "cal 0-3 " <<  _mCnt.size() << " " << _uCnt.size() << " " << _tCnt.size()<< endl;	

	for(int i=0 ; i<_clsSz ; i++){
//	cerr << "cal 1 " << i << " " << _uCnt[i] << endl;	
//	cerr << "cal 1 "  << i << " "<< uCnt().at(i) << endl;	
	}

	for(size_t i=0 ; i<_clsSz ; i++){
		_tCnt[i] = _mCnt[i] + _uCnt[i];
		total  += _tCnt[i];
		mTotal += _mCnt[i];
	}
	_mRatio = mTotal/total;
		
	for(size_t i=0 ; i<_clsSz ; i++){
		if(_tCnt[i]!=0){
			_mmCnt[i] = (_mCnt[i]/_tCnt[i]) * _nCnt[i];
			_uuCnt[i] = (_uCnt[i]/_tCnt[i]) * _nCnt[i];
		}
		else{
			_mmCnt[i]=0;
			_uuCnt[i]=0;
		}
	}

	for(size_t i=0 ; i<_clsSz ; i++){
		_mmCnt[i]+=_mCnt[i];
		_uuCnt[i]+=_uCnt[i];
	}

	_mtCnt =_utCnt = _total = 0;
//	cerr << "check cls " << _clsSz << " " << _mCnt.size() << " " << _uCnt.size() << " " << _nCnt.size() << endl;
	for(size_t i=0 ; i<_clsSz ; i++){
//		cerr << "calcond 1 " << i << " " << _mCnt[i]  << endl;
//		cerr << "calcond 2 " << i << " " << _uCnt[i]  << endl;
//		cerr << "calcond 3 " << i << " " << _nCnt[i] << endl;
		_tCnt[i] = _mCnt[i] + _uCnt[i] + _nCnt[i];
		_mtCnt += _mmCnt[i];
		_utCnt += _uuCnt[i];
		_total += _tCnt[i];
	}
//	cerr << "total calsond " << _total << endl;
	_mtShr = Condition::divDef( _mtCnt, _total, 0.0);
	_utShr = Condition::divDef( _utCnt, _total, 0.0);
	for(size_t i=0 ; i<_clsSz ; i++){
		_uShrRN[i] = Condition::divDef( _uuCnt[i], _utCnt ,0.0);
		_mShrRN[i] = Condition::divDef( _mmCnt[i], _mtCnt ,0.0);
		_tShrRN[i] = Condition::divDef( _tCnt[i] , _total ,0.0);
			
	}

	// コストによるシェアの計算                   
	// xShrC=C(j)*xShr(j)/SUMi(C(i)*xShr(i))  
	// C(j)=SUMi(cost(j,i))                   
	//cost(j,i)とはクラスjをiと予測したときのコスト
	// ---------------------------------------------
	// SUMi(C(i)*xShr(i))の計算
	for(size_t i=0 ; i<_clsSz ; i++){
		uCost+= _cls->getTTLCost(i) * _uShrRN[i];
		mCost+= _cls->getTTLCost(i) * _mShrRN[i];
		tCost+= _cls->getTTLCost(i) * _tShrRN[i];
	}
	// C(j)*xShr(j)/SUMi(C(i)*xShr(i)) の計算
	for(size_t i=0 ; i<_clsSz ; i++){
//		cerr << "share " << i << " | " <<  _cls->getTTLCost(i) << " "  << _uShrRN[i] << " " << uCost << " " <<  _mShrRN[i] << " " << mCost << endl;
		_uShrRC[i] = Condition::divDef(_cls->getTTLCost(i) * _uShrRN[i], uCost , _uShrRN[i]);
		_mShrRC[i] = Condition::divDef(_cls->getTTLCost(i) * _mShrRN[i], mCost , _mShrRN[i]);
		_tShrRC[i] = Condition::divDef(_cls->getTTLCost(i) * _tShrRN[i], tCost , _tShrRN[i]);
	}
}

void Condition::reset(void)
{
	for(size_t i=0 ; i<_clsSz ; i++){
		_mCnt[i]=0;	
		_uCnt[i]=0;
		_nCnt[i]=0;
		_tCnt[i]=0;
		_tShrRN[i]=0;
		_tShrRC[i]=0;
		_mmCnt[i]=0;
		_mShrRN[i]=0;
		_mShrRC[i]=0;
		_uuCnt[i]=0;
		_uShrRN[i]=0;
		_uShrRC[i]=0;
	}
	_mtCnt  = 0;
	_utCnt  = 0;
	_tShr   = &_tShrRC;
	_total  = 0;
	_mShr   = &_mShrRC;	
	_uShr   = &_uShrRC;
	_mCount = 0;
	_uCount = 0;
	_nCount = 0;
	_mRatio = 0;
	_mtShr  = 0;
	_utShr  = 0;
	_splitBefor = 0;
	_splitAfter = DBL_MAX;
	_splitgain  = DBL_MAX;
}
//----------------------------------------------------
// conditionからconditonの優劣の上限を求める
// パターン項目でかつクラスサイズが2のときのみ意味がある
//----------------------------------------------------
double Condition::calUpperBound(void)
{
	if(_clsSz!=2){ return 0; }
	double x = _mmCnt[0];
	double y = _mmCnt[1];
	double xMax = _mmCnt[0]+_uuCnt[0];
	double yMax = _mmCnt[1]+_uuCnt[1];

	// f(0,y)
	_mmCnt[0] = 0;
	_uuCnt[0] = xMax -_mmCnt[0];
	_mmCnt[1] = y;
	_uuCnt[1] = yMax - _mmCnt[1];
	calCndCnt();
	calCndCntSplit();
	double svMin = _splitAfter;

	// f(x,0)
	_mmCnt[0] = x;
	_uuCnt[0] = xMax - _mmCnt[0];
	_mmCnt[1] = 0;
	_uuCnt[1] = yMax - _mmCnt[1];
	calCndCnt();
	calCndCntSplit();
	double sv = _splitAfter;
	if( svMin>sv){ svMin = sv;}

	// f(0,0)
	_mmCnt[0] = 0;
	_uuCnt[0] = xMax - _mmCnt[0];
	_mmCnt[1] = 0;
	_uuCnt[1] = yMax - _mmCnt[1];
	calCndCnt();
	calCndCntSplit();
	sv = _splitAfter;
	if( svMin>sv ){ svMin = sv; }

	// f(x,y)
	_mmCnt[0] = x;
	_uuCnt[0] = xMax - _mmCnt[0];
	_mmCnt[1] = y;
	_uuCnt[1] = yMax - _mmCnt[1];
	calCndCnt();
	calCndCntSplit();
	sv = _splitAfter;
	if( svMin>sv ){ svMin = sv;}

	return svMin;
}

// マッチ件数が０だとtrue;
bool Condition::calObjValCnt(RegPtn* regptn,vector<ValtypeVec>& idxdat)
{
	for(size_t i=0 ; i<idxdat.size() ; i++){
		int clsno = _cls->getVal(i);
		if( regptn->isMatch(idxdat[i]) ){
			_mCnt[clsno]+=1;
		}
		else{
			_uCnt[clsno]+=1; 
		}
	}
	// 件数集計。エントロピーの計算
	calCndCnt();
	calCndCntSplit();
	if(_mtCnt==0){ return true;}
	return false;
}

//----------------------------------------------------
// conditionセットメソッド
//	__mCnt,__uCnt,__nCnt,			<=null比率込みの件数値(match,unmatch,null)
//__mCount,__nCount,__nCount	<=件数(match,unmatch,null)
// をセットする。
//	cond.__calCndCnt()
//	cond.__calCndCntSplit()
//	で成績のいいものを見つけて、self.__basevalにbase値をセットする
//----------------------------------------------------
// conditionセットメソッド（nummber用）
//----------------------------------------------------
struct numInfo{
	double val;
	int clsNo;
	double ratio;
	
	numInfo(double v,int c,double r):val(v),clsNo(c),ratio(r){}
};
bool operator<(const numInfo& left, const numInfo& right)
{
  return left.val < right.val ;
}


void Condition::setCondition(Pointer& pointer , NumericItem* dCls)
{
	vector<numInfo> numlist;		
	Condition cond(_cls); 
	for(int i=0 ; i<pointer.size() ; i++){
		int recNo = pointer.atLnum(i);
		int	clsNo = _cls->getVal(recNo);
		if( dCls->null(recNo) ){
			cond.add_nCnt(clsNo,pointer.atLratio(i));
			cond.add_mCount(1);
			cond.add_uCount(1);
			cond.add_nCount(1);
		}
		else{
			numInfo numinfo(dCls->getVal(recNo) ,clsNo, pointer.atLratio(i) );
			numlist.push_back( numinfo );
			cond.add_uCnt(clsNo,pointer.atLratio(i));
			cond.add_uCount(1);
		}
	}

	sort(numlist.begin(),numlist.end());
	double bestTh=0;
	double th=0;
	double bestSv=DBL_MAX;

	for(int i=0 ; i<numlist.size() ; i++){
		th = numlist[i].val;
		cond.add_uCnt(numlist[i].clsNo,numlist[i].ratio*-1);
		cond.add_mCnt(numlist[i].clsNo,numlist[i].ratio);
		cond.add_mCount(1);
		cond.add_uCount(-1);
		if( i!=(numlist.size()-1) && th == numlist[i+1].val){ continue; }
		cond.calCndCnt();
		cond.calCndCntSplit();
		if( cond.splitAfter() < bestSv){
			copy(cond);
			bestSv = cond.splitAfter();
			if( i!=(numlist.size()-1)){ bestTh=(th+numlist[i+1].val)/2; }
			else											{  bestTh=th; }
		}
	}
	_baseval.set(bestTh); 
	return;
}

void Condition::tcnt_set(map<string,int>& cntset)
{
	for(map<string,int>::iterator it=cntset.begin();it!=cntset.end();it++){
		int x = _cls->item2id(const_cast<char *>((*it).first.c_str())) ;
		_tCnt[x] = (*it).second;
	}
}


void Condition::setCondition(Pointer& pointer , CategoryItem* dCls)
{
	int bestCombi=0;
	double bestSv=DBL_MAX;

	vector<char> bestCombiNull;
	vector<char> catList(dCls->itmSz(),'0');


	vector<double> nullcnt( _cls->itmSz(),0);
	vector<double> valClsCntL(_cls->itmSz(),0) ;
	vector< vector<double> > valClsCnt(dCls->itmSz(),valClsCntL);
	vector<int> valRecCnt(dCls->itmSz(),0) ;

	int nulrec=0;
	for(int i=0 ; i<pointer.size() ; i++){
		int recNo = pointer.atLnum(i);
		int	clsNo = _cls->getVal(recNo);
		if( dCls->null(recNo) ){
			nullcnt[clsNo] += pointer.atLratio(i);			
			nulrec++; 
		}
		else{
			valClsCnt[dCls->getVal(recNo)][clsNo] += pointer.atLratio(i);
			valRecCnt[dCls->getVal(recNo)]++;	
		}
	}
	//カテゴリが１０以上かどうか処理を変える
	if(dCls->itmSz()<10){
		for(int i=0 ; i < CategoryItem::end[dCls->itmSz()-1]-1 ; i=i+2){
			Condition cond(_cls); 
			vector<char> combiNull(dCls->itmSz()+1,'0');
						
			cond.nullDset(nulrec,nullcnt);
	
			for(int j=0 ; j<dCls->itmSz() ; j++ ){
				double total = 0;
				if( dCls->catcombi[i][j] =='M')	{ cond.add_mCount(valRecCnt[j]); }
				else 														{ cond.add_uCount(valRecCnt[j]); }
				for(int k=0 ; k<_cls->itmSz() ; k++ ){
					if( dCls->catcombi[i][j] =='M') { cond.add_mCnt(k,valClsCnt[j][k]); }
					else														{ cond.add_uCnt(k,valClsCnt[j][k]); }
					total+=valClsCnt[j][k];
				}
				if(total==0){ combiNull[j]='N' ;}
			}

			cond.calCndCnt();
			cond.calCndCntSplit();
//			cerr << "stc32-0 " << bestSv << endl;
//			cerr << "stc32 " << bestSv << " " << ( cond.splitAfter() < bestSv ) << endl;
			if(cond.splitAfter() < bestSv){
//				cerr << "stc33 "  << cond.splitAfter() << " " << bestSv << endl;
				copy(cond);
//				cerr << "stc34" << endl;
				bestSv=cond.splitAfter();
				bestCombi=i;
				bestCombiNull=combiNull;
			}
		}
		for(int j=0 ; j<dCls->itmSz() ; j++ ){
			if( bestCombiNull[j] == 'N'){ catList[j]='N';}
			else												{ catList[j]=dCls->catcombi[bestCombi][j]; }
		}
	}
	else{
		Condition cond(_cls); 
		cond.nullDset(nulrec,nullcnt);
		for(int i=0 ; i<catList.size() ; i++){ catList[i] = 'M'; }
		for(int j=0 ; j<dCls->itmSz() ; j++ ){
			for(int k=0 ; k<_cls->itmSz() ; k++ ){
				cond.add_mCnt(k,valClsCnt[j][k]);
			}
			cond.add_mCount(valRecCnt[j]);
		}
		cond.calCndCnt();
		cond.calCndCntSplit();
		copy(cond);
		bestSv=cond.splitAfter();
		bool improve = true;
		int moveCat=-1;
		while(improve){
			improve=false;
			for(int j=0 ; j<dCls->itmSz() ; j++ ){
				if(catList[j] == 'M'){
					// Match->unmatchへ
					for(int k=0 ; k<_cls->itmSz() ; k++ ){
						cond.add_mCnt(k,valClsCnt[j][k]*-1);
						cond.add_uCnt(k,valClsCnt[j][k]);
					}
					cond.add_mCount(valRecCnt[j]*-1);						
					cond.add_uCount(valRecCnt[j]);
					cond.calCndCnt();
					cond.calCndCntSplit();
					if(cond.splitAfter() < bestSv){
						bestSv=cond.splitAfter();
						moveCat=j;
						improve=true;
					}
					// 元に戻す		
					for(int k=0 ; k<_cls->itmSz() ; k++ ){
						cond.add_mCnt(k,valClsCnt[j][k]);
						cond.add_uCnt(k,valClsCnt[j][k]*-1);
					}
					cond.add_mCount(valRecCnt[j]*-1);
					cond.add_uCount(valRecCnt[j]);
				}
			}
			if(improve){
				catList[moveCat] = 'U';
				for(int k=0 ; k<_cls->itmSz() ; k++ ){
					cond.add_mCnt(k,valClsCnt[moveCat][k]);
					cond.add_uCnt(k,valClsCnt[moveCat][k]*-1);
				}
				cond.add_mCount(valRecCnt[moveCat]*-1);
				cond.add_uCount(valRecCnt[moveCat]);
			}
		}
		cond.calCndCnt();
		cond.calCndCntSplit()	;
		copy(cond);
	}
	_baseval.set(catList); 
	return;
}

void Condition::setCondition(Pointer& pointer , PatternItem* dCls)
{
	double bestSv=DBL_MAX;
	int	regNo = -1;
	vector< vector<bool> > &  att = dCls->att();
	for(int i=0 ;i<att.size() ;i++){
		Condition cond(_cls); 
		for(int j=0 ; j<pointer.size() ; j++){
			int recNo = pointer.atLnum(j);
			int	clsNo = _cls->getVal(recNo);
			if( dCls->null(recNo) ){
				cond.add_nCnt(clsNo,pointer.atLratio(j));
				cond.add_mCount(1);
				cond.add_uCount(1);
				cond.add_nCount(1);
			}
			else{
				if( att[i][recNo] ){
					cond.add_mCnt(clsNo,pointer.atLratio(j));
					cond.add_mCount(1);
				}
				else{
					cond.add_uCnt(clsNo,pointer.atLratio(j));
					cond.add_uCount(1);
				}
			}
		}
		cond.calCndCnt();
		cond.calCndCntSplit();
		if( cond.splitAfter() < bestSv){
			copy(cond);
			bestSv = cond.splitAfter();
			regNo = i;
		}
	}
	_baseval.set(dCls->regInfo(regNo)); 
	return;	
}


void Condition::copy(Condition& cond)
{
	_mCnt  = cond.mCnt();
	_mtCnt = cond.mtCnt();
	_uCnt  = cond.uCnt();
	_utCnt = cond.utCnt();
	_nCnt = cond.nCnt();

	_tCnt = cond.tCnt();

	_tShrRN=cond.tShrRN(); 
	_tShrRC=cond.tShrRC(); 
	_tShr = &_tShrRC;

	_total=cond.total();

	_mmCnt = cond.mmCnt(); 
	_mShrRN = cond.mShrRN();
	_mShrRC = cond.mShrRC();
	_mShr =&_mShrRC;

	_uuCnt = cond.uuCnt(); 
	_uShrRN = cond.uShrRN(); 
	_uShrRC = cond.uShrRC(); 
	_uShr = &_uShrRC;
	_mCount= cond.mCount(); 
	_uCount= cond.uCount(); 
	_nCount= cond.nCount(); 

	_mRatio=cond.mRatio(); 
	_mtShr=cond.mtShr(); 
	_utShr=cond.utShr(); 
	_splitBefor=cond.splitBefor(); 
	_splitAfter=cond.splitAfter(); 
	_splitgain=cond.splitgain(); 
	_baseval=cond.baseVal(); 

}

int Condition::maxClassNo(void){
	int maxno = 0;
	double maxval = _tShr->at(0);
	for(int i=1 ; i<_clsSz ; i++){
		if( maxval<_tShr->at(i)){
			maxval = _tShr->at(i);
			maxno = i;
		}
	}
	return maxno;
}

void Condition::nullDset(int nrec,vector<double> &nullD){
	for(int j=0 ; j<_cls->itmSz() ; j++ ){
		_nCnt[j] = nullD[j];
	}
	_mCount=nrec;
	_uCount=nrec;	
	_nCount=nrec;
}




