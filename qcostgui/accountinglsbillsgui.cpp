#include "accountinglsbillsgui.h"

#include "accountinglsbillsdatagui.h"
#include "attributesgui.h"

#include "accountinglsbills.h"

class AccountingLSBillsGUIPrivate {
public:
    AccountingLSBillsGUIPrivate(AccountingLSBills *myBills, PriceFieldModel * pfm, MathParser * prs, QString * wpf ):
        attributesGUI( new AttributesGUI(pfm, prs, wpf) ),
        dataGUI( new AccountingLSBillsDataGUI( myBills) ){

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
