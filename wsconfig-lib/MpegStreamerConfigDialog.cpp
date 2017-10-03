//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "MpegStreamerConfigDialog.h"
#include "ConfigDialog.h"
#include "tvnserver/resource.h"
#include "util/CommonInputValidation.h"
#include "tvnserver-app/NamingDefs.h"
#include "win-system/Process.h"

MpegStreamerConfigDialog::MpegStreamerConfigDialog()
: BaseDialog(IDD_CONFIG_MPEG_STREAMER_PAGE), m_parent(NULL)
{
}

MpegStreamerConfigDialog::~MpegStreamerConfigDialog()
{
	clear_screens();
}

void MpegStreamerConfigDialog::setParentDialog(BaseDialog *dialog)
{
  m_parent = dialog;
}

void MpegStreamerConfigDialog::initControls()
{
	HWND dialogHwnd = m_ctrlThis.getWindow();

	m_displays.setWindow(GetDlgItem(dialogHwnd, IDC_COMBO_MPEG_STREAMER_MONOTORS));
	m_capturedAreaLeft.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_LEFT));
	m_capturedAreaTop.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_TOP));
	m_capturedAreaWidth.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_WIDTH));
	m_capturedAreaHeight.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_HEIGHT));
	m_windows.setWindow(GetDlgItem(dialogHwnd, IDC_COMBO_MPEG_STREAMER_WINDOWS));
	m_captureDisplay.setWindow(GetDlgItem(dialogHwnd, IDC_RADIO_MPEG_STREAMER_MONITOR));
	m_captureArea.setWindow(GetDlgItem(dialogHwnd, IDC_RADIO_MPEG_STREAMER_AREA));
	m_captureWindow.setWindow(GetDlgItem(dialogHwnd, IDC_RADIO_MPEG_STREAMER_WINDOW));
	m_logProcessOutput.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_WRITE_OUTPUT2FILE));
	m_hideProcessWindow.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_HIDE_WINDOW));

	m_openLogDir.setWindow(GetDlgItem(dialogHwnd, IDC_IDC_MPEG_STREAMER_OPEN_LOG_DIR_BUTTON));
}

BOOL MpegStreamerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_MPEG_STREAMER_WRITE_OUTPUT2FILE:
	case IDC_MPEG_STREAMER_HIDE_WINDOW:
	case IDC_RADIO_MPEG_STREAMER_MONITOR:
	case IDC_RADIO_MPEG_STREAMER_AREA:
	case IDC_RADIO_MPEG_STREAMER_WINDOW:
		if (notificationID == BN_CLICKED)
			onMpegStreamerEnabled();
		break;
	case IDC_MPEG_STREAMER_FRAMERATE:
	case IDC_MPEG_STREAMER_AREA_LEFT:
	case IDC_MPEG_STREAMER_AREA_TOP:
	case IDC_MPEG_STREAMER_AREA_WIDTH:
	case IDC_MPEG_STREAMER_AREA_HEIGHT:
		if (notificationID == EN_UPDATE)
			onMpegStreamerChange();
		break;
	case IDC_COMBO_MPEG_STREAMER_MONOTORS:
	case IDC_COMBO_MPEG_STREAMER_WINDOWS:
		if (notificationID == CBN_SELCHANGE)
		{
			/*StringStorage ss;
			ss.format(_T("%d"), notificationID);
			MessageBox(m_ctrlThis.getWindow(), ss.getString(), _T("222"), MB_ICONSTOP | MB_OK);*/
			onMpegStreamerChange();
		}
		break;
	case IDC_IDC_MPEG_STREAMER_OPEN_LOG_DIR_BUTTON:
		if (notificationID == BN_CLICKED)
			onOpenLogDir();
		break;
	}
	return TRUE;
}

BOOL MpegStreamerConfigDialog::onInitDialog()
{
	m_config = Configurator::getInstance()->getServerConfig();

	initControls();
	updateUI();

  return TRUE;
}

void MpegStreamerConfigDialog::onMpegStreamerEnabled()
{
	m_displays.setEnabled(m_captureDisplay.isChecked());

	m_capturedAreaLeft.setEnabled(m_captureArea.isChecked());
	m_capturedAreaTop.setEnabled(m_captureArea.isChecked());
	m_capturedAreaWidth.setEnabled(m_captureArea.isChecked());
	m_capturedAreaHeight.setEnabled(m_captureArea.isChecked());

	m_windows.setEnabled(m_captureWindow.isChecked());

	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void MpegStreamerConfigDialog::onMpegStreamerChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void MpegStreamerConfigDialog::onOpenLogDir()
{
	StringStorage logDir;
	m_config->getLogFileDir(&logDir);
	StringStorage command;
	command.format(_T("explorer /select,%s\\%s.log"), logDir.getString(), LogNames::SERVER_LOG_FILE_STUB_NAME);
	Process explorer(command.getString());
	try
	{
		explorer.start();
	}
	catch (Exception &e)
	{
		MessageBox(m_ctrlThis.getWindow(),
			e.getMessage(),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
	}
}

bool MpegStreamerConfigDialog::validateInput()
{
	StringStorage ss;
	long i;
		
	if (m_captureDisplay.isChecked())
	{
		if (m_displays.getSelectedItemIndex() < 0)
		{
			MessageBox(m_ctrlThis.getWindow(), StringTable::getString(IDS_MPEG_STREAMER_NO_DISPLAY_SELECTED), StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
			return false;
		}
	}
	else if (m_captureArea.isChecked())
	{
		m_capturedAreaLeft.getText(&ss);
		if (!CommonInputValidation::parseNumber(&ss, &i) || i < 0) {
			MessageBox(m_ctrlThis.getWindow(),
				StringTable::getString(IDS_SET_MPEG_STREAMER_AREA_ERROR),
				StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
			return false;
		}
		m_capturedAreaTop.getText(&ss);
		if (!CommonInputValidation::parseNumber(&ss, &i) || i < 0) {
			MessageBox(m_ctrlThis.getWindow(),
				StringTable::getString(IDS_SET_MPEG_STREAMER_AREA_ERROR),
				StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
			return false;
		}
		m_capturedAreaWidth.getText(&ss);
		if (!CommonInputValidation::parseNumber(&ss, &i) || i < 0) {
			MessageBox(m_ctrlThis.getWindow(),
				StringTable::getString(IDS_SET_MPEG_STREAMER_AREA_ERROR),
				StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
			return false;
		}
		m_capturedAreaHeight.getText(&ss);
		if (!CommonInputValidation::parseNumber(&ss, &i) || i < 0) {
			MessageBox(m_ctrlThis.getWindow(),
				StringTable::getString(IDS_SET_MPEG_STREAMER_AREA_ERROR),
				StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
			return false;
		}
	}
	else if (m_captureWindow.isChecked())
	{
		if (m_windows.getSelectedItemIndex() < 0)
		{
			MessageBox(m_ctrlThis.getWindow(), StringTable::getString(IDS_MPEG_STREAMER_NO_WINDOW_SELECTED), StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
			return false;
		}
	}
	else
		MessageBox(m_ctrlThis.getWindow(), StringTable::getString(IDS_MPEG_STREAMER_NO_MODE_SELECTED), StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);

	return true;
}

void MpegStreamerConfigDialog::updateUI()
{	
	m_logProcessOutput.check(m_config->logMpegStreamerProcessOutput());
	m_hideProcessWindow.check(m_config->hideMpegStreamerProcessWidnow());

	set_monitors();

	set_area();

	set_windows();

	switch (m_config->getMpegStreamerCaptureMode())
	{
	case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_DISPLAY:
		m_captureDisplay.check(true);
		break;
	case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_AREA:
		m_captureArea.check(true);
		break;
	case ServerConfig::MPEG_STREAMER_CAPTURE_MODE_WINDOW:
		m_captureWindow.check(true);
		break;
	default:
		m_captureDisplay.check(true);
		break;
	}
	onMpegStreamerEnabled();
}

void MpegStreamerConfigDialog::set_monitors() 
{
	clear_screens();

	//getting dimensions of the monitors
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&screens);

	//getting names of real monitors
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	for (ScreenList::iterator i = screens.begin(); i != screens.end(); i++)
	{
		Screen* s = (*i);
		EnumDisplayDevices(s->DeviceName, 0, &dd, 0);
		//wcsncpy(s->DeviceName, dd.DeviceName, sizeof(s->DeviceName) / sizeof(WCHAR));//this name is more detailed and so cannot be used when seek by EnumDisplayMonitors
		wcsncpy(s->DeviceString, dd.DeviceString, sizeof(s->DeviceString) / sizeof(WCHAR));
	}

	//fill dropdown list
	StringStorage ss;
	for (ScreenList::iterator i = screens.begin(); i != screens.end(); i++)
	{
		Screen* s = (*i);
		ss.format(_T("%s (%dx%d)"), s->DeviceString, s->width, s->height);
		m_displays.addItem(ss.getString(), s->DeviceName);
	}

	//select
	StringStorage dn;
	m_config->getMpegStreamerCapturedDisplayDeviceName(&dn);
	for (int i = m_displays.getItemsCount() - 1; i >= 0; i--)
	{
		WCHAR* ws = (WCHAR*)m_displays.getItemData(i);
		StringStorage ss;
#ifdef UNICODE
		//TCHAR == WCHAR
		ss = StringStorage(ws);
#else
		//TCHAR == char	
		TO BE IMPLEMENTED!
#endif
		if (ss.isEqualTo(&dn))
		{
			m_displays.setSelectedItem(i);
			break;
		}
	}
}
MpegStreamerConfigDialog::ScreenList MpegStreamerConfigDialog::screens = ScreenList();
void MpegStreamerConfigDialog::clear_screens()
{
	for (Screen* s : screens) 
		delete s;
	screens.clear();
}
BOOL CALLBACK MpegStreamerConfigDialog::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX* mi = new MONITORINFOEX();
	mi->cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(hMonitor, mi))
	{
		delete(mi);
		return TRUE;// continue enumerating
	}
	
	/*StringStorage ss;
	ss.format(_T("%d,%d,%d,%d"), mi->rcWork.left, mi->rcWork.top, mi->rcWork.right - mi->rcWork.left, mi->rcWork.bottom - mi->rcWork.top);
	MessageBox(NULL, ss.getString(), StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);*/

	Screen* s = new Screen();
	s->x = mi->rcMonitor.left;
	s->y = mi->rcMonitor.top;
	s->width = mi->rcMonitor.right - mi->rcMonitor.left;
	s->height = mi->rcMonitor.bottom - mi->rcMonitor.top;
	wcsncpy(s->DeviceName, mi->szDevice, sizeof(s->DeviceName) / sizeof(WCHAR));
	delete(mi);

	ScreenList* screens = (ScreenList*)dwData;
	screens->push_back(s);

	return TRUE;// continue enumerating
}

void MpegStreamerConfigDialog::set_area()
{
	LONG x, y, w, h;
	m_config->getMpegStreamerCapturedArea(&x, &y, &w, &h);
	TCHAR ts[255];
	m_capturedAreaLeft.setText(_itot(x, ts, 10));
	m_capturedAreaTop.setText(_itot(y, ts, 10));
	m_capturedAreaWidth.setText(_itot(w, ts, 10));
	m_capturedAreaHeight.setText(_itot(h, ts, 10));
}

void MpegStreamerConfigDialog::set_windows()
{
	EnumWindows(EnumWindowsProc, (LPARAM)&m_windows);
	
	//select
	StringStorage wt;
	m_config->getMpegStreamerCapturedWindowTitle(&wt);
	for (int i = m_windows.getItemsCount() - 1; i >= 0; i--)
	{
		StringStorage ss;
		m_windows.getItemText(i, &ss);
		if (ss.isEqualTo(&wt))
		{
			m_windows.setSelectedItem(i);
			break;
		}
	}
}
BOOL CALLBACK MpegStreamerConfigDialog::EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	WCHAR title[512];
	if (GetWindowText(hwnd, title, sizeof(title)))
	{
		StringStorage t;
#ifdef UNICODE
			//TCHAR == WCHAR
			t = StringStorage(title);
#else
			//TCHAR == char	
			TO BE IMPLEMENTED!
#endif
		ComboBox* cb = (ComboBox*)lParam;
		for (int i = cb->getItemsCount() - 1; i >= 0; i--)
		{
			StringStorage ss;
			cb->getItemText(i, &ss);
			if(ss.isEqualTo(&t))
				return TRUE;// continue enumerating
		}
		cb->addItem(title);
	}
	return TRUE;// continue enumerating
}

void MpegStreamerConfigDialog::apply()
{
	AutoLock al(m_config);

	m_config->logMpegStreamerProcessOutput(m_logProcessOutput.isChecked());
	m_config->hideMpegStreamerProcessWidnow(m_hideProcessWindow.isChecked());

	StringStorage ss;
	if (m_captureDisplay.isChecked())
	{
		m_config->setMpegStreamerCaptureMode(ServerConfig::MPEG_STREAMER_CAPTURE_MODE_DISPLAY);

		if (m_displays.getSelectedItemIndex() >= 0)
		{
			WCHAR* ws = (WCHAR*)m_displays.getItemData(m_displays.getSelectedItemIndex());
#ifdef UNICODE
			//TCHAR == WCHAR
			ss = StringStorage(ws);
#else
			//TCHAR == char	
			TO BE IMPLEMENTED!
#endif
			m_config->setMpegStreamerCapturedDisplayDeviceName(ss.getString());
		}
	}
	else if (m_captureArea.isChecked())
	{
		m_config->setMpegStreamerCaptureMode(ServerConfig::MPEG_STREAMER_CAPTURE_MODE_AREA);

		LONG x, y, w, h;
		m_capturedAreaLeft.getText(&ss);
		x = _ttoi(ss.getString());
		m_capturedAreaTop.getText(&ss);
		y = _ttoi(ss.getString());
		m_capturedAreaWidth.getText(&ss);
		w = _ttoi(ss.getString());
		m_capturedAreaHeight.getText(&ss);
		h = _ttoi(ss.getString());
		m_config->setMpegStreamerCapturedArea(x, y, w, h);
	}
	else if (m_captureWindow.isChecked())
	{
		m_config->setMpegStreamerCaptureMode(ServerConfig::MPEG_STREAMER_CAPTURE_MODE_WINDOW);

		if (m_windows.getSelectedItemIndex() >= 0)
		{
			m_windows.getItemText(m_windows.getSelectedItemIndex(), &ss);
			m_config->setMpegStreamerCapturedWindowTitle(ss.getString());
		}
	}
	else
		MessageBox(m_ctrlThis.getWindow(), StringTable::getString(IDS_MPEG_STREAMER_NO_MODE_SELECTED), StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
}
