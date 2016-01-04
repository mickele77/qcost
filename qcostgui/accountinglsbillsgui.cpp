#include "accountinglsbillsgui.h"

#include "accountinglsbillsdatagui.h"
#include "attributesgui.h"

#include "accountinglsbills.h"

#include <QShowEvent>

class AccountingLSBillsGUIPrivate {
public:
    AccountingLSBillsGUIPrivate(AccountingLSBills *myBills, PriceFieldModel * pfm, MathParser * prs, QString * wpf ):
        attributesGUI( new AttributesGUI(pfm, prs, wpf) ),
        dataGUI( new AccountingLSBillsDataGUI( myBills) ){
        attributesGUI->setBill( myBills );
    }

    AttributesGUI * attributesGUI;
    AccountingLSBillsDataGUI * dataGUI;
};

AccountingLSBillsGUI::AccountingLSBillsGUI( AccountingLSBills *myBills, PriceFieldModel * pfm, MathParser * prs, QString * wpf,
                                            QWidget *parent):
    QTabWidget(parent),
    m_d( new AccountingLSBillsGUIPrivate(myBills, pfm, prs, wpf) ){
    addTab( m_d->dataGUI, trUtf8("Importi complessivi") );
    addTab( m_d->attributesGUI, trUtf8("Etichette") );
}

void AccountingLSBillsGUI::showEvent(QShowEvent * event ) {
    if( event->type() == QEvent::Show ){
        m_d->attributesGUI->activateAttributeModel();
    }
    QWidget::showEvent( event );
}
