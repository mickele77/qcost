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

#include "billwriter.h"

#include "bill.h"

#include "billitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "projectpricelistparentitem.h"
#include "attribute.h"

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QVariant>
#include <QString>

class BillPrivate{
public:
    BillPrivate( const QString &n, MathParser * p = NULL ):
        name( n ),
        parser(p),
        rootItem(new BillItem( NULL, NULL, parser )),
        priceList( NULL ),
        priceListIdTmp(0){
        id = nextId++;
    };
    ~BillPrivate(){
        delete rootItem;
    };

    unsigned int id;
    static unsigned int nextId;

    QString name;
    QString description;

    MathParser * parser;
    BillItem * rootItem;
    PriceList * priceList;
    unsigned int priceListIdTmp;
};

unsigned int BillPrivate::nextId = 0;

Bill::Bill(const QString &n, ProjectItem *parent, MathParser * parser ):
    ProjectItem(parent),
    m_d( new BillPrivate( n, parser ) ) {
    connect( m_d->rootItem, SIGNAL(dataChanged(BillItem*,int)), this, SLOT(updateValue(BillItem*,int)) );
    connect( m_d->rootItem, SIGNAL(amountTotalChanged(QString)), this, SIGNAL(amountTotalChanged(QString)) );
    connect( m_d->rootItem, SIGNAL(amountHumanFactorChanged(QString)), this, SIGNAL(amountHumanFactorChanged(QString)) );
    connect( m_d->rootItem, SIGNAL(amountTotalChanged(double)), this, SIGNAL(amountTotalChanged(double)) );
    connect( m_d->rootItem, SIGNAL(amountHumanFactorChanged(double)), this, SIGNAL(amountHumanFactorChanged(double)) );
}

Bill::Bill(Bill & b):
    QAbstractTableModel(),
    ProjectItem( b.ProjectItem::parent() ),
    m_d( new BillPrivate( b.m_d->name, b.m_d->parser ) ) {
    connect( m_d->rootItem, SIGNAL(dataChanged(BillItem*,int)), this, SLOT(updateValue(BillItem*,int)) );
    connect( m_d->rootItem, SIGNAL(amountTotalChanged(QString)), this, SIGNAL(amountTotalChanged(QString)) );
    connect( m_d->rootItem, SIGNAL(amountHumanFactorChanged(QString)), this, SIGNAL(amountHumanFactorChanged(QString)) );
    connect( m_d->rootItem, SIGNAL(amountTotalChanged(double)), this, SIGNAL(amountTotalChanged(double)) );
    connect( m_d->rootItem, SIGNAL(amountHumanFactorChanged(double)), this, SIGNAL(amountHumanFactorChanged(double)) );

    *this = b;
}

Bill::~Bill(){
    emit aboutToDelete();
    delete m_d;
}

Bill &Bill::operator=(const Bill &cp) {
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    setPriceList( cp.m_d->priceList );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
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

void Bill::setPriceCol(int v) {
    if( v > -1 && v < m_d->priceList->nPriceCol() &&
            m_d->rootItem->priceCol() != v ){
        m_d->rootItem->setPriceCol( v );
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
    if( plMode != None ){
        if( m_d->priceList != NULL ){
            if( plMode == SearchAndAdd ){
                // cerca e aggiunge se manca
                QList<BillItem *> allItems = m_d->rootItem->allChildren();
                for(QList<BillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                    PriceItem * newPriceItem = pl->priceItemCode( (*i)->priceItem()->code() );
                    if( newPriceItem == NULL ){
                        newPriceItem = pl->appendPriceItem();
                        *newPriceItem = (*i)->priceItem();
                    }
                    (*i)->setPriceItem( newPriceItem );
                }
            } else if( plMode == Add ){
                // aggiunge sempre e comunque
                QList<BillItem *> allItems = m_d->rootItem->allChildren();
                for(QList<BillItem *>::iterator i = allItems.begin(); i != allItems.end(); ++i ){
                    PriceItem * newPriceItem = pl->appendPriceItem();
                    *newPriceItem = (*i)->priceItem();
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
                m_d->rootItem->setPriceCol( 0 );
            } else {
                if( m_d->rootItem->priceCol() > pl->nPriceCol() ){
                    m_d->rootItem->setPriceCol( 0 );
                }
            }
        }
        m_d->priceList = pl;
        emit priceListChanged( pl );
    }
}

PriceList *Bill::priceList() {
    return m_d->priceList;
}

int Bill::priceCol() {
    return m_d->rootItem->priceCol();
}


BillItem *Bill::billItem(const QModelIndex &index ) const {
    if (index.isValid()) {
        return static_cast<BillItem *>(index.internalPointer());
    }
    return m_d->rootItem;
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

double Bill::amountTotal() {
    return m_d->rootItem->amountTotal();
}

double Bill::amountTotal(Attribute *attr) {
    return m_d->rootItem->amountTotal(attr);
}

double Bill::amountTotal(const QList<Attribute *> &attrs) {
    return m_d->rootItem->amountTotal(attrs);
}

double Bill::amountHumanFactor() {
    return m_d->rootItem->amountHumanFactor();
}

void Bill::nextId() {
    m_d->id = m_d->nextId++;
}

unsigned int Bill::id() {
    return m_d->id;
}

void Bill::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "Bill" );
    writer->writeAttribute( "id", QString::number(m_d->id) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeAttribute( "description", m_d->description );
    if( m_d->priceList ){
        writer->writeAttribute( "priceList", QString::number( m_d->priceList->id() ) );
    }
    writer->writeAttribute( "priceCol", QString::number( m_d->rootItem->priceCol() ) );

    m_d->rootItem->writeXml( writer );

    writer->writeEndElement();
}

void Bill::readXml(QXmlStreamReader *reader, AttributeList * attrList, ProjectPriceListParentItem *priceLists) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "BILL"){
        loadFromXml( reader->attributes(), priceLists );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILLITEM" && reader->isStartElement()) {
            m_d->rootItem->readXml( reader, attrList, m_d->priceList );
        }
    }
}

void Bill::readXmlTmp(QXmlStreamReader *reader ) {
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

void Bill::loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists) {
    for( QXmlStreamAttributes::const_iterator i=attrs.begin(); i != attrs.end(); ++i ){
        QString nameUp = (*i).name().toString().toUpper();
        if( nameUp == "ID" ){
            m_d->id = (*i).value().toUInt();
        }
        qWarning( "%s", nameUp.toStdString().c_str() );
    }

    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "name" ) ){
        setName( attrs.value( "name").toString() );
    }
    if( attrs.hasAttribute( "description" ) ){
        setDescription( attrs.value( "description").toString() );
    }
    if( attrs.hasAttribute( "priceList" ) ){
        m_d->priceList = priceLists->priceListId( attrs.value( "priceList").toUInt() );
    }
    if( attrs.hasAttribute( "priceCol" ) ){
        m_d->rootItem->setPriceCol( attrs.value( "priceCol").toInt() );
    }
}

void Bill::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "name" ) ){
        setName( attrs.value( "name").toString() );
    }
    if( attrs.hasAttribute( "description" ) ){
        setDescription( attrs.value( "description").toString() );
    }
    if( attrs.hasAttribute( "priceList" ) ){
        m_d->priceListIdTmp = attrs.value( "priceList").toUInt();
    }
    if( attrs.hasAttribute( "priceCol" ) ){
        m_d->rootItem->setPriceCol( attrs.value( "priceCol").toInt() );
    }
}

void Bill::setTmpData( ProjectPriceListParentItem * priceLists ) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
}

QList<PriceItem *> Bill::connectedPriceItems() {
    return m_d->rootItem->connectedPriceItems();
}

void Bill::writeOnTable(QTextCursor *cursor, int printData ) {
    m_d->rootItem->writeOnTable(cursor, printData);
}

void Bill::loadTmpData(ProjectPriceListParentItem * priceLists, AttributeList *attrList) {
    m_d->priceList = priceLists->priceListId( m_d->priceListIdTmp );
    m_d->rootItem->loadTmpData( attrList, m_d->priceList );
}

void Bill::removeUsingAttribute(Attribute *attr){
    return m_d->rootItem->removeUsingAttribute(attr);
}
