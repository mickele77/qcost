#include "pricedatatabledelegate.h"

#include "priceitemdatasetviewmodel.h"

#include <QStyle>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QPalette>

class PriceDataTableDelegatePrivate{
public:
    PriceDataTableDelegatePrivate(PriceItemDataSetViewModel * dModel):
        buttonState( QStyle::State_None ),
        dataModel(dModel){
    }
    QStyle::State buttonState;
    int currentCol;
    PriceItemDataSetViewModel * dataModel;
};

PriceDataTableDelegate::PriceDataTableDelegate(PriceItemDataSetViewModel * dModel, QObject *parent) :
    QStyledItemDelegate(parent),
    m_d(new PriceDataTableDelegatePrivate(dModel) ){
}

PriceDataTableDelegate::~PriceDataTableDelegate(){
    delete m_d;
}

void PriceDataTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint( painter, option, index);
    if( m_d->dataModel != nullptr ){
        if( index.row() == m_d->dataModel->associatedAPRow() ){
            QStyleOptionButton button;

            button.text = "AP";
            button.features = QStyleOptionButton::AutoDefaultButton;
            button.rect = comboBoxRect( option );

            if( index.data(Qt::CheckStateRole).toInt() == Qt::Checked ){
                if( m_d->currentCol == index.column() ){
                    button.state = m_d->buttonState;
                }
                button.state = QStyle::State_Enabled;
            } else {
                button.state = QStyle::State_None;
                button.palette.setCurrentColorGroup( QPalette::Disabled );
            }

            QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
        }
    }
}

bool PriceDataTableDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)  {
    if( m_d->dataModel != nullptr ){
        if( index.row() == m_d->dataModel->associatedAPRow() ){
            if( event->type() == QEvent::MouseButtonRelease )  {
                QMouseEvent * e = (QMouseEvent *)event;
                if( comboBoxRect( option ).contains( e->pos()) ){
                    emit editAssociatedAP( index );
                    m_d->buttonState =  QStyle::State_Enabled | QStyle::State_Raised;
                    m_d->currentCol = -1;
                    return true;
                }
            }
            if( event->type() == QEvent::MouseButtonPress )  {
                QMouseEvent * e = (QMouseEvent *)event;
                if( comboBoxRect( option ).contains( e->pos()) ){
                    m_d->buttonState = QStyle::State_Enabled | QStyle::State_Sunken;
                    m_d->currentCol= index.column();
                    return true;
                }
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void PriceDataTableDelegate::setModel(PriceItemDataSetViewModel *dModel) {
    m_d->dataModel = dModel;
}

QRect PriceDataTableDelegate::comboBoxRect( const QStyleOptionViewItem &option ) const {
    int checkBoxWidth = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option).width() + 6;

    QRect r = option.rect;
    int x,y,w,h;
    x = r.left() + checkBoxWidth;
    y = r.top();
    h = r.height();
    w = r.width() - checkBoxWidth;

    return QRect(x,y,w,h);
}
