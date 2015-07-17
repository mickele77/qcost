#include "billdelegate.h"

#include "unitmeasurelist.h"
#include "unitmeasure.h"
#include <QComboBox>
#include <QVariant>

class BillDelegatePrivate{
public:
    BillDelegatePrivate(UnitMeasureList * uml, PriceList * pl ):
        unitMeasureList(uml),
        priceList(pl){
    };
    UnitMeasureList * unitMeasureList;
    PriceList * priceList;
};

BillDelegate::BillDelegate(UnitMeasureList * uml, PriceList * pl, QObject *parent) :
    QStyledItemDelegate(parent),
    m_d( new BillDelegatePrivate(uml, pl)) {
}

BillDelegate::~BillDelegate() {
}

QWidget *BillDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if(index.column() == 0){
        QComboBox *cb = new QComboBox(parent);
        /* for(int i=0; i < m_d->unitMeasureList->size(); ++i ){
            UnitMeasure * um = m_d->unitMeasureList->value(i);
            QVariant v = qVariantFromValue((void *) um );
            cb->addItem( um->tag(), v );
        }*/
        return cb;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void BillDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        QVariant v = index.data(Qt::EditRole);
        int cbIndex = cb->findData( v );
        if(cbIndex >= 0){
            cb->setCurrentIndex(cbIndex);
        }
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void BillDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->itemData( cb->currentIndex() ), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

