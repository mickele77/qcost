#include "pricefieldtabledelegate.h"

#include "pricefieldmodel.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QVariant>

class PriceFieldTableDelegatePrivate{
public:
    PriceFieldTableDelegatePrivate( PriceFieldModel * m ):
        model(m),
        applyFormulaCol(PriceFieldModel::applyFormulaCol() ),
        multiplyByCol(PriceFieldModel::multiplyByCol() ),
        fieldTypeCol(PriceFieldModel::fieldTypeCol() ){
    }
    PriceFieldModel * model;
    int applyFormulaCol;
    int multiplyByCol;
    int fieldTypeCol;
};

PriceFieldTableDelegate::PriceFieldTableDelegate(PriceFieldModel * model, QObject *parent) :
    QStyledItemDelegate(parent),
    m_d(new PriceFieldTableDelegatePrivate(model) ){
}

PriceFieldTableDelegate::~PriceFieldTableDelegate(){
    delete m_d;
}

void PriceFieldTableDelegate::paint(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    if (index.column() == m_d->applyFormulaCol ) {
        Q_ASSERT(index.isValid());
        QList< QPair<PriceFieldModel::ApplyFormula, QString > > list = PriceFieldModel::applyFormulaNames();
        PriceFieldModel::ApplyFormula val = (PriceFieldModel::ApplyFormula)(index.data().toInt());
        for( QList< QPair<PriceFieldModel::ApplyFormula, QString > >::iterator i=list.begin(); i != list.end(); ++i ){
            if( (*i).first == val ){
                painter->drawText(option.rect, Qt::AlignVCenter, (*i).second );
                return;
            }
        }
    } else if (index.column() == m_d->multiplyByCol ) {
        Q_ASSERT(index.isValid());
        QList< QPair<int, QString > > list = m_d->model->multiplyByNames( index.row() );
        int val = index.data().toInt();
        for( QList< QPair<int, QString > >::iterator i=list.begin(); i != list.end(); ++i ){
            if( (*i).first == val ){
                painter->drawText(option.rect, Qt::AlignVCenter, (*i).second );
                return;
            }
        }
    } else if (index.column() == m_d->fieldTypeCol ) {
        Q_ASSERT(index.isValid());
        QList< QPair<PriceFieldModel::FieldType, QString > > list = PriceFieldModel::standardFieldTypeNames();
        PriceFieldModel::FieldType val = (PriceFieldModel::FieldType)(index.data().toInt());
        for( QList< QPair<PriceFieldModel::FieldType, QString > >::iterator i=list.begin(); i != list.end(); ++i ){
            if( (*i).first == val ){
                painter->drawText(option.rect, Qt::AlignVCenter, (*i).second );
                return;
            }
        }
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QWidget * PriceFieldTableDelegate::createEditor(QWidget *parent,
                                                const QStyleOptionViewItem & viewItem,
                                                const QModelIndex & index ) const {
    if( index.column() == m_d->applyFormulaCol ) {
        QComboBox * cBox = new QComboBox( parent );
        QList< QPair<PriceFieldModel::ApplyFormula, QString > > list = PriceFieldModel::applyFormulaNames();
        for( QList< QPair<PriceFieldModel::ApplyFormula, QString > >::iterator i=list.begin(); i != list.end(); ++i ){
            cBox->addItem( (*i).second, QVariant( (*i).first) );
        }
        return cBox;
    } else if( index.column() == m_d->multiplyByCol ) {
        QComboBox * cBox = new QComboBox( parent );
        QList< QPair<int , QString > > list = m_d->model->multiplyByNames(index.row());
        for( QList< QPair<int, QString > >::iterator i=list.begin(); i != list.end(); ++i ){
            cBox->addItem( (*i).second, QVariant( (*i).first) );
        }
        return cBox;
    } else if( index.column() == m_d->fieldTypeCol ) {
        QComboBox * cBox = new QComboBox( parent );
        QList< QPair<PriceFieldModel::FieldType, QString > > list = PriceFieldModel::standardFieldTypeNames();
        for( QList< QPair<PriceFieldModel::FieldType, QString > >::iterator i=list.begin(); i != list.end(); ++i ){
            cBox->addItem( (*i).second, QVariant( (*i).first) );
        }
        return cBox;
    }
    return QStyledItemDelegate::createEditor( parent, viewItem, index );
}

void PriceFieldTableDelegate::setEditorData(QWidget *editor,
                                            const QModelIndex &index) const {
    if( (index.column() == m_d->applyFormulaCol) || (index.column() == m_d->multiplyByCol) || (index.column() == m_d->fieldTypeCol) ){
        QVariant value = index.model()->data( index, Qt::DisplayRole );
        QComboBox * cBox = static_cast<QComboBox *>( editor );
        cBox->setCurrentIndex( cBox->findData( value ) );
    } else {
        QStyledItemDelegate::setEditorData( editor, index );
    }
}

void PriceFieldTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->itemData( cb->currentIndex() ), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
