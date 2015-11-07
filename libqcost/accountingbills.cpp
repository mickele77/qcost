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
#include "accountingbills.h"

#include "paymentdatamodel.h"
#include "accountingbill.h"
#include "paymentdata.h"
#include "projectaccountingparentitem.h"
#include "priceitem.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QObject>
#include <QVariant>
#include <QDate>
#include <QList>

class AccountingBillsPrivate{
public:
    AccountingBillsPrivate( PriceFieldModel * pfm, MathParser * p = NULL ):
        priceFieldModel(pfm),
        parser(p),
        nextId(1){
    }
    QList<AccountingBill *> billContainer;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    unsigned int nextId;
};

AccountingBills::AccountingBills(ProjectItem *parent, PriceFieldModel * pfm, MathParser * prs ):
    ProjectItem(parent),
    m_d( new AccountingBillsPrivate( pfm, prs ) ){
}

int AccountingBills::billCount() {
    return m_d->billContainer.size();
}

AccountingBill *AccountingBills::bill(int i) {
    if( i > -1 && i < m_d->billContainer.size() ){
        return m_d->billContainer[i];
    }
    return NULL;
}

ProjectItem *AccountingBills::child(int number) {
    if( number >= 0 && number < m_d->billContainer.size() ){
        return m_d->billContainer[number];
    }
    return NULL;
}

int AccountingBills::childCount() const {
    return m_d->billContainer.size();
}

int AccountingBills::childNumber(ProjectItem *item) {
    AccountingBill * b = dynamic_cast<AccountingBill *>(item);
    if( b ){
        return m_d->billContainer.indexOf( b );
    }
    return -1;
}

bool AccountingBills::canChildrenBeInserted() {
    return true;
}

bool AccountingBills::insertChildren(int position, int count) {
    if (position < 0 || position > m_d->billContainer.size())
        return false;

    emit beginInsertChildren( this, position, position+count-1);

    for (int row = 0; row < count; ++row) {
        QString purposedBillName = QString("%1 %2").arg(trUtf8("Libretto"), QString::number(m_d->nextId++));
        QList<AccountingBill *>::iterator i = m_d->billContainer.begin();
        while( i != m_d->billContainer.end() ){
            if( (*i)->name().toUpper() == purposedBillName.toUpper() ){
                purposedBillName = QString("%1 %2").arg(trUtf8("Libretto"), QString::number(m_d->nextId++));
                i = m_d->billContainer.begin();
            } else {
                i++;
            }
        }
        AccountingBill *item = new AccountingBill( purposedBillName, this, m_d->priceFieldModel, m_d->parser );
        ProjectAccountingParentItem * pItem = dynamic_cast<ProjectAccountingParentItem *>(parentItem());
        if( pItem ){
            for( int i=0; i < pItem->paymentDatasCount(); ++i ){
                item->insertPayments( i );
                item->item(i)->setDateBegin( pItem->paymentData( i )->dateBegin() );
                item->item(i)->setDateEnd( pItem->paymentData( i )->dateEnd() );
            }
        }
        connect( item, &AccountingBill::modelChanged, this, &AccountingBills::modelChanged );
        connect( item, &AccountingBill::requestInsertBills, this, &AccountingBills::insertPaymentsSignal );
        connect( item, &AccountingBill::requestRemoveBills, this, &AccountingBills::removePaymentsSignal );
        connect( item, &AccountingBill::requestDateBeginChange, this, &AccountingBills::changePaymentDateBeginSignal );
        connect( item, &AccountingBill::requestDateEndChange, this, &AccountingBills::changePaymentDateEndSignal );
        m_d->billContainer.insert(position, item);
        emit modelChanged();
    }

    emit endInsertChildren();

    return true;
}

bool AccountingBills::appendChild() {
    return insertChildren( m_d->billContainer.size(), 1 );
}

bool AccountingBills::removeChildren(int position, int count) {
    if( count <= 0 ){
        return true;
    }

    if (position < 0
            || (position + count) > m_d->billContainer.size()
            || count < 1 )
        return false;

    emit beginRemoveChildren( this, position, position+count-1);

    for (int row = 0; row < count; ++row){
        AccountingBill * item = m_d->billContainer.at( position );
        disconnect( item, &AccountingBill::modelChanged, this, &AccountingBills::modelChanged );
        delete item;
        m_d->billContainer.removeAt(  position );
    }

    emit endRemoveChildren();

    return true;
}

Qt::ItemFlags AccountingBills::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant AccountingBills::data() const {
    return QVariant( QObject::trUtf8("Libretti delle Misure") );
}

bool AccountingBills::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}

bool AccountingBills::isUsingPriceList(PriceList *pl) {
    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        if( (*i)->priceList() == pl ){
            return true;
        }
    }
    return false;
}

bool AccountingBills::isUsingPriceItem(PriceItem *p) {
    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        if( (*i)->isUsingPriceItem( p ) ){
            return true;
        }
        if( p->hasChildren() ){
            for( int i=0; i < p->childrenCount(); ++i ){
                bool ret = isUsingPriceItem( p->childItem(i) );
                if( ret ){
                    return true;
                }
            }
        }
    }
    return false;
}

void AccountingBills::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingBills");
    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}


void AccountingBills::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGBILLS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ACCOUNTINGBILL" && reader->isStartElement()) {
            if(appendChild()){
                m_d->billContainer.last()->readXml( reader, priceLists );
            }
        }
    }
}

void AccountingBills::loadTmpData( ProjectPriceListParentItem * priceLists, AccountingLSBills * lsBills, AccountingTAMBill * tamBill ) {
    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->loadTmpData( priceLists, lsBills, tamBill );
    }
}

bool AccountingBills::insertPayments(int position, int count) {
    bool ret = true;

    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        ret = ret && (*i)->insertPayments( position, count );
    }

    return ret;
}

bool AccountingBills::removeBills(int position, int count) {
    bool ret = true;

    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        ret = ret && (*i)->removePayments( position, count );
    }

    return ret;
}

void AccountingBills::changeBillDateEnd(const QDate &newDate, int position) {
    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->setBillDateEnd( newDate, position );
    }
}

void AccountingBills::changeBillDateBegin(const QDate &newDate, int position) {
    for( QList<AccountingBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->setBillDateBegin( newDate, position );
    }
}

bool AccountingBills::clear() {
    bool ret = removeChildren( 0, m_d->billContainer.size() );
    if( ret ){
        m_d->nextId = 1;
    }
    return ret;
}
