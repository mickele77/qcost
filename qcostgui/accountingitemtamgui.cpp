#include "accountingitemtamgui.h"
#include "ui_accountingitemtamgui.h"

#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingitemattributemodel.h"
#include "pricefieldmodel.h"
#include "qcalendardialog.h"

#include <QDate>
#include <QLabel>

class AccountingItemTAMGUIPrivate{
public:
    AccountingItemTAMGUIPrivate( PriceFieldModel * pfm ):
        ui(new Ui::AccountingItemTAMGUI),
        bill(NULL),
        item(NULL),
        itemAttributeModel( new AccountingItemAttributeModel(NULL, NULL) ),
        priceFieldModel(pfm){
    }
    ~AccountingItemTAMGUIPrivate(){
        delete ui;
    }
    Ui::AccountingItemTAMGUI * ui;
    AccountingBill * bill;
    AccountingBillItem * item;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingItemTAMGUI::AccountingItemTAMGUI(PriceFieldModel * pfm, QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemTAMGUIPrivate( pfm ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingItemTAMGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingItemTAMGUI::removeAttribute );

    m_d->ui->beginDateLineEdit->installEventFilter( this );
    m_d->ui->endDateLineEdit->installEventFilter( this );
}

AccountingItemTAMGUI::~AccountingItemTAMGUI() {
    delete m_d;
}

void AccountingItemTAMGUI::setAccountingItem(AccountingBillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != NULL ){
            disconnect( m_d->item, &AccountingBillItem::dateBeginChanged, this, &AccountingItemTAMGUI::setDateBegin );
            disconnect( m_d->item, &AccountingBillItem::dateEndChanged, this, &AccountingItemTAMGUI::setDateEnd );
            disconnect( m_d->item, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingItemNULL );

            disconnect( m_d->item, &AccountingBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
        m_d->ui->totalAmountToBeDiscountedLineEdit->clear();
        m_d->ui->amountNotToBeDiscountedLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();

        m_d->ui->beginDateLineEdit->clear();
        m_d->ui->endDateLineEdit->clear();

        m_d->item = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->item != NULL ){
            m_d->ui->beginDateLineEdit->setText( m_d->item->dateBeginStr() );
            m_d->ui->endDateLineEdit->setText( m_d->item->dateEndStr() );
            connect( m_d->item, &AccountingBillItem::dateBeginChanged, this, &AccountingItemTAMGUI::setDateBegin );
            connect( m_d->item, &AccountingBillItem::dateEndChanged, this, &AccountingItemTAMGUI::setDateEnd );

            connect( m_d->item, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingItemNULL );

            m_d->ui->totalAmountToBeDiscountedLineEdit->setText( m_d->item->totalAmountToBeDiscountedStr() );
            connect( m_d->item, &AccountingBillItem::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToBeDiscountedLineEdit->setText( m_d->item->amountNotToBeDiscountedStr() );
            connect( m_d->item, &AccountingBillItem::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->item->totalAmountStr() );
            connect( m_d->item, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
        }
    }
}

void AccountingItemTAMGUI::setAccountingItemNULL() {
    setAccountingItem( NULL );
}

void AccountingItemTAMGUI::addAttribute(){
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

void AccountingItemTAMGUI::removeAttribute(){
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

void AccountingItemTAMGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->bill != NULL ){
        m_d->itemAttributeModel->setAttributeModel( NULL );
        disconnect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingNULL );
    }

    m_d->bill = b;

    if( m_d->bill != NULL ){
        m_d->itemAttributeModel->setAttributeModel( m_d->bill->attributeModel() );
        connect( m_d->bill, &AccountingBill::aboutToBeDeleted, this, &AccountingItemTAMGUI::setAccountingNULL );
    }

    // quando si cambia computo corrente la scheda della riga si azzera
    setAccountingItem( NULL );
}

void AccountingItemTAMGUI::setAccountingNULL() {
    setAccountingBill(NULL);
}

void AccountingItemTAMGUI::setDateBegin( const QString &newVal ) {
    m_d->ui->beginDateLineEdit->setText( newVal );
}

void AccountingItemTAMGUI::setDateEnd( const QString &newVal ) {
    m_d->ui->endDateLineEdit->setText( newVal );
}

bool AccountingItemTAMGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick )     {
        if( m_d->item != NULL ){
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
