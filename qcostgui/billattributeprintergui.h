#ifndef BILLATTRIBUTEPRINTERGUI_H
#define BILLATTRIBUTEPRINTERGUI_H

#include "billprinter.h"

class BillAttribute;
class BillAttributeModel;
class PriceFieldModel;

#include <QDialog>

class BillAttributePrinterGUIPrivate;

class BillAttributePrinterGUI : public QDialog {
    Q_OBJECT
public:
    explicit BillAttributePrinterGUI( BillPrinter::PrintBillItemsOption *prItemsOption,
                                      BillPrinter::AttributePrintOption * prOption,
                                      QList<int> * pFlds,
                                      QList<BillAttribute *> * pAttrs,
                                      double *pWidth, double *pHeight,
                                      Qt::Orientation *pOrient,
                                      bool * groupPrAm,
                                      PriceFieldModel * pfm,
                                      BillAttributeModel * bam,
                                      QWidget *parent = 0);
    ~BillAttributePrinterGUI();
private slots:
    void setPrintData();
    void insertPriceFieldComboBox();
    void removePriceFieldComboBox();
private:
    BillAttributePrinterGUIPrivate * m_d;
};

#endif // BILLATTRIBUTEPRINTERGUI_H
