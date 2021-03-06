/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

HANDLE hHookIconPressedEvt;

void DrawStatusIcons(MCONTACT hContact, HDC hDC, const RECT &r, int gap)
{
	int x = r.left;
	int cx_icon = GetSystemMetrics(SM_CXSMICON);
	int cy_icon = GetSystemMetrics(SM_CYSMICON);

	int nIcon = 0;
	while (StatusIconData *si = Srmm_GetNthIcon(hContact, nIcon++)) {
		HICON hIcon = ((si->flags & MBF_DISABLED) && si->hIconDisabled) ? si->hIconDisabled : si->hIcon;

		SetBkMode(hDC, TRANSPARENT);
		DrawIconEx(hDC, x, (r.top + r.bottom - cy_icon) >> 1, hIcon, cx_icon, cy_icon, 0, nullptr, DI_NORMAL);

		x += cx_icon + gap;
	}
}

void CheckStatusIconClick(MCONTACT hContact, HWND hwndFrom, POINT pt, const RECT &rc, int gap, int click_flags)
{
	unsigned int iconNum = (pt.x - rc.left) / (GetSystemMetrics(SM_CXSMICON) + gap);
	StatusIconData *si = Srmm_GetNthIcon(hContact, iconNum);
	if (si == nullptr)
		return;

	ClientToScreen(hwndFrom, &pt);

	StatusIconClickData sicd = { sizeof(sicd) };
	sicd.clickLocation = pt;
	sicd.dwId = si->dwId;
	sicd.szModule = si->szModule;
	sicd.flags = click_flags;
	NotifyEventHooks(hHookIconPressedEvt, hContact, (LPARAM)&sicd);
}

static int OnSrmmIconChanged(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		WindowList_Broadcast(g_dat.hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
	else {
		HWND hwnd = WindowList_Find(pci->hWindowList, hContact);
		if (hwnd == nullptr)
			hwnd = SM_FindWindowByContact(hContact);
		if (hwnd != nullptr)
			PostMessage(GetParent(hwnd), DM_STATUSICONCHANGE, 0, 0);
	}
	return 0;
}

int InitStatusIcons()
{
	HookEvent(ME_MSG_ICONSCHANGED, OnSrmmIconChanged);

	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);
	return 0;
}

int DeinitStatusIcons()
{
	DestroyHookableEvent(hHookIconPressedEvt);
	return 0;
}

int GetStatusIconsCount(MCONTACT hContact)
{
	int nIcon = 0;
	while (Srmm_GetNthIcon(hContact, nIcon) != nullptr)
		nIcon++;
	return nIcon;
}
