#include "accountinglsbillsdatagui.h"
#include "ui_accountinglsbillsdatagui.h"

#include "accountinglsbills.h"

#include <QShowEvent>

class AccountingLSBillsDataGUIPrivate{
public:
    AccountingLSBillsDataGUIPrivate(AccountingLSBills * myBills) :
        ui(new Ui::AccountingLSBillsDataGUI),
        lsBills(myBills){

    }
    ~AccountingLSBillsDataGUIPrivate(){
        delete ui;
    }

    Ui::AccountingLSBillsDataGUI *ui;
    AccountingLSBills * lsBills;
};

AccountingLSBillsDataGUI::AccountingLSBillsDataGUI(AccountingLSBills * myBills, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingLSBillsDataGUIPrivate( myBills ) ) {
    m_d->ui->setupUi(this);
}

AccountingLSBillsDataGUI::~AccountingLSBillsDataGUI() {
    delete m_d;
}

void AccountingLSBillsDataGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        m_d->lsBills->updateAmounts();
        m_d->ui->projAmountLineEdit->setText( m_d->lsBills->projAmountStr() );
        m_d->ui->accAmountLineEdit->setText( m_d->lsBills->accAmountStr() );
        m_d->ui->percentageAccountedLineEdit->setText( m_d->lsBills->percentageAccountedStr() );
    }
    QWidget::showEvent( event );
}
