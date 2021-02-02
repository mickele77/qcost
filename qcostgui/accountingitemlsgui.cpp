#include "accountingitemlsgui.h"
#include "ui_accountingitemlsgui.h"

#include "accountinglsbills.h"
#include "accountinglsbill.h"
#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingitemattributemodel.h"
#include "pricefieldmodel.h"
#include "qcalendardialog.h"

#include <QShowEvent>
#include <QDate>
#include <QLabel>

class AccountingItemLSGUIPrivate{
public:
    AccountingItemLSGUIPrivate(  AccountingLSBills * lsb, PriceFieldModel * pfm ):
        ui(new Ui::AccountingItemLSGUI),
        lsBills( lsb ),
        bill(nullptr),
        item(nullptr),
        itemAttributeModel( new AccountingItemAttributeModel(nullptr, nullptr) ),
        priceFieldModel(pfm){
    }
    ~AccountingItemLSGUIPrivate(){
        delete ui;
    }
    Ui::AccountingItemLSGUI * ui;
    AccountingLSBills * lsBills;
    AccountingBill * bill;
    AccountingBillItem * item;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingItemLSGUI::AccountingItemLSGUI( AccountingLSBills * lsBills, PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemLSGUIPrivate( lsBills, pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingItemLSGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingItemLSGUI::removeAttribute );

    connect( m_d->ui->lumpSumsComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingItemLSGUI::setLSBill );

    m_d->ui->beginDateLineEdit->installEventFilter( this );
    m_d->ui->endDateLineEdit->installEventFilter( this );
}

AccountingItemLSGUI::~AccountingItemLSGUI() {
    delete m_d;
}

void AccountingItemLSGUI::setItem(AccountingBillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != nullptr ){
            disconnect( m_d->item, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemLSGUI::setAccountingItemnullptr );

            disconnect( m_d->item, &AccountingBillItem::dateBeginChanged, m_d->ui->beginDateLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::dateEndChanged, m_d->ui->endDateLineEdit, &QLineEdit::setText );

            disconnect( m_d->item, &AccountingBillItem::quantityChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->item, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->item, &AccountingBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::setText );
        }
        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();

        m_d->ui->percentageAccountedLineEdit->clear();
        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();

        m_d->ui->PPUTotalToDiscountLineEdit->clear();
        m_d->ui->PPUNotToDiscountLineEdit->clear();

        m_d->item = b;
        m_d->itemAttributeModel->setItem( b );

        updateLumpSumsComboBox();

        if( m_d->item != nullptr ){
            connect( m_d->item, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemLSGUI::setAccountingItemnullptr );

            m_d->ui->beginDateLineEdit->setText( m_d->item->dateBeginStr() );
            connect( m_d->item, &AccountingBillItem::dateBeginChanged, m_d->ui->beginDateLineEdit, &QLineEdit::setText );
            m_d->ui->endDateLineEdit->setText( m_d->item->dateEndStr() );
            connect( m_d->item, &AccountingBillItem::dateEndChanged, m_d->ui->endDateLineEdit, &QLineEdit::setText );

            m_d->ui->percentageAccountedLineEdit->setText( m_d->item->quantityStr() );
            connect( m_d->item, &AccountingBillItem::quantityChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->item->totalAmountToDiscountStr() );
            connect( m_d->item, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->item->amountNotToDiscountStr() );
            connect( m_d->item, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->item->totalAmountStr() );
            connect( m_d->item, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            m_d->ui->PPUTotalToDiscountLineEdit->setText( m_d->item->PPUTotalToDiscountStr() );
            connect( m_d->item, &AccountingBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->PPUNotToDiscountLineEdit->setText( m_d->item->PPUNotToDiscountStr() );
            connect( m_d->item, &AccountingBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemLSGUI::setAccountingItemnullptr() {
    setItem( nullptr );
}

void AccountingItemLSGUI::addAttribute(){
    if( m_d->itemAttributeModel != nullptr ){
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

void AccountingItemLSGUI::removeAttribute(){
    if( m_d->itemAttributeModel != nullptr ){
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

void AccountingItemLSGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->bill != nullptr ){
        m_d->itemAttributeModel->setAttributeModel( nullptr );
        disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemLSGUI::setAccountingnullptr );
    }

    m_d->bill = b;

    if( m_d->bill != nullptr ){
        m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributesModel() );
        connect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemLSGUI::setAccountingnullptr );
    }

    // quando si cambia computo corrente la scheda della riga si azzera
    setItem( nullptr );
}

void AccountingItemLSGUI::setAccountingnullptr() {
    setAccountingBill(nullptr);
}

void AccountingItemLSGUI::updateLumpSumsComboBox() {
    disconnect( m_d->ui->lumpSumsComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingItemLSGUI::setLSBill );

    m_d->ui->lumpSumsComboBox->clear();
    int currIndex = 0;
    QVariant v = qVariantFromValue((void *) nullptr);
    m_d->ui->lumpSumsComboBox->insertItem( 0, "", v);
    for( int i=0; i < m_d->lsBills->billCount(); ++i ){
        AccountingLSBill * b = m_d->lsBills->bill(i);
        v = qVariantFromValue((void *) b );
        m_d->ui->lumpSumsComboBox->insertItem( (i+1), b->name(), v );
        if( m_d->item != nullptr ){
            if( m_d->item->lsBill() == b ){
                currIndex = i+1;
            }
        }
    }
    m_d->ui->lumpSumsComboBox->setCurrentIndex( currIndex );

    connect( m_d->ui->lumpSumsComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingItemLSGUI::setLSBill );
}

void AccountingItemLSGUI::setLSBill() {
    if( m_d->item != nullptr ){
        QVariant v = m_d->ui->lumpSumsComboBox->currentData();
        AccountingLSBill * newLSBill = nullptr;
        if( v.isValid() ){
            newLSBill = (AccountingLSBill *) v.value<void *>();
        }
        m_d->item->setLSBill( newLSBill );
    }
}

bool AccountingItemLSGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick )     {
        if( m_d->item != nullptr ){
            if( object == m_d->ui->beginDateLineEdit ){
                QDate d = m_d->item->dateBegin();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->item->setDateBegin( d );
                }
            } else if( object == m_d->ui->endDateLineEdit ){
                QDate d = m_d->item->dateEnd();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->item->setDateEnd( d );
                }
            }
        }
    }
    return false;
}

void AccountingItemLSGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        updateLumpSumsComboBox();
    }
    QWidget::showEvent( event );
}
