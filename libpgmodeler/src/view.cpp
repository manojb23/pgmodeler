#include "view.h"

View::View(void) : BaseTable()
{
 //Definindo configurações inicias para a visão
 obj_type=OBJ_VIEW;
 attributes[ParsersAttributes::DECLARATION]="";
 attributes[ParsersAttributes::REFERENCES]="";
 attributes[ParsersAttributes::SELECT_EXP]="";
 attributes[ParsersAttributes::FROM_EXP]="";
 attributes[ParsersAttributes::EXP_SIMPLES]="";
}

View::~View(void)
{
 //Limpa a lista de referências da visão
 references.clear();

 //Limpa as demais listas
 exp_select.clear();
 exp_from.clear();
 exp_where.clear();
}

int View::getReferenceIndex(Reference &refer)
{
 vector<Reference>::iterator itr, itr_end;
 bool existe=false;
 int idx=-1;

 /* Varre a lista de referencias da visão para
    comparar os elementos */
 itr=references.begin();
 itr_end=references.end();

 while(itr!=itr_end && !existe)
 {
  /* Compara o conteúdo do elemento atual (itr) com
     o conteúdo da referência do parâmetro */
  existe=((*itr)==refer);
  itr++;
  idx++;
 }

 /* Caso a referência não exista retorna -1 caso
    contrário retorna o próprio índice */
 if(!existe) idx=-1;
 return(idx);
}

void View::addReference(Reference &refer, unsigned tipo_sql, int id_exp)
{
 int idx;
 vector<unsigned> *vet_idref=NULL;
 Column *col=NULL;

 //Verifica se a referência já existe na visão
 idx=getReferenceIndex(refer);

 //Caso não seja encontrada (idx = -1)
 if(idx < 0)
 {
  //Insere a nova referência na lista
  references.push_back(refer);
  idx=references.size()-1;
 }

 /* Selecionando a lista de expressões de acordo o
    parâmetro tipo_sql. */
 if(tipo_sql==Reference::SQL_REFER_SELECT)
  vet_idref=&exp_select;
 else if(tipo_sql==Reference::SQL_REFER_FROM)
  vet_idref=&exp_from;
 else
  vet_idref=&exp_where;

 //Insere a referência na posição especificada por id_exp
 if(id_exp >= 0 && id_exp < static_cast<int>(vet_idref->size()))
  vet_idref->insert(vet_idref->begin() + id_exp, static_cast<unsigned>(idx));

 /* Caso se tente inserir uma referência num índice inválido,
    é disparada uma exceção */
 else if(id_exp >= 0 && id_exp >= static_cast<int>(vet_idref->size()))
  throw Exception(ERR_REF_OBJ_INV_INDEX,__PRETTY_FUNCTION__,__FILE__,__LINE__);

 //Insere a referência no fim da lista de expressões
 else
  vet_idref->push_back(static_cast<unsigned>(idx));

 col=refer.getColumn();
 if(col && col->isAddedByRelationship() &&
    col->getObjectId() > this->object_id)
  this->object_id=BaseObject::getGlobalId();
}

unsigned View::getReferenceCount(void)
{
 return(references.size());
}

unsigned View::getReferenceCount(unsigned tipo_sql, int tipo_ref)
{
 vector<unsigned> *vet_idref=NULL;

 /* Selecionando a lista de expressões de acordo o
    parâmetro tipo_sql. */
 if(tipo_sql==Reference::SQL_REFER_SELECT)
  vet_idref=&exp_select;
 else if(tipo_sql==Reference::SQL_REFER_FROM)
  vet_idref=&exp_from;
 else
  vet_idref=&exp_where;

 /* Caso o tipo de referência não esteja especificado
    retorna o tamanho total do vetor de expressões */
 if(tipo_ref < 0)
  return(vet_idref->size());

 /* Caso contrário, faz a contagem dos elementos da lista
    de acordo com o tipo da referência */
 else
 {
  vector<unsigned>::iterator itr, itr_end;
  unsigned qtd;

  qtd=0;
  itr=vet_idref->begin();
  itr_end=vet_idref->end();
  while(itr!=itr_end)
  {
   /* Caso o tipo da referência atual seja igual ao tipo da referência
      passada pelo parâmetro, incrementa a quantidade */
   if(references[(*itr)].getReferenceType()==static_cast<unsigned>(tipo_ref)) qtd++;
   itr++;
  }

  //Retorna a quantidade calculada
  return(qtd);
 }
}

Reference View::getReference(unsigned id_ref)
{
 /* Caso tente acessar uma referência com índice
    inválido, dispara uma exceção */
 if(id_ref >= references.size())
  throw Exception(ERR_REF_OBJ_INV_INDEX,__PRETTY_FUNCTION__,__FILE__,__LINE__);

 return(references[id_ref]);
}

Reference View::getReference(unsigned id_ref, unsigned tipo_sql)
{
 vector<unsigned> *vet_idref=NULL;
 unsigned idx;

 /* Selecionando a lista de expressões de acordo o
    parâmetro tipo_sql. */
 if(tipo_sql==Reference::SQL_REFER_SELECT)
  vet_idref=&exp_select;
 else if(tipo_sql==Reference::SQL_REFER_FROM)
  vet_idref=&exp_from;
 else
  vet_idref=&exp_where;

 //Retorna a referência na posição indicada por id_ref
 idx=vet_idref->at(id_ref);
 return(references[idx]);
}

void View::removeReference(unsigned id_ref)
{
 vector<unsigned> *vet_idref[3]={&exp_select, &exp_from, &exp_where};
 vector<unsigned>::iterator itr, itr_aux, itr_end;
 unsigned i;

 /* Dispara uma exceção caso se tente remover uma referência
    com índice inválido */
 if(id_ref >= references.size())
  throw Exception(ERR_REF_OBJ_INV_INDEX,__PRETTY_FUNCTION__,__FILE__,__LINE__);

 /* Varre as três listas de expressões para verificar se o
    a referência não está sendo usada em uma delas */
 for(i=0; i < 3; i++)
 {
  //Obtém o ínicio e o fim da lista
  itr=vet_idref[i]->begin();
  itr_end=vet_idref[i]->end();

  //Varre a lista
  while(itr!=itr_end && !vet_idref[i]->empty())
  {
   /* Caso a referencia atual seja igual   referência
      a ser removeda, o item da lista de expressão atual
      o qual contém a referência a ser excluída será
      removido. */
   if(references[*itr]==references[id_ref])
    vet_idref[i]->erase(itr);

   itr++;
  }
 }

 //Remove a referência da lista
 references.erase(references.begin() + id_ref);
}

void View::removeReferences(void)
{
 references.clear();
 exp_select.clear();
 exp_from.clear();
 exp_where.clear();
}

void View::removeReference(unsigned id_exp, unsigned tipo_sql)
{
 vector<unsigned> *vet_idref=NULL;

 /* Selecionando a lista de expressões de acordo o
    parâmetro tipo_sql. */
 if(tipo_sql==Reference::SQL_REFER_SELECT)
  vet_idref=&exp_select;
 else if(tipo_sql==Reference::SQL_REFER_FROM)
  vet_idref=&exp_from;
 else
  vet_idref=&exp_where;

 if(id_exp >= vet_idref->size())
  throw Exception(ERR_REF_OBJ_INV_INDEX,__PRETTY_FUNCTION__,__FILE__,__LINE__);

 //Remove a referência da lista de expressão
 vet_idref->erase(vet_idref->begin() + id_exp);
}

int View::getReferenceIndex(Reference &ref, unsigned tipo_sql)
{
 vector<unsigned> *vet_idref=NULL;
 vector<unsigned>::iterator itr, itr_aux, itr_end;
 int idx_ref;
 bool enc=false;

  /* Selecionando a lista de expressões de acordo o
    parâmetro tipo_sql. */
 if(tipo_sql==Reference::SQL_REFER_SELECT)
  vet_idref=&exp_select;
 else if(tipo_sql==Reference::SQL_REFER_FROM)
  vet_idref=&exp_from;
 else
  vet_idref=&exp_where;

 idx_ref=getReferenceIndex(ref);
 itr=vet_idref->begin();
 itr_end=vet_idref->end();

 while(itr!=itr_end && !enc)
 {
  enc=(static_cast<int>(*itr)==idx_ref);
  if(!enc) itr++;
 }

 if(!enc)
  return(-1);
 else
  return(itr-vet_idref->begin());
}

void View::setDeclarationAttribute(void)
{
 QString decl;

 if(exp_select.size() > 0)
 {
  vector<unsigned> *vet_ref[3]={&exp_select, &exp_from, &exp_where};
  vector<unsigned>::iterator itr, itr_end;
  QString palavras[3]={"SELECT ", "\n FROM ", "\n WHERE "};
  unsigned i, qtd, idx, tipo_sql[3]={Reference::SQL_REFER_SELECT,
                                     Reference::SQL_REFER_FROM,
                                     Reference::SQL_REFER_WHERE};

  //Varre as listas de expressões
  for(i=0; i < 3; i++)
  {
   //Caso a lista não esteja vazia
   if(vet_ref[i]->size() > 0)
   {
    //Concatena a palavra referente   lista de expressões
    decl+=palavras[i];

    itr=vet_ref[i]->begin();
    itr_end=vet_ref[i]->end();
    while(itr!=itr_end)
    {
     /* Concatena   declaração da visão a sql da referência atual
        especificando o tipo da sql a ser gerada */
     idx=(*itr);
     decl+=references[idx].getSQLDefinition(tipo_sql[i]);
     itr++;
    }

    if(tipo_sql[i]==Reference::SQL_REFER_SELECT ||
       tipo_sql[i]==Reference::SQL_REFER_FROM)
    {
     //Removendo os espaços e vírgula finais
     qtd=decl.size();
     if(decl[qtd-2]==',')
       //decl.erase(qtd-2);
       decl.remove(qtd-2,2);
    }
   }
  }
 }
 attributes[ParsersAttributes::DECLARATION]=decl;
}

void View::setReferencesAttribute(void)
{
 QString str_aux;
 QString vet_atrib[]={ ParsersAttributes::SELECT_EXP,
                       ParsersAttributes::FROM_EXP,
                       ParsersAttributes::EXP_SIMPLES };
 vector<unsigned> *vet_exp[]={&exp_select, &exp_from, &exp_where};
 int qtd, i, i1;

 //Obtém a definição XMl das referências
 qtd=references.size();
 for(i=0; i < qtd; i++)
  str_aux+=references[i].getXMLDefinition();
 attributes[ParsersAttributes::REFERENCES]=str_aux;

 /* Este bloco concatena os indices de referências
    em cada vetor de expressão separando-os por vírgula */
 for(i=0; i < 3; i++)
 {
  str_aux="";
  qtd=vet_exp[i]->size();
  for(i1=0; i1 < qtd; i1++)
  {
   str_aux+=QString("%1").arg(vet_exp[i]->at(i1));
   if(i1 < qtd-1) str_aux+=",";
  }
  attributes[vet_atrib[i]]=str_aux;
 }
}

bool View::isReferRelationshipColumn(void)
{
 Column *coluna=NULL;
 unsigned qtd, i;
 bool enc=false;

 qtd=references.size();

 for(i=0; i < qtd && !enc; i++)
 {
  coluna=references[i].getColumn();
  enc=(coluna && coluna->isAddedByRelationship());
 }

 return(enc);
}

bool View::isReferencingTable(Tabela *tab)
{
 Tabela *tab_aux=NULL;
 unsigned qtd, i;
 bool enc=false;

 qtd=references.size();

 for(i=0; i < qtd && !enc; i++)
 {
  tab_aux=references[i].getTable();
  enc=(tab_aux && (tab_aux == tab));
 }

 return(enc);
}

bool View::isReferencingColumn(Column *col)
{
 unsigned qtd, i;
 bool enc=false;

 if(col)
 {
  qtd=references.size();
  for(i=0; i < qtd && !enc; i++)
   enc=(col==references[i].getColumn());
 }
 return(enc);
}

QString View::getCodeDefinition(unsigned tipo_def)
{
 if(tipo_def==SchemaParser::SQL_DEFINITION)
  setDeclarationAttribute();
 else
 {
  setPositionAttribute();
  setReferencesAttribute();
 }

 return(BaseObject::__getCodeDefinition(tipo_def));
}

void View::operator = (View &visao)
{
 (*dynamic_cast<BaseGraphicObject *>(this))=reinterpret_cast<BaseGraphicObject &>(visao);

 this->references=visao.references;
 this->exp_select=visao.exp_select;
 this->exp_from=visao.exp_from;
 this->exp_where=visao.exp_where;
}
