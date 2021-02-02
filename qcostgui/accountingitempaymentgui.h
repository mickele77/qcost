#ifndef ACCOUNTINGITEMPAYMENTGUI_H
#define ACCOUNTINGITEMPAYMENTGUI_H

class PriceFieldModel;
class AccountingTAMBill;
class AccountingTAMBillItem ;
class AccountingBill;
class AccountingBillItem ;

#include <QWidget>

class AccountingItemPaymentGUIPrivate;

class AccountingItemPaymentGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingItemPaymentGUI(PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingItemPaymentGUI();
    
    void setAccountingTAMBill(AccountingTAMBill *b);
    void setAccountingBill(AccountingBill *b);

public slots:
    void setAccountingItem(AccountingTAMBillItem *b );
    void setAccountingItem(AccountingBillItem *b );
    void setAccountingItemnullptr();

private slots:
    void addAttribute();
    void removeAttribute();

    void setAccountingnullptr();

    void setDateBegin( const QString & newVal );
    void setDateEnd( const QString & newVal );
private:
    AccountingItemPaymentGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMPAYMENTGUI_H
