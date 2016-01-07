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
#include "project.h"

#include "projectrootitem.h"
#include "projectdataparentitem.h"
#include "projectpricelistparentitem.h"
#include "projectbillparentitem.h"
#include "bill.h"
#include "pricelist.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"
#include "pricefieldmodel.h"

#include <QXmlStreamReader>
#include <QTextStream>

class ProjectPrivate{
public:
    ProjectPrivate( MathParser * parser ):
        unitMeasureModel( new UnitMeasureModel( parser ) ),
        priceFieldModel( new PriceFieldModel(parser)),
        rootItem( new ProjectRootItem() ),
        priceListParentItem( new ProjectPriceListParentItem( rootItem, priceFieldModel, parser )),
        billParentItem(new ProjectBillParentItem( rootItem, priceFieldModel,  parser )),
        lastXMLVersion(1.0){
        rootItem->insertChild( new ProjectDataParentItem(rootItem) );
        rootItem->insertChild( priceListParentItem );
        rootItem->insertChild( billParentItem );
    };
    UnitMeasureModel * unitMeasureModel;
    PriceFieldModel * priceFieldModel;
    ProjectRootItem * rootItem;
    ProjectPriceListParentItem * priceListParentItem;
    ProjectBillParentItem * billParentItem;
    double lastXMLVersion;
};

Project::Project( MathParser * p,QObject *parent) :
    QAbstractItemModel(parent),
    m_d( new ProjectPrivate(p) ) {
    connect( m_d->rootItem, &ProjectRootItem::dataChanged, this, &Project::updateData );

    connect( m_d->unitMeasureModel, &UnitMeasureModel::removeSignal, this, &Project::removeUnitMeasure );

    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::removePriceItemSignal, this, &Project::removePriceItem );
    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::removePriceListSignal, this, &Project::removePriceList );
    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::beginInsertChildren, this, &Project::beginInsertPriceLists );
    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::endInsertChildren, this, &Project::endInsertPriceLists );
    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::beginRemoveChildren, this, &Project::beginRemovePriceLists );
    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::endRemoveChildren, this, &Project::endRemovePriceLists );

    connect( m_d->billParentItem, &ProjectBillParentItem::beginInsertChildren, this, &Project::beginInsertBills );
    connect( m_d->billParentItem, &ProjectBillParentItem::endInsertChildren, this, &Project::endInsertBills );
    connect( m_d->billParentItem, &ProjectBillParentItem::beginRemoveChildren, this, &Project::beginRemoveBills );
    connect( m_d->billParentItem, &ProjectBillParentItem::endRemoveChildren, this, &Project::endRemoveBills );

    connect( m_d->priceFieldModel, &PriceFieldModel::modelChanged, this, &Project::modelChanged );
    connect( m_d->unitMeasureModel, &UnitMeasureModel::modelChanged, this, &Project::modelChanged );
    connect( m_d->priceListParentItem, &ProjectPriceListParentItem::modelChanged, this, &Project::modelChanged );
    connect( m_d->billParentItem, &ProjectBillParentItem::modelChanged, this, &Project::modelChanged );
    connect( this, &Project::rowsMoved, this, &Project::modelChanged );
    connect( this, &Project::rowsRemoved, this, &Project::modelChanged );
    connect( this, &Project::modelReset, this, &Project::modelChanged );
    connect( this, &Project::dataChanged, this, &Project::modelChanged );
}

Project::~Project(){
    delete m_d;
}

void Project::createSimpleProject(SimpleProjectType projType){
    if( projType == ProjectEmpty ){
        clear();
    } else if( projType == ProjectSimple ){
        clear();
        m_d->unitMeasureModel->insertStandardUnits();
        m_d->priceListParentItem->insertChildren( 0 );
        m_d->billParentItem->insertChildren( 0 );
        m_d->billParentItem->bill(0)->insertStandardAttributes();
        m_d->billParentItem->bill(0)->setPriceList( m_d->priceListParentItem->priceList(0) );
    } else if( projType == ProjectHumanNetNoDiscount ){
        clear();
        m_d->unitMeasureModel->insertStandardUnits();
        m_d->priceFieldModel->createHumanNetPriceFields();
        m_d->priceListParentItem->insertChildren( 0 );
        m_d->billParentItem->insertChildren( 0 );
        m_d->billParentItem->bill(0)->insertStandardAttributes();
        m_d->billParentItem->bill(0)->setPriceList( m_d->priceListParentItem->priceList(0) );
    }
}

void Project::beginInsertPriceLists(int first, int last){
    beginInsertRows( index(0, m_d->priceListParentItem), first, last);
}

void Project::endInsertPriceLists(){
    endInsertRows();
}

void Project::beginRemovePriceLists(int first, int last){
    if( last >= first ){
        beginRemoveRows( index(0, m_d->priceListParentItem), first, last);
    }
}

void Project::endRemovePriceLists(){
    endRemoveRows();
}

void Project::beginInsertBills(int first, int last){
    beginInsertRows( index(0, m_d->billParentItem), first, last);
}

void Project::endInsertBills(){
    endInsertRows();
}

void Project::beginRemoveBills(int first, int last){
    if( last >= first ){
        beginRemoveRows( index(0, m_d->billParentItem), first, last);
    }
}

void Project::endRemoveBills(){
    endRemoveRows();
}

UnitMeasureModel *Project::unitMeasureModel() {
    return m_d->unitMeasureModel;
}

PriceFieldModel *Project::priceFieldModel() {
    return m_d->priceFieldModel;
}

int Project::priceListCount() {
    return m_d->priceListParentItem->priceListCount();
}

PriceList *Project::priceList(int i) {
    return m_d->priceListParentItem->priceList(i);
}

int Project::billCount() {
    return m_d->billParentItem->billCount();
}

Bill *Project::bill(int i) {
    return m_d->billParentItem->bill( i );
}

int Project::columnCount(const QModelIndex & /* parent */) const {
    return 1;
}

int Project::rowCount(const QModelIndex &parent) const {
    ProjectItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

Qt::ItemFlags Project::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    ProjectItem * item = getItem( index );

    return item->flags();
}

QVariant Project::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    ProjectItem *item = getItem(index);

    return item->data();
}

bool Project::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole)
        return false;

    ProjectItem *item = getItem(index);
    bool result = item->setData( value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

QVariant Project::headerData(int section, Qt::Orientation orientation,
                             int role) const {
    Q_UNUSED(section);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(trUtf8("Progetto") );

    return QVariant();
}

bool Project::insertRows(int position, int rows, const QModelIndex &parent) {
    ProjectItem *parentItem = getItem(parent);
    bool success;

    // beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows );
    // endInsertRows();

    return success;
}

bool Project::appendRow(const QModelIndex &parent) {
    return insertRows( rowCount(parent), 1, parent );
}

bool Project::removeRows(int position, int rows, const QModelIndex &parent) {
    if( rows > 0 && position > -1 ){
        ProjectItem *parentItem = getItem(parent);
        bool success = true;

        // beginRemoveRows(parent, position, position + rows - 1);
        success = parentItem->removeChildren(position, rows);
        // endRemoveRows();

        return success;
    }
    return false;
}

void Project::clear() {
    m_d->billParentItem->clear();
    m_d->priceListParentItem->clear();
    m_d->priceFieldModel->clear();
    m_d->unitMeasureModel->clear();
}

QModelIndex Project::index(int row, int column, const QModelIndex &parent) const {
    if ( (parent.isValid() && parent.column() != 0) || column != 0 )
        return QModelIndex();

    ProjectItem *parentItem = getItem(parent);

    ProjectItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex Project::index( int column, ProjectItem * item) const {
    return createIndex( item->childNumber(), column, item);
}

QModelIndex Project::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    ProjectItem *childItem = getItem(index);
    ProjectItem *parentItem = childItem->parentItem();

    if (parentItem == m_d->rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool Project::canChildrenBeInserted(const QModelIndex &index) {
    ProjectItem * item = getItem( index );
    return item->canChildrenBeInserted();
}

void Project::updateData(int column, ProjectItem *item) {
    QModelIndex i = index( column, item );
    emit dataChanged( i, i );
}

void Project::removeUnitMeasure(int row, int count) {
    for( int i=row+count-1; i >= row; --i){
        UnitMeasure * ump = m_d->unitMeasureModel->unitMeasure(i);
        if( ump ){
            bool isUsed = false;
            for( int j=0; j < m_d->priceListParentItem->priceListCount(); ++j ){
                if( m_d->priceListParentItem->priceList(j)->isUsingUnitMeasure(ump) ){
                    isUsed = true;
                    break;
                }
            }
            if( !isUsed ){
                m_d->unitMeasureModel->removeUnitMeasurePrivate( i );
            }
        } else {
            m_d->unitMeasureModel->removeUnitMeasurePrivate( i );
        }
    }
}

void Project::removePriceItem(PriceList *pl, int position, int count, const QModelIndex &parent) {
    PriceItem *parentItem = pl->priceItem(parent);

    if( parentItem ){
        for( int i=position+count-1; i >= position; --i ){
            PriceItem *childItem = dynamic_cast<PriceItem *>(parentItem->child(i));
            if ( ! m_d->billParentItem->isUsingPriceItem(childItem) ){
                pl->removePriceItems( i, 1, parent );
            }
        }
    }
}


void Project::removePriceList(int position, int count) {
    for( int i=position+count-1; i >= position; --i ){
        PriceList * pl = m_d->priceListParentItem->priceList(i);
        bool isUsed = m_d->billParentItem->isUsingPriceList(pl);
        if( !isUsed ){
            QList<PriceItem *> priceItemList = pl->priceItemList();
            QList<PriceItem *>::iterator j=priceItemList.begin();
            while( j!=priceItemList.end() && !isUsed ){
                if ( m_d->billParentItem->isUsingPriceItem(*j) ){
                    isUsed = true;
                }
                ++j;
            }
        }
        if( !isUsed ){
            m_d->priceListParentItem->removeChildrenPrivate( i );
        }
    }
}

ProjectItem * Project::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        ProjectItem *item = static_cast<ProjectItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_d->rootItem;
}

void Project::writeXml(QXmlStreamWriter *writer, const QString &vers) {
    writer->setAutoFormatting(true);
    writer->setCodec("UTF-8");

    writer->writeStartDocument();
    writer->writeStartElement( "QCostProject" );
    writer->writeAttribute("version", vers );

    m_d->priceFieldModel->writeXml( writer, vers );
    m_d->unitMeasureModel->writeXml( writer, vers );
    m_d->priceListParentItem->writeXml( writer, vers );
    m_d->billParentItem->writeXml( writer );

    writer->writeEndElement();
    writer->writeEndDocument();

}

void Project::readXml(QXmlStreamReader *reader) {
    while( (reader->name().toString().toUpper() != "QCOSTPROJECT" ) &&
           (!reader->atEnd()) &&
           (!reader->hasError())){
        reader->readNext();
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError())){
        reader->readNext();
        if( reader->isStartElement() ){
            QString tagUp = reader->name().toString().toUpper();
            if( tagUp == "BILLS"){
                m_d->billParentItem->readXml( reader, m_d->priceListParentItem );
            } else if( tagUp == "PRICELISTS"){
                m_d->priceListParentItem->readXml( reader, m_d->unitMeasureModel );
            } else if( tagUp == "UNITMEASUREMODEL"){
                m_d->unitMeasureModel->readXml( reader );
            } else if( tagUp == "PRICEFIELDMODEL"){
                m_d->priceFieldModel->readXml( reader );
            }
        }
    }
}
