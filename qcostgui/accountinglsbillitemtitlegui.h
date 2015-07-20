#ifndef ACCOUNTINGLSBILLITEMTITLEGUI_H
#define ACCOUNTINGLSITEMTITLEGUI_H

class PriceFieldModel;
class AccountingLSBill;
class AccountingLSBillItem;

#include <QWidget>

class AccountingLSBillItemTitleGUIPrivate;

class AccountingLSBillItemTitleGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingLSBillItemTitleGUI( PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingLSBillItemTitleGUI();
    
    void setBill(AccountingLSBill *b);

public slots:
    void setBillItem( AccountingLSBillItem * b );
    void setBillItemNULL();
    void setBillNULL();

private slots:
    void addAttribute();
    void removeAttribute();
private:
    AccountingLSBillItemTitleGUIPrivate * m_d;
};

#endif // ACCOUNTINGLSITEMTITLEGUI_H
