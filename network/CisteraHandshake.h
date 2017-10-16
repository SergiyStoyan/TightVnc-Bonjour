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

//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************



#ifndef __CISTERA_HANDSHAKE_H_INCLUDED__
#define __CISTERA_HANDSHAKE_H_INCLUDED__

#include "network/socket/SocketIPv4.h"
#include "network/socket/SocketStream.h"

class CisteraHandshake
{
public:
	
	struct clientRequest
	{
		char clientVersion[4] = "1.0";
		bool encrypt = true;
		bool mpegStream = true;
		byte mpegFramerate = 10;
		bool rfbVideo = false;
		UINT16 mpegStreamPort = 5720;
	};

	static const int serverResponse_mpegStreamAesKeySalt_SIZE = 30;

	struct serverResponse
	{
		char serverVersion[4] = "1.0";
		byte mpegStreamAesKeySalt[serverResponse_mpegStreamAesKeySalt_SIZE];
	};
};

#endif // __CISTERA_HANDSHAKE_H_INCLUDED__
