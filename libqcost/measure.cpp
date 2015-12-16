#include "measure.h"

#include "accountingbillitem.h"
#include "billitem.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QVariant>
#include <QString>

class MeasurePrivate{
public:
    MeasurePrivate( BillItem * bItem, MathParser * p, UnitMeasure * ump ):
        parser( p ),
        unitMeasure( ump ),
        billItem(bItem),
        accountingBillItem(NULL),
        comment(""),
        formula(""),
        quantity(0.0){
    }
    MeasurePrivate( AccountingBillItem * accBItem, MathParser * p, UnitMeasure * ump ):
        parser( p ),
        unitMeasure( ump ),
        billItem(NULL),
        accountingBillItem(accBItem),
        comment(""),
        formula(""),
        quantity(0.0){
    }
    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser != NULL ){
            return parser->toString( i, f, prec );
        } else {
            return QString::number( i, f, prec );
        }
    }
    MathParser * parser;
    UnitMeasure * unitMeasure;
    BillItem * billItem;
    QList< QPair<BillItem *, int> > connectedBillItems;
    AccountingBillItem * accountingBillItem;
    QList< QPair<AccountingBillItem *, int> > connectedAccBillItems;
    QString comment;
    QString formula;
    double quantity;
    QXmlStreamAttributes tmpAttrs;
};

Measure::Measure(BillItem * bItem , MathParser * p, UnitMeasure * ump) :
    QObject(0),
    m_d( new MeasurePrivate( bItem, p, ump )){
}

Measure::Measure(AccountingBillItem *accBItem, MathParser *p, UnitMeasure *ump):
    QObject(0),
    m_d( new MeasurePrivate( accBItem, p, ump )){
}

Measure::~Measure(){
    delete m_d;
}

Measure &Measure::operator=(const Measure &cp) {
    if( &cp != this ){
        setUnitMeasure( cp.m_d->unitMeasure );
        setComment( cp.m_d->comment );
        if( (m_d->billItem != NULL) && (m_d->billItem == cp.m_d->billItem) ){
            setFormula( cp.m_d->formula, true );
        } else if( (m_d->accountingBillItem != NULL) && (m_d->accountingBillItem == cp.m_d->accountingBillItem) ){
            setFormula( cp.m_d->formula, true );
        } else {
            setFormula( cp.formula(), false );
        }
    }

    return *this;
}

QString Measure::comment() const{
    return m_d->comment;
}

void Measure::setComment( const QString & nc ){
    m_d->comment = nc;
}

void Measure::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        m_d->unitMeasure = ump;
        updateQuantity();
    }
}

QString Measure::formula() const {
    QString displayedForm = m_d->formula;

    if( m_d->billItem != NULL ){
        QRegExp rx("(\\[|\\])");
        QStringList formSplitted = displayedForm.split( rx );
        displayedForm.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( i%2 == 1 ){
                bool ok = false;
                int priceFieldConnItem = 0;
                BillItem * connItem = NULL;
                if( formSplitted.at(i).contains(":")){
                    QStringList formSplittedAmounts = formSplitted.at(i).split(":");
                    if( formSplittedAmounts.size() > 0 ){
                        uint connItemId = formSplittedAmounts.at(0).toUInt(&ok);
                        if( ok ){
                            connItem = m_d->billItem->findItemFromId( connItemId );
                            int purpPriceFieldConnTiem = formSplittedAmounts.at(1).toInt( &ok );
                            if( ok ){
                                priceFieldConnItem = purpPriceFieldConnTiem;
                            }
                        }
                    }
                } else {
                    uint connItemId = formSplitted.at(i).toUInt(&ok);
                    if( ok ){
                        connItem = m_d->billItem->findItemFromId( connItemId );
                    }
                }

                if( ok && connItem != NULL  ){
                    if(  priceFieldConnItem > 0 ){
                        displayedForm += "[" + connItem->progCode() + ":" + QString::number(priceFieldConnItem)+ "]";
                    } else {
                        displayedForm += "[" + connItem->progCode() + "]";
                    }
                } else {
                    displayedForm += "[Err]";
                }
            } else {
                displayedForm += formSplitted.at(i);
            }
        }
    } else if( m_d->accountingBillItem != NULL ){
        QRegExp rx("(\\[|\\])");
        QStringList formSplitted = displayedForm.split( rx );
        displayedForm.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( i%2 == 1 ){
                bool ok = false;
                int priceFieldConnItem = 0;
                AccountingBillItem * connItem = NULL;
                if( formSplitted.at(i).contains(":")){
                    QStringList formSplittedAmounts = formSplitted.at(i).split(":");
                    if( formSplittedAmounts.size() > 0 ){
                        uint connItemId = formSplittedAmounts.at(0).toUInt(&ok);
                        if( ok ){
                            connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                            int purpPriceFieldConnTiem = formSplittedAmounts.at(1).toInt( &ok );
                            if( ok ){
                                priceFieldConnItem = purpPriceFieldConnTiem;
                            }
                        }
                    }
                } else {
                    uint connItemId = formSplitted.at(i).toUInt(&ok);
                    if( ok ){
                        connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                    }
                }

                if( ok && connItem != NULL  ){
                    if(  priceFieldConnItem > 0 ){
                        displayedForm += "[" + connItem->fullProgCode() + ":" + QString::number(priceFieldConnItem)+ "]";
                    } else {
                        displayedForm += "[" + connItem->fullProgCode() + "]";
                    }
                } else {
                    displayedForm += "[Err]";
                }
            } else {
                displayedForm += formSplitted.at(i);
            }
        }
    }
    return displayedForm;
}

void Measure::setFormula( const QString & nf, bool connItemFromId ){
    if( nf != m_d->formula ){
        // azzera l'elenco dei BillItem connessi
        for( QList< QPair<BillItem *, int> >::iterator it = m_d->connectedBillItems.begin(); it != m_d->connectedBillItems.end(); ++it ){
            if( it->second > -1 ){
                disconnect( it->first, static_cast<void(BillItem::*)( int, double )>(&BillItem::amountChanged), this, &Measure::updateQuantity );
            } else {
                disconnect( it->first, &BillItem::quantityChanged, this, &Measure::updateQuantity );
            }
        }
        m_d->connectedBillItems.clear();

        // azzera l'elenco dei AccountingBillItem connessi
        for( QList< QPair<AccountingBillItem *, int> >::iterator it = m_d->connectedAccBillItems.begin(); it != m_d->connectedAccBillItems.end(); ++it ){
            if( it->second > -1 ){
                disconnect( it->first, &AccountingBillItem::amountsChanged, this, &Measure::updateQuantity );
            } else {
                disconnect( it->first, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
            }
        }
        m_d->connectedBillItems.clear();

        // trasferisce provvisoriamente il nuovo valore della formula in newForm
        QString newForm = nf;

        if( m_d->billItem != NULL ){
            if( connItemFromId ){
                QRegExp rx("(\\[|\\])");
                QStringList newFormSplitted = newForm.split( rx );
                newForm.clear();
                for( int i=0; i < newFormSplitted.size(); i++ ){
                    if( i%2 == 1 ){
                        if( newFormSplitted.at(i).contains(":")){
                            QStringList newFormSplittedAmounts = newFormSplitted.at(i).split(":");
                            if( newFormSplittedAmounts.size() > 0 ){
                                bool ok = false;
                                uint connItemId = newFormSplittedAmounts.at(0).toUInt(&ok);
                                BillItem * connItem = NULL;
                                if( ok ){
                                    connItem = m_d->billItem->findItemFromId( connItemId );
                                }
                                if( connItem != NULL ){
                                    int connItemPriceField = 0;
                                    if( newFormSplittedAmounts.size() > 1 ){
                                        connItemPriceField = newFormSplittedAmounts.at(1).toInt();
                                    }
                                    if( connItemPriceField > 0 ){
                                        m_d->connectedBillItems << qMakePair( connItem, connItemPriceField );
                                        newForm += "[" + QString::number(connItem->id()) + ":" + QString::number(connItemPriceField) + "]";
                                        connect( connItem, static_cast<void(BillItem::*)( int, double )>(&BillItem::amountChanged), this, &Measure::updateQuantity );
                                    } else {
                                        m_d->connectedBillItems << qMakePair( connItem, -1 );
                                        newForm += "[" + QString::number(connItem->id()) + "]";
                                        connect( connItem, &BillItem::quantityChanged, this, &Measure::updateQuantity );
                                    }
                                } else {
                                    newForm += "0";
                                }
                            }
                        } else {
                            bool ok = false;
                            uint connItemId = newFormSplitted.at(0).toUInt(&ok);
                            BillItem * connItem = NULL;
                            if( ok ){
                                connItem = m_d->billItem->findItemFromId( connItemId );
                            }
                            if( connItem != NULL ){
                                m_d->connectedBillItems << qMakePair(connItem, -1);
                                newForm += "[" + QString::number(connItem->id()) + "]";
                                connect( connItem, &BillItem::quantityChanged, this, &Measure::updateQuantity );
                            } else {
                                newForm += "0";
                            }
                        }
                    } else {
                        newForm += newFormSplitted.at(i);
                    }
                }
            } else {
                QRegExp rx("(\\[|\\])");
                QStringList newFormSplitted = newForm.split( rx );
                newForm.clear();
                for( int i=0; i < newFormSplitted.size(); i++ ){
                    if( i%2 == 1 ){
                        if( newFormSplitted.at(i).contains(":")){
                            QStringList newFormSplittedAmounts = newFormSplitted.at(i).split(":");
                            if( newFormSplittedAmounts.size() > 0 ){
                                BillItem * connItem = m_d->billItem->findItemFromProgCode( newFormSplittedAmounts.at(0) );
                                if( connItem != NULL ){
                                    int connItemPriceField = 0;
                                    if( newFormSplittedAmounts.size() > 1 ){
                                        connItemPriceField = newFormSplittedAmounts.at(1).toInt();
                                    }
                                    if( connItemPriceField > 0 ){
                                        m_d->connectedBillItems << qMakePair( connItem, connItemPriceField );
                                        newForm += "[" + QString::number(connItem->id()) + ":" + QString::number(connItemPriceField) + "]";
                                        connect( connItem, static_cast<void(BillItem::*)( int, double )>(&BillItem::amountChanged), this, &Measure::updateQuantity );
                                    } else {
                                        m_d->connectedBillItems << qMakePair( connItem, -1 );
                                        newForm += "[" + QString::number(connItem->id()) + "]";
                                        connect( connItem, &BillItem::quantityChanged, this, &Measure::updateQuantity );
                                    }
                                } else {
                                    newForm += "0";
                                }
                            }
                        } else {
                            BillItem * connItem = m_d->billItem->findItemFromProgCode( newFormSplitted.at(i) );
                            if( connItem != NULL ){
                                m_d->connectedBillItems << qMakePair(connItem, -1);
                                newForm += "[" + QString::number(connItem->id()) + "]";
                                connect( connItem, &BillItem::quantityChanged, this, &Measure::updateQuantity );
                            } else {
                                newForm += "0";
                            }
                        }
                    } else {
                        newForm += newFormSplitted.at(i);
                    }
                }
            }
        } else if( m_d->accountingBillItem != NULL ){
            if( connItemFromId ){
                QRegExp rx("(\\[|\\])");
                QStringList newFormSplitted = newForm.split( rx );
                newForm.clear();
                for( int i=0; i < newFormSplitted.size(); i++ ){
                    if( i%2 == 1 ){
                        if( newFormSplitted.at(i).contains(":")){
                            QStringList newFormSplittedAmounts = newFormSplitted.at(i).split(":");
                            if( newFormSplittedAmounts.size() > 0 ){
                                bool ok = false;
                                uint connItemId = newFormSplittedAmounts.at(0).toUInt(&ok);
                                AccountingBillItem * connItem = NULL;
                                if( ok ){
                                    connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                }
                                if( connItem != NULL ){
                                    int connItemPriceField = 0;
                                    if( newFormSplittedAmounts.size() > 1 ){
                                        connItemPriceField = newFormSplittedAmounts.at(1).toInt();
                                    }
                                    if( connItemPriceField > 0 ){
                                        m_d->connectedAccBillItems << qMakePair( connItem, connItemPriceField );
                                        newForm += "[" + QString::number(connItem->id()) + ":" + QString::number(connItemPriceField) + "]";
                                        connect( connItem, &AccountingBillItem::amountsChanged, this, &Measure::updateQuantity );
                                    } else {
                                        m_d->connectedAccBillItems << qMakePair( connItem, -1 );
                                        newForm += "[" + QString::number(connItem->id()) + "]";
                                        connect( connItem, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
                                    }
                                } else {
                                    newForm += "0";
                                }
                            }
                        } else {
                            bool ok = false;
                            uint connItemId = newFormSplitted.at(0).toUInt(&ok);
                            AccountingBillItem * connItem = NULL;
                            if( ok ){
                                connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                            }
                            if( connItem != NULL ){
                                m_d->connectedAccBillItems << qMakePair(connItem, -1);
                                newForm += "[" + QString::number(connItem->id()) + "]";
                                connect( connItem, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
                            } else {
                                newForm += "0";
                            }
                        }
                    } else {
                        newForm += newFormSplitted.at(i);
                    }
                }
            } else {
                QRegExp rx("(\\[|\\])");
                QStringList newFormSplitted = newForm.split( rx );
                newForm.clear();
                for( int i=0; i < newFormSplitted.size(); i++ ){
                    if( i%2 == 1 ){
                        if( newFormSplitted.at(i).contains(":")){
                            QStringList newFormSplittedAmounts = newFormSplitted.at(i).split(":");
                            if( newFormSplittedAmounts.size() > 0 ){
                                AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromProgCode( newFormSplittedAmounts.at(0) );
                                if( connItem != NULL ){
                                    int connItemPriceField = 0;
                                    if( newFormSplittedAmounts.size() > 1 ){
                                        connItemPriceField = newFormSplittedAmounts.at(1).toInt();
                                    }
                                    if( connItemPriceField > 0 ){
                                        m_d->connectedAccBillItems << qMakePair( connItem, connItemPriceField );
                                        newForm += "[" + QString::number(connItem->id()) + ":" + QString::number(connItemPriceField) + "]";
                                        connect( connItem, &AccountingBillItem::amountsChanged, this, &Measure::updateQuantity );
                                    } else {
                                        m_d->connectedAccBillItems << qMakePair( connItem, -1 );
                                        newForm += "[" + QString::number(connItem->id()) + "]";
                                        connect( connItem, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
                                    }
                                } else {
                                    newForm += "0";
                                }
                            }
                        } else {
                            AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromProgCode( newFormSplitted.at(i) );
                            if( connItem != NULL ){
                                m_d->connectedAccBillItems << qMakePair(connItem, -1);
                                newForm += "[" + QString::number(connItem->id()) + "]";
                                connect( connItem, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
                            } else {
                                newForm += "0";
                            }
                        }
                    } else {
                        newForm += newFormSplitted.at(i);
                    }
                }
            }
        } else {
            QRegExp rx("(\\[|\\])");
            QStringList newFormSplitted = newForm.split( rx );
            newForm.clear();
            for( int i=0; i < newFormSplitted.size(); i++ ){
                if( i%2 == 1 ){
                    newForm += "0";
                } else {
                    newForm += newFormSplitted.at(i);
                }
            }
        }

        m_d->formula = newForm;
        updateQuantity();
    }
}

QString Measure::effectiveFormula(){
    QString effFormula = m_d->formula;
    if( m_d->billItem != NULL ){
        QRegExp rx("(\\[|\\])");
        QStringList formSplitted = effFormula.split( rx );
        effFormula.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( i%2 == 1 ){
                bool ok = false;
                uint connItemId = 0;
                int priceFieldConnItem = -1;
                if( formSplitted.at(i).contains(":")){
                    QStringList formSplittedAmounts = formSplitted.at(i).split(":");
                    if( formSplittedAmounts.size() > 1 ){
                        connItemId = formSplittedAmounts.at(0).toUInt( &ok );
                        if( ok ){
                            priceFieldConnItem = formSplittedAmounts.at(1).toInt( &ok ) - 1;
                        }
                    }
                } else {
                    connItemId = formSplitted.at(i).toUInt( &ok );
                }
                if( ok && connItemId > 0 ){
                    BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                    if( connItem != NULL ){
                        QList<BillItem *> connItems;
                        connItem->appendConnectedItems( &connItems );
                        if( connItems.contains(m_d->billItem) ){
                            ok = false;
                        } else {
                            if( priceFieldConnItem > -1 ){
                                effFormula += connItem->amountStr(priceFieldConnItem);
                            } else {
                                effFormula += connItem->quantityStr();
                            }
                        }
                    } else {
                        ok = false;
                    }
                }
                if( !ok ){
                    effFormula += "0";
                }
            } else {
                effFormula += formSplitted.at(i);
            }
        }
    } else if( m_d->accountingBillItem != NULL ){
        QRegExp rx("(\\[|\\])");
        QStringList formSplitted = effFormula.split( rx );
        effFormula.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( i%2 == 1 ){
                bool ok = false;
                uint connItemId = 0;
                int priceFieldConnItem = -1;
                if( formSplitted.at(i).contains(":")){
                    QStringList formSplittedAmounts = formSplitted.at(i).split(":");
                    if( formSplittedAmounts.size() > 1 ){
                        connItemId = formSplittedAmounts.at(0).toUInt( &ok );
                        if( ok ){
                            priceFieldConnItem = formSplittedAmounts.at(1).toInt( &ok ) - 1;
                        }
                    }
                } else {
                    connItemId = formSplitted.at(i).toUInt( &ok );
                }
                if( ok && connItemId > 0 ){
                    AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                    if( connItem != NULL ){
                        QList<AccountingBillItem *> connItems;
                        connItem->appendConnectedItems( &connItems );
                        if( connItems.contains(m_d->accountingBillItem) ){
                            ok = false;
                        } else {
                            if( priceFieldConnItem > -1 ){
                                effFormula += connItem->amountStr(priceFieldConnItem);
                            } else {
                                effFormula += connItem->quantityStr();
                            }
                        }
                    } else {
                        ok = false;
                    }
                }
                if( !ok ){
                    effFormula += "0";
                }
            } else {
                effFormula += formSplitted.at(i);
            }
        }
    } else {
        QRegExp rx("(\\[|\\])");
        QStringList formSplitted = effFormula.split( rx );
        effFormula.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( i%2 == 1 ){
                effFormula += "0";
            } else {
                effFormula += formSplitted.at(i);
            }
        }
    }
    return effFormula;
}

void Measure::updateQuantity(){
    double v = m_d->parser->evaluate( effectiveFormula() );
    if( m_d->unitMeasure ) {
        v = m_d->unitMeasure->applyPrecision( v );
    }
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( v );
    }
}

double Measure::quantity() const{
    double ret = 0.0;
    if( m_d->unitMeasure != NULL ){
        ret = m_d->unitMeasure->applyPrecision( m_d->quantity );
    } else {
        ret = m_d->quantity;
    }
    return ret;
}

QString Measure::quantityStr() const{
    QString realFormula = m_d->formula;
    realFormula.remove(" ");
    if( realFormula.isEmpty() ){
        return QString();
    }
    if( m_d->unitMeasure != NULL ){
        return m_d->toString( quantity(), 'f', m_d->unitMeasure->precision() ) ;
    } else {
        return m_d->toString( quantity(), 'f', 6 ) ;
    }
}

void Measure::writeXml( QXmlStreamWriter * writer ){
    writer->writeStartElement( "Measure" );
    writer->writeAttribute( "comment", comment() );
    QString f = formula();
    if( m_d->parser->decimalSeparator() != "." ){
        f.replace( m_d->parser->decimalSeparator(), ".");
    }
    writer->writeAttribute( "formula", f );
    writer->writeEndElement();
}


void Measure::loadFromXmlTmp() {
    if( m_d->tmpAttrs.hasAttribute( "comment" ) ){
        setComment(  m_d->tmpAttrs.value( "comment").toString() );
    }
    if( m_d->tmpAttrs.hasAttribute( "formula" ) ){
        QString f = m_d->tmpAttrs.value( "formula").toString();
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( ".", m_d->parser->decimalSeparator());
        }
        setFormula( f );
    }
    m_d->tmpAttrs.clear();
}

void Measure::loadXmlTmp(const QXmlStreamAttributes &attrs) {
    m_d->tmpAttrs.clear();
    m_d->tmpAttrs = attrs;
}

QList<BillItem *> Measure::connectedBillItems() {
    QList<BillItem *> ret;
    for( QList< QPair<BillItem *, int> >::iterator iter = m_d->connectedBillItems.begin();
         iter != m_d->connectedBillItems.end(); ++iter ) {
        ret << iter->first;
    }
    return ret;
}

QList<AccountingBillItem *> Measure::connectedAccBillItems() {
    QList<AccountingBillItem *> ret;
    for( QList< QPair<AccountingBillItem *, int> >::iterator iter = m_d->connectedAccBillItems.begin();
         iter != m_d->connectedAccBillItems.end(); ++iter ) {
        ret << iter->first;
    }
    return ret;
}
