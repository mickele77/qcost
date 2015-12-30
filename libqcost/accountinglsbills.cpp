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
#include "accountinglsbills.h"

#include "accountinglsbill.h"
#include "priceitem.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QObject>
#include <QVariant>
#include <QList>

class AccountingLSBillsPrivate{
public:
    AccountingLSBillsPrivate( PriceFieldModel * pfm, MathParser * p = NULL ):
        priceFieldModel(pfm),
        parser(p),
        nextId(1){
    }
    QList<AccountingLSBill *> billContainer;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    unsigned int nextId;
};

AccountingLSBills::AccountingLSBills(ProjectItem *parent, PriceFieldModel * pfm, MathParser * prs ):
    ProjectItem(parent),
    m_d( new AccountingLSBillsPrivate( pfm, prs ) ){
}

int AccountingLSBills::billCount() {
    return m_d->billContainer.size();
}

AccountingLSBill *AccountingLSBills::bill(int i) {
    if( i > -1 && i < m_d->billContainer.size() ){
        return m_d->billContainer[i];
    }
    return NULL;
}

AccountingLSBill *AccountingLSBills::billId(unsigned int dd) {
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i){
        if( (*i)->id() == dd ){
            return (*i);
        }
    }
    return NULL;
}

ProjectItem *AccountingLSBills::child(int number) {
    if( number >= 0 && number < m_d->billContainer.size() ){
        return m_d->billContainer[number];
    }
    return NULL;
}

int AccountingLSBills::childCount() const {
    return m_d->billContainer.size();
}

int AccountingLSBills::childNumber(ProjectItem *item) {
    AccountingLSBill * b = dynamic_cast<AccountingLSBill *>(item);
    if( b ){
        return m_d->billContainer.indexOf( b );
    }
    return -1;
}

bool AccountingLSBills::canChildrenBeInserted() {
    return true;
}

bool AccountingLSBills::insertChildren(int position, int count) {
    if (position < 0 || position > m_d->billContainer.size())
        return false;

    emit beginInsertChildren( this, position, position+count-1);

    for (int row = 0; row < count; ++row) {
        QString purposedBillCode = QString("%1 %2").arg( "C", QString::number(m_d->nextId));
        QString purposedBillName = QString("%1 %2").arg(trUtf8("Categoria"), QString::number(m_d->nextId++));
        QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin();
        while( i != m_d->billContainer.end() ){
            if( ((*i)->code().toUpper() == purposedBillCode.toUpper()) ||
                  ((*i)->name().toUpper() == purposedBillName.toUpper()) ){
                purposedBillCode = QString("%1 %2").arg( "C", QString::number(m_d->nextId));
                purposedBillName = QString("%1 %2").arg(trUtf8("Categoria"), QString::number(m_d->nextId++));
                i = m_d->billContainer.begin();
            } else {
                i++;
            }
        }

        AccountingLSBill *item = new AccountingLSBill( purposedBillCode, purposedBillName,
                                                       this, m_d->priceFieldModel, m_d->parser );
        connect( item, &AccountingLSBill::modelChanged, this, &AccountingLSBills::modelChanged );
        m_d->billContainer.insert(position, item);
    }

    emit endInsertChildren();

    return true;
}

bool AccountingLSBills::appendChild() {
    return insertChildren( m_d->billContainer.size(), 1 );
}

bool AccountingLSBills::removeChildren(int position, int count) {
    if( count <= 0 ){
        return true;
    }

    if (position < 0
            || (position + count) > m_d->billContainer.size()
            || count < 1 )
        return false;

    emit beginRemoveChildren( this, position, position+count-1);

    for (int row = 0; row < count; ++row){
        AccountingLSBill * item = m_d->billContainer.at( position );
        disconnect( item, &AccountingLSBill::modelChanged, this, &AccountingLSBills::modelChanged );
        delete item;
        m_d->billContainer.removeAt(  position );
    }

    emit endRemoveChildren();

    return true;
}

Qt::ItemFlags AccountingLSBills::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant AccountingLSBills::data() const {
    return QVariant( QObject::trUtf8("Categorie a corpo") );
}

bool AccountingLSBills::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}

bool AccountingLSBills::isUsingPriceList(PriceList *pl) {
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        if( (*i)->priceList() == pl ){
            return true;
        }
    }
    return false;
}

bool AccountingLSBills::isUsingPriceItem(PriceItem *p) {
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
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

void AccountingLSBills::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingLSBills");
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void AccountingLSBills::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILLS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ACCOUNTINGLSBILL" && reader->isStartElement()) {
            if(appendChild()){
                m_d->billContainer.last()->readXml( reader, priceLists );
            }
        }
    }
}

bool AccountingLSBills::clear() {
    bool ret = removeChildren( 0, m_d->billContainer.size() );
    if( ret ){
        m_d->nextId = 1;
    }
    return ret;
}
