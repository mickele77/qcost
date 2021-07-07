#include "importbillitemmeasuresclipboard.h"
#include "ui_importbillitemmeasuresclipboard.h"

#include "measuresmodel.h"
#include "measure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QFileDialog>
#include <QClipboard>

class ImportBillItemMeasuresClipboardPrivate {
public:
    ImportBillItemMeasuresClipboardPrivate( MeasuresModel * mModel, int mPosition, MathParser * prs ):
        ui(new Ui::ImportBillItemMeasuresClipboard),
        measuresModel(mModel),
        insertPosition(mPosition),
        parser(prs){
    }

    ~ImportBillItemMeasuresClipboardPrivate(){
        delete ui;
    }

    Ui::ImportBillItemMeasuresClipboard *ui;
    MeasuresModel * measuresModel;
    int insertPosition;
    MathParser * parser;
    QString fileName;
};

ImportBillItemMeasuresClipboard::ImportBillItemMeasuresClipboard( MeasuresModel * mModel, int mPosition, MathParser * prs, QWidget *parent) :
    QDialog(parent),
    m_d( new ImportBillItemMeasuresClipboardPrivate( mModel, mPosition, prs ) ){
    m_d->ui->setupUi(this);

    setWindowTitle( tr("Importa misure da file TXT") );

    m_d->ui->decimalSeparatorLineEdit->setText( tr("."));
    m_d->ui->thousandSeparatorLineEdit->setText( tr(""));

    connect( m_d->ui->buttonBox, &QDialogButtonBox::accepted, this, &ImportBillItemMeasuresClipboard::importComments );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

ImportBillItemMeasuresClipboard::~ImportBillItemMeasuresClipboard() {
    delete m_d;
}

void ImportBillItemMeasuresClipboard::importComments() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString clipText = clipboard->text();

    if( !clipText.isEmpty() ){
        QStringList lines = clipText.split(QRegExp("[\r\n]"), Qt::SkipEmptyParts);
        int insPosEff = m_d->insertPosition;
        for( auto line = lines.begin(); line != lines.end(); ++line ) {
            m_d->measuresModel->insertRows( insPosEff, 1 );
            QString com = *line;
            com.remove( m_d->ui->thousandSeparatorLineEdit->text() );
            com.replace( m_d->ui->decimalSeparatorLineEdit->text(), m_d->parser->decimalSeparator() );
            m_d->measuresModel->measure(insPosEff)->setComment( com );
            insPosEff++;
        }
    }

    accept();
}
