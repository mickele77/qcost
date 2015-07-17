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
#ifndef QTEXTFORMATUSERDEFINED_H
#define QTEXTFORMATUSERDEFINED_H

namespace QTextFormatUserDefined {
enum TableCellBorders { TableCellAllBordersStyle   = 0x100001, // stile linea definito come QTextFrameFormat::BorderStyle
                        TableCellBorderLeftStyle   = 0x100002,
                        TableCellBorderRightStyle  = 0x100003,
                        TableCellBorderTopStyle    = 0x100004,
                        TableCellBorderBottomStyle = 0x100005,
                        TableCellAllBordersWidth   = 0x100006, // spessore definito in pt
                        TableCellBorderLeftWidth   = 0x100007,
                        TableCellBorderRightWidth  = 0x100008,
                        TableCellBorderTopWidth    = 0x100009,
                        TableCellBorderBottomWidth = 0x100010,
                        TableCellAllBordersColor   = 0x100011, // colore definito come QColor
                        TableCellBorderLeftColor   = 0x100012,
                        TableCellBorderRightColor  = 0x100013,
                        TableCellBorderTopColor    = 0x100014,
                        TableCellBorderBottomColor = 0x100015};
}

#endif // QTEXTFORMATUSERDEFINED_H
