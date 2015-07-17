#ifndef ACCOUNTINGITEMBILLGUI_H
#define ACCOUNTINGITEMBILLGUI_H

class PriceFieldModel;
class AccountingTAMBill;
class AccountingTAMBillItem ;
class AccountingBill;
class AccountingBillItem ;

#include <QWidget>

class AccountingItemBillGUIPrivate;

class AccountingItemBillGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingItemBillGUI(PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingItemBillGUI();
    
    void setAccountingTAMBill(AccountingTAMBill *b);
    void setAccountingBill(AccountingBill *b);

public slots:
    void setAccountingItem(AccountingTAMBillItem *b );
    void setAccountingItem(AccountingBillItem *b );
    void setAccountingItemNULL();

private slots:
    void addAttribute();
    void removeAttribute();

    void setAccountingNULL();

    void setDateBegin( const QString & newVal );
    void setDateEnd( const QString & newVal );
private:
    AccountingItemBillGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMBILLGUI_H
