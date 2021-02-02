#include "qcalendardialog.h"
#include "ui_qcalendardialog.h"

class QCalendarDialogPrivate{
public:
    QCalendarDialogPrivate( QDate * d ):
        date(d),
        ui(new Ui::QCalendarDialog)  {
    }
    ~QCalendarDialogPrivate(){
        delete ui;
    }

    QDate * date;
    Ui::QCalendarDialog *ui;
};

QCalendarDialog::QCalendarDialog(QDate * d, QWidget *parent) :
    QDialog(parent),
    m_d( new QCalendarDialogPrivate(d) ) {

    m_d->ui->setupUi(this);
    m_d->ui->calendarWidget->setSelectedDate( *d );
    connect( m_d->ui->buttonBox, &QDialogButtonBox::accepted, this, &QCalendarDialog::changeDateAndClose );
    connect( m_d->ui->buttonBox, &QDialogButtonBox::rejected, this, &QCalendarDialog::reject );
}

QCalendarDialog::~QCalendarDialog() {
    delete m_d;
}

void QCalendarDialog::changeDateAndClose(){
    if( m_d->date != nullptr ) {
        *(m_d->date) = m_d->ui->calendarWidget->selectedDate();
    }
    accept();
}
