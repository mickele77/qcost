#ifndef ACCOUNTINGITEMTAMGUI_H
#define ACCOUNTINGITEMTAMGUI_H

class PriceFieldModel;
class AccountingTAMBill;
class AccountingBill;
class AccountingBillItem ;

#include <QWidget>

class AccountingItemTAMGUIPrivate;

class AccountingItemTAMGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingItemTAMGUI(AccountingTAMBill *tamBill, PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingItemTAMGUI();
    
    void setAccountingBill(AccountingBill *b);

public slots:
    void setAccountingItem(AccountingBillItem *b );
    void setAccountingItemNULL();

private slots:
    void addAttribute();
    void removeAttribute();

    void setAccountingNULL();

    void setDateBegin( const QString & newVal );
    void setDateEnd( const QString & newVal );

    void updateTAMBillComboBox();

private:
    AccountingItemTAMGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMTAMGUI_H
