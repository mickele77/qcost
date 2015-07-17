#include "accountinglsbillitemtitlegui.h"
#include "ui_accountinglsbillitemtitlegui.h"

#include "bill.h"
#include "billitemattributemodel.h"
#include "billitem.h"
#include "pricefieldmodel.h"

#include <QLabel>

class AccountingLSBillItemTitleGUIPrivate{
public:
    AccountingLSBillItemTitleGUIPrivate( PriceFieldModel * pfm ):
        ui(new Ui::AccountingLSBillItemTitleGUI),
        bill(NULL),
        item(NULL),
        itemAttributeModel( new BillItemAttributeModel(NULL, NULL) ),
        priceFieldModel(pfm){
    };
    ~AccountingLSBillItemTitleGUIPrivate(){
        delete ui;
    }
    Ui::AccountingLSBillItemTitleGUI * ui;
    Bill * bill;
    BillItem * item;
    BillItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingLSBillItemTitleGUI::AccountingLSBillItemTitleGUI(BillItem * item, PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingLSBillItemTitleGUIPrivate( pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillItemTitleGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillItemTitleGUI::removeAttribute );

    setBillItem( item );

    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &AccountingLSBillItemTitleGUI::updateAmountNamesValues );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &AccountingLSBillItemTitleGUI::updateAmountNamesValues );
    updateAmountNamesValues();
    connect( m_d->priceFieldModel, &PriceFieldModel::amountNameChanged, this, &AccountingLSBillItemTitleGUI::updateAmountName );
}

AccountingLSBillItemTitleGUI::~AccountingLSBillItemTitleGUI() {
    delete m_d;
}

void AccountingLSBillItemTitleGUI::setBillItem(BillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != NULL ){
            disconnect( m_d->item, &BillItem::nameChanged, this, &AccountingLSBillItemTitleGUI::updateLineEdit );
            disconnect( m_d->item, &BillItem::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillItemNULL );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::textEdited, this, &AccountingLSBillItemTitleGUI::updateItem );
        }

        m_d->ui->titleLineEdit->clear();
        m_d->item = b;
        m_d->itemAttributeModel->setBillItem( b );

        if( m_d->item != NULL ){
            m_d->ui->titleLineEdit->setText( m_d->item->name() );
            connect( m_d->item, &BillItem::nameChanged, this, &AccountingLSBillItemTitleGUI::updateLineEdit );
            connect( m_d->item, &BillItem::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillItemNULL );
            connect( m_d->ui->titleLineEdit, &QLineEdit::textEdited, this, &AccountingLSBillItemTitleGUI::updateItem );

            connect( m_d->item, static_cast<void(BillItem::*)(int,const QString &)>(&BillItem::amountChanged), this, &AccountingLSBillItemTitleGUI::updateAmountValue );
            updateAmountValues();
        }
    }
}

void AccountingLSBillItemTitleGUI::setBillItemNULL() {
    setBillItem( NULL );
}

void AccountingLSBillItemTitleGUI::updateLineEdit() {
    if( m_d->item ){
        if( m_d->ui->titleLineEdit->text() != m_d->item->name() ){
            m_d->ui->titleLineEdit->setText( m_d->item->name() );
        }
    }
}

void AccountingLSBillItemTitleGUI::updateItem() {
    if( m_d->item ){
        if( m_d->ui->titleLineEdit->text() != m_d->item->name() ){
            m_d->item->setName( m_d->ui->titleLineEdit->text() );
        }
    }
}

void AccountingLSBillItemTitleGUI::addAttribute(){
    if( m_d->itemAttributeModel != NULL ){
        if( m_d->ui->attributeTableView->selectionModel() ){
            int count = 1;
            QModelIndexList selectedRows = m_d->ui->attributeTableView->selectionModel()->selectedRows();
            if( selectedRows.size() > 1 ){
                count = selectedRows.size();
            }
            int row = m_d->itemAttributeModel->rowCount();
            QModelIndex currentIndex = m_d->ui->attributeTableView->selectionModel()->currentIndex();
            if( currentIndex.isValid() ){
                row = currentIndex.row() + 1;
            }
            m_d->itemAttributeModel->insertRows( row, count );
        }
    }
}

void AccountingLSBillItemTitleGUI::removeAttribute(){
    if( m_d->itemAttributeModel != NULL ){
        if( m_d->ui->attributeTableView->selectionModel() ){
            QModelIndexList selectedRows = m_d->ui->attributeTableView->selectionModel()->selectedRows();
            int count = selectedRows.size();
            if( count > 0 ){
                int row = selectedRows.at(0).row();
                for( int i=1; i<selectedRows.size(); ++i){
                    if( selectedRows.at(i).row() < row ){
                        row = selectedRows.at(i).row();
                    }
                }
                m_d->itemAttributeModel->removeRows( row, count );
            }
        }
    }
}

void AccountingLSBillItemTitleGUI::setBill(Bill *b) {
    if( m_d->bill != NULL ){
        m_d->itemAttributeModel->setAttributeModel( NULL );
        disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillNULL );
    }

    m_d->bill = b;

    if( m_d->bill != NULL ){
        m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributeModel() );
        connect( m_d->bill, &Bill::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillNULL );
    }

    // quando si cambia computo corrente la scheda della riga si azzera
    setBillItem( NULL );
}

void AccountingLSBillItemTitleGUI::setBillNULL() {
    setBill(NULL);
}

void AccountingLSBillItemTitleGUI::updateAmountNamesValues(){
    for( QList<QLabel* >::iterator i = m_d->amountDataFieldLabel.begin(); i != m_d->amountDataFieldLabel.end(); ++i ){
        m_d->ui->amountsDataLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountDataFieldLabel.clear();
    for( QList<QLineEdit* >::iterator i = m_d->amountDataFieldLEdit.begin(); i != m_d->amountDataFieldLEdit.end(); ++i ){
        m_d->ui->amountsDataLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountDataFieldLEdit.clear();

    QLabel * label;
    QLineEdit * lEdit;

    for( int i=0; i<m_d->priceFieldModel->fieldCount(); ++i){
        label = new QLabel( m_d->priceFieldModel->amountName( i ) );
        lEdit = new QLineEdit();
        lEdit->setReadOnly( true );
        lEdit->setAlignment( Qt::AlignRight );
        if( m_d->item != NULL ){
            lEdit->setText( m_d->item->amountStr(i) );
        }
        m_d->ui->amountsDataLayout->addWidget( label, i, 0 );
        m_d->ui->amountsDataLayout->addWidget( lEdit, i, 1 );
        m_d->amountDataFieldLabel.append( label );
        m_d->amountDataFieldLEdit.append( lEdit );
    }
}

void AccountingLSBillItemTitleGUI::updateAmountValue(int priceField, const QString & newVal){
    if( priceField > -1 && priceField < m_d->amountDataFieldLEdit.size() ){
        m_d->amountDataFieldLEdit.at(priceField)->setText( newVal );
    }
}

void AccountingLSBillItemTitleGUI::updateAmountValues(){
    for( int i = 0; i < m_d->amountDataFieldLEdit.size(); ++i){
        if( (i < m_d->priceFieldModel->fieldCount()) && ( m_d->item != NULL ) ){
            m_d->amountDataFieldLEdit.at(i)->setText( m_d->item->amountStr(i) );
        } else {
            m_d->amountDataFieldLEdit.at(i)->clear();
        }
    }
}

void AccountingLSBillItemTitleGUI::updateAmountName( int priceField, const QString & newName ){
    if( priceField > -1 && priceField < m_d->amountDataFieldLabel.size() ){
        m_d->amountDataFieldLabel.at(priceField)->setText( newName );
    }
}
