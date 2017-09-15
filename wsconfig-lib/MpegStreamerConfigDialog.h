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
#include "gui/ComboBox.h"

#include "server-config-lib/Configurator.h"

#include <list>

class MpegStreamerConfigDialog : public BaseDialog
{
public:
	struct Screen
	{
		LONG x;
		LONG y;
		LONG width;
		LONG height;
		//StringStorage name;
	};
	typedef list<Screen *> ScreenList; 
	static MpegStreamerConfigDialog::ScreenList Screens;
	
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
	void set_monitors();
	//BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

	// Configuration
	ServerConfig *m_config;
	// Controls
	CheckBox m_enableMpegStreamer;
	TextBox m_MpegStreamerDestinationPort;
	TextBox m_MpegStreamerFramerate;
	TextBox m_MpegStreamerDelayMss;
	CheckBox m_turnOffRfbVideo;
	CheckBox m_hideStreamerWindow;
	ComboBox m_desktops;
	ComboBox m_windows;

	BaseDialog *m_parent;
};

#endif
