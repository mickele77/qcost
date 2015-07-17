#include "pricelistdbviewer.h"

#include "pricelistdbwidget.h"

#include <QDialogButtonBox>
#include <QGridLayout>

class PriceListDBViewerPrivate{
public:
    PriceListDBViewerPrivate(MathParser * p, QWidget *parent):
        mainLayout( new QGridLayout(parent)),
        dbWidget( new PriceListDBWidget( NULL, NULL, p, "viewerConnection", parent )),
        buttonBox( new QDialogButtonBox(QDialogButtonBox::Ok) ) {
        mainLayout->addWidget( dbWidget, 0, 0);
        mainLayout->addWidget( buttonBox, 1, 0);
        dbWidget->hideImportOptions();
    }

    QGridLayout * mainLayout;
    PriceListDBWidget * dbWidget;
    QDialogButtonBox * buttonBox;
};

PriceListDBViewer::PriceListDBViewer(MathParser * p, QWidget *parent) :
    QDialog(parent),
    m_d( new PriceListDBViewerPrivate( p, this)){
    connect( m_d->buttonBox, &QDialogButtonBox::accepted, this, &PriceListDBViewer::accept );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

PriceListDBViewer::~PriceListDBViewer(){
    delete m_d;
}
