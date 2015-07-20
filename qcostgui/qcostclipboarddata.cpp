#include "qcostclipboarddata.h"

#include "project.h"
#include "accountinglsbill.h"
#include "accountinglsbillitem.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"
#include "accountingbill.h"
#include "accountingbillitem.h"
#include "bill.h"
#include "billitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"
#include "unitmeasure.h"

#include <QXmlStreamWriter>

class QCostClipboardDataPrivate{
public:
    QCostClipboardDataPrivate(){
    }
    QList<AccountingLSBill *> copiedAccLSBills;
    QCostClipboardData::Mode copiedAccLSBillsMode;
    QList<AccountingLSBillItem *> copiedAccLSBillItems;
    AccountingBill * copiedAccLSBillItemsBill;
    QCostClipboardData::Mode copiedAccLSBillItemsMode;

    QList<AccountingTAMBill *> copiedAccTAMBills;
    QCostClipboardData::Mode copiedAccTAMBillsMode;
    QList<AccountingTAMBillItem *> copiedAccTAMBillItems;
    AccountingBill * copiedAccTAMBillItemsBill;
    QCostClipboardData::Mode copiedAccTAMBillItemsMode;

    QList<AccountingBill *> copiedAccBills;
    QCostClipboardData::Mode copiedAccBillsMode;
    QList<AccountingBillItem *> copiedAccBillItems;
    AccountingBill * copiedAccBillItemsBill;
    QCostClipboardData::Mode copiedAccBillItemsMode;

    QList<Bill *> copiedBills;
    QCostClipboardData::Mode copiedBillsMode;
    QList<BillItem *> copiedBillItems;
    Bill * copiedBillItemsBill;
    QCostClipboardData::Mode copiedBillItemsMode;

    QList<PriceList *> copiedPriceLists;
    QCostClipboardData::Mode copiedPriceListsMode;

    QList<PriceItem *> copiedPriceItems;
    PriceList * copiedPriceItemsPriceList;
    QCostClipboardData::Mode copiedPriceItemsMode;
};

QCostClipboardData::QCostClipboardData() :
    QMimeData(),
    m_d( new QCostClipboardDataPrivate() ){
}

QCostClipboardData::~QCostClipboardData(){
    delete m_d;
}

QCostClipboardData &QCostClipboardData::operator=(const QCostClipboardData &cp) {
    if( &cp != this ){
        setCopiedAccountingBills( cp.m_d->copiedAccBills, cp.m_d->copiedAccBillsMode );
        setCopiedAccountingBillItems( cp.m_d->copiedAccBillItems, cp.m_d->copiedAccBillItemsBill, cp.m_d->copiedAccBillItemsMode );
        setCopiedBills( cp.m_d->copiedBills, cp.m_d->copiedBillItemsMode );
        setCopiedBillItems( cp.m_d->copiedBillItems, cp.m_d->copiedBillItemsBill, cp.m_d->copiedBillItemsMode );
        setCopiedPriceLists( cp.m_d->copiedPriceLists, cp.m_d->copiedPriceListsMode );
        setCopiedPriceItems( cp.m_d->copiedPriceItems, cp.m_d->copiedPriceItemsPriceList, cp.m_d->copiedPriceItemsMode );
    }

    return *this;
}

QList<AccountingBill *> QCostClipboardData::copiedAccountingBills() {
    return m_d->copiedAccBills;
}

QCostClipboardData::Mode QCostClipboardData::copiedAccountingBillItemsMode() const {
    return m_d->copiedAccBillItemsMode;
}

QList<Bill *> QCostClipboardData::copiedBills() {
    return m_d->copiedBills;
}

QCostClipboardData::Mode QCostClipboardData::copiedBillItemsMode() const {
    return m_d->copiedBillItemsMode;
}

void QCostClipboardData::removeFromList(){
    AccountingBill * a = dynamic_cast<AccountingBill *>(sender() );
    if( a != NULL ){
        if( m_d->copiedAccBills.contains( a ) ){
            m_d->copiedAccBills.removeAll(a);
        }
        if( m_d->copiedAccBillItemsBill == a ){
            m_d->copiedAccBillItemsBill = NULL;
            m_d->copiedAccBillItems.clear();
        }
    }

    AccountingBillItem * ai = dynamic_cast<AccountingBillItem *>(sender() );
    if( ai != NULL ){
        m_d->copiedAccBillItems.removeAll( ai );
        if( m_d->copiedAccBillItems.size() < 1 ){
            m_d->copiedAccBillItemsBill = NULL;
        }
    }

    Bill * b = dynamic_cast<Bill *>(sender() );
    if( b != NULL ){
        if( m_d->copiedBills.contains( b ) ){
            m_d->copiedBills.removeAll(b);
        }
        if( m_d->copiedBillItemsBill == b ){
            m_d->copiedBillItemsBill = NULL;
            m_d->copiedBillItems.clear();
        }
    }

    BillItem * bi = dynamic_cast<BillItem *>(sender() );
    if( bi != NULL ){
        m_d->copiedBillItems.removeAll( bi );
        if( m_d->copiedBillItems.size() < 1 ){
            m_d->copiedBillItemsBill = NULL;
        }
    }

    PriceList * pl = dynamic_cast<PriceList *>(sender() );
    if( pl != NULL ){
        if( m_d->copiedPriceLists.contains( pl ) ){
            m_d->copiedPriceLists.removeAll(pl);
        }
        if( m_d->copiedPriceItemsPriceList == pl ){
            m_d->copiedPriceItemsPriceList = NULL;
            m_d->copiedPriceItems.clear();
        }
    }

    PriceItem * pi = dynamic_cast<PriceItem *>(sender() );
    if( pi != NULL ){
        m_d->copiedPriceItems.removeAll( pi );
        if( m_d->copiedPriceItems.size() < 1 ){
            m_d->copiedPriceItemsPriceList = NULL;
        }
    }

}

void QCostClipboardData::setCopiedAccountingBills(QList<AccountingBill *> ca, QCostClipboardData::Mode m) {
    m_d->copiedAccBills = ca;
    m_d->copiedAccBillsMode = m;
    for( QList<AccountingBill *>::iterator i = m_d->copiedAccBills.begin(); i != m_d->copiedAccBills.end(); ++i ){
        connect( (*i), &AccountingBill::aboutToBeDeleted, this, &QCostClipboardData::removeFromList );
    }
}

void QCostClipboardData::getCopiedAccountingBillItems( QList<AccountingBillItem *> * accountingItems,
                                                       AccountingBill * &accounting,
                                                       QCostClipboardData::Mode * mode ) const{
    *accountingItems = m_d->copiedAccBillItems;
    accounting = m_d->copiedAccBillItemsBill;
    *mode = m_d->copiedAccBillItemsMode;
}

void QCostClipboardData::setCopiedAccountingBillItems(QList<AccountingBillItem *> ai, AccountingBill *a, QCostClipboardData::Mode m) {
    m_d->copiedAccBillItems = ai;
    m_d->copiedAccBillItemsMode = m;
    m_d->copiedAccBillItemsBill = a;
    for( QList<AccountingBillItem *>::iterator i = m_d->copiedAccBillItems.begin(); i != m_d->copiedAccBillItems.end(); ++i ){
        connect( (*i), &AccountingBillItem::aboutToBeDeleted, this, &QCostClipboardData::removeFromList );
    }
}

void QCostClipboardData::setCopiedBills(QList<Bill *> cb, QCostClipboardData::Mode m) {
    m_d->copiedBills = cb;
    m_d->copiedBillsMode = m;
    for( QList<Bill *>::iterator i = m_d->copiedBills.begin(); i != m_d->copiedBills.end(); ++i ){
        connect( (*i), &Bill::aboutToBeDeleted, this, &QCostClipboardData::removeFromList );
    }
}

void QCostClipboardData::getCopiedBillItems( QList<BillItem *> * billItems,
                                             Bill * &bill,
                                             QCostClipboardData::Mode * mode ) const{
    *billItems = m_d->copiedBillItems;
    bill = m_d->copiedBillItemsBill;
    *mode = m_d->copiedBillItemsMode;
}

void QCostClipboardData::setCopiedBillItems(QList<BillItem *> bi, Bill *b, QCostClipboardData::Mode m) {
    m_d->copiedBillItems = bi;
    m_d->copiedBillItemsMode = m;
    m_d->copiedBillItemsBill = b;
    for( QList<BillItem *>::iterator i = m_d->copiedBillItems.begin(); i != m_d->copiedBillItems.end(); ++i ){
        connect( (*i), &BillItem::aboutToBeDeleted, this, &QCostClipboardData::removeFromList );
    }
}

void QCostClipboardData::updateText(){
    // TODO

    QString ret;
    QXmlStreamWriter writer( & ret );
    writer.setAutoFormatting(true);
    writer.setCodec("UTF-8");

    writer.writeStartElement( "copiedBillItems");

    Project p;
    for( QList<BillItem *>::iterator i = m_d->copiedBillItems.begin(); i != m_d->copiedBillItems.end(); ++i ){
        if( (*i)->priceItem() ){
            if( (*i)->priceItem()->unitMeasure()){
                if( p.unitMeasureModel()->findTag( (*i)->priceItem()->unitMeasure()->tag() ) < 0 ){
                    p.unitMeasureModel()->append((*i)->priceItem()->unitMeasure()->tag());
                }
            }

        }
    }

    writer.writeEndElement();

    setText( ret );
}

QList<PriceList *> QCostClipboardData::copiedPriceLists(){
    return m_d->copiedPriceLists;
}

QCostClipboardData::Mode QCostClipboardData::copiedPriceListsMode(){
    return m_d->copiedPriceListsMode;
}

void QCostClipboardData::setCopiedPriceLists(QList<PriceList *> pl, QCostClipboardData::Mode m) {
    m_d->copiedPriceLists = pl;
    m_d->copiedPriceListsMode = m;
    for( QList<PriceList *>::iterator i = m_d->copiedPriceLists.begin(); i != m_d->copiedPriceLists.end(); ++i ){
        connect( (*i), &PriceList::aboutToBeDeleted, this, &QCostClipboardData::removeFromList );
    }
}

QList<PriceItem *> QCostClipboardData::copiedPriceItems(){
    return m_d->copiedPriceItems;
}

PriceList * QCostClipboardData::copiedPriceItemsPriceList(){
    return m_d->copiedPriceItemsPriceList;
}

QCostClipboardData::Mode QCostClipboardData::copiedPriceItemsMode(){
    return m_d->copiedPriceItemsMode;
}

void QCostClipboardData::setCopiedPriceItems(QList<PriceItem *> pi, PriceList *pl, QCostClipboardData::Mode m) {
    m_d->copiedPriceItems = pi;
    m_d->copiedPriceItemsMode = m;
    m_d->copiedPriceItemsPriceList = pl;
    for( QList<PriceItem *>::iterator i = m_d->copiedPriceItems.begin(); i != m_d->copiedPriceItems.end(); ++i ){
        connect( (*i), &PriceItem::aboutToBeDeleted, this, &QCostClipboardData::removeFromList );
    }
}

void QCostClipboardData::getCopiedPriceItems( QList<PriceItem *> *priceItems, PriceList * &priceList, QCostClipboardData::Mode *mode) const{
    *priceItems = m_d->copiedPriceItems;
    priceList = m_d->copiedPriceItemsPriceList;
    *mode = m_d->copiedPriceItemsMode;
}
