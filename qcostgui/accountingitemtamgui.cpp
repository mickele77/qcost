#include "accountingitemtamgui.h"
#include "ui_accountingitemtamgui.h"

#include "accountingtambill.h"
#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingitemattributemodel.h"
#include "pricefieldmodel.h"
#include "qcalendardialog.h"

#include <QShowEvent>
#include <QDate>
#include <QLabel>

class AccountingItemTAMGUIPrivate{
public:
    AccountingItemTAMGUIPrivate( AccountingTAMBill * tb, PriceFieldModel * pfm ):
        ui(new Ui::AccountingItemTAMGUI),
        tamBill(tb),
        bill(nullptr),
        item(nullptr),
        itemAttributeModel( new AccountingItemAttributeModel(nullptr, nullptr) ),
        priceFieldModel(pfm){
    }
    ~AccountingItemTAMGUIPrivate(){
        delete ui;
    }
    Ui::AccountingItemTAMGUI * ui;
    AccountingTAMBill * tamBill;
    AccountingBill * bill;
    AccountingBillItem * item;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingItemTAMGUI::AccountingItemTAMGUI(AccountingTAMBill * tamBill, PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemTAMGUIPrivate( tamBill, pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingItemTAMGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingItemTAMGUI::removeAttribute );

    connect( m_d->ui->tamBillComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingItemTAMGUI::setTAMBill );

    m_d->ui->dateLineEdit->installEventFilter( this );
    m_d->ui->beginDateLineEdit->installEventFilter( this );
    m_d->ui->endDateLineEdit->installEventFilter( this );
}

AccountingItemTAMGUI::~AccountingItemTAMGUI() {
    delete m_d;
}

void AccountingItemTAMGUI::setAccountingItem(AccountingBillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != nullptr ){
            disconnect( m_d->item, &AccountingBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::dateBeginChanged, m_d->ui->beginDateLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::dateEndChanged, m_d->ui->endDateLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::dateBeginChanged, this, &AccountingItemTAMGUI::updateTAMBillComboBox );
            disconnect( m_d->item, &AccountingBillItem::dateEndChanged, this, &AccountingItemTAMGUI::updateTAMBillComboBox );

            disconnect( m_d->item, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingItemnullptr );

            disconnect( m_d->item, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();

        m_d->ui->dateLineEdit->clear();
        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();

        m_d->item = b;
        m_d->itemAttributeModel->setItem( b );

        updateTAMBillComboBox();

        if( m_d->item != nullptr ){
            m_d->ui->dateLineEdit->setText( m_d->item->dateStr() );
            m_d->ui->beginDateLineEdit->setText( m_d->item->dateBeginStr() );
            m_d->ui->endDateLineEdit->setText( m_d->item->dateEndStr() );
            connect( m_d->item, &AccountingBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            connect( m_d->item, &AccountingBillItem::dateBeginChanged, m_d->ui->beginDateLineEdit, &QLineEdit::setText );
            connect( m_d->item, &AccountingBillItem::dateEndChanged, m_d->ui->endDateLineEdit, &QLineEdit::setText );
            connect( m_d->item, &AccountingBillItem::dateBeginChanged, this, &AccountingItemTAMGUI::updateTAMBillComboBox );
            connect( m_d->item, &AccountingBillItem::dateEndChanged, this, &AccountingItemTAMGUI::updateTAMBillComboBox );

            connect( m_d->item, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingItemnullptr );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->item->totalAmountToDiscountStr() );
            connect( m_d->item, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->item->amountNotToDiscountStr() );
            connect( m_d->item, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->item->totalAmountStr() );
            connect( m_d->item, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemTAMGUI::setAccountingItemnullptr() {
    setAccountingItem( nullptr );
}

void AccountingItemTAMGUI::addAttribute(){
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

void AccountingItemTAMGUI::removeAttribute(){
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

void AccountingItemTAMGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->bill != nullptr ){
        m_d->itemAttributeModel->setAttributeModel( nullptr );
        disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingnullptr );
    }

    m_d->bill = b;

    if( m_d->bill != nullptr ){
        m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributesModel() );
        connect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingnullptr );
    }

    // quando si cambia la scheda di contabilita' corrente la scheda della riga si azzera
    setAccountingItemnullptr();
}

void AccountingItemTAMGUI::setAccountingnullptr() {
    setAccountingBill(nullptr);
}

void AccountingItemTAMGUI::updateTAMBillComboBox() {
    disconnect( m_d->ui->tamBillComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingItemTAMGUI::setTAMBill );

    m_d->ui->tamBillComboBox->clear();
    int currIndex = 0;
    QVariant v = QVariant::fromValue((void *) nullptr);
    m_d->ui->tamBillComboBox->insertItem( 0, "", v);
    QList<AccountingTAMBillItem *> bills = m_d->tamBill->bills();
    for( int i=0; i < bills.size(); ++i ){
        AccountingTAMBillItem * b = bills.at(i);
        v = QVariant::fromValue((void *) b );
        m_d->ui->tamBillComboBox->insertItem( (i+1), b->title(), v );
        if( m_d->item != nullptr ){
            if( m_d->item->tamBillItem() == b ){
                currIndex = i+1;
            }
        }
    }
    m_d->ui->tamBillComboBox->setCurrentIndex( currIndex );

    connect( m_d->ui->tamBillComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingItemTAMGUI::setTAMBill );
}

void AccountingItemTAMGUI::setTAMBill() {
    if( m_d->item != nullptr ){
        QVariant v = m_d->ui->tamBillComboBox->currentData();
        AccountingTAMBillItem * newTAMBillItem = nullptr;
        if( v.isValid() ){
            newTAMBillItem = (AccountingTAMBillItem *) v.value<void *>();
        }
        m_d->item->setTAMBillItem( newTAMBillItem );
    }
}

bool AccountingItemTAMGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick )     {
        if( m_d->item != nullptr ){
            if( object == m_d->ui->dateLineEdit ){
                QDate d = m_d->item->date();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->item->setDate( d );
                }
            } else if( object == m_d->ui->beginDateLineEdit ){
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

void AccountingItemTAMGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        updateTAMBillComboBox();
    }
    QWidget::showEvent( event );
}
