#include "accountinggui.h"
#include "ui_accountinggui.h"

#include "projectaccountingparentitem.h"

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

    // m_d->ui->workProgressView->setModel( m_d->accounting->workProgresses() );
    // m_d->ui->amountDiscountToApplyLineEdit->setText( m_d->accounting->workProgresses()->amountDiscountToApplyStr() );
    // m_d->ui->amountDiscountLineEdit->setText( m_d->accounting->workProgresses()->amountDiscountStr() );
    // m_d->ui->amountNoDiscountLineEdit->setText( m_d->accounting->workProgresses()->amountNoDiscountStr() );
    // m_d->ui->amountTotalLineEdit->setText( m_d->accounting->workProgresses()->amountTotalStr() );

    connect( m_d->ui->addWorkProgressPushButton, &QPushButton::clicked, this, &AccountingGUI::addWorkProgress );
    connect( m_d->ui->delWorkProgressPushButton, &QPushButton::clicked, this, &AccountingGUI::delWorkProgress );
}

AccountingGUI::~AccountingGUI() {
    delete m_d;
}

void AccountingGUI::addWorkProgress() {
    // m_d->accounting->workProgresses()->insertWorkProgress();
}

void AccountingGUI::delWorkProgress() {
    // m_d->accounting->workProgresses()->removeWorkProgress();
}
