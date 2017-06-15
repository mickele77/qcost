#ifndef IMPORTLSITEMMEASURESTXT_H
#define IMPORTLSITEMMEASURESTXT_H

class ImportLSItemMeasuresTXTPrivate;
class AccountingLSMeasuresModel;
class MathParser;

#include <QDialog>

class ImportLSItemMeasuresTXT : public QDialog {
    Q_OBJECT
public:
    explicit ImportLSItemMeasuresTXT( AccountingLSMeasuresModel * mModel, int mPosition,
                                        MathParser * prs, QWidget *parent = 0);
    ~ImportLSItemMeasuresTXT();
private slots:
    void insertFieldComboBox();
    void removeFieldComboBox();
    void editFileName();
    void importMeasures();
private:
    ImportLSItemMeasuresTXTPrivate * m_d;
};

#endif // IMPORTLSITEMMEASURESTXT_H
