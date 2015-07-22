#include "accountinglsbillitemtitlegui.h"
#include "ui_accountinglsbillitemtitlegui.h"

#include "accountinglsbill.h"
#include "accountingitemattributemodel.h"
#include "accountinglsbillitem.h"
#include "pricefieldmodel.h"

#include <QLabel>

class AccountingLSBillItemTitleGUIPrivate{
public:
    AccountingLSBillItemTitleGUIPrivate( PriceFieldModel * pfm ):
        ui(new Ui::AccountingLSBillItemTitleGUI),
        bill(NULL),
        item(NULL),
        itemAttributeModel( new AccountingItemAttributeModel( NULL ) ),
        priceFieldModel(pfm){
    }
    ~AccountingLSBillItemTitleGUIPrivate(){
        delete ui;
    }
    Ui::AccountingLSBillItemTitleGUI * ui;
    AccountingLSBill * bill;
    AccountingLSBillItem * item;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
};

AccountingLSBillItemTitleGUI::AccountingLSBillItemTitleGUI(PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingLSBillItemTitleGUIPrivate( pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillItemTitleGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillItemTitleGUI::removeAttribute );
}

AccountingLSBillItemTitleGUI::~AccountingLSBillItemTitleGUI() {
    delete m_d;
}

void AccountingLSBillItemTitleGUI::setBillItem(AccountingLSBillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != NULL ){
            disconnect( m_d->item, &AccountingLSBillItem::nameChanged, m_d->ui->nameLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->item, &AccountingLSBillItem::setName );
            disconnect( m_d->item, &AccountingLSBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::totalAmountAccountedChanged, m_d->ui->totalAmountAccountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillItemNULL );
        }

        m_d->ui->nameLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->totalAmountAccountedLineEdit->clear();
        m_d->ui->percentageAccountedLineEdit->clear();
        m_d->item = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->item != NULL ){
            m_d->ui->nameLineEdit->setText( m_d->item->name() );
            connect( m_d->item, &AccountingLSBillItem::nameChanged, m_d->ui->nameLineEdit, &QLineEdit::setText );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->item, &AccountingLSBillItem::setName );
            connect( m_d->item, &AccountingLSBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            connect( m_d->item, &AccountingLSBillItem::totalAmountAccountedChanged, m_d->ui->totalAmountAccountedLineEdit, &QLineEdit::setText );
            connect( m_d->item, &AccountingLSBillItem::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );
            connect( m_d->item, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillItemNULL );
        }
    }
}

void AccountingLSBillItemTitleGUI::setBillItemNULL() {
    setBillItem( NULL );
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

void AccountingLSBillItemTitleGUI::setBill( AccountingLSBill *b ) {
    if( b != m_d->bill ){
        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( NULL );
            disconnect( m_d->bill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillNULL );
        }

        m_d->bill = b;

        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributeModel() );
            connect( m_d->bill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSBillItemTitleGUI::setBillNULL );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        setBillItemNULL();
    }
}

void AccountingLSBillItemTitleGUI::setBillNULL() {
    setBill(NULL);
}
