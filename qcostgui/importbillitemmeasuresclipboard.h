#ifndef IMPORTBILLITEMMEASURESCLIPBOARD_H
#define IMPORTBILLITEMMEASURESCLIPBOARD_H

class ImportBillItemMeasuresClipboardPrivate;
class MeasuresModel;
class MathParser;

#include <QDialog>

class ImportBillItemMeasuresClipboard : public QDialog {
    Q_OBJECT
public:
    explicit ImportBillItemMeasuresClipboard( MeasuresModel * mModel, int mPosition,
                                        MathParser * prs, QWidget *parent = 0);
    ~ImportBillItemMeasuresClipboard();
private slots:
    void importComments();
private:
    ImportBillItemMeasuresClipboardPrivate * m_d;
};

#endif // IMPORTBILLITEMMEASURESCLIPBOARD_H
