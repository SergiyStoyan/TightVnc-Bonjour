//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************


#include "gui/BaseDialog.h"
#include "gui/TextBox.h"
#include "gui/CheckBox.h"

#include "server-config-lib/Configurator.h"

class MpegStreamerConfigDialog : public BaseDialog
{
public:
	MpegStreamerConfigDialog();
	virtual ~MpegStreamerConfigDialog();

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
	void onUseWindowsUserAsBonjourServiceNameClick();
	void onBonjourTextChange();

private:
	void initControls();

protected:
	// Configuration
	ServerConfig *m_config;
	// Controls
	CheckBox m_enableBonjourService;
	CheckBox m_useWindowsUserAsBonjourServiceName;
	TextBox m_BonjourServiceName;
	TextBox m_BonjourServicePort;
	TextBox m_BonjourServiceType;

	BaseDialog *m_parent;
};

#endif
