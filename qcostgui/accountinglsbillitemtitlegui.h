#ifndef ACCOUNTINGLSBILLITEMTITLEGUI_H
#define ACCOUNTINGLSITEMTITLEGUI_H

class PriceFieldModel;
class Bill;
class BillItem;

#include <QWidget>

class AccountingLSBillItemTitleGUIPrivate;

class AccountingLSBillItemTitleGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingLSBillItemTitleGUI(BillItem *item, PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingLSBillItemTitleGUI();
    
    void setBill(Bill *b);

public slots:
    void setBillItem( BillItem * b );

private slots:
    void updateLineEdit();
    void updateItem();

    void addAttribute();
    void removeAttribute();
    void setBillItemNULL();
    void setBillNULL();

    void updateAmountNamesValues();
    void updateAmountValue(int priceField, const QString &newVal);
    void updateAmountValues();
    void updateAmountName(int priceField, const QString &newName);
private:
    AccountingLSBillItemTitleGUIPrivate * m_d;
};

#endif // ACCOUNTINGLSITEMTITLEGUI_H
