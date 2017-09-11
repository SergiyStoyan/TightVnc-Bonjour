//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#ifndef __MPEG_STREAMER_DIALOG_H_
#define __MPEG_STREAMER_DIALOG_H_

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

	void onMpegStreamerEnabledClick();
	void onMpegStreamerChange();

private:
	void initControls();

protected:
	// Configuration
	ServerConfig *m_config;
	// Controls
	CheckBox m_enableMpegStreamer;
	TextBox m_MpegStreamerDestinationPort;
	TextBox m_MpegStreamerFramerate;
	TextBox m_MpegStreamerDelayMss;
	CheckBox m_turnOffRfbVideo;
	CheckBox m_hideStreamerWindow;

	BaseDialog *m_parent;
};

#endif
