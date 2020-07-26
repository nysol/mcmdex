

if [ -z "$preCMD" ] ; then
	preCMD=time
fi

takemcmd=/Users/nain/work/git/mcmdex/takecmd/


p0=/Users/nain/work/date/20190509/simset20/
p1=/Users/nain/work/date/20181213/lcm53/
p2=/Users/nain/work/date/20181213/lcm_seq21/
p3=/Users/nain/work/date/20190407/mace22/

p1=/Users/nain/soft/uno/lcm53/

# CMD
MLCM=${takemcmd}mlcm
MLCMSEQ=${takemcmd}mlcmseq
MSSPC=${takemcmd}msspc
MMEDSET=${takemcmd}mmedset
MGRHFIL=${takemcmd}mgrhfil
MMACE=${takemcmd}mmace
MSIMSET=${takemcmd}msimset

TLCM=${p1}lcm
TLCMSEQ=${p2}lcm_seq
TSSPC=${p0}sspc
TMEDSET=${p0}medset
TGRHFIL=${p0}grhfil
TMACE=${p3}mace
TSIMSET=${p0}simset

DIFFSIMPLE="-rq"

#INPUT
sDPATH1=/Users/nain/work/git/mcmdex/takecmd/check/data/

tra100=${sDPATH1}datam.tra_100
tra10=${sDPATH1}datam.tra_10
smallSAMP=${sDPATH1}test.dat
smallGSAMP=${sDPATH1}goo.dat
smallSAMP2=${sDPATH1}tests.dat
smallSAMP3=${sDPATH1}testm.dat
smallSAMP4=${sDPATH1}testme.dat
smallCONST=${sDPATH1}constraints.dat
smallSAMPT=${sDPATH1}testt.dat
smallSAMPE=${sDPATH1}teste.dat

sample0=${sDPATH1}samp0.dat
strain0=${sDPATH1}strain0.dat

# OUTPUT
rDPATH0=/Users/nain/work/git/mcmdex/takecmd/check/out/

rm -rf ${rDPATH0}
mkdir ${rDPATH0}

function lcmminiRUN() {

#${preCMD} ${MLCM} F -c ${strain0} ${sample0} 1 ${rDPATH0}outmsamp0c.dat 
#${preCMD} ${TLCM} F -c ${strain0} ${sample0} 1 ${rDPATH0}outtsamp0c.dat 

${preCMD} ${MLCM} F ${sample0} 1 ${rDPATH0}outmsampF0.dat 
${preCMD} ${TLCM} F ${sample0} 1 ${rDPATH0}outtsampF0.dat 

${preCMD} ${MLCM} M ${sample0} 1 ${rDPATH0}outmsampM0.dat 
${preCMD} ${TLCM} M ${sample0} 1 ${rDPATH0}outtsampM0.dat 

${preCMD} ${MLCM} C ${sample0} 1 ${rDPATH0}outmsampC0.dat 
${preCMD} ${TLCM} C ${sample0} 1 ${rDPATH0}outtsampC0.dat 
}

function lcmRUN() {

${preCMD} ${MLCM} F -c ${strain0} ${sample0} 1 ${rDPATH0}outmsamp0c.dat 
${preCMD} ${TLCM} F -c ${strain0} ${sample0} 1 ${rDPATH0}outtsamp0c.dat 

${preCMD} ${MLCM} F ${sample0} 1 ${rDPATH0}outmsampF0.dat 
${preCMD} ${TLCM} F ${sample0} 1 ${rDPATH0}outtsampF0.dat 

${preCMD} ${MLCM} M ${sample0} 1 ${rDPATH0}outmsampM0.dat 
${preCMD} ${TLCM} M ${sample0} 1 ${rDPATH0}outtsampM0.dat 

${preCMD} ${MLCM} C ${sample0} 1 ${rDPATH0}outmsampC0.dat 
${preCMD} ${TLCM} C ${sample0} 1 ${rDPATH0}outtsampC0.dat 

${preCMD} ${MLCM} F ${tra100} 1000 ${rDPATH0}outmlcmF.dat 
#8.42 real         8.33 user         0.06 sys
#8.99 real         8.85 user         0.07 sys
#9.04 real         8.90 user         0.07 sys

${preCMD} ${TLCM} F ${tra100} 1000 ${rDPATH0}outtlcmF.dat 
#8.71 real         8.63 user         0.06 sys
#9.20 real         9.06 user         0.07 sys
#9.14 real         9.00 user         0.07 sys

${preCMD} ${MLCM} M ${tra100} 1000 ${rDPATH0}outmlcmM.dat 
#45.07 real        44.93 user         0.09 sys
#49.17 real        48.81 user         0.15 sys
#51.26 real        50.26 user         0.36 sys
${preCMD} ${TLCM} M ${tra100} 1000 ${rDPATH0}outtlcmM.dat 
#46.60 real        46.43 user         0.10 sys
#47.86 real        47.52 user         0.15 sys
#47.85 real        47.50 user         0.15 sys


${preCMD} ${MLCM} C ${tra100} 1000 ${rDPATH0}outmlcmC.dat 
#32.79 real        31.34 user         0.20 sys
#36.79 real        35.89 user         0.17 sys
#36.27 real        35.60 user         0.24 sy
${preCMD} ${TLCM} C ${tra100} 1000 ${rDPATH0}outtlcmC.dat 
#35.35 real        34.01 user         0.19 sys
#42.23 real        40.30 user         0.51 sys
#35.05 real        34.40 user         0.22 sys


}

function lcmseqRUN() {

# テスト
${preCMD} ${MLCMSEQ} F ${smallSAMP} 1 ${rDPATH0}outmseqF.dat 
${preCMD} ${TLCMSEQ} F ${smallSAMP} 1 ${rDPATH0}outtseqF.dat 

${preCMD} ${MLCMSEQ} F -p 0.5 ${smallSAMP} 1 ${rDPATH0}outmseqFp.dat 
${preCMD} ${TLCMSEQ} F -p 0.5 ${smallSAMP} 1 ${rDPATH0}outtseqFp.dat 

${preCMD} ${MLCMSEQ} F -P 0.5 ${smallSAMP} 1 ${rDPATH0}outmseqFpl.dat 
${preCMD} ${TLCMSEQ} F -P 0.5 ${smallSAMP} 1 ${rDPATH0}outtseqFpl.dat 

${preCMD} ${MLCMSEQ} F -w data/weight.dat ${smallSAMP} 1 ${rDPATH0}outmseqFw.dat 
${preCMD} ${TLCMSEQ} F -w data/weight.dat ${smallSAMP} 1 ${rDPATH0}outtseqFw.dat 

${preCMD} ${MLCMSEQ} F -w data/weight.dat -p 0.5 ${smallSAMP} 1 ${rDPATH0}outmseqFpw.dat 
${preCMD} ${TLCMSEQ} F -w data/weight.dat -p 0.5 ${smallSAMP} 1 ${rDPATH0}outtseqFpw.dat 

${preCMD} ${MLCMSEQ} F -w data/weight.dat -P 0.5 ${smallSAMP} 1 ${rDPATH0}outmseqFplw.dat 
${preCMD} ${TLCMSEQ} F -w data/weight.dat -P 0.5 ${smallSAMP} 1 ${rDPATH0}outtseqFplw.dat 

}

function sspcRUN() {


${preCMD} ${MSSPC} R -b 10 ${tra100} 0.1 ${rDPATH0}outmsspcRb10.dat 


${preCMD} ${TSSPC} R -b 10 ${tra100} 0.1 ${rDPATH0}outtsspcRb10.dat 


${preCMD} ${MSSPC} R ${tra100} 0.1 ${rDPATH0}outmsspcR.dat 

${preCMD} ${TSSPC} R ${tra100} 0.1 ${rDPATH0}outtsspcR.dat 


${preCMD} ${MSSPC} C ${tra100} 0.1 ${rDPATH0}outmsspcC.dat 
#35.37 real        35.15 user         0.15 sys
#44.16 real        42.34 user         0.55 sys
#37.07 real        36.55 user         0.23 sys

${preCMD} ${TSSPC} C ${tra100} 0.1 ${rDPATH0}outtsspcC.dat 
#42.98 real        42.67 user         0.17 sys
#45.05 real        44.09 user         0.39 sys
#44.70 real        44.10 user         0.26 sys

${preCMD} ${MSSPC} C -2 ${smallSAMP} ${tra100} 0.1 ${rDPATH0}outmsspcC2.dat 
#${preCMD} ${TSSPC} C -2 ${smallSAMP} ${tra100} 0.1 ${rDPATH0}outtsspcC2.dat 

${preCMD} ${MSSPC} Rt -k 3 ${smallSAMP} 0.1 ${rDPATH0}outmsspck3.dat 
${preCMD} ${TSSPC} Rt -k 3 ${smallSAMP} 0.1 ${rDPATH0}outtsspck3.dat 

${preCMD} ${MSSPC} Rt -K 3 ${smallSAMP} 0.1 ${rDPATH0}outmsspcLK3.dat 
${preCMD} ${TSSPC} Rt -K 3 ${smallSAMP} 0.1 ${rDPATH0}outtsspcLK3.dat 


${preCMD} ${MSSPC} RfE ${smallSAMPE} 0.1 ${rDPATH0}outmsspce.dat 
${preCMD} ${TSSPC} RfE ${smallSAMPE} 0.1 ${rDPATH0}outtsspce.dat 

${preCMD} ${MSSPC} Rf ${smallSAMPE} 0.1 ${rDPATH0}outmsspce0.dat 
${preCMD} ${TSSPC} Rf ${smallSAMPE} 0.1 ${rDPATH0}outtsspce0.dat 

}
function sspcminiRUN() {

${preCMD} ${MSSPC} Rft  ${smallSAMP} 0.1 ${rDPATH0}outmsspcnw.dat 
${preCMD} ${TSSPC} Rft  ${smallSAMP} 0.1 ${rDPATH0}outtsspcnw.dat 

${preCMD} ${MSSPC} Rft -l 5 ${smallSAMP} 0.1 ${rDPATH0}outmsspcnwl5.dat 
${preCMD} ${TSSPC} Rft -l 5 ${smallSAMP} 0.1 ${rDPATH0}outtsspcnwl5.dat 

${preCMD} ${MSSPC} Rft -l 5 -k 2  ${smallSAMP} 0.1 ${rDPATH0}outmsspcnwl5k2.dat 
${preCMD} ${TSSPC} Rft -l 5 -k 2  ${smallSAMP} 0.1 ${rDPATH0}outtsspcnwl5k2.dat 


${preCMD} ${MSSPC} Rf  ${smallSAMPT} 0.1 ${rDPATH0}outmsspcntw.dat 
${preCMD} ${TSSPC} Rf  ${smallSAMPT} 0.1 ${rDPATH0}outtsspcntw.dat 


${preCMD} ${MSSPC} Rft -w ${sDPATH1}weight.dat ${smallSAMP} 0.1 ${rDPATH0}outmsspcw.dat 
${preCMD} ${TSSPC} Rft -w ${sDPATH1}weight.dat ${smallSAMP} 0.1 ${rDPATH0}outtsspcw.dat 


${preCMD} ${MSSPC} Rf -W ${sDPATH1}weightt.dat ${smallSAMPT} 0.1 ${rDPATH0}outmsspcwt.dat 
${preCMD} ${TSSPC} Rf -W ${sDPATH1}weightt.dat ${smallSAMPT} 0.1 ${rDPATH0}outtsspcwt.dat 

${preCMD} ${MSSPC} RfE ${smallSAMPE} 0.1 ${rDPATH0}outmsspce.dat 
${preCMD} ${TSSPC} RfE ${smallSAMPE} 0.1 ${rDPATH0}outtsspce.dat 

${preCMD} ${MSSPC} Rf ${smallSAMPE} 0.1 ${rDPATH0}outmsspce0.dat 
${preCMD} ${TSSPC} Rf ${smallSAMPE} 0.1 ${rDPATH0}outtsspce0.dat 


}

function grhfilRUN() {

${preCMD} ${MGRHFIL} U -t 1 ${smallGSAMP} ${rDPATH0}outmgfil.dat 
${preCMD} ${TGRHFIL} U -t 1 ${smallGSAMP} ${rDPATH0}outtgfil.dat 

}

function medsetRUN() {

${preCMD} ${MMEDSET} -l 5 ${smallSAMP4} ${smallSAMP} 0.700000 ${rDPATH0}outmmed.dat 
${preCMD} ${TMEDSET} -l 5 ${smallSAMP4} ${smallSAMP} 0.700000 ${rDPATH0}outtmed.dat 


}

function maceRUN() {

${preCMD} ${MMACE} M -l 2 ${smallSAMP3} ${rDPATH0}outmmaceM.dat 
${preCMD} ${TMACE} M -l 2 ${smallSAMP3} ${rDPATH0}outtmaceM.dat 

${preCMD} ${MMACE} C -l 2 ${smallSAMP3} ${rDPATH0}outmmaceC.dat 
${preCMD} ${TMACE} C -l 2 ${smallSAMP3} ${rDPATH0}outtmaceC.dat 

}

function simsetRUN() {

${preCMD} ${MSIMSET} T ${smallSAMP} 2 4 ${rDPATH0}outmsimT.dat 
${preCMD} ${TSIMSET} T ${smallSAMP} 2 4 ${rDPATH0}outtsimT.dat 

${preCMD} ${MSIMSET} TM -v 0.7 ${smallSAMP} 0.0001 2 ${rDPATH0}outmsim.dat 
${preCMD} ${TSIMSET} TM -v 0.7 ${smallSAMP} 0.0001 2 ${rDPATH0}outtsim.dat 

}


function lcmminiCHK() {

diff ${DIFFSIMPLE} ${rDPATH0}outmsampF0.dat ${rDPATH0}outtsampF0.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsampM0.dat ${rDPATH0}outtsampM0.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsampC0.dat ${rDPATH0}outtsampC0.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsamp0c.dat ${rDPATH0}outtsamp0c.dat
}

function lcmCHK() {

diff ${DIFFSIMPLE} ${rDPATH0}outmsampF0.dat ${rDPATH0}outtsampF0.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsampM0.dat ${rDPATH0}outtsampM0.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsampC0.dat ${rDPATH0}outtsampC0.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsamp0c.dat ${rDPATH0}outtsamp0c.dat

diff ${DIFFSIMPLE} ${rDPATH0}outmlcmF.dat ${rDPATH0}outtlcmF.dat 


diff ${DIFFSIMPLE} ${rDPATH0}outmlcmM.dat ${rDPATH0}outtlcmM.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmlcmC.dat ${rDPATH0}outtlcmC.dat 

}


function lcmseqCHK() {

diff ${DIFFSIMPLE} ${rDPATH0}outmseqF.dat ${rDPATH0}outtseqF.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmseqFp.dat ${rDPATH0}outtseqFp.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmseqFpl.dat ${rDPATH0}outtseqFpl.dat 


diff ${DIFFSIMPLE} ${rDPATH0}outtseqFw.dat ${rDPATH0}outtseqFw.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outtseqFpw.dat ${rDPATH0}outtseqFpw.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outtseqFplw.dat ${rDPATH0}outtseqFplw.dat 

}

function sspcCHK() {

diff ${DIFFSIMPLE} ${rDPATH0}outmsspcRb10.dat ${rDPATH0}outtsspcRb10.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsspcR.dat ${rDPATH0}outtsspcR.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsspcC.dat ${rDPATH0}outtsspcC.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsspcC2.dat ${rDPATH0}outtsspcC2.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsspck3.dat ${rDPATH0}outtsspck3.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsspcLK3.dat ${rDPATH0}outtsspcLK3.dat 

diff ${DIFFSIMPLE} ${rDPATH0}outmsspce.dat ${rDPATH0}outtsspce.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsspce0.dat ${rDPATH0}outtsspce0.dat 

}

function sspcminiCHK() {

diff ${DIFFSIMPLE} ${rDPATH0}outmsspcnwl5.dat ${rDPATH0}outtsspcnwl5.dat
diff ${DIFFSIMPLE} ${rDPATH0}outmsspcnwl5k2.dat ${rDPATH0}outtsspcnwl5k2.dat
diff ${DIFFSIMPLE} ${rDPATH0}outmsspcw.dat ${rDPATH0}outtsspcw.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsspcnw.dat ${rDPATH0}outtsspcnw.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsspcntw.dat ${rDPATH0}outtsspcntw.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsspcwt.dat ${rDPATH0}outtsspcwt.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsspce.dat ${rDPATH0}outtsspce.dat 


}


function grhfilCHK() {

diff ${DIFFSIMPLE} ${rDPATH0}outmgfil.dat ${rDPATH0}outtgfil.dat 

}

function medsetCHK(){
diff ${DIFFSIMPLE} ${rDPATH0}outmmed.dat ${rDPATH0}outtmed.dat 
}

function maceCHK (){
diff ${DIFFSIMPLE} ${rDPATH0}outmmaceM.dat ${rDPATH0}outtmaceM.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmmaceC.dat ${rDPATH0}outtmaceC.dat 
}

function simsetCHK (){

diff ${DIFFSIMPLE} ${rDPATH0}outmsimT.dat ${rDPATH0}outtsimT.dat 
diff ${DIFFSIMPLE} ${rDPATH0}outmsim.dat ${rDPATH0}outtsim.dat 

}


function allRUN(){

lcmRUN
grhfilRUN
lcmseqRUN
sspcRUN
grhfilRUN
medsetRUN
maceRUN
simsetRUN

}

function allCHK(){

lcmCHK
grhfilCHK
lcmseqCHK
sspcCHK
grhfilCHK
medsetCHK
maceCHK
simsetCHK

}


function shrtest(){


${preCMD} ${MSSPC} Rft# ${smallSAMP} 0.1 ${rDPATH0}01
${preCMD} ${MSSPC} Rft#  -b 2  ${smallSAMP} 0.1 ${rDPATH0}02

${preCMD} ${TSSPC} Rft# ${smallSAMP} 0.1 ${rDPATH0}11
${preCMD} ${TSSPC} Rft#  -b 2  ${smallSAMP} 0.1 ${rDPATH0}12

exit


${preCMD} ${MSSPC} RftE ${smallSAMP} 0.1 ${rDPATH0}05
${preCMD} ${TSSPC} RftE ${smallSAMP} 0.1 ${rDPATH0}15

${preCMD} ${MSSPC} Rf ${smallSAMP} 0.1 ${rDPATH0}06
${preCMD} ${TSSPC} Rf ${smallSAMP} 0.1 ${rDPATH0}16
${preCMD} ${MSSPC} RfE ${smallSAMP} 0.1 ${rDPATH0}07
${preCMD} ${TSSPC} RfE ${smallSAMP} 0.1 ${rDPATH0}17


exit


${preCMD} ${MSSPC} Rft  -L 3 -l 3  ${smallSAMP} 0.1 ${rDPATH0}03
${preCMD} ${MSSPC} Rft  -L 3 -b 2  ${smallSAMP} 0.1 ${rDPATH0}04


${preCMD} ${TSSPC} Rft  -L 3 -l 3  ${smallSAMP} 0.1 ${rDPATH0}13
${preCMD} ${TSSPC} Rft  -L 3 -b 2  ${smallSAMP} 0.1 ${rDPATH0}14

}

#${preCMD} ${TSSPC} Rft -w ${sDPATH1}weight.dat -b 2  ${smallSAMP} 0.1 ${rDPATH0}outmsspcw.dat 


#${preCMD} ${TSSPC} Rft -w ${sDPATH1}weight.dat -L 3 -l 3 -b 2  ${smallSAMP} 0.1 ${rDPATH0}outmsspcw.dat 

#${preCMD} ${MSSPC} Rf -W ${sDPATH1}weightt.dat ${smallSAMPT} 0.1 ${rDPATH0}outmsspcwt.dat 
#${preCMD} ${MSSPC} Rt -k 3 -K 3 ${smallSAMP} 0.1 ${rDPATH0}outmsspcRb10Kk.dat 
#${preCMD} ${TSSPC} Rt -k 3 -K 3 ${smallSAMP} 0.1 ${rDPATH0}outtsspcRb10Kk.dat 

sspcminiRUN
#allRUN
#lcmminiRUN
#lcmRUN
#lcmseqRUN
#sspcRUN
#maceRUN
#simsetRUN

echo vvvvvvvv1
sspcminiCHK
#allCHK
#lcmminiCHK
#lcmCHK
#lcmseqCHK
#sspcCHK
#maceCHK
#simsetCHK

echo vvvvvvvv2

