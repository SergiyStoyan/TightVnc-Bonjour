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

	void onMpegStreamerEnabled();
	//void onMpegStreamerCapturedDisplaySet();
	void onMpegStreamerChange();

private:
	void initControls();
	void set_monitors();
	void set_area();
	void set_windows();

	struct Screen
	{
		LONG x;
		LONG y;
		LONG width;
		LONG height;
		WCHAR DeviceName[CCHDEVICENAME];
		WCHAR DeviceString[128];
	};
	typedef list<Screen*> ScreenList;
	static ScreenList screens;
	static void clear_screens();
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

	static BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam);

	// Configuration
	ServerConfig *m_config;
	// Controls
	CheckBox m_enableMpegStreamer;
	TextBox m_destinationPort;
	TextBox m_framerate;
	TextBox m_delayMss;
	CheckBox m_turnOffRfbVideo;
	CheckBox m_hideStreamerWindow;
	CheckBox m_logProcessOutput;
	ComboBox m_displays;
	TextBox m_capturedAreaLeft;
	TextBox m_capturedAreaTop;
	TextBox m_capturedAreaWidth;
	TextBox m_capturedAreaHeight;
	ComboBox m_windows;
	CheckBox m_captureDisplay;//actually it is radiobox
	CheckBox m_captureArea;//actually it is radiobox
	CheckBox m_captureWindow;//actually it is radiobox

	BaseDialog *m_parent;
};

#endif
