#include "paymentdatamodel.h"

#include "paymentdata.h"

#include <QXmlStreamReader>
#include <QFont>
#include <QList>

class PaymentDataModelPrivate {
public:
    PaymentDataModelPrivate( AccountingBillItem * rootItem, MathParser * prs ):
        rootData( new PaymentData( NULL, PaymentData::Root, rootItem, prs ) ),
        parser(prs){
    }
    PaymentData * data(const QModelIndex &index ) const {
        if (index.isValid()) {
            return static_cast<PaymentData *>(index.internalPointer());
        }
        return rootData;
    }
    PaymentData * rootData;
    MathParser * parser;
};

PaymentDataModel::PaymentDataModel( AccountingBillItem * rootItem, MathParser * prs ):
    QAbstractItemModel(),
    m_d( new PaymentDataModelPrivate(rootItem, prs) ){
    connect( m_d->rootData, &PaymentData::dataChanged, this, &PaymentDataModel::modelChanged );
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

Qt::ItemFlags PaymentDataModel::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void PaymentDataModel::updateAmounts() {
    m_d->rootData->updateAmounts();
}

void PaymentDataModel::insertPayment(int payNum, AccountingBillItem *pay) {
    if( payNum > -1 && payNum <= m_d->rootData->childrenCount() ){
        beginInsertRows( QModelIndex(), payNum, payNum );
        m_d->rootData->insertPayment( payNum, pay );
        endInsertRows();
    }
}

void PaymentDataModel::removePayment(int payNum, AccountingBillItem *pay) {
    if( payNum > -1 && payNum < m_d->rootData->childrenCount() ){
        beginRemoveRows( QModelIndex(), payNum, payNum );
        m_d->rootData->removePayment( payNum, pay );
        endRemoveRows();
    }
}
