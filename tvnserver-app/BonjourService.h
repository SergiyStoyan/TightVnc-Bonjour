
#ifndef _BONJOUR_SERVICE_H_
#define _BONJOUR_SERVICE_H_

#include "log-server/LogServer.h"
#include "log-server/ClientLogger.h"
#include "win-system/Service.h"

#include "thread/Thread.h"
#include "WinServiceEvents.h"
#include "NewConnectionEvents.h"

class BonjourService : public Service//,
                   //public TvnServerListener,
                   //private LogInitListener
{
  /**
   * Creates object.
   */
	BonjourService(WinServiceEvents *winServiceEvents,
             NewConnectionEvents *newConnectionEvents);
  /**
   * Deletes object.
   */
  virtual ~BonjourService();

  /**
   * Installs tvnserver service.
   * @throws SystemException on fail.
   */
  static void install() throw(SystemException);
  /**
   * Stops and removes tvnserver service.
   * @throws SystemException when failed to remove service.
   */
  static void remove() throw(SystemException);
  /**
   * Reinstalls tvnserver service (combite call of remove and install methods).
   * @remark ignores if remove call throws exception.
   * @throws SystemException when fail to install service.
   */
  static void reinstall() throw(SystemException);
  /**
   * Starts tvnserver service.
   * @param waitCompletion if true, wait until the status becomes
   *   SERVICE_RUNNING.
   * @throws SystemException on fail.
   */
  static void start(bool waitCompletion = false) throw(SystemException);
  /**
   * Stopps tvnserver service.
   * @param waitCompletion if true, wait until the status becomes
   *   SERVICE_STOPPED.
   * @throws SystemException on fail.
   */
  static void stop(bool waitCompletion = false) throw(SystemException);

protected:

	/**
	* Inherited from superclass.
	* Starts tvnserver execution.
	* @throws SystemException when failed to start.
	*/
	virtual void onStart() throw(SystemException);

  LogServer m_logServer;
  ClientLogger m_clientLogger;

  WinServiceEvents *m_winServiceEvents;
  NewConnectionEvents *m_newConnectionEvents;
};

#endif
