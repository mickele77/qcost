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

#include "accountingbill.h"

#include "attributemodel.h"
#include "accountingbillitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "projectpricelistparentitem.h"
#include "pricefieldmodel.h"

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QVariant>
#include <QString>

#include <QDebug>

class AccountingBillPrivate{
public:
    AccountingBillPrivate( const QString &n, AccountingBill * b, PriceFieldModel * pfm, MathParser * prs = NULL ):
        id(0),
        name(n),
        priceFieldModel(pfm),
        parser(prs),
        rootItem(new AccountingBillItem( NULL, AccountingBillItem::Root, pfm, parser )),
        priceList( NULL ),
        attributeModel( new AttributeModel( b, parser, pfm )),
        priceListIdTmp(0){
    }
    ~AccountingBillPrivate(){
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
    AccountingBillItem * rootItem;
    PriceList * priceList;
    AttributeModel * attributeModel;
    unsigned int priceListIdTmp;
};

AccountingBill::AccountingBill( const QString &n, ProjectItem *parent, PriceFieldModel * pfm, MathParser * parser ):
    QAbstractItemModel(),
    ProjectItem(parent),
    m_d( new AccountingBillPrivate( n, this, pfm, parser ) ) {
    connect( m_d->rootItem, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)>(&AccountingBillItem::dataChanged), this, &AccountingBill::updateValue );

    connect( m_d->rootItem, &AccountingBillItem::discountChanged, this, &AccountingBill::discountChanged );
    connect( m_d->rootItem, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBill::totalAmountToDiscountChanged );
    connect( m_d->rootItem, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBill::amountNotToDiscountChanged );
    connect( m_d->rootItem, &AccountingBillItem::amountToDiscountChanged, this, &AccountingBill::amountToDiscountChanged );
    connect( m_d->rootItem, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBill::amountDiscountedChanged );
    connect( m_d->rootItem, &AccountingBillItem::totalAmountChanged, this, &AccountingBill::totalAmountChanged );

    connect( m_d->rootItem, &AccountingBillItem::itemChanged, this, &AccountingBill::modelChanged );

    connect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingBill::modelChanged );
}

AccountingBill::AccountingBill(AccountingBill & b):
    QAbstractItemModel(),
    ProjectItem( b.ProjectItem::parentItem() ),
    m_d( new AccountingBillPrivate( "", this, b.m_d->priceFieldModel, b.m_d->parser ) ) {

    *this = b;

    connect( m_d->rootItem, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)>(&AccountingBillItem::dataChanged), this, &AccountingBill::updateValue );

    connect( m_d->rootItem, &AccountingBillItem::discountChanged, this, &AccountingBill::discountChanged );
    connect( m_d->rootItem, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBill::totalAmountToDiscountChanged );
    connect( m_d->rootItem, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBill::amountNotToDiscountChanged );
    connect( m_d->rootItem, &AccountingBillItem::amountToDiscountChanged, this, &AccountingBill::amountToDiscountChanged );
    connect( m_d->rootItem, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBill::amountDiscountedChanged );
    connect( m_d->rootItem, &AccountingBillItem::totalAmountChanged, this, &AccountingBill::totalAmountChanged );

    connect( m_d->rootItem, &AccountingBillItem::itemChanged, this, &AccountingBill::modelChanged );

    connect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingBill::modelChanged );
}

AccountingBill::~AccountingBill(){
    disconnect( m_d->rootItem, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)>(&AccountingBillItem::dataChanged), this, &AccountingBill::updateValue );

    disconnect( m_d->rootItem, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBill::totalAmountToDiscountChanged );
    disconnect( m_d->rootItem, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBill::amountNotToDiscountChanged );
    disconnect( m_d->rootItem, &AccountingBillItem::amountToDiscountChanged, this, &AccountingBill::amountToDiscountChanged );
    disconnect( m_d->rootItem, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBill::amountDiscountedChanged );
    disconnect( m_d->rootItem, &AccountingBillItem::totalAmountChanged, this, &AccountingBill::totalAmountChanged );

    disconnect( m_d->rootItem, &AccountingBillItem::itemChanged, this, &AccountingBill::modelChanged );

    disconnect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingBill::modelChanged );

    emit aboutToBeDeleted();

    delete m_d;
}

AccountingBill &AccountingBill::operator=(const AccountingBill &cp) {
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    setPriceList( cp.m_d->priceList );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
    return *this;
}

QString AccountingBill::name() {
    return m_d->name;
}

void AccountingBill::setName(const QString &value) {
    if( m_d->name != value ){
        m_d->name = value;
        emit nameChanged( m_d->name );
        if( m_parentItem ){
            m_parentItem->setDataChanged( 0, this );
        }
    }
}

QString AccountingBill::description() {
    return m_d->description;
}

void AccountingBill::setDescription(const QString &value) {
    if( m_d->description != value ){
        m_d->description = value;
        emit descriptionChanged( value );
    }
}

void AccountingBill::setPriceDataSet(int v) {
    if( v > -1 && v < m_d->priceList->priceDataSetCount() &&
            m_d->rootItem->currentPriceDataSet() != v ){
        m_d->rootItem->setCurrentPriceDataSet( v );
    }
}

ProjectItem *AccountingBill::child(int /*number*/) {
    return NULL;
}

int AccountingBill::childCount() const {
    return 0;
}

int AccountingBill::childNumber(ProjectItem * /*item*/) {
    return -1;
}

bool AccountingBill::clear() {
    bool ret = m_d->rootItem->clear();
    m_d->rootItem->setCurrentPriceDataSet( 0 );
    setName("");
    setPriceList( NULL );
    return ret;
}

bool AccountingBill::canChildrenBeInserted() {
    return false;
}

bool AccountingBill::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool AccountingBill::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

Qt::ItemFlags AccountingBill::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant AccountingBill::data() const {
    return QVariant( m_d->name );
}

bool AccountingBill::setData(const QVariant &value) {
    if( m_d->name != value ){
        m_d->name = value.toString();
        return true;
    } return false;
}

QList<int> AccountingBill::totalAmountPriceFields() {
    return m_d->rootItem->totalAmountPriceFields();
}

void AccountingBill::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    m_d->rootItem->setTotalAmountPriceFields( newAmountFields );
}

AccountingPriceFieldModel *AccountingBill::totalAmountPriceFieldModel() {
    return m_d->rootItem->totalAmountPriceFieldModel();
}

QList<int> AccountingBill::noDiscountAmountPriceFields() {
    return m_d->rootItem->noDiscountAmountPriceFields();
}

void AccountingBill::setNoDiscountAmountPriceFields(const QList<int> &newAmountFields) {
    m_d->rootItem->setNoDiscountAmountPriceFields( newAmountFields );
}

AccountingPriceFieldModel *AccountingBill::noDiscountAmountPriceFieldModel() {
    return m_d->rootItem->noDiscountAmountPriceFieldModel();
}

void AccountingBill::setPriceList(PriceList *pl, AccountingBill::SetPriceListMode plMode) {
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
                                AccountingBillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
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
                            AccountingBillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
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

PriceList *AccountingBill::priceList() {
    return m_d->priceList;
}

int AccountingBill::priceDataSet() {
    return m_d->rootItem->currentPriceDataSet();
}

double AccountingBill::discount() {
    return m_d->rootItem->discount();
}

QString AccountingBill::discountStr() {
    return m_d->rootItem->discountStr();
}

void AccountingBill::setDiscount(double newVal) {
    m_d->rootItem->setDiscount( newVal );
}

void AccountingBill::setDiscount( const QString & newVal) {
    m_d->rootItem->setDiscount( newVal );
}

AccountingBillItem *AccountingBill::item(const QModelIndex &index ) const {
    if (index.isValid()) {
        return static_cast<AccountingBillItem *>(index.internalPointer());
    }
    return m_d->rootItem;
}

AccountingBillItem *AccountingBill::item(int childNum, const QModelIndex &parentIndex ) {
    if (parentIndex.isValid()) {
        AccountingBillItem * parentItem = static_cast<AccountingBillItem *>(parentIndex.internalPointer());
        if( childNum > -1 && childNum < parentItem->childrenCount() ){
            return parentItem->childItem( childNum );
        }
    }
    if( childNum > -1 && childNum < m_d->rootItem->childrenCount() ){
        return m_d->rootItem->childItem( childNum );
    }
    return m_d->rootItem;
}

AccountingBillItem *AccountingBill::lastItem(const QModelIndex &parentIndex) {
    if (parentIndex.isValid()) {
        AccountingBillItem * parentItem = static_cast<AccountingBillItem *>(parentIndex.internalPointer());
        return parentItem->childItem( parentItem->childrenCount()-1 );
    }
    return m_d->rootItem;
}

AccountingBillItem *AccountingBill::itemId(unsigned int itemId) {
    return m_d->rootItem->itemId( itemId );
}

QVariant AccountingBill::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    AccountingBillItem *i = item(index);

    return i->data(index.column(), role);
}

QVariant AccountingBill::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal )
        return m_d->rootItem->data(section, role);
    else if (orientation == Qt::Vertical )
        return QVariant( section + 1 );
    return QVariant();
}

bool AccountingBill::hasChildren(const QModelIndex & parent ) const {
    AccountingBillItem *parentItem = item(parent);
    return parentItem->hasChildren();
}

int AccountingBill::rowCount(const QModelIndex &parent) const {
    AccountingBillItem *parentItem = item(parent);
    return parentItem->childrenCount();
}

int AccountingBill::columnCount(const QModelIndex &) const {
    return m_d->rootItem->columnCount();
}

Qt::ItemFlags AccountingBill::flags(const QModelIndex &index) const {
    return QAbstractItemModel::flags(index);

    AccountingBillItem *b = item(index);
    if( b != NULL ){
        return (b->flags(  index.column() ) );
    } else {
        return QAbstractItemModel::flags(index);
    }
}

bool AccountingBill::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;

    AccountingBillItem *b = item(index);
    bool result = b->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool AccountingBill::insertPayments( int inputPos, int count ) {
    int position = inputPos;
    if( position == -1 ){
        position = m_d->rootItem->childrenCount();
    }

    bool success = false;

    beginInsertRows(QModelIndex(), position, position + count - 1);
    success = m_d->rootItem->insertChildren( AccountingBillItem::Payment, position, count );
    endInsertRows();

    if( success ){
        for( int i=position; i < position+count; ++i ){
            connect( m_d->rootItem->childItem( i ), &AccountingBillItem::requestDateBeginChangeSignal, this, &AccountingBill::requestDateBeginChange );
            connect( m_d->rootItem->childItem( i ), &AccountingBillItem::requestDateEndChangeSignal, this, &AccountingBill::requestDateEndChange );
        }
        m_d->rootItem->updateProgressiveCode();
    }

    return success;
}

bool AccountingBill::insertItems(AccountingBillItem::ItemType mt, int inputPos, int count, const QModelIndex &parent) {
    AccountingBillItem *parentItem = item(parent);

    int position = inputPos;
    if( position == -1 ){
        position = parentItem->childrenCount();
    }

    bool success = false;

    if( mt == AccountingBillItem::Payment ){
        emit requestInsertBills( position, count );
        success = true;
    } else {
        beginInsertRows(parent, position, position + count - 1);
        success = parentItem->insertChildren( mt, position, count );
        endInsertRows();
    }

    if( success ){
        m_d->rootItem->updateProgressiveCode();
    }

    return success;
}

bool AccountingBill::removePayments(int position, int rows) {
    bool success = false;

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    success = m_d->rootItem->removeChildren(position, rows);
    endRemoveRows();

    m_d->rootItem->updateProgressiveCode();

    return success;
}

bool AccountingBill::removeItems(int position, int rows, const QModelIndex &parent) {
    AccountingBillItem *parentItem = item(parent);
    bool success = false;

    for( int row=(position+rows-1); row >= position ; --row){
        AccountingBillItem *item = parentItem->childItem( row );
        if( item->itemType() == AccountingBillItem::Payment ){
            emit requestRemoveBills( row, 1 );
            success = true;
        } else {
            beginRemoveRows(parent, row, row);
            success = parentItem->removeChildren(row, 1);
            endRemoveRows();
        }
    }

    if( success ){
        m_d->rootItem->updateProgressiveCode();
    }

    return success;
}

QModelIndex AccountingBill::parent(const QModelIndex &index) const {
    if ( !index.isValid() )
        return QModelIndex();

    AccountingBillItem *childItem = item(index);
    AccountingBillItem *parentItem = childItem->parent();

    if (parentItem == m_d->rootItem || parentItem == NULL )
        return QModelIndex();

    return createIndex( parentItem->childNumber(), 0, parentItem );
}

QModelIndex AccountingBill::index(int row, int col, const QModelIndex &parent) const {
    if( parent.isValid() && parent.column() != 0)
        return QModelIndex();

    AccountingBillItem *parentItem = item(parent);

    if( parentItem ){
        AccountingBillItem *childItem = dynamic_cast<AccountingBillItem *>(parentItem->child(row));
        if (childItem)
            return createIndex(row, col, childItem);
    }
    return QModelIndex();
}

QModelIndex AccountingBill::index(AccountingBillItem *item, int column) const {
    if (item == NULL )
        return QModelIndex();

    if( item->parent() == NULL ){
        return QModelIndex();
    } else {
        return createIndex(item->childNumber(), column, item);
    }
}

bool AccountingBill::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow ) {

    if(beginMoveRows( sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow )){
        AccountingBillItem * srcParent = item( sourceParent );
        AccountingBillItem * dstParent = item( destinationParent );
        for( int i=0; i<count; ++i ){
            srcParent->childItem( (sourceRow+count-1)-i )->setParent( dstParent, destinationRow );
        }

        endMoveRows();
        return true;
    }
    return false;
}

double AccountingBill::totalAmountToDiscountAttribute(Attribute *attr) const {
    return m_d->rootItem->totalAmountToDiscountAttribute(attr);
}

QString AccountingBill::totalAmountToDiscountAttributeStr(Attribute *attr) const {
    return m_d->rootItem->totalAmountToDiscountAttributeStr(attr);
}

double AccountingBill::amountNotToDiscountAttribute(Attribute *attr) const {
    return m_d->rootItem->amountNotToDiscountAttribute(attr);
}

QString AccountingBill::amountNotToDiscountAttributeStr(Attribute *attr) const {
    return m_d->rootItem->amountNotToDiscountAttributeStr(attr);
}

double AccountingBill::totalAmountAttribute(Attribute *attr) const {
    return m_d->rootItem->totalAmountAttribute(attr);
}

QString AccountingBill::totalAmountAttributeStr(Attribute *attr) const {
    return m_d->rootItem->totalAmountAttributeStr(attr);
}

double AccountingBill::totalAmountToDiscount() const {
    return m_d->rootItem->totalAmountToDiscount();
}

double AccountingBill::amountNotToDiscount() const {
    return m_d->rootItem->amountNotToDiscount();
}

double AccountingBill::amountToDiscount() const {
    return m_d->rootItem->amountToDiscount();
}

double AccountingBill::amountDiscounted() const {
    return m_d->rootItem->amountDiscounted();
}

double AccountingBill::totalAmount() const {
    return m_d->rootItem->totalAmount();
}

QString AccountingBill::totalAmountToDiscountStr() const {
    return m_d->rootItem->totalAmountToDiscountStr();
}

QString AccountingBill::amountNotToDiscountStr() const {
    return m_d->rootItem->amountNotToDiscountStr();
}

QString AccountingBill::amountToDiscountStr() const {
    return m_d->rootItem->amountToDiscountStr();
}

QString AccountingBill::amountDiscountedStr() const {
    return m_d->rootItem->amountDiscountedStr();
}

QString AccountingBill::totalAmountStr() const {
    return m_d->rootItem->totalAmountStr();
}

AttributeModel *AccountingBill::attributeModel() {
    return m_d->attributeModel;
}

void AccountingBill::updateValue(AccountingBillItem * item, int column) {
    QModelIndex i = index( item, column);
    emit dataChanged( i, i);
}

bool AccountingBill::isUsingPriceItem(PriceItem *p) {
    return m_d->rootItem->isUsingPriceItem( p );
}

bool AccountingBill::isUsingPriceList(PriceList *pl) {
    return m_d->priceList == pl;
}

void AccountingBill::setBillDateEnd(const QDate &newDate, int position) {
    AccountingBillItem * item = m_d->rootItem->childItem(position);
    if( item != NULL ){
        item->setDateEnd( newDate );
    }
}

void AccountingBill::setBillDateBegin(const QDate &newDate, int position) {
    AccountingBillItem * item = m_d->rootItem->childItem(position);
    if( item != NULL ){
        item->setDateBegin( newDate );
    }
}

void AccountingBill::nextId() {
    m_d->id++;
}

unsigned int AccountingBill::id() {
    return m_d->id;
}

void AccountingBill::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingBill" );
    writer->writeAttribute( "id", QString::number(m_d->id) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeAttribute( "discount", QString::number(m_d->rootItem->discount()) );

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

    fields.clear();
    fieldList = m_d->rootItem->noDiscountAmountPriceFields();
    i= fieldList.begin();
    if( i != fieldList.end() ){
        fields += QString::number((*i));
        ++i;
    }
    for( ; i != fieldList.end(); ++i ){
        fields += ", " + QString::number((*i));
    }
    writer->writeAttribute( "noDiscountAmountPriceFields", fields );

    if( m_d->priceList ){
        writer->writeAttribute( "priceList", QString::number( m_d->priceList->id() ) );
    }
    writer->writeAttribute( "priceDataSet", QString::number( m_d->rootItem->currentPriceDataSet() ) );

    m_d->attributeModel->writeXml( writer );

    m_d->rootItem->writeXml( writer );

    writer->writeEndElement();
}

void AccountingBill::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "ACCOUNTINGBILL"){
        loadFromXml( reader->attributes(), priceLists );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGBILL") ){
        reader->readNext();
        QString tag = reader->name().toString().toUpper();
        if( tag == "ATTRIBUTEMODEL" && reader->isStartElement()) {
            m_d->attributeModel->readXml( reader );
        }
        if( tag == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXml( reader, m_d->priceList, m_d->attributeModel );
        }
    }
    m_d->rootItem->updateProgressiveCode();
}

void AccountingBill::loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists) {
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
        if( nameUp == "DISCOUNT" ){
            m_d->rootItem->setDiscount( (*i).value().toDouble() );
        }
        if( nameUp == "TOTALAMOUNTPRICEFIELDS" ){
            QStringList pFieldsStr = (*i).value().toString().split(",");
            QList<int> pFields;
            for(QStringList::const_iterator pStr = pFieldsStr.constBegin(); pStr != pFieldsStr.constEnd(); pStr++ ){
                bool ok = false;
                int p = (*pStr).toInt( &ok );
                if( ok ){
                    pFields << p;
                }
            }
            m_d->rootItem->setTotalAmountPriceFields( pFields );
        }
        if( nameUp == "NODISCOUNTAMOUNTPRICEFIELDS" ){
            QStringList pFieldsStr = (*i).value().toString().split(",");
            QList<int> pFields;
            for(QStringList::const_iterator pStr = pFieldsStr.constBegin(); pStr != pFieldsStr.constEnd(); pStr++ ){
                bool ok = false;
                int p = (*pStr).toInt( &ok );
                if( ok ){
                    pFields << p;
                }
            }
            m_d->rootItem->setNoDiscountAmountPriceFields( pFields );
        }
    }
}

void AccountingBill::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
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

void AccountingBill::setTmpData( ProjectPriceListParentItem * priceLists ) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
}

QList<PriceItem *> AccountingBill::connectedPriceItems() {
    return m_d->rootItem->connectedPriceItems();
}

void AccountingBill::writeODTAccountingOnTable( QTextCursor *cursor,
                                                AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                AccountingPrinter::PrintPPUDescOption prPPUDescOption ) const {
    m_d->rootItem->writeODTAccountingOnTable(cursor, prAmountsOption, prPPUDescOption );
}

void AccountingBill::writeODTAttributeAccountingOnTable(QTextCursor *cursor,
                                                        AccountingPrinter::AttributePrintOption prOption,
                                                        AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                        AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                        const QList<Attribute *> &attrsToPrint ) const {
    m_d->rootItem->writeODTAttributeAccountingOnTable( cursor, prOption, prAmountsOption, prItemsOption, attrsToPrint );
}


void AccountingBill::writeODTSummaryOnTable( QTextCursor *cursor,
                                             AccountingPrinter::PrintAmountsOption prAmountsOption,
                                             AccountingPrinter::PrintPPUDescOption prItemsOption,
                                             bool writeDetails ) const {
    m_d->rootItem->writeODTSummaryOnTable( cursor, prAmountsOption, prItemsOption, writeDetails );
}
void AccountingBill::loadTmpData(ProjectPriceListParentItem * priceLists, AccountingLSBills * lsBills, AccountingTAMBill * tamBill ) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
    m_d->rootItem->loadTmpData( lsBills, tamBill, m_d->priceList, m_d->attributeModel );
}

void AccountingBill::insertStandardAttributes(){
    m_d->attributeModel->insertStandardAttributes();
}
