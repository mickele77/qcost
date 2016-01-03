#ifndef ACCOUNTINGLSBILLSDATAGUI_H
#define ACCOUNTINGLSBILLSDATAGUI_H

class AccountingLSBills;
class AccountingLSBillsDataGUIPrivate;

#include <QWidget>

class AccountingLSBillsDataGUI : public QWidget
{
    Q_OBJECT

public:
    explicit AccountingLSBillsDataGUI(AccountingLSBills *myBills, QWidget *parent = 0);
    ~AccountingLSBillsDataGUI();

    void showEvent(QShowEvent *event);
private:
    AccountingLSBillsDataGUIPrivate *m_d;
};

#endif // ACCOUNTINGLSBILLSDATAGUI_H
