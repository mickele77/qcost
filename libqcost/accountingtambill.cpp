/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2014 Mocciola Michele

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

#include "accountingtambill.h"

#include "attributemodel.h"
#include "accountingtambillitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "projectpricelistparentitem.h"
#include "pricefieldmodel.h"

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QVariant>
#include <QDate>
#include <QString>

class AccountingTAMBillPrivate{
public:
    AccountingTAMBillPrivate( const QString &n, AccountingTAMBill * b, PriceFieldModel * pfm, MathParser * prs = NULL ):
        id(0),
        name(n),
        priceFieldModel(pfm),
        parser(prs),
        rootItem(new AccountingTAMBillItem( NULL, AccountingBillItem::Root, pfm, parser )),
        priceList( NULL ),
        attributeModel( new AttributeModel( b, parser, pfm )),
        priceListIdTmp(0){
    }
    ~AccountingTAMBillPrivate(){
        delete rootItem;
        delete attributeModel;
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

    QString name;
    QString description;

    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    AccountingTAMBillItem * rootItem;
    PriceList * priceList;
    AttributeModel * attributeModel;
    unsigned int priceListIdTmp;
};

AccountingTAMBill::AccountingTAMBill( const QString &n, ProjectItem *parent, PriceFieldModel * pfm, MathParser * parser ):
    QAbstractItemModel(),
    ProjectItem(parent),
    m_d( new AccountingTAMBillPrivate( n, this, pfm, parser ) ) {

    connect( m_d->rootItem, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)>(&AccountingBillItem::dataChanged), this, &AccountingTAMBill::updateValue );

    connect( m_d->rootItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBill::totalAmountToDiscountChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBill::amountNotToDiscountChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::amountToDiscountChanged, this, &AccountingTAMBill::amountToDiscountChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::amountDiscountedChanged, this, &AccountingTAMBill::amountDiscountedChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::totalAmountChanged, this, &AccountingTAMBill::totalAmountChanged );

    connect( m_d->rootItem, &AccountingTAMBillItem::itemChanged, this, &AccountingTAMBill::modelChanged );

    connect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingTAMBill::modelChanged );
}

AccountingTAMBill::AccountingTAMBill(AccountingTAMBill & b):
    QAbstractItemModel(),
    ProjectItem( b.ProjectItem::parentItem() ),
    m_d( new AccountingTAMBillPrivate( "", this, b.m_d->priceFieldModel, b.m_d->parser ) ) {

    *this = b;

    connect( m_d->rootItem, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)>(&AccountingBillItem::dataChanged), this, &AccountingTAMBill::updateValue );

    connect( m_d->rootItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBill::totalAmountToDiscountChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBill::amountNotToDiscountChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::amountToDiscountChanged, this, &AccountingTAMBill::amountToDiscountChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::amountDiscountedChanged, this, &AccountingTAMBill::amountDiscountedChanged );
    connect( m_d->rootItem, &AccountingTAMBillItem::totalAmountChanged, this, &AccountingTAMBill::totalAmountChanged );

    connect( m_d->rootItem, &AccountingTAMBillItem::itemChanged, this, &AccountingTAMBill::modelChanged );

    connect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingTAMBill::modelChanged );
}

AccountingTAMBill::~AccountingTAMBill(){
    disconnect( m_d->rootItem, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)>(&AccountingBillItem::dataChanged), this, &AccountingTAMBill::updateValue );

    disconnect( m_d->rootItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBill::totalAmountToDiscountChanged );
    disconnect( m_d->rootItem, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBill::amountNotToDiscountChanged );
    disconnect( m_d->rootItem, &AccountingTAMBillItem::amountToDiscountChanged, this, &AccountingTAMBill::amountToDiscountChanged );
    disconnect( m_d->rootItem, &AccountingTAMBillItem::amountDiscountedChanged, this, &AccountingTAMBill::amountDiscountedChanged );
    disconnect( m_d->rootItem, &AccountingTAMBillItem::totalAmountChanged, this, &AccountingTAMBill::totalAmountChanged );

    disconnect( m_d->rootItem, &AccountingBillItem::itemChanged, this, &AccountingTAMBill::modelChanged );

    disconnect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingTAMBill::modelChanged );

    emit aboutToBeDeleted();

    delete m_d;
}

AccountingTAMBill &AccountingTAMBill::operator=(const AccountingTAMBill &cp) {
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    setPriceList( cp.m_d->priceList );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
    return *this;
}

QString AccountingTAMBill::name() {
    return m_d->name;
}

void AccountingTAMBill::setName(const QString &value) {
    if( m_d->name != value ){
        m_d->name = value;
        emit nameChanged( m_d->name );
        if( m_parentItem ){
            m_parentItem->setDataChanged( 0, this );
        }
    }
}

QString AccountingTAMBill::description() {
    return m_d->description;
}

void AccountingTAMBill::setDescription(const QString &value) {
    if( m_d->description != value ){
        m_d->description = value;
        emit descriptionChanged( value );
    }
}

void AccountingTAMBill::setPriceDataSet(int v) {
    int effV = 0;
    if( m_d->priceList != NULL ){
        if( v > -1 && v < m_d->priceList->priceDataSetCount() &&
                m_d->rootItem->currentPriceDataSet() != v ){
            effV = v;
        }
    }
    m_d->rootItem->setCurrentPriceDataSet( effV );
}

ProjectItem *AccountingTAMBill::child(int /*number*/) {
    return NULL;
}

int AccountingTAMBill::childCount() const {
    return 0;
}

int AccountingTAMBill::childNumber(ProjectItem * /*item*/) {
    return -1;
}

bool AccountingTAMBill::reset() {
    return m_d->rootItem->clear();
}

bool AccountingTAMBill::canChildrenBeInserted() {
    return false;
}

bool AccountingTAMBill::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool AccountingTAMBill::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool AccountingTAMBill::clear() {
    beginResetModel();
    bool ret = m_d->rootItem->clear();
    setPriceDataSet( 0 );
    setPriceList( NULL );
    endResetModel();
    return ret;
}

Qt::ItemFlags AccountingTAMBill::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant AccountingTAMBill::data() const {
    return QVariant( m_d->name );
}

bool AccountingTAMBill::setData(const QVariant &value) {
    if( m_d->name != value ){
        m_d->name = value.toString();
        return true;
    } return false;
}

QList<int> AccountingTAMBill::totalAmountPriceFields() {
    return m_d->rootItem->totalAmountPriceFields();
}

void AccountingTAMBill::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    m_d->rootItem->setTotalAmountPriceFields( newAmountFields );
}

AccountingPriceFieldModel *AccountingTAMBill::totalAmountPriceFieldModel() {
    return m_d->rootItem->totalAmountPriceFieldModel();
}

QList<int> AccountingTAMBill::noDiscountAmountPriceFields() {
    return m_d->rootItem->noDiscountAmountPriceFields();
}

void AccountingTAMBill::setNoDiscountAmountPriceFields(const QList<int> &newAmountFields) {
    m_d->rootItem->setNoDiscountAmountPriceFields( newAmountFields );
}

AccountingPriceFieldModel *AccountingTAMBill::noDiscountAmountPriceFieldModel() {
    return m_d->rootItem->noDiscountAmountPriceFieldModel();
}

void AccountingTAMBill::setPriceList(PriceList *pl, AccountingTAMBill::SetPriceListMode plMode) {
    if( pl != m_d->priceList ){
        if( plMode != None ){
            if( m_d->priceList != NULL ){
                if( plMode == SearchAndAdd ){
                    // cerca in base al codice e aggiunge se manca
                    QList<AccountingBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        PriceItem * newPriceItem = NULL;
                        if( (*i)->priceItem()!= NULL ){
                            newPriceItem = pl->priceItemCode( (*i)->priceItem()->code() );
                            if( newPriceItem == NULL ){
                                newPriceItem = pl->appendPriceItem();
                                AccountingTAMBillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
                                *newPriceItem = *((*i)->priceItem());
                            }
                        }
                        (*i)->setPriceItem( newPriceItem );
                    }
                } else if( plMode == Add ){
                    // aggiunge sempre e comunque
                    QList<AccountingBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        PriceItem * newPriceItem = NULL;
                        if( (*i)->priceItem()!= NULL ){
                            newPriceItem = pl->appendPriceItem();
                            AccountingTAMBillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
                            *newPriceItem = *((*i)->priceItem());
                        }
                        (*i)->setPriceItem( newPriceItem );
                    }
                } else if( plMode == Search ){
                    // cerca
                    QList<AccountingBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        (*i)->setPriceItem( pl->priceItemCode( (*i)->priceItem()->code() ) );
                    }
                } else if( plMode == NULLPriceItem ){
                    // annulla
                    QList<AccountingBillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<AccountingBillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
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

PriceList *AccountingTAMBill::priceList() {
    return m_d->priceList;
}

int AccountingTAMBill::priceDataSet() {
    return m_d->rootItem->currentPriceDataSet();
}

double AccountingTAMBill::discount() {
    return m_d->rootItem->discount();
}

void AccountingTAMBill::setDiscount(double newVal) {
    m_d->rootItem->setDiscount( newVal );
}

QList<AccountingTAMBillItem *> AccountingTAMBill::bills() {
    QList<AccountingTAMBillItem *> ret;
    for( int i=0; i < m_d->rootItem->childrenCount(); i++ ){
        ret << dynamic_cast<AccountingTAMBillItem *>(m_d->rootItem->childItem( i ));
    }
    return ret;
}


AccountingTAMBillItem *AccountingTAMBill::item(const QModelIndex &index ) const {
    if (index.isValid()) {
        return static_cast<AccountingTAMBillItem *>(index.internalPointer());
    }
    return m_d->rootItem;
}

AccountingTAMBillItem *AccountingTAMBill::item(int childNum, const QModelIndex &parentIndex ) {
    if (parentIndex.isValid()) {
        AccountingTAMBillItem * parentItem = static_cast<AccountingTAMBillItem *>(parentIndex.internalPointer());
        if( childNum > -1 && childNum < parentItem->childrenCount() ){
            return dynamic_cast<AccountingTAMBillItem *>(parentItem->childItem( childNum ));
        }
    }
    return m_d->rootItem;
}

AccountingTAMBillItem *AccountingTAMBill::lastAccountingMeasure(const QModelIndex &parentIndex) {
    if (parentIndex.isValid()) {
        AccountingTAMBillItem * parentItem = static_cast<AccountingTAMBillItem *>(parentIndex.internalPointer());
        return dynamic_cast<AccountingTAMBillItem *>(parentItem->childItem( parentItem->childrenCount()-1 ));
    }
    return m_d->rootItem;
}

AccountingTAMBillItem *AccountingTAMBill::itemId(unsigned int itemId) {
    return dynamic_cast<AccountingTAMBillItem *>(m_d->rootItem->itemId( itemId ));
}

QVariant AccountingTAMBill::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    AccountingTAMBillItem *i = item(index);

    return i->data(index.column(), role);
}

QVariant AccountingTAMBill::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal )
        return m_d->rootItem->data(section, role);
    else if (orientation == Qt::Vertical )
        return QVariant( section + 1 );
    return QVariant();
}

int AccountingTAMBill::rowCount(const QModelIndex &parent) const {
    AccountingTAMBillItem *parentItem = item(parent);
    return parentItem->childrenCount();
}

int AccountingTAMBill::columnCount(const QModelIndex &) const {
    return m_d->rootItem->columnCount();
}

Qt::ItemFlags AccountingTAMBill::flags(const QModelIndex &index) const {
    AccountingTAMBillItem *b = item(index);
    if( b ){
        return (b->flags(  index.column() ) );
    } else {
        return QAbstractItemModel::flags(index);
    }
}

bool AccountingTAMBill::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;

    AccountingTAMBillItem *b = item(index);
    bool result = b->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool AccountingTAMBill::insertItems(AccountingBillItem::ItemType mt, int inputPos, int count, const QModelIndex &parent) {
    AccountingTAMBillItem *parentItem = item(parent);

    int position = inputPos;
    if( position == -1 ){
        position = parentItem->childrenCount();
    }

    bool success = false;

    beginInsertRows(parent, position, position + count - 1);
    success = parentItem->insertChildren( mt, position, count );
    endInsertRows();

    m_d->rootItem->updateProgressiveCode();

    return success;
}

bool AccountingTAMBill::removeItems(int position, int rows, const QModelIndex &parent) {
    AccountingTAMBillItem *parentItem = item(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    if( success ){
        m_d->rootItem->updateProgressiveCode();
    }

    return success;
}

QModelIndex AccountingTAMBill::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    AccountingTAMBillItem *childItem = item(index);
    AccountingTAMBillItem *parentItem = dynamic_cast<AccountingTAMBillItem *>( childItem->parent());

    if (parentItem == m_d->rootItem || parentItem == 0 )
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

QModelIndex AccountingTAMBill::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    AccountingTAMBillItem *parentItem = dynamic_cast<AccountingTAMBillItem *>(item(parent));

    if( parentItem ){
        AccountingTAMBillItem *childItem = dynamic_cast<AccountingTAMBillItem *>(parentItem->child(row));
        if (childItem)
            return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex AccountingTAMBill::index(AccountingTAMBillItem *item, int column) const {
    if (item == NULL )
        return QModelIndex();

    if( item->parent() == NULL ){
        return QModelIndex();
    } else {
        return createIndex(item->childNumber(), column, item);
    }
}

bool AccountingTAMBill::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow ) {

    if(beginMoveRows( sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow )){
        AccountingTAMBillItem * srcParent = item( sourceParent );
        AccountingTAMBillItem * dstParent = item( destinationParent );
        for( int i=0; i<count; ++i ){
            srcParent->childItem( (sourceRow+count-1)-i )->setParent( dstParent, destinationRow );
        }

        endMoveRows();
        return true;
    }
    return false;
}

void AccountingTAMBill::updateValue(AccountingBillItem * item, int column) {
    QModelIndex i = index( dynamic_cast<AccountingTAMBillItem *>(item), column);
    emit dataChanged( i, i);
}

bool AccountingTAMBill::isUsingPriceItem(PriceItem *p) {
    return m_d->rootItem->isUsingPriceItem( p );
}

bool AccountingTAMBill::isUsingPriceList(PriceList *pl) {
    return m_d->priceList == pl;
}

double AccountingTAMBill::totalAmountToDiscount() const {
    return m_d->rootItem->totalAmountToDiscount();
}

double AccountingTAMBill::amountNotToDiscount() const {
    return m_d->rootItem->amountNotToDiscount();
}

double AccountingTAMBill::amountToDiscount() const {
    return m_d->rootItem->amountToDiscount();
}

double AccountingTAMBill::amountDiscounted() const {
    return m_d->rootItem->amountDiscounted();
}

double AccountingTAMBill::totalAmount() const {
    return m_d->rootItem->totalAmount();
}

QString AccountingTAMBill::totalAmountToDiscountStr() const {
    return m_d->rootItem->totalAmountToDiscountStr();
}

QString AccountingTAMBill::amountNotToDiscountStr() const {
    return m_d->rootItem->amountNotToDiscountStr();
}

QString AccountingTAMBill::amountToDiscountStr() const {
    return m_d->rootItem->amountToDiscountStr();
}

QString AccountingTAMBill::amountDiscountedStr() const {
    return m_d->rootItem->amountDiscountedStr();
}

QString AccountingTAMBill::totalAmountStr() const {
    return m_d->rootItem->totalAmountStr();
}

AttributeModel *AccountingTAMBill::attributeModel() {
    return m_d->attributeModel;
}

double AccountingTAMBill::totalAmountToDiscountAttribute(Attribute *attr) const {
    return m_d->rootItem->totalAmountToDiscountAttribute(attr);
}

QString AccountingTAMBill::totalAmountToDiscountAttributeStr(Attribute *attr) const {
    return m_d->rootItem->totalAmountToDiscountAttributeStr(attr);
}

double AccountingTAMBill::amountNotToDiscountAttribute(Attribute *attr) const {
    return m_d->rootItem->amountNotToDiscountAttribute(attr);
}

QString AccountingTAMBill::amountNotToDiscountAttributeStr(Attribute *attr) const {
    return m_d->rootItem->amountNotToDiscountAttributeStr(attr);
}

double AccountingTAMBill::totalAmountAttribute(Attribute *attr) const {
    return m_d->rootItem->totalAmountAttribute(attr);
}

QString AccountingTAMBill::totalAmountAttributeStr(Attribute *attr) const {
    return m_d->rootItem->totalAmountAttributeStr(attr);
}

void AccountingTAMBill::nextId() {
    m_d->id++;
}

unsigned int AccountingTAMBill::id() {
    return m_d->id;
}

void AccountingTAMBill::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingTAMBill" );
    writer->writeAttribute( "id", QString::number(m_d->id) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeAttribute( "description", m_d->description );
    if( m_d->priceList ){
        writer->writeAttribute( "priceList", QString::number( m_d->priceList->id() ) );
    }
    writer->writeAttribute( "priceDataSet", QString::number( m_d->rootItem->currentPriceDataSet() ) );
    writer->writeAttribute( "discount", QString::number( m_d->rootItem->discount() ) );

    m_d->attributeModel->writeXml( writer );

    m_d->rootItem->writeXml( writer );

    writer->writeEndElement();
}

void AccountingTAMBill::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "ACCOUNTINGTAMBILL"){
        loadFromXml( reader->attributes(), priceLists );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGTAMBILL") ){
        reader->readNext();
        QString tag = reader->name().toString().toUpper();
        if( tag == "ACCOUNTINGATTRIBUTEMODEL" && reader->isStartElement()) {
            m_d->attributeModel->readXml( reader );
        }
        if( tag == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXml( reader, m_d->priceList, m_d->attributeModel );
        }
    }
    m_d->rootItem->updateProgressiveCode();
}

void AccountingTAMBill::readXmlTmp(QXmlStreamReader *reader ) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "ACCOUNTINGTAMBILL"){
        loadFromXmlTmp( reader->attributes() );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGTAMBILL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXmlTmp( reader );
        }
    }
}

void AccountingTAMBill::loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists) {
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
        if( nameUp == "DATEBEGIN" ){
            m_d->rootItem->setDateBegin( (*i).value().toString() );
        }
        if( nameUp == "DATEEND" ){
            m_d->rootItem->setDateEnd( (*i).value().toString() );
        }
    }
}

void AccountingTAMBill::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
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
        if( nameUp == "DATEBEGIN" ){
            m_d->rootItem->setDateBegin( (*i).value().toString() );
        }
        if( nameUp == "DATEEND" ){
            m_d->rootItem->setDateEnd( (*i).value().toString() );
        }
    }
}

void AccountingTAMBill::setTmpData( ProjectPriceListParentItem * priceLists ) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
}

QList<PriceItem *> AccountingTAMBill::connectedPriceItems() {
    return m_d->rootItem->connectedPriceItems();
}

void AccountingTAMBill::writeODTAccountingOnTable(QTextCursor *cursor,
                                                  AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                  AccountingPrinter::PrintPPUDescOption prItemsOption) const {
    m_d->rootItem->writeODTAccountingOnTable(cursor, prAmountsOption, prItemsOption );
}

void AccountingTAMBill::writeODTAttributeAccountingOnTable(QTextCursor *cursor,
                                                           AccountingPrinter::AttributePrintOption prOption,
                                                           AccountingPrinter::PrintAmountsOption printAmountsOption,
                                                           AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                           const QList<Attribute *> &attrsToPrint) const {
    m_d->rootItem->writeODTAttributeAccountingOnTable( cursor, prOption, printAmountsOption, prItemsOption, attrsToPrint );
}


void AccountingTAMBill::writeODTSummaryOnTable(QTextCursor *cursor,
                                               AccountingPrinter::PrintAmountsOption prAmountsOption,
                                               AccountingPrinter::PrintPPUDescOption prItemsOption,
                                               bool writeDetails ) const {
    m_d->rootItem->writeODTSummaryOnTable( cursor, prAmountsOption, prItemsOption, writeDetails );
}

void AccountingTAMBill::loadTmpData(ProjectPriceListParentItem * priceLists) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
    m_d->rootItem->loadTmpData( NULL, NULL, m_d->priceList, m_d->attributeModel );
}

void AccountingTAMBill::insertStandardAttributes(){
    m_d->attributeModel->insertStandardAttributes();
}
