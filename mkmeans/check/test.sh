CMDDIR="../cmd"
$CMDDIR/mkmeans n=5 f=val1,val2,val3 i=dat.csv a=cls d=0 o=rls1.csv S=1
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat1.csv a=cls d=0 o=rls2.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat2.csv a=cls d=0 o=rls3.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat3.csv a=cls d=0 o=rls4.csv


$CMDDIR/mkmeans n=5 f=val1,val2,val3 i=dat.csv a=cls d=1 o=rls5.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat1.csv a=cls d=1 o=rls6.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat2.csv a=cls d=1 o=rls7.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat3.csv a=cls d=1 o=rls8.csv

$CMDDIR/mkmeans n=5 f=val1,val2,val3 i=dat.csv a=cls o=rls9.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat1.csv a=cls o=rls10.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat2.csv a=cls o=rls11.csv
$CMDDIR/mkmeans f=val1,val2 n=2 i=dat3.csv a=cls o=rls12.csv


