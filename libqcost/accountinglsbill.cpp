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

#include "accountinglsbill.h"

#include "billprinter.h"
#include "attributemodel.h"
#include "accountinglsbillitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "projectpricelistparentitem.h"
#include "pricefieldmodel.h"

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QVariant>
#include <QString>

class AccountingLSBillPrivate{
public:
    AccountingLSBillPrivate( const QString &n, AccountingLSBill * b, PriceFieldModel * pfm, MathParser * prs = NULL ):
        id(0),
        name( n ),
        priceFieldModel(pfm),
        parser(prs),
        rootItem(new AccountingLSBillItem( NULL, NULL, pfm, parser )),
        priceList( NULL ),
        attributeModel( new AttributeModel( b, parser, pfm )),
        priceListIdTmp(0){
    }
    ~AccountingLSBillPrivate(){
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
    AccountingLSBillItem * rootItem;
    PriceList * priceList;
    AttributeModel * attributeModel;
    unsigned int priceListIdTmp;
};

AccountingLSBill::AccountingLSBill(const QString &n, ProjectItem *parent, PriceFieldModel * pfm, MathParser * parser ):
    QAbstractTableModel(),
    ProjectItem(parent),
    m_d( new AccountingLSBillPrivate( n, this, pfm, parser ) ) {
    connect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)>(&AccountingLSBillItem::dataChanged), this, &AccountingLSBill::updateValue );
    connect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(const QString &)>(&AccountingLSBillItem::totalAmountChanged), this, static_cast<void(AccountingLSBill::*)(const QString &)>(&AccountingLSBill::totalAmountChanged) );
    connect( m_d->rootItem, &AccountingLSBillItem::itemChanged, this, &AccountingLSBill::modelChanged );

    connect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingLSBill::modelChanged );
}

AccountingLSBill::AccountingLSBill(AccountingLSBill & b):
    QAbstractTableModel(),
    ProjectItem( b.ProjectItem::parentItem() ),
    m_d( new AccountingLSBillPrivate( b.m_d->name, this, b.m_d->priceFieldModel, b.m_d->parser ) ) {

    *this = b;

    connect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)>(&AccountingLSBillItem::dataChanged), this, &AccountingLSBill::updateValue );
    connect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(const QString &)>(&AccountingLSBillItem::totalAmountChanged), this, static_cast<void(AccountingLSBill::*)(const QString &)>(&AccountingLSBill::totalAmountChanged) );
    connect( m_d->rootItem, &AccountingLSBillItem::itemChanged, this, &AccountingLSBill::modelChanged );

    connect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingLSBill::modelChanged );
}

AccountingLSBill::~AccountingLSBill(){
    disconnect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)>(&AccountingLSBillItem::dataChanged), this, &AccountingLSBill::updateValue );
    disconnect( m_d->rootItem, static_cast<void(AccountingLSBillItem::*)(const QString &)>(&AccountingLSBillItem::totalAmountChanged), this, static_cast<void(AccountingLSBill::*)(const QString &)>(&AccountingLSBill::totalAmountChanged) );
    disconnect( m_d->rootItem, &AccountingLSBillItem::itemChanged, this, &AccountingLSBill::modelChanged );

    disconnect( m_d->attributeModel, &AttributeModel::modelChanged, this, &AccountingLSBill::modelChanged );

    emit aboutToBeDeleted();

    delete m_d;
}

AccountingLSBill &AccountingLSBill::operator=(const AccountingLSBill &cp) {
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    setPriceList( cp.m_d->priceList );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
    return *this;
}

QString AccountingLSBill::name() {
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

QString AccountingLSBill::description() {
    return m_d->description;
}

void AccountingLSBill::setDescription(const QString &value) {
    if( m_d->description != value ){
        m_d->description = value;
        emit descriptionChanged( value );
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

void AccountingLSBill::updateValue(AccountingLSBillItem * item, int column) {
    QModelIndex i = index( item, column);
    emit dataChanged( i, i);
}

bool AccountingLSBill::isUsingPriceItem(PriceItem *p) {
    return m_d->rootItem->isUsingPriceItem( p );
}

bool AccountingLSBill::isUsingPriceList(PriceList *pl) {
    return m_d->priceList == pl;
}

double AccountingLSBill::totalAmount() const {
    return m_d->rootItem->totalAmount();
}

QString AccountingLSBill::totalAmountStr() const {
    return m_d->rootItem->totalAmountStr();
}

double AccountingLSBill::totalAmountAccounted() const {
    return m_d->rootItem->totalAmountAccounted();
}

QString AccountingLSBill::totalAmountAccountedStr() const {
    return m_d->rootItem->totalAmountAccountedStr();
}

double AccountingLSBill::percentageAccounted() const {
    return m_d->rootItem->percentageAccounted();
}

QString AccountingLSBill::percentageAccountedStr() const {
    return m_d->rootItem->percentageAccountedStr();
}

AttributeModel *AccountingLSBill::attributeModel() {
    return m_d->attributeModel;
}

double AccountingLSBill::totalAmountAttribute(Attribute *attr) {
    return m_d->rootItem->totalAmountAttribute( attr );
}

QString AccountingLSBill::totalAmountAttributeStr(Attribute *attr) {
    return m_d->rootItem->totalAmountAttributeStr( attr );
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
    if( m_d->priceList ){
        writer->writeAttribute( "priceList", QString::number( m_d->priceList->id() ) );
    }
    writer->writeAttribute( "priceDataSet", QString::number( m_d->rootItem->currentPriceDataSet() ) );

    m_d->attributeModel->writeXml( writer );

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
            m_d->attributeModel->readXml( reader );
        }
        if( tag == "ACCOUNTINGLSBILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXml( reader, m_d->priceList, m_d->attributeModel );
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

void AccountingLSBill::writeODTBillOnTable( QTextCursor *cursor,
                                            AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                            bool writeAmounts ) {
    m_d->rootItem->writeODTBillOnTable( cursor, prItemsOption, writeAmounts );
}

void AccountingLSBill::writeODTAttributeBillOnTable(QTextCursor *cursor,
                                                    AccountingPrinter::AttributePrintOption prOption,
                                                    AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                    const QList<Attribute *> &attrsToPrint,
                                                    bool writeAmounts) {
    m_d->rootItem->writeODTAttributeBillOnTable( cursor, prOption, prItemsOption, attrsToPrint, writeAmounts );
}


void AccountingLSBill::writeODTSummaryOnTable(QTextCursor *cursor,
                                              AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                              bool writeAmounts,
                                              bool writeDetails ) {
    m_d->rootItem->writeODTSummaryOnTable( cursor, prItemsOption, writeAmounts, writeDetails );
}

void AccountingLSBill::loadTmpData(ProjectPriceListParentItem * priceLists) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
    m_d->rootItem->loadTmpData( m_d->priceList, m_d->attributeModel );
}
