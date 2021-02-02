#ifndef ACCOUNTINGLSBILLITEMTITLEGUI_H
#define ACCOUNTINGLSBILLITEMTITLEGUI_H

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
    void setBillItemnullptr();
    void setBillnullptr();

private slots:
    void addAttribute();
    void removeAttribute();
private:
    AccountingLSBillItemTitleGUIPrivate * m_d;
};

#endif // ACCOUNTINGLSBILLITEMTITLEGUI_H
