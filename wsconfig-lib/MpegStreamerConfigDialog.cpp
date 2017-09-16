//********************************************************************************************
//Author: Sergey Stoyan, CliverSoft.com
//        http://cliversoft.com
//        sergey.stoyan@gmail.com
//        stoyan@cliversoft.com
//********************************************************************************************

#include "MpegStreamerConfigDialog.h"
#include "ConfigDialog.h"
#include "tvnserver/resource.h"
#include "CommonInputValidation.h"

MpegStreamerConfigDialog::MpegStreamerConfigDialog()
: BaseDialog(IDD_CONFIG_MPEG_STREAMER_PAGE), m_parent(NULL)
{
}

MpegStreamerConfigDialog::~MpegStreamerConfigDialog()
{
}

void MpegStreamerConfigDialog::setParentDialog(BaseDialog *dialog)
{
  m_parent = dialog;
}

void MpegStreamerConfigDialog::initControls()
{
	HWND dialogHwnd = m_ctrlThis.getWindow();

	m_enableMpegStreamer.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_ENABLED));
	m_destinationPort.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_DESTINATION_PORT));
	m_framerate.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_FRAMERATE));
	m_delayMss.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_START_DELAY));
	m_turnOffRfbVideo.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_TURN_OFF_RFB_VIDEO));
	m_hideStreamerWindow.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_HIDE_WINDOW));
	m_displays.setWindow(GetDlgItem(dialogHwnd, IDC_COMBO_MPEG_STREAMER_MONOTORS));
	m_capturedAreaLeft.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_LEFT));
	m_capturedAreaTop.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_TOP));
	m_capturedAreaWidth.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_WIDTH));
	m_capturedAreaHeight.setWindow(GetDlgItem(dialogHwnd, IDC_MPEG_STREAMER_AREA_HEIGHT));
	m_windows.setWindow(GetDlgItem(dialogHwnd, IDC_COMBO_MPEG_STREAMER_WINDOWS));
	m_captureDisplay.setWindow(GetDlgItem(dialogHwnd, IDC_RADIO_MPEG_STREAMER_MONITOR));
	m_captureArea.setWindow(GetDlgItem(dialogHwnd, IDC_RADIO_MPEG_STREAMER_AREA));
	m_captureWindow.setWindow(GetDlgItem(dialogHwnd, IDC_RADIO_MPEG_STREAMER_WINDOW));
}

BOOL MpegStreamerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
	switch (controlID)
	{
	case IDC_MPEG_STREAMER_ENABLED:
	case IDC_RADIO_MPEG_STREAMER_MONITOR:
	case IDC_RADIO_MPEG_STREAMER_AREA:
	case IDC_RADIO_MPEG_STREAMER_WINDOW:
		if (notificationID == BN_CLICKED)
			onMpegStreamerEnabledClick();
		break;
	case IDC_MPEG_STREAMER_DESTINATION_PORT:
	case IDC_MPEG_STREAMER_FRAMERATE:
	case IDC_MPEG_STREAMER_START_DELAY:
	case IDC_MPEG_STREAMER_TURN_OFF_RFB_VIDEO:
	case IDC_MPEG_STREAMER_HIDE_WINDOW:
	case IDC_MPEG_STREAMER_AREA_LEFT:
	case IDC_MPEG_STREAMER_AREA_TOP:
	case IDC_MPEG_STREAMER_AREA_WIDTH:
	case IDC_MPEG_STREAMER_AREA_HEIGHT:
		if (notificationID == EN_UPDATE)
			onMpegStreamerChange();
		break;
	case IDC_COMBO_MPEG_STREAMER_MONOTORS:
	case IDC_COMBO_MPEG_STREAMER_WINDOWS:
		if (notificationID == 1/*||notificationID == 9*/)
			onMpegStreamerChange();
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

void MpegStreamerConfigDialog::onMpegStreamerEnabledClick()
{
	m_destinationPort.setEnabled(m_enableMpegStreamer.isChecked());
	m_framerate.setEnabled(m_enableMpegStreamer.isChecked());
	m_delayMss.setEnabled(m_enableMpegStreamer.isChecked());
	m_turnOffRfbVideo.setEnabled(m_enableMpegStreamer.isChecked());
	m_hideStreamerWindow.setEnabled(m_enableMpegStreamer.isChecked());

	m_captureDisplay.setEnabled(m_enableMpegStreamer.isChecked());
	m_displays.setEnabled(m_captureDisplay.isChecked());

	m_captureArea.setEnabled(m_enableMpegStreamer.isChecked());
	m_capturedAreaLeft.setEnabled(m_captureArea.isChecked());
	m_capturedAreaTop.setEnabled(m_captureArea.isChecked());
	m_capturedAreaWidth.setEnabled(m_captureArea.isChecked());
	m_capturedAreaHeight.setEnabled(m_captureArea.isChecked());

	m_captureWindow.setEnabled(m_enableMpegStreamer.isChecked());
	m_windows.setEnabled(m_captureWindow.isChecked());

	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

void MpegStreamerConfigDialog::onMpegStreamerChange()
{
	((ConfigDialog *)m_parent)->updateApplyButtonState();
}

bool MpegStreamerConfigDialog::validateInput()
{
	if (!m_enableMpegStreamer.isChecked())
		return true;

	StringStorage ss;
	long i;
	m_destinationPort.getText(&ss);
	if (!CommonInputValidation::parseNumber(&ss, &i) || i < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_MPEG_STREAMER_PORT_ERROR),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}
	if (!CommonInputValidation::validatePort(&m_destinationPort))
		return false;

	m_framerate.getText(&ss);
	if (!CommonInputValidation::parseNumber(&ss, &i) || i < 1) {
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_MPEG_STREAMER_FRAMERATE_ERROR),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	m_delayMss.getText(&ss);
	if (!CommonInputValidation::parseNumber(&ss, &i))
	{
		MessageBox(m_ctrlThis.getWindow(),
			StringTable::getString(IDS_SET_MPEG_STREAMER_START_DELAY_ERROR),
			StringTable::getString(IDS_CAPTION_BAD_INPUT), MB_ICONSTOP | MB_OK);
		return false;
	}

	return true;
}

void MpegStreamerConfigDialog::updateUI()
{
	m_enableMpegStreamer.check(m_config->isMpegStreamerEnabled());
	MpegStreamerConfigDialog::onMpegStreamerEnabledClick();

	TCHAR ts[255];
	m_destinationPort.setText(_itot(m_config->getMpegStreamerDestinationPort(), ts, 10));

	m_framerate.setText(_itot(m_config->getMpegStreamerFramerate(), ts, 10));

	m_delayMss.setText(_itot(m_config->getMpegStreamerDelayMss(), ts, 10));

	m_turnOffRfbVideo.check(m_config->isMpegStreamerRfbVideoTunedOff());

	m_hideStreamerWindow.check(m_config->isMpegStreamerWindowHidden());

	set_monitors();

	set_area();

	switch (m_config->getMpegStreamerCaptureMode())
	{
	case MpegStreamerCaptureDisplay:
		m_captureDisplay.check(true);
		break;
	case MpegStreamerCaptureArea:
		m_captureArea.check(true);
		break;
	case MpegStreamerCaptureWindow:
		m_captureWindow.check(true);
		break;
	default:
		m_captureDisplay.check(true);
		break;
	}
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX* mi = new MONITORINFOEX();
	mi->cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(hMonitor, mi))
		return TRUE;// continue enumerating

	MpegStreamerConfigDialog::Screen* s = new MpegStreamerConfigDialog::Screen();
	s->x = mi->rcMonitor.left;
	s->y = mi->rcMonitor.top;
	s->width = mi->rcMonitor.right - mi->rcMonitor.left;
	s->height = mi->rcMonitor.bottom - mi->rcMonitor.top;
	wcsncpy(s->DeviceName, mi->szDevice, sizeof(s->DeviceName) / sizeof(WCHAR));
	MpegStreamerConfigDialog::Screens.push_back(s);

	return TRUE;// continue enumerating
}
MpegStreamerConfigDialog::ScreenList MpegStreamerConfigDialog::Screens = MpegStreamerConfigDialog::ScreenList();
void MpegStreamerConfigDialog::set_monitors() 
{
	int numberOfScreens = GetSystemMetrics(SM_CMONITORS);
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	Screens.clear();

	//getting dimensions of the monitors
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

	//getting names of real monitors
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	for (ScreenList::iterator i = Screens.begin(); i != Screens.end(); i++)
	{
		Screen* s = (*i);
		EnumDisplayDevices(s->DeviceName, 0, &dd, 0);
		wcsncpy(s->DeviceName, dd.DeviceName, sizeof(s->DeviceName) / sizeof(WCHAR));
		wcsncpy(s->DeviceString, dd.DeviceString, sizeof(s->DeviceString) / sizeof(WCHAR));
	}

	//fill dropdown list
	StringStorage ss;
	for (ScreenList::iterator i = Screens.begin(); i != Screens.end(); i++)
	{
		Screen* s = (*i);
		ss.format(_T("%s (%d,%d),(%dx%d)"), s->DeviceString, s->x, s->y, s->width, s->height);
		m_displays.addItem(ss.getString(), s->DeviceName);
	}

	//select
	StringStorage dn;
	m_config->getMpegStreamerCapturedDesktopDeviceName(&dn);
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

void MpegStreamerConfigDialog::apply()
{
	AutoLock al(m_config);

	m_config->enableMpegStreamer(m_enableMpegStreamer.isChecked());

	StringStorage ss;
	m_destinationPort.getText(&ss);
	m_config->setMpegStreamerDestinationPort(_ttoi(ss.getString()));

	m_framerate.getText(&ss);
	m_config->setMpegStreamerFramerate(_ttoi(ss.getString()));

	m_delayMss.getText(&ss);
	m_config->setMpegStreamerDelayMss(_ttoi(ss.getString()));

	m_config->turnOffMpegStreamerRfbVideo(m_turnOffRfbVideo.isChecked());

	m_config->hideMpegStreamerWindow(m_hideStreamerWindow.isChecked());

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
			/*for (ScreenList::iterator i = Screens.begin(); i != Screens.end(); i++)
			{
				Screen* s = (*i);
				if (!wcsncmp(s->DeviceName, ws, sizeof(s->DeviceName) / sizeof(WCHAR)))
				{
					m_capturedAreaLeft = s->x;
					break;
				}
			}*/
	}
	else
		ss.format(_T(""));
	m_config->setMpegStreamerCapturedDesktopDeviceName(ss.getString());

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

	if (m_captureDisplay.isChecked())
	{
		m_config->setMpegStreamerCaptureMode(MpegStreamerCaptureDisplay);
	}
	else if (m_captureArea.isChecked())
		m_config->setMpegStreamerCaptureMode(MpegStreamerCaptureArea);
	else if (m_captureWindow.isChecked())
		m_config->setMpegStreamerCaptureMode(MpegStreamerCaptureWindow);

	//MessageBox(m_ctrlThis.getWindow(),
	//	ss.getString(),
	//	_T("222"), MB_ICONSTOP | MB_OK);
}
