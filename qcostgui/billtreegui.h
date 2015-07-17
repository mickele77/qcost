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
#ifndef BILLTREEGUI_H
#define BILLTREEGUI_H

#include "pricelistdbwidget.h"

class Project;
class Bill;
class BillItem;
class PriceItem;
class MathParser;
class PriceFieldModel;

#include <QWidget>
#include <QModelIndex>

class BillTreeGUIPrivate;

class BillTreeGUI : public QWidget {
    Q_OBJECT
public:
    friend class BillGUI;
    explicit BillTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString *EPAFileName, Bill * b, MathParser *prs, Project *prj, QWidget *parent = 0);
    ~BillTreeGUI();
    
    BillItem * currentBillItem();
    void setBill( Bill * );

    void showEvent(QShowEvent * event);

signals:
    void currentItemChanged( BillItem * );
    void editBillItemPrice( BillItem * );

private slots:
    void addItems();
    void addChildItems();
    void removeItems();

    void changeCurrentItem( const QModelIndex & currentIndex );

    void editBillItemPrice( const QModelIndex & index);

    void setPriceList();
    void setPriceDataSet();

    void clear();
    void billTreeViewCustomMenuRequested(QPoint pos);
    void editAttributes();

    void copyToClipboard();
    void pasteFromClipboard();
    void cutToClipboard();
    void resizeColumnsToContents();

    // aggiorna etichette e importi di tutti i campi prezzo
    void updateAmountsNameValue();
    // aggiorna solo l'etichetta di un campo prezzo
    void updateAmountName( int priceField, const QString & newName );
    // aggiorna solo l'importo di un campo prezzo
    void updateAmountValue(int priceField, const QString &newVal);


private:
    BillTreeGUIPrivate * m_d;

    void populatePriceListComboBox();
    void setPriceListComboBox();
    void setPriceDataSetSpinBox();
};

#endif // BILLTREEGUI_H
