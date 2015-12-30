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
#ifndef PRICEITEMGUI_H
#define PRICEITEMGUI_H

#include "pricelistdbwidget.h"

class Project;
class Bill;
class PriceItem;
class UnitMeasure;
class MathParser;

#include <QWidget>

class PriceItemGUIPrivate;

class PriceItemGUI : public QWidget {
    Q_OBJECT
public:
    explicit PriceItemGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                           QString *EPAFileName,
                           PriceItem * pr, int curPriceDataSet,
                           MathParser * parser, Project *prj,
                           QWidget *parent = 0);
    ~PriceItemGUI();

    void clearWidgets();

signals:
    void editPriceItemAP( PriceItem * pItem, Bill * APToEdit );
    void importSinglePriceItemDB( PriceItem * item );
    void currentPriceDataSetChanged( int );

public slots:
    void setPriceItem( PriceItem * newPriceItem, int newCurPriceDataSet = 0 );
    void setCurrentPriceDataSet(int priceDataSet);
    void setPriceItemNULL();

private slots:
    void setParentLongDescriptionGUI();

    void setItemUnitMeasureFromComboBox();
    void setItemUnitMeasureFromComboBox(UnitMeasure *ump);

    void setUnitMeasure( int i );

    void insertPriceDataSet();
    void removePriceDataSet();
    void setCurrentPriceDataSet();

    void editPriceItemAP(const QModelIndex &index);
    void editPriceItemAP();

    void setPriceValueToLineEdit(int priceField, int priceCol, const QString &newVal);
    void setPriceValueFromLineEdit();

    void setPriceValueAssociateAP(bool newVal);

    void setAssociatedAPToCheckBox(int pCol, bool val);

    // quando l'utente termina la modifica del codice, sincronizza il valore del puntatore
    void setCodeFromLineEdit();

    // quando l'utente termina la modifica del nome, sincronizza il valore del puntatore
    void setShortDescriptionFromLineEdit();

    void setLongDescFromTextEdit();

    void emitImportSinglePriceItemDB();

    void updateFieldList();
    void updatePriceDataGUI();
    void updatePriceDataTable();
    void importSingePriceItemDB();

    void setOverheadsToLineEdit(int priceDataSet, const QString &newVal);
    void setOverheadsFromLineEdit();
    void setIOFRFromCheckBox();
    void setIOFRToCheckBox(int priceDataSet, bool newVal);
    void setProfitsToLineEdit(int priceDataSet, const QString &newVal);
    void setProfitsFromLineEdit();
    void setIPFRFromCheckBox();
    void setIPFRToCheckBox(int priceDataSet, bool newVal);

    void updateUnitMeasureComboBox();

private:
    PriceItemGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // PRICEGUI_H
