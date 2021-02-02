#ifndef BILLITEMTITLEGUI_H
#define BILLITEMTITLEGUI_H

class PriceFieldModel;
class Bill;
class BillItem;

#include <QWidget>

class BillItemTitleGUIPrivate;

class BillItemTitleGUI : public QWidget
{
    Q_OBJECT
    
public:
    explicit BillItemTitleGUI(BillItem *item, PriceFieldModel *pfm, QWidget *parent = 0);
    ~BillItemTitleGUI();
    
    void setBill(Bill *b);

public slots:
    void setBillItem( BillItem * b );

private slots:
    void updateLineEdit();
    void updateItem();

    void addAttribute();
    void removeAttribute();
    void setBillItemnullptr();
    void setBillnullptr();

    void updateAmountNamesValues();
    void updateAmountValue(int priceField, const QString &newVal);
    void updateAmountValues();
    void updateAmountName(int priceField, const QString &newName);
private:
    BillItemTitleGUIPrivate * m_d;
};

#endif // BILLITEMTITLEGUI_H
