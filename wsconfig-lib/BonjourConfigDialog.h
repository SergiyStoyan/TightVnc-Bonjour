//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef _BONJOUR_DIALOG_H_
#define _BONJOUR_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/TextBox.h"
#include "gui/CheckBox.h"

#include "server-config-lib/Configurator.h"

class BonjourConfigDialog : public BaseDialog
{
public:
	BonjourConfigDialog();
  virtual ~BonjourConfigDialog();

  void setParentDialog(BaseDialog *dialog);

  bool validateInput();
  void updateUI();
  void apply();

protected:

  //
  // Inherited from BaseDialog.
  //

  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onNotify(UINT controlID, LPARAM data) { return TRUE; }
  virtual BOOL onDestroy() { return TRUE; }

  //
  // Controls event handlers.
  //

  void onBonjourEnabledClick();
  void onBonjourAgentNameChange();

private:
  void initControls();

protected:
	// Configuration
	ServerConfig *m_config;
	// Controls
  CheckBox m_enableBonjourService;
  TextBox m_BonjourAgentName;

  BaseDialog *m_parent;
};

#endif
