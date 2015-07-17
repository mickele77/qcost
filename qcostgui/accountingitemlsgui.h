#ifndef ACCOUNTINGITEMLSGUI_H
#define ACCOUNTINGITEMLSGUI_H

class PriceFieldModel;
class AccountingBill;
class AccountingBillItem ;

#include <QWidget>

class AccountingItemLSGUIPrivate;

class AccountingItemLSGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingItemLSGUI(PriceFieldModel *pfm, QWidget *parent = 0);
    ~AccountingItemLSGUI();
    
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
private:
    AccountingItemLSGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMLSGUI_H
