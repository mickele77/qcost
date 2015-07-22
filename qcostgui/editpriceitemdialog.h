#ifndef EDITPRICEITEMDIALOG_H
#define EDITPRICEITEMDIALOG_H

#include "pricelistdbwidget.h"
#include <QVariant>

class Project;
class BillItem;
class AccountingBillItem;
class AccountingTAMBillItem;
class AccountingLSBillItem;
class PriceList;
class MathParser;

class EditPriceItemDialogPrivate;

#include <QDialog>

class EditPriceItemDialog : public QDialog {
    Q_OBJECT
public:
    EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                         QString * EPAFileName,
                         PriceList * pl, int priceDataSet, BillItem *bItem,
                         MathParser *prs, Project *prj,
                         QWidget *parent = 0);
    EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                        QString * EPAFileName,
                        PriceList * pl, int priceDataSet, AccountingBillItem *bItem,
                        MathParser *prs, Project *prj,
                        QWidget *parent = 0);
    EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                        QString * EPAFileName,
                        PriceList * pl, int priceDataSet, AccountingTAMBillItem *bItem,
                        MathParser *prs, Project *prj,
                        QWidget *parent = 0);
    EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                        QString * EPAFileName,
                        PriceList * pl, int priceDataSet, AccountingLSBillItem *bItem,
                        MathParser *prs, Project *prj,
                        QWidget *parent = 0);
    ~EditPriceItemDialog();
private slots:
    void changePriceItemAndClose();

private:
    EditPriceItemDialogPrivate * m_d;
    void init(PriceList *pl);
};

#endif // EDITPRICEITEMDIALOG_H
