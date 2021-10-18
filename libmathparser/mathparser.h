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
#ifndef MATHPARSER_H
#define MATHPARSER_H

#include "mathparser_export.h"

class QString;
class QDate;

class MathParserPrivate;

#include <QLocale>
#include <QObject>

class EXPORT_MATHPARSER_LIB_OPT MathParser : public QObject {
    Q_OBJECT
public:
    MathParser( const QLocale & loc );

    double evaluateLocal(const QString & exprInput , QString *errorMsg = 0 );
    double evaluate(const QString & exprInput , QString *errorMsg = 0 );
    QDate evaluateDate( const QString & date, QLocale::FormatType format = QLocale::NarrowFormat );

    QString	toString( const QDate & date, QLocale::FormatType format = QLocale::NarrowFormat ) const;
    QString	toString( double i, char f = 'g', int prec = 6 ) const;

    QString decimalSeparator();
    QString thousandSeparator();
    void setSeparators( const QChar & newDecSep, const QChar & newThSep );


    QString spellInt(QString numStr );
    QString spellDouble(QString num);

signals:
    void separatorsChanged( QString oldDecSep, QString newDecSep, QString oldThSep, QString newThSep );

private:
    MathParserPrivate * m_d;
};

#endif // MATHPARSER_H
