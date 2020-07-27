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
_title="Computation of Longest-common subsequence";
_doc="\
\n\
Compute the longest common subsequence from a given two strings.\n\
Then output the Jaccard coefficient from the summed set of the two strings, with the common part as the product set.\n\
\n\
mlcs f=field [w=field] [-norm] [i=infile] [o=outfile]\n\
f= : Specify the two items to be compared.\n\
th= : Output a pair of values greater than the given Jaccard coefficient\n\
\n\
ex.)\n\
\n\
show dat.csv\n\
text1,text2\n\
ABCDEFR,ADCER\n\
ABCDER,ABCDER\n\
ABCDER,ZZZZZZ\n\
AAACCC,ZZZCCC\n\
\n\
Compute LCS between text1 and text2\n\
$ mlcs f=text1,text2 i=dat.csv\n\
text1,text2,comSeq,comSeqSize,diff1,diff1Size,diff2,diff2Size,JC\n\
ABCDEFR,ADCER,ADER,4,BCF,3,C,1,0.5\n\
ABCDER,ABCDER,ABCDER,6,,0,,0,1\n\
AAACCC,ZZZCCC,CCC,3,AAA,3,ZZZ,3,0.3333333333\n\
\n\
Given theresh hold. Only pairs with a jaccard coefficient of 0.5 or greater are output.\n\
$ mlcs f=text1,text2 th=0.5 i=dat.csv\n\
text1,text2,comSeq,comSeqSize,diff1,diff1Size,diff2,diff2Size,JC\n\
ABCDEFR,ADCER,ADER,4,BCF,3,C,1,0.5\n\
ABCDER,ABCDER,ABCDER,6,,0,,0,1\n\
\n\
";

