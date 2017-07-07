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
#include <kgmod.h>
#include <vector>

using namespace std;

namespace kglib { ////////////////////////////////////////////// start namespace

class ptnParam
{
	int _indexSize;
	bool   _subseq;
	bool   _order;
	int    _begin;
	int    _end;
	char	 _delim;
	int _indexLen;
	
	public:

	ptnParam(void):_indexSize(2),_subseq(false),_order(false),_begin(0),_end(0), _delim('\0'),_indexLen(5){}

	ptnParam(int iS,bool sseq,bool od,int bn,int en,char dl,int len):
		_indexSize(iS),_subseq(sseq),_order(od),_begin(bn),_end(en),_delim(dl),_indexLen(len){}

	//アクセッサ
	size_t  indexSize(void)const { return _indexSize;}
	bool    isSubSeq(void) const { return _subseq;}
	bool    isOrder(void)  const { return _order;}
	int     begin(void)    const { return _begin;}
	int     end(void)      const { return _end;}
	char 		delim(void)		 const { return _delim;}
	int     indexLen(void) const { return _indexLen;}

	void setIndexSize(int size){_indexSize=size;}
};


	


class ptnParams 
{
	vector<ptnParam> _param;

	public:
	ptnParams(void){}
	ptnParams(vector<vector<kgstr_t> > param,char delim)
	{
		for(size_t i=0; i<param.at(0).size(); i++)
		{
			// デフォルト値の設定
			int indexSize   = 0;
			bool   subsequence = false;
			bool   orderedItem = false;
			int    begin       = 0;
			int    end         = 0;
			int    indexLen    = 5;
			// 指定値
			kgstr_t vis=param.at(1).at(i); // インデックスサイズ
			kgstr_t vss=param.at(2).at(i); // subsequence
			kgstr_t voi=param.at(3).at(i); // orderedItem
			kgstr_t vbg=param.at(4).at(i); // begin
			kgstr_t ved=param.at(5).at(i); // end
			kgstr_t vil=param.at(6).at(i); // index長さ

			if(!vis.empty()){
				int n=atoi(vis.c_str());
				if(n>1) indexSize=n;
			}

			if(!vss.empty()){
				if(vss=="true") subsequence=true;
			}

			if(!voi.empty()){
				if(voi=="true") orderedItem=true;
			}

			if(!vbg.empty()){
				size_t n=atoi(vbg.c_str());
				if(n>0) begin=n;
			}

			if(!ved.empty()){
				size_t n=atoi(ved.c_str());
				if(n>0) end=n;
			}
			if(!vil.empty()){
				size_t n=atoi(vil.c_str());
				if(n>0) indexLen=n;
			}
			_param.push_back(
				ptnParam(indexSize,subsequence,orderedItem,begin,end,delim,indexLen)
			);
		}
	}

	size_t size(void) 					const { return _param.size();}
	ptnParam at(size_t no) 			const { return _param.at(no);}
	size_t  indexSize(size_t no)const { return _param.at(no).indexSize();}
	bool    isSubSeq(size_t no) const { return _param.at(no).isSubSeq();}
	bool    isOrder(size_t no)  const { return _param.at(no).isOrder();}
	int     begin(size_t no)    const { return _param.at(no).begin();}
	int     end(size_t no)      const { return _param.at(no).end();}
	char    delim(size_t no)    const { return _param.at(no).delim();}
	int     indexLen(size_t no) const { return _param.at(no).indexLen();}

};

}
