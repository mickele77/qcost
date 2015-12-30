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
#ifndef PRICELISTPRINTERGUI_H
#define PRICELISTPRINTERGUI_H

class PriceFieldModel;

#include "pricelistprinter.h"

#include <QDialog>

class PriceListPrinterGUIPrivate;

class PriceListPrinterGUI : public QDialog {
    Q_OBJECT

public:
    /**
    @param pData Le opzioni di scrittura dell'elenco prezzi
    @param priceColNumber restituisce la colonna da stampare
    @param priceColNumber riceve il numero di colonne prezzo presenti */
    explicit PriceListPrinterGUI( PriceListPrinter::PrintPriceItemsOption * prItemsOption,
                                  QList<int> * printFields,
                                  bool * printNumLetters,
                                  double *pWidth,
                                  double *pHeight,
                                  Qt::Orientation *pOrient,
                                  int * printPriceDataSet,
                                  bool *printPriceList,
                                  bool *printPriceAP,
                                  bool *APgroupPrAm,
                                  int priceDataSetCount,
                                  PriceFieldModel * pfm,
                                  QWidget *parent = 0);
    ~PriceListPrinterGUI();

private slots:
    void setPrintData();
    void insertPriceFieldComboBox();
    void removePriceFieldComboBox();
private:
    PriceListPrinterGUIPrivate * m_d;
};

#endif // PRICELISTPRINTERGUI_H
