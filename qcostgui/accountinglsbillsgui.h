#ifndef ACCOUNTINGLSBILLSGUI_H
#define ACCOUNTINGLSBILLSGUI_H

class AccountingLSBills;
class PriceFieldModel;
class MathParser;
class AccountingLSBillsGUIPrivate;

#include <QTabWidget>

class AccountingLSBillsGUI : public QTabWidget {
public:
    AccountingLSBillsGUI(AccountingLSBills * myBills, PriceFieldModel *pfm, MathParser *prs, QString *wpf, QWidget * parent = 0 );
private:
    AccountingLSBillsGUIPrivate * m_d;
};

#endif // ACCOUNTINGLSBILLSGUI_H
