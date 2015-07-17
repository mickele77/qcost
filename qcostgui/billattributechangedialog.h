#ifndef BILLATTRIBUTECHANGEDIALOG_H
#define BILLATTRIBUTECHANGEDIALOG_H

class BillAttributeChangeDialogPrivate;
class BillItem;
class BillAttributeModel;

#include <QDialog>

class BillAttributeChangeDialog : public QDialog {
    Q_OBJECT
public:
    explicit BillAttributeChangeDialog(QList<BillItem *> * itemsList,
                                       BillAttributeModel * billAttrModel,
                                       QWidget *parent = 0);
    ~BillAttributeChangeDialog();

private slots:
    void addAttributes();
    void removeAttributes();
    void setAttributes();

private:
    BillAttributeChangeDialogPrivate * m_d;
};

#endif // BILLATTRIBUTECHANGEDIALOG_H
