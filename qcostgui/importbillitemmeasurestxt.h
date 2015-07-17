#ifndef IMPORTBILLITEMMEASURESTXT_H
#define IMPORTBILLITEMMEASURESTXT_H

class ImportBillItemMeasuresTXTPrivate;
class MeasuresModel;
class MathParser;

#include <QDialog>

class ImportBillItemMeasuresTXT : public QDialog {
    Q_OBJECT
public:
    explicit ImportBillItemMeasuresTXT( MeasuresModel * mModel, int mPosition,
                                        MathParser * prs, QWidget *parent = 0);
    ~ImportBillItemMeasuresTXT();
private slots:
    void insertFieldComboBox();
    void removeFieldComboBox();
    void editFileName();
    void importMeasures();
private:
    ImportBillItemMeasuresTXTPrivate * m_d;
};

#endif // IMPORTBILLITEMMEASURESTXT_H
