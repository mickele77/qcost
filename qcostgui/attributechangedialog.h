#ifndef ATTRIBUTECHANGEDIALOG_H
#define ATTRIBUTECHANGEDIALOG_H

class AttributeChangeDialogPrivate;
class AccountingTAMBillItem;
class AccountingBillItem;
class BillItem;
class AttributeModel;

#include <QDialog>

class AttributeChangeDialog : public QDialog {
    Q_OBJECT
public:
    explicit AttributeChangeDialog(QList<BillItem *> * itemsList,
                                       AttributeModel * accountingAttrModel,
                                       QWidget *parent = 0);
    explicit AttributeChangeDialog(QList<AccountingBillItem *> * itemsList,
                                       AttributeModel * accountingAttrModel,
                                       QWidget *parent = 0);
    explicit AttributeChangeDialog(QList<AccountingTAMBillItem *> * itemsList,
                                       AttributeModel * accountingAttrModel,
                                       QWidget *parent = 0);
    ~AttributeChangeDialog();

private slots:
    void addAttributes();
    void removeAttributes();
    void setAttributes();

private:
    AttributeChangeDialogPrivate * m_d;
    void setup();
};

#endif // ATTRIBUTECHANGEDIALOG_H
