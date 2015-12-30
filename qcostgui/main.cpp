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
#include "qcostgui.h"
#include <QApplication>

#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    if( !QLocale::system().name().startsWith( "it" ) ){
        QTranslator translatorQCostGUI;
        // QString transFileName = QString("qcostgui_") + QLocale::system().name();
        QString transFileName = QString("qcostgui_en");
        if( translatorQCostGUI.load( transFileName ) ){
            app.installTranslator(&translatorQCostGUI);
        }

        QTranslator translatorLibQCost;
        // transFileName = QString("libqcost_") + QLocale::system().name();
        transFileName = QString("libqcost_en");
        if( translatorLibQCost.load( transFileName ) ){
            app.installTranslator(&translatorLibQCost);
        }
    }

    QCostGUI w;
    w.show();
    
    return app.exec();
}
