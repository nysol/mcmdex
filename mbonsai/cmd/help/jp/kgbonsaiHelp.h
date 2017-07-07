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
/// kgbonsaiHelp.h : kgbonsai help
// =============================================================================
_title="系列データによる決定木生成";
_doc="\
mbonsai 系列データによる決定木生成\n\
==================================\n\
本コマンドは、系列パターンを説明変数として利用可能な決定木モデル構築コマンドである。\n\
系列データの解析は様々な応用分野において適用できる。\n\
顧客が購入したブランドの順序，スーパーや百貨店における売り場の巡回パターン，\n\
傷病の発症順序,これらは全て系列データとして解析できる。\n\
本コマンドのオリジナルのアイデアは、九州大学の研究チームによって開発されたBONSAIにあり、\n\
分子生物学におけるアミノ酸配列を解析することを目的に構築された手法である。\n\
この手法をビジネスデータに応用する中で、いくつかの改良を加えたものが本コマンドで、\n\
以下のような特徴をもつ。\n\
\n\
-   カテゴリ系列データから有用なパターンを抽出し、その有無を決定木における節点の条件とすることができる。\n\
\n\
-   モデル精度を高めるように、系列を構成するアイテムをグルーピングすることができる。\n\
\n\
-   複数の系列データを扱うことができる。\n\
\n\
-   系列データ以外にも数値変数、カテゴリ変数も含めてモデル化ができる。\n\
\n\
-   決定木モデルの構築モードと未知データを与えての予測モードがある。\n\
\n\
-   3つ以上のクラス分類問題にも対応している。\n\
\n\
まず直感的理解を得るために，以下に例を示す。\n\
ある小売店における顧客別のブランドa,b,cの購入系列と、\n\
その店からの離反の有無についてのデータが表に示されている。\n\
ブランドの購入順序が顧客の離反に関係しているとの仮説のもと、\n\
離反の有無を目的変数に、そしてブランドの購買順序に関する部分的なパターン(ここでのパターンとは部分文字列、例えば\n\
、“ab”,“cbb”を考えればよい。パターンの定義の詳細は次節を参照のこと。)を説明変数にして決定木モデルを構築する。\n\
このような説明変数としてのパターンは候補パターンと呼ばれ、モデルの精度に寄与するであろうパターンが事前に生成され(その方法も次節を参照のこと)、\n\
それらのパターンを0-1変数としてデータセットが作成される(表)。\n\
このデータセットから一般的な方法で決定木が構築される。\n\
実際に本コマンドを用いて作成された決定木が図に示されている。\n\
\n\
ll\n\
\n\
  ブランド系列    離反\n\
\n\
  bcaba           yes\n\
  bcabcaa         yes\n\
  aaabac          yes\n\
  caa             yes\n\
  cca              no\n\
  cacbc            no\n\
  bcc              no\n\
  acca             no\n\
\n\
  : \n\
  表のブランド系列項目から抜き出された部分的な購入パターンを説明変数とし(候補パターンと呼ぶ)、\n\
  全サンプルに対してパターンを含むかどうかの0-1データに変換されたデータセットの例。\n\
  一行目の顧客は部分パターン“a”,“b”,“c”,“ab”を含んでいるが、“aa”,“cc”は含んでいない。\n\
\n\
   a   b   c   aa   ab   cc   …  離反\n\
  --- --- --- ---- ---- ---- --- ------\n\
   1   1   1   0    1    0       yes\n\
   1   1   1   1    1    0       yes\n\
   1   1   1   1    1    0       yes\n\
   1   0   1   1    0    0    …  yes\n\
   1   0   1   0    0    1       no\n\
   1   1   1   0    0    0       no\n\
   0   1   1   0    0    1       no\n\
   1   0   1   0    0    1       no\n\
\n\
  : \n\
  表のブランド系列項目から抜き出された部分的な購入パターンを説明変数とし(候補パターンと呼ぶ)、\n\
  全サンプルに対してパターンを含むかどうかの0-1データに変換されたデータセットの例。\n\
  一行目の顧客は部分パターン“a”,“b”,“c”,“ab”を含んでいるが、“aa”,“cc”は含んでいない。\n\
\n\
ll\n\
\n\
[image]\n\
\n\
[image]\n\
\n\
さらにBONSAIの特筆すべき特徴として、パターンを構成する要素(アルファベットと呼ぶ)を自動的にグルーピングする機能がある。\n\
ここで、各アルファベットに対するグループをインデックスと呼ぶ。\n\
例えば、a,b,cの３つのブランドを2つのインデックスにグルーピングする仕方は3通りあるが((aとbc、bとac、cとab)、\n\
それぞれのグルーピングに応じてオリジナルの系列のアルファベットを対応するインデックスで置換し、上述の手順で決定木を3つ構築し、分類精度の最もよい決定木を選択する。\n\
このようにして構築された決定木が図に示されている。\n\
得られたグルーピングは、分類モデルの精度を高めるようなグルーピングになっているため、\n\
そこから有用な知見が得られることが期待できる。\n\
\n\
書式1:モデル構築モード\n\
----------------------\n\
    mbonsai i= [p=] [n=] [d=] c= O= [delim=] [cost=] [seed=] [cand=] [iter=] [cv=|ts=]\n\
            [leafSize=] [--help]\n\
\n\
  i=          : 訓練データファイル名\n\
  p=          : パターン項目名(複数指定可)\n\
              : 項目名の後にコロンで区切って5つのパラメータを指定できる。\n\
              : p=項目名:is:seq:ordered:head:tail:rs\n\
              : is: インデックスのサイズ\n\
              :    指定を省略すれば、インデックスを生成せず、オリジナルのアルファベットのパターンを用いる。\n\
              : seq: パターンの種類\n\
              :   true:部分系列パターン\n\
              :   false:部分文字列パターン(default)\n\
              : ordered: インデックスの生成において、アルファベットに順序があると仮定するかどうか。\n\
              :     (isの指定を省略していれば無視される)\n\
              :   true: 順序あり, アルファベットをある閾値以上/未満でグルーピングする。\n\
              :   false: 順序なし(default)\n\
              : head: 先頭一致の先頭からの文字数(デフォルトは先頭一致を考慮しない)\n\
              : tail: 末尾一致の先頭からの文字数(デフォルトは末尾一致を考慮しない)\n\
              : rs: 正規パターンのサイズ上限(デフォルトは5)\n\
  n=          : 数値項目名(複数指定可)\n\
  d=          : カテゴリ項目名(複数指定可)\n\
  c=          : クラス項目名\n\
  O=          : 出力ディレクトリ名(text,PMMLによるモデル,モデル統計など)\n\
  delim=      : パターンの区切り文字(デフォルト:空文字,すなわち1バイト文字を1つのアルファベットと見なす)\n\
  cost=       : コストファイル名\n\
  seed=       : 乱数の種(default=-1:時間依存)\n\
  cand=       : 説明変数としてのパターン数(default=30,範囲:1〜256)\n\
  iter=       : ローカルサーチ回数(default=1)\n\
  leafSize=   : 一つのリーフに含まれるサンプル数の下限(default:制限なし)\n\
  alpha=      : 枝刈り度を指定する。省略時は0.01が設定される。ただし、ts=もしくはcv=を指定した場合、\n\
              : このパラメータは無効化される。\n\
  ts=         : テストサンプル法によるテストデータの割合を指定する。値を省略して“ts=”と指定すると0.333が用いられる。\n\
  cv=         : 交差検証法によるデータの分割数を指定する。値を省略して“cv=”と指定すると10が用いられる。\n\
              : ts=,cv=のいずれも指定されていない場合は、alpha=0.01と指定されたと見なされる。\n\
              : alpha=,ts=,cv=を指定したとしても、PMMLに最大木と枝刈り度が記録されるので、\n\
              : 予測モードにおいて、alphaの値を変更して予測することが可能である。\n\
  --help      : ヘルプの表示\n\
\n\
書式2:予測モード\n\
----------------\n\
           mbonsai -predict i= I= o= [alpha=] [--help]\n\
\n\
  -predict   : 予測モードで動作させる。予測モードでは必須\n\
  i=         : 入力データ【必須】\n\
             : 項目名は、モデル構築モードで利用した項目と同じでなければならない。\n\
  I=         : モデル構築モードでの出力先ディレクトリパス【必須】\n\
             : 利用するファイルは以下のとおり。\n\
             : bonsai.pmml: pmmlによる決定木モデル\n\
  o=         : 予測結果ファイル名\n\
             : 入力データに“predict”という項目を追加して出力する。\n\
             : 項目名は、モデル構築モードで利用した項目と同じでなければならない。\n\
  alpha=     : 枝刈りの複雑度パラメータ。\n\
             : 0以上の実数値を与える以外に、以下の2つは特殊な意味を持つシンボルとして指定できる。\n\
             : min: 推定誤分類率最小の枝刈りモデルに対応する\\alphaの値。\n\
             : 1se: 同じく1SEルールの枝刈りモデルに対応する\\alphaの値。\n\
             : これら2つのシンボルによる指定は、モデル構築時にts=もしくはcv=を指定した時のみ有効である。\n\
             : 省略時の動作:\n\
             : モデル構築時にts=もしくはcv=を指定した場合は、minが用いられる。\n\
             : モデル構築時にalpha=を指定した場合は、その値が用いられる。\n\
  delim=     : パターンの区切り文字(デフォルト:空文字,すなわち1バイト文字を1つのアルファベットと見なす)\n\
  --help     : ヘルプの表示\n\
\n\
利用例\n\
------\n\
例1 モデル構築の例\n\
\n\
ここでは、ts=,cv=,alpha=のいずれも指定されていないので、alpha=0.01で枝刈りされた結果がmodel.txtに出力される。\n\
\n\
    $ more input.csv\n\
    ブランド系列,離反\n\
    bcaba,yes\n\
    bcabcaa,yes\n\
    aaabac,yes\n\
    caa,yes\n\
    cca,no\n\
    cacbc,no\n\
    bcc,no\n\
    acca,no\n\
\n\
    $ mbonsai p=ブランド系列:2 c=離反 i=input.csv O=result1\n\
\n\
    $ more result1/model.txt\n\
\n\
    [alphabet-index]\n\
    Field Name: ブランド系列\n\
    Index[1]={a}\n\
    Index[2]={b,c}\n\
\n\
    [decision tree]\n\
    if($ブランド系列 has 11)\n\
      then $離反=yes (hit/sup)=(3/3)\n\
      else if($ブランド系列 has 2212)\n\
        then $離反=yes (hit/sup)=(1/1)\n\
        else $離反=no (hit/sup)=(4/4)\n\
\n\
    numberOfLeaves=3\n\
    deepestLevel&=& 2\n\
\n\
    [Confusion Matrix by Training]\n\
    ## By count\n\
             Predicted As \\ldots\n\
        yes no  Total\n\
    yes 4   0   4\n\
    no  0   4   4\n\
    Total   4   4   8\n\
\n\
    ## By cost\n\
             Predicted As \\ldots\n\
        yes no  Total\n\
    yes 0   0   0\n\
    no  0   0   0\n\
    Total   0   0   0\n\
\n\
    ## Detailed accuracy by class\n\
    class,recall,precision,FPrate,F-measure\n\
    yes,1,1,0,1\n\
    no,1,1,0,1\n\
\n\
    ## Summary\n\
    accuracy=1\n\
    totalCost=0\n\
\n\
    $ more result1/model.pmml\n\
    <?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
    <PMML version=\"4.0\">\n\
      <Header copyright=\"KGMOD\">\n\
        <Application name=\"mclassify\" version=\"1.0\"/>\n\
        <Timestamp>2014/07/20 23:00:13</Timestamp>\n\
      </Header>\n\
      <DataDictionary numberOfFields=\"2\">\n\
        <DataField name=\"ブランド系列\" optype=\"categorical\" dataType=\"string\">\n\
          <Value value=\"b\"/>\n\
          <Value value=\"c\"/>\n\
          <Value value=\"a\"/>\n\
        </DataField>\n\
        <DataField name=\"離反\" optype=\"categorical\" dataType=\"string\">\n\
          <Value value=\"yes\"/>\n\
          <Value value=\"no\"/>\n\
        </DataField>\n\
      </DataDictionary>\n\
      <TreeModel functionName=\"classification\" splitCharacteristic=\"binarySplit\">\n\
        <MiningSchema>\n\
          <MiningField name=\"ブランド系列\">\n\
            <Extension extender=\"KGMOD\" name=\"alphabetIndex\">\n\
              <alphabetIndex alphabet=\"b\" index=\"2\"/>\n\
              <alphabetIndex alphabet=\"c\" index=\"1\"/>\n\
              <alphabetIndex alphabet=\"a\" index=\"2\"/>\n\
            </Extension>\n\
          </MiningField>\n\
          <MiningField name=\"離反\" usageType=\"predicted\"/>\n\
        </MiningSchema>\n\
        <Node id=\"0\" score=\"yes\" recordCount=\"8\">\n\
          <Extension extender=\"KGMOD\" name=\"pruning\" value=\"0.000000\"/>\n\
          <True/>\n\
          <ScoreDistribution value=\"yes\" recordCount=\"4\"/>\n\
          <ScoreDistribution value=\"no\" recordCount=\"4\"/>\n\
          <Node id=\"1\" score=\"yes\" recordCount=\"4\">\n\
            <Extension extender=\"KGMOD\" name=\"pruning\" value=\"0.000000\"/>\n\
            <Extension extender=\"KGMOD\" name=\"patternPredicate\" value=\"substring\">\n\
              <SimplePredicate field=\"ブランド系列\" operator=\"contain\">\n\
                <index seqNo=\"1\" value=\"2\"/>\n\
                <index seqNo=\"2\" value=\"2\"/>\n\
              </SimplePredicate>\n\
            </Extension>\n\
            <ScoreDistribution value=\"yes\" recordCount=\"4\"/>\n\
            <ScoreDistribution value=\"no\" recordCount=\"0\"/>\n\
          </Node>\n\
          <Node id=\"2\" score=\"no\" recordCount=\"4\">\n\
            <Extension extender=\"KGMOD\" name=\"pruning\" value=\"0.000000\"/>\n\
            <Extension extender=\"KGMOD\" name=\"patternPredicate\" value=\"substring\">\n\
              <SimplePredicate field=\"ブランド系列\" operator=\"notcontain\">\n\
                <index seqNo=\"1\" value=\"1\"/>\n\
                <index seqNo=\"2\" value=\"1\"/>\n\
              </SimplePredicate>\n\
            </Extension>\n\
            <ScoreDistribution value=\"yes\" recordCount=\"0\"/>\n\
            <ScoreDistribution value=\"no\" recordCount=\"4\"/>\n\
          </Node>\n\
        </Node>\n\
      </TreeModel>\n\
    </PMML>\n\
\n\
例2 例1のモデルから未知データを予測する\n\
\n\
構築した決定木モデルで未知データ(以下では訓練データを未知データとして使っている)を予測する例。\n\
予測結果としては、各クラスの予測確率(yes,no項目)、\n\
およびその確率が最も高いクラス名(predict項目)が出力される。\n\
\n\
    $ more unknown.csv\n\
    ブランド系列\n\
    bcaba\n\
    bcabcaa\n\
    aaabac\n\
    caa\n\
    cca\n\
    cacbc\n\
    bcc\n\
    acca\n\
\n\
    $ mbonsai -predict i=unknown.csv I=result1 o=predict.csv\n\
\n\
    $ more predict.csv\n\
    ブランド系列,predict,yes,no\n\
    bcaba,yes,1,0\n\
    bcabcaa,yes,1,0\n\
    aaabac,yes,1,0\n\
    caa,yes,1,0\n\
    cca,no,0,1\n\
    cacbc,no,0,1\n\
    bcc,no,0,1\n\
    acca,no,0,1\n\
";

