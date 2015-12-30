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
#include "billattributeprintergui.h"
#include "ui_billattributeprintergui.h"

#include "attributesmodel.h"
#include "pricefieldmodel.h"

#include <QComboBox>
#include <QPageSize>

class BillAttributePrinterGUIPrivate{
public:
    BillAttributePrinterGUIPrivate( BillPrinter::PrintBillItemsOption * prItemsOption,
                                    BillPrinter::AttributePrintOption *prOption,
                                    QList<int> * pFlds,
                                    QList<Attribute *> * pAttrs,
                                    double * pWidth,
                                    double * pHeight,
                                    Qt::Orientation * pOrient,
                                    bool *groupPrAm,
                                    PriceFieldModel * pfm,
                                    AttributesModel * bam ):
        ui(new Ui::BillAttributePrinterGUI),
        printItemsOption(prItemsOption),
        printOption(prOption),
        printFields(pFlds),
        printAttributes(pAttrs),
        billAttributeModel(bam),
        paperWidth( pWidth ),
        paperHeight( pHeight ),
        paperOrientation( pOrient ),
        groupPriceAmount( groupPrAm ),
        priceFieldsNames( pfm->fieldNames() ){
        pageSizeList << QPageSize( QPageSize::A4 );
    };
    ~BillAttributePrinterGUIPrivate(){
        delete ui;
    };

    Ui::BillAttributePrinterGUI *ui;
    BillPrinter::PrintBillItemsOption * printItemsOption;
    QList<QComboBox *> priceFieldComboBoxList;
    BillPrinter::AttributePrintOption *printOption;
    QList<int> * printFields;
    QList<Attribute *> * printAttributes;
    AttributesModel * billAttributeModel;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    bool * groupPriceAmount;
    QList<QString> priceFieldsNames;
    QList<QPageSize> pageSizeList;
};

BillAttributePrinterGUI::BillAttributePrinterGUI( BillPrinter::PrintBillItemsOption * prItemsOption,
                                                  BillPrinter::AttributePrintOption *prOption,
                                                  QList<int> * pFlds,
                                                  QList<Attribute *> * pAttrs,
                                                  double * pWidth,
                                                  double * pHeight,
                                                  Qt::Orientation * pOrient,
                                                  bool *groupPrAm,
                                                  PriceFieldModel * pfm,
                                                  AttributesModel * bam,
                                                  QWidget *parent ) :
    QDialog(parent),
    m_d( new BillAttributePrinterGUIPrivate( prItemsOption, prOption, pFlds, pAttrs, pWidth, pHeight, pOrient, groupPrAm, pfm, bam) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributesTableView->setModel( m_d->billAttributeModel );

    connect( this, &BillAttributePrinterGUI::accepted, this, &BillAttributePrinterGUI::setPrintData );
    connect( m_d->ui->insertPriceFieldPushButton, &QPushButton::clicked, this, &BillAttributePrinterGUI::insertPriceFieldComboBox );
    connect( m_d->ui->removePriceFieldPushButton, &QPushButton::clicked, this, &BillAttributePrinterGUI::removePriceFieldComboBox );

    m_d->ui->groupPriceAmountCheckBox->setChecked( *groupPrAm );

    for( int i=0; i < m_d->pageSizeList.size(); ++i ){
        m_d->ui->paperDimensionsComboBox->addItem( m_d->pageSizeList.at(i).name() );
    }

    if( *(m_d->printItemsOption) == BillPrinter::PrintShortDesc ){
        m_d->ui->printShortDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == BillPrinter::PrintLongDesc ){
        m_d->ui->printLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == BillPrinter::PrintShortLongDesc ){
        m_d->ui->printShortLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == BillPrinter::PrintShortLongDescOpt ){
        m_d->ui->printShortLongDescOptRadioButton->setChecked( true );
    }

    insertPriceFieldComboBox();

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

BillAttributePrinterGUI::~BillAttributePrinterGUI() {
    delete m_d;
}

void BillAttributePrinterGUI::setPrintData(){
    if( m_d->ui->attributePrintSimpleRadioButton->isChecked() ){
        *m_d->printOption = BillPrinter::AttributePrintSimple;
    } else if( m_d->ui->attributePrintUnionRadioButton->isChecked() ){
        *m_d->printOption = BillPrinter::AttributePrintUnion;
    } else if( m_d->ui->attributePrintIntersectionRadioButton->isChecked() ){
        *m_d->printOption = BillPrinter::AttributePrintIntersection;
    }

    m_d->printFields->clear();
    for( int i=0; i < m_d->priceFieldComboBoxList.size(); ++i){
        m_d->printFields->append( m_d->priceFieldComboBoxList.at(i)->currentIndex() );
    }

    m_d->printAttributes->clear();
    QModelIndexList	selRows = m_d->ui->attributesTableView->selectionModel()->selectedRows();
    for( QModelIndexList::iterator i = selRows.begin(); i != selRows.end(); ++i ){
        Attribute * attr = m_d->billAttributeModel->attribute((*i).row() );
        if( !(m_d->printAttributes->contains(attr)) ){
            m_d->printAttributes->append( attr );
        }
    }

    *(m_d->paperWidth) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).width();
    *(m_d->paperHeight) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).height();

    if( m_d->ui->paperHorizzontalRadioButton->isChecked() ){
        *m_d->paperOrientation = Qt::Horizontal;
    } else {
        *m_d->paperOrientation = Qt::Vertical;
    }

    if( m_d->ui->printShortDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintShortDesc;
    } else if( m_d->ui->printLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintLongDesc;
    } else if( m_d->ui->printShortLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintShortLongDesc;
    } else if( m_d->ui->printShortLongDescOptRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintShortLongDescOpt;
    }

    *(m_d->groupPriceAmount) = m_d->ui->groupPriceAmountCheckBox->isChecked();
}

void BillAttributePrinterGUI::insertPriceFieldComboBox() {
    QComboBox * box = new QComboBox( this );

    // box populate
    for( QList<QString>::iterator i=m_d->priceFieldsNames.begin(); i != m_d->priceFieldsNames.end(); ++i ){
        box->addItem( *i );
    }
    m_d->priceFieldComboBoxList.append( box );
    m_d->ui->priceFieldComboBoxLayout->insertWidget( m_d->priceFieldComboBoxList.size()-1, box );
}

void BillAttributePrinterGUI::removePriceFieldComboBox() {
    if( !m_d->priceFieldComboBoxList.isEmpty() ){
        m_d->ui->priceFieldComboBoxLayout->removeWidget( m_d->priceFieldComboBoxList.last() );
        delete m_d->priceFieldComboBoxList.takeLast();
    }
}
