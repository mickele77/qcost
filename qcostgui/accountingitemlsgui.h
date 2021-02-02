#ifndef ACCOUNTINGITEMLSGUI_H
#define ACCOUNTINGITEMLSGUI_H

class PriceFieldModel;
class AccountingLSBills;
class AccountingBill;
class AccountingBillItem ;

#include <QWidget>

class AccountingItemLSGUIPrivate;

class AccountingItemLSGUI : public QWidget {
    Q_OBJECT

public:
    explicit AccountingItemLSGUI( AccountingLSBills *lsBills, PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingItemLSGUI();
    
    void setAccountingBill(AccountingBill *b);

    void showEvent(QShowEvent * event);
public slots:
    void setItem(AccountingBillItem *b );
    void setAccountingItemnullptr();

private slots:
    void addAttribute();
    void removeAttribute();

    void setAccountingnullptr();

    void updateLumpSumsComboBox();
    void setLSBill();
private:
    AccountingItemLSGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMLSGUI_H
