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
#include <cstdio>
#include "attrvec.h"

 
void AttrVec::push(CategoryItem * fld){
	_catitem.push_back(fld);
	_attritemC++;
}
void AttrVec::push(NumericItem * fld){
	_numitem.push_back(fld);
	_attritemC++;
}
void AttrVec::push(PatternItem * fld){
	_patitem.push_back(fld);
	_attritemC++;
}
void AttrVec::init_read(char **lin){
	for(int i=0 ; i<_catitem.size() ; i++){ _catitem[i]->init_read(lin);}
	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->init_read(lin);}
}

void AttrVec::itemSet(map<kgstr_t,vector<kgstr_t> > & ilist){
	for(int i=0 ; i<_catitem.size() ; i++){ _catitem[i]->itemSet(ilist[_catitem[i]->name()]);}
	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->itemSet(ilist[_patitem[i]->name()]);}
}

void AttrVec::idxSet(map<kgstr_t,map<kgstr_t,int> > & ilist){
	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->idxSet(ilist[_patitem[i]->name()]);}
}

int AttrVec::item2id(kgstr_t name ,kgstr_t idname){
	for(int i=0 ; i<_catitem.size() ; i++){
		if(_catitem[i]->name()==name ){
		 return _catitem[i]->item2id(const_cast<char*>(idname.c_str()));
		}
	}
	return -1;
}

//void AttrVec::init_readT(char **lin){
//	for(int i=0 ; i<_catitem.size() ; i++){ _catitem[i]->init_readT(lin);}
//	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->init_readT(lin);}
//}
//void AttrVec::read(char **lin,bool dset){
void AttrVec::read(char **lin){
	//for(int i=0 ; i<_catitem.size() ; i++){ _catitem[i]->read(lin,dset); }
	//for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->read(lin,dset); }
	//for(int i=0 ; i<_numitem.size() ; i++){ _numitem[i]->read(lin,dset); }
	for(int i=0 ; i<_catitem.size() ; i++){ _catitem[i]->read(lin); }
	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->read(lin); }
	for(int i=0 ; i<_numitem.size() ; i++){ _numitem[i]->read(lin); }
}
//void AttrVec::readT(char **lin){
//	for(int i=0 ; i<_catitem.size() ; i++){ _catitem[i]->readT(lin); }
//	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->readT(lin); }
//	for(int i=0 ; i<_numitem.size() ; i++){ _numitem[i]->readT(lin); }
//}

void AttrVec::adjustIndexSize(void){
	for(int i=0 ; i<_patitem.size() ; i++){
		if(_patitem[i]->idxSz()==0){
			_patitem[i]->noidxOn();
		}
	}
}

void AttrVec::setAiQue(AIQueue &aique){
	for(int i=0 ; i<_patitem.size() ; i++){
		aique.add( _patitem[i]->setAindex(_rand_mod.get()) );
	}
}


void AttrVec::prerun(void){
	for(int i=0 ; i<_patitem.size() ; i++){ _patitem[i]->init_aindex(); }
}

void AttrVec::updAique(AIQueue &aique,ClsItem* _cls){
	for(int i=0 ; i<_patitem.size() ; i++){
		AIndex* ai = aique.get(i);
		if( ai->updated() ){
			_patitem[i]->make_idx(ai);
			ai->updated(false);
		}
		_patitem[i]->make_regexp(_cls);
	}
}

fldItem * AttrVec::setCondition(Node* node,Pointer& pointer,ClsItem* _cls)
{
	fldItem *better_class=NULL;
	for(int i=0 ; i<_patitem.size() ; i++){
		Condition	comd_tmp(_cls);
		comd_tmp.setCondition(pointer,_patitem[i]);

		if ( node->updCondition(comd_tmp,_patitem[i])){ better_class=_patitem[i]; }
	}
	for(int i=0 ; i<_numitem.size() ; i++){
		Condition	comd_tmp(_cls);
		comd_tmp.setCondition(pointer,_numitem[i]);
		if ( node->updCondition(comd_tmp,_numitem[i])){ better_class=_numitem[i]; }
	}
	for(int i=0 ; i<_catitem.size() ; i++){ 
		Condition	comd_tmp(_cls);
		comd_tmp.setCondition(pointer,_catitem[i]);
		if ( node->updCondition(comd_tmp,_catitem[i])){ better_class=_catitem[i]; }
	}
	return better_class;
}

void AttrVec::output(ostream* ofp){
	for(int i=0 ; i<_patitem.size() ; i++){ 
		*ofp << "Field Name: " << _patitem[i]->name() << endl;
		_patitem[i]->printIdx(ofp);
	}	
}

void AttrVec::dFldPrint(FILE* fp){
	for(int i=0 ; i<_catitem.size() ; i++){ 
		fprintf(fp,"\t\t<DataField name=\"%s\" optype=\"categorical\" dataType=\"string\">\n" , 
						_catitem[i]->name().c_str() );
		for(int j=0 ; j<_catitem[i]->itmSz() ; j++ ){
			fprintf(fp,"\t\t\t<Value value=\"%s\"/>\n", _catitem[i]->id2item(j).c_str());
		}
		fprintf(fp,"\t\t</DataField>\n");
	}
	for(int i=0 ; i<_numitem.size() ; i++){ 
		fprintf(fp,"\t\t<DataField name=\"%s\" optype=\"continuous\" dataType=\"double\"/>\n" ,
						_numitem[i]->name().c_str() );
	}
	for(int i=0 ; i<_patitem.size() ; i++){ 
		fprintf(fp,"\t\t<DataField name=\"%s\" optype=\"categorical\" dataType=\"string\">\n" ,
						_patitem[i]->name().c_str() );
		for(int j=0 ; j<_patitem[i]->itmSz() ; j++ ){
			fprintf(fp,"\t\t\t<Value value=\"%s\"/>\n", _patitem[i]->id2item(j).c_str());
		}
		fprintf(fp,"\t\t</DataField>\n");
	}
}
void AttrVec::showAlphabet(){
	for(int i=0 ; i<_patitem.size() ; i++){
		for(int j=0 ; j<_patitem[i]->itmSz() ; j++ ){
			cerr << _patitem[i]->id2item(j).c_str();
		}
		cerr << " ";
	}
}

void AttrVec::infoPrint(FILE* fp)
{
	for(int i=0 ; i<_catitem.size() ; i++){ 
		fprintf(fp, "\t\t\t<MiningField name=\"%s\" missingValueTreatment=\"asMean\"/>\n"
						 , _catitem[i]->name().c_str() );
	}
	for(int i=0 ; i<_patitem.size() ; i++){ 
		fprintf(fp,"\t\t\t<MiningField name=\"%s\">\n" , _patitem[i]->name().c_str() );
		fprintf(fp,"\t\t\t\t<Extension extender=\"KGMOD\" name=\"alphabetIndex\">\n");
		for(int j=0 ; j<_patitem[i]->itmSz() ; j++ ){
			fprintf(fp,"\t\t\t\t\t<alphabetIndex alphabet=\"%s\" index=\"%ld\"/>\n" ,
						 _patitem[i]->id2item(j).c_str() , _patitem[i]->id2bestAidx(j) );
		}
		fprintf(fp, "\t\t\t\t</Extension>\n");
		fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"indexSize\" value=\"%lu\"/>\n",_patitem[i]->idxSz());

		if(_patitem[i]->begin()!=0){
			fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"head\" value=\"%d\"/>\n",_patitem[i]->begin());
		}
		if(_patitem[i]->end()!=0){
			fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"tail\" value=\"%d\"/>\n",_patitem[i]->end());
		}
		if(_patitem[i]->ordered()){
			fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"order\" value=\"true\"/>\n");
		}else{
			fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"order\" value=\"false\"/>\n");
		}
		if(_patitem[i]->subseq()){
			fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"subsequence\" value=\"true\"/>\n");
		}else{
			fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"subsequence\" value=\"false\"/>\n");
		}
		fprintf(fp, "\t\t\t\t<Extension extender=\"KGMOD\" name=\"indexLen\" value=\"%d\"/>\n",_patitem[i]->indexLen());

		fprintf(fp, "\t\t\t</MiningField>\n");
	}
	for(int i=0 ; i<_numitem.size() ; i++){ 
		fprintf(fp, "\t\t\t<MiningField name=\"%s\" missingValueTreatment=\"asMean\"/>\n"
						 , _numitem[i]->name().c_str() );
	}
}
fldItem * AttrVec::findclass(string name){
	for(int i=0 ; i<_catitem.size() ; i++){ 
		if(_catitem[i]->name()==name){ return _catitem[i] ;}
	}
	for(int i=0 ; i<_patitem.size() ; i++){
		if(_patitem[i]->name()==name){ return _patitem[i] ;}
	}
	for(int i=0 ; i<_numitem.size() ; i++){ 
		if(_numitem[i]->name()==name){ return _numitem[i] ;}
	}
	cerr << "no math" << endl;
	return NULL ;
}

void AttrVec::show(int indent)
{
	for(int i=0; i<indent; i++) printf(" ");
	printf("## class AttrVec ##\n");
	printf("_attritemC: %d\n",_attritemC);

	if(_catitem.size()>0) printf("category fields:\n");
	for(int i=0 ; i<_catitem.size() ; i++){ 
		_catitem[i]->show(indent+2);
	}
	printf("\n");

	if(_patitem.size()>0) printf("pattern fields:\n");
	for(int i=0 ; i<_patitem.size() ; i++){ 
		_patitem[i]->show(indent+2);
	}
	printf("\n");

	if(_numitem.size()>0) printf("numerical fields:\n");
	for(int i=0 ; i<_numitem.size() ; i++){ 
		_numitem[i]->show(indent+2);
	}
	printf("\n");
}

