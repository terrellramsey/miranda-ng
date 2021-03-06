/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

void SetupInfobar(InfobarWindowData* idat)
{
	DWORD colour = db_get_dw(0, SRMMMOD, SRMSGSET_INFOBARBKGCOLOUR, SRMSGDEFSET_INFOBARBKGCOLOUR);

	HWND hwnd = idat->hWnd;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETBKGNDCOLOR, 0, colour);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETBKGNDCOLOR, 0, colour);

	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_INFOBAR_NAME, &lf, &colour);

	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf2.cbSize = sizeof(cf2);
	cf2.crTextColor = colour;
	cf2.bCharSet = lf.lfCharSet;
	wcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf2.wWeight = (WORD)lf.lfWeight;
	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	LoadMsgDlgFont(MSGFONTID_INFOBAR_STATUS, &lf, &colour);
	cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf2.cbSize = sizeof(cf2);
	cf2.crTextColor = colour;
	cf2.bCharSet = lf.lfCharSet;
	wcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf2.wWeight = (WORD)lf.lfWeight;
	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	RefreshInfobar(idat);
}

static HICON GetExtraStatusIcon(InfobarWindowData* idat)
{
	BYTE bXStatus = db_get_b(idat->mwd->m_hContact, idat->mwd->m_szProto, "XStatusId", 0);
	if (bXStatus > 0)
		return (HICON)CallProtoService(idat->mwd->m_szProto, PS_GETCUSTOMSTATUSICON, bXStatus, 0);

	return nullptr;
}

void RefreshInfobar(InfobarWindowData* idat)
{
	HWND hwnd = idat->hWnd;
	CSrmmWindow *dat = idat->mwd;
	ptrW szContactStatusMsg(db_get_wsa(dat->m_hContact, "CList", "StatusMsg"));
	ptrW szXStatusName(db_get_wsa(idat->mwd->m_hContact, idat->mwd->m_szProto, "XStatusName"));
	ptrW szXStatusMsg(db_get_wsa(idat->mwd->m_hContact, idat->mwd->m_szProto, "XStatusMsg"));
	HICON hIcon = GetExtraStatusIcon(idat);
	wchar_t szText[2048];
	SETTEXTEX st;
	if (szXStatusMsg && *szXStatusMsg)
		mir_snwprintf(szText, L"%s (%s)", TranslateW(szXStatusName), szXStatusMsg);
	else
		wcsncpy_s(szText, TranslateW(szXStatusName), _TRUNCATE);
	st.flags = ST_DEFAULT;
	st.codepage = 1200;
	SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)pcli->pfnGetContactDisplayName(dat->m_hContact, 0));
	SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)szContactStatusMsg);
	hIcon = (HICON)SendDlgItemMessage(hwnd, IDC_XSTATUSICON, STM_SETICON, (WPARAM)hIcon, 0);
	if (hIcon)
		DestroyIcon(hIcon);

	SetToolTipText(hwnd, idat->hXStatusTip, szText, nullptr);
	SendMessage(hwnd, WM_SIZE, 0, 0);
	InvalidateRect(hwnd, nullptr, TRUE);
	RedrawWindow(GetDlgItem(hwnd, IDC_AVATAR), nullptr, nullptr, RDW_INVALIDATE);
}

static INT_PTR CALLBACK InfobarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bWasCopy;
	InfobarWindowData* idat = (InfobarWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!idat && msg != WM_INITDIALOG)
		return FALSE;

	switch (msg) {
	case WM_INITDIALOG:
		bWasCopy = FALSE;
		idat = (InfobarWindowData*)lParam;
		idat->hWnd = hwnd;
		{
			RECT rect = { 0 };
			idat->hXStatusTip = CreateToolTip(hwnd, nullptr, nullptr, &rect);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)idat);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_NAME, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwnd, IDC_INFOBAR_STATUS, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SetupInfobar(idat);
		}
		return TRUE;

	case WM_SIZE:
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			int dlgWidth, dlgHeight;
			int avatarWidth = 0;
			int avatarHeight = 0;

			RECT rc;
			GetClientRect(hwnd, &rc);
			dlgWidth = rc.right - rc.left;
			dlgHeight = rc.bottom - rc.top;
			if (idat->mwd->m_hbmpAvatarPic && (g_dat.flags & SMF_AVATAR)) {
				BITMAP bminfo;
				GetObject(idat->mwd->m_hbmpAvatarPic, sizeof(bminfo), &bminfo);
				if (bminfo.bmWidth != 0 && bminfo.bmHeight != 0) {
					avatarHeight = dlgHeight - 2;
					avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
					if (avatarWidth > dlgHeight) {
						avatarWidth = dlgHeight - 2;
						avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
					}
				}
			}
			HDWP hdwp = BeginDeferWindowPos(4);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INFOBAR_NAME), 0, 16, 0, dlgWidth - avatarWidth - 2 - 32, dlgHeight / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_INFOBAR_STATUS), 0, 16, dlgHeight / 2, dlgWidth - avatarWidth - 2 - 32, dlgHeight / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_AVATAR), 0, dlgWidth - avatarWidth - 2, (dlgHeight - avatarHeight) / 2, avatarWidth, (dlgHeight + avatarHeight - 2) / 2, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_XSTATUSICON), 0, dlgWidth - avatarWidth - 2 - 16, dlgHeight / 4 - 8, 16, 16, SWP_NOZORDER);
			rc.left = dlgWidth - avatarWidth - 2 - 16;
			rc.top = dlgHeight / 4 - 8;
			rc.bottom = rc.top + 20;
			rc.right = rc.left + 16;
			SetToolTipRect(hwnd, idat->hXStatusTip, &rc);
			EndDeferWindowPos(hdwp);
		}
		return TRUE;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)g_dat.hInfobarBrush;

	case WM_DROPFILES:
		SendMessage(GetParent(hwnd), WM_DROPFILES, wParam, lParam);
		return FALSE;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->idFrom) {
			case IDC_INFOBAR_NAME:
			case IDC_INFOBAR_STATUS:
				switch (pNmhdr->code) {
				case EN_MSGFILTER:
					switch (((MSGFILTER*)lParam)->msg) {
					case WM_CHAR:
						SendMessage(GetParent(hwnd), ((MSGFILTER *)lParam)->msg, ((MSGFILTER *)lParam)->wParam, ((MSGFILTER *)lParam)->lParam);
						SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;

					case WM_LBUTTONUP:
						CHARRANGE sel;
						SendDlgItemMessage(hwnd, pNmhdr->idFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
						bWasCopy = FALSE;
						if (sel.cpMin != sel.cpMax) {
							SendDlgItemMessage(hwnd, pNmhdr->idFrom, WM_COPY, 0, 0);
							sel.cpMin = sel.cpMax;
							SendDlgItemMessage(hwnd, pNmhdr->idFrom, EM_EXSETSEL, 0, (LPARAM)&sel);
							bWasCopy = TRUE;
						}
						SetFocus(GetParent(hwnd));
					}
					break;
				}
				break;
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == GetDlgItem(hwnd, IDC_AVATAR)) {
				RECT rect;
				HDC hdcMem = CreateCompatibleDC(dis->hDC);
				int itemWidth = dis->rcItem.right - dis->rcItem.left + 1;
				int itemHeight = dis->rcItem.bottom - dis->rcItem.top + 1;
				HBITMAP hbmMem = CreateCompatibleBitmap(dis->hDC, itemWidth, itemHeight);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmMem);
				rect.top = 0;
				rect.left = 0;
				rect.right = itemWidth - 1;
				rect.bottom = itemHeight - 1;
				FillRect(hdcMem, &rect, g_dat.hInfobarBrush);
				if (idat->mwd->m_hbmpAvatarPic && (g_dat.flags & SMF_AVATAR)) {
					BITMAP bminfo;
					GetObject(idat->mwd->m_hbmpAvatarPic, sizeof(bminfo), &bminfo);
					if (bminfo.bmWidth != 0 && bminfo.bmHeight != 0) {
						int avatarHeight = itemHeight;
						int avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
						if (avatarWidth > itemWidth) {
							avatarWidth = itemWidth;
							avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
						}

						AVATARDRAWREQUEST adr = { sizeof(adr) };
						adr.hContact = idat->mwd->m_hContact;
						adr.hTargetDC = hdcMem;
						adr.rcDraw.right = avatarWidth - 1;
						adr.rcDraw.bottom = avatarHeight - 1;
						adr.dwFlags = AVDRQ_DRAWBORDER | AVDRQ_HIDEBORDERONTRANSPARENCY;
						CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);
					}
				}
				BitBlt(dis->hDC, 0, 0, itemWidth, itemHeight, hdcMem, 0, 0, SRCCOPY);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmMem);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);
				return TRUE;
			}
		}
		return Menu_DrawItem(lParam);

	case WM_LBUTTONDOWN:
		SendMessage(idat->mwd->GetHwnd(), WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case WM_RBUTTONUP:
		{
			HMENU hMenu = Menu_BuildContactMenu(idat->mwd->m_hContact);

			POINT pt;
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, GetParent(hwnd), nullptr);
			DestroyMenu(hMenu);
		}
		break;

	case WM_DESTROY:
		if (idat->hXStatusTip != nullptr) {
			DestroyWindow(idat->hXStatusTip);
			idat->hXStatusTip = nullptr;
		}
		mir_free(idat);
	}
	return FALSE;
}

InfobarWindowData* CreateInfobar(HWND hParent, CSrmmWindow *dat)
{
	InfobarWindowData *idat = (InfobarWindowData*)mir_alloc(sizeof(InfobarWindowData));
	idat->mwd = dat;
	idat->hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_INFOBAR), hParent, InfobarWndProc, (LPARAM)idat);
	RichUtil_SubClass(idat->hWnd);
	SetWindowPos(idat->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOREPOSITION);
	return idat;
}
