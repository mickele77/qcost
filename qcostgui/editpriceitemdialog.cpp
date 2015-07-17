#include "editpriceitemdialog.h"

#include "importpriceitemdbdialog.h"
#include "pricelisttreegui.h"
#include "priceitemgui.h"

#include "project.h"
#include "billitem.h"
#include "pricelist.h"
#include "priceitem.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSplitter>

class EditPriceItemDialogPrivate{
public:
    EditPriceItemDialogPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * EPAFileName, PriceList * pl, int pCol, BillItem * bItem, MathParser * prs, Project * prj, QWidget * parent ):
        billItem(bItem),
        priceCol(pCol),
        parser( prs ),
        unitMeasureModel( prj->unitMeasureModel() ),
        mainLayout( new QGridLayout( parent ) ),
        mainSplitter( new QSplitter( parent )),
        priceListTreeGUI( new PriceListTreeGUI( EPAImpOptions, EPAFileName, pl, pCol, prs, prj->priceFieldModel(), prj->unitMeasureModel(), mainSplitter )),
        priceItemGUI( new PriceItemGUI( EPAImpOptions, EPAFileName, NULL, 0, prs, prj, mainSplitter )),
        buttonBox( new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)) {
        mainLayout->addWidget( mainSplitter, 0,0 );
        mainLayout->addWidget( buttonBox, 1, 0 );
        // (PriceItem*)
        QObject::connect( priceListTreeGUI, &PriceListTreeGUI::currentItemChanged, priceItemGUI, &PriceItemGUI::setPriceItem );
    }
    BillItem * billItem;
    int priceCol;
    MathParser * parser;
    UnitMeasureModel * unitMeasureModel;
    QGridLayout * mainLayout;
    QSplitter * mainSplitter;
    PriceListTreeGUI * priceListTreeGUI;
    PriceItemGUI * priceItemGUI;
    QDialogButtonBox * buttonBox;
};

EditPriceItemDialog::EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                          QString * EPAFileName,
                                          PriceList * pl, int priceCol, BillItem * bItem, MathParser * prs, Project * prj,
                                          QWidget *parent):
    QDialog( parent ),
    m_d( new EditPriceItemDialogPrivate( EPAImpOptions, EPAFileName, pl, priceCol, bItem, prs, prj, this ) ) {
    connect( m_d->buttonBox, &QDialogButtonBox::accepted, this, &EditPriceItemDialog::changePriceItemAndClose );
    connect( m_d->buttonBox, &QDialogButtonBox::rejected, this, &EditPriceItemDialog::reject );

    m_d->priceListTreeGUI->setPriceList( pl, priceCol );
    m_d->priceListTreeGUI->setCurrentPriceItem( bItem->priceItem() );

    setWindowTitle( trUtf8("Cambia articolo Elenco Prezzi - %1").arg( pl->name() ) );

    connect( m_d->priceListTreeGUI, &PriceListTreeGUI::currentItemChanged, m_d->priceItemGUI, &PriceItemGUI::setPriceItem );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

EditPriceItemDialog::~EditPriceItemDialog(){
    delete m_d;
}

void EditPriceItemDialog::changePriceItemAndClose(){
    if( m_d->priceListTreeGUI->currentPriceItem() ) {
        if( !m_d->priceListTreeGUI->currentPriceItem()->hasChildren() ){
            PriceItem * pItem = m_d->priceListTreeGUI->currentPriceItem();
            m_d->billItem->setPriceItem( pItem );
        }
    }
    accept();
}
