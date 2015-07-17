#ifndef EDITPRICEITEMDIALOG_H
#define EDITPRICEITEMDIALOG_H

#include "pricelistdbwidget.h"
#include <QVariant>

class Project;
class Bill;
class BillItem;
class PriceList;
class MathParser;

class EditPriceItemDialogPrivate;

#include <QDialog>

class EditPriceItemDialog : public QDialog {
    Q_OBJECT
public:
    EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                         QString * EPAFileName,
                         PriceList * pl, int priceCol, BillItem *bItem, MathParser *prs, Project *prj,
                         QWidget *parent = 0);
    ~EditPriceItemDialog();
private slots:
    void changePriceItemAndClose();

private:
    EditPriceItemDialogPrivate * m_d;
};

#endif // EDITPRICEITEMDIALOG_H
