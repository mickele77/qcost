#ifndef QCOSTCLIPBOARDDATA_H
#define QCOSTCLIPBOARDDATA_H

class QCostClipboardDataPrivate;
class AccountingLSBill;
class AccountingLSBillItem;
class AccountingTAMBill;
class AccountingTAMBillItem;
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

    QList<AccountingLSBill *> copiedAccountingLSBills();
    QCostClipboardData::Mode copiedAccountingLSBillItemsMode() const;
    void setCopiedAccountingLSBills( QList<AccountingLSBill *> cb, Mode m);
    void setCopiedAccountingLSBillItems( QList<AccountingLSBillItem *> accountingItems, AccountingLSBill * b, Mode m);
    void getCopiedAccountingLSBillItems( QList<AccountingLSBillItem *> *accountingItems,
                                         AccountingLSBill * b,
                                         QCostClipboardData::Mode *mode) const;

    QList<AccountingTAMBill *> copiedAccountingTAMBills();
    QCostClipboardData::Mode copiedAccountingTAMBillItemsMode() const;
    void setCopiedAccountingTAMBills( QList<AccountingTAMBill *> cb, Mode m);
    void setCopiedAccountingTAMBillItems( QList<AccountingTAMBillItem *> accountingItems, AccountingTAMBill * b, Mode m);
    void getCopiedAccountingTAMBillItems( QList<AccountingTAMBillItem *> *accountingItems, AccountingTAMBill * b, QCostClipboardData::Mode *mode) const;

    QList<AccountingBill *> copiedAccountingBills();
    QCostClipboardData::Mode copiedAccountingBillItemsMode() const;
    void setCopiedAccountingBills( QList<AccountingBill *> cb, Mode m);
    void setCopiedAccountingBillItems( QList<AccountingBillItem *> accountingItems, AccountingBill * b, Mode m);
    void getCopiedAccountingBillItems( QList<AccountingBillItem *> *accountingItems,
                                       AccountingBill * b,
                                       QCostClipboardData::Mode *mode) const;

    QList<Bill *> copiedBills();
    QCostClipboardData::Mode copiedBillItemsMode() const;
    void setCopiedBills( QList<Bill *> cb, Mode m);
    void setCopiedBillItems( QList<BillItem *> bi, Bill * b, Mode m);
    void getCopiedBillItems(QList<BillItem *> *billItems, Bill *bill, QCostClipboardData::Mode *mode) const;

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
