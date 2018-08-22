/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2016 Mocciola Michele

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include "priceitemgui.h"

#include "ui_priceitemgui.h"

#include "pricedatatabledelegate.h"
#include "importpriceitemdbdialog.h"
#include "editpriceitemapdialog.h"
#include "priceitemdatasetviewmodel.h"

#include "project.h"
#include "priceitemdatasetmodel.h"
#include "pricefieldmodel.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"
#include "unitmeasure.h"
#include "mathparser.h"

class PriceItemGUIPrivate{
public:
    PriceItemGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * impOptions, QString * fileName,
                         MathParser * prs, Project * prj ):
        project(prj),
        priceFieldModel( prj->priceFieldModel() ),
        parser( prs ),
        unitMeasureModel( prj->unitMeasureModel() ),
        priceItem(NULL),
        currentPriceDataSet(0),
        ui(new Ui::PriceItemGUI() ),
        EPAImportOptions(impOptions),
        EPAFileName(fileName){
    };
    Project * project;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    UnitMeasureModel * unitMeasureModel;

    PriceItem * priceItem;
    int currentPriceDataSet;

    Ui::PriceItemGUI *ui;
    PriceItemDataSetViewModel * dataSetViewModel;
    PriceDataTableDelegate * dataTableDelegate;
    QList<QLabel *> singlePriceDataFieldLabels;
    QList<QLineEdit *> singlePriceDataFieldLineEdits;
    QMap<PriceListDBWidget::ImportOptions, bool> * EPAImportOptions;
    QString * EPAFileName;
};

PriceItemGUI::PriceItemGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                            QString *EPAFileName,
                            PriceItem * pr, int curPriceDataSet,
                            MathParser * parser, Project *prj,
                            QWidget *parent ) :
    QWidget(parent),
    m_d( new PriceItemGUIPrivate( EPAImpOptions, EPAFileName, parser, prj ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->priceDataGroupBox->setVisible( false );
    m_d->ui->parentCodeLineEdit->setVisible( false );
    m_d->ui->parentShortDescLineEdit->setVisible( false );
    m_d->ui->parentLongDescTextEdit->setVisible( false );
    m_d->ui->importPriceItemDBButton->setDisabled( true );

    m_d->dataSetViewModel = new PriceItemDataSetViewModel( NULL, 0, parser, this );
    m_d->ui->dataTableView->setModel( m_d->dataSetViewModel );

    setPriceItem( pr, curPriceDataSet );

    m_d->dataTableDelegate = new PriceDataTableDelegate();
    m_d->dataTableDelegate->setModel( m_d->dataSetViewModel );
    m_d->ui->dataTableView->setItemDelegate( m_d->dataTableDelegate );
    connect( m_d->dataTableDelegate, &PriceDataTableDelegate::editAssociatedAP, this, static_cast<void(PriceItemGUI::*)(const QModelIndex &)>(&PriceItemGUI::editPriceItemAP) );

    updateUnitMeasureComboBox();
    connect( m_d->unitMeasureModel, &UnitMeasureModel::modelChanged, this, &PriceItemGUI::updateUnitMeasureComboBox );

    connect( m_d->ui->addDataColPushButton, &QPushButton::clicked, this, &PriceItemGUI::insertPriceDataSet );
    connect( m_d->ui->delDataColPushButton, &QPushButton::clicked, this, &PriceItemGUI::removePriceDataSet );
    connect( m_d->ui->setCurrentPriceColPushButton, &QPushButton::clicked, this, static_cast<void(PriceItemGUI::*)()>(&PriceItemGUI::setCurrentPriceDataSet) );
    connect( m_d->ui->importPriceItemDBButton, &QPushButton::clicked, this, &PriceItemGUI::importSingePriceItemDB );

    // sincronizza la descrizione lunga
    m_d->ui->longDescTextEdit->installEventFilter(this);

    updateFieldList();
    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &PriceItemGUI::updateFieldList );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &PriceItemGUI::updateFieldList );
    connect( m_d->priceFieldModel, &PriceFieldModel::modelChanged, this, &PriceItemGUI::updateFieldList );

    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &PriceItemGUI::updatePriceDataTable );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &PriceItemGUI::updatePriceDataTable );
    connect( m_d->ui->showDeltaPriceColCheckBox, &QCheckBox::clicked, this, &PriceItemGUI::updatePriceDataTable );
}

PriceItemGUI::~PriceItemGUI(){
    delete m_d;
}

void PriceItemGUI::updateFieldList(){
    for( QList<QLabel *>::iterator i = m_d->singlePriceDataFieldLabels.begin(); i != m_d->singlePriceDataFieldLabels.end(); ++i){
        m_d->ui->singleDataGroupBoxLayout->removeWidget( *i );
        delete (*i);
    }
    m_d->singlePriceDataFieldLabels.clear();
    for( QList<QLineEdit *>::iterator i = m_d->singlePriceDataFieldLineEdits.begin(); i != m_d->singlePriceDataFieldLineEdits.end(); ++i){
        m_d->ui->singleDataGroupBoxLayout->removeWidget( *i );
        delete (*i);
    }
    m_d->singlePriceDataFieldLineEdits.clear();
    for( int i=0; i < m_d->priceFieldModel->fieldCount(); ++i ){
        QLabel * label = new QLabel( m_d->priceFieldModel->priceName(i), m_d->ui->singleDataGroupBox );
        m_d->ui->singleDataGroupBoxLayout->addWidget( label, i, 0 );
        m_d->singlePriceDataFieldLabels.append( label );

        QLineEdit * lEdit = new QLineEdit( m_d->ui->singleDataGroupBox );
        bool ro = m_d->priceFieldModel->applyFormula(i)==PriceFieldModel::ToPriceItems || m_d->priceFieldModel->applyFormula(i)==PriceFieldModel::ToPriceAndBillItems;
        lEdit->setReadOnly( ro );
        lEdit->setAlignment( Qt::AlignRight );
        if( m_d->priceItem != NULL ){
            if( m_d->parser != NULL ){
                lEdit->setText( m_d->parser->toString( m_d->priceItem->value(i, m_d->currentPriceDataSet ), 'f', m_d->priceFieldModel->precision(i) ) );
            } else {
                lEdit->setText( QString::number( m_d->priceItem->value(i, m_d->currentPriceDataSet ), 'f', m_d->priceFieldModel->precision(i) ) );
            }
        }
        connect( lEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setPriceValueFromLineEdit );
        m_d->ui->singleDataGroupBoxLayout->addWidget( lEdit, i, 1 );
        m_d->singlePriceDataFieldLineEdits.append( lEdit );
    }
}

void PriceItemGUI::editPriceItemAP( const QModelIndex & index ){
    if( m_d->priceItem != NULL ){
        if( index.column()  < m_d->priceItem->priceDataSetCount() ){
            if( m_d->priceItem->associateAP(index.column() ) ){
                EditPriceItemAPDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, m_d->priceItem, index.column(), m_d->project, m_d->parser, this );
                dialog.exec();
            }
        }
    }
}

void PriceItemGUI::editPriceItemAP(){
    if( m_d->priceItem != NULL ){
        int curPriceCol = m_d->currentPriceDataSet<0?0:m_d->currentPriceDataSet;
        if( curPriceCol  < m_d->priceItem->priceDataSetCount() ){
            if( m_d->priceItem->associateAP( curPriceCol ) ){
                EditPriceItemAPDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, m_d->priceItem, curPriceCol, m_d->project, m_d->parser, this );
                dialog.exec();
            }
        }
    }
}

void PriceItemGUI::clearWidgets() {
    m_d->ui->parentCodeLineEdit->clear();
    m_d->ui->parentCodeLineEdit->setVisible( false );

    m_d->ui->parentShortDescLineEdit->clear();
    m_d->ui->parentShortDescLineEdit->setVisible( false );

    m_d->ui->parentLongDescTextEdit->clear();
    m_d->ui->parentLongDescTextEdit->setVisible( false );

    m_d->ui->codeCheckBox->setChecked( false );
    m_d->ui->codeCheckBox->setVisible( false );

    m_d->ui->codeLineEdit->clear();

    m_d->ui->shortDescCheckBox->setChecked(false );
    m_d->ui->shortDescCheckBox->setVisible( false );

    m_d->ui->shortDescLineEdit->clear();

    m_d->ui->longDescCheckBox->setChecked(false);
    m_d->ui->longDescCheckBox->setVisible( false );

    m_d->ui->longDescTextEdit->clear();

    // m_d->ui->dataTableView->setModel(NULL);
    m_d->dataSetViewModel->setModel( NULL );

    m_d->ui->unitMeasureComboBox->setCurrentIndex(0);
}

void PriceItemGUI::setPriceItem(PriceItem * newPriceItem, int newCurPriceDataSet ) {
    if( m_d->priceItem != newPriceItem ){
        if( m_d->priceItem ){
            if( m_d->priceItem->parentItem() != NULL ){
                if( m_d->priceItem->parentItem()->parentItem() != NULL ){
                    // connette code
                    disconnect( m_d->ui->codeCheckBox, &QCheckBox::toggled, m_d->priceItem, &PriceItem::setInheritCodeFromParent );
                    disconnect( m_d->priceItem, &PriceItem::inheritCodeFromParentChanged,m_d->ui->parentCodeLineEdit, &QLineEdit::setVisible );
                    disconnect( m_d->priceItem->parentItem(), &PriceItem::codeFullChanged, m_d->ui->parentCodeLineEdit, &QLineEdit::setText );

                    // connette shortDesc
                    disconnect( m_d->ui->shortDescCheckBox, &QCheckBox::toggled, m_d->priceItem, &PriceItem::setInheritShortDescFromParent );
                    disconnect( m_d->priceItem, &PriceItem::inheritShortDescFromParentChanged, m_d->ui->parentShortDescLineEdit, &QLineEdit::setVisible );
                    disconnect( m_d->priceItem->parentItem(), &PriceItem::shortDescriptionFullChanged, m_d->ui->parentShortDescLineEdit, &QLineEdit::setText );

                    // connette longDesc
                    disconnect( m_d->ui->longDescCheckBox, &QCheckBox::toggled, m_d->priceItem, &PriceItem::setInheritLongDescFromParent );
                    disconnect( m_d->priceItem, &PriceItem::inheritLongDescFromParentChanged, m_d->ui->parentLongDescTextEdit, &QPlainTextEdit::setVisible );
                    disconnect( m_d->priceItem->parentItem(), &PriceItem::longDescriptionFullChanged, this, &PriceItemGUI::setParentLongDescriptionGUI );
                }
            }

            disconnect( m_d->priceItem, &PriceItem::aboutToBeDeleted, this, &PriceItemGUI::setPriceItemNULL );

            // disconnette code
            disconnect( m_d->priceItem, &PriceItem::codeChanged, m_d->ui->codeLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->codeLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setCodeFromLineEdit );

            // disconnette shortDesc
            disconnect( m_d->priceItem, &PriceItem::shortDescriptionChanged, m_d->ui->shortDescLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->shortDescLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setShortDescriptionFromLineEdit );

            // disconnette longDesc
            disconnect( m_d->priceItem, &PriceItem::longDescriptionChanged, m_d->ui->longDescTextEdit, &QPlainTextEdit::setPlainText );
            disconnect( m_d->ui->longDescTextEdit, &QPlainTextEdit::textChanged, this, &PriceItemGUI::setLongDescFromTextEdit );

            // disconnette unita' di misura
            disconnect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, static_cast<void(PriceItemGUI::*) (UnitMeasure *)>(&PriceItemGUI::setItemUnitMeasureFromComboBox) );
            disconnect( m_d->ui->unitMeasureComboBox, static_cast<void(QComboBox::*) ( int )>(&QComboBox::currentIndexChanged), this, static_cast<void(PriceItemGUI::*) ( int )>(&PriceItemGUI::setUnitMeasure) );

            // disconnette valori
            disconnect( m_d->priceItem, &PriceItem::valueChanged, this, &PriceItemGUI::setPriceValueToLineEdit );
            disconnect( m_d->ui->singleDataEditAPPushButton, &QPushButton::clicked, this, static_cast<void(PriceItemGUI::*)()> (&PriceItemGUI::editPriceItemAP) );
            disconnect( m_d->ui->singleDataAssociateAPCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setPriceValueAssociateAP );
            disconnect( m_d->ui->singleDataAssociateAPCheckBox, &QCheckBox::toggled, m_d->ui->singleDataEditAPPushButton, &QPushButton::setEnabled );
            disconnect( m_d->priceItem, &PriceItem::associateAPChanged, this, &PriceItemGUI::setAssociatedAPToCheckBox );
            // spese generali
            disconnect( m_d->ui->singleDataOverheadsLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setOverheadsFromLineEdit );
            disconnect( m_d->priceItem, &PriceItem::overheadsChanged, this, &PriceItemGUI::setOverheadsToLineEdit );
            disconnect( m_d->ui->singleDataIOFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIOFRFromCheckBox );
            disconnect( m_d->priceItem, &PriceItem::inheritOverheadsFromRootChanged, this, &PriceItemGUI::setIOFRToCheckBox );
            // utili
            disconnect( m_d->ui->singleDataProfitsLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setProfitsFromLineEdit );
            disconnect( m_d->priceItem, &PriceItem::profitsChanged, this, &PriceItemGUI::setProfitsToLineEdit );
            disconnect( m_d->ui->singleDataIPFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIPFRFromCheckBox );
            disconnect( m_d->priceItem, &PriceItem::inheritProfitsFromRootChanged, this, &PriceItemGUI::setIPFRToCheckBox );

            disconnect( m_d->priceItem, static_cast<void(PriceItem::*)(bool)>(&PriceItem::hasChildrenChanged), this, &PriceItemGUI::updatePriceDataGUI );

            // m_d->ui->dataTableView->setModel( NULL );
            // m_d->dataTableDelegate->setModel( NULL );
            m_d->dataSetViewModel->setModel( NULL );
        }

        clearWidgets();
        m_d->priceItem = newPriceItem;
        m_d->ui->importPriceItemDBButton->setDisabled( m_d->priceItem == NULL );

        if( m_d->priceItem != NULL ){
            connect( m_d->priceItem, &PriceItem::aboutToBeDeleted, this, &PriceItemGUI::setPriceItemNULL );

            // connette code
            m_d->ui->codeLineEdit->setText( m_d->priceItem->code() );
            m_d->ui->codeCheckBox->setChecked( m_d->priceItem->inheritCodeFromParent() );
            connect( m_d->priceItem, &PriceItem::codeChanged, m_d->ui->codeLineEdit, &QLineEdit::setText );
            connect( m_d->ui->codeLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setCodeFromLineEdit );

            // connette shortDesc
            m_d->ui->shortDescLineEdit->setText( m_d->priceItem->shortDescription() );
            m_d->ui->shortDescCheckBox->setChecked( m_d->priceItem->inheritShortDescFromParent() );
            connect( m_d->priceItem, &PriceItem::shortDescriptionChanged, m_d->ui->shortDescLineEdit, &QLineEdit::setText );
            connect( m_d->ui->shortDescLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setShortDescriptionFromLineEdit );

            // connette longDesc
            m_d->ui->longDescTextEdit->setPlainText( m_d->priceItem->longDescription() );
            m_d->ui->longDescCheckBox->setChecked( m_d->priceItem->inheritLongDescFromParent() );
            connect( m_d->priceItem, &PriceItem::longDescriptionChanged, m_d->ui->longDescTextEdit, &QPlainTextEdit::setPlainText );
            // connect( m_d->ui->longDescTextEdit, &QPlainTextEdit::textChanged, this, &PriceItemGUI::setLongDescFromTextEdit );

            // connette unita' di misura
            setItemUnitMeasureFromComboBox();
            connect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, static_cast<void(PriceItemGUI::*) (UnitMeasure *)>(&PriceItemGUI::setItemUnitMeasureFromComboBox) );
            connect( m_d->ui->unitMeasureComboBox, static_cast<void(QComboBox::*) ( int )>(&QComboBox::currentIndexChanged), this, static_cast<void(PriceItemGUI::*) ( int )>(&PriceItemGUI::setUnitMeasure) );

            // connette dati prezzo
            updatePriceDataGUI();
            // valori
            connect( m_d->priceItem, &PriceItem::valueChanged, this, &PriceItemGUI::setPriceValueToLineEdit );
            connect( m_d->ui->singleDataEditAPPushButton, &QPushButton::clicked, this, static_cast<void(PriceItemGUI::*)()> (&PriceItemGUI::editPriceItemAP) );
            connect( m_d->ui->singleDataAssociateAPCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setPriceValueAssociateAP );
            connect( m_d->ui->singleDataAssociateAPCheckBox, &QCheckBox::toggled, m_d->ui->singleDataEditAPPushButton, &QPushButton::setEnabled );
            connect( m_d->priceItem, &PriceItem::associateAPChanged, this, &PriceItemGUI::setAssociatedAPToCheckBox );
            // spese generali
            connect( m_d->ui->singleDataOverheadsLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setOverheadsFromLineEdit );
            connect( m_d->priceItem, &PriceItem::overheadsChanged, this, &PriceItemGUI::setOverheadsToLineEdit );
            connect( m_d->ui->singleDataIOFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIOFRFromCheckBox );
            connect( m_d->priceItem, &PriceItem::inheritOverheadsFromRootChanged, this, &PriceItemGUI::setIOFRToCheckBox );
            // utili
            connect( m_d->ui->singleDataProfitsLineEdit, &QLineEdit::editingFinished, this, &PriceItemGUI::setProfitsFromLineEdit );
            connect( m_d->priceItem, &PriceItem::profitsChanged, this, &PriceItemGUI::setProfitsToLineEdit );
            connect( m_d->ui->singleDataIPFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIPFRFromCheckBox );
            connect( m_d->priceItem, &PriceItem::inheritProfitsFromRootChanged, this, &PriceItemGUI::setIPFRToCheckBox );

            connect( m_d->priceItem, static_cast<void(PriceItem::*)(bool)>(&PriceItem::hasChildrenChanged), this, &PriceItemGUI::updatePriceDataGUI );

            m_d->dataSetViewModel->setModel( m_d->priceItem->dataModel() );

            if( m_d->priceItem->parentItem() != NULL ){
                if( m_d->priceItem->parentItem()->parentItem() != NULL ){
                    // connette code
                    m_d->ui->codeCheckBox->setVisible( true );
                    m_d->ui->codeCheckBox->setChecked( m_d->priceItem->inheritCodeFromParent() );
                    connect( m_d->ui->codeCheckBox, &QCheckBox::toggled, m_d->priceItem, &PriceItem::setInheritCodeFromParent );
                    connect( m_d->priceItem, &PriceItem::inheritCodeFromParentChanged,m_d->ui->parentCodeLineEdit, &QLineEdit::setVisible );
                    m_d->ui->parentCodeLineEdit->setVisible( m_d->priceItem->inheritCodeFromParent() );
                    m_d->ui->parentCodeLineEdit->setText( m_d->priceItem->parentItem()->codeFull() );
                    connect( m_d->priceItem->parentItem(), &PriceItem::codeFullChanged, m_d->ui->parentCodeLineEdit, &QLineEdit::setText );

                    // connette shortDesc
                    m_d->ui->shortDescCheckBox->setVisible( true );
                    m_d->ui->shortDescCheckBox->setChecked( m_d->priceItem->inheritShortDescFromParent() );
                    connect( m_d->ui->shortDescCheckBox, &QCheckBox::toggled, m_d->priceItem, &PriceItem::setInheritShortDescFromParent );
                    connect( m_d->priceItem, &PriceItem::inheritShortDescFromParentChanged, m_d->ui->parentShortDescLineEdit, &QLineEdit::setVisible );
                    m_d->ui->parentShortDescLineEdit->setVisible( m_d->priceItem->inheritShortDescFromParent() );
                    m_d->ui->parentShortDescLineEdit->setText( m_d->priceItem->parentItem()->shortDescriptionFull() );
                    connect( m_d->priceItem->parentItem(), &PriceItem::shortDescriptionFullChanged, m_d->ui->parentShortDescLineEdit, &QLineEdit::setText );

                    // connette longDesc
                    m_d->ui->longDescCheckBox->setVisible( true );
                    m_d->ui->longDescCheckBox->setChecked( m_d->priceItem->inheritLongDescFromParent() );
                    connect( m_d->ui->longDescCheckBox, &QCheckBox::toggled, m_d->priceItem, &PriceItem::setInheritLongDescFromParent );
                    connect( m_d->priceItem, &PriceItem::inheritLongDescFromParentChanged, m_d->ui->parentLongDescTextEdit, &QPlainTextEdit::setVisible );
                    m_d->ui->parentLongDescTextEdit->setVisible( m_d->priceItem->inheritLongDescFromParent() );
                    m_d->ui->parentLongDescTextEdit->setPlainText( m_d->priceItem->parentItem()->longDescriptionFull() );
                    connect( m_d->priceItem->parentItem(), &PriceItem::longDescriptionFullChanged, this, &PriceItemGUI::setParentLongDescriptionGUI );
                } else {
                    // connette code
                    m_d->ui->codeCheckBox->setVisible( false );
                    m_d->ui->parentCodeLineEdit->setVisible( false );
                    // connette shortDesc
                    m_d->ui->shortDescCheckBox->setVisible( false );
                    m_d->ui->parentShortDescLineEdit->setVisible( false );
                    // connette longDesc
                    m_d->ui->longDescCheckBox->setVisible( false );
                    m_d->ui->parentLongDescTextEdit->setVisible( false );
                }
            }
        }
        updatePriceDataTable();
    }
    if( m_d->priceItem != NULL ){
        setCurrentPriceDataSet( newCurPriceDataSet );
    }
}

void PriceItemGUI::setAssociatedAPToCheckBox(int pCol,bool val ){
    if( m_d->priceItem != NULL ){
        if( m_d->currentPriceDataSet == pCol ){
            m_d->ui->singleDataAssociateAPCheckBox->setChecked( val );
            m_d->ui->singleDataEditAPPushButton->setEnabled( val );
        }
    }
}

void PriceItemGUI::setCodeFromLineEdit(){
    if( m_d->priceItem != NULL ){
        m_d->priceItem->setCode(  m_d->ui->codeLineEdit->text() );
    }
}

void PriceItemGUI::setShortDescriptionFromLineEdit(){
    if( m_d->priceItem != NULL ){
        m_d->priceItem->setShortDescription(  m_d->ui->shortDescLineEdit->text() );
    }
}

void PriceItemGUI::setLongDescFromTextEdit(){
    if( m_d->priceItem != NULL ){
        m_d->priceItem->setLongDescription( m_d->ui->longDescTextEdit->toPlainText() );
    }
}

void PriceItemGUI::emitImportSinglePriceItemDB() {
    if( m_d->priceItem != NULL ){
        emit importSinglePriceItemDB( m_d->priceItem );
    }
}

bool PriceItemGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::FocusOut)     {
        if( m_d->priceItem != NULL ){
            if (object == m_d->ui->longDescTextEdit)  {
                m_d->priceItem->setLongDescription( m_d->ui->longDescTextEdit->toPlainText() );
            }
        }
    }
    return false;
}

void PriceItemGUI::setCurrentPriceDataSet(int priceDataSet) {
    if( m_d->currentPriceDataSet != priceDataSet ){
        m_d->currentPriceDataSet = priceDataSet;
        updatePriceDataGUI();
        m_d->dataSetViewModel->setCurrentPriceDataSet( priceDataSet );
    }
}

void PriceItemGUI::setPriceValueToLineEdit( int priceField, int priceCol, const QString & newVal ){
    if( priceCol == m_d->currentPriceDataSet ){
        if( priceField < m_d->singlePriceDataFieldLineEdits.size() ){
            if( newVal != m_d->singlePriceDataFieldLineEdits.at( priceField )->text() ){
                m_d->singlePriceDataFieldLineEdits.at( priceField )->setText( newVal );
            }
        }
    }
}

void PriceItemGUI::setPriceValueFromLineEdit(){
    if( m_d->priceItem ){
        QLineEdit * lEdit = qobject_cast<QLineEdit *>(QObject::sender());
        if( lEdit ){
            for( int i=0; i < m_d->singlePriceDataFieldLineEdits.size(); ++i ){
                if( m_d->singlePriceDataFieldLineEdits.at(i) == lEdit ){
                    if( m_d->parser ){
                        QString v = lEdit->text();
                        m_d->priceItem->setValue( i, m_d->currentPriceDataSet, m_d->parser->evaluate(v));
                    } else {
                        m_d->priceItem->setValue( i, m_d->currentPriceDataSet, lEdit->text().toDouble() );
                    }
                    break;
                }
            }
        }
    }
}

void PriceItemGUI::setOverheadsToLineEdit( int priceDataSet, const QString & newVal ){
    if( priceDataSet == m_d->currentPriceDataSet ){
        if( newVal != m_d->ui->singleDataOverheadsLineEdit->text() ){
            m_d->ui->singleDataOverheadsLineEdit->setText( newVal );
        }
    }
}

void PriceItemGUI::setOverheadsFromLineEdit(){
    if( m_d->priceItem ){
        m_d->priceItem->setOverheads( m_d->currentPriceDataSet, m_d->ui->singleDataOverheadsLineEdit->text() );
    }
}

void PriceItemGUI::setIOFRFromCheckBox(){
    if( m_d->priceItem ){
        m_d->priceItem->setInheritOverheadsFromRoot( m_d->currentPriceDataSet, m_d->ui->singleDataIOFRCheckBox->isChecked() );
    }
}

void PriceItemGUI::setIOFRToCheckBox( int priceDataSet, bool newVal ){
    if( priceDataSet == m_d->currentPriceDataSet ){
        if( newVal != m_d->ui->singleDataIOFRCheckBox->isChecked() ){
            m_d->ui->singleDataIOFRCheckBox->setChecked( newVal );
        }
    }
}

void PriceItemGUI::setProfitsToLineEdit( int priceDataSet, const QString & newVal ){
    if( priceDataSet == m_d->currentPriceDataSet ){
        if( newVal != m_d->ui->singleDataProfitsLineEdit->text() ){
            m_d->ui->singleDataProfitsLineEdit->setText( newVal );
        }
    }
}

void PriceItemGUI::setProfitsFromLineEdit(){
    if( m_d->priceItem ){
        m_d->priceItem->setProfits( m_d->currentPriceDataSet, m_d->ui->singleDataProfitsLineEdit->text() );
    }
}

void PriceItemGUI::setIPFRFromCheckBox(){
    if( m_d->priceItem ){
        m_d->priceItem->setInheritProfitsFromRoot( m_d->currentPriceDataSet, m_d->ui->singleDataIPFRCheckBox->isChecked() );
    }
}

void PriceItemGUI::setIPFRToCheckBox( int priceDataSet, bool newVal ){
    if( priceDataSet == m_d->currentPriceDataSet ){
        if( newVal != m_d->ui->singleDataIPFRCheckBox->isChecked() ){
            m_d->ui->singleDataIPFRCheckBox->setChecked( newVal );
        }
    }
}

void PriceItemGUI::setPriceValueAssociateAP(bool newVal){
    if( m_d->priceItem ){
        int cp = m_d->currentPriceDataSet<0?0:m_d->currentPriceDataSet;
        m_d->priceItem->setAssociateAP( cp, newVal );
    }
}

void PriceItemGUI::setParentLongDescriptionGUI(){
    if( m_d->priceItem ){
        if( m_d->priceItem->parentItem()  ){
            m_d->ui->parentLongDescTextEdit->setPlainText( m_d->priceItem->parentItem()->longDescriptionFull() );
        }
    }
}

void PriceItemGUI::updateUnitMeasureComboBox() {
    if( m_d->priceItem != NULL ){
        disconnect( m_d->ui->unitMeasureComboBox, static_cast<void(QComboBox::*)( int )>(&QComboBox::currentIndexChanged), this, &PriceItemGUI::setUnitMeasure );
    }
    m_d->ui->unitMeasureComboBox->clear();
    m_d->ui->unitMeasureComboBox->addItem( QString("---"), qVariantFromValue((void *) NULL ));
    for( int i=0; i<m_d->unitMeasureModel->size(); ++i){
        m_d->ui->unitMeasureComboBox->addItem( m_d->unitMeasureModel->unitMeasure(i)->tag(), qVariantFromValue((void *) m_d->unitMeasureModel->unitMeasure(i) ));
    }

    if( m_d->priceItem != NULL ){
        int i = m_d->ui->unitMeasureComboBox->findData( qVariantFromValue((void *) m_d->priceItem->unitMeasure() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->unitMeasureComboBox->setCurrentIndex( i );
        connect( m_d->ui->unitMeasureComboBox, static_cast<void(QComboBox::*)( int )>(&QComboBox::currentIndexChanged), this, &PriceItemGUI::setUnitMeasure );
    }
}

void PriceItemGUI::setPriceItemNULL() {
    setPriceItem( NULL );
}

void PriceItemGUI::setItemUnitMeasureFromComboBox() {
    if( m_d->priceItem ){
        int i = m_d->ui->unitMeasureComboBox->findData( qVariantFromValue((void *) m_d->priceItem->unitMeasure() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->unitMeasureComboBox->setCurrentIndex( i );
    }
}

void PriceItemGUI::setItemUnitMeasureFromComboBox( UnitMeasure * ump ) {
    int i = m_d->ui->unitMeasureComboBox->findData( qVariantFromValue((void *) ump ) );
    if( i < 0 ){
        i = 0;
    }
    m_d->ui->unitMeasureComboBox->setCurrentIndex( i );
}

void PriceItemGUI::setUnitMeasure(int i) {
    if( m_d->priceItem ){
        QVariant v = m_d->ui->unitMeasureComboBox->itemData(i);
        UnitMeasure * ump = (UnitMeasure *) v.value<void *>();
        m_d->priceItem->setUnitMeasure( ump );
    }
}

void PriceItemGUI::insertPriceDataSet() {
    if( m_d->priceItem ){
        if( m_d->ui->dataTableView->selectionModel() ){
            QModelIndexList selCols = m_d->ui->dataTableView->selectionModel()->selectedColumns();
            if( selCols.isEmpty() ){
                m_d->priceItem->dataModel()->insertPriceDataSet( -1, 1 );
            } else {
                m_d->priceItem->dataModel()->insertPriceDataSet( selCols.first().column(), selCols.size() );
            }
        }
    }
}

void PriceItemGUI::removePriceDataSet() {
    if( m_d->priceItem ){
        if( m_d->ui->dataTableView->selectionModel() ){
            QModelIndexList selCols = m_d->ui->dataTableView->selectionModel()->selectedColumns();
            if( !selCols.isEmpty() ){
                m_d->priceItem->dataModel()->removePriceDataSet( selCols.first().column(), selCols.size() );
            }
        }
    }
}

void PriceItemGUI::setCurrentPriceDataSet(){
    if( m_d->priceItem ){
        QModelIndex currIndex = m_d->ui->dataTableView->currentIndex();
        if( currIndex.isValid() ){
            emit currentPriceDataSetChanged( currIndex.column() );
        }
    }
}

void PriceItemGUI::updatePriceDataGUI(){
    if( m_d->priceItem != NULL ){
        if( m_d->priceItem->hasChildren() ){
            m_d->ui->priceDataGroupBox->setVisible( false );
        } else {
            m_d->ui->priceDataGroupBox->setVisible( true );
            // connette dati prezzo
            for( int i=0; i < m_d->singlePriceDataFieldLineEdits.size(); ++i){
                if( i < m_d->priceFieldModel->fieldCount() ){
                    m_d->singlePriceDataFieldLineEdits.at(i)->setText( m_d->priceItem->valueStr( i, m_d->currentPriceDataSet ) );
                }
            }
            disconnect( m_d->ui->singleDataOverheadsLineEdit, &QLineEdit::textChanged, this, &PriceItemGUI::setOverheadsFromLineEdit );
            m_d->ui->singleDataOverheadsLineEdit->setText( m_d->priceItem->overheadsStr( m_d->currentPriceDataSet ) );
            connect( m_d->ui->singleDataOverheadsLineEdit, &QLineEdit::textChanged, this, &PriceItemGUI::setOverheadsFromLineEdit );

            disconnect( m_d->ui->singleDataIOFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIOFRFromCheckBox );
            m_d->ui->singleDataIOFRCheckBox->setChecked( m_d->priceItem->inheritOverheadsFromRoot(m_d->currentPriceDataSet) );
            connect( m_d->ui->singleDataIOFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIOFRFromCheckBox );

            disconnect( m_d->ui->singleDataProfitsLineEdit, &QLineEdit::textChanged, this, &PriceItemGUI::setProfitsFromLineEdit );
            m_d->ui->singleDataProfitsLineEdit->setText( m_d->priceItem->profitsStr( m_d->currentPriceDataSet ) );
            connect( m_d->ui->singleDataProfitsLineEdit, &QLineEdit::textChanged, this, &PriceItemGUI::setProfitsFromLineEdit );

            disconnect( m_d->ui->singleDataIPFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIPFRFromCheckBox );
            m_d->ui->singleDataIPFRCheckBox->setChecked( m_d->priceItem->inheritProfitsFromRoot(m_d->currentPriceDataSet) );
            connect( m_d->ui->singleDataIPFRCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setIPFRFromCheckBox );

            disconnect( m_d->ui->singleDataAssociateAPCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setPriceValueAssociateAP );
            m_d->ui->singleDataAssociateAPCheckBox->setChecked( m_d->priceItem->associateAP(m_d->currentPriceDataSet) );
            m_d->ui->singleDataEditAPPushButton->setEnabled( m_d->priceItem->associateAP(m_d->currentPriceDataSet) );
            connect( m_d->ui->singleDataAssociateAPCheckBox, &QCheckBox::toggled, this, &PriceItemGUI::setPriceValueAssociateAP );
        }
    }
}

void PriceItemGUI::updatePriceDataTable(){
    for( int i=0; i < m_d->priceFieldModel->fieldCount(); ++i ){
        m_d->ui->dataTableView->setRowHidden( 2*i+1, !m_d->ui->showDeltaPriceColCheckBox->isChecked() );
    }
}

void PriceItemGUI::importSingePriceItemDB(){
    if( m_d->priceItem ){
        ImportPriceItemDBDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, m_d->priceItem, m_d->currentPriceDataSet, "PriceItemGUI::importSingePriceItemDB", m_d->parser, m_d->unitMeasureModel, this );
        dialog.exec();
    }
}
