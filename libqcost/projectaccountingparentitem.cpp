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

#include "paymentadatamodel.h"
#include "paymentdata.h"
#include "accountingbills.h"
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
        nextId(1),
        dataModel( new PaymentDataModel(p) ),
        totalAmountToDiscount(0.0),
        amountNotToDiscount(0.0),
        amountToDiscount(0.0),
        amountDiscounted(0.0),
        totalAmount(0.0){
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

    AccountingBills * measuresBills;
    AccountingLSBills * lumpSumBills;
    AccountingTAMBill * timeAndMaterialBill;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    unsigned int nextId;
    PaymentDataModel * dataModel;

    double totalAmountToDiscount;
    double amountNotToDiscount;
    double amountToDiscount;
    double amountDiscounted;
    double totalAmount;

    static int amountPrecision;
};

int ProjectAccountingParentItemPrivate::amountPrecision = 2;

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
    connect( m_d->measuresBills, &AccountingBills::insertPaymentsSignal, this, &ProjectAccountingParentItem::insertPayments );
    connect( m_d->measuresBills, &AccountingBills::removePaymentsSignal, this, &ProjectAccountingParentItem::removeBills );
    connect( m_d->measuresBills, &AccountingBills::changePaymentDateBeginSignal, this, &ProjectAccountingParentItem::changeBillDateBegin );
    connect( m_d->measuresBills, &AccountingBills::changePaymentDateEndSignal, this, &ProjectAccountingParentItem::changeBillDateEnd );

    connect( m_d->dataModel, &PaymentDataModel::insertPaymentsSignal, this, &ProjectAccountingParentItem::insertPayments );
    connect( m_d->dataModel, &PaymentDataModel::removePaymentsSignal, this, &ProjectAccountingParentItem::removeBills );
    connect( m_d->dataModel, &PaymentDataModel::modelChanged, this, &ProjectAccountingParentItem::modelChanged );

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

int ProjectAccountingParentItem::workProgressBillsCount() {
    return m_d->dataModel->paymentsCount();
}

PaymentData *ProjectAccountingParentItem::workProgressBillData(int pos) {
    return m_d->dataModel->billData( pos );
}

PaymentDataModel * ProjectAccountingParentItem::dataModel(){
    return m_d->dataModel;
}

void ProjectAccountingParentItem::updateAmounts() {
    m_d->totalAmountToDiscount = 0.0;
    m_d->amountNotToDiscount = 0.0;
    m_d->amountToDiscount = 0.0;
    m_d->amountDiscounted = 0.0;
    m_d->totalAmount = 0.0;
    for( int i=0; i < m_d->measuresBills->billCount(); ++i ){
        m_d->totalAmountToDiscount += m_d->measuresBills->bill(i)->totalAmountToDiscount();
        m_d->amountNotToDiscount += m_d->measuresBills->bill(i)->amountNotToDiscount();
        m_d->amountToDiscount += m_d->measuresBills->bill(i)->amountToDiscount();
        m_d->amountDiscounted += m_d->measuresBills->bill(i)->amountDiscounted();
        m_d->totalAmount += m_d->measuresBills->bill(i)->totalAmount();
    }
}

double ProjectAccountingParentItem::totalAmountToDiscount() {
    return m_d->totalAmountToDiscount;
}

double ProjectAccountingParentItem::amountNotToDiscount() {
    return m_d->amountNotToDiscount;
}

double ProjectAccountingParentItem::amountToDiscount() {
    return m_d->amountToDiscount;
}

double ProjectAccountingParentItem::amountDiscounted() {
    return m_d->amountDiscounted;
}

double ProjectAccountingParentItem::totalAmount() {
    return m_d->totalAmount;
}

QString ProjectAccountingParentItem::totalAmountToDiscountStr() {
    return m_d->amountToString( m_d->totalAmountToDiscount );
}

QString ProjectAccountingParentItem::amountNotToDiscountStr() {
    return m_d->amountToString( m_d->amountNotToDiscount );
}

QString ProjectAccountingParentItem::amountToDiscountStr() {
    return m_d->amountToString( m_d->amountToDiscount );
}

QString ProjectAccountingParentItem::amountDiscountedStr() {
    return m_d->amountToString( m_d->amountDiscounted );
}

QString ProjectAccountingParentItem::totalAmountStr() {
    return m_d->amountToString( m_d->totalAmount );
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

bool ProjectAccountingParentItem::clear() {
    bool ret = true;
    ret = ret && m_d->dataModel->clear();
    ret = m_d->measuresBills->clear() && ret;
    ret = m_d->lumpSumBills->clear() && ret;
    ret = m_d->timeAndMaterialBill->clear() && ret;
    return ret;
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
    writer->writeStartElement( "Accounting");
    m_d->dataModel->writeXml(writer);
    m_d->measuresBills->writeXml( writer );
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
        if( nodeName == "PAYMENTDATAS" && reader->isStartElement()) {
            m_d->dataModel->readXml( reader );
        }
        nodeName = reader->name().toString().toUpper();
        if( nodeName == "ACCOUNTINGBILLS" && reader->isStartElement()) {
            m_d->measuresBills->readXml( reader, priceLists );
        }
        nodeName = reader->name().toString().toUpper();
        if( nodeName == "ACCOUNTINGLSBILLS" && reader->isStartElement()) {
            m_d->lumpSumBills->readXml( reader, priceLists );
        }
        nodeName = reader->name().toString().toUpper();
        if( nodeName == "ACCOUNTINGTAMBILL" && reader->isStartElement()) {
            m_d->timeAndMaterialBill->readXml( reader, priceLists );
        }
    }
}

void ProjectAccountingParentItem::insertPayments( int position, int count) {
    for( int i=0; i < count; i++){
        m_d->measuresBills->insertPayments( position );
        m_d->dataModel->insertPayments( position );
        for( int j=0; j<m_d->measuresBills->billCount(); ++j ){
            AccountingBillItem * addedItem = m_d->measuresBills->bill(j)->item(position);
            m_d->dataModel->billData( position )->addBillItem( addedItem );
        }
    }
}

void ProjectAccountingParentItem::removeBills(int position, int count) {
    m_d->dataModel->removePayments( position, count);
    m_d->measuresBills->removeBills( position, count);
}

void ProjectAccountingParentItem::changeBillDateEnd(const QDate &newDate, int position) {
    m_d->dataModel->changePaymentDateEnd(newDate, position);
}

void ProjectAccountingParentItem::changeBillDateBegin(const QDate &newDate, int position) {
    m_d->dataModel->changePaymentDateBegin(newDate, position);
}
