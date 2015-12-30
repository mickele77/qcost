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
#ifndef PRICELISTDBWIDGET_H
#define PRICELISTDBWIDGET_H

class MathParser;

#include <QWidget>

class PriceListDBWidgetPrivate;

class PriceListDBWidget : public QWidget {
    Q_OBJECT
public:
    enum ImportOptions{
        ImportCode,
        InheritCode,
        ImportShortDesc,
        InheritShortDesc,
        ImportLongDesc,
        InheritLongDesc,
        ImportOverheads,
        ImportProfits
    };

    enum ImportMode{
        ImportSingle,
        ImportMulti
    };

    explicit PriceListDBWidget( QMap<ImportOptions, bool> * impOptions, QString * fileNameInput, MathParser * p,
                                const QString & connectionName = QLatin1String( "defaultConnection" ),
                                QWidget *parent = 0);
    ~PriceListDBWidget();
    
    void setMode(ImportMode m);

    void importSinglePriceItemDB(QList<QPair<QString, QVariant> > *retData );
    void importMultiPriceItemDB(QList<QList<QPair<QString, QVariant> > > *itemDataList, QList<int> *hierarchy );

    void hideImportOptions();

private slots:
    void setCurrentPrice(const QModelIndex &current);
    void editDBFileName();
    void applyFilter();
    void syncImportOptions(bool isChecked);
private:
    PriceListDBWidgetPrivate * m_d;
};

#endif // PRICELISTDBWIDGET_H
