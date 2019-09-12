#include "accountingitempaymentgui.h"
#include "ui_accountingitempaymentgui.h"

#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"
#include "accountingitemattributemodel.h"
#include "pricefieldmodel.h"
#include "qcalendardialog.h"

#include <QDate>
#include <QLabel>

class AccountingItemPaymentGUIPrivate{
public:
    AccountingItemPaymentGUIPrivate( PriceFieldModel * pfm ):
        ui(new Ui::AccountingItemPaymentGUI),
        TAMBill(nullptr),
        TAMBillItem(nullptr),
        bill(nullptr),
        billItem(nullptr),
        itemAttributeModel( new AccountingItemAttributeModel(nullptr, nullptr) ),
        priceFieldModel(pfm){
    }
    ~AccountingItemPaymentGUIPrivate(){
        delete ui;
    }
    Ui::AccountingItemPaymentGUI * ui;
    AccountingTAMBill * TAMBill;
    AccountingTAMBillItem * TAMBillItem;
    AccountingBill * bill;
    AccountingBillItem * billItem;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingItemPaymentGUI::AccountingItemPaymentGUI(PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemPaymentGUIPrivate( pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingItemPaymentGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingItemPaymentGUI::removeAttribute );

    m_d->ui->beginDateLineEdit->installEventFilter( this );
    m_d->ui->endDateLineEdit->installEventFilter( this );
}

AccountingItemPaymentGUI::~AccountingItemPaymentGUI() {
    delete m_d;
}

void AccountingItemPaymentGUI::setAccountingItem(AccountingTAMBillItem *b) {
    if( m_d->TAMBillItem != b || m_d->billItem != nullptr ){
        if( m_d->TAMBillItem != nullptr ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingItemNULL );

            m_d->ui->beginDateLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::startDateChanged, this, &AccountingItemPaymentGUI::setDateBegin );
            m_d->ui->endDateLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::endDateChanged, this, &AccountingItemPaymentGUI::setDateEnd );

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
        if( m_d->billItem != nullptr ){
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingItemNULL );

            m_d->ui->beginDateLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::dateBeginChanged, this, &AccountingItemPaymentGUI::setDateBegin );
            m_d->ui->endDateLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::dateEndChanged, this, &AccountingItemPaymentGUI::setDateEnd );

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }

        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();
        m_d->billItem = nullptr;
        m_d->TAMBillItem = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->TAMBillItem != nullptr ){
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingItemNULL );

            m_d->ui->beginDateLineEdit->setText( m_d->TAMBillItem->startDateStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::startDateChanged, this, &AccountingItemPaymentGUI::setDateBegin );
            m_d->ui->endDateLineEdit->setText( m_d->TAMBillItem->endDateStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::endDateChanged, this, &AccountingItemPaymentGUI::setDateEnd );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->TAMBillItem->totalAmountToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->TAMBillItem->amountNotToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->TAMBillItem->totalAmountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemPaymentGUI::setAccountingItem(AccountingBillItem *b) {
    if( m_d->billItem != b || m_d->TAMBillItem != nullptr ){
        if( m_d->TAMBillItem != nullptr ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingItemNULL );

            m_d->ui->beginDateLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::startDateChanged, this, &AccountingItemPaymentGUI::setDateBegin );
            m_d->ui->endDateLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::endDateChanged, this, &AccountingItemPaymentGUI::setDateEnd );

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->clear();
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
        if( m_d->billItem != nullptr ){
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingItemNULL );

            m_d->ui->beginDateLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::dateBeginChanged, this, &AccountingItemPaymentGUI::setDateBegin );
            m_d->ui->endDateLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::dateEndChanged, this, &AccountingItemPaymentGUI::setDateEnd );

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->clear();
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }

        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();
        m_d->billItem = b;
        m_d->TAMBillItem = nullptr;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->billItem != nullptr ){
            connect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingItemNULL );

            m_d->ui->beginDateLineEdit->setText( m_d->billItem->dateBeginStr() );
            connect( m_d->billItem, &AccountingBillItem::dateBeginChanged, this, &AccountingItemPaymentGUI::setDateBegin );
            m_d->ui->endDateLineEdit->setText( m_d->billItem->dateEndStr() );
            connect( m_d->billItem, &AccountingBillItem::dateEndChanged, this, &AccountingItemPaymentGUI::setDateEnd );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->billItem->totalAmountToDiscountStr() );
            connect( m_d->billItem, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->billItem->amountNotToDiscountStr() );
            connect( m_d->billItem, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->billItem->totalAmountStr() );
            connect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemPaymentGUI::setAccountingItemNULL() {
    setAccountingItem( static_cast<AccountingBillItem *> ( nullptr ) );
    setAccountingItem( static_cast<AccountingBillItem *> ( nullptr ) );
}

void AccountingItemPaymentGUI::addAttribute(){
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

void AccountingItemPaymentGUI::removeAttribute(){
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

void AccountingItemPaymentGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( m_d->TAMBill != b ){
        if( m_d->TAMBill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( nullptr );
            disconnect( m_d->TAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingNULL );
        }
        if( m_d->bill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( nullptr );
            disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingNULL );
        }

        m_d->bill = nullptr;
        m_d->TAMBill = b;

        if( m_d->TAMBill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( m_d->TAMBill->attributesModel() );
            connect( m_d->TAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingNULL );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        setAccountingItemNULL();
    }
}

void AccountingItemPaymentGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->bill != b ){
        if( m_d->TAMBill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( nullptr );
            disconnect( m_d->TAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingNULL );
        }
        if( m_d->bill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( nullptr );
            disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingNULL );
        }

        m_d->bill = b;
        m_d->TAMBill = nullptr;

        if( m_d->bill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributesModel() );
            connect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemPaymentGUI::setAccountingNULL );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        setAccountingItemNULL();
    }
}

void AccountingItemPaymentGUI::setAccountingNULL() {
    setAccountingTAMBill(nullptr);
    setAccountingBill(nullptr);
}

void AccountingItemPaymentGUI::setDateBegin( const QString &newVal ) {
    m_d->ui->beginDateLineEdit->setText( newVal );
}

void AccountingItemPaymentGUI::setDateEnd( const QString &newVal ) {
    m_d->ui->endDateLineEdit->setText( newVal );
}

bool AccountingItemPaymentGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick )     {
        if( m_d->TAMBillItem != nullptr ){
            if( object == m_d->ui->beginDateLineEdit ){
                QDate d = m_d->TAMBillItem->startDate();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->TAMBillItem->setStartDate( d );
                }
            } else if( object == m_d->ui->endDateLineEdit ){
                QDate d = m_d->TAMBillItem->endDate();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->TAMBillItem->setEndDate( d );
                }
            }
        }
        if( m_d->billItem != nullptr ){
            if( object == m_d->ui->beginDateLineEdit ){
                QDate d = m_d->billItem->dateBegin();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->billItem->setDateBegin( d );
                }
            } else if( object == m_d->ui->endDateLineEdit ){
                QDate d = m_d->billItem->dateEnd();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->billItem->setDateEnd( d );
                }
            }
        }
    }
    return false;
}
