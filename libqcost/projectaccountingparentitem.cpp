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

#include "paymentdatamodel.h"
#include "paymentdata.h"
#include "accountingbill.h"
#include "accountinglsbills.h"
#include "accountingtambill.h"
#include "priceitem.h"
#include "mathparser.h"

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
        nextId(1) {
    }
    ~ProjectAccountingParentItemPrivate(){
        delete dataModel;
    }
    QString amountToString( double val ){
        if( parser != NULL ){
            return parser->toString( val, 'f', amountPrecision );
        }
        return QString::number( val, 'f', amountPrecision );
    }

    AccountingBill * measuresBill;
    AccountingLSBills * lumpSumBills;
    AccountingTAMBill * timeAndMaterialBill;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    unsigned int nextId;
    PaymentDataModel * dataModel;

    static int amountPrecision;
};

int ProjectAccountingParentItemPrivate::amountPrecision = 2;

ProjectAccountingParentItem::ProjectAccountingParentItem( ProjectItem *parent, PriceFieldModel * pfm, MathParser * prs ):
    ProjectRootItem(parent),
    m_d( new ProjectAccountingParentItemPrivate( pfm, prs ) ){
    m_d->measuresBill = new AccountingBill( trUtf8("Libretto delle misure"), this, pfm, prs );
    m_d->dataModel = new PaymentDataModel( m_d->measuresBill->rootItem(), prs );
    insertChild( m_d->measuresBill );

    connect( m_d->measuresBill, &AccountingBill::modelChanged, this, &ProjectAccountingParentItem::modelChanged );
    connect( m_d->measuresBill, &AccountingBill::paymentInserted, m_d->dataModel, &PaymentDataModel::insertPayment );
    connect( m_d->measuresBill, &AccountingBill::paymentRemoved, m_d->dataModel, &PaymentDataModel::removePayment );

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

int ProjectAccountingParentItem::paymentsCount() {
    return m_d->measuresBill->paymentsCount();
}

PaymentData *ProjectAccountingParentItem::paymentData(int pos) {
    return m_d->dataModel->paymentData( pos );
}

AccountingBill *ProjectAccountingParentItem::measuresBill() {
    return m_d->measuresBill;
}

PaymentDataModel * ProjectAccountingParentItem::dataModel(){
    return m_d->dataModel;
}

double ProjectAccountingParentItem::totalAmountToDiscount() {
    return m_d->measuresBill->totalAmountToDiscount();
}

double ProjectAccountingParentItem::amountNotToDiscount() {
    return m_d->measuresBill->amountNotToDiscount();
}

double ProjectAccountingParentItem::amountToDiscount() {
    return m_d->measuresBill->amountToDiscount();
}

double ProjectAccountingParentItem::amountDiscounted() {
    return m_d->measuresBill->amountDiscounted();
}

double ProjectAccountingParentItem::totalAmount() {
    return m_d->measuresBill->totalAmount();
}

QString ProjectAccountingParentItem::totalAmountToDiscountStr() {
    return m_d->measuresBill->totalAmountToDiscountStr();
}

QString ProjectAccountingParentItem::amountNotToDiscountStr() {
    return m_d->measuresBill->amountNotToDiscountStr();
}

QString ProjectAccountingParentItem::amountToDiscountStr() {
    return m_d->measuresBill->amountToDiscountStr();
}

QString ProjectAccountingParentItem::amountDiscountedStr() {
    return m_d->measuresBill->amountDiscountedStr();
}

QString ProjectAccountingParentItem::totalAmountStr() {
    return m_d->measuresBill->totalAmountStr();
}

AccountingLSBills *ProjectAccountingParentItem::lumpSumBills() {
    return m_d->lumpSumBills;
}

AccountingTAMBill *ProjectAccountingParentItem::timeAndMaterialBill() {
    return m_d->timeAndMaterialBill;
}

ProjectItem *ProjectAccountingParentItem::child(int number) {
    if( number == 0 ){
        return m_d->measuresBill;
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
    if( item == m_d->measuresBill ){
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

bool ProjectAccountingParentItem::clear() {
    bool ret = true;
    ret = m_d->measuresBill->clear() && ret;
    ret = m_d->lumpSumBills->clear() && ret;
    ret = m_d->timeAndMaterialBill->clear() && ret;
    return ret;
}

bool ProjectAccountingParentItem::isUsingPriceList(PriceList *pl) {
    if( m_d->measuresBill->isUsingPriceList( pl ) ){
        return true;
    } else if( m_d->lumpSumBills->isUsingPriceList( pl ) ){
        return true;
    } else if( m_d->timeAndMaterialBill->isUsingPriceList( pl ) ){
        return true;
    }
    return false;
}

bool ProjectAccountingParentItem::isUsingPriceItem(PriceItem *p) {
    if( m_d->measuresBill->isUsingPriceItem( p ) ){
        return true;
    } else if( m_d->lumpSumBills->isUsingPriceItem( p ) ){
        return true;
    } else if( m_d->timeAndMaterialBill->isUsingPriceItem( p ) ){
        return true;
    }
    return false;
}

void ProjectAccountingParentItem::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "Accounting");
    m_d->measuresBill->writeXml( writer );
    m_d->lumpSumBills->writeXml( writer );
    m_d->timeAndMaterialBill->writeXml( writer );
    writer->writeEndElement();
}

void ProjectAccountingParentItem::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTING") ){
        reader->readNext();
        QString nodeName = reader->name().toString().toUpper();
        nodeName = reader->name().toString().toUpper();
        if( nodeName == "ACCOUNTINGBILL" && reader->isStartElement()) {
            m_d->measuresBill->readXml( reader, priceLists, m_d->lumpSumBills, m_d->timeAndMaterialBill );
        }
        if( nodeName == "ACCOUNTINGLSBILLS" && reader->isStartElement()) {
            m_d->lumpSumBills->readXml( reader, priceLists );
        }
        if( nodeName == "ACCOUNTINGTAMBILL" && reader->isStartElement()) {
            m_d->timeAndMaterialBill->readXml( reader, priceLists );
        }
    }
}
