#include "paymentdata.h"

#include "accountingbillitem.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
#include <QXmlStreamAttribute>
#include <QList>
#include <QDate>

class PaymentDataPrivate {
public:
    PaymentDataPrivate( PaymentData * parent,
                        PaymentData::DataType dType, AccountingBillItem * pay,
                        MathParser * prs ):
        parentData(parent),
        dataType(dType),
        parser(prs),
        dateBegin( QDate::currentDate() ),
        dateEnd( QDate::currentDate() ),
        associatedPayment(pay),
        totalAmountToDiscount(0.0),
        amountNotToDiscount(0.0),
        amountToDiscount(0.0),
        amountDiscounted(0.0),
        totalAmount(0.0){
    }
    QString	toString(double i, char f, int prec ) const{
        if( parser != NULL ){
            return parser->toString( i, f, prec );
        } else {
            return QString::number( i, f, prec );
        }
    }

    PaymentData * parentData;
    PaymentData::DataType dataType;
    MathParser * parser;
    QList<PaymentData *> childrenContainer;
    QDate dateBegin;
    QDate dateEnd;
    AccountingBillItem * associatedPayment;


    double totalAmountToDiscount;
    double amountNotToDiscount;
    double amountToDiscount;
    double amountDiscounted;
    double totalAmount;

    static int amountPrecision;
};

int PaymentDataPrivate::amountPrecision = 2;

PaymentData::PaymentData( PaymentData * parent,
                          PaymentData::DataType dType,
                          AccountingBillItem *pay,
                          MathParser * prs ) :
    QObject(),
    m_d( new PaymentDataPrivate(parent, dType, pay, prs) ){
    if( m_d->dataType == Payment ){
        PaymentData * lumpSumData = new PaymentData( this, PaymentData::LumpSum, pay, prs );
        m_d->childrenContainer << lumpSumData;
        PaymentData * ppuData = new PaymentData( this, PaymentData::PPU, pay, prs );
        m_d->childrenContainer << ppuData;
        PaymentData * tamData = new PaymentData( this, PaymentData::TimeAndMaterials, pay, prs );
        m_d->childrenContainer << tamData;
    }
}

PaymentData::~PaymentData(){
    for( int i=0; i < m_d->childrenContainer.size(); ++i ){
        delete m_d->childrenContainer.takeAt(0);
    }
}

PaymentData *PaymentData::parentData() {
    return m_d->parentData;
}

int PaymentData::childrenCount() const {
    return m_d->childrenContainer.size();
}

PaymentData *PaymentData::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

int PaymentData::childNumber() const {
    if( m_d->parentData != NULL ){
        return m_d->parentData->m_d->childrenContainer.indexOf( const_cast<PaymentData *>(this) );
    }
    return 0;
}

QString PaymentData::name() {
    if( m_d->dataType == Payment ){
        return trUtf8("S.A.L. N. %1 (%2-%3)").arg( QString::number(childNumber()+1),
                                                   m_d->parser->toString( m_d->dateBegin),
                                                   m_d->parser->toString( m_d->dateEnd) );
    } else if( m_d->dataType == PPU ){
        return trUtf8("Opere a misura");
    } else if( m_d->dataType == LumpSum ){
        return trUtf8("Opere a corpo");
    } else if( m_d->dataType == TimeAndMaterials ){
        return trUtf8("Opere in economia");
    }
    return QString();
}

QString PaymentData::totalAmount() {
    return m_d->toString( m_d->totalAmount, 'f', m_d->amountPrecision );
}

void PaymentData::updateAmounts() {
    if( m_d->dataType == PaymentData::PPU ){
        m_d->totalAmountToDiscount = m_d->associatedPayment->totalAmountToDiscount( AccountingBillItem::PPU );
        m_d->amountNotToDiscount = m_d->associatedPayment->amountNotToDiscount( AccountingBillItem::PPU );
        m_d->totalAmount = m_d->associatedPayment->totalAmount( AccountingBillItem::PPU );
    } else if( m_d->dataType == PaymentData::LumpSum ){
        m_d->totalAmountToDiscount = m_d->associatedPayment->totalAmountToDiscount( AccountingBillItem::LumpSum );
        m_d->amountNotToDiscount = m_d->associatedPayment->amountNotToDiscount( AccountingBillItem::LumpSum );
        m_d->totalAmount = m_d->associatedPayment->totalAmount( AccountingBillItem::LumpSum );
    } else if( m_d->dataType == PaymentData::TimeAndMaterials ){
        m_d->totalAmountToDiscount = m_d->associatedPayment->totalAmountToDiscount( AccountingBillItem::TimeAndMaterials );
        m_d->amountNotToDiscount = m_d->associatedPayment->amountNotToDiscount( AccountingBillItem::TimeAndMaterials );
        m_d->totalAmount = m_d->associatedPayment->totalAmount( AccountingBillItem::TimeAndMaterials );
    } else {
        for( QList<PaymentData *>::iterator chld = m_d->childrenContainer.begin(); chld != m_d->childrenContainer.end(); ++chld ){
            (*chld)->updateAmounts();
        }
        m_d->totalAmountToDiscount = m_d->associatedPayment->totalAmountToDiscount();
        m_d->amountNotToDiscount = m_d->associatedPayment->amountNotToDiscount();
        m_d->totalAmount = m_d->associatedPayment->totalAmount();
    }
}

QDate PaymentData::dateBegin() const {
    return m_d->dateBegin;
}

QDate PaymentData::dateEnd() const {
    return m_d->dateEnd;
}

AccountingBillItem *PaymentData::associatedPayment() {
    return m_d->associatedPayment;
}

void PaymentData::setAssociatedPayment(AccountingBillItem *newPayment) {
    if( m_d->associatedPayment != newPayment ){
        m_d->associatedPayment = newPayment;
        connect( newPayment, &AccountingBillItem::aboutToBeDeleted, this, static_cast<void(PaymentData::*)()>(&PaymentData::removeBillItem) );
        updateAmounts();
    }
}

void PaymentData::removeBillItem() {
    AccountingBillItem * billItem = dynamic_cast<AccountingBillItem *>(sender());
    if( billItem == m_d->associatedPayment ){
        m_d->associatedPayment = NULL;
        updateAmounts();
    }
}

bool PaymentData::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

bool PaymentData::insertPayment(int position, AccountingBillItem *pay) {
    if( m_d->dataType == Root ){
        if( (position >= 0) && (position <= m_d->childrenContainer.size()) ){
            PaymentData * newData = new PaymentData( this, PaymentData::Payment, pay, m_d->parser );
            m_d->childrenContainer.insert( position, newData );
            connect( newData, &PaymentData::dataChanged, this, &PaymentData::dataChanged );
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool PaymentData::removePayment(int position, AccountingBillItem *pay) {
    if( m_d->dataType == Root ){
        if( (position >= 0) && (position < m_d->childrenContainer.size()) ){
            if( m_d->childrenContainer.at(position)->m_d->associatedPayment == pay ){
                delete m_d->childrenContainer.takeAt( position);
                emit dataChanged();
                return true;
            }
        }
    }
    return false;
}

PaymentData::DataType PaymentData::dataType() {
    return m_d->dataType;
}
