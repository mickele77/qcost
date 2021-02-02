#include "accountingitemcommentgui.h"
#include "ui_accountingitemcommentgui.h"

#include "accountingtambillitem.h"
#include "accountingbillitem.h"

#include <QLabel>

class AccountingItemCommentGUIPrivate{
public:
    AccountingItemCommentGUIPrivate():
        ui(new Ui::AccountingItemCommentGUI),
        TAMBillItem(nullptr),
        billItem(nullptr){
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
    if( m_d->TAMBillItem != b || m_d->billItem != nullptr ){
        if( m_d->TAMBillItem != nullptr ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemnullptr );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
        if( m_d->billItem != nullptr ){
            disconnect( m_d->billItem, &AccountingBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemnullptr );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }

        m_d->ui->titleLineEdit->clear();
        m_d->billItem = nullptr;
        m_d->TAMBillItem = b;

        if( m_d->TAMBillItem != nullptr ){
            m_d->ui->titleLineEdit->setText( m_d->TAMBillItem->text() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemnullptr );
            connect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
    }
}

void AccountingItemCommentGUI::setAccountingItem(AccountingBillItem *b) {
    if( m_d->billItem != b || m_d->TAMBillItem != nullptr ){
        if( m_d->TAMBillItem != nullptr ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemnullptr );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
        if( m_d->billItem != nullptr ){
            disconnect( m_d->billItem, &AccountingBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemnullptr );
            disconnect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }

        m_d->ui->titleLineEdit->clear();
        m_d->billItem = b;
        m_d->TAMBillItem = nullptr;

        if( m_d->billItem != nullptr ){
            m_d->ui->titleLineEdit->setText( m_d->billItem->text() );
            connect( m_d->billItem, &AccountingBillItem::textChanged, this, &AccountingItemCommentGUI::updateTextLineEdit );
            connect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemCommentGUI::setAccountingItemnullptr );
            connect( m_d->ui->titleLineEdit, &QLineEdit::editingFinished, this, &AccountingItemCommentGUI::updateTextMeasure );
        }
    }
}

void AccountingItemCommentGUI::setAccountingItemnullptr() {
    setAccountingItem( (AccountingTAMBillItem *)(nullptr) );
    setAccountingItem( (AccountingBillItem *)(nullptr) );
}

void AccountingItemCommentGUI::updateTextLineEdit() {
    if( m_d->TAMBillItem != nullptr ){
        m_d->ui->titleLineEdit->setText( m_d->TAMBillItem->text() );
    } else if( m_d->billItem != nullptr ){
        m_d->ui->titleLineEdit->setText( m_d->billItem->text() );
    }
}

void AccountingItemCommentGUI::updateTextMeasure() {
    if( m_d->TAMBillItem != nullptr ){
        m_d->TAMBillItem->setText( m_d->ui->titleLineEdit->text() );
    } else if( m_d->billItem != nullptr ){
        m_d->billItem->setText( m_d->ui->titleLineEdit->text() );
    }
}
