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
// =============================================================================
/// kgedistHelp.h : kgedist help
// =============================================================================
_title="編集距離計算";
_doc="\
medist k=field s=field f=field [w=field] [-norm] [i=infile] [o=outfile]\n\
k=,s=の項目順にレコードが並んでいる必要がある。\n\
k= : ここで指定された項目をシーケンスの単位とする\n\
s= : シーケンスID\n\
w= : 重み項目\n\
f= : アイテム\n\
-norm: シーケンス長で規格化(0〜1.0)する。\n\
\n\
例)\n\
\n\
dat.csvの内容\n\
aid,seq,item\n\
id1,1,a\n\
id1,2,a\n\
id1,3,b\n\
id1,3,c\n\
id2,3,a\n\
id2,4,c\n\
id2,5,a\n\
id3,3,a\n\
id3,4,a\n\
id3,5,c\n\
id4,3,x\n\
id4,4,y\n\
id4,5,z\n\
\n\
$ medist k=aid s=seq f=item i=dat.csv\n\
key1,key2,distance\n\
id1,id2,3\n\
id1,id3,1.5\n\
id1,id4,7\n\
id2,id3,2\n\
id2,id4,6\n\
id3,id4,6\n\
\n\
$ medist k=aid s=seq f=item -norm i=dat.csv\n\
key1,key2,distance\n\
id1,id2,0.4166666667\n\
id1,id3,0.2083333333\n\
id1,id4,1\n\
id2,id3,0.3333333333\n\
id2,id4,1\n\
id3,id4,1\n\
\n\
";

