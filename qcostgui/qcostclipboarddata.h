#ifndef QCOSTCLIPBOARDDATA_H
#define QCOSTCLIPBOARDDATA_H

class QCostClipboardDataPrivate;
class AccountingBill;
class AccountingBillItem;
class Bill;
class BillItem;
class PriceList;
class PriceItem;

#include <QMimeData>

class QCostClipboardData : public QMimeData {
    Q_OBJECT
public:
    enum Mode{
        Cut,
        Copy
    };

    explicit QCostClipboardData();
    ~QCostClipboardData();

    QCostClipboardData &operator =(const QCostClipboardData &cp);

    QList<AccountingBill *> copiedAccountingBills();
    QCostClipboardData::Mode copiedAccountingBillItemsMode() const;
    void setCopiedAccountingBills( QList<AccountingBill *> cb, Mode m);

    void setCopiedAccountingBillItems( QList<AccountingBillItem *> accountingItems, AccountingBill * acc, Mode m);
    void getCopiedAccountingBillItems(QList<AccountingBillItem *> *accountingItems, AccountingBill * &acc, QCostClipboardData::Mode *mode) const;

    QList<Bill *> copiedBills();
    QCostClipboardData::Mode copiedBillItemsMode() const;
    void setCopiedBills( QList<Bill *> cb, Mode m);

    void setCopiedBillItems( QList<BillItem *> bi, Bill * b, Mode m);
    void getCopiedBillItems(QList<BillItem *> *billItems, Bill * &bill, QCostClipboardData::Mode *mode) const;

    QList<PriceList *> copiedPriceLists();
    QCostClipboardData::Mode copiedPriceListsMode();
    void setCopiedPriceLists( QList<PriceList *> pl, Mode m);

    QList<PriceItem *> copiedPriceItems();
    PriceList * copiedPriceItemsPriceList();
    QCostClipboardData::Mode copiedPriceItemsMode();
    void setCopiedPriceItems( QList<PriceItem *> pi, PriceList * pl, Mode m);
    void getCopiedPriceItems( QList<PriceItem *> *priceItems, PriceList * &priceList, QCostClipboardData::Mode *mode) const;

private slots:
    void removeFromList();
private:
    QCostClipboardDataPrivate * m_d;
    void updateText();
};

#endif // QCOSTCLIPBOARDDATA_H
