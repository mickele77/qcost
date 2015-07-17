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
#include "pricelist.h"

#include "unitmeasuremodel.h"
#include "pricefieldmodel.h"
#include "priceitem.h"
#include "priceitemdatasetmodel.h"
#include "mathparser.h"

#include <QXmlStreamReader>
#include <QTextStream>

class PriceListPrivate{
public:
    PriceListPrivate( const QString &n, PriceFieldModel *priceFields, MathParser * prs ):
        id(0),
        name( n ),
        rootItem( new PriceItem( NULL, priceFields, prs ) ),
        parser(prs) {
    };
    ~PriceListPrivate(){
        delete rootItem;
    };

    unsigned int id;
    QString name;
    QString description;
    PriceItem * rootItem;
    MathParser * parser;
};

PriceList::PriceList(const QString &n , PriceFieldModel *priceFields, ProjectItem *parent, MathParser * p):
    ProjectItem(parent),
    m_d( new PriceListPrivate( n, priceFields, p ) ){
    connect( m_d->rootItem, &PriceItem::dataChanged, this, &PriceList::updateValueTotal );
    connect( m_d->rootItem, static_cast<void(PriceItem::*)(PriceItem*,QList<int>)>(&PriceItem::hasChildrenChanged), this, &PriceList::updateChildrenChanged );
    connect( m_d->rootItem, &PriceItem::priceDataSetCountChanged, this, &PriceList::priceDataSetCountChanged );

    connect( m_d->rootItem, &PriceItem::beginInsertPriceDataSets, this, &PriceList::beginChangingColumns );
    connect( m_d->rootItem, &PriceItem::endInsertPriceDataSets, this, &PriceList::endChangingColumns );
    connect( m_d->rootItem, &PriceItem::beginRemovePriceDataSets, this, &PriceList::beginChangingColumns );
    connect( m_d->rootItem, &PriceItem::endRemovePriceDataSets, this, &PriceList::endChangingColumns );
    connect( m_d->rootItem, &PriceItem::itemChanged, this, &PriceList::modelChanged );

    connect( priceFields, &PriceFieldModel::beginInsertPriceField, this, &PriceList::beginChangingColumns );
    connect( priceFields, &PriceFieldModel::endInsertPriceField, this, &PriceList::endChangingColumns );
}

PriceList::~PriceList(){
    emit aboutToBeDeleted();
    delete m_d;
}

PriceList &PriceList::operator=(const PriceList &cp) {
    setName( cp.m_d->name );
    setDescription( cp.m_d->description );
    *(m_d->rootItem) = *(cp.m_d->rootItem);
    return *this;
}

int PriceList::firstValueCol() {
    return m_d->rootItem->firstValueCol();
}

int PriceList::childNumber(ProjectItem *item) {
    Q_UNUSED(item);
    return -1;
}

ProjectItem *PriceList::child(int number) {
    Q_UNUSED(number);
    return NULL;
}

int PriceList::childCount() const {
    return 0;
}

bool PriceList::canChildrenBeInserted() {
    return false;
}

bool PriceList::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool PriceList::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

Qt::ItemFlags PriceList::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant PriceList::data() const {
    return QVariant( m_d->name );
}

bool PriceList::setData(const QVariant &value) {
    if( m_d->name != value ){
        m_d->name = value.toString();
        return true;
    } return false;
}

int PriceList::columnCount(const QModelIndex & /* parent */) const {
    return m_d->rootItem->columnCount();
}

QVariant PriceList::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    PriceItem *i = priceItem(index);

    return i->data(index.column(), role);
}

Qt::ItemFlags PriceList::flags(const QModelIndex &index) const {
    PriceItem *i = priceItem(index);
    if( i ){
        return (i->flags(  index.column() ) );
    } else {
        return QAbstractItemModel::flags(index);
    }
}

PriceItem *PriceList::priceItem(const QModelIndex &index) const {
    if (index.isValid()) {
        return static_cast<PriceItem*>(index.internalPointer());
    }
    return m_d->rootItem;
}

PriceItem *PriceList::priceItemId(unsigned int i) {
    return m_d->rootItem->priceItemId( i );
}

PriceItem *PriceList::defaultPriceItem() {
    // per adesso va bene così, forse bisognerà migliorarlo
    PriceItem * item = m_d->rootItem;
    while( item->hasChildren() ){
        item = dynamic_cast<PriceItem *>(item->child(0));
    }
    if( item == m_d->rootItem ){
        return NULL;
    } else {
        return item;
    }
}

PriceItem *PriceList::priceItemCode(const QString &c) {
    return m_d->rootItem->priceItemFullCode( c );
}

QVariant PriceList::headerData(int section, Qt::Orientation orientation,
                               int role) const {
    if (orientation == Qt::Horizontal )
        return m_d->rootItem->data(section, role);
    else if (orientation == Qt::Vertical )
        return QVariant( section + 1 );
    return QVariant();
}

QModelIndex PriceList::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    PriceItem *parentItem = dynamic_cast<PriceItem *>(priceItem(parent));

    if( parentItem ){
        PriceItem *childItem = dynamic_cast<PriceItem *>(parentItem->child(row));
        if (childItem)
            return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex PriceList::index(PriceItem * item, int column) const {
    if (item == NULL )
        return QModelIndex();

    if( item->parentItem() == NULL ){
        return QModelIndex();
    } else {
        return createIndex(item->childNumber(), column, item);
    }
}

void PriceList::writeODTOnTable( QTextCursor *cursor, PriceListPrinter::PrintPriceItemsOption printOption, const QList<int> fieldsToPrint, int priceColToPrint ) {
    m_d->rootItem->writeODTOnTable( cursor, printOption, fieldsToPrint, priceColToPrint );
}

bool PriceList::insertRows(int inputPos, int count, const QModelIndex &parent) {
    PriceItem *parentItem = priceItem(parent);

    int position = inputPos;
    if( position == -1 ){
        position = parentItem->childrenCount();
    }

    bool success = false;

    beginInsertRows(parent, position, position + count - 1);
    success = parentItem->insertChildren(position, count );
    endInsertRows();

    return success;
}

PriceItem * PriceList::appendPriceItem() {
    insertRows( -1 , 1 );
    return m_d->rootItem->lastChild();
}

QModelIndex PriceList::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    PriceItem *childItem = priceItem(index);
    PriceItem *parentItem = dynamic_cast<PriceItem *>( childItem->parentItem());

    if (parentItem == m_d->rootItem || parentItem == 0 )
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool PriceList::removeRows(int position, int rows, const QModelIndex &parent){
    emit removePriceItemSignal( position, rows, parent );
    return true;
}

bool PriceList::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow ) {

    if(beginMoveRows( sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow )){
        PriceItem * srcParent = priceItem( sourceParent );
        PriceItem * dstParent = priceItem( destinationParent );
        for( int i=0; i<count; ++i ){
            srcParent->childItem( (sourceRow+count-1)-i )->setParentItem( dstParent, destinationRow );
        }

        endMoveRows();
        return true;
    }
    return false;
}

bool PriceList::removePriceItems(int position, int rows, const QModelIndex &parent) {
    PriceItem *parentItem = priceItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int PriceList::rowCount(const QModelIndex &parent) const
{
    PriceItem *parentItem = priceItem(parent);

    return parentItem->childrenCount();
}

bool PriceList::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;

    PriceItem *i = priceItem(index);
    bool result = i->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

QString PriceList::name() const {
    return m_d->name;
}

void PriceList::setName(const QString &value) {
    if( m_d->name != value ){
        m_d->name = value;
        emit nameChanged( m_d->name );
        if( m_parentItem ){
            m_parentItem->setDataChanged( 0, this );
        }
    }
}

QString PriceList::description() const {
    return m_d->description;
}

int PriceList::priceDataSetCount() {
    return m_d->rootItem->priceDataSetCount();
}

void PriceList::beginChangingColumns() {
    beginResetModel();
}

void PriceList::endChangingColumns() {
    endResetModel();
}

bool PriceList::isUsingUnitMeasure(UnitMeasure * ump) {
    return m_d->rootItem->isUsingUnitMeasure( ump );
}

QList<PriceItem *> PriceList::priceItemList() {
    return m_d->rootItem->allChildrenList();
}

void PriceList::setDescription(const QString &value) {
    if( m_d->description != value ){
        m_d->description = value;
        emit descriptionChanged( value );
    }
}

void PriceList::updateValueTotal(PriceItem * item, int column) {
    QModelIndex i = index( item, column);
    emit dataChanged( i, i);
}

void PriceList::updateChildrenChanged(PriceItem * item, QList<int> cols){
    for( int j=0; j<cols.size(); ++j){
        QModelIndex i = index( item, cols.at(j) );
        emit dataChanged( i, i);
    }
}

void PriceList::nextId() {
    m_d->id++;
}

unsigned int PriceList::id() {
    return m_d->id;
}

void PriceList::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "PriceList" );
    writer->writeAttribute( "id", QString::number(m_d->id) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeAttribute( "description", m_d->description );

    m_d->rootItem->dataModel()->writeXml( writer );

    m_d->rootItem->writeXml( writer );

    writer->writeEndElement();
}

void PriceList::readXml(QXmlStreamReader *reader, UnitMeasureModel * uml ) {
    if(reader->isStartElement() && reader->name().toString().toUpper() == "PRICELIST"){
        loadFromXml( reader->attributes() );
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "PRICELIST") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "PRICEITEM" && reader->isStartElement()) {
            m_d->rootItem->readXml( reader, uml );
        }
        int currentPriceDataSet = -1;
        if( reader->name().toString().toUpper() == "PRICEITEMDATASET" && reader->isStartElement()) {
            currentPriceDataSet++;
            if( currentPriceDataSet >= m_d->rootItem->dataModel()->priceDataSetCount() ){
                m_d->rootItem->dataModel()->appendPriceDataSet( currentPriceDataSet - m_d->rootItem->priceDataSetCount() + 1 );
            }
            m_d->rootItem->dataModel()->loadXmlPriceDataSet( currentPriceDataSet, reader->attributes() );
        }
    }
}

void PriceList::loadFromXml(const QXmlStreamAttributes &attrs ) {
    for( QXmlStreamAttributes::const_iterator i = attrs.begin(); i != attrs.end(); ++i ){
        if( (*i).name().toString().toUpper() == "ID" ){
            m_d->id = (*i).value().toString().toUInt();
        }
        if( (*i).name().toString().toUpper() == "NAME" ){
            setName( (*i).value().toString() );
        }
        if( (*i).name().toString().toUpper() == "DESCRIPTION" ){
            setDescription( (*i).value().toString() );
        }
    }
}

void PriceList::loadTmpData( ProjectPriceListParentItem * priceLists ) {
    m_d->rootItem->loadTmpData( priceLists );
}
