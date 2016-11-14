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

#ifndef _TOOLTIP_H_
#define _TOOLTIP_H_

#include "util/CommonHeader.h"
#include "Control.h"

//
// Abstract tooltip class
//

class Tooltip
{
public:
  Tooltip();
  virtual ~Tooltip();

  virtual void showTooltip(Control *control) = 0;

  virtual void setText(const TCHAR *text) = 0;
  virtual void setTitle(const TCHAR *caption) = 0;

  virtual void getText(StringStorage *text) const = 0;
  virtual void getTitle(StringStorage *title) const = 0;

  virtual void setIconType(int iconType) = 0;
  virtual int getIconType() const = 0;
};

#endif
