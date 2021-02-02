#include "editpriceitemdialog.h"

#include "importpriceitemdbdialog.h"
#include "pricelisttreegui.h"
#include "priceitemgui.h"

#include "project.h"
#include "accountinglsbillitem.h"
#include "accountingtambillitem.h"
#include "accountingbillitem.h"
#include "billitem.h"
#include "pricelist.h"
#include "priceitem.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSplitter>

class EditPriceItemDialogPrivate{
public:
    EditPriceItemDialogPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * EPAFileName, PriceList * pl, int pCol,
                                BillItem * bItem, AccountingBillItem * aItem, AccountingTAMBillItem * aTAMItem, AccountingLSBillItem * aLSItem,
                                MathParser * prs, Project * prj, QWidget * parent ):
        billItem(bItem),
        accountingItem(aItem),
        accountingTAMItem(aTAMItem),
        accountingLSItem(aLSItem),
        priceDataSet(pCol),
        parser( prs ),
        unitMeasureModel( prj->unitMeasureModel() ),
        mainLayout( new QGridLayout( parent ) ),
        mainSplitter( new QSplitter( parent )),
        priceListTreeGUI( new PriceListTreeGUI( EPAImpOptions, EPAFileName, pl, pCol, prs, prj->priceFieldModel(), prj->unitMeasureModel(), mainSplitter )),
        priceItemGUI( new PriceItemGUI( EPAImpOptions, EPAFileName, nullptr, 0, prs, prj, mainSplitter )),
        buttonBox( new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)) {
        mainLayout->addWidget( mainSplitter, 0,0 );
        mainLayout->addWidget( buttonBox, 1, 0 );
        // (PriceItem*)
        QObject::connect( priceListTreeGUI, &PriceListTreeGUI::currentItemChanged, priceItemGUI, &PriceItemGUI::setPriceItem );
    }
    BillItem * billItem;
    AccountingBillItem * accountingItem;
    AccountingTAMBillItem * accountingTAMItem;
    AccountingLSBillItem * accountingLSItem;
    int priceDataSet;
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
                                          PriceList * pl, int priceDataSet, BillItem * bItem, MathParser * prs, Project * prj,
                                          QWidget *parent):
    QDialog( parent ),
    m_d( new EditPriceItemDialogPrivate( EPAImpOptions, EPAFileName, pl, priceDataSet, bItem, nullptr, nullptr, nullptr, prs, prj, this ) ) {
    init(pl);
}

EditPriceItemDialog::EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                          QString * EPAFileName,
                                          PriceList * pl, int priceDataSet, AccountingBillItem * aItem, MathParser * prs, Project * prj,
                                          QWidget *parent):
    QDialog( parent ),
    m_d( new EditPriceItemDialogPrivate( EPAImpOptions, EPAFileName, pl, priceDataSet, nullptr, aItem, nullptr, nullptr, prs, prj, this ) ) {
    init(pl);
}

EditPriceItemDialog::EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                          QString * EPAFileName,
                                          PriceList * pl, int priceDataSet, AccountingTAMBillItem * aItem,
                                          MathParser * prs, Project * prj,
                                          QWidget *parent):
    QDialog( parent ),
    m_d( new EditPriceItemDialogPrivate( EPAImpOptions, EPAFileName, pl, priceDataSet, nullptr, nullptr, aItem, nullptr, prs, prj, this ) ) {
    init(pl);
}

EditPriceItemDialog::EditPriceItemDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                          QString * EPAFileName,
                                          PriceList * pl, int priceDataSet, AccountingLSBillItem * lsItem,
                                          MathParser * prs, Project * prj,
                                          QWidget *parent):
    QDialog( parent ),
    m_d( new EditPriceItemDialogPrivate( EPAImpOptions, EPAFileName, pl, priceDataSet, nullptr, nullptr, nullptr, lsItem, prs, prj, this ) ) {
    init(pl);
}

void EditPriceItemDialog::init(PriceList * pl){
    connect( m_d->buttonBox, &QDialogButtonBox::accepted, this, &EditPriceItemDialog::changePriceItemAndClose );
    connect( m_d->buttonBox, &QDialogButtonBox::rejected, this, &EditPriceItemDialog::reject );

    m_d->priceListTreeGUI->setPriceList( pl, m_d->priceDataSet );
    if( m_d->billItem != nullptr ){
        m_d->priceListTreeGUI->setCurrentPriceItem( m_d->billItem->priceItem() );
    } else if( m_d->accountingItem != nullptr ){
        if( m_d->accountingItem->itemType() == AccountingBillItem::PPU ){
            m_d->priceListTreeGUI->setCurrentPriceItem( m_d->accountingItem->priceItem() );
        }
    } else if( m_d->accountingTAMItem != nullptr ){
        if( m_d->accountingTAMItem->itemType() == AccountingTAMBillItem::PPU ){
            m_d->priceListTreeGUI->setCurrentPriceItem( m_d->accountingTAMItem->priceItem() );
        }
    } else if( m_d->accountingLSItem != nullptr ){
        m_d->priceListTreeGUI->setCurrentPriceItem( m_d->accountingLSItem->priceItem() );
    }

    setWindowTitle( tr("Cambia articolo Elenco Prezzi - %1").arg( pl->name() ) );

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
            if( m_d->billItem != nullptr ){
                m_d->billItem->setPriceItem( pItem );
            } else if( m_d->accountingItem != nullptr ){
                if( m_d->accountingItem->itemType() == AccountingBillItem::PPU ){
                    m_d->accountingItem->setPriceItem( pItem );
                }
            } else if( m_d->accountingTAMItem != nullptr ){
                if( m_d->accountingTAMItem->itemType() == AccountingTAMBillItem::PPU ){
                    m_d->accountingTAMItem->setPriceItem( pItem );
                }
            } else if( m_d->accountingLSItem != nullptr ){
                m_d->accountingLSItem->setPriceItem( pItem );
            }
        }
    }
    accept();
}
