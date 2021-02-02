#include "importlsitemmeasurestxt.h"
#include "ui_importlsitemmeasurestxt.h"

#include "accountinglsmeasuresmodel.h"
#include "accountinglsmeasure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QFileDialog>

class ImportLSItemMeasuresTXTPrivate {
public:
    ImportLSItemMeasuresTXTPrivate( AccountingLSMeasuresModel * mModel, int mPosition, MathParser * prs ):
        ui(new Ui::ImportLSItemMeasuresTXT),
        measuresModel(mModel),
        insertPosition(mPosition),
        parser(prs){
        fieldsNames << QObject::tr("---");
        fieldsNames << QObject::tr("Commento");
        fieldsNames << QObject::tr("Misure prog.");
        fieldsNames << QObject::tr("Data cont.");
        fieldsNames << QObject::tr("Misure cont.");

        fieldsSeparator << qMakePair( QString("\t"), QObject::tr("TAB") );
        fieldsSeparator << qMakePair( QString(" "), QObject::tr("Spazio") );
    }

    ~ImportLSItemMeasuresTXTPrivate(){
        delete ui;
    }

    Ui::ImportLSItemMeasuresTXT *ui;
    QList<QComboBox *> fieldComboBoxList;
    QList<QString> fieldsNames;
    QList< QPair<QString, QString> > fieldsSeparator;
    AccountingLSMeasuresModel * measuresModel;
    int insertPosition;
    MathParser * parser;
    QString fileName;
};

ImportLSItemMeasuresTXT::ImportLSItemMeasuresTXT( AccountingLSMeasuresModel * mModel, int mPosition, MathParser * prs, QWidget *parent) :
    QDialog(parent),
    m_d( new ImportLSItemMeasuresTXTPrivate( mModel, mPosition, prs ) ){
    m_d->ui->setupUi(this);

    setWindowTitle( tr("Importa misure da file TXT") );

    m_d->ui->decimalSeparatorLineEdit->setText( tr("."));
    m_d->ui->thousandSeparatorLineEdit->setText( tr(""));

    connect( m_d->ui->insertFieldPushButton, &QPushButton::clicked, this, &ImportLSItemMeasuresTXT::insertFieldComboBox );
    connect( m_d->ui->removeFieldPushButton, &QPushButton::clicked, this, &ImportLSItemMeasuresTXT::removeFieldComboBox );
    connect( m_d->ui->editFileNameButton, &QToolButton::clicked, this, &ImportLSItemMeasuresTXT::editFileName );

    connect( m_d->ui->buttonBox, &QDialogButtonBox::accepted, this, &ImportLSItemMeasuresTXT::importMeasures );

    for( int i=0; i < m_d->fieldsSeparator.size(); ++i ){
        m_d->ui->fieldsSeparatorComboBox->addItem( m_d->fieldsSeparator.at(i).second );
    }
    m_d->ui->fieldsSeparatorComboBox->setCurrentIndex(0);

    insertFieldComboBox();
    m_d->fieldComboBoxList.at(0)->setCurrentIndex(1);
    insertFieldComboBox();
    m_d->fieldComboBoxList.at(1)->setCurrentIndex(2);

    editFileName();

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

ImportLSItemMeasuresTXT::~ImportLSItemMeasuresTXT() {
    delete m_d;
}

void ImportLSItemMeasuresTXT::insertFieldComboBox() {
    QComboBox * box = new QComboBox( this );

    // box populate
    for( QList<QString>::iterator i=m_d->fieldsNames.begin(); i != m_d->fieldsNames.end(); ++i ){
        box->addItem( *i );
    }
    m_d->fieldComboBoxList.append( box );
    m_d->ui->fieldComboBoxLayout->insertWidget( m_d->fieldComboBoxList.size()-1, box );
}

void ImportLSItemMeasuresTXT::removeFieldComboBox() {
    if( !m_d->fieldComboBoxList.isEmpty() ){
        m_d->ui->fieldComboBoxLayout->removeWidget( m_d->fieldComboBoxList.last() );
        delete m_d->fieldComboBoxList.takeLast();
    }
}

void ImportLSItemMeasuresTXT::editFileName() {
    QString fileName = QFileDialog::getOpenFileName( this,
                                                     tr("Apri file TXT"), ".",
                                                     tr("Documento di testo (*.txt)"));
    if( !(fileName.isEmpty()) ){
        QFile fileTxt( fileName );
        if(fileTxt.open(QIODevice::ReadOnly | QIODevice::Text)){
            m_d->fileName = fileName;
            m_d->ui->fileNameLineEdit->setText( m_d->fileName );

            QTextStream in( &fileTxt );
            m_d->ui->filePreviewTextEdit->clear();
            while( !in.atEnd() ){
                m_d->ui->filePreviewTextEdit->appendPlainText( in.readLine() );
            }
            m_d->ui->filePreviewTextEdit->moveCursor( QTextCursor::Start );
        }
    }
}

void ImportLSItemMeasuresTXT::importMeasures() {
    if( !m_d->fileName.isEmpty() ){
        QFile fileTxt( m_d->fileName );
        if(fileTxt.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in( &fileTxt );
            int insPosEff = m_d->insertPosition;
            while( !in.atEnd() ){
                m_d->measuresModel->insertRows( insPosEff, 1 );
                QString fieldSep = m_d->fieldsSeparator.at( m_d->ui->fieldsSeparatorComboBox->currentIndex()).first;
                QStringList line = in.readLine().split( fieldSep );
                for( int i=0; i < m_d->fieldComboBoxList.size(); ++i ){
                    if( i >= line.size() ){
                        break;
                    }
                    // commento
                    if( m_d->fieldComboBoxList.at(i)->currentIndex() == 1 ){
                        m_d->measuresModel->measure(insPosEff)->setComment( line.at(i));
                    }
                    // misura prog
                    if( m_d->fieldComboBoxList.at(i)->currentIndex() == 2 ){
                        QString form = line.at(i);
                        form.remove( m_d->ui->thousandSeparatorLineEdit->text() );
                        form.replace( m_d->ui->decimalSeparatorLineEdit->text(), m_d->parser->decimalSeparator() );
                        m_d->measuresModel->measure(insPosEff)->setProjFormula( form );
                    }
                    // data contabilita
                    if( m_d->fieldComboBoxList.at(i)->currentIndex() == 3 ){
                        QString form = line.at(i);
                        m_d->measuresModel->measure(insPosEff)->setAccDate( form );
                    }
                    // misura contabilita
                    if( m_d->fieldComboBoxList.at(i)->currentIndex() == 4 ){
                        QString form = line.at(i);
                        form.remove( m_d->ui->thousandSeparatorLineEdit->text() );
                        form.replace( m_d->ui->decimalSeparatorLineEdit->text(), m_d->parser->decimalSeparator() );
                        m_d->measuresModel->measure(insPosEff)->setAccFormula( form );
                    }
                }
                insPosEff++;
            }
        }
    }

    accept();
}
