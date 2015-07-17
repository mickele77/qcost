#include "importbillitemmeasurestxt.h"
#include "ui_importbillitemmeasurestxt.h"

#include "billitemmeasuresmodel.h"
#include "billitemmeasure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QFileDialog>

class ImportBillItemMeasuresTXTPrivate {
public:
    ImportBillItemMeasuresTXTPrivate( BillItemMeasuresModel * mModel, int mPosition, MathParser * prs ):
        ui(new Ui::ImportBillItemMeasuresTXT),
        measuresModel(mModel),
        insertPosition(mPosition),
        parser(prs){
        fieldsNames << QObject::trUtf8("---");
        fieldsNames << QObject::trUtf8("Commento");
        fieldsNames << QObject::trUtf8("Misure");

        fieldsSeparator << qMakePair( QString("\t"), QObject::trUtf8("TAB") );
        fieldsSeparator << qMakePair( QString(" "), QObject::trUtf8("Spazio") );
    }

    ~ImportBillItemMeasuresTXTPrivate(){
        delete ui;
    }

    Ui::ImportBillItemMeasuresTXT *ui;
    QList<QComboBox *> fieldComboBoxList;
    QList<QString> fieldsNames;
    QList< QPair<QString, QString> > fieldsSeparator;
    BillItemMeasuresModel * measuresModel;
    int insertPosition;
    MathParser * parser;
    QString fileName;
};

ImportBillItemMeasuresTXT::ImportBillItemMeasuresTXT( BillItemMeasuresModel * mModel, int mPosition, MathParser * prs, QWidget *parent) :
    QDialog(parent),
    m_d( new ImportBillItemMeasuresTXTPrivate( mModel, mPosition, prs ) ){
    m_d->ui->setupUi(this);

    setWindowTitle( trUtf8("Importa misure da file TXT") );

    m_d->ui->decimalSeparatorLineEdit->setText( trUtf8("."));
    m_d->ui->thousandSeparatorLineEdit->setText( trUtf8(""));

    connect( m_d->ui->insertFieldPushButton, &QPushButton::clicked, this, &ImportBillItemMeasuresTXT::insertFieldComboBox );
    connect( m_d->ui->removeFieldPushButton, &QPushButton::clicked, this, &ImportBillItemMeasuresTXT::removeFieldComboBox );
    connect( m_d->ui->editFileNameButton, &QToolButton::clicked, this, &ImportBillItemMeasuresTXT::editFileName );

    connect( m_d->ui->buttonBox, &QDialogButtonBox::accepted, this, &ImportBillItemMeasuresTXT::importMeasures );

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

ImportBillItemMeasuresTXT::~ImportBillItemMeasuresTXT() {
    delete m_d;
}

void ImportBillItemMeasuresTXT::insertFieldComboBox() {
    QComboBox * box = new QComboBox( this );

    // box populate
    for( QList<QString>::iterator i=m_d->fieldsNames.begin(); i != m_d->fieldsNames.end(); ++i ){
        box->addItem( *i );
    }
    m_d->fieldComboBoxList.append( box );
    m_d->ui->fieldComboBoxLayout->insertWidget( m_d->fieldComboBoxList.size()-1, box );
}

void ImportBillItemMeasuresTXT::removeFieldComboBox() {
    if( !m_d->fieldComboBoxList.isEmpty() ){
        m_d->ui->fieldComboBoxLayout->removeWidget( m_d->fieldComboBoxList.last() );
        delete m_d->fieldComboBoxList.takeLast();
    }
}

void ImportBillItemMeasuresTXT::editFileName() {
    QString fileName = QFileDialog::getOpenFileName( this,
                                                     trUtf8("Apri file TXT"), ".",
                                                     trUtf8("Documento di testo (*.txt)"));
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

void ImportBillItemMeasuresTXT::importMeasures() {
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
                    // misura
                    if( m_d->fieldComboBoxList.at(i)->currentIndex() == 2 ){
                        QString form = line.at(i);
                        form.remove( m_d->ui->thousandSeparatorLineEdit->text() );
                        form.replace( m_d->ui->decimalSeparatorLineEdit->text(), m_d->parser->decimalSeparator() );
                        m_d->measuresModel->measure(insPosEff)->setFormula( form );
                    }
                }
                insPosEff++;
            }
        }
    }

    accept();
}
