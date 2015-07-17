#ifndef PRICEFIELDTABLEDELEGATE_H
#define PRICEFIELDTABLEDELEGATE_H

#include <QStyledItemDelegate>

class PriceFieldTableDelegatePrivate;

class PriceFieldTableDelegate : public QStyledItemDelegate {
public:
    explicit PriceFieldTableDelegate(QObject *parent = 0);
    ~PriceFieldTableDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &viewItem, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    PriceFieldTableDelegatePrivate * m_d;
};

#endif // PRICEFIELDTABLEDELEGATE_H
