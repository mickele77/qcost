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
#include "attributesmodel.h"
#include "mathparser.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QObject>
#include <QVariant>
#include <QList>

class AccountingLSBillsPrivate{
public:
    AccountingLSBillsPrivate( AccountingLSBills * lsBills, PriceFieldModel * pfm, MathParser * prs = nullptr ):
        priceFieldModel(pfm),
        attributesModel( new AttributesModel( lsBills, prs, pfm ) ),
        parser(prs),
        nextId(1),
        projAmount(0.0),
        accAmount(0.0),
        percentageAccounted(1.0){
    }
    ~AccountingLSBillsPrivate(){
        delete attributesModel;
    }

    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser != nullptr ){
            return parser->toString( i, f, prec );
        } else {
            return QString::number( i, f, prec );
        }
    }

    QList<AccountingLSBill *> billContainer;
    PriceFieldModel * priceFieldModel;
    AttributesModel * attributesModel;
    MathParser * parser;
    unsigned int nextId;
    double projAmount;
    double accAmount;
    double percentageAccounted;

    static int amountPrecision;
    static int percentagePrecision;
};

int AccountingLSBillsPrivate::amountPrecision = 2;
int AccountingLSBillsPrivate::percentagePrecision = 6;

AccountingLSBills::AccountingLSBills(ProjectItem *parent, PriceFieldModel * pfm, MathParser * prs ):
    ProjectItem(parent),
    m_d( new AccountingLSBillsPrivate( this, pfm, prs ) ){
    connect( m_d->attributesModel, &AttributesModel::modelChanged, this, &AccountingLSBills::modelChanged );
}

AccountingLSBills::~AccountingLSBills(){
    delete m_d;
}

bool AccountingLSBills::isEmpty() {
    return (m_d->billContainer.size() == 0);
}

int AccountingLSBills::billCount() {
    return m_d->billContainer.size();
}

AccountingLSBill *AccountingLSBills::bill(int i) {
    if( i > -1 && i < m_d->billContainer.size() ){
        return m_d->billContainer[i];
    }
    return nullptr;
}

AccountingLSBill *AccountingLSBills::billId(unsigned int dd) {
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i){
        if( (*i)->id() == dd ){
            return (*i);
        }
    }
    return nullptr;
}

ProjectItem *AccountingLSBills::child(int number) {
    if( number >= 0 && number < m_d->billContainer.size() ){
        return m_d->billContainer[number];
    }
    return nullptr;
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
        QString purposedBillName = QString("%1 %2").arg(tr("Categoria"), QString::number(m_d->nextId++));
        QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin();
        while( i != m_d->billContainer.end() ){
            if( ((*i)->code().toUpper() == purposedBillCode.toUpper()) ||
                    ((*i)->name().toUpper() == purposedBillName.toUpper()) ){
                purposedBillCode = QString("%1 %2").arg( "C", QString::number(m_d->nextId));
                purposedBillName = QString("%1 %2").arg(tr("Categoria"), QString::number(m_d->nextId++));
                i = m_d->billContainer.begin();
            } else {
                i++;
            }
        }

        AccountingLSBill *item = new AccountingLSBill( purposedBillCode, purposedBillName,
                                                       this, m_d->priceFieldModel, m_d->attributesModel, m_d->parser );
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
    return QVariant( QObject::tr("Categorie a corpo") );
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

void AccountingLSBills::writeXml20(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "AccountingLSBills");
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->writeXml20( writer );
    }
    writer->writeEndElement();
}

void AccountingLSBills::readXml20(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILLS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ACCOUNTINGLSBILL" && reader->isStartElement()) {
            if(appendChild()){
                m_d->billContainer.last()->readXml20( reader, priceLists );
            }
        }
    }
}

double AccountingLSBills::projAmount() const {
    return m_d->projAmount;
}

QString AccountingLSBills::projAmountStr() const {
    return m_d->toString( m_d->projAmount, 'f', m_d->amountPrecision );
}

double AccountingLSBills::accAmount() const {
    return m_d->accAmount;
}

QString AccountingLSBills::accAmountStr() const {
    return m_d->toString( m_d->accAmount, 'f', m_d->amountPrecision );
}

double AccountingLSBills::percentageAccounted() const {
    return m_d->percentageAccounted;
}

QString AccountingLSBills::percentageAccountedStr() const {
    return QString("%1 %").arg( m_d->toString( m_d->percentageAccounted * 100.0, 'f', m_d->percentagePrecision ) );
}

void AccountingLSBills::updateAmounts() {
    m_d->projAmount = 0.0;
    m_d->accAmount = 0.0;
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        m_d->projAmount += (*i)->projAmount();
        m_d->accAmount += (*i)->accAmount();
    }
    m_d->percentageAccounted = 1.0;
    if( m_d->projAmount != 0.0 ){
        m_d->percentageAccounted = m_d->accAmount / m_d->projAmount;
    }
}

AttributesModel *AccountingLSBills::attributesModel() {
    return m_d->attributesModel;
}

void AccountingLSBills::activateAttributeModel() {
    if( m_d->attributesModel != nullptr ){
        m_d->attributesModel->setBill( this );
    }
}

double AccountingLSBills::projAmountAttribute(Attribute *attr) {
    double ret = 0.0;
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        ret += (*i)->PPUTotalToDiscount( attr );
    }
    return ret;
}

QString AccountingLSBills::projAmountAttributeStr(Attribute *attr) {
    return m_d->toString( projAmountAttribute(attr), 'f', m_d->amountPrecision );
}

double AccountingLSBills::accAmountAttribute(Attribute *attr) {
    double ret = 0.0;
    for( QList<AccountingLSBill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        ret += (*i)->accAmountAttribute( attr );
    }
    return ret;
}

QString AccountingLSBills::accAmountAttributeStr(Attribute *attr) {
    return m_d->toString( accAmountAttribute(attr), 'f', m_d->amountPrecision );
}

bool AccountingLSBills::clear() {
    bool ret = removeChildren( 0, m_d->billContainer.size() );
    if( ret ){
        m_d->nextId = 1;
    }
    return ret;
}
