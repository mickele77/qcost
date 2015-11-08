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
    PaymentDataPrivate( PaymentData * pItem, PaymentData::DataType dType, MathParser * prs ):
        parentData(pItem),
        dataType(dType),
        parser(prs),
        dateBegin( QDate::currentDate() ),
        dateEnd( QDate::currentDate() ),
        totalAmountToDiscount(0.0),
        PPUTotalAmountToDiscount(0.0),
        LSTotalAmountToDiscount(0.0),
        TAMTotalAmountToDiscount(0.0),
        amountNotToDiscount(0.0),
        PPUAmountNotToDiscount(0.0),
        LSAmountNotToDiscount(0.0),
        TAMAmountNotToDiscount(0.0),
        amountToDiscount(0.0),
        PPUAmountToDiscount(0.0),
        LSAmountToDiscount(0.0),
        TAMAmountToDiscount(0.0),
        amountDiscounted(0.0),
        PPUAmountDiscounted(0.0),
        LSAmountDiscounted(0.0),
        TAMAmountDiscounted(0.0),
        totalAmount(0.0),
        PPUTotalAmount(0.0),
        LSTotalAmount(0.0),
        TAMTotalAmount(0.0){
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
    QList<AccountingBillItem *> billItemContainer;

    double totalAmountToDiscount;
    double PPUTotalAmountToDiscount;
    double LSTotalAmountToDiscount;
    double TAMTotalAmountToDiscount;

    double amountNotToDiscount;
    double PPUAmountNotToDiscount;
    double LSAmountNotToDiscount;
    double TAMAmountNotToDiscount;

    double amountToDiscount;
    double PPUAmountToDiscount;
    double LSAmountToDiscount;
    double TAMAmountToDiscount;

    double amountDiscounted;
    double PPUAmountDiscounted;
    double LSAmountDiscounted;
    double TAMAmountDiscounted;

    double totalAmount;
    double PPUTotalAmount;
    double LSTotalAmount;
    double TAMTotalAmount;

    static int amountPrecision;
};

int PaymentDataPrivate::amountPrecision = 2;

PaymentData::PaymentData( PaymentData * parent,
                          PaymentData::DataType dType,
                          MathParser * prs ) :
    QObject(),
    m_d( new PaymentDataPrivate(parent, dType, prs) ){
    if( m_d->dataType == Payment ){
        PaymentData * lumpSumData = new PaymentData( this, PaymentData::LumpSum, prs );
        m_d->childrenContainer << lumpSumData;
        PaymentData * ppuData = new PaymentData( this, PaymentData::PPU, prs );
        m_d->childrenContainer << ppuData;
        PaymentData * tamData = new PaymentData( this, PaymentData::TimeAndMaterials, prs );
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
    if( m_d->dataType == Payment ){
        return m_d->toString( m_d->totalAmount, 'f', m_d->amountPrecision );
    } else if( m_d->dataType == PPU ){
        return m_d->toString( m_d->PPUTotalAmount, 'f', m_d->amountPrecision );
    } else if( m_d->dataType == LumpSum ){
        return m_d->toString( m_d->LSTotalAmount, 'f', m_d->amountPrecision );
    } else if( m_d->dataType == TimeAndMaterials ){
        return m_d->toString( m_d->TAMTotalAmount, 'f', m_d->amountPrecision );
    }
    return QString();
}

void PaymentData::updateAmounts() {
    for( QList<AccountingBillItem *>::iterator i=m_d->billItemContainer.begin(); i != m_d->billItemContainer.end(); ++i ){
        m_d->totalAmountToDiscount = (*i)->totalAmountToDiscount();
        m_d->PPUTotalAmountToDiscount = (*i)->totalAmountToDiscount( AccountingBillItem::PPU );
        m_d->LSTotalAmountToDiscount = (*i)->totalAmountToDiscount( AccountingBillItem::LumpSum );
        m_d->TAMTotalAmountToDiscount = (*i)->totalAmountToDiscount( AccountingBillItem::TimeAndMaterials );

        m_d->amountNotToDiscount = (*i)->amountNotToDiscount();
        m_d->PPUAmountNotToDiscount = (*i)->amountNotToDiscount( AccountingBillItem::PPU );
        m_d->LSAmountNotToDiscount = (*i)->amountNotToDiscount( AccountingBillItem::LumpSum );
        m_d->TAMAmountNotToDiscount = (*i)->amountNotToDiscount( AccountingBillItem::TimeAndMaterials );

        m_d->totalAmount = (*i)->totalAmount();
        m_d->PPUTotalAmount = (*i)->totalAmount( AccountingBillItem::PPU );
        m_d->LSTotalAmount = (*i)->totalAmount( AccountingBillItem::LumpSum );
        m_d->TAMTotalAmount = (*i)->totalAmount( AccountingBillItem::TimeAndMaterials );
    }
}

QDate PaymentData::dateBegin() const {
    return m_d->dateBegin;
}

void PaymentData::setDateBegin(const QDate &newDate) {
    if( m_d->dataType == Payment ){
        if( newDate != m_d->dateBegin ){
            m_d->dateBegin = newDate;
            for( QList<AccountingBillItem *>::iterator i = m_d->billItemContainer.begin();
                 i != m_d->billItemContainer.end(); ++i ){
                (*i)->setDateBegin( newDate );
            }
            emit dataChanged();
        }
    }
}

void PaymentData::setDateBegin( const QString & newDate ) {
    QDate purpNewDate = m_d->parser->evaluateDate( newDate );
    setDateBegin( purpNewDate );
}

QDate PaymentData::dateEnd() const {
    return m_d->dateEnd;
}

void PaymentData::setDateEnd(const QDate &newDate) {
    if( m_d->dataType == Payment ){
        if( newDate != m_d->dateEnd ){
            m_d->dateEnd = newDate;
            for( QList<AccountingBillItem *>::iterator i = m_d->billItemContainer.begin();
                 i != m_d->billItemContainer.end(); ++i ){
                (*i)->setDateEnd( newDate );
            }
            emit dataChanged();
        }
    }
}

void PaymentData::setDateEnd( const QString & newDate ) {
    QDate purpNewDate = m_d->parser->evaluateDate( newDate );
    setDateEnd( purpNewDate );
}

void PaymentData::addBillItem(AccountingBillItem *billItem) {
    if( !m_d->billItemContainer.contains(billItem)){
        m_d->billItemContainer.append( billItem );
        billItem->setDateBegin( m_d->dateBegin );
        billItem->setDateEnd( m_d->dateEnd );
        connect( billItem, &AccountingBillItem::aboutToBeDeleted, this, static_cast<void(PaymentData::*)()>(&PaymentData::removeBillItem) );
    }
    updateAmounts();
}

void PaymentData::removeBillItem() {
    AccountingBillItem * billItem = dynamic_cast<AccountingBillItem *>(sender());
    if( billItem ){
        m_d->billItemContainer.removeAll( billItem );
    }
    updateAmounts();
}

void PaymentData::removeBillItem(AccountingBillItem *billItem) {
    m_d->billItemContainer.removeAll( billItem );
    updateAmounts();
}

bool PaymentData::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

bool PaymentData::insertPayments(int position, int count) {
    if( m_d->dataType == Root ){
        if( (position >= 0) && (position <= m_d->childrenContainer.size()) ){
            for( int i=0; i < count; ++i ){
                PaymentData * newData = new PaymentData( this, PaymentData::Payment, m_d->parser );
                m_d->childrenContainer.insert( position, newData );
                connect( newData, &PaymentData::dataChanged, this, &PaymentData::dataChanged );
            }
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool PaymentData::appendPayments( int count) {
    return insertPayments(m_d->childrenContainer.size(), count );
}

bool PaymentData::removePayments(int position, int purpCount) {
    if( m_d->dataType == Root ){
        if( (position >= 0) && (position < m_d->childrenContainer.size()) ){
            int count = purpCount;
            if( (position+count) >= m_d->childrenContainer.size() ){
                count = m_d->childrenContainer.size() - position;
            }
            for( int i=0; i < count; ++i ){
                delete m_d->childrenContainer.takeAt( position);
            }
            emit dataChanged();
            return true;
        }
    }
    return false;
}

PaymentData::DataType PaymentData::dataType() {
    return m_d->dataType;
}
