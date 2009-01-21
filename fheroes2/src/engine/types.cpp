/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov                               *
 *   afletdinov@mail.dc.baikal.ru                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "types.h"

u32 ReadBE32(const u8 *p)
{
    return ((((u32) *p) << 24) | (((u32) *(p + 1)) << 16) | (((u32) *(p + 2)) << 8) | ((u32) *(p + 3)));
}

u32 ReadLE32(const u8 *p)
{
    return ((((u32) *(p + 3)) << 24) | (((u32) *(p + 2)) << 16) | (((u32) *(p + 1)) << 8) | ((u32) *p));
}

u16 ReadBE16(const u8 *p)
{
    return((((u16) *p) << 8) | ((u16) *(p + 1)));
}

u16 ReadLE16(const u8 *p)
{
    return((((u16) *(p + 1)) << 8) | ((u16) *p));
}

void WriteBE32(char *p, u32 x)
{
    *p = static_cast<char>(x >> 24);
    *(p + 1) = static_cast<char>((x & 0x00FF0000) >> 16);
    *(p + 2) = static_cast<char>((x & 0x0000FF00) >> 8);
    *(p + 3) = static_cast<char>(x & 0x000000FF);
}

void WriteBE16(char *p, u16 x)
{
    *p = static_cast<char>(x >> 8);
    *(p + 1) = static_cast<char>(x & 0x00FF);
}