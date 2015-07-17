/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2014 Mocciola Michele

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

#include <QString>
#include <QLocale>
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
    Q_DECLARE_TR_FUNCTIONS()
public:
    MathParserPrivate( const QLocale & loc ):
        locale(loc),
        decimalSeparator(loc.decimalPoint()),
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
    };
    QLocale locale;
    // separatore decimale
    QChar decimalSeparator;
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

    double evaluate(const QString &exprInput, QString * errorMsg){
        QString expr = exprInput;
        expr.remove(" ");

        for( int i=0; i < operatorsChar.size(); ++i){
            // cerca l'operatore i-esimo
            QChar oper = operatorsChar.at(i);

            // cerca l'ultima occorrenza dell'operatore i-esimo
            int pos = lastIndexOfOP( expr, oper, expr.size() - 1 );
            while( pos > - 1 ){
                if( isOperator(expr, oper, pos) ) {
                    // Divide l'espressione in due parti
                    QString leftPart = expr.left( pos );
                    QString rightPart;
                    if( (pos+1) < expr.size() ){
                        rightPart = expr.mid( pos+1 );
                    }
                    if( leftPart.isEmpty() ){
                        if( errorMsg ){
                            errorMsg->append( trUtf8("Manca un valore prima dell'operatore %1.\n").arg(oper) );
                        }
                        return 0.0;
                    }
                    if( rightPart.isEmpty() ){
                        if( errorMsg ){
                            errorMsg->append( trUtf8("Manca un valore dopo l'operatore %1.\n").arg(oper) );
                        }
                        return 0.0;
                    }

                    double leftValue = evaluate( leftPart, errorMsg );
                    double rightValue = evaluate( rightPart, errorMsg );

                    // Ora esegue il calcolo tra le due parti
                    switch( oper.toLatin1() ){
                    case '-':{
                        return leftValue - rightValue;
                        break; }
                    case '+':{
                        return leftValue + rightValue;
                        break; }
                    case '/':{
                        if( rightValue == 0.0 ){
                            if( errorMsg ){
                                errorMsg->append( trUtf8("Divisione per zero.\n") );
                            }
                            return 0.0;
                        }
                        return leftValue / rightValue;
                        break; }
                    case '*':{
                        return leftValue * rightValue;
                        break; }
                    case '^':{
                        return pow( leftValue, rightValue );
                        break; }
                    case 'E':{
                        return leftValue * pow(10.0, rightValue );
                        break; }
                    }
                }
                // cerca se c'è un altro operatore prima di oper
                if( pos>-1 ){
                    pos = lastIndexOfOP( expr, oper, pos-1 );
                }
            }
        }

        // controlla se l'espressione è una funzione, tipo "sin(2+3)"
        int pos = expr.indexOf( '(' );
        if( (pos > -1) && (expr.at(expr.size()-1)==')') ){
            // estrae il nome della funzione
            QString function;
            if( pos > 0 ){
                function = expr.left(pos);
            }

            // estrae l'espressione tra parentesi e la calcola
            QString argumentStr;
            if( (pos+1) < expr.size() ){
                argumentStr = expr.mid( pos+1, expr.size()-(pos+2));
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
                errorMsg->append( trUtf8("Errore: funzione %1 sconosciuta.\n").arg(function));
            }
            return 0.0;
        }

        // Se siamo arrivato fin qui vuol dire che la stringa è un numero.
        // Restituiamo il valore numerico
        if( isValue(expr) ){
            bool ok = false;
            double ret = locale.toDouble( expr, &ok );
            if( !ok ){
                if( errorMsg ){
                    errorMsg->append( trUtf8("Errore nella conversione del valore %1.").arg(expr));
                }
            }
            return ret;
        }

        // controlla se il valore è una costante
        if( constantsMap.contains(expr) ){
            return constantsMap.value( expr );
        }

        // se siamo arrivati fin qui qualcosa è andata storto
        if( errorMsg ){
            errorMsg->append( trUtf8("Errore di sintassi nella parte %1.\n").arg(expr) );
        }
        return 0.0;
    };
};

MathParser::MathParser( const QLocale & loc ):
    m_d( new MathParserPrivate( loc ) ){
}

double MathParser::evaluate(const QString &exprInput, QString *errorMsg) {
    QString expr = exprInput;
    expr = expr.remove(' ');
    expr = expr.toUpper();
    return m_d->evaluate( expr, errorMsg);
}

QString MathParser::toString(double i, char f, int prec) const {
    return m_d->locale.toString( i, f, prec );
}

QString MathParser::decimalSeparator() {
    return m_d->decimalSeparator;
}
