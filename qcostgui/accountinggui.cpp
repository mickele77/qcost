#include "accountinggui.h"
#include "ui_accountinggui.h"

#include "projectaccountingparentitem.h"
#include "paymentadatamodel.h"

#include <QShowEvent>

class AccountingGUIPrivate {
public:
    AccountingGUIPrivate( ProjectAccountingParentItem * acc ):
        accounting(acc),
        ui(new Ui::AccountingGUI){
    }
    ~AccountingGUIPrivate(){
        delete ui;
    }

    ProjectAccountingParentItem * accounting;
    Ui::AccountingGUI *ui;
};

AccountingGUI::AccountingGUI( ProjectAccountingParentItem * acc, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingGUIPrivate( acc ) ) {
    m_d->ui->setupUi(this);

    m_d->ui->workProgressView->setModel( m_d->accounting->dataModel() );
    updateAmounts();

    connect( m_d->ui->addWorkProgressPushButton, &QPushButton::clicked, this, &AccountingGUI::addPayment );
    connect( m_d->ui->delWorkProgressPushButton, &QPushButton::clicked, this, &AccountingGUI::removePayment );
}

AccountingGUI::~AccountingGUI() {
    delete m_d;
}

void AccountingGUI::updateAmounts(){
    m_d->accounting->updateAmounts();
    m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->accounting->totalAmountToDiscountStr() );
    m_d->ui->amountNotToDiscountLineEdit->setText( m_d->accounting->amountNotToDiscountStr() );
    m_d->ui->amountToDiscountLineEdit->setText( m_d->accounting->amountToDiscountStr() );
    m_d->ui->amountDiscountedLineEdit->setText( m_d->accounting->amountDiscountedStr() );
    m_d->ui->totalAmountLineEdit->setText( m_d->accounting->totalAmountStr() );
}

void AccountingGUI::addPayment() {
    if( m_d->accounting != NULL ){
        int pos = m_d->accounting->workProgressBillsCount();
        if( m_d->ui->workProgressView->selectionModel() ){
            QModelIndex currIndex = m_d->ui->workProgressView->selectionModel()->currentIndex();
            if( currIndex.isValid() ){
                pos = currIndex.row();
            }
        }
        m_d->accounting->dataModel()->insertPaymentsRequest( pos );
    }
}

void AccountingGUI::removePayment() {
    if( m_d->accounting != NULL ){
        int pos = m_d->accounting->workProgressBillsCount()-1;
        if( m_d->ui->workProgressView->selectionModel() ){
            QModelIndex currIndex = m_d->ui->workProgressView->selectionModel()->currentIndex();
            if( currIndex.isValid() ){
                pos = currIndex.row();
            }
        }
        m_d->accounting->dataModel()->removePaymentsRequest( pos );
    }
}

void AccountingGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        updateAmounts();
    }
    QWidget::showEvent( event );
}
