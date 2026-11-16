// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
#include <shellapi.h>
// No text/command/scheme supplied by caller. Success is OS acceptance only.
bool Sys_B2OpenCredit284(unsigned destination)
{
    const wchar_t *url;
    switch(destination){
    case 0:url=L"https://github.com/Togorian/Q2RTX-VR";break;
    case 1:url=L"https://ko-fi.com/togorian";break;
    default:return false;
    }
    return (INT_PTR)ShellExecuteW(NULL,L"open",url,NULL,NULL,SW_SHOWNORMAL)>32;
}

// Session memory belongs to the platform lifetime, not a reconstructed menu.
static unsigned b2_credit_queue301;
bool Sys_B2QueueCredit301(unsigned destination)
{
    if(destination>=2)return false;
    b2_credit_queue301|=1u<<destination;return true;
}
void Sys_B2FlushCredits301(bool normal_exit)
{
    unsigned pending=b2_credit_queue301;b2_credit_queue301=0;
    if(!normal_exit)return;
    for(unsigned i=0;i<2;i++)if(pending&(1u<<i)){
        if(!Sys_B2OpenCredit284(i))MessageBoxW(NULL,
            i?L"Could not open the queued address:\nhttps://ko-fi.com/togorian":
              L"Could not open the queued address:\nhttps://github.com/Togorian/Q2RTX-VR",
            L"Q2RTX VR - queued link",MB_OK|MB_ICONERROR);
    }
}
