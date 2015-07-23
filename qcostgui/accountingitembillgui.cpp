#include "accountingitembillgui.h"
#include "ui_accountingitembillgui.h"

#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"
#include "accountingitemattributemodel.h"
#include "pricefieldmodel.h"
#include "qcalendardialog.h"

#include <QDate>
#include <QLabel>

class AccountingItemBillGUIPrivate{
public:
    AccountingItemBillGUIPrivate( PriceFieldModel * pfm ):
        ui(new Ui::AccountingItemBillGUI),
        TAMBill(NULL),
        TAMBillItem(NULL),
        bill(NULL),
        billItem(NULL),
        itemAttributeModel( new AccountingItemAttributeModel(NULL, NULL) ),
        priceFieldModel(pfm){
    }
    ~AccountingItemBillGUIPrivate(){
        delete ui;
    }
    Ui::AccountingItemBillGUI * ui;
    AccountingTAMBill * TAMBill;
    AccountingTAMBillItem * TAMBillItem;
    AccountingBill * bill;
    AccountingBillItem * billItem;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingItemBillGUI::AccountingItemBillGUI(PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemBillGUIPrivate( pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingItemBillGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingItemBillGUI::removeAttribute );

    m_d->ui->beginDateLineEdit->installEventFilter( this );
    m_d->ui->endDateLineEdit->installEventFilter( this );
}

AccountingItemBillGUI::~AccountingItemBillGUI() {
    delete m_d;
}

void AccountingItemBillGUI::setAccountingItem(AccountingTAMBillItem *b) {
    if( m_d->TAMBillItem != b || m_d->billItem != NULL ){
        if( m_d->TAMBillItem != NULL ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::dateBeginChanged, this, &AccountingItemBillGUI::setDateBegin );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::dateEndChanged, this, &AccountingItemBillGUI::setDateEnd );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingItemNULL );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
        if( m_d->billItem != NULL ){
            disconnect( m_d->billItem, &AccountingBillItem::dateBeginChanged, this, &AccountingItemBillGUI::setDateBegin );
            disconnect( m_d->billItem, &AccountingBillItem::dateEndChanged, this, &AccountingItemBillGUI::setDateEnd );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingItemNULL );

            disconnect( m_d->billItem, &AccountingBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }

        m_d->ui->totalAmountToBeDiscountedLineEdit->clear();
        m_d->ui->amountNotToBeDiscountedLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();
        m_d->billItem = NULL;
        m_d->TAMBillItem = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->TAMBillItem != NULL ){
            m_d->ui->beginDateLineEdit->setText( m_d->TAMBillItem->dateBeginStr() );
            m_d->ui->endDateLineEdit->setText( m_d->TAMBillItem->dateEndStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::dateBeginChanged, this, &AccountingItemBillGUI::setDateBegin );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::dateEndChanged, this, &AccountingItemBillGUI::setDateEnd );

            connect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingItemNULL );

            m_d->ui->totalAmountToBeDiscountedLineEdit->setText( m_d->TAMBillItem->totalAmountToBeDiscountedStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToBeDiscountedLineEdit->setText( m_d->TAMBillItem->amountNotToBeDiscountedStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->TAMBillItem->totalAmountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemBillGUI::setAccountingItem(AccountingBillItem *b) {
    if( m_d->billItem != b || m_d->TAMBillItem != NULL ){
        if( m_d->TAMBillItem != NULL ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::dateBeginChanged, this, &AccountingItemBillGUI::setDateBegin );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::dateEndChanged, this, &AccountingItemBillGUI::setDateEnd );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingItemNULL );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
        if( m_d->billItem != NULL ){
            disconnect( m_d->billItem, &AccountingBillItem::dateBeginChanged, this, &AccountingItemBillGUI::setDateBegin );
            disconnect( m_d->billItem, &AccountingBillItem::dateEndChanged, this, &AccountingItemBillGUI::setDateEnd );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingItemNULL );

            disconnect( m_d->billItem, &AccountingBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }

        m_d->ui->totalAmountToBeDiscountedLineEdit->clear();
        m_d->ui->amountNotToBeDiscountedLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();
        m_d->billItem = b;
        m_d->TAMBillItem = NULL;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->billItem != NULL ){
            m_d->ui->beginDateLineEdit->setText( m_d->billItem->dateBeginStr() );
            m_d->ui->endDateLineEdit->setText( m_d->billItem->dateEndStr() );
            connect( m_d->billItem, &AccountingBillItem::dateBeginChanged, this, &AccountingItemBillGUI::setDateBegin );
            connect( m_d->billItem, &AccountingBillItem::dateEndChanged, this, &AccountingItemBillGUI::setDateEnd );

            connect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingItemNULL );

            m_d->ui->totalAmountToBeDiscountedLineEdit->setText( m_d->billItem->totalAmountToBeDiscountedStr() );
            connect( m_d->billItem, &AccountingBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToBeDiscountedLineEdit->setText( m_d->billItem->amountNotToBeDiscountedStr() );
            connect( m_d->billItem, &AccountingBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->billItem->totalAmountStr() );
            connect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemBillGUI::setAccountingItemNULL() {
    setAccountingItem( (AccountingBillItem *) (NULL) );
    setAccountingItem( (AccountingTAMBillItem *) (NULL) );
}

void AccountingItemBillGUI::addAttribute(){
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

void AccountingItemBillGUI::removeAttribute(){
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

void AccountingItemBillGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( m_d->TAMBill != b ){
        if( m_d->TAMBill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( NULL );
            disconnect( m_d->TAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingNULL );
        }
        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( NULL );
            disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingNULL );
        }

        m_d->bill = NULL;
        m_d->TAMBill = b;

        if( m_d->TAMBill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( m_d->TAMBill->attributeModel() );
            connect( m_d->TAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingNULL );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        setAccountingItemNULL();
    }
}

void AccountingItemBillGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->bill != b ){
        if( m_d->TAMBill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( NULL );
            disconnect( m_d->TAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingNULL );
        }
        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( NULL );
            disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingNULL );
        }

        m_d->bill = b;
        m_d->TAMBill = NULL;

        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributeModel() );
            connect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemBillGUI::setAccountingNULL );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        setAccountingItemNULL();
    }
}

void AccountingItemBillGUI::setAccountingNULL() {
    setAccountingTAMBill(NULL);
    setAccountingBill(NULL);
}

void AccountingItemBillGUI::setDateBegin( const QString &newVal ) {
    m_d->ui->beginDateLineEdit->setText( newVal );
}

void AccountingItemBillGUI::setDateEnd( const QString &newVal ) {
    m_d->ui->endDateLineEdit->setText( newVal );
}

bool AccountingItemBillGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick )     {
        if( m_d->TAMBillItem != NULL ){
            if( object == m_d->ui->beginDateLineEdit ){
                QDate d = m_d->TAMBillItem->dateBegin();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->TAMBillItem->setDateBegin( d );
                }
            } else if( object == m_d->ui->endDateLineEdit ){
                QDate d = m_d->TAMBillItem->dateEnd();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->TAMBillItem->setDateEnd( d );
                }
            }
        }
        if( m_d->billItem != NULL ){
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
