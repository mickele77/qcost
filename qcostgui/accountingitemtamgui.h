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
    
    void showEvent(QShowEvent *event);

    void setAccountingBill(AccountingBill *b);

public slots:
    void setAccountingItem(AccountingBillItem *b );
    void setAccountingItemnullptr();

private slots:
    void addAttribute();
    void removeAttribute();

    void setAccountingnullptr();

    void updateTAMBillComboBox();

    void setTAMBill();
private:
    AccountingItemTAMGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMTAMGUI_H
