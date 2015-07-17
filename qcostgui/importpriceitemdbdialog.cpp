#include "importpriceitemdbdialog.h"

#include "priceitem.h"
#include "unitmeasuremodel.h"

#include <QGridLayout>
#include <QDialogButtonBox>
#include <QVariant>

class ImportPriceItemDBDialogPrivate{
public:
    ImportPriceItemDBDialogPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * impOptions, QString * fileName,
                                    PriceItem * importPriceItem, int colPrice,
                                    const QString & conName,
                                    MathParser * p, UnitMeasureModel * uml,
                                    QWidget *parent):
        mainLayout( new QGridLayout(parent)),
        dbWidget( new PriceListDBWidget( impOptions, fileName, p, conName, parent )),
        buttonBox( new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) ),
        importingDataPriceItem(importPriceItem),
        priceDataSet(colPrice),
        unitMeasureModel(uml) {
        mainLayout->addWidget( dbWidget, 0, 0);
        mainLayout->addWidget( buttonBox, 1, 0);
    }

    QGridLayout * mainLayout;
    PriceListDBWidget * dbWidget;
    QDialogButtonBox * buttonBox;
    PriceItem * importingDataPriceItem;
    int priceDataSet;
    UnitMeasureModel * unitMeasureModel;
};

ImportPriceItemDBDialog::ImportPriceItemDBDialog(QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                                 QString * EPAFileName,
                                                 PriceItem * importPriceItem, int priceDataSet,
                                                 const QString & connectionName,
                                                 MathParser *prs, UnitMeasureModel * uml,
                                                 QWidget *parent ) :
    QDialog(parent),
    m_d( new ImportPriceItemDBDialogPrivate( EPAImpOptions, EPAFileName, importPriceItem, priceDataSet, connectionName, prs, uml, this )) {
    connect( m_d->buttonBox, &QDialogButtonBox::accepted, this, &ImportPriceItemDBDialog::importSinglePriceItemDB );
    connect( m_d->buttonBox, &QDialogButtonBox::rejected, this, &ImportPriceItemDBDialog::reject );
    m_d->dbWidget->setMode( PriceListDBWidget::ImportSingle );
    setWindowTitle(trUtf8("Importa voce di prezzo da E.P.A."));
}

ImportPriceItemDBDialog::ImportPriceItemDBDialog( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                                  QString * EPAFileName,
                                                  const QString & connectionName,
                                                  MathParser *prs, UnitMeasureModel * uml,
                                                  QWidget *parent ) :
    QDialog(parent),
    m_d( new ImportPriceItemDBDialogPrivate( EPAImpOptions, EPAFileName, NULL, 0, connectionName, prs, uml, this )) {
    connect( m_d->buttonBox, &QDialogButtonBox::accepted, this, static_cast<void(ImportPriceItemDBDialog::*)()>(&ImportPriceItemDBDialog::importMultiPriceItemDB) );
    connect( m_d->buttonBox, &QDialogButtonBox::rejected, this, &ImportPriceItemDBDialog::reject );
    m_d->dbWidget->setMode( PriceListDBWidget::ImportMulti );
    setWindowTitle(trUtf8("Importa da E.P.A."));

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

ImportPriceItemDBDialog::~ImportPriceItemDBDialog(){
    delete m_d;
}

void ImportPriceItemDBDialog::importSinglePriceItemDB(){
    QList<QPair<QString,QVariant> > data;
    m_d->dbWidget->importSinglePriceItemDB( &data );
    if( m_d->importingDataPriceItem != NULL ){
        for(int j=0; j < data.size(); ++j){
            if( data.at(j).first.toUpper() == "CODE" ){
                m_d->importingDataPriceItem->setCode( data.at(j).second.toString());
            } else if( data.at(j).first.toUpper() == "SHORTDESC" ){
                m_d->importingDataPriceItem->setShortDescription( data.at(j).second.toString());
            } else if( data.at(j).first.toUpper() == "LONGDESC" ){
                m_d->importingDataPriceItem->setLongDescription( data.at(j).second.toString());
            } else if( data.at(j).first.toUpper() == "UNITMEASURE" ){
                if( data.at(j).second.toString() == "---"){
                    m_d->importingDataPriceItem->setUnitMeasure( NULL );
                } else {
                    int umRow = m_d->unitMeasureModel->findTag( data.at(j).second.toString() );
                    if( umRow < 0 ){
                        umRow = m_d->unitMeasureModel->append( data.at(j).second.toString() );
                    }
                    if( umRow > -1 ){
                        m_d->importingDataPriceItem->setUnitMeasure( m_d->unitMeasureModel->unitMeasure(umRow) );
                    }
                }
            } else if( data.at(j).first.toUpper() == "PRICETOTAL" ){
                m_d->importingDataPriceItem->setValue( PriceFieldModel::PriceTotal, m_d->priceDataSet, data.at(j).second.toDouble());
            } else if( data.at(j).first.toUpper() == "PRICEHUMAN" ){
                m_d->importingDataPriceItem->setValue( PriceFieldModel::PriceHuman, m_d->priceDataSet, data.at(j).second.toDouble());
            } else if( data.at(j).first.toUpper() == "PRICEEQUIPMENT" ){
                m_d->importingDataPriceItem->setValue( PriceFieldModel::PriceEquipment, m_d->priceDataSet, data.at(j).second.toDouble());
            } else if( data.at(j).first.toUpper() == "PRICEMATERIAL" ){
                m_d->importingDataPriceItem->setValue( PriceFieldModel::PriceMaterial, m_d->priceDataSet, data.at(j).second.toDouble());
            } else if( data.at(j).first.toUpper() == "OVERHEADS" ){
                m_d->importingDataPriceItem->setInheritOverheadsFromRoot( false );
                m_d->importingDataPriceItem->setOverheads( m_d->priceDataSet, data.at(j).second.toDouble() );
            } else if( data.at(j).first.toUpper() == "PROFITS" ){
                m_d->importingDataPriceItem->setInheritProfitsFromRoot( false );
                m_d->importingDataPriceItem->setProfits( m_d->priceDataSet, data.at(j).second.toDouble() );
            }
        }
    }
    accept();
}

void ImportPriceItemDBDialog::importMultiPriceItemDB(){
    QList< QList< QPair< QString, QVariant > > > itemDataList;
    QList<int> hierarchy;
    m_d->dbWidget->importMultiPriceItemDB( &itemDataList, &hierarchy );
    emit importMultiPriceItemDB( itemDataList, hierarchy );
    accept();
}
