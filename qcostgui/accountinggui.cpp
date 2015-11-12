#include "accountinggui.h"
#include "ui_accountinggui.h"

#include "projectaccountingparentitem.h"
#include "paymentdatamodel.h"

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
}

AccountingGUI::~AccountingGUI() {
    delete m_d;
}

void AccountingGUI::updateAmounts(){
    m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->accounting->totalAmountToDiscountStr() );
    m_d->ui->amountNotToDiscountLineEdit->setText( m_d->accounting->amountNotToDiscountStr() );
    m_d->ui->amountToDiscountLineEdit->setText( m_d->accounting->amountToDiscountStr() );
    m_d->ui->amountDiscountedLineEdit->setText( m_d->accounting->amountDiscountedStr() );
    m_d->ui->totalAmountLineEdit->setText( m_d->accounting->totalAmountStr() );
}

void AccountingGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        updateAmounts();
    }
    QWidget::showEvent( event );
}
