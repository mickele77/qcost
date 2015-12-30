#ifndef ATTRIBUTECHANGEDIALOG_H
#define ATTRIBUTECHANGEDIALOG_H

class AttributeChangeDialogPrivate;
class AccountingTAMBillItem;
class AccountingBillItem;
class BillItem;
class AttributesModel;

#include <QDialog>

class AttributeChangeDialog : public QDialog {
    Q_OBJECT
public:
    explicit AttributeChangeDialog(QList<BillItem *> * itemsList,
                                       AttributesModel * accountingAttrModel,
                                       QWidget *parent = 0);
    explicit AttributeChangeDialog(QList<AccountingBillItem *> * itemsList,
                                       AttributesModel * accountingAttrModel,
                                       QWidget *parent = 0);
    explicit AttributeChangeDialog(QList<AccountingTAMBillItem *> * itemsList,
                                       AttributesModel * accountingAttrModel,
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
