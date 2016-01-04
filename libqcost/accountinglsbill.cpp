/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2016 Mocciola Michele

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "accountinglsbill.h"

#include "billprinter.h"
#include "attributesmodel.h"
#include "accountinglsbillitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "projectpricelistparentitem.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QVariant>
#include <QString>

class AccountingLSBillPrivate{
public:
    AccountingLSBillPrivate( const QString &c, const QString &n,
                             PriceFieldModel * pfm, AttributesModel * attrsModel, MathParser * prs = NULL ):
        id(0),
        code( c ),
        name( n ),
        PPUTotalToDiscount(0.0),
        PPUNotToDiscount(0.0),
        priceFieldModel(pfm),
        attributesModel(attrsModel),
        parser(prs),
        rootItem(new AccountingLSBillItem( NULL, NULL, pfm, parser )),
        priceList( NULL ),
        priceListIdTmp(0){
    }
    ~AccountingLSBillPrivate(){
        delete rootItem;
    }

    static void setPriceItemParents( PriceList *pl, PriceItem * basePriceItem, PriceItem * newPriceItem ){
        if( basePriceItem->parentItem() != NULL ){
            if( basePriceItem->parentItem()->parentItem() != NULL ){
                PriceItem * newPriceItemParent = pl->priceItemCode( basePriceItem->parentItem()->code() );
                if( newPriceItemParent == NULL ){
                    newPriceItemParent = pl->appendPriceItem();
                    *newPriceItemParent = *(basePriceItem->parentItem());
                }
                setPriceItemParents( pl, basePriceItem->parentItem(), newPriceItemParent );
                newPriceItem->setParentItem( newPriceItemParent );
            }
        }
    }

    unsigned int id;

    QString code;
    QString name;
    QString description;
    double PPUTotalToDiscount;
    double PPUNotToDiscount;

    PriceFieldModel * priceFieldModel;
    AttributesModel * attributesModel;
    MathParser * parser;
    AccountingLSBillItem * rootItem;
    PriceList * priceList;
    unsigned int priceListIdTmp;

    static int amountPrecision;
};

int AccountingLSBillPrivate::amountPrecision = 2;

AccountingLSBill::AccountingLSBill( const QString &c, const QString &n,
                                    ProjectItem *parent,
                                    PriceFieldModel * pfm,
                                    AttributesModel * attrsModel,
                                    MathParser * parser ):
    QAbstractItemModel(),
    ProjectItem(parent),
    m_d( new AccountingLSBillPrivate( c, n, pfm, attrsModel, parser ) ) {
    connect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)>(&AccountingLSBillItem::dataChanged), this, &AccountingLSBill::updateData );

    connect( m_d->rootItem, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBill::projAmountChanged );
    connect( m_d->rootItem, &AccountingLSBillItem::accAmountChanged, this, &AccountingLSBill::accAmountChanged );
    connect( m_d->rootItem, &AccountingLSBillItem::percentageAccountedChanged, this, &AccountingLSBill::percentageAccountedChanged );

    connect( m_d->rootItem, &AccountingLSBillItem::itemChanged, this, &AccountingLSBill::modelChanged );
}

AccountingLSBill::AccountingLSBill(AccountingLSBill & b):
    QAbstractItemModel(),
    ProjectItem( b.ProjectItem::parentItem() ),
    m_d( new AccountingLSBillPrivate( b.m_d->code, b.m_d->name,
                                      b.m_d->priceFieldModel, b.m_d->attributesModel, b.m_d->parser ) ) {

    *this = b;

    connect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)>(&AccountingLSBillItem::dataChanged), this, &AccountingLSBill::updateData );

    connect( m_d->rootItem, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBill::projAmountChanged );
    connect( m_d->rootItem, &AccountingLSBillItem::accAmountChanged, this, &AccountingLSBill::accAmountChanged );
    connect( m_d->rootItem, &AccountingLSBillItem::percentageAccountedChanged, this, &AccountingLSBill::percentageAccountedChanged );

    connect( m_d->rootItem, &AccountingLSBillItem::itemChanged, this, &AccountingLSBill::modelChanged );
}

AccountingLSBill::~AccountingLSBill(){
    disconnect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)>(&AccountingLSBillItem::dataChanged), this, &AccountingLSBill::updateData );

    disconnect( m_d->rootItem, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBill::projAmountChanged );
    disconnect( m_d->rootItem, &AccountingLSBillItem::accAmountChanged, this, &AccountingLSBill::accAmountChanged );
    disconnect( m_d->rootItem, &AccountingLSBillItem::percentageAccountedChanged, this, &AccountingLSBill::percentageAccountedChanged );

    disconnect( m_d->rootItem, &AccountingLSBillItem::itemChanged, this, &AccountingLSBill::modelChanged );

    emit aboutToBeDeleted();

    delete m_d;
}

AccountingLSBill &AccountingLSBill::operator=(const AccountingLSBill &cp) {
    setCode( cp.m_d->code );
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    setPriceList( cp.m_d->priceList );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
    return *this;
}

QString AccountingLSBill::code() const {
    return m_d->code;
}

void AccountingLSBill::setCode(const QString &value) {
    if( m_d->code != value ){
        m_d->code = value;
        emit codeChanged( m_d->code );
    }
}

QString AccountingLSBill::name()  const {
    return m_d->name;
}

void AccountingLSBill::setName(const QString &value) {
    if( m_d->name != value ){
        m_d->name = value;
        emit nameChanged( m_d->name );
        if( m_parentItem ){
            m_parentItem->setDataChanged( 0, this );
        }
    }
}

QString AccountingLSBill::description()  const {
    return m_d->description;
}

double AccountingLSBill::PPUTotalToDiscount() const {
    return m_d->PPUTotalToDiscount;
}

QString AccountingLSBill::PPUTotalToDiscountStr() const {
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->PPUTotalToDiscount, 'f', m_d->amountPrecision );
    }
    return QString::number( m_d->PPUTotalToDiscount, 'f', m_d->amountPrecision );
}

double AccountingLSBill::PPUNotToDiscount() const {
    return m_d->PPUNotToDiscount;
}

QString AccountingLSBill::PPUNotToDiscountStr() const {
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->PPUNotToDiscount, 'f', m_d->amountPrecision );
    }
    return QString::number( m_d->PPUNotToDiscount, 'f', m_d->amountPrecision );
}

void AccountingLSBill::setDescription(const QString &value) {
    if( m_d->description != value ){
        m_d->description = value;
        emit descriptionChanged( value );
    }
}

void AccountingLSBill::setPPUTotalToDiscount(const QString &newValue) {
    double v = 0.0;
    if( m_d->parser != NULL ){
        v = m_d->parser->evaluate( newValue );
    } else {
        v = newValue.toDouble();
    }
    if( v != m_d->PPUTotalToDiscount ){
        m_d->PPUTotalToDiscount = v;
        emit PPUTotalToDiscountChanged( PPUTotalToDiscountStr() );
    }
}

void AccountingLSBill::setPPUNotToDiscount(const QString &newValue) {
    double v = 0.0;
    if( m_d->parser != NULL ){
        v = m_d->parser->evaluate( newValue );
    } else {
        v = newValue.toDouble();
    }
    if( v != m_d->PPUNotToDiscount ){
        m_d->PPUNotToDiscount = v;
        emit PPUNotToDiscountChanged( PPUNotToDiscountStr() );
    }
}

void AccountingLSBill::setPriceDataSet(int v) {
    if( v > -1 && v < m_d->priceList->priceDataSetCount() &&
            m_d->rootItem->currentPriceDataSet() != v ){
        m_d->rootItem->setCurrentPriceDataSet( v );
    }
}

ProjectItem *AccountingLSBill::child(int /*number*/) {
    return NULL;
}

int AccountingLSBill::childCount() const {
    return 0;
}

int AccountingLSBill::childNumber(ProjectItem * /*item*/) {
    return -1;
}

bool AccountingLSBill::reset() {
    return m_d->rootItem->reset();
}

bool AccountingLSBill::canChildrenBeInserted() {
    return false;
}

bool AccountingLSBill::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool AccountingLSBill::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

Qt::ItemFlags AccountingLSBill::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant AccountingLSBill::data() const {
    return QVariant( m_d->name );
}

bool AccountingLSBill::setData(const QVariant &value) {
    if( m_d->name != value ){
        m_d->name = value.toString();
        return true;
    } return false;
}

void AccountingLSBill::setPriceList(PriceList *pl, AccountingLSBill::SetPriceListMode plMode) {
    if( pl != m_d->priceList ){
        if( plMode != None ){
            if( m_d->priceList != NULL ){
                if( plMode == SearchAndAdd ){
                    // cerca in base al codice e aggiunge se manca
                    QList<AccountingLSBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingLSBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        PriceItem * newPriceItem = NULL;
                        if( (*i)->priceItem()!= NULL ){
                            newPriceItem = pl->priceItemCode( (*i)->priceItem()->code() );
                            if( newPriceItem == NULL ){
                                newPriceItem = pl->appendPriceItem();
                                AccountingLSBillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
                                *newPriceItem = *((*i)->priceItem());
                            }
                        }
                        (*i)->setPriceItem( newPriceItem );
                    }
                } else if( plMode == Add ){
                    // aggiunge sempre e comunque
                    QList<AccountingLSBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingLSBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        PriceItem * newPriceItem = NULL;
                        if( (*i)->priceItem()!= NULL ){
                            newPriceItem = pl->appendPriceItem();
                            AccountingLSBillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
                            *newPriceItem = *((*i)->priceItem());
                        }
                        (*i)->setPriceItem( newPriceItem );
                    }
                } else if( plMode == Search ){
                    // cerca
                    QList<AccountingLSBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingLSBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        (*i)->setPriceItem( pl->priceItemCode( (*i)->priceItem()->code() ) );
                    }
                } else if( plMode == NULLPriceItem ){
                    // annulla
                    QList<AccountingLSBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingLSBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        (*i)->setPriceItem( NULL );
                    }
                } else if( plMode == ResetBill ){
                    // resetta il computo
                    removeItems( 0, m_d->rootItem->childrenCount() );
                }
                if( pl == NULL ){
                    m_d->rootItem->setCurrentPriceDataSet( 0 );
                } else {
                    if( m_d->rootItem->currentPriceDataSet() > pl->priceDataSetCount() ){
                        m_d->rootItem->setCurrentPriceDataSet( 0 );
                    }
                }
            }
            m_d->priceList = pl;
            emit priceListChanged( pl );
        }
    }
}

PriceList *AccountingLSBill::priceList() {
    return m_d->priceList;
}

int AccountingLSBill::priceDataSet() {
    return m_d->rootItem->currentPriceDataSet();
}

QList<int> AccountingLSBill::totalAmountPriceFields() {
    return m_d->rootItem->totalAmountPriceFields();
}

void AccountingLSBill::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    m_d->rootItem->setTotalAmountPriceFields( newAmountFields );
}

AccountingPriceFieldModel *AccountingLSBill::totalAmountPriceFieldModel() {
    return m_d->rootItem->totalAmountPriceFieldModel();
}


AccountingLSBillItem *AccountingLSBill::item(const QModelIndex &index ) const {
    if (index.isValid()) {
        return static_cast<AccountingLSBillItem *>(index.internalPointer());
    }
    return m_d->rootItem;
}

AccountingLSBillItem *AccountingLSBill::item(int childNum, const QModelIndex &parentIndex ) {
    if (parentIndex.isValid()) {
        AccountingLSBillItem * parentItem = static_cast<AccountingLSBillItem *>(parentIndex.internalPointer());
        if( childNum > -1 && childNum < parentItem->childrenCount() ){
            return parentItem->childItem( childNum );
        }
    }
    return m_d->rootItem;
}

AccountingLSBillItem *AccountingLSBill::lastItem(const QModelIndex &parentIndex) {
    if (parentIndex.isValid()) {
        AccountingLSBillItem * parentItem = static_cast<AccountingLSBillItem *>(parentIndex.internalPointer());
        return parentItem->childItem( parentItem->childrenCount()-1 );
    }
    return m_d->rootItem;
}

AccountingLSBillItem *AccountingLSBill::itemId(unsigned int itemId) {
    return m_d->rootItem->itemId( itemId );
}

QVariant AccountingLSBill::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    AccountingLSBillItem *i = item(index);

    return i->data(index.column(), role);
}

QVariant AccountingLSBill::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal )
        return m_d->rootItem->data(section, role);
    else if (orientation == Qt::Vertical )
        return QVariant( section + 1 );
    return QVariant();
}

int AccountingLSBill::rowCount(const QModelIndex &parent) const {
    AccountingLSBillItem *parentItem = item(parent);
    return parentItem->childrenCount();
}

int AccountingLSBill::columnCount(const QModelIndex &) const {
    return m_d->rootItem->columnCount();
}

Qt::ItemFlags AccountingLSBill::flags(const QModelIndex &index) const {
    AccountingLSBillItem *b = item(index);
    if( b ){
        return (b->flags(  index.column() ) );
    } else {
        return QAbstractItemModel::flags(index);
    }
}

bool AccountingLSBill::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;

    AccountingLSBillItem *b = item(index);
    bool result = b->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool AccountingLSBill::insertBillItems(PriceItem *p, int inputPos, int count, const QModelIndex &parent) {
    AccountingLSBillItem *parentItem = item(parent);

    int position = inputPos;
    if( position == -1 ){
        position = parentItem->childrenCount();
    }

    bool success = false;

    beginInsertRows(parent, position, position + count - 1);
    success = parentItem->insertChildren(p, position, count );
    endInsertRows();

    return success;
}

bool AccountingLSBill::removeItems(int position, int rows, const QModelIndex &parent) {
    AccountingLSBillItem *parentItem = item(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

QModelIndex AccountingLSBill::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    AccountingLSBillItem *childItem = item(index);
    AccountingLSBillItem *parentItem = dynamic_cast<AccountingLSBillItem *>( childItem->parent());

    if (parentItem == m_d->rootItem || parentItem == 0 )
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

QModelIndex AccountingLSBill::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    AccountingLSBillItem *parentItem = dynamic_cast<AccountingLSBillItem *>(item(parent));

    if( parentItem ){
        AccountingLSBillItem *childItem = dynamic_cast<AccountingLSBillItem *>(parentItem->child(row));
        if (childItem)
            return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex AccountingLSBill::index(AccountingLSBillItem *item, int column) const {
    if (item == NULL )
        return QModelIndex();

    if( item->parent() == NULL ){
        return QModelIndex();
    } else {
        return createIndex(item->childNumber(), column, item);
    }
}

bool AccountingLSBill::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow ) {

    if(beginMoveRows( sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow )){
        AccountingLSBillItem * srcParent = item( sourceParent );
        AccountingLSBillItem * dstParent = item( destinationParent );
        for( int i=0; i<count; ++i ){
            srcParent->childItem( (sourceRow+count-1)-i )->setParent( dstParent, destinationRow );
        }

        endMoveRows();
        return true;
    }
    return false;
}

void AccountingLSBill::updateData(AccountingLSBillItem * item, int column) {
    QModelIndex i = index( item, column);
    emit dataChanged( i, i);
}

bool AccountingLSBill::isUsingPriceItem(PriceItem *p) {
    return m_d->rootItem->isUsingPriceItem( p );
}

bool AccountingLSBill::isUsingPriceList(PriceList *pl) {
    return m_d->priceList == pl;
}

double AccountingLSBill::projAmount() const {
    return m_d->rootItem->projAmount();
}

QString AccountingLSBill::projAmountStr() const {
    return m_d->rootItem->projAmountStr();
}

double AccountingLSBill::accAmount() const {
    return m_d->rootItem->accAmount();
}

QString AccountingLSBill::accAmountStr() const {
    return m_d->rootItem->accAmountStr();
}

double AccountingLSBill::percentageAccounted() const {
    return m_d->rootItem->percentageAccounted();
}

QString AccountingLSBill::percentageAccountedStr() const {
    return m_d->rootItem->percentageAccountedStr();
}

double AccountingLSBill::percentageAccounted(const QDate &dBegin, const QDate &dEnd) const {
    return m_d->rootItem->percentageAccounted(dBegin, dEnd);
}

AttributesModel *AccountingLSBill::attributesModel() {
    return m_d->attributesModel;
}

double AccountingLSBill::PPUTotalToDiscount(Attribute *attr) {
    return m_d->rootItem->projAmountAttribute( attr );
}

QString AccountingLSBill::PPUTotalToDiscountStr(Attribute *attr) {
    return m_d->rootItem->projAmountAttributeStr( attr );
}

double AccountingLSBill::accAmountAttribute(Attribute *attr) {
    return m_d->rootItem->accAmountAttribute( attr );
}

QString AccountingLSBill::accAmountAttributeStr(Attribute *attr) {
    return m_d->rootItem->accAmountAttributeStr( attr );
}

void AccountingLSBill::nextId() {
    m_d->id++;
}

unsigned int AccountingLSBill::id() {
    return m_d->id;
}

void AccountingLSBill::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingLSBill" );
    writer->writeAttribute( "id", QString::number(m_d->id) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeAttribute( "description", m_d->description );
    writer->writeAttribute( "PPUTotalToDiscount", QString::number(m_d->PPUTotalToDiscount) );
    writer->writeAttribute( "PPUNotToDiscount", QString::number(m_d->PPUNotToDiscount) );

    QString fields;
    QList<int> fieldList = m_d->rootItem->totalAmountPriceFields();
    QList<int>::iterator i= fieldList.begin();
    if( i != fieldList.end() ){
        fields += QString::number((*i));
        ++i;
    }
    for( ; i != fieldList.end(); ++i ){
        fields += ", ";
        fields += QString::number((*i));
    }
    writer->writeAttribute( "totalAmountPriceFields", fields );

    if( m_d->priceList ){
        writer->writeAttribute( "priceList", QString::number( m_d->priceList->id() ) );
    }
    writer->writeAttribute( "priceDataSet", QString::number( m_d->rootItem->currentPriceDataSet() ) );

    m_d->attributesModel->writeXml( writer );

    m_d->rootItem->writeXml( writer );

    writer->writeEndElement();
}

void AccountingLSBill::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILL"){
        loadFromXml( reader->attributes(), priceLists );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILL") ){
        reader->readNext();
        QString tag = reader->name().toString().toUpper();
        if( tag == "ATTRIBUTEMODEL" && reader->isStartElement()) {
            m_d->attributesModel->readXml( reader );
        }
        if( tag == "ACCOUNTINGLSBILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXml( reader, m_d->priceList, m_d->attributesModel );
        }
    }
}

void AccountingLSBill::readXmlTmp(QXmlStreamReader *reader ) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "BILL"){
        loadFromXmlTmp( reader->attributes() );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXmlTmp( reader );
        }
    }
}

void AccountingLSBill::loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists) {
    for( QXmlStreamAttributes::const_iterator i=attrs.begin(); i != attrs.end(); ++i ){
        QString nameUp = (*i).name().toString().toUpper();
        if( nameUp == "ID" ){
            m_d->id = (*i).value().toUInt();
        }
        if( nameUp == "NAME" ){
            setName( (*i).value().toString() );
        }
        if( nameUp == "DESCRIPTION" ){
            setDescription( (*i).value().toString() );
        }
        if( nameUp == "PRICELIST" ){
            m_d->priceList = priceLists->priceListId( (*i).value().toUInt() );
        }
        if( nameUp == "PRICEDATASET" ){
            m_d->rootItem->setCurrentPriceDataSet( (*i).value().toInt() );
        }
    }
}

void AccountingLSBill::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
    for( QXmlStreamAttributes::const_iterator i=attrs.begin(); i != attrs.end(); ++i ){
        QString nameUp = (*i).name().toString().toUpper();
        if( nameUp == "ID" ){
            m_d->id = (*i).value().toUInt();
        }
        if( nameUp == "NAME" ){
            setName( (*i).value().toString() );
        }
        if( nameUp == "DESCRIPTION" ){
            setDescription( (*i).value().toString() );
        }
        if( nameUp == "PRICELIST" ){
            m_d->priceListIdTmp = (*i).value().toUInt();
        }
        if( nameUp == "PRICEDATASET" ){
            m_d->rootItem->setCurrentPriceDataSet( (*i).value().toInt() );
        }
    }
}

void AccountingLSBill::setTmpData( ProjectPriceListParentItem * priceLists ) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
}

QList<PriceItem *> AccountingLSBill::connectedPriceItems() {
    return m_d->rootItem->connectedPriceItems();
}

void AccountingLSBill::writeODTAccountingOnTable( QTextCursor *cursor,
                                                  const QDate &dateBegin, const QDate &dateEnd,
                                                  AccountingPrinter::PrintLSOption prLSOption,
                                                  AccountingPrinter::PrintPPUDescOption prPPUOption,
                                                  bool writeAmounts ) {
    m_d->rootItem->writeODTAccountingOnTable( cursor, dateBegin, dateEnd, prLSOption, prPPUOption, writeAmounts );
}

void AccountingLSBill::writeODTAttributeBillOnTable(QTextCursor *cursor,
                                                    AccountingPrinter::AttributePrintOption prOption,
                                                    AccountingPrinter::PrintPPUDescOption prPPUOption,
                                                    const QList<Attribute *> &attrsToPrint,
                                                    bool writeAmounts) {
    m_d->rootItem->printODTAttributeBillOnTable( cursor, prOption, prPPUOption, attrsToPrint, writeAmounts );
}

void AccountingLSBill::activateAttributeModel() {
    if( m_d->attributesModel != NULL ){
        m_d->attributesModel->setBill( this );
    }
}


void AccountingLSBill::writeODTSummaryOnTable(QTextCursor *cursor,
                                              AccountingPrinter::PrintPPUDescOption prItemsOption,
                                              bool writeAmounts,
                                              bool writeDetails ) {
    m_d->rootItem->printODTSummaryOnTable( cursor, prItemsOption, writeAmounts, writeDetails );
}

void AccountingLSBill::loadTmpData(ProjectPriceListParentItem * priceLists) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
    m_d->rootItem->loadTmpData( m_d->priceList, m_d->attributesModel );
}
