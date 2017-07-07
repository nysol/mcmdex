AUTOMAKE_OPTIONS = subdir-objects

AM_CXXFLAGS = @XML2HEAD@

bin_PROGRAMS = mbonsai 

### excmd command end ###

mbonsai_SOURCES = aiqueue.cpp \
attrvec.cpp \
clstbl.cpp \
condition.cpp \
cost.cpp \
itemmap.cpp \
kgbonsai.cpp \
kglibxmltool.cpp \
node.cpp \
regptn.cpp \
regptnque.cpp \
tree.cpp \
flditem/catitem.cpp \
flditem/numitem.cpp \
flditem/clsitem.cpp \
flditem/patitem.cpp \
aiqueue.h \
attrvec.h \
baseinfo.h \
clstbl.h \
condition.h \
cost.h \
flditem.h \
itemmap.h \
kgbonsai.h \
kglibxmltool.h \
node.h \
pointer.h \
predictdata.h \
ptnParam.h \
regptn.h \
regptnque.h \
tree.h \
flditem/catitem.h \
flditem/clsitem.h \
flditem/numitem.h \
flditem/patitem.h \
valtype/valtypedbl.h \
valtype/valtypeint.h \
valtype/valtypevec.h

mbonsai_LDFLAGS = @XML2LIB@
mbonsai_LDADD = -lkgmod3 -lboost_filesystem -lboost_system


