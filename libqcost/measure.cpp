#include "measure.h"

#include "varsmodel.h"
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

    static QStringList splitQString(const QString & str, const QList<QChar> &matchStr );
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

QStringList MeasurePrivate::splitQString( const QString & str, const QList<QChar> & matchStr ){
    QStringList retList;
    QString retStr;
    for( QString::const_iterator i = str.begin(); i != str.end(); ++i ){
        bool match = false;
        for( QList<QChar>::const_iterator j = matchStr.begin(); j != matchStr.end(); ++j ){
            if( *i == *j ){
                match = true;
                break;
            }
        }
        if( match ) {
            if( !retStr.isEmpty() ){
                retList.append( retStr );
            }
            retList.append( *i );
            retStr.clear();
        } else {
            retStr.append( *i);
        }
    }
    if( !retStr.isEmpty() ){
        retList.append( retStr );
    }
    return retList;
}


Measure::Measure(BillItem * bItem , MathParser * p, UnitMeasure * ump) :
    QObject(0),
    m_d( new MeasurePrivate( bItem, p, ump )){
    connect( m_d->billItem, &BillItem::currentPriceDataSetChanged, this, &Measure::updateQuantity);
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
        QList<QChar> matchStr;
        matchStr << '[' << ']' << '{' << '}';
        QStringList formSplitted = MeasurePrivate::splitQString( displayedForm,  matchStr );
        displayedForm.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( formSplitted.at(i) == "[" ){
                bool signOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "]"  ){
                        bool ok = false;
                        uint connItemId = formSplitted.at(i+1).toUInt(&ok);
                        if( ok ){
                            BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                            if( connItem != NULL ){
                                signOk = true;
                                displayedForm += "[" + connItem->progCode() + "]";
                                i = i+2;
                            }
                        }
                    }
                }
                if( ! signOk ){
                    displayedForm += formSplitted.at(i);
                }
            } else if( formSplitted.at(i) == "{" ){
                bool signOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "}"  ){
                        QString intStr = formSplitted.at(i+1);
                        if( intStr.contains(":") ){
                            QStringList intStrSplit = intStr.split(":");
                            if( intStrSplit.size() > 1 ){
                                bool ok = false;
                                uint connItemId = intStrSplit.at(0).toUInt(&ok);
                                if( ok ){
                                    int connPriceDataSet = intStrSplit.at(1).toInt(&ok);
                                    if( ok ){
                                        BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                                        if( connItem != NULL ){
                                            signOk = true;
                                            displayedForm += "{" + connItem->progCode() + ":" + QString::number(connPriceDataSet) + "}";
                                            i = i+2;
                                        }
                                    }
                                }
                            }
                        } else {
                            bool ok = false;
                            uint connItemId = intStr.toUInt(&ok);
                            if( ok ){
                                BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                                if( connItem != NULL ){
                                    signOk = true;
                                    displayedForm += "{" + connItem->progCode() + "}";
                                    i = i+2;
                                }
                            }
                        }
                    }
                }
                if( ! signOk ){
                    displayedForm += formSplitted.at(i);
                }
            } else {
                displayedForm += formSplitted.at(i);
            }
        }
    } else if( m_d->accountingBillItem != NULL ){
        QList<QChar> matchStr;
        matchStr << '[' << ']' << '{' << '}';
        QStringList formSplitted = MeasurePrivate::splitQString( displayedForm,  matchStr );
        displayedForm.clear();
        for( int i=0; i < formSplitted.size(); i++ ){
            if( formSplitted.at(i) == "[" ){
                bool signOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "]"  ){
                        bool ok = false;
                        uint connItemId = formSplitted.at(i+1).toUInt(&ok);
                        if( ok ){
                            AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                            if( connItem != NULL ){
                                signOk = true;
                                displayedForm += "[" + connItem->progCode() + "]";
                                i = i+2;
                            }
                        }
                    }
                }
                if( ! signOk ){
                    displayedForm += formSplitted.at(i);
                }
            } else if( formSplitted.at(i) == "{" ){
                bool signOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "}"  ){
                        QString intStr = formSplitted.at(i+1);
                        if( intStr.contains(":") ){
                            QStringList intStrSplit = intStr.split(":");
                            if( intStrSplit.size() > 1 ){
                                bool ok = false;
                                uint connItemId = intStrSplit.at(0).toUInt(&ok);
                                if( ok ){
                                    int connPriceDataSet = intStrSplit.at(1).toInt(&ok);
                                    if( ok ){
                                        AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                        if( connItem != NULL ){
                                            signOk = true;
                                            displayedForm += "{" + connItem->progCode() + ":" + QString::number(connPriceDataSet) + "}";
                                            i = i+2;
                                        }
                                    }
                                }
                            }
                        } else {
                            bool ok = false;
                            uint connItemId = intStr.toUInt(&ok);
                            if( ok ){
                                AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                if( connItem != NULL ){
                                    signOk = true;
                                    displayedForm += "{" + connItem->progCode() + "}";
                                    i = i+2;
                                }
                            }
                        }
                    }
                }
                if( ! signOk ){
                    displayedForm += formSplitted.at(i);
                }
            } else {
                displayedForm += formSplitted.at(i);
            }
        }
    }
    return displayedForm;
}

void Measure::setFormula( const QString & newFormulaInput, bool connItemFromId ){
    // il valore della nuova formula che verrà memorizzato
    QString newFormula;

    if( m_d->billItem != NULL ){
        // azzera l'elenco dei BillItem connessi
        for( QList< QPair<BillItem *, int> >::iterator it = m_d->connectedBillItems.begin(); it != m_d->connectedBillItems.end(); ++it ){
            if( it->second > -2 ){
                disconnect( it->first, static_cast<void(BillItem::*)( int, double )>(&BillItem::amountChanged), this, &Measure::updateQuantity );
            } else {
                disconnect( it->first, &BillItem::quantityChanged, this, &Measure::updateQuantity );
            }
        }
        m_d->connectedBillItems.clear();

        bool setFormulaOk = true;
        // spezziamo newForm in base ai diversi tipi di parentesi
        QList<QChar> matchStr;
        matchStr << '[' << ']' << '{' << '}';
        QStringList formSplitted = MeasurePrivate::splitQString( newFormulaInput,  matchStr );
        for( int i=0; i < formSplitted.size(); i++ ){
            if( formSplitted.at(i) == "[" ){
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "]"  ){
                        BillItem * connItem = NULL;
                        if( connItemFromId ){
                            bool uintOk = false;
                            uint connItemId = formSplitted.at(i+1).toUInt( &uintOk );
                            if( uintOk ){
                                connItem = m_d->billItem->findItemFromId( connItemId );
                            }
                        } else {
                            connItem = m_d->billItem->findItemFromProgCode( formSplitted.at(i+1) );
                        }
                        if( connItem != NULL ){
                            setFormulaOk = true;
                            m_d->connectedBillItems << qMakePair(connItem, -2);
                            newFormula += "[" + QString::number(connItem->id()) + "]";
                            connect( connItem, &BillItem::quantityChanged, this, &Measure::updateQuantity );
                            i = i+2;
                        } else {
                            setFormulaOk = false;
                        }
                    } else {
                        setFormulaOk = false;
                    }
                } else {
                    setFormulaOk = false;
                }
            } else if( formSplitted.at(i) == "{" ){
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "}"  ){
                        QStringList formSplittedAmounts = formSplitted.at(i+1).split(":");
                        if( formSplittedAmounts.size() > 1 ){
                            BillItem * connItem = NULL;
                            if( connItemFromId ){
                                bool uintOk = false;
                                uint connItemId = formSplittedAmounts.at(0).toUInt( &uintOk );
                                if( uintOk ){
                                    connItem = m_d->billItem->findItemFromId( connItemId );
                                }
                            } else {
                                connItem = m_d->billItem->findItemFromProgCode( formSplittedAmounts.at(0) );
                            }
                            bool intOk = false;
                            int connPriceDataSet = formSplittedAmounts.at(1).toInt( &intOk);
                            if( connItem != NULL && intOk ){
                                setFormulaOk = true;
                                m_d->connectedBillItems << qMakePair(connItem, connPriceDataSet);
                                newFormula += "{" + QString::number(connItem->id()) + ":" + QString::number(connPriceDataSet) + "}";
                                connect( connItem, static_cast<void(BillItem::*)( int, double )>(&BillItem::amountChanged), this, &Measure::updateQuantity );
                                i = i+2;
                            } else {
                                setFormulaOk = false;
                            }
                        } else {
                            BillItem * connItem = NULL;
                            if( connItemFromId ){
                                bool uintOk = false;
                                uint connItemId = formSplitted.at(i+1).toUInt( &uintOk );
                                if( uintOk ){
                                    connItem = m_d->billItem->findItemFromId( connItemId );
                                }
                            } else {
                                connItem = m_d->billItem->findItemFromProgCode( formSplitted.at(i+1) );
                            }
                            if( connItem != NULL ){
                                setFormulaOk = true;
                                m_d->connectedBillItems << qMakePair(connItem, -1);
                                newFormula += "{" + QString::number(connItem->id()) + "}";
                                connect( connItem, static_cast<void(BillItem::*)( int, double )>(&BillItem::amountChanged), this, &Measure::updateQuantity );
                                i = i+2;
                            } else {
                                setFormulaOk = false;
                            }
                        }
                    } else {
                        setFormulaOk = false;
                    }
                } else {
                    setFormulaOk = false;
                }
            } else {
                newFormula += formSplitted.at(i);
            }
            if( !setFormulaOk ) {
                newFormula = newFormulaInput;
                break;
            }
        }
    } else if( m_d->accountingBillItem != NULL ){
        // azzera l'elenco degli oggetti AccountingBillItem connessi
        for( QList< QPair<AccountingBillItem *, int> >::iterator it = m_d->connectedAccBillItems.begin(); it != m_d->connectedAccBillItems.end(); ++it ){
            if( it->second > -2 ){
                disconnect( it->first, &AccountingBillItem::amountToDiscountChanged, this, &Measure::updateQuantity );
                disconnect( it->first, &AccountingBillItem::amountNotToDiscountChanged, this, &Measure::updateQuantity );
            } else {
                disconnect( it->first, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
            }
        }
        m_d->connectedBillItems.clear();

        bool setFormulaOk = true;
        // spezziamo newForm in base ai diversi tipi di parentesi
        QList<QChar> matchStr;
        matchStr << '[' << ']' << '{' << '}';
        QStringList formSplitted = MeasurePrivate::splitQString( newFormulaInput,  matchStr );
        for( int i=0; i < formSplitted.size(); i++ ){
            if( formSplitted.at(i) == "[" ){
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "]"  ){
                        AccountingBillItem * connItem = NULL;
                        if( connItemFromId ){
                            bool uintOk = false;
                            uint connItemId = formSplitted.at(i+1).toUInt( &uintOk );
                            if( uintOk ){
                                connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                            }
                        } else {
                            connItem = m_d->accountingBillItem->findItemFromProgCode( formSplitted.at(i+1) );
                        }
                        if( connItem != NULL ){
                            setFormulaOk = true;
                            m_d->connectedAccBillItems << qMakePair(connItem, -2);
                            newFormula += "[" + QString::number(connItem->id()) + "]";
                            connect( connItem, &AccountingBillItem::quantityChanged, this, &Measure::updateQuantity );
                            i = i+2;
                        } else {
                            setFormulaOk = false;
                        }
                    } else {
                        setFormulaOk = false;
                    }
                } else {
                    setFormulaOk = false;
                }
            } else if( formSplitted.at(i) == "{" ){
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "}"  ){
                        QStringList formSplittedAmounts = formSplitted.at(i+1).split(":");
                        if( formSplittedAmounts.size() > 1 ){
                            AccountingBillItem * connItem = NULL;
                            if( connItemFromId ){
                                bool uintOk = false;
                                uint connItemId = formSplittedAmounts.at(0).toUInt( &uintOk );
                                if( uintOk ){
                                    connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                }
                            } else {
                                connItem = m_d->accountingBillItem->findItemFromProgCode( formSplittedAmounts.at(0) );
                            }
                            bool intOk = false;
                            int connPriceDataSet = formSplittedAmounts.at(1).toInt( &intOk);
                            if( connItem != NULL && intOk ){
                                setFormulaOk = true;
                                m_d->connectedAccBillItems << qMakePair(connItem, connPriceDataSet);
                                newFormula += "{" + QString::number(connItem->id()) + ":" + QString::number(connPriceDataSet) + "}";
                                connect( connItem, &AccountingBillItem::amountToDiscountChanged, this, &Measure::updateQuantity );
                                connect( connItem, &AccountingBillItem::amountNotToDiscountChanged, this, &Measure::updateQuantity );
                                i = i+2;
                            } else {
                                setFormulaOk = false;
                            }
                        } else {
                            AccountingBillItem * connItem = NULL;
                            if( connItemFromId ){
                                bool uintOk = false;
                                uint connItemId = formSplitted.at(i+1).toUInt( &uintOk );
                                if( uintOk ){
                                    connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                }
                            } else {
                                connItem = m_d->accountingBillItem->findItemFromProgCode( formSplitted.at(i+1) );
                            }
                            if( connItem != NULL ){
                                setFormulaOk = true;
                                m_d->connectedAccBillItems << qMakePair(connItem, -2);
                                newFormula += "{" + QString::number(connItem->id()) + "}";
                                connect( connItem, &AccountingBillItem::amountToDiscountChanged, this, &Measure::updateQuantity );
                                connect( connItem, &AccountingBillItem::amountNotToDiscountChanged, this, &Measure::updateQuantity );
                                i = i+2;
                            } else {
                                setFormulaOk = false;
                            }
                        }
                    } else {
                        setFormulaOk = false;
                    }
                } else {
                    setFormulaOk = false;
                }
            } else {
                newFormula += formSplitted.at(i);
            }
            if( !setFormulaOk ) {
                newFormula = newFormulaInput;
                break;
            }
        }
    } else {
        newFormula = newFormulaInput;
    }

    if( newFormula != m_d->formula ){
        m_d->formula = newFormula;
        updateQuantity();
    }
}

QString Measure::effectiveFormula() const{
    QString effFormula;
    if( m_d->billItem != NULL ){
        QList<QChar> matchStr;
        matchStr << '[' << ']' << '{' << '}';
        QStringList formSplitted = MeasurePrivate::splitQString( m_d->formula,  matchStr );
        for( int i=0; i < formSplitted.size(); i++ ){
            if( formSplitted.at(i) == "[" ){
                bool effectiveFormulaOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "]"  ){
                        bool uintOk = false;
                        uint connItemId = formSplitted.at(i+1).toUInt(&uintOk );
                        if( uintOk ){
                            BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                            if( connItem != NULL ){
                                effFormula += connItem->quantityStr();
                                effectiveFormulaOk = true;
                                i = i+2;
                            }
                        }
                    }
                }
                if( ! effectiveFormulaOk ){
                    effFormula += "0";
                }
            } else if( formSplitted.at(i) == "{" ){
                bool effectiveFormulaOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "}"  ){
                        QStringList formSplittedAmounts = formSplitted.at(i+1).split(":");
                        if( formSplittedAmounts.size() > 1 ){
                            bool uintOk = false;
                            uint connItemId = formSplittedAmounts.at(0).toUInt(&uintOk );
                            if( uintOk ){
                                BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                                bool intOk = false;
                                int connPriceDataSet = formSplittedAmounts.at(1).toInt(&intOk ) - 1;
                                if( connItem != NULL && intOk ){
                                    effFormula += connItem->amountStr( connPriceDataSet );
                                    effectiveFormulaOk = true;
                                    i = i+2;
                                }
                            }
                        } else {
                            bool uintOk = false;
                            uint connItemId = formSplitted.at(i+1).toUInt(&uintOk );
                            if( uintOk ){
                                BillItem * connItem = m_d->billItem->findItemFromId( connItemId );
                                if( connItem != NULL ){
                                    effFormula += connItem->amountStr( m_d->billItem->currentPriceDataSet() );
                                    effectiveFormulaOk = true;
                                    i = i+2;
                                }
                            }
                        }
                    }
                }
                if( ! effectiveFormulaOk ){
                    effFormula += "0";
                }
            } else {
                effFormula += formSplitted.at(i);
            }
        }
    } else if( m_d->accountingBillItem != NULL ){
        QList<QChar> matchStr;
        matchStr << '[' << ']' << '{' << '}';
        QStringList formSplitted = MeasurePrivate::splitQString( m_d->formula,  matchStr );
        for( int i=0; i < formSplitted.size(); i++ ){
            if( formSplitted.at(i) == "[" ){
                bool effectiveFormulaOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "]"  ){
                        bool uintOk = false;
                        uint connItemId = formSplitted.at(i+1).toUInt(&uintOk );
                        if( uintOk ){
                            AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                            if( connItem != NULL ){
                                effFormula += connItem->quantityStr();
                                effectiveFormulaOk = true;
                                i = i+2;
                            }
                        }
                    }
                }
                if( ! effectiveFormulaOk ){
                    effFormula += "0";
                }
            } else if( formSplitted.at(i) == "{" ){
                bool effectiveFormulaOk = false;
                if( (i+2) < formSplitted.size() ){
                    if( formSplitted.at(i+2) == "}"  ){
                        QStringList formSplittedAmounts = formSplitted.at(i+1).split(":");
                        if( formSplittedAmounts.size() > 1 ){
                            bool uintOk = false;
                            uint connItemId = formSplittedAmounts.at(0).toUInt(&uintOk );
                            if( uintOk ){
                                AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                bool intOk = false;
                                int connPriceDataSet = formSplittedAmounts.at(1).toInt(&intOk ) - 1;
                                if( connItem != NULL && intOk ){
                                    if( connPriceDataSet == 0 ){
                                        effFormula += connItem->amountToDiscountStr();
                                    } else if( connPriceDataSet == 1 ){
                                        effFormula += connItem->amountNotToDiscountStr();
                                    } else {
                                        effFormula += "0";
                                    }
                                    effectiveFormulaOk = true;
                                    i = i+2;
                                }
                            }
                        } else {
                            bool uintOk = false;
                            uint connItemId = formSplitted.at(i+1).toUInt(&uintOk );
                            if( uintOk ){
                                AccountingBillItem * connItem = m_d->accountingBillItem->findItemFromId( connItemId );
                                if( connItem != NULL ){
                                    effFormula += connItem->amountToDiscountStr();
                                    effectiveFormulaOk = true;
                                    i = i+2;
                                }
                            }
                        }
                    }
                }
                if( ! effectiveFormulaOk ){
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

void Measure::writeXml10( QXmlStreamWriter * writer ) const {
    writer->writeStartElement( "BillItemMeasure" );
    writer->writeAttribute( "comment", m_d->comment );
    QString f = effectiveFormula();
    if( m_d->parser->decimalSeparator() != "." ){
        f.replace( m_d->parser->decimalSeparator(), ".");
    }
    writer->writeAttribute( "formula", f );
    writer->writeEndElement();
}

void Measure::loadFromXml10(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "comment" ) ){
        setComment(  attrs.value( "comment").toString() );
    }
    if( attrs.hasAttribute( "formula" ) ){
        QString f = attrs.value( "formula").toString();
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( ".", m_d->parser->decimalSeparator());
        }
        setFormula( f );
    }
}

void Measure::writeXml20( QXmlStreamWriter * writer ) const {
    writer->writeStartElement( "Measure" );
    writer->writeAttribute( "comment", comment() );
    QString f = formula();
    if( m_d->parser->decimalSeparator() != "." ){
        f.replace( m_d->parser->decimalSeparator(), ".");
    }
    writer->writeAttribute( "formula", f );
    writer->writeEndElement();
}

void Measure::loadFromXmlTmp20() {
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

void Measure::loadXmlTmp20(const QXmlStreamAttributes &attrs) {
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
