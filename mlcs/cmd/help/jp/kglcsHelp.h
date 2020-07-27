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
_titleL="最長共通部分列の計算";
_docL="\
\n\
与えられた2つの文字列から最長共通部分列を計算し、共通部分を積集合として、2つの文字列の和集合から\n\
計算されたJaccard係数を出力する。\n\
\n\
mlcs f=field [w=field] [-norm] [i=infile] [o=outfile]\n\
f= : 比較したい2つの項目を指定する\n\
th= : 与えれたJaccard係数以上のペアを出力する\n\
\n\
例)\n\
\n\
dat.csvの内容\n\
text1,text2\n\
ABCDEFR,ADCER\n\
ABCDER,ABCDER\n\
ABCDER,ZZZZZZ\n\
AAACCC,ZZZCCC\n\
\n\
text1とtext2の最長共通部分列を計算する。\n\
$ mlcs f=text1,text2 i=dat.csv\n\
text1,text2,comSeq,comSeqSize,diff1,diff1Size,diff2,diff2Size,JC\n\
ABCDEFR,ADCER,ADER,4,BCF,3,C,1,0.5\n\
ABCDER,ABCDER,ABCDER,6,,0,,0,1\n\
AAACCC,ZZZCCC,CCC,3,AAA,3,ZZZ,3,0.3333333333\n\
\n\
閾値(th=)を指定した場合。jaccard係数で0.5以上のペアだけが出力される。\n\
$ mlcs f=text1,text2 th=0.5 i=dat.csv\n\
text1,text2,comSeq,comSeqSize,diff1,diff1Size,diff2,diff2Size,JC\n\
ABCDEFR,ADCER,ADER,4,BCF,3,C,1,0.5\n\
ABCDER,ABCDER,ABCDER,6,,0,,0,1\n\
\n\
";

