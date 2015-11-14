/*
    Copyright (C) 2014 Michele Mocciola

    This file is part of QStruct.

    QStruct is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QStruct is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QStruct.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ODTCREATOR_EXPORT_H
#define ODTCREATOR_EXPORT_H

#include <QtGlobal>

#ifdef BUILD_SHARED_WIN
    #ifdef BUILD_ODTCREATOR_LIB
        #define EXPORT_ODTCREATOR_LIB_OPT Q_DECL_EXPORT
    #else
        #define EXPORT_ODTCREATOR_LIB_OPT Q_DECL_IMPORT
    #endif
#else
    #define EXPORT_ODTCREATOR_LIB_OPT
#endif

#endif // ODTCREATOR_EXPORT_H

