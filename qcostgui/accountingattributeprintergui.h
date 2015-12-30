#ifndef ACCOUNTINGATTRIBUTEPRINTERGUI_H
#define ACCOUNTINGATTRIBUTEPRINTERGUI_H

#include "accountingprinter.h"

class Attribute;
class AttributesModel;

#include <QDialog>

class AccountingAttributePrinterGUIPrivate;

class AccountingAttributePrinterGUI : public QDialog {
    Q_OBJECT
public:
    explicit AccountingAttributePrinterGUI(AccountingPrinter::PrintPPUDescOption *prItemsOption,
                                           AccountingPrinter::PrintAmountsOption *prAmountsOption,
                                           AccountingPrinter::AttributePrintOption * prOption,
                                           QList<Attribute *> *pAttrs,
                                           double *pWidth, double *pHeight,
                                           Qt::Orientation *pOrient,
                                           AttributesModel * bam,
                                           QWidget *parent = 0);
    ~AccountingAttributePrinterGUI();
private slots:
    void setPrintData();
private:
    AccountingAttributePrinterGUIPrivate * m_d;
};

#endif // ACCOUNTINGATTRIBUTEPRINTERGUI_H
