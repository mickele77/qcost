#include "paymentdatamodel.h"

#include "paymentdata.h"

#include <QXmlStreamReader>
#include <QFont>
#include <QList>

class PaymentDataModelPrivate {
public:
    PaymentDataModelPrivate( MathParser * prs ):
        rootData( new PaymentData( NULL, PaymentData::Root, prs ) ) {
    }
    PaymentData * data(const QModelIndex &index ) const {
        if (index.isValid()) {
            return static_cast<PaymentData *>(index.internalPointer());
        }
        return rootData;
    }
    PaymentData * rootData;
};

PaymentDataModel::PaymentDataModel( MathParser * prs ):
    QAbstractItemModel(),
    m_d( new PaymentDataModelPrivate(prs) ){
    connect( m_d->rootData, &PaymentData::dataChanged, this, &PaymentDataModel::modelChanged );
}

void PaymentDataModel::writeXml( QXmlStreamWriter *writer ) {
    m_d->rootData->writeXml( writer );
}

void PaymentDataModel::readXml(QXmlStreamReader *reader ){
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "PAYMENTDATAS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "PAYMENTDATA" && reader->isStartElement()) {
            m_d->rootData->readXml( reader );
        }
    }
}

QModelIndex PaymentDataModel::parent(const QModelIndex &index) const {
    if ( !index.isValid() )
        return QModelIndex();

    PaymentData *childData = m_d->data(index);
    PaymentData *parentData = childData->parentData();

    if (parentData == m_d->rootData || parentData == NULL )
        return QModelIndex();

    return createIndex( parentData->childNumber(), 0, parentData );
}

QModelIndex PaymentDataModel::index(int row, int col, const QModelIndex &parent) const {
    if( parent.isValid() && parent.column() != 0)
        return QModelIndex();

    PaymentData *parentData = m_d->data(parent);

    if( parentData != NULL ){
        PaymentData *childData = dynamic_cast<PaymentData *>(parentData->child(row));
        if (childData) {
            return createIndex(row, col, childData);
        }
    }
    return QModelIndex();
}

PaymentData * PaymentDataModel::paymentData(int pos) {
    if( pos >= 0 && pos < m_d->rootData->childrenCount() ){
        return m_d->rootData->child( pos );
    }
    return NULL;
}

QVariant PaymentDataModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if( (role == Qt::EditRole) || (role == Qt::DisplayRole) ){
        PaymentData *data = m_d->data(index);
        if( index.column() == 0 ){
            return QVariant( data->name() );
        } else if( index.column() == 1 ){
            return QVariant( data->totalAmount() );
        }
    } else if( role == Qt::TextAlignmentRole ){
        if( index.column() == 0 ){
            return (Qt::AlignLeft + Qt::AlignVCenter);
        } else if( index.column() == 1 ){
            return (Qt::AlignRight + Qt::AlignVCenter);
        }
    } else if( role == Qt::FontRole ){
        PaymentData *data = m_d->data(index);
        if( data->dataType() == PaymentData::Payment ){
            QFont font;
            font.setItalic( true );
            return font;
        }
    }

    return QVariant();
}

QVariant PaymentDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if( (role == Qt::EditRole) || (role == Qt::DisplayRole) ){
        if (orientation == Qt::Horizontal ){
            switch( section ){
            case 0 :{
                return QVariant( trUtf8("S.A.L."));
                break;
            }
            case 1 :{
                return QVariant( trUtf8("Importo totale"));
                break;
            }
            }
        }
    }
    return QVariant();
}

int PaymentDataModel::rowCount(const QModelIndex &parent) const {
    PaymentData *parentItem = m_d->data(parent);
    return parentItem->childrenCount();
}

int PaymentDataModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 2;
}

bool PaymentDataModel::hasChildren(const QModelIndex &parent) const {
    PaymentData *parentItem = m_d->data(parent);
    return parentItem->hasChildren();
}

int PaymentDataModel::paymentsCount() const {
    return m_d->rootData->childrenCount();
}

void PaymentDataModel::insertPaymentsRequest(int position, int count) {
    emit insertPaymentsSignal( position, count );
}

void PaymentDataModel::removePaymentsRequest(int position, int count) {
    emit removePaymentsSignal( position, count );
}

void PaymentDataModel::changePaymentDateEnd(const QDate &newDate, int position) {
    if( position >= 0 && position < m_d->rootData->childrenCount() ){
        m_d->rootData->child(position)->setDateEnd( newDate );
    }
}

void PaymentDataModel::changePaymentDateBegin(const QDate &newDate, int position) {
    if( position >= 0 && position < m_d->rootData->childrenCount() ){
        m_d->rootData->child(position)->setDateBegin( newDate );
    }
}

bool PaymentDataModel::insertPayments(int position, int count) {
    bool ret = false;
    if( (position >= 0) && (position <= m_d->rootData->childrenCount()) ){
        beginInsertRows( QModelIndex(), position, position+count-1);
        ret = m_d->rootData->insertPayments( position, count );
        endInsertRows();
    }
    return ret;
}

bool PaymentDataModel::removePayments(int position, int purpCount) {
    if( purpCount <= 0 ){
        return true;
    }
    bool ret = false;
    if( (position >= 0) && (position < m_d->rootData->childrenCount()) ){
        int count = purpCount;
        if( (position+count) >= m_d->rootData->childrenCount() ){
            count = m_d->rootData->childrenCount() - position;
        }
        beginRemoveRows( QModelIndex(), position, position+count-1);
        ret = m_d->rootData->removePayments( position, count );
        endRemoveRows();
    }
    return ret;
}

bool PaymentDataModel::clear() {
    return removePayments( 0, m_d->rootData->childrenCount() );
}

Qt::ItemFlags PaymentDataModel::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void PaymentDataModel::updateAmounts() {
    m_d->rootData->updateAmounts();
}
