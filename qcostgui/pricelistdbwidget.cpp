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
#include "pricelistdbwidget.h"
#include "ui_pricelistdbwidget.h"

#include "pricelistdbmodel.h"
#include "mathparser.h"

#include <QTextStream>
#include <QFileDialog>

class PriceListDBWidgetPrivate{
public:
    PriceListDBWidgetPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * impOptions, QString * fileNameInput, MathParser * prs,
                              const QString & connectionName ):
        ui(new Ui::PriceListDBWidget),
        model( new PriceListDBModel( prs, connectionName ) ),
        parser(prs),
        importMode( PriceListDBWidget::ImportSingle ),
        fileName(fileNameInput),
        importOptions(impOptions),
        fileNameWasCreated(false){
        if( fileName == NULL ){
            fileName = new QString();
            fileNameWasCreated = true;
        }
    }
    ~PriceListDBWidgetPrivate(){
        delete ui;
        delete model;
        if( fileNameWasCreated ){
            delete fileName;
        }
    }
    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser == NULL ){
            return QString::number( i, f, prec );
        } else {
            return parser->toString( i, f, prec );
        }
    }
    Ui::PriceListDBWidget *ui;
    PriceListDBModel * model;
    MathParser * parser;
    QPersistentModelIndex currentIndex;
    PriceListDBWidget::ImportMode importMode;
    QString * fileName;
    QMap<PriceListDBWidget::ImportOptions, bool> * importOptions;
    bool fileNameWasCreated;
};

PriceListDBWidget::PriceListDBWidget( QMap<ImportOptions, bool> * impOptions, QString * fileNameInput, MathParser * parser,
                                      const QString & connectionName, QWidget *parent) :
    QWidget(parent),
    m_d( new PriceListDBWidgetPrivate( impOptions, fileNameInput, parser, connectionName ) ){
    m_d->ui->setupUi(this);

    m_d->ui->importOptionsGroupBox->setVisible( false );
    m_d->ui->filtersGroupBox->setVisible( false );

    if( m_d->importOptions != NULL ){
        if( m_d->importOptions->contains( ImportCode ) ){
            m_d->ui->importCodeCheckBox->setChecked( m_d->importOptions->value(ImportCode) );
        } else {
            m_d->importOptions->insert( ImportCode, m_d->ui->importCodeCheckBox->isChecked() );
        }
        connect( m_d->ui->importCodeCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( m_d->importOptions->contains( InheritCode ) ){
            m_d->ui->inheritCodeCheckBox->setChecked( m_d->importOptions->value(InheritCode) );
        } else {
            m_d->importOptions->insert( InheritCode, m_d->ui->inheritCodeCheckBox->isChecked() );
        }
        connect( m_d->ui->inheritCodeCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( impOptions->contains( ImportShortDesc ) ){
            m_d->ui->importShortDescCheckBox->setChecked( m_d->importOptions->value(ImportShortDesc) );
        } else {
            m_d->importOptions->insert( ImportShortDesc, m_d->ui->importShortDescCheckBox->isChecked() );
        }
        connect( m_d->ui->importShortDescCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( impOptions->contains( InheritShortDesc ) ){
            m_d->ui->inheritShortDescCheckBox->setChecked( m_d->importOptions->value(InheritShortDesc) );
        } else {
            m_d->importOptions->insert( InheritShortDesc, m_d->ui->inheritShortDescCheckBox->isChecked() );
        }
        connect( m_d->ui->inheritShortDescCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( impOptions->contains( ImportLongDesc ) ){
            m_d->ui->importLongDescCheckBox->setChecked( m_d->importOptions->value(ImportLongDesc) );
        } else {
            m_d->importOptions->insert( ImportLongDesc, m_d->ui->importLongDescCheckBox->isChecked() );
        }
        connect( m_d->ui->importLongDescCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( impOptions->contains( InheritLongDesc ) ){
            m_d->ui->inheritLongDescCheckBox->setChecked( m_d->importOptions->value(InheritLongDesc) );
        } else {
            m_d->importOptions->insert( InheritLongDesc, m_d->ui->inheritLongDescCheckBox->isChecked() );
        }
        connect( m_d->ui->inheritLongDescCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( impOptions->contains( ImportOverheads ) ){
            m_d->ui->importOverheadsCheckBox->setChecked( m_d->importOptions->value(ImportOverheads) );
        } else {
            m_d->importOptions->insert( ImportOverheads, m_d->ui->importOverheadsCheckBox->isChecked() );
        }
        connect( m_d->ui->importOverheadsCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
        if( impOptions->contains( ImportProfits ) ){
            m_d->ui->importProfitsCheckBox->setChecked( m_d->importOptions->value(ImportProfits) );
        } else {
            m_d->importOptions->insert( ImportProfits, m_d->ui->importProfitsCheckBox->isChecked() );
        }
        connect( m_d->ui->importProfitsCheckBox, &QCheckBox::toggled, this, &PriceListDBWidget::syncImportOptions );
    }

    m_d->ui->priceListView->setModel( m_d->model );

    if( m_d->importMode == ImportSingle ){
        m_d->ui->priceListView->setSelectionMode( QAbstractItemView::SingleSelection );
    } else if( m_d->importMode == ImportMulti ){
        m_d->ui->priceListView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    }
    connect( m_d->ui->priceListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &PriceListDBWidget::setCurrentPrice );

    m_d->ui->dbFileNameLineEdit->setText( *( m_d->fileName ));
    if( !m_d->fileName->isEmpty() ){
        m_d->model->setCurrentFile( *(m_d->fileName) );
    }
    connect( m_d->ui->editDBFileNameToolButton, &QPushButton::clicked, this, &PriceListDBWidget::editDBFileName );

    connect( m_d->ui->applyFiltersPushButton, &QCheckBox::clicked, this, &PriceListDBWidget::applyFilter );
}

PriceListDBWidget::~PriceListDBWidget() {
    delete m_d;
}

void PriceListDBWidget::setMode(PriceListDBWidget::ImportMode m) {
    if( m != m_d->importMode ){
        m_d->importMode = m;
        if( m == ImportSingle ){
            m_d->ui->priceListView->setSelectionMode( QAbstractItemView::SingleSelection );
        } else if( m == ImportMulti ){
            m_d->ui->priceListView->setSelectionMode( QAbstractItemView::ExtendedSelection );
        }
    }
}

void PriceListDBWidget::setCurrentPrice(const QModelIndex &current ) {
    if( current.isValid() ){
        m_d->currentIndex = QPersistentModelIndex( current );
        m_d->ui->codeLineEdit->setText( m_d->model->code(m_d->currentIndex) );
        m_d->ui->shortDescLineEdit->setText( m_d->model->shortDescription(m_d->currentIndex) );
        m_d->ui->longDescTextEdit->setPlainText( m_d->model->longDescription(m_d->currentIndex) );
        m_d->ui->unitMeasureLineEdit->setText( m_d->model->unitMeasure( m_d->currentIndex ) );
        m_d->ui->priceTotalLineEdit->setText( m_d->toString( m_d->model->priceTotal(m_d->currentIndex), 'f', 2) );
        m_d->ui->priceHumanLineEdit->setText( m_d->toString( m_d->model->priceHuman(m_d->currentIndex), 'f', 2) );
        m_d->ui->priceEquipmentLineEdit->setText( m_d->toString( m_d->model->priceEquipment(m_d->currentIndex), 'f', 2) );
        m_d->ui->priceMaterialLineEdit->setText( m_d->toString( m_d->model->priceMaterial(m_d->currentIndex), 'f', 2) );
        m_d->ui->overheadsLineEdit->setText( m_d->model->overheadsStr(m_d->currentIndex) );
        m_d->ui->profitsLineEdit->setText( m_d->model->profitsStr(m_d->currentIndex) );
    }
}

void PriceListDBWidget::editDBFileName(){
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    trUtf8("Apri E.P. Archivio"), ".",
                                                    trUtf8("E.P.Archivio QCost(*.qdb);;"));
    if( QFileInfo( fileName ).exists() ){
        *(m_d->fileName) = fileName;
        m_d->ui->dbFileNameLineEdit->setText( fileName );
        m_d->model->setCurrentFile( fileName );
    }
}

void PriceListDBWidget::importSinglePriceItemDB( QList< QPair<QString, QVariant> > * retData ){
    if( m_d->ui->priceListView->selectionMode() ){
        QModelIndex currIndex;
        currIndex = m_d->ui->priceListView->selectionModel()->currentIndex();
        if( currIndex.isValid() ){
            if( m_d->ui->importCodeCheckBox->isChecked() ){
                *retData << qMakePair( QString("code"), QVariant( m_d->model->code(currIndex, m_d->ui->inheritCodeCheckBox->isChecked()) ) );
            }
            if( m_d->ui->importShortDescCheckBox->isChecked() ){
                *retData << qMakePair( QString("shortDesc"), QVariant( m_d->model->shortDescription(currIndex, m_d->ui->inheritShortDescCheckBox->isChecked()) ) );
            }
            if( m_d->ui->importLongDescCheckBox->isChecked() ){
                *retData << qMakePair( QString("longDesc"), QVariant( m_d->model->longDescription(currIndex, m_d->ui->inheritLongDescCheckBox->isChecked()) ) );
            }
            if( m_d->ui->importUnitMeasureCheckBox->isChecked() ){
                *retData << qMakePair( QString("unitMeasure"), QVariant( m_d->model->unitMeasure(currIndex) ) );
            }
            if( m_d->ui->importPriceTotalCheckBox->isChecked() ){
                *retData << qMakePair( QString("priceTotal"), QVariant( m_d->model->priceTotal(currIndex) ) );
            }
            if( m_d->ui->importPriceHumanCheckBox->isChecked() ){
                *retData << qMakePair( QString("priceHuman"), QVariant( m_d->model->priceHuman(currIndex) ) );
            }
            if( m_d->ui->importPriceEquipmentCheckBox->isChecked() ){
                *retData << qMakePair( QString("priceEquipment"), QVariant( m_d->model->priceEquipment(currIndex) ) );
            }
            if( m_d->ui->importPriceMaterialCheckBox->isChecked() ){
                *retData << qMakePair( QString("priceMaterial"), QVariant( m_d->model->priceMaterial(currIndex) ) );
            }
            if( m_d->ui->importOverheadsCheckBox->isChecked() ){
                *retData << qMakePair( QString("overheads"), QVariant( m_d->model->overheads(currIndex) ) );
            }
            if( m_d->ui->importProfitsCheckBox->isChecked() ){
                *retData << qMakePair( QString("profits"), QVariant( m_d->model->profits(currIndex) ) );
            }
        }
    }
}

void PriceListDBWidget::importMultiPriceItemDB(QList<QList<QPair<QString, QVariant> > > *itemDataList, QList<int> *hierarchy) {
    if( m_d->ui->priceListView->selectionMode() ){
        QModelIndexList indexList = m_d->ui->priceListView->selectionModel()->selectedRows();

        for( int i=0; i < indexList.size(); ++i ){
            const QModelIndex currIndex = indexList.at(i);

            if( currIndex.isValid() ){
                if( currIndex.parent().isValid() ){
                    hierarchy->append( indexList.indexOf(currIndex.parent()) );
                } else {
                    hierarchy->append( -1 );
                }

                QList< QPair<QString, QVariant> > retData;
                if( m_d->ui->importCodeCheckBox->isChecked() ){
                    retData << qMakePair( QString("code"), QVariant( m_d->model->code(currIndex, m_d->ui->inheritCodeCheckBox->isChecked()) ) );
                }
                if( m_d->ui->importShortDescCheckBox->isChecked() ){
                    retData << qMakePair( QString("shortDesc"), QVariant( m_d->model->shortDescription(currIndex, m_d->ui->inheritShortDescCheckBox->isChecked()) ) );
                }
                if( m_d->ui->importLongDescCheckBox->isChecked() ){
                    retData << qMakePair( QString("longDesc"), QVariant( m_d->model->longDescription(currIndex, m_d->ui->inheritLongDescCheckBox->isChecked()) ) );
                }
                if( m_d->ui->importUnitMeasureCheckBox->isChecked() ){
                    retData << qMakePair( QString("unitMeasure"), QVariant( m_d->model->unitMeasure(currIndex) ) );
                }
                if( m_d->ui->importPriceTotalCheckBox->isChecked() ){
                    retData << qMakePair( QString("priceTotal"), QVariant( m_d->model->priceTotal(currIndex) ) );
                }
                if( m_d->ui->importPriceHumanCheckBox->isChecked() ){
                    retData << qMakePair( QString("priceHuman"), QVariant( m_d->model->priceHuman(currIndex) ) );
                }
                if( m_d->ui->importPriceEquipmentCheckBox->isChecked() ){
                    retData << qMakePair( QString("priceEquipment"), QVariant( m_d->model->priceEquipment(currIndex) ) );
                }
                if( m_d->ui->importPriceMaterialCheckBox->isChecked() ){
                    retData << qMakePair( QString("priceMaterial"), QVariant( m_d->model->priceMaterial(currIndex) ) );
                }
                if( m_d->ui->importOverheadsCheckBox->isChecked() ){
                    retData << qMakePair( QString("overheads"), QVariant( m_d->model->overheads(currIndex) ) );
                }
                if( m_d->ui->importProfitsCheckBox->isChecked() ){
                    retData << qMakePair( QString("profits"), QVariant( m_d->model->profits(currIndex) ) );
                }
                (*itemDataList) << retData;
            }
        }
    }
}

void PriceListDBWidget::hideImportOptions() {
    m_d->ui->importOptionsVisibleCheckBox->hide();
}

void PriceListDBWidget::applyFilter(){
    if( m_d->ui->filtersGroupBox->isChecked() ){
        QString filter;
        if( !m_d->ui->codeFilterLineEdit->text().isEmpty() ){
            QString v = m_d->ui->codeFilterLineEdit->text();
            v.replace("*", "%");
            v.replace("'", "''");
            filter.append( QString("code LIKE '%1'").arg( v ));
        }
        if( !m_d->ui->shortDescFilterLineEdit->text().isEmpty() ){
            if( !filter.isEmpty() ){
                filter.append( " AND " );
            }
            QString v = m_d->ui->shortDescFilterLineEdit->text();
            v.replace("*", "%");
            v.replace("'", "''");
            filter.append( QString("shortDesc LIKE '%1'").arg( v ));
        }
        if( !m_d->ui->longDescFilterLineEdit->text().isEmpty() ){
            if( !filter.isEmpty() ){
                filter.append( " AND " );
            }
            QString v = m_d->ui->longDescFilterLineEdit->text();
            v.replace("*", "%");
            v.replace("'", "''");
            filter.append( QString("longDesc LIKE '%1'").arg( v ));
        }
        m_d->model->applyFilter( filter );
    } else {
        m_d->model->applyFilter( "" );
    }
}

void PriceListDBWidget::syncImportOptions( bool isChecked ) {
    if( m_d->importOptions != NULL ){
        QCheckBox * checkBoxSender = dynamic_cast<QCheckBox * >(sender());
        if( checkBoxSender == m_d->ui->importCodeCheckBox ){
            (*(m_d->importOptions))[ImportCode] = isChecked;
        } else if( checkBoxSender == m_d->ui->inheritCodeCheckBox ){
            (*(m_d->importOptions))[InheritCode] = isChecked;
        } else if( checkBoxSender == m_d->ui->importShortDescCheckBox ){
            (*(m_d->importOptions))[ImportShortDesc] = isChecked;
        } else if( checkBoxSender == m_d->ui->inheritShortDescCheckBox ){
            (*(m_d->importOptions))[InheritShortDesc] = isChecked;
        } else if( checkBoxSender == m_d->ui->importLongDescCheckBox ){
            (*(m_d->importOptions))[ImportLongDesc] = isChecked;
        } else if( checkBoxSender == m_d->ui->inheritLongDescCheckBox ){
            (*(m_d->importOptions))[InheritLongDesc] = isChecked;
        } else if( checkBoxSender == m_d->ui->importOverheadsCheckBox ){
            (*(m_d->importOptions))[ImportOverheads] = isChecked;
        } else if( checkBoxSender == m_d->ui->importProfitsCheckBox ){
            (*(m_d->importOptions))[ImportProfits] = isChecked;
        }
    }
}
