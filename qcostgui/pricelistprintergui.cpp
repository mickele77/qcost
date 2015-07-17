/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2014 Mocciola Michele

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
#include "pricelistprintergui.h"
#include "ui_pricelistprintergui.h"

#include "pricefieldmodel.h"

#include <QComboBox>
#include <QPageSize>

class PriceListPrinterGUIPrivate {
public:
    PriceListPrinterGUIPrivate( PriceListPrinter::PrintPriceItemsOption * prItemsOptions,
                                QList<int> * fields,
                                double *pWidth,
                                double *pHeight,
                                Qt::Orientation *pOrient,
                                int * priceCol,
                                bool *prPrList,
                                bool *prAP,
                                bool * APgrPrAm,
                                PriceFieldModel * pfm ):
        ui(new Ui::PriceListPrinterGUI),
        printItemsOption( prItemsOptions ),
        printFields(fields),
        printPriceDataSet(priceCol),
        printPriceList( prPrList ),
        printPriceAP( prAP ),
        APgroupPriceAmount( APgrPrAm ),
        paperWidth(pWidth),
        paperHeight(pHeight),
        paperOrientation( pOrient ),
        priceFieldsNames( pfm->fieldNames() ){
        pageSizeList << QPageSize( QPageSize::A4 );
    };
    ~PriceListPrinterGUIPrivate(){
        delete ui;
    };

    Ui::PriceListPrinterGUI *ui;
    PriceListPrinter::PrintPriceItemsOption * printItemsOption;
    QList<int> * printFields;
    int *printPriceDataSet;
    bool *printPriceList;
    bool *printPriceAP;
    bool *APgroupPriceAmount;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    QList<QComboBox *> priceFieldComboBoxList;
    QList<QString> priceFieldsNames;
    QList< QPageSize > pageSizeList;
};

PriceListPrinterGUI::PriceListPrinterGUI( PriceListPrinter::PrintPriceItemsOption * prItemsOption,
                                          QList<int> * printFields,
                                          double *pWidth,
                                          double *pHeight,
                                          Qt::Orientation *pOrient,
                                          int * printPriceDataSet,
                                          bool *printPriceList,
                                          bool *printPriceAP,
                                          bool *APgroupPrAm,
                                          int priceDataSetCount,
                                          PriceFieldModel * pfm,
                                          QWidget *parent ) :
    QDialog(parent),
    m_d( new PriceListPrinterGUIPrivate( prItemsOption, printFields, pWidth, pHeight, pOrient, printPriceDataSet, printPriceList, printPriceAP, APgroupPrAm, pfm ) ){

    m_d->ui->setupUi(this);

    if( priceDataSetCount > 0 ){
        m_d->ui->priceDataSetSpinBox->setMaximum( priceDataSetCount );
        m_d->ui->priceDataSetSpinBox->setEnabled( true );
    } else {
        m_d->ui->priceDataSetSpinBox->setMaximum(1);
        m_d->ui->priceDataSetSpinBox->setDisabled( true );
    }
    m_d->ui->priceDataSetSpinBox->setValue( *printPriceDataSet + 1 );

    connect( m_d->ui->insertPriceFieldPushButton, &QPushButton::clicked, this, &PriceListPrinterGUI::insertPriceFieldComboBox );
    connect( m_d->ui->removePriceFieldPushButton, &QPushButton::clicked, this, &PriceListPrinterGUI::removePriceFieldComboBox );

    connect( this, &PriceListPrinterGUI::accepted, this, &PriceListPrinterGUI::setPrintData );

    m_d->ui->printPriceListCheckBox->setChecked( *(m_d->printPriceList) );
    m_d->ui->printAPCheckBox->setChecked( *(m_d->printPriceAP) );
    m_d->ui->ApgroupPrAmCheckBox->setChecked( *(m_d->APgroupPriceAmount) );

    for( int i=0; i < m_d->pageSizeList.size(); ++i ){
        m_d->ui->paperDimensionsComboBox->addItem( m_d->pageSizeList.at(i).name() );
    }

    if( *(m_d->printItemsOption) == PriceListPrinter::PrintShortDesc ){
        m_d->ui->printShortDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == PriceListPrinter::PrintLongDesc ){
        m_d->ui->printLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == PriceListPrinter::PrintShortLongDesc ){
        m_d->ui->printShortLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == PriceListPrinter::PrintShortLongDescOpt ){
        m_d->ui->printShortLongDescOptRadioButton->setChecked( true );
    }

    insertPriceFieldComboBox();

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

PriceListPrinterGUI::~PriceListPrinterGUI() {
    delete m_d;
}

void PriceListPrinterGUI::setPrintData(){
    m_d->printFields->clear();
    for( int i=0; i < m_d->priceFieldComboBoxList.size(); ++i){
        m_d->printFields->append( m_d->priceFieldComboBoxList.at(i)->currentIndex() );
    }
    *(m_d->printPriceDataSet) = m_d->ui->priceDataSetSpinBox->value() - 1;

    *(m_d->paperWidth) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).width();
    *(m_d->paperHeight) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).height();

    if( m_d->ui->paperHorizzontalRadioButton->isChecked() ){
        *m_d->paperOrientation = Qt::Horizontal;
    } else {
        *m_d->paperOrientation = Qt::Vertical;
    }

    if( m_d->ui->printShortDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = PriceListPrinter::PrintShortDesc;
    } else if( m_d->ui->printLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = PriceListPrinter::PrintLongDesc;
    } else if( m_d->ui->printShortLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = PriceListPrinter::PrintShortLongDesc;
    } else if( m_d->ui->printShortLongDescOptRadioButton->isChecked() ){
        *(m_d->printItemsOption) = PriceListPrinter::PrintShortLongDescOpt;
    }

    *(m_d->printPriceList) = m_d->ui->printPriceListCheckBox->isChecked();
    *(m_d->printPriceAP) = m_d->ui->printAPCheckBox->isChecked();
    *(m_d->APgroupPriceAmount) = m_d->ui->ApgroupPrAmCheckBox->isChecked();
}

void PriceListPrinterGUI::insertPriceFieldComboBox() {
    QComboBox * box = new QComboBox( this );

    // box populate
    for( QList<QString>::iterator i=m_d->priceFieldsNames.begin(); i != m_d->priceFieldsNames.end(); ++i ){
        box->addItem( *i );
    }
    m_d->priceFieldComboBoxList.append( box );
    m_d->ui->priceFieldComboBoxLayout->insertWidget( m_d->priceFieldComboBoxList.size()-1, box );
}

void PriceListPrinterGUI::removePriceFieldComboBox() {
    if( !m_d->priceFieldComboBoxList.isEmpty() ){
        m_d->ui->priceFieldComboBoxLayout->removeWidget( m_d->priceFieldComboBoxList.last() );
        delete m_d->priceFieldComboBoxList.takeLast();
    }
}
