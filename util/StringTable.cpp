// Copyright (C) 2009,2010,2016 Cistera.com.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the CisteraVNC software.  Please visit our Web site:
//
//                       http://www.cistera.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "StringTable.h"
#include "ResourceLoader.h"

map<UINT, StringStorage> StringTable::_cache;

StringTable::StringTable()
{
}

const TCHAR *StringTable::getString(UINT id)
{
  if (_cache.find(id) == _cache.end()) {
    StringStorage string;
    if (ResourceLoader::getInstance()->loadString(id, &string)) {
      _cache[id] = string;
    } else {
      return _T("Requested string from StringTable cannot be received");
    }
  }
  return (*_cache.find(id)).second.getString();
}
