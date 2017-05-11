#include "accountingitemcommentgui.h"
#include "ui_accountingitemcommentgui.h"

#include "accountingtambillitem.h"
#include "accountingbillitem.h"

#include <QLabel>

class AccountingItemCommentGUIPrivate{
public:
    AccountingItemCommentGUIPrivate():
        ui(new Ui::AccountingItemCommentGUI),
        TAMBillItem(NULL),
        billItem(NULL){
    }
    ~AccountingItemCommentGUIPrivate(){
        delete ui;
    }
    Ui::AccountingItemCommentGUI * ui;
    AccountingTAMBillItem * TAMBillItem;
    AccountingBillItem * billItem;
    PriceFieldModel * priceFieldModel;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

AccountingItemCommentGUI::AccountingItemCommentGUI(QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemCommentGUIPrivate() ) {
    m_d->ui->setupUi(this);
}

AccountingItemCommentGUI::~AccountingItemCommentGUI() {
    delete m_d;
}

void AccountingItemCommentGUI::setAccountingItem(AccountingTAMBillItem *b) {
    if( m_d->TAMBillItem != b || m_d->billItem != NULL ){
        if( m_d->TAMBillItem != NULL ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemNULL );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
        if( m_d->billItem != NULL ){
            disconnect( m_d->billItem, &AccountingBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemNULL );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }

        m_d->ui->titleLineEdit->clear();
        m_d->billItem = NULL;
        m_d->TAMBillItem = b;

        if( m_d->TAMBillItem != NULL ){
            m_d->ui->titleLineEdit->setText( m_d->TAMBillItem->text() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemNULL );
            connect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
    }
}

void AccountingItemCommentGUI::setAccountingItem(AccountingBillItem *b) {
    if( m_d->billItem != b || m_d->TAMBillItem != NULL ){
        if( m_d->TAMBillItem != NULL ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemNULL );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
        if( m_d->billItem != NULL ){
            disconnect( m_d->billItem, &AccountingBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemNULL );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }

        m_d->ui->titleLineEdit->clear();
        m_d->billItem = b;
        m_d->TAMBillItem = NULL;

        if( m_d->billItem != NULL ){
            m_d->ui->titleLineEdit->setText( m_d->billItem->text() );
            connect( m_d->billItem, &AccountingBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            connect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemNULL );
            connect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
    }
}

void AccountingItemCommentGUI::setAccountingItemNULL() {
    setAccountingItem( (AccountingTAMBillItem *)(NULL) );
    setAccountingItem( (AccountingBillItem *)(NULL) );
}

void AccountingItemCommentGUI::updateTextLineEdit() {
    if( m_d->TAMBillItem != NULL ){
        m_d->ui->titleLineEdit->setText( m_d->TAMBillItem->text() );
    } else if( m_d->billItem != NULL ){
        m_d->ui->titleLineEdit->setText( m_d->billItem->text() );
    }
}

void AccountingItemCommentGUI::updateTextMeasure() {
    if( m_d->TAMBillItem != NULL ){
        m_d->TAMBillItem->setText( m_d->ui->titleLineEdit->text() );
    } else if( m_d->billItem != NULL ){
        m_d->billItem->setText( m_d->ui->titleLineEdit->text() );
    }
}
