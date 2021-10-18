/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2016 Mocciola Michele

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include "mathparser.h"

#include <QDate>
#include <QString>
#include <QMap>
#include <QCoreApplication>
#include <cmath>

double plus( double v ){
    return v;
}

double minus( double v ){
    return -v;
}

class MathParserPrivate{
    Q_DECLARE_TR_FUNCTIONS( MathParserPrivate )
public:
    MathParserPrivate( const QLocale & loc ):
        locale(loc),
        decimalSeparator(loc.decimalPoint()),
        thousandSeparator(loc.groupSeparator()),
        numChar( QString("1234567890%1").arg(decimalSeparator) ),
        numCharMinus( numChar + "-" ),
        numCharMinusPlus( numChar + "-+" ),
        operatorsChar("+-*/^E"){

        // costanti
        constantsMap.insert( "PI", 3.141592654 );
        constantsMap.insert( "E", 2.718281828 );
        // aggiungere qui altre costanti

        // funzioni aritmetiche
        functionsMap.insert( QString("-"), &minus);
        functionsMap.insert( QString("+"), &plus);
        functionsMap.insert( QString("ABS"), &fabs);
        functionsMap.insert( QString("SQRT"), &sqrt);
        functionsMap.insert( QString("LOG"), &log);
        functionsMap.insert( QString("LOG10"), &log10);
        functionsMap.insert( QString("EXP"), &exp);
        functionsMap.insert( QString("ROUND"), &round);
        functionsMap.insert( QString("CEIL"), &ceil);
        functionsMap.insert( QString("FLOOR"), &floor);

        // funzioni trigonometriche
        functionsMap.insert( QString("SIN"), &sin);
        functionsMap.insert( QString("COS"), &cos);
        functionsMap.insert( QString("TAN"), &tan);
        functionsMap.insert( QString("ASIN"), &asin);
        functionsMap.insert( QString("ACOS"), &acos);
        functionsMap.insert( QString("ATAN"), &atan);

        // funzioni iperboliche
        functionsMap.insert( QString("SINH"), &sinh);
        functionsMap.insert( QString("COSH"), &cosh);
        functionsMap.insert( QString("TANH"), &tanh);

        // aggiungere qui altre funzioni
    }
    QLocale locale;
    // separatore decimale
    QChar decimalSeparator;
    // separatore migliaia
    QChar thousandSeparator;
    // caratteri numerici
    QString numChar;
    // caratteri numerici con segno -
    QString numCharMinus;
    // caratteri numerici con segno - e +
    QString numCharMinusPlus;
    // operatori
    QString operatorsChar;
    // elenco costanti
    QMap<QString, double> constantsMap;
    // elenco funzioni
    QMap<QString, double(*)(double)> functionsMap;

    // Ultimo indice all'esterno delle parentesi
    int lastIndexOfOP(QString source, QChar search, int start ){

        int parOpen = 0, parClose = 0;

        if( start >= source.size() ){
            start = source.size() - 1;
        }

        for( int i=start; i >= 0; --i){
            if( (source.at(i) == search) && ( parOpen == parClose ) ){
                return i;
            }
            if( source.at(i) == '(' ){
                parOpen++;
            } else if( source.at(i) == ')' ){
                parClose++;
            }
        }
        return -1;
    }

    // controlla l'operatore alla posizione n
    bool isOperator( const QString & expr, QChar op, int n){
        if( op=='+'){
            if( n > 1 ){
                if( expr.at(n-1).toUpper() =='E'){
                    if( numChar.contains(expr.at(n-1) )){
                        return false;
                    }
                }
            }
        }

        if( op=='-' ){
            if( n == 0 ){
                return false;
            } else {
                if( n > 1 ){
                    // controlla il meno unario (ad esempio 2*-3  o  2.5E-6)
                    // controlla il carattere davanti al meno
                    QChar c = expr.at(n-1);
                    if( operatorsChar.contains( c )){
                        // meno unario
                        return false;
                    }
                    if( (c.toUpper() == 'E') ){
                        if( numChar.contains(expr.at(n-1) )){
                            return false;
                        }
                    }
                }
            }
        }

        if( op=='E' ){
            // se E è il primo o l'ultimo caratter,e allora non è un operatore ma la variable E
            if( (n == 0) || (n == expr.size()-1) ){
                return false;
            }
            // se a sinistra o a destra di E non ci sono numeri, allora non è un operatore ma la variabile E
            if( n > 1 ){
                if( !numChar.contains( expr.at(n-1) ) ){
                    return false;
                }
            }
            if( n < expr.size() ){
                if( !numCharMinusPlus.contains( expr.at(n+1) ) ){
                    return false;
                }
            }
        }

        return true;
    }

    // Controlla se il valore è accettabile
    bool isValue(const QString & value){
        for( int i=0; i < value.size(); ++i ){
            // controlla i caratteri uno ad uno

            QChar c = value.at(i);
            if( !numCharMinus.contains(c)){
                return false;
            }

            if( (c == decimalSeparator) && ((i+1) < value.size())){
                if( value.mid(i+1).contains(decimalSeparator) ){
                    return false;
                }
            }

            if( (c=='-') && (i!=0)){
                return false;
            }
        }
        return true;
    }

    // Prende come input la formula con separatore decimale definito da utente
    double evaluateLocal(const QString &exprInput, QString * errorMsg){
        QString expr = exprInput;
        expr.remove(" ");
        expr = expr.toUpper();
        expr.remove( thousandSeparator );
        expr.replace( decimalSeparator, ".");
        return evaluate( expr, errorMsg );
    }

    // Prende come input la formula con separatore decimale "."
    double evaluate(const QString &exprInput, QString * errorMsg){
        QString expr = exprInput;
        expr.remove(" ");
        expr = expr.toUpper();

        if( exprInput.size() == 0 ){
            return 0.0;
        }

        for( int i=0; i < operatorsChar.size(); ++i){
            // cerca l'operatore i-esimo
            QChar oper = operatorsChar.at(i);

            // cerca l'ultima occorrenza dell'operatore i-esimo
            int pos = lastIndexOfOP( exprInput, oper, exprInput.size() - 1 );
            while( pos > - 1 ){
                if( isOperator(exprInput, oper, pos) ) {
                    // Divide l'espressione in due parti
                    QString leftPart = exprInput.left( pos );
                    QString rightPart;
                    if( (pos+1) < exprInput.size() ){
                        rightPart = exprInput.mid( pos+1 );
                    }
                    if( leftPart.isEmpty() ){
                        if( errorMsg ){
                            errorMsg->append( tr("Manca un valore prima dell'operatore %1.\n").arg(oper) );
                        }
                        return 0.0;
                    }
                    if( rightPart.isEmpty() ){
                        if( errorMsg ){
                            errorMsg->append( tr("Manca un valore dopo l'operatore %1.\n").arg(oper) );
                        }
                        return 0.0;
                    }

                    double leftValue = evaluate( leftPart, errorMsg );
                    double rightValue = evaluate( rightPart, errorMsg );

                    // Ora esegue il calcolo tra le due parti
                    switch( oper.toLatin1() ){
                    case '-':{
                        return leftValue - rightValue; }
                    case '+':{
                        return leftValue + rightValue; }
                    case '/':{
                        if( rightValue == 0.0 ){
                            if( errorMsg ){
                                errorMsg->append( tr("Divisione per zero.\n") );
                            }
                            return 0.0;
                        }
                        return leftValue / rightValue; }
                    case '*':{
                        return leftValue * rightValue; }
                    case '^':{
                        return pow( leftValue, rightValue ); }
                    case 'E':{
                        return leftValue * pow(10.0, rightValue ); }
                    }
                }
                // cerca se c'è un altro operatore prima di oper
                if( pos>-1 ){
                    pos = lastIndexOfOP( exprInput, oper, pos-1 );
                }
            }
        }

        // controlla se l'espressione è una funzione, tipo "sin(2+3)"
        int pos = exprInput.indexOf( '(' );
        if( (pos > -1) && (exprInput.at(exprInput.size()-1)==')') ){
            // estrae il nome della funzione
            QString function;
            if( pos > 0 ){
                function = exprInput.left(pos);
            }

            // estrae l'espressione tra parentesi e la calcola
            QString argumentStr;
            if( (pos+1) < exprInput.size() ){
                argumentStr = exprInput.mid( pos+1, exprInput.size()-(pos+2));
            }
            double argumentValue = evaluate(argumentStr, errorMsg);

            if( function.isEmpty() ){
                return argumentValue;
            }
            // valuta la funzione
            if( functionsMap.contains(function) ){
                return (*(functionsMap.value(function)))( argumentValue );
            }
            // non abbiamo trovato la funzione: errore!
            if( errorMsg ){
                errorMsg->append( tr("Errore: funzione %1 sconosciuta.\n").arg(function));
            }
            return 0.0;
        }

        // Se siamo arrivati fin qui la stringa è un numero: restituiamo il valore numerico
        if( isValue(exprInput) ){
            bool ok = false;
            double ret = exprInput.toDouble( &ok );
            if( !ok ){
                if( errorMsg ){
                    errorMsg->append( tr("Errore nella conversione del valore %1.").arg(exprInput));
                }
            }
            return ret;
        }

        // controlla se il valore è una costante
        if( constantsMap.contains(exprInput) ){
            return constantsMap.value( exprInput );
        }

        // se siamo arrivati fin qui qualcosa è andata storto
        if( errorMsg ){
            errorMsg->append( tr("Errore di sintassi nella parte %1.\n").arg(exprInput) );
        }
        return 0.0;
    }
};

MathParser::MathParser( const QLocale & loc ):
    QObject(),
    m_d( new MathParserPrivate( loc ) ){
}

double MathParser::evaluateLocal(const QString &exprInput, QString *errorMsg) {
    return m_d->evaluateLocal( exprInput, errorMsg);
}

double MathParser::evaluate(const QString &exprInput, QString *errorMsg) {
    return m_d->evaluate( exprInput, errorMsg);
}

QDate MathParser::evaluateDate(const QString &date, QLocale::FormatType format) {
    return m_d->locale.toDate(date, format );
}

QString MathParser::toString(const QDate &date, QLocale::FormatType format) const {
    return m_d->locale.toString( date, format );
}

QString MathParser::toString(double i, char f, int prec) const {
    QString string = QString::number( i, f, prec );
    int dotIndex = string.indexOf( '.' );
    string.replace('.', m_d->decimalSeparator );
    if( dotIndex == -1 ) {
        dotIndex = string.size();
    }
    for( int i = dotIndex - 3; i > 0; i-= 3 ){
        string.insert( i, m_d->thousandSeparator );
    }
    return string;
    // return m_d->locale.toString( i, f, prec);
}

QString MathParser::decimalSeparator() {
    return m_d->decimalSeparator;
}

QString MathParser::thousandSeparator() {
    return m_d->thousandSeparator;
}

void MathParser::setSeparators(const QChar &newDecSep, const QChar &newThSep) {
    if( newDecSep != newThSep ) {
        QChar oldDecSep;
        QChar oldThSep;
        if( newDecSep != m_d->decimalSeparator ) {
            oldDecSep = m_d->decimalSeparator;
            m_d->decimalSeparator = newDecSep;
        }
        if( newThSep != m_d->thousandSeparator ) {
            oldThSep = m_d->thousandSeparator;
            m_d->thousandSeparator = newThSep;
        }
        if( !(oldDecSep.isNull()) || !(oldThSep.isNull()) ){
            emit separatorsChanged( oldDecSep, newDecSep, oldThSep, newThSep );
        }
    }
}

QString MathParser::spellInt(QString numStr) {
    QList<QString> oneDigitNums;
    oneDigitNums << "" << "uno" << "due" << "tre" << "quattro" << "cinque" << "sei" << "sette" << "otto" << "nove";

    QList<QString> teenNums;
    teenNums << "dieci" << "undici" << "dodici" << "tredici" << "quattordici" << "quindici" << "sedici" << "diciassette" << "diciotto" << "diciannove";

    QList<QString> tensNums;
    tensNums << "" << teenNums.at(0) << "venti" << "trenta" << "quaranta" << "cinquanta" << "sessanta" << "settanta" << "ottanta" << "novanta";

    QList<QString> hundredNums;
    hundredNums << "cent" << "cento";

    QList<QString> thousandDigitsFirst;
    thousandDigitsFirst << "" << "mille" << "milione" << "miliardo" << "bilione" << "biliardo";
    QList<QString> thousandDigitsSec;
    thousandDigitsSec << "" << "mila" << "milioni" << "miliardi" << "bilioni" << "biliardi";
    QList< QList<QString> > thounsandNums;
    thounsandNums << thousandDigitsFirst << thousandDigitsSec;

    QList<QString> text;
    text << "" << "" << "";
    QList<int> digit;
    digit << 0 << 0 << 0;

    QString result;

    bool ok = false;
    int num = numStr.toInt( &ok );
    bool isPositive = (num >= 0);
    if( !isPositive ){
        num = -num;
    }

    if( !ok ) {
        return "zero";
    }

    switch( numStr.length() % 3 ) {
    case 1:	numStr	= "00" + numStr;
        break;
    case 2:	numStr	= "0" + numStr;
        break;
    }
    int numLen = numStr.length();

    if ( numLen > (6 * 3) ){
        return QString("*** Errore ***");
    } if( num == 0 ) {
        return "zero";
    }

    int sect = 0;
    while( (sect + 1) * 3 <= numLen ) {
        QString subNumStr = numStr.mid(((numLen - 1) - ((sect + 1) * 3)) + 1, 3);
        if( subNumStr != "000" ) {
            int subNum = subNumStr.toInt();
            digit[0] = subNumStr.mid(0, 1).toInt();
            digit[1] = subNumStr.mid(1, 1).toInt();
            digit[2] = subNumStr.mid(2, 1).toInt();
            int twoDigits = digit[1] * 10 + digit[2];
            if( twoDigits < 10 ) {
                text[2]	= oneDigitNums[digit[2]];
                text[1]	= "";
            } else if( twoDigits < 20 ) {
                text[2]	= "";
                text[1]	= teenNums[twoDigits - 10];
            } else {
                // ventitre => ventitrè
                if( sect == 0 && digit[2] == 3 ) {
                    text[2]	= QObject::tr("trè");
                } else {
                    text[2]	= oneDigitNums[digit[2]];
                }
                // novantaotto => novantotto
                if( digit[2] == 1 || digit[2] == 8 ) {
                    text[1]	= tensNums[digit[1]].mid(0, tensNums[digit[1]].length() -1);
                } else {
                    text[1]	= tensNums[digit[1]];
                }
            }
            if( digit[0] == 0 ) {
                text[0]	= "";
            } else {
                int IDcent = 0;
                // centoottanta => centottanta
                if( (digit[1] == 8) || (digit[1] == 0 && digit[2] == 8) ) {
                    IDcent	= 0;
                } else {
                    IDcent	= 1;
                }
                if( digit[0] != 1 ) {
                    text[0]	= oneDigitNums[digit[0]] + hundredNums[IDcent];
                } else {
                    text[0]	= hundredNums[IDcent];
                }
            }

            // unomille => mille
            // miliardo => unmiliardo
            if( subNum == 1 && sect != 0 ) {
                if( sect >= 2 ) {
                    result = "un" + thounsandNums[0][sect] + result;
                } else {
                    result = thounsandNums[0][sect] + result;
                }
            } else {
                result = text[0] + text[1] + text[2] + thounsandNums[1][sect] + result;
            }
        }
        sect++;
    }

    if( !isPositive ){
        result = QString("%1 ").arg( QObject::tr("meno")) + result;
    }

    return result;
}

QString MathParser::spellDouble( QString num ){
    QStringList numSplit = num.split( m_d->decimalSeparator );
    if( numSplit.size() > 1 ){
        return spellInt(numSplit.at(0)) + "/" + numSplit.at(1);
    } else {
        return spellInt(num);
    }
}
