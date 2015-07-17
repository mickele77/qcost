#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

class SettingsDialogPrivate;

#include <QDialog>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QString * wordProcessorFile, QWidget *parent = 0);
    ~SettingsDialog();
private slots:
    void setValuesAndExit();
    void wordProcessorSelectFile();
private:
    SettingsDialogPrivate * m_d;
};

#endif // SETTINGSDIALOG_H
