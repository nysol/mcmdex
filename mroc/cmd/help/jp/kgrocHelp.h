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
/// kgrocHelp.h : kgroc help
// =============================================================================
_title="分類モデル評価のためのROC曲線もしくはリフトチャート用データの出力";
_doc="\
mroc ac=項目名 pc=項目名 s=数値 i=ファイル名 [o=]\n\
ソート条件: pc=項目文字列昇順+s=項目数値降順\n\
ac= : 実クラス項目名(必須)\n\
pc= : 予測クラス項目名(必須)\n\
s=  : 予測クラスの確率(もしくはスコア)\n\
i=  : 入力ファイル名(必須:標準入力は利用できない)\n\
o=  : 出力ファイル名(オプション:defaultは標準出力)\n\
\n\
入力ファイルについて:\n\
  分類モデルを構築した時の訓練データの各ケース(レコード)について，\n\
  全クラスの予測スコア(確率)が示されたデータを入力とする.\n\
  mcmdが提供する分類モデル構築コマンドは全てこのようなデータを出力する.\n\
	例) クラスがa,bのケース\n\
    id,実クラス,予測クラス,確率\n\
    1,a,a,0.89\n\
    1,a,b,0.11\n\
    2,b,a,0.72\n\
    2,b,b,0.28\n\
    3,a,a,0.72\n\
    3,a,b,0.28\n\
    4,a,a,0.53\n\
    4,a,b,0.47\n\
    5,b,a,0.43\n\
    5,b,b,0.57\n\
    6,b,a,0.25\n\
    6,b,b,0.75\n\
  実クラスが項目「実クラス」で示される6つのケース(id=1~6)について予測クラス別\n\
  (aとb)の予測確率.\n\
  id=1のケースは実クラスがaであり，a,bと予測される確率がそれぞれ0.89,0.11\n\
  ということ．\n\
  注:上記の例は簡単のために，id,予測クラス順に並べているが，このコマンドを\n\
     実際に利用する時は事前に,\n\
     $ sortf f=予測クラス,確率%nr\n\
     にて予測クラス文字昇順+確率数値降順で並べ替えておく必要がある．\n\
\n\
実行例) mroc ac=実クラス pc=予測クラス i=dat.csv s=確率\n\
\n\
出力ファイルについて:\n\
  上記の入力データに対して実行例を実行すると以下の出力結果を得る.\n\
  ただし，わかりやすさのために一部を固定長で示している．\n\
  出力は，各クラスで，そのクラスをpositiveとしその他のクラスをnegativeと\n\
  した場合の分類モデルの評価指標がいくつか示される．\n\
  positiveと予測するする確率の下限値が2番目の項目に示されている．\n\
  例えば3行目の例では，クラスaをpositive，その他(ここではクラスb)をnegative\n\
  とし，positiveである(すなわちクラスaである)確率が0.72以上をpositiveと予測\n\
  した場合のFPrateは0.33でrecallは0.66（以下省略)となる.\n\
    実クラス,確率,FPrate,recall,precision,TP,posRate\n\
    a, 1.79e+308, 0   , 0   , 0   , 0, 0\n\
    a, 0.89     , 0   , 0.33, 1   , 0, 0.16\n\
    a, 0.72     , 0.33, 0.66, 0.66, 1, 0.5\n\
    a, 0.53     , 0.33, 1   , 0.75, 1, 0.66\n\
    a, 0.43     , 0.67, 1   , 0.6 , 2, 0.83\n\
    a, 0.25     , 1   , 1   , 0.5 , 3, 1\n\
    b, 1.79e+308, 0   , 0   , 0   , 0, 0\n\
    b, 0.75     , 0   , 0.33, 1   , 0, 0.16\n\
    b, 0.57     , 0   , 0.67, 1   , 0, 0.33\n\
    b, 0.47     , 0.33, 0.66, 0.66, 1, 0.5\n\
    b, 0.28     , 0.67, 1   , 0.6 , 2, 0.83\n\
    b, 0.11     , 1   , 1   , 0.5 , 3, 1\n\
  各チャートの作成は各クラス毎にx,y軸を以下の通りに設定し散布図を描く.\n\
  1) ROC曲線:x軸=FPrate,y軸=recall\n\
  2) リフトチャート:x軸=posRate,y軸=TP\n\
  3) 再現率-精度曲線:x軸=precision,y軸=recall\n\
\n\
";

