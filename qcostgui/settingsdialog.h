#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

class MathParser;
class SettingsDialogPrivate;

#include <QDialog>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QString * wordProcessorFile, MathParser * parser, QWidget *parent = 0);
    ~SettingsDialog();
private slots:
    void setValuesAndExit();
    void wordProcessorSelectFile();
private:
    SettingsDialogPrivate * m_d;
};

#endif // SETTINGSDIALOG_H
