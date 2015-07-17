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
#ifndef LOADFROMTXTDIALOG_H
#define LOADFROMTXTDIALOG_H

#include "pricelistdbmodel.h"

#include <QDialog>

class LoadFromTXTDialogPrivate;

class LoadFromTXTDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoadFromTXTDialog(PriceListDBModel *m,
                               QList<PriceListDBModel::PriceColType> * pCols,
                               QTextStream * in,
                               QString * decimalSeparator,
                               QString * thousandSeparator,
                               bool *setShortDescFromLong,
                               double *overheads,
                               double *profits,
                               QLocale *l,
                               QWidget *parent = 0);
    ~LoadFromTXTDialog();
    
private:
    LoadFromTXTDialogPrivate * m_d;

private slots:
    void addComboBox();
    void delComboBox();
    void setValues();
};

#endif // LOADFROMTXTDIALOG_H
