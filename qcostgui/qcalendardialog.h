#ifndef QCALENDARDIALOG_H
#define QCALENDARDIALOG_H

#include <QDialog>

class QCalendarDialogPrivate;

class QCalendarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QCalendarDialog(QDate *d, QWidget *parent = 0);
    ~QCalendarDialog();

private slots:
    void changeDateAndClose();
private:
    QCalendarDialogPrivate * m_d;
};

#endif // QCALENDARDIALOG_H
