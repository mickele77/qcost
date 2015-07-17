#ifndef BILLDELEGATE_H
#define BILLDELEGATE_H

#include <QStyledItemDelegate>

class UnitMeasureList;
class BillDelegatePrivate;
class PriceList;

class BillDelegate : public QStyledItemDelegate
{
public:
    explicit BillDelegate(UnitMeasureList *uml, PriceList * pl, QObject *parent = 0);
    ~BillDelegate();

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData ( QWidget *editor, const QModelIndex &index ) const;
    void setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

private:
    BillDelegatePrivate * m_d;
};

#endif // BILLDELEGATE_H
