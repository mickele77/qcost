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

#include "bill.h"

#include "billprinter.h"
#include "attributesmodel.h"
#include "varsmodel.h"
#include "billitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "projectpricelistparentitem.h"
#include "pricefieldmodel.h"

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QVariant>
#include <QString>

class BillPrivate{
public:
    BillPrivate( const QString &n, Bill * b, PriceFieldModel * pfm, MathParser * prs = NULL ):
        id(0),
        name( n ),
        priceFieldModel(pfm),
        parser(prs),
        varsModel( new VarsModel( parser )),
        rootItem(new BillItem( NULL, NULL, pfm, parser, varsModel )),
        priceList( NULL ),
        attributesModel( new AttributesModel( b, parser, pfm )),
        priceListIdTmp(0){
    }
    ~BillPrivate(){
        delete rootItem;
        delete attributesModel;
        delete varsModel;
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
    VarsModel * varsModel;
    BillItem * rootItem;
    PriceList * priceList;
    AttributesModel * attributesModel;
    unsigned int priceListIdTmp;
};

Bill::Bill(const QString &n, ProjectItem *parent, PriceFieldModel * pfm, MathParser * parser ):
    QAbstractItemModel(),
    ProjectItem(parent),
    m_d( new BillPrivate( n, this, pfm, parser ) ) {
    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &Bill::insertPriceField );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &Bill::removePriceField );

    connect( m_d->rootItem, static_cast<void(BillItem::*)(BillItem*,int)>(&BillItem::dataChanged), this, &Bill::updateValue );
    connect( m_d->rootItem, static_cast<void(BillItem::*)(int,double)>(&BillItem::amountChanged), this, static_cast<void(Bill::*)(int,double)>(&Bill::amountChanged) );
    connect( m_d->rootItem, static_cast<void(BillItem::*)(int,const QString &)>(&BillItem::amountChanged), this, static_cast<void(Bill::*)(int,const QString &)>(&Bill::amountChanged) );
    connect( m_d->rootItem, &BillItem::itemChanged, this, &Bill::modelChanged );

    connect( m_d->attributesModel, &AttributesModel::modelChanged, this, &Bill::modelChanged );
}

Bill::Bill(Bill & b):
    QAbstractItemModel(),
    ProjectItem( b.ProjectItem::parentItem() ),
    m_d( new BillPrivate( b.m_d->name, this, b.m_d->priceFieldModel, b.m_d->parser ) ) {

    *this = b;

    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &Bill::insertPriceField );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &Bill::removePriceField );

    connect( m_d->rootItem, static_cast<void(BillItem::*)(BillItem*,int)>(&BillItem::dataChanged), this, &Bill::updateValue );
    connect( m_d->rootItem, static_cast<void(BillItem::*)(int,double)>(&BillItem::amountChanged), this, static_cast<void(Bill::*)(int,double)>(&Bill::amountChanged) );
    connect( m_d->rootItem, static_cast<void(BillItem::*)(int,const QString &)>(&BillItem::amountChanged), this, static_cast<void(Bill::*)(int,const QString &)>(&Bill::amountChanged) );
    connect( m_d->rootItem, &BillItem::itemChanged, this, &Bill::modelChanged );

    connect( m_d->attributesModel, &AttributesModel::modelChanged, this, &Bill::modelChanged );
}

Bill::~Bill(){
    disconnect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &Bill::insertPriceField );
    disconnect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &Bill::removePriceField );

    disconnect( m_d->rootItem, static_cast<void(BillItem::*)(BillItem*,int)>(&BillItem::dataChanged), this, &Bill::updateValue );
    disconnect( m_d->rootItem, static_cast<void(BillItem::*)(int,double)>(&BillItem::amountChanged), this, static_cast<void(Bill::*)(int,double)>(&Bill::amountChanged) );
    disconnect( m_d->rootItem, static_cast<void(BillItem::*)(int,const QString &)>(&BillItem::amountChanged), this, static_cast<void(Bill::*)(int,const QString &)>(&Bill::amountChanged) );
    disconnect( m_d->rootItem, &BillItem::itemChanged, this, &Bill::modelChanged );

    disconnect( m_d->attributesModel, &AttributesModel::modelChanged, this, &Bill::modelChanged );

    emit aboutToBeDeleted();

    delete m_d;
}

Bill &Bill::operator=(const Bill &cp) {
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    setPriceList( cp.m_d->priceList );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
    *(m_d->attributesModel) = *(cp.m_d->attributesModel);
    *(m_d->varsModel) = *(cp.m_d->varsModel);
    return *this;
}

QString Bill::name() {
    return m_d->name;
}

void Bill::setName(const QString &value) {
    if( m_d->name != value ){
        m_d->name = value;
        emit nameChanged( m_d->name );
        if( m_parentItem ){
            m_parentItem->setDataChanged( 0, this );
        }
    }
}

QString Bill::description() {
    return m_d->description;
}

void Bill::setDescription(const QString &value) {
    if( m_d->description != value ){
        m_d->description = value;
        emit descriptionChanged( value );
    }
}

void Bill::setPriceDataSet(int v) {
    if( v > -1 && v < m_d->priceList->priceDataSetCount() &&
            m_d->rootItem->currentPriceDataSet() != v ){
        m_d->rootItem->setCurrentPriceDataSet( v );
    }
}

ProjectItem *Bill::child(int /*number*/) {
    return NULL;
}

int Bill::childCount() const {
    return 0;
}

int Bill::childNumber(ProjectItem * /*item*/) {
    return -1;
}

bool Bill::reset() {
    return m_d->rootItem->reset();
}

bool Bill::canChildrenBeInserted() {
    return false;
}

bool Bill::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool Bill::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

Qt::ItemFlags Bill::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant Bill::data() const {
    return QVariant( m_d->name );
}

bool Bill::setData(const QVariant &value) {
    if( m_d->name != value ){
        m_d->name = value.toString();
        return true;
    } return false;
}

void Bill::setPriceList(PriceList *pl, Bill::SetPriceListMode plMode) {
    if( pl != m_d->priceList ){
        if( plMode != None ){
            if( m_d->priceList != NULL ){
                if( plMode == SearchAndAdd ){
                    // cerca in base al codice e aggiunge se manca
                    QList<BillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<BillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        PriceItem * newPriceItem = NULL;
                        if( (*i)->priceItem()!= NULL ){
                            newPriceItem = pl->priceItemCode( (*i)->priceItem()->code() );
                            if( newPriceItem == NULL ){
                                newPriceItem = pl->appendPriceItem();
                                BillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
                                *newPriceItem = *((*i)->priceItem());
                            }
                        }
                        (*i)->setPriceItem( newPriceItem );
                    }
                } else if( plMode == Add ){
                    // aggiunge sempre e comunque
                    QList<BillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<BillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        PriceItem * newPriceItem = NULL;
                        if( (*i)->priceItem()!= NULL ){
                            newPriceItem = pl->appendPriceItem();
                            BillPrivate::setPriceItemParents( pl, (*i)->priceItem(), newPriceItem );
                            *newPriceItem = *((*i)->priceItem());
                        }
                        (*i)->setPriceItem( newPriceItem );
                    }
                } else if( plMode == Search ){
                    // cerca
                    QList<BillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<BillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        (*i)->setPriceItem( pl->priceItemCode( (*i)->priceItem()->code() ) );
                    }
                } else if( plMode == NULLPriceItem ){
                    // annulla
                    QList<BillItem *> allItems = m_d->rootItem->allChildren();
                    for(QList<BillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                        (*i)->setPriceItem( NULL );
                    }
                } else if( plMode == ResetBill ){
                    // resetta il computo
                    removeBillItems( 0, m_d->rootItem->childrenCount() );
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

PriceList *Bill::priceList() {
    return m_d->priceList;
}

int Bill::priceDataSet() {
    return m_d->rootItem->currentPriceDataSet();
}


BillItem *Bill::billItem(const QModelIndex &index ) const {
    if (index.isValid()) {
        return static_cast<BillItem *>(index.internalPointer());
    }
    return m_d->rootItem;
}

BillItem *Bill::billItem(int childNum, const QModelIndex &parentIndex ) {
    if (parentIndex.isValid()) {
        BillItem * parentItem = static_cast<BillItem *>(parentIndex.internalPointer());
        if( childNum > -1 && childNum < parentItem->childrenCount() ){
            return parentItem->childItem( childNum );
        }
    }
    return m_d->rootItem;
}

BillItem *Bill::lastBillItem(const QModelIndex &parentIndex) {
    if (parentIndex.isValid()) {
        BillItem * parentItem = static_cast<BillItem *>(parentIndex.internalPointer());
        return parentItem->childItem( parentItem->childrenCount()-1 );
    }
    return m_d->rootItem;
}

BillItem *Bill::billItemId(unsigned int itemId) {
    return m_d->rootItem->itemFromId( itemId );
}

QVariant Bill::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    BillItem *i = billItem(index);

    return i->data(index.column(), role);
}

QVariant Bill::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal )
        return m_d->rootItem->data(section, role);
    else if (orientation == Qt::Vertical )
        return QVariant( section + 1 );
    return QVariant();
}

int Bill::rowCount(const QModelIndex &parent) const {
    BillItem *parentItem = billItem(parent);
    return parentItem->childrenCount();
}

int Bill::columnCount(const QModelIndex &) const {
    return m_d->rootItem->columnCount();
}

Qt::ItemFlags Bill::flags(const QModelIndex &index) const {
    BillItem *b = billItem(index);
    if( b ){
        return (b->flags(  index.column() ) );
    } else {
        return QAbstractItemModel::flags(index);
    }
}

bool Bill::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;

    BillItem *b = billItem(index);
    bool result = b->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool Bill::insertBillItems(PriceItem *p, int inputPos, int count, const QModelIndex &parent) {
    BillItem *parentItem = billItem(parent);

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

bool Bill::removeBillItems(int position, int rows, const QModelIndex &parent) {
    BillItem *parentItem = billItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void Bill::clear() {
    removeBillItems( 0, m_d->rootItem->childrenCount() );
    m_d->attributesModel->clear();
    m_d->varsModel->clear();
}

QModelIndex Bill::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    BillItem *childItem = billItem(index);
    BillItem *parentItem = dynamic_cast<BillItem *>( childItem->parent());

    if (parentItem == m_d->rootItem || parentItem == 0 )
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

QModelIndex Bill::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    BillItem *parentItem = dynamic_cast<BillItem *>(billItem(parent));

    if( parentItem ){
        BillItem *childItem = dynamic_cast<BillItem *>(parentItem->child(row));
        if (childItem)
            return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex Bill::index(BillItem *item, int column) const {
    if (item == NULL )
        return QModelIndex();

    if( item->parent() == NULL ){
        return QModelIndex();
    } else {
        return createIndex(item->childNumber(), column, item);
    }
}

bool Bill::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow ) {

    if(beginMoveRows( sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow )){
        BillItem * srcParent = billItem( sourceParent );
        BillItem * dstParent = billItem( destinationParent );
        for( int i=0; i<count; ++i ){
            srcParent->childItem( (sourceRow+count-1)-i )->setParent( dstParent, destinationRow );
        }

        endMoveRows();
        return true;
    }
    return false;
}

void Bill::updateValue(BillItem * item, int column) {
    QModelIndex i = index( item, column);
    emit dataChanged( i, i);
}

bool Bill::isUsingPriceItem(PriceItem *p) {
    return m_d->rootItem->isUsingPriceItem( p );
}

bool Bill::isUsingPriceList(PriceList *pl) {
    return m_d->priceList == pl;
}

double Bill::amount(int field) const {
    return m_d->rootItem->amount( field );
}

QString Bill::amountStr(int field) const {
    return m_d->rootItem->amountStr( field );
}

AttributesModel *Bill::attributesModel() {
    return m_d->attributesModel;
}

double Bill::amountAttribute(Attribute *attr, int field) {
    return m_d->rootItem->amountAttribute( attr, field );
}

QString Bill::amountAttributeStr(Attribute *attr, int field) {
    return m_d->rootItem->amountAttributeStr( attr, field );
}

VarsModel *Bill::varsModel() {
    return m_d->varsModel;
}

void Bill::nextId() {
    m_d->id++;
}

unsigned int Bill::id() {
    return m_d->id;
}

void Bill::insertPriceField( int firstPFInserted, int lastPFInserted ){
    for( int pf = firstPFInserted; pf <= lastPFInserted; ++pf ){
        beginInsertColumns( QModelIndex(), m_d->rootItem->firstPriceFieldCol() + pf * 2, m_d->rootItem->firstPriceFieldCol() + pf * 2 + 1 );
        m_d->rootItem->insertAmount(pf);
        endInsertColumns();
    }
}

void Bill::removePriceField( int firstPFRemoved, int lastPFRemoved ){
    for( int pf = firstPFRemoved; pf <= lastPFRemoved; ++pf ){
        beginRemoveColumns( QModelIndex(), m_d->rootItem->firstPriceFieldCol() + pf * 2, m_d->rootItem->firstPriceFieldCol() + pf * 2 + 1 );
        m_d->rootItem->removeAmount(pf);
        endRemoveColumns();
    }
}

void Bill::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "Bill" );
    writer->writeAttribute( "id", QString::number(m_d->id) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeAttribute( "description", m_d->description );
    if( m_d->priceList ){
        writer->writeAttribute( "priceList", QString::number( m_d->priceList->id() ) );
    }
    writer->writeAttribute( "priceDataSet", QString::number( m_d->rootItem->currentPriceDataSet() ) );

    m_d->attributesModel->writeXml( writer );
    m_d->varsModel->writeXml( writer );

    m_d->rootItem->writeXml( writer );

    writer->writeEndElement();
}

void Bill::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "BILL"){
        loadXml( reader->attributes(), priceLists );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL") ){
        reader->readNext();
        if( reader->isStartElement() ){
            QString tag = reader->name().toString().toUpper();
            if( tag == "ATTRIBUTESMODEL" ) {
                m_d->attributesModel->readXml( reader );
            }
            if( tag == "VARSMODEL" ) {
                m_d->varsModel->readXml( reader );
            }
            if( tag == "BILLITEM" ) {
                m_d->rootItem->readXmlTmp( reader );
            }
        }
    }
    m_d->rootItem->readFromXmlTmp( m_d->priceList, m_d->attributesModel );
}

void Bill::readXmlTmp(QXmlStreamReader *reader ) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "BILL"){
        loadFromXmlTmp( reader->attributes() );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL") ){
        reader->readNext();
        if( reader->isStartElement() ){
            QString tag = reader->name().toString().toUpper();
            if( tag == "ATTRIBUTESMODEL" ) {
                m_d->attributesModel->readXml( reader );
            }
            if( tag == "VARSMODEL" ) {
                m_d->varsModel->readXml( reader );
            }
            if( tag == "BILLITEM" ) {
                m_d->rootItem->readXmlTmp( reader );
            }
        }
    }
}

void Bill::readFromXmlTmp(ProjectPriceListParentItem * priceLists) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
    m_d->rootItem->readFromXmlTmp( m_d->priceList, m_d->attributesModel );
}

void Bill::loadXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists) {
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

void Bill::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
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

QList<PriceItem *> Bill::connectedPriceItems() {
    return m_d->rootItem->connectedPriceItems();
}

void Bill::writeODTBillOnTable( QTextCursor *cursor,
                                BillPrinter::PrintBillItemsOption prItemsOption,
                                const QList<int> fieldsToPrint,
                                bool groupPrAm ) {
    m_d->rootItem->writeODTBillOnTable(cursor, prItemsOption, fieldsToPrint, groupPrAm );
}

void Bill::writeODTAttributeBillOnTable(QTextCursor *cursor,
                                        BillPrinter::AttributePrintOption prOption,
                                        BillPrinter::PrintBillItemsOption prItemsOption,
                                        const QList<int> &fieldsToPrint,
                                        const QList<Attribute *> &attrsToPrint,
                                        bool groupPrAm) {
    m_d->rootItem->writeODTAttributeBillOnTable( cursor, prOption, prItemsOption, fieldsToPrint, attrsToPrint, groupPrAm );
}


void Bill::writeODTSummaryOnTable(QTextCursor *cursor,
                                  BillPrinter::PrintBillItemsOption prItemsOption,
                                  const QList<int> fieldsToPrint,
                                  bool groupPrAm,
                                  bool writeDetails ) {
    m_d->rootItem->writeODTSummaryOnTable(cursor, prItemsOption, fieldsToPrint, groupPrAm, writeDetails );
}

void Bill::insertStandardAttributes(){
    m_d->attributesModel->insertStandardAttributes();
}
