#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QFileDialog>

class SettingsDialogPrivate{
public:
    SettingsDialogPrivate( QString * wpf ):
        ui(new Ui::SettingsDialog),
        wordProcessorFile(wpf ) {
    }
    ~SettingsDialogPrivate(){
        delete ui;
    }

    Ui::SettingsDialog *ui;
    QString *wordProcessorFile;
};

SettingsDialog::SettingsDialog(QString *wordProcessorFile, QWidget *parent) :
    QDialog(parent),
    m_d( new SettingsDialogPrivate(wordProcessorFile) ){
    m_d->ui->setupUi(this);
    m_d->ui->wordProcessorFileLineEdit->setText( *wordProcessorFile );

    connect( m_d->ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::setValuesAndExit );
    connect( m_d->ui->wordProcessorSelectFileToolButton, &QToolButton::clicked, this, &SettingsDialog::wordProcessorSelectFile );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

SettingsDialog::~SettingsDialog() {
    delete m_d;
}

void SettingsDialog::setValuesAndExit(){
    *(m_d->wordProcessorFile) = m_d->ui->wordProcessorFileLineEdit->text();
    accept();
}

void SettingsDialog::wordProcessorSelectFile(){
    QString txt = QFileDialog::getOpenFileName( this, tr("Seleziona WordProcessor ODT"), "", "" );
    if( !txt.isEmpty() ){
        m_d->ui->wordProcessorFileLineEdit->setText(txt);
    }
}
