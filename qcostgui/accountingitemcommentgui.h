#ifndef ACCOUNTINGITEMCOMMENTGUI_H
#define ACCOUNTINGITEMCOMMENTGUI_H

class PriceFieldModel;
class AccountingTAMBillItem;
class AccountingBillItem;

#include <QWidget>

class AccountingItemCommentGUIPrivate;

class AccountingItemCommentGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit AccountingItemCommentGUI(QWidget *parent);
    ~AccountingItemCommentGUI();
    
public slots:
    void setAccountingItem(AccountingTAMBillItem *b );
    void setAccountingItem(AccountingBillItem *b );
    void setAccountingItemnullptr();

private slots:
    void updateTextLineEdit();
    void updateTextMeasure();

private:
    AccountingItemCommentGUIPrivate * m_d;
};

#endif // ACCOUNTINGITEMCOMMENTGUI_H
