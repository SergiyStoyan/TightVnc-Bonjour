//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef _BONJOUR_SERVICE_H_
#define _BONJOUR_SERVICE_H_

class BonjourService 
{
public:
	BonjourService();
   ~BonjourService();

   static void Start();
   static void Stop();
};

#endif
