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
#include "projectaccountingparentitem.h"

#include "accountingbills.h"
#include "accountinglsbills.h"
#include "accountingtambill.h"
#include "priceitem.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QObject>
#include <QVariant>
#include <QList>

class ProjectAccountingParentItemPrivate{
public:
    ProjectAccountingParentItemPrivate( PriceFieldModel * pfm, MathParser * p = NULL ):
        priceFieldModel(pfm),
        parser(p),
        nextId(1){
    }
    ~ProjectAccountingParentItemPrivate(){
    }

    AccountingBills * measuresBills;
    AccountingLSBills * lumpSumBills;
    AccountingTAMBill * timeAndMaterialBill;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    unsigned int nextId;
};

ProjectAccountingParentItem::ProjectAccountingParentItem( ProjectItem *parent, PriceFieldModel * pfm, MathParser * prs ):
    ProjectRootItem(parent),
    m_d( new ProjectAccountingParentItemPrivate( pfm, prs ) ){
    m_d->measuresBills = new AccountingBills( this, pfm, prs );
    insertChild( m_d->measuresBills );
    connect( m_d->measuresBills, &AccountingBills::beginInsertChildren, this, &ProjectAccountingParentItem::beginInsertChildren );
    connect( m_d->measuresBills, &AccountingBills::endInsertChildren, this, &ProjectAccountingParentItem::endInsertChildren );
    connect( m_d->measuresBills, &AccountingBills::beginRemoveChildren, this, &ProjectAccountingParentItem::beginRemoveChildren );
    connect( m_d->measuresBills, &AccountingBills::endRemoveChildren, this, &ProjectAccountingParentItem::endRemoveChildren );
    connect( m_d->measuresBills, &AccountingBills::modelChanged, this, &ProjectAccountingParentItem::modelChanged );

    m_d->lumpSumBills = new AccountingLSBills( this, pfm, prs );
    insertChild( m_d->lumpSumBills );
    connect( m_d->lumpSumBills, &AccountingLSBills::beginInsertChildren, this, &ProjectAccountingParentItem::beginInsertChildren );
    connect( m_d->lumpSumBills, &AccountingLSBills::endInsertChildren, this, &ProjectAccountingParentItem::endInsertChildren );
    connect( m_d->lumpSumBills, &AccountingLSBills::beginRemoveChildren, this, &ProjectAccountingParentItem::beginRemoveChildren );
    connect( m_d->lumpSumBills, &AccountingLSBills::endRemoveChildren, this, &ProjectAccountingParentItem::endRemoveChildren );
    connect( m_d->lumpSumBills, &AccountingLSBills::modelChanged, this, &ProjectAccountingParentItem::modelChanged );

    m_d->timeAndMaterialBill = new AccountingTAMBill( trUtf8("Opere in economia"), this, pfm, prs );
    insertChild( m_d->timeAndMaterialBill );
    connect( m_d->timeAndMaterialBill, &AccountingTAMBill::modelChanged, this, &ProjectAccountingParentItem::modelChanged );
}

ProjectAccountingParentItem::~ProjectAccountingParentItem(){
    delete m_d;
}

AccountingBills *ProjectAccountingParentItem::accountingBills() {
    return m_d->measuresBills;
}

AccountingLSBills *ProjectAccountingParentItem::lumpSumBills() {
    return m_d->lumpSumBills;
}

AccountingTAMBill *ProjectAccountingParentItem::timeAndMaterialBill() {
    return m_d->timeAndMaterialBill;
}

ProjectItem *ProjectAccountingParentItem::child(int number) {
    if( number == 0 ){
        return m_d->measuresBills;
    } else if( number == 1 ){
        return m_d->lumpSumBills;
    } else if( number == 2 ){
        return m_d->timeAndMaterialBill;
    }
    return NULL;
}

int ProjectAccountingParentItem::childCount() const {
    return 3;
}

int ProjectAccountingParentItem::childNumber(ProjectItem *item) {
    if( item == m_d->measuresBills ){
        return 0;
    } else if( item == m_d->lumpSumBills ){
        return 1;
    } else if( item == m_d->timeAndMaterialBill ){
        return 2;
    }
    return -1;
}

bool ProjectAccountingParentItem::canChildrenBeInserted() {
    return false;
}

Qt::ItemFlags ProjectAccountingParentItem::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant ProjectAccountingParentItem::data() const {
    return QVariant( QObject::trUtf8("Contabilità") );
}

bool ProjectAccountingParentItem::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}

bool ProjectAccountingParentItem::isUsingPriceList(PriceList *pl) {
    if( m_d->measuresBills->isUsingPriceList( pl ) ){
        return true;
    } else if( m_d->lumpSumBills->isUsingPriceList( pl ) ){
        return true;
    } else if( m_d->timeAndMaterialBill->isUsingPriceList( pl ) ){
        return true;
    }
    return false;
}

bool ProjectAccountingParentItem::isUsingPriceItem(PriceItem *p) {
    if( m_d->measuresBills->isUsingPriceItem( p ) ){
        return true;
    } else if( m_d->lumpSumBills->isUsingPriceItem( p ) ){
        return true;
    } else if( m_d->timeAndMaterialBill->isUsingPriceItem( p ) ){
        return true;
    }
    return false;
}

void ProjectAccountingParentItem::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "Accountings");
    m_d->measuresBills->writeXml( writer );
    m_d->lumpSumBills->writeXml( writer );
    m_d->timeAndMaterialBill->writeXml( writer );
    writer->writeEndElement();
}


void ProjectAccountingParentItem::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "MEASURESBILLS" && reader->isStartElement()) {
            m_d->measuresBills->readXml( reader, priceLists );
        }
        if( reader->name().toString().toUpper() == "LUMPSUMBILLS" && reader->isStartElement()) {
            m_d->lumpSumBills->readXml( reader, priceLists );
        }
        if( reader->name().toString().toUpper() == "TIMEANDMATERIALSBILLS" && reader->isStartElement()) {
            m_d->timeAndMaterialBill->readXml( reader, priceLists );
        }
    }
}

