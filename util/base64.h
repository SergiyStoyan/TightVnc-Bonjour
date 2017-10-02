// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
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

#ifndef _BASE64_H_
#define _BASE64_H_

#include <stdint.h>
#include <stdlib.h>

class base64
{
public:
	base64();
	~base64();
	char* encode(const unsigned char *data, size_t input_length, size_t *output_length);
	unsigned char* decode(const char *data, size_t input_length, size_t *output_length);

private:
	char decoding_table[256];
};

#endif//_BASE64_H_
