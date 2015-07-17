#ifndef PRICEDATATABLEDELEGATE_H
#define PRICEDATATABLEDELEGATE_H

#include <QStyledItemDelegate>

class PriceDataTableDelegatePrivate;
class PriceItemDataSetViewModel;

class PriceDataTableDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit PriceDataTableDelegate(PriceItemDataSetViewModel *dModel = NULL, QObject *parent = 0);
    ~PriceDataTableDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    void setModel(PriceItemDataSetViewModel *dModel );

signals:
    void editAssociatedAP( const QModelIndex & );

private:
    PriceDataTableDelegatePrivate * m_d;

    QRect comboBoxRect(const QStyleOptionViewItem &option) const;
};

#endif // PRICEDATATABLEDELEGATE_H
