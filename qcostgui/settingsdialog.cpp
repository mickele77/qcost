#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "mathparser.h"

#include <QFileDialog>

class SettingsDialogPrivate{
public:
    SettingsDialogPrivate( QString * wpf, MathParser * mp ):
        ui(new Ui::SettingsDialog),
        wordProcessorFile(wpf ),
        parser( mp ){
    }
    ~SettingsDialogPrivate(){
        delete ui;
    }

    Ui::SettingsDialog *ui;
    QString *wordProcessorFile;
    MathParser * parser;
};

SettingsDialog::SettingsDialog(QString *wordProcessorFile, MathParser * parser, QWidget *parent) :
    QDialog(parent),
    m_d( new SettingsDialogPrivate(wordProcessorFile, parser ) ){
    m_d->ui->setupUi(this);
    m_d->ui->wordProcessorFileLineEdit->setText( *wordProcessorFile );
    if( m_d->parser != nullptr ) {
        m_d->ui->decSeparatorLEdit->setText( m_d->parser->decimalSeparator() );
        m_d->ui->thSeparatorLEdit->setText( m_d->parser->thousandSeparator() );
    }

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
    QChar decSeparator;
    if( m_d->ui->decSeparatorLEdit->text().length() > 0 ){
        decSeparator = m_d->ui->decSeparatorLEdit->text().at(0);
    }
    QChar thSeparator;
    if( m_d->ui->thSeparatorLEdit->text().length() > 0 ){
        thSeparator = m_d->ui->thSeparatorLEdit->text().at(0);
    }
    m_d->parser->setSeparators( decSeparator, thSeparator );
    accept();
}

void SettingsDialog::wordProcessorSelectFile(){
    QString txt = QFileDialog::getOpenFileName( this, tr("Seleziona WordProcessor ODT"), "", "" );
    if( !txt.isEmpty() ){
        m_d->ui->wordProcessorFileLineEdit->setText(txt);
    }
}
