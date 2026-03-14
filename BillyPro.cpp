// BillyPro.cpp  -  Win32 + BASS audio player  v4
// Compile:
//   cl BillyPro.cpp /W3 /O2 /link bass.lib User32.lib Gdi32.lib
//          Comctl32.lib Shell32.lib Ole32.lib Winmm.lib Uxtheme.lib
//          windowscodecs.lib Comdlg32.lib

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <wincodec.h>
#include <commdlg.h>
#include <cmath>
#include <cwchar>
#include <vector>
#include <algorithm>
#include "bass.h"
#include "Resource.h"


#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Uxtheme.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Comdlg32.lib")

// ============================================================
//  IDs
// ============================================================
#define ID_LISTBOX          101
#define ID_BTN_PLAYPAUSE    103
#define ID_BTN_PAUSE        116
#define ID_BTN_STOP         104
#define ID_BTN_PREV         109
#define ID_BTN_NEXT         110
#define ID_VOLUMECANVAS     105
#define ID_VOL_PCT          112
#define ID_SEEKCANVAS       106
#define ID_TIME_CURRENT     107
#define ID_TIME_TOTAL       108
#define ID_TIME_REMAIN      111
#define ID_STATUSBAR        120
#define ID_BTN_SHUFFLE      121
#define ID_BTN_REPEAT       122
#define ID_BTN_MONO         123
#define ID_BTN_NORMALIZE    124
#define ID_BTN_BASSBOOST    125
#define IDM_HELP_CONTROLS   2302
#define IDM_OPTIONS         2401
#define IDM_OPTIONS_SHOW    2402

// Context menu
#define IDC_CTX_PLAY        301
#define IDC_CTX_REMOVE      302
#define IDC_CTX_PROPERTIES  303  // Windows shell Properties
#define IDC_CTX_AUDIOINFO   304  // Our custom audio info dialog

// Search dialog
#define ID_SEARCH_EDIT      201
#define ID_SEARCH_LIST      202
#define ID_SEARCH_OK        203
#define ID_SEARCH_CANCEL    204

// Audio info dialog
#define ID_INFO_ARTWORK     401
#define ID_INFO_TEXT        402
#define ID_INFO_CLOSE       403
#define ID_INFO_SAVE_ART    404
#define ID_INFO_REPLACE_ART 405
// Metadata edit fields
#define ID_INFO_TITLE       410
#define ID_INFO_ARTIST      411
#define ID_INFO_ALBUM       412
#define ID_INFO_YEAR        413
#define ID_INFO_TRACK       414
#define ID_INFO_GENRE       415
#define ID_INFO_SAVE_TAGS   416

// Convert dialog
#define IDM_FILE_CONVERT    2004
#define ID_CONV_LIST        501
#define ID_CONV_ADDFOLDER   502
#define ID_CONV_ADDPLAYLIST 503
#define ID_CONV_REMOVE      504
#define ID_CONV_OUTDIR      505
#define ID_CONV_BROWSE      506
#define ID_CONV_FORMAT      507
#define ID_CONV_QUALITY     508
#define ID_CONV_START       509
#define ID_CONV_CLOSE       510
#define ID_CONV_PROGRESS    511
#define ID_CONV_STATUS      512
#define ID_CONV_SELECTALL   513
#define ID_CONV_NORMALIZE   514
#define ID_CONV_METADATA    515

#define IDT_PLAYBACK        1
#define IDT_SEEK_REPEAT     2
#define IDT_PEAK_DECAY      3   // volume peak meter decay
#define IDT_PEAK_METER      4   // audio peak meter always-running
#define WM_PLAYNEXT         (WM_APP + 1)
#define WM_TRAYICON         (WM_APP + 2)
#define ID_TRAY_RESTORE     601
#define ID_TRAY_EXIT        602
#ifndef IDC_STATIC
#define IDC_STATIC          (-1)
#endif

#define IDM_FILE_OPENFOLDER 2001
#define IDM_FILE_EXIT       2003
#define IDM_PLAY_PLAYPAUSE  2101
#define IDM_PLAY_STOP       2102
#define IDM_PLAY_NEXT       2103
#define IDM_PLAY_PREV       2104
#define IDM_PLAY_SHUFFLE    2105
#define IDM_PLAY_REPEAT     2106
#define IDM_HELP_ABOUT      2301

// Resource icon ID - matches BillyPro.rc: IDI_BILLYPRO ICON "BillyPro.ico"
// Include resource.h if it's in your project, otherwise use the numeric ID from it.
struct Theme {
    COLORREF bg, bgList, text, textDim, accent, seekTrk, btnFace, btnBorder, btnSym;
};
// COLORREF = 0x00BBGGRR  (R in low byte, B in high byte)
// Blue #1E78D2: R=0x1E, G=0x78, B=0xD2  ->  0x00D2781E
// Blue #40A0E8: R=0x40, G=0xA0, B=0xE8  ->  0x00E8A040
static const Theme LIGHT = {
    0xF0F0F0, 0xFFFFFF, 0x1A1A1A, 0x505050,
    0x00D2781E, 0xCCCCCC,  // accent: blue #1E78D2
    0xE1E1E1, 0xADADAD, 0x1A1A1A
};


// ============================================================
//  Globals
// ============================================================
static const wchar_t CLASS_NAME[] = L"BillyProWnd";
static const wchar_t SEEK_CLASS[] = L"BillySeekBar";
static const wchar_t VOL_CLASS[] = L"BillyVolBar";
static const wchar_t APP_TITLE[] = L"Billy Pro";

HWND g_hwnd = NULL;
HWND hListBox = NULL;
HWND hPrevBtn = NULL;
HWND hPlayBtn = NULL;
HWND hPauseBtn = NULL;
HWND hStopBtn = NULL;
HWND hNextBtn = NULL;
HWND hShuffleBtn = NULL;
HWND hRepeatBtn = NULL;
HWND hMonoBtn = NULL;
HWND hNormalizeBtn = NULL;
HWND hBassBoostBtn = NULL;
HWND hVolumeCanvas = NULL;
HWND hSeekCanvas = NULL;
HWND hTimeCur = NULL;
HWND hTimeTot = NULL;
HWND hTimeRemain = NULL;
HWND hVolPct = NULL;
HWND hStatus = NULL;

HSTREAM currentStream = 0;
float   currentVolume = 0.8f;
bool    g_shuffle = false;
bool    g_repeat = false;
bool    g_mono = false;
bool    g_normalize = false;
wchar_t g_iniPath[MAX_PATH] = L"";
bool    g_bassBoost = false;
float   g_bbFreqLow = 30.0f;   // Hz
float   g_bbFreqHigh = 100.0f;  // Hz
float   g_bbGainDB = 5.0f;    // dB
// Recording
wchar_t g_recOutDir[MAX_PATH] = L"";
// DSP handles
HDSP    g_dspMono = 0;
HDSP    g_dspBass = 0;

bool    g_seekDragging = false;
double  g_seekDragPos = 0.0;

// Volume peak meter
bool    g_volDragging = false;    // user is dragging the volume bar
float   g_audioPeakHold = 0.0f;  // audio peak hold level
ULONGLONG g_audioPeakTime = 0;   // time peak was last set (ms)
bool    g_volPeaking = false;    // we're showing peak hold
float   g_peakLevel = 0.0f;     // current peak level 0..1
float   g_bassLevel = 0.0f;     // low frequency level 0..1
float   g_peakHold = 0.0f;     // held peak marker 0..1

bool    g_seekKeyHeld = false;
int     g_seekKeyDir = 0;

Theme   g_theme = LIGHT;

static WNDPROC g_OldListProc = NULL;
static WNDPROC g_OldBtnProc = NULL;
static HWND    g_hoveredBtn = NULL;

static WNDPROC g_OldSearchProc = NULL;

struct Track { wchar_t path[MAX_PATH]; wchar_t display[MAX_PATH]; };
std::vector<Track> g_playlist;
std::vector<int>   g_shuffleOrder;
int g_currentIndex = -1;

// Right-click context track index
int g_ctxTrackIndex = -1;

HFONT  g_fontUI = NULL;
HFONT  g_fontMono = NULL;
HFONT  g_fontBold = NULL;
HBRUSH g_brBg = NULL;
HBRUSH g_brList = NULL;

HWND   g_hwndSearch = NULL;
HWND   hSearchEdit = NULL;
HWND   hSearchList = NULL;
std::vector<int> g_searchResults;

// Listbox drag-reorder state
static bool  g_lbDragging = false;
static int   g_lbDragFrom = -1;
static int   g_lbDragCur = -1;
static bool  g_lbDragMoved = false;  // did mouse move enough to be a drag?

HWND   g_hwndInfo = NULL;   // audio info window
HWND   g_hwndConvert = NULL; // convert window
bool   g_trayAdded = false;  // system tray icon active

// ============================================================
//  Forward declarations
// ============================================================
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon();
void ShowTrayContextMenu(HWND hwnd);
void UpdateStatusBar();
void PlayIndex(int idx);
void UpdatePlayBtn();
void UpdateTimeDisplays();
void StopAudio();
void TogglePlayPause();
void PlayNext();
void PlayPrev();
void SeekToSeconds(double sec);
void UpdateVolume();
void LayoutControls(HWND hwnd);

void ApplyTheme();
void RebuildShuffleOrder();
void UpdateWindowTitle();
void OpenSearchDialog();
void OpenAudioInfoDialog(int trackIdx);
void ShowTrackContextMenu(HWND hwnd, int trackIdx, POINT pt);
LRESULT CALLBACK SearchWndProc(HWND, UINT, WPARAM, LPARAM);
void OpenConvertDialog();
// KEY FIX: global accelerator-style message pre-filter
bool HandleGlobalKey(WPARAM vk);

// ============================================================
//  Helpers
// ============================================================
static void FormatTime(double sec, wchar_t* out, size_t n)
{
    if (sec < 0 || !_finite(sec)) { wcsncpy_s(out, n, L"--:--", _TRUNCATE); return; }
    int t = (int)sec;
    swprintf_s(out, n, L"%d:%02d", t / 60, t % 60);
}

static const wchar_t* Filename(const wchar_t* p)
{
    const wchar_t* s = wcsrchr(p, L'\\');
    if (!s) s = wcsrchr(p, L'/');
    return s ? s + 1 : p;
}

static bool IsAudio(const wchar_t* p)
{
    const wchar_t* e = wcsrchr(p, L'.');
    if (!e) return false;
    static const wchar_t* EXT[] = {
        L".mp3",L".wav",L".flac",L".ogg",L".m4a",
        L".aac",L".wma",L".opus",L".ape",L".aiff",nullptr };
    for (int i = 0; EXT[i]; ++i)
        if (_wcsicmp(e, EXT[i]) == 0) return true;
    return false;
}

// ============================================================
//  Playlist
// ============================================================
void ClearPlaylist()
{
    g_playlist.clear(); g_shuffleOrder.clear(); g_currentIndex = -1;
    if (hListBox) SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
}

void AddTrack(const wchar_t* path)
{
    Track t; wcsncpy_s(t.path, path, _TRUNCATE);
    wcsncpy_s(t.display, Filename(path), _TRUNCATE);
    g_playlist.push_back(t);
    if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
}

void RebuildShuffleOrder()
{
    g_shuffleOrder.resize(g_playlist.size());
    for (size_t i = 0; i < g_shuffleOrder.size(); ++i) g_shuffleOrder[i] = (int)i;
    for (size_t i = g_shuffleOrder.size(); i > 1; --i)
        std::swap(g_shuffleOrder[i - 1], g_shuffleOrder[rand() % i]);
}

void LoadFolder(const wchar_t* folder)
{
    ClearPlaylist();
    wchar_t pat[MAX_PATH]; swprintf_s(pat, L"%s\\*.*", folder);
    WIN32_FIND_DATA fd; HANDLE h = FindFirstFile(pat, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", folder, fd.cFileName);
                if (IsAudio(full)) AddTrack(full);
            }
        } while (FindNextFile(h, &fd));
        FindClose(h);
    }
    RebuildShuffleOrder();
    if (!g_playlist.empty()) { SendMessage(hListBox, LB_SETCURSEL, 0, 0); g_currentIndex = 0; }
    UpdateStatusBar();
}

// ============================================================
//  DSP: Mono mix-down
// ============================================================
void CALLBACK DSP_Mono(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    // BASS DSP buffers are always float interleaved L,R,L,R...
    float* buf = (float*)buffer;
    DWORD floats = length / sizeof(float);
    // Mix stereo pairs to mono
    for (DWORD i = 0; i + 1 < floats; i += 2) {
        float m = (buf[i] + buf[i + 1]) * 0.5f;
        buf[i] = buf[i + 1] = m;
    }
}

// DSP: Bass boost with soft clip limiter
// 1-pole low-pass IIR state (stereo)
static float s_lpL = 0.0f, s_lpR = 0.0f;
// Limiter envelope follower
static float s_limGain = 1.0f;

void CALLBACK DSP_BassBoost(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf = (float*)buffer;
    DWORD floats = length / sizeof(float);
    // Low-shelf: cutoff ~120Hz, alpha = exp(-2*pi*120/44100)
    const float alpha = 0.9830f;
    // How much extra bass gain to ADD (linear)
    float bassGain = powf(10.0f, g_bbGainDB / 20.0f) - 1.0f;
    // Limiter constants: fast attack, slow release
    const float limAttack = 0.001f;   // per sample attack
    const float limRelease = 0.00001f; // per sample release
    const float limThresh = 0.95f;    // ceiling

    for (DWORD i = 0; i + 1 < floats; i += 2) {
        // 1. Extract bass via low-pass
        s_lpL = alpha * s_lpL + (1.0f - alpha) * buf[i];
        s_lpR = alpha * s_lpR + (1.0f - alpha) * buf[i + 1];

        // 2. Add boosted bass to original
        float outL = buf[i] + s_lpL * bassGain;
        float outR = buf[i + 1] + s_lpR * bassGain;

        // 3. True peak limiter - gain reduction based on peak of both channels
        float peak = max(fabsf(outL), fabsf(outR));
        if (peak * s_limGain > limThresh && peak > 0.0f) {
            // Gain reduction needed - attack
            float targetGain = limThresh / peak;
            s_limGain += (targetGain - s_limGain) * limAttack;
            if (s_limGain > targetGain) s_limGain = targetGain; // never exceed target
        }
        else {
            // Release - slowly return gain to 1.0
            s_limGain += (1.0f - s_limGain) * limRelease;
            if (s_limGain > 1.0f) s_limGain = 1.0f;
        }

        // 4. Apply gain reduction to both channels equally (keeps stereo image)
        buf[i] = outL * s_limGain;
        buf[i + 1] = outR * s_limGain;
    }
}

// ============================================================
//  INI settings
// ============================================================
static void GetIniPath()
{
    if (g_iniPath[0]) return;
    wchar_t dir[MAX_PATH];
    GetModuleFileName(NULL, dir, MAX_PATH);
    wchar_t* sl = wcsrchr(dir, L'\\');
    if (sl) sl[1] = 0;
    wcscpy_s(g_iniPath, dir);
    wcscat_s(g_iniPath, L"BillyPro.ini");
}

void SaveSettings()
{
    GetIniPath();
    wchar_t buf[64];
    swprintf_s(buf, L"%d", (int)g_bassBoost);  WritePrivateProfileString(L"DSP", L"BassBoost", buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_bbFreqLow);        WritePrivateProfileString(L"DSP", L"BBFreqLow", buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_bbFreqHigh);       WritePrivateProfileString(L"DSP", L"BBFreqHigh", buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_bbGainDB);         WritePrivateProfileString(L"DSP", L"BBGainDB", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_mono);        WritePrivateProfileString(L"DSP", L"Mono", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_normalize);   WritePrivateProfileString(L"DSP", L"Normalize", buf, g_iniPath);
    // Recording output dir
    WritePrivateProfileString(L"Paths", L"RecordOutDir", g_recOutDir, g_iniPath);
}

void LoadSettings()
{
    GetIniPath();
    wchar_t buf[64];
#define GETI(sec,key,def) (int)GetPrivateProfileInt(sec, key, def, g_iniPath)
#define GETF(sec,key,def) ([&]{ GetPrivateProfileString(sec,key,L#def,buf,64,g_iniPath); return (float)_wtof(buf); }())
    g_bassBoost = GETI(L"DSP", L"BassBoost", 0) != 0;
    g_bbFreqLow = GETF(L"DSP", L"BBFreqLow", 30.0);
    g_bbFreqHigh = GETF(L"DSP", L"BBFreqHigh", 100.0);
    g_bbGainDB = GETF(L"DSP", L"BBGainDB", 5.0);
    g_mono = GETI(L"DSP", L"Mono", 0) != 0;
    g_normalize = GETI(L"DSP", L"Normalize", 0) != 0;
    GetPrivateProfileString(L"Paths", L"RecordOutDir", L"", g_recOutDir, MAX_PATH, g_iniPath);
#undef GETI
#undef GETF
}


void ApplyDSP()
{
    if (!currentStream) return;
    // Remove existing DSP
    if (g_dspMono) { BASS_ChannelRemoveDSP(currentStream, g_dspMono);   g_dspMono = 0; }
    if (g_dspBass) { BASS_ChannelRemoveDSP(currentStream, g_dspBass);   g_dspBass = 0; }
    // Reapply
    if (g_mono)
        g_dspMono = BASS_ChannelSetDSP(currentStream, DSP_Mono, NULL, 1);
    if (g_bassBoost)
        g_dspBass = BASS_ChannelSetDSP(currentStream, DSP_BassBoost, NULL, 2);
    // Normalization: quick-scan peak and adjust volume
    if (g_normalize) {
        HSTREAM scan = BASS_StreamCreateFile(FALSE,
            g_playlist[g_currentIndex].path, 0, 0,
            BASS_UNICODE | BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
        if (scan) {
            float peak = 0.001f;
            float fbuf[4096];
            DWORD got;
            int chunks = 0;
            while (chunks < 200 &&
                (got = BASS_ChannelGetData(scan, fbuf, sizeof(fbuf))) > 0) {
                DWORD n = got / sizeof(float);
                for (DWORD j = 0; j < n; j++) {
                    float a = fabsf(fbuf[j]);
                    if (a > peak) peak = a;
                }
                chunks++;
            }
            BASS_StreamFree(scan);
            float normVol = currentVolume / peak;
            if (normVol > 4.0f) normVol = 4.0f;
            BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, normVol);
        }
    }
    else {
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
    }
}

// ============================================================
//  Recording
// ============================================================

// ============================================================
//  Audio
// ============================================================
void AddFolder(const wchar_t* folder)
{
    // Like LoadFolder but does NOT clear existing playlist
    wchar_t pat[MAX_PATH]; swprintf_s(pat, L"%s\\*.*", folder);
    WIN32_FIND_DATA fd; HANDLE h = FindFirstFile(pat, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", folder, fd.cFileName);
                if (IsAudio(full)) AddTrack(full);
            }
        } while (FindNextFile(h, &fd));
        FindClose(h);
    }
}

void StopAudio()
{
    KillTimer(g_hwnd, IDT_PLAYBACK);
    KillTimer(g_hwnd, IDT_SEEK_REPEAT);
    KillTimer(g_hwnd, IDT_PEAK_METER);
    g_seekKeyHeld = false;
    if (currentStream) {
        BASS_ChannelStop(currentStream);
        BASS_StreamFree(currentStream);
        currentStream = 0;
    }
    g_peakLevel = 0.0f; g_peakHold = 0.0f; g_audioPeakHold = 0.0f; g_audioPeakTime = 0; g_bassLevel = 0.0f;
    if (hTimeCur)    SetWindowText(hTimeCur, L"0:00");
    if (hTimeTot)    SetWindowText(hTimeTot, L"/ 0:00");
    if (hTimeRemain) SetWindowText(hTimeRemain, L"");
    if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
    if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
    UpdatePlayBtn();
    UpdateStatusBar();
    SetWindowText(g_hwnd, APP_TITLE);
}

void CALLBACK EndSyncProc(HSYNC, DWORD, DWORD, void* user)
{
    PostMessage((HWND)user, WM_PLAYNEXT, 0, 0);
}

void UpdateVolume()
{
    if (currentStream)
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
    if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
    if (hVolPct) {
        wchar_t pct[16];
        swprintf_s(pct, L"%d%%", (int)(currentVolume * 100 + 0.5f));
        SetWindowText(hVolPct, pct);
    }
}

void SeekToSeconds(double sec)
{
    if (!currentStream) return;
    if (sec < 0) sec = 0;
    double lenS = BASS_ChannelBytes2Seconds(currentStream,
        BASS_ChannelGetLength(currentStream, BASS_POS_BYTE));
    if (sec > lenS) sec = lenS;
    BASS_ChannelSetPosition(currentStream,
        BASS_ChannelSeconds2Bytes(currentStream, sec), BASS_POS_BYTE);
    UpdateTimeDisplays();
    if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
}

static double GetTrackLength()
{
    if (!currentStream) return 1.0;
    double s = BASS_ChannelBytes2Seconds(currentStream,
        BASS_ChannelGetLength(currentStream, BASS_POS_BYTE));
    return (s > 0) ? s : 1.0;
}

static double GetPlayPos()
{
    if (!currentStream) return 0.0;
    return BASS_ChannelBytes2Seconds(currentStream,
        BASS_ChannelGetPosition(currentStream, BASS_POS_BYTE));
}

void UpdateTimeDisplays()
{
    if (!currentStream) return;
    double pos = GetPlayPos(), len = GetTrackLength(), rem = len - pos;
    wchar_t buf[32];
    if (hTimeCur) { FormatTime(pos, buf, _countof(buf)); SetWindowText(hTimeCur, buf); }
    if (hTimeRemain && rem >= 0) {
        wchar_t r[32]; FormatTime(rem, r, _countof(r));
        swprintf_s(buf, L"-%s", r); SetWindowText(hTimeRemain, buf);
    }
}

void UpdatePlayBtn()
{
    if (hPlayBtn)  InvalidateRect(hPlayBtn, NULL, TRUE);
    if (hPauseBtn) InvalidateRect(hPauseBtn, NULL, TRUE);
    if (hStopBtn)  InvalidateRect(hStopBtn, NULL, TRUE);
    if (hPrevBtn) InvalidateRect(hPrevBtn, NULL, TRUE);
    if (hNextBtn) InvalidateRect(hNextBtn, NULL, TRUE);
}

void UpdateWindowTitle()
{
    if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
        wchar_t buf[MAX_PATH + 32];
        swprintf_s(buf, L"%s - %s", g_playlist[g_currentIndex].display, APP_TITLE);
        SetWindowText(g_hwnd, buf);
    }
    else SetWindowText(g_hwnd, APP_TITLE);
}

void PlayIndex(int idx)
{
    if (idx < 0 || idx >= (int)g_playlist.size()) return;
    StopAudio();
    g_currentIndex = idx;
    SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
    SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)idx);
    SendMessage(hListBox, LB_SETCURSEL, idx, 0);
    SendMessage(hListBox, LB_SETTOPINDEX, max(0, idx - 3), 0);

    currentStream = BASS_StreamCreateFile(FALSE, g_playlist[idx].path, 0, 0, BASS_UNICODE | BASS_SAMPLE_FLOAT);
    if (!currentStream) {
        wchar_t msg[MAX_PATH + 80];
        swprintf_s(msg, L"Cannot open:\n%s\n\nBASS error: %d",
            g_playlist[idx].path, BASS_ErrorGetCode());
        MessageBox(g_hwnd, msg, L"Playback Error", MB_ICONERROR);
        return;
    }
    BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);

    double lenS = BASS_ChannelBytes2Seconds(currentStream,
        BASS_ChannelGetLength(currentStream, BASS_POS_BYTE));
    if (hTimeTot) {
        wchar_t buf[32], tot[40];
        FormatTime(lenS, buf, _countof(buf));
        swprintf_s(tot, L"/ %s", buf);
        SetWindowText(hTimeTot, tot);
    }
    BASS_ChannelSetSync(currentStream, BASS_SYNC_END | BASS_SYNC_MIXTIME, 0, EndSyncProc, g_hwnd);
    ApplyDSP();
    BASS_ChannelPlay(currentStream, FALSE);
    SetTimer(g_hwnd, IDT_PLAYBACK, 100, NULL);
    SetTimer(g_hwnd, IDT_PEAK_METER, 50, NULL);

    UpdatePlayBtn();
    UpdateStatusBar();
    UpdateWindowTitle();
    UpdateTimeDisplays();
    if (hSeekCanvas)   InvalidateRect(hSeekCanvas, NULL, FALSE);
    if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
}

void TogglePlayPause()
{
    if (!currentStream) {
        if (g_currentIndex >= 0) PlayIndex(g_currentIndex);
        else if (!g_playlist.empty()) PlayIndex(0);
        return;
    }
    if (BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING) {
        BASS_ChannelPause(currentStream);
        KillTimer(g_hwnd, IDT_PLAYBACK);
    }
    else {
        BASS_ChannelPlay(currentStream, FALSE);
        SetTimer(g_hwnd, IDT_PLAYBACK, 100, NULL);
    }
    UpdatePlayBtn();
    UpdateStatusBar();
}

void PlayNext()
{
    if (g_playlist.empty()) return;
    int next;
    if (g_shuffle) {
        int p = 0;
        for (int i = 0; i < (int)g_shuffleOrder.size(); ++i)
            if (g_shuffleOrder[i] == g_currentIndex) { p = i; break; }
        next = g_shuffleOrder[(p + 1) % g_shuffleOrder.size()];
    }
    else {
        next = g_currentIndex + 1;
        if (next >= (int)g_playlist.size()) {
            if (g_repeat) next = 0; else { StopAudio(); return; }
        }
    }
    PlayIndex(next);
}

void PlayPrev()
{
    if (g_playlist.empty()) return;
    if (currentStream && GetPlayPos() > 3.0) { SeekToSeconds(0); return; }
    int prev;
    if (g_shuffle) {
        int p = 0;
        for (int i = 0; i < (int)g_shuffleOrder.size(); ++i)
            if (g_shuffleOrder[i] == g_currentIndex) { p = i; break; }
        prev = g_shuffleOrder[(p - 1 + (int)g_shuffleOrder.size()) % g_shuffleOrder.size()];
    }
    else {
        prev = g_currentIndex - 1;
        if (prev < 0) prev = g_repeat ? (int)g_playlist.size() - 1 : 0;
    }
    PlayIndex(prev);
}

// ============================================================
//  System Tray Icon
// ============================================================
void AddTrayIcon(HWND hwnd)
{
    if (g_trayAdded) return;
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_BILLYPRO),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    if (!nid.hIcon)
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcsncpy_s(nid.szTip, L"Billy Pro", _TRUNCATE);
    Shell_NotifyIcon(NIM_ADD, &nid);
    g_trayAdded = true;
}

void RemoveTrayIcon()
{
    if (!g_trayAdded) return;
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_hwnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
    g_trayAdded = false;
}

void UpdateTrayTooltip()
{
    if (!g_trayAdded) return;
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_TIP;
    if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
        swprintf_s(nid.szTip, L"Billy Pro - %s", g_playlist[g_currentIndex].display);
    else
        wcsncpy_s(nid.szTip, L"Billy Pro", _TRUNCATE);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void ShowTrayContextMenu(HWND hwnd)
{
    POINT pt; GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_RESTORE, L"Restore Billy Pro");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_PLAY_PLAYPAUSE, L"Play / Pause");
    AppendMenu(hMenu, MF_STRING, IDM_PLAY_NEXT, L"Next");
    AppendMenu(hMenu, MF_STRING, IDM_PLAY_PREV, L"Previous");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
    SetForegroundWindow(hwnd); // required for menu to dismiss properly
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
}

// ============================================================
//  Status bar
// ============================================================
void UpdateStatusBar()
{
    if (!hStatus) return;
    wchar_t left[256] = L"Ready", right[128] = L"";

    if (!g_playlist.empty() && !currentStream)
        swprintf_s(left, L"%d Files", (int)g_playlist.size());

    if (currentStream) {
        if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
            swprintf_s(left, L"%d / %d  %s",
                g_currentIndex + 1, (int)g_playlist.size(),
                g_playlist[g_currentIndex].display);
        BASS_CHANNELINFO info = {};
        if (BASS_ChannelGetInfo(currentStream, &info)) {
            float brate = 0;
            BASS_ChannelGetAttribute(currentStream, BASS_ATTRIB_BITRATE, &brate);

            // Determine format from file extension (most reliable for FLAC/WAV)
            const wchar_t* fmt = L"Audio";
            if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
                const wchar_t* ext = wcsrchr(g_playlist[g_currentIndex].path, L'.');
                if (ext) {
                    if (_wcsicmp(ext, L".mp3") == 0) fmt = L"MP3";
                    else if (_wcsicmp(ext, L".flac") == 0) fmt = L"FLAC";
                    else if (_wcsicmp(ext, L".wav") == 0) fmt = L"WAV";
                    else if (_wcsicmp(ext, L".ogg") == 0) fmt = L"OGG";
                    else if (_wcsicmp(ext, L".m4a") == 0) fmt = L"M4A";
                    else if (_wcsicmp(ext, L".aac") == 0) fmt = L"AAC";
                    else if (_wcsicmp(ext, L".wma") == 0) fmt = L"WMA";
                    else if (_wcsicmp(ext, L".opus") == 0) fmt = L"OPUS";
                    else if (_wcsicmp(ext, L".ape") == 0) fmt = L"APE";
                    else if (_wcsicmp(ext, L".aiff") == 0) fmt = L"AIFF";
                }
            }
            // Fallback to BASS ctype if extension didn't match
            if (wcscmp(fmt, L"Audio") == 0) {
                if (info.ctype & BASS_CTYPE_STREAM_MP3)  fmt = L"MP3";
                else if (info.ctype & BASS_CTYPE_STREAM_OGG)  fmt = L"OGG";
                else if (info.ctype & BASS_CTYPE_STREAM_WAV)  fmt = L"WAV";
                else if (info.ctype & BASS_CTYPE_STREAM_AIFF) fmt = L"AIFF";
                else if (info.ctype & BASS_CTYPE_STREAM_CA)   fmt = L"AAC";
            }

            const wchar_t* ch = (info.chans == 2) ? L"Stereo" : (info.chans == 1) ? L"Mono" : L"Multi";

            // For WAV: show bit depth
            if (wcscmp(fmt, L"WAV") == 0) {
                int bits = (int)info.origres; // BASS stores original bit depth here
                if (bits <= 0) bits = (info.flags & BASS_SAMPLE_FLOAT) ? 32 :
                    (info.flags & BASS_SAMPLE_8BITS) ? 8 : 16;
                swprintf_s(right, L"WAV %d-bit  %d Hz  %s", bits, info.freq, ch);
            }
            else if (brate > 0)
                swprintf_s(right, L"%s %.0f kbps  %d Hz  %s", fmt, brate, info.freq, ch);
            else
                swprintf_s(right, L"%s  %d Hz  %s", fmt, info.freq, ch);
        }
        if (BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PAUSED) {
            wchar_t tmp[256]; swprintf_s(tmp, L"%s  [Paused]", right);
            wcsncpy_s(right, tmp, _TRUNCATE);
        }
    }
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)left);
    SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)right);
    UpdateTrayTooltip();
}

// ============================================================
//  Custom seek bar
// ============================================================
static double GetPlaybackFrac()
{
    if (!currentStream) return 0.0;
    QWORD pos = BASS_ChannelGetPosition(currentStream, BASS_POS_BYTE);
    QWORD len = BASS_ChannelGetLength(currentStream, BASS_POS_BYTE);
    return (len > 0) ? (double)pos / (double)len : 0.0;
}

static double FracFromX(HWND hwnd, int x)
{
    RECT rc; GetClientRect(hwnd, &rc);
    if (rc.right <= 0) return 0.0;
    double f = (double)x / (double)rc.right;
    return (f < 0.0) ? 0.0 : (f > 1.0) ? 1.0 : f;
}

static void PaintSeekBar(HWND hwnd)
{
    PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps);
    RECT rc; GetClientRect(hwnd, &rc);
    int W = rc.right, H = rc.bottom;

    // Background (track)
    HBRUSH brT = CreateSolidBrush(g_theme.seekTrk);
    FillRect(dc, &rc, brT); DeleteObject(brT);

    double frac = g_seekDragging ? (g_seekDragPos / GetTrackLength()) : GetPlaybackFrac();
    if (frac < 0) frac = 0; if (frac > 1) frac = 1;
    int fillW = (int)(frac * W);
    if (fillW > 0) {
        RECT fr = { 0, 0, fillW, H };
        HBRUSH brF = CreateSolidBrush(g_theme.accent);
        FillRect(dc, &fr, brF); DeleteObject(brF);
    }

    // Thin border
    HPEN pen = CreatePen(PS_SOLID, 1, 0xAAAAAA);
    HPEN op = (HPEN)SelectObject(dc, pen);
    SelectObject(dc, GetStockObject(NULL_BRUSH));
    Rectangle(dc, 0, 0, W, H);
    SelectObject(dc, op); DeleteObject(pen);

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK SeekBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_PAINT:       PaintSeekBar(hwnd); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_LBUTTONDOWN: {
        if (!currentStream) return 0;
        SetCapture(hwnd); g_seekDragging = true;
        g_seekDragPos = FracFromX(hwnd, GET_X_LPARAM(lParam)) * GetTrackLength();
        wchar_t buf[16]; FormatTime(g_seekDragPos, buf, _countof(buf));
        SetWindowText(hTimeCur, buf);
        InvalidateRect(hwnd, NULL, FALSE); return 0;
    }
    case WM_MOUSEMOVE:
        if (g_seekDragging && (wParam & MK_LBUTTON)) {
            g_seekDragPos = FracFromX(hwnd, GET_X_LPARAM(lParam)) * GetTrackLength();
            wchar_t buf[16]; FormatTime(g_seekDragPos, buf, _countof(buf));
            SetWindowText(hTimeCur, buf);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    case WM_LBUTTONUP:
        if (g_seekDragging) {
            ReleaseCapture(); g_seekDragging = false;
            SeekToSeconds(FracFromX(hwnd, GET_X_LPARAM(lParam)) * GetTrackLength());
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    case WM_CAPTURECHANGED:
        if (g_seekDragging) { g_seekDragging = false; InvalidateRect(hwnd, NULL, FALSE); }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//  Volume bar  -  VERTICAL, fills bottom-to-top (Billy style)
// ============================================================
static void PaintVolBar(HWND hwnd)
{
    PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps);
    RECT rc; GetClientRect(hwnd, &rc);
    int W = rc.right, H = rc.bottom;

    // Background
    HBRUSH brBg = CreateSolidBrush(g_theme.bg);
    FillRect(dc, &rc, brBg); DeleteObject(brBg);

    // Draw segmented LED-style squares like Billy
    // Each segment: small square with 1px gap between
    const int segH = 3;   // height of each square segment
    const int gap = 1;   // gap between segments
    int step = segH + gap;
    int numSegs = H / step;
    if (numSegs < 1) numSegs = 1;

    int volSegs = (int)(currentVolume * numSegs);
    int peakSegs = (int)(g_peakLevel * numSegs);
    int bassSegs = (int)(g_bassLevel * numSegs);

    for (int i = 0; i < numSegs; i++) {
        // i=0 is bottom segment, i=numSegs-1 is top
        int segY = H - (i + 1) * step + gap;
        RECT sr = { 1, segY, W - 1, segY + segH };

        COLORREF col;
        if (i < volSegs) {
            // Volume fill: blue
            col = g_theme.accent;
        }
        else if (currentStream && i < peakSegs) {
            // Peak level above volume: green
            col = 0x20A020;
        }
        else {
            // Empty segment: dim square outline only
            HPEN pen = CreatePen(PS_SOLID, 1, 0xBBBBBB);
            HPEN op = (HPEN)SelectObject(dc, pen);
            SelectObject(dc, GetStockObject(NULL_BRUSH));
            Rectangle(dc, sr.left, sr.top, sr.right, sr.bottom);
            SelectObject(dc, op); DeleteObject(pen);
            continue;
        }
        // Override bottom segments with red if bass is active
        if (currentStream && i < bassSegs)
            col = 0x0000CC;  // red (BGR) for bass
        HBRUSH br = CreateSolidBrush(col);
        FillRect(dc, &sr, br); DeleteObject(br);
    }

    EndPaint(hwnd, &ps);
}

static void VolumeFromY(HWND hwnd, int y)
{
    RECT rc; GetClientRect(hwnd, &rc);
    int H = rc.bottom;
    if (H <= 0) return;
    // y=0 is top (max volume), y=H is bottom (zero)
    float f = 1.0f - (float)y / (float)H;
    if (f < 0) f = 0; if (f > 1) f = 1;
    currentVolume = f;
    if (currentVolume > g_peakHold) g_peakHold = currentVolume;
    UpdateVolume();
}


LRESULT CALLBACK VolBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_PAINT:       PaintVolBar(hwnd); return 0;
    case WM_ERASEBKGND: return 1;

    case WM_LBUTTONDOWN:
        SetCapture(hwnd); g_volDragging = true;
        VolumeFromY(hwnd, GET_Y_LPARAM(lParam));
        SetTimer(hwnd, IDT_PEAK_DECAY, 50, NULL);
        return 0;

    case WM_MOUSEMOVE:
        if (g_volDragging && (wParam & MK_LBUTTON))
            VolumeFromY(hwnd, GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONUP:
        if (g_volDragging) {
            ReleaseCapture(); g_volDragging = false;
            VolumeFromY(hwnd, GET_Y_LPARAM(lParam));
            g_peakHold = currentVolume;
        }
        return 0;

    case WM_CAPTURECHANGED:
        g_volDragging = false;
        return 0;

    case WM_TIMER:
        if (wParam == IDT_PEAK_DECAY) {
            if (g_peakHold > currentVolume + 0.005f) {
                g_peakHold -= 0.012f;
                if (g_peakHold < currentVolume) g_peakHold = currentVolume;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else {
                g_peakHold = currentVolume;
                KillTimer(hwnd, IDT_PEAK_DECAY);
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        currentVolume += (delta > 0) ? 0.05f : -0.05f;
        if (currentVolume < 0) currentVolume = 0;
        if (currentVolume > 1) currentVolume = 1;
        if (currentVolume > g_peakHold) {
            g_peakHold = currentVolume;
            SetTimer(hwnd, IDT_PEAK_DECAY, 50, NULL);
        }
        UpdateVolume();
        return 0;
    }

    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return TRUE;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//  Transport buttons (GDI polygon drawing)
// ============================================================
static void FillCentered(HDC dc, RECT rc, int w, int h, COLORREF c)
{
    int cx = (rc.left + rc.right) / 2, cy = (rc.top + rc.bottom) / 2;
    RECT r = { cx - w / 2,cy - h / 2,cx - w / 2 + w,cy - h / 2 + h };
    HBRUSH br = CreateSolidBrush(c); FillRect(dc, &r, br); DeleteObject(br);
}
static void TriRight(HDC dc, int cx, int cy, int half, COLORREF c)
{
    POINT p[3] = { {cx - half,cy - half},{cx - half,cy + half},{cx + half,cy} };
    HBRUSH br = CreateSolidBrush(c); HPEN pn = CreatePen(PS_SOLID, 1, c);
    HBRUSH ob = (HBRUSH)SelectObject(dc, br); HPEN op = (HPEN)SelectObject(dc, pn);
    Polygon(dc, p, 3); SelectObject(dc, ob); SelectObject(dc, op);
    DeleteObject(br); DeleteObject(pn);
}
static void TriLeft(HDC dc, int cx, int cy, int half, COLORREF c)
{
    POINT p[3] = { {cx + half,cy - half},{cx + half,cy + half},{cx - half,cy} };
    HBRUSH br = CreateSolidBrush(c); HPEN pn = CreatePen(PS_SOLID, 1, c);
    HBRUSH ob = (HBRUSH)SelectObject(dc, br); HPEN op = (HPEN)SelectObject(dc, pn);
    Polygon(dc, p, 3); SelectObject(dc, ob); SelectObject(dc, op);
    DeleteObject(br); DeleteObject(pn);
}
static void VBar(HDC dc, int cx, int cy, int h2, COLORREF c)
{
    RECT r = { cx - 2,cy - h2,cx + 2,cy + h2 + 1 };
    HBRUSH br = CreateSolidBrush(c); FillRect(dc, &r, br); DeleteObject(br);
}

enum BtnType { BTN_PREV, BTN_PLAY, BTN_PAUSE, BTN_STOP, BTN_NEXT, BTN_SHUFFLE, BTN_REPEAT, BTN_MONO, BTN_NORMALIZE, BTN_BASSBOOST };

static void DrawBtn(DRAWITEMSTRUCT* dis, BtnType type)
{
    HDC  dc = dis->hDC; RECT rc = dis->rcItem;
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    bool toggled = (type == BTN_SHUFFLE && g_shuffle)
        || (type == BTN_REPEAT && g_repeat)
        || (type == BTN_MONO && g_mono)
        || (type == BTN_NORMALIZE && g_normalize)
        || (type == BTN_BASSBOOST && g_bassBoost)
        ;

    // No border - Billy style. Hover = light blue tint, pressed = slightly darker.
    bool hovered = (g_hoveredBtn == dis->hwndItem);
    COLORREF face = toggled ? g_theme.accent
        : pressed ? (0xC8DCF4)
        : hovered ? (0xF0D2B4)
        : g_theme.bg;
    HBRUSH br = CreateSolidBrush(face); FillRect(dc, &rc, br); DeleteObject(br);
    // No border drawn for transport buttons (Shuffle/Repeat keep theirs below)

    int cx = (rc.left + rc.right) / 2, cy = (rc.top + rc.bottom) / 2;
    COLORREF sym = (pressed || toggled) ? 0xFFFFFF : 0x404040;
    int h = 4; // symbol half-size

    switch (type) {
    case BTN_PREV: {
        // |< : thin bar, gap, slim triangle pointing left
        RECT bar = { cx - 5, cy - h, cx - 4, cy + h + 1 };
        HBRUSH b = CreateSolidBrush(sym); FillRect(dc, &bar, b); DeleteObject(b);
        // slim triangle: narrow width relative to height
        POINT p[3] = { {cx + 2,cy - h},{cx + 2,cy + h},{cx - 2,cy} };
        HBRUSH tb = CreateSolidBrush(sym); HPEN tp = CreatePen(PS_SOLID, 1, sym);
        HBRUSH ob = (HBRUSH)SelectObject(dc, tb); HPEN op = (HPEN)SelectObject(dc, tp);
        Polygon(dc, p, 3);
        SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(tb); DeleteObject(tp);
        break;
    }
    case BTN_PLAY: {
        // Same size as other symbols - simple right triangle, h=4 like rest
        POINT p[3] = { {cx - 3, cy - h}, {cx - 3, cy + h}, {cx + h, cy} };
        HBRUSH tb = CreateSolidBrush(sym); HPEN tp = CreatePen(PS_SOLID, 1, sym);
        HBRUSH ob = (HBRUSH)SelectObject(dc, tb); HPEN op = (HPEN)SelectObject(dc, tp);
        Polygon(dc, p, 3);
        SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(tb); DeleteObject(tp);
        break;
    }
    case BTN_PAUSE: {
        RECT r1 = { cx - 4,cy - h,cx - 1,cy + h + 1 }, r2 = { cx + 1,cy - h,cx + 4,cy + h + 1 };
        HBRUSH pb = CreateSolidBrush(sym);
        FillRect(dc, &r1, pb); FillRect(dc, &r2, pb); DeleteObject(pb);
        break;
    }
    case BTN_STOP:
    { RECT sq = { cx - 4, cy - 4, cx + 4, cy + 5 }; HBRUSH b = CreateSolidBrush(sym); FillRect(dc, &sq, b); DeleteObject(b); }
    break;
    case BTN_NEXT: {
        // >| : slim triangle pointing right, gap, thin bar
        POINT p[3] = { {cx - 2,cy - h},{cx - 2,cy + h},{cx + 2,cy} };
        HBRUSH tb = CreateSolidBrush(sym); HPEN tp = CreatePen(PS_SOLID, 1, sym);
        HBRUSH ob = (HBRUSH)SelectObject(dc, tb); HPEN op = (HPEN)SelectObject(dc, tp);
        Polygon(dc, p, 3);
        SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(tb); DeleteObject(tp);
        RECT bar = { cx + 4, cy - h, cx + 5, cy + h + 1 };
        HBRUSH b = CreateSolidBrush(sym); FillRect(dc, &bar, b); DeleteObject(b);
        break;
    }
    case BTN_MONO:
    case BTN_NORMALIZE:
    case BTN_BASSBOOST:
    case BTN_SHUFFLE:
    case BTN_REPEAT: {
        sym = (pressed || toggled) ? 0xFFFFFF : 0x000000; // black text for these
        // Shuffle/Repeat keep a visible border
        HPEN spn = CreatePen(PS_SOLID, 1, g_theme.btnBorder);
        HPEN sop = (HPEN)SelectObject(dc, spn);
        SelectObject(dc, GetStockObject(NULL_BRUSH));
        Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
        SelectObject(dc, sop); DeleteObject(spn);
        SetBkMode(dc, TRANSPARENT); SetTextColor(dc, sym);
        HFONT hf = CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HFONT of = (HFONT)SelectObject(dc, hf);
        const wchar_t* lbl = L"";
        switch (type) {
        case BTN_SHUFFLE:    lbl = L"Shuffle"; break;
        case BTN_REPEAT:     lbl = L"Repeat";  break;
        case BTN_MONO:       lbl = L"Mono";    break;
        case BTN_NORMALIZE: lbl = L"Norm";    break;
        case BTN_BASSBOOST:  lbl = L"BassBoost"; break;
        default:             lbl = L"";        break;
        }
        DrawText(dc, lbl, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, of); DeleteObject(hf);
        break;
    }
    }
}

// ============================================================
//  Button subclass - enables hot tracking for hover highlight
// ============================================================
LRESULT CALLBACK BtnHotProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_MOUSEMOVE && g_hoveredBtn != hwnd) {
        g_hoveredBtn = hwnd;
        TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
        TrackMouseEvent(&tme);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
    if (msg == WM_MOUSELEAVE) {
        g_hoveredBtn = NULL;
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
    return CallWindowProc(g_OldBtnProc, hwnd, msg, wParam, lParam);
}

// ============================================================
//  Listbox subclass  (keyboard + right-click + drag-reorder)
// ============================================================

// Helper: hit-test listbox item at client coords
static int LBHitTest(HWND hwnd, int y)
{
    int idx = (int)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(0, y));
    if (HIWORD(idx)) return LB_ERR; // outside
    int count = (int)SendMessage(hwnd, LB_GETCOUNT, 0, 0);
    if (LOWORD(idx) >= (UINT)count) return LB_ERR;
    return LOWORD(idx);
}

// Move a track from srcIdx to destIdx in both g_playlist and the listbox
static void ReorderTrack(int srcIdx, int destIdx)
{
    if (srcIdx == destIdx) return;
    int count = (int)g_playlist.size();
    if (srcIdx < 0 || srcIdx >= count || destIdx < 0 || destIdx >= count) return;

    // Move in playlist vector
    Track t = g_playlist[srcIdx];
    g_playlist.erase(g_playlist.begin() + srcIdx);
    g_playlist.insert(g_playlist.begin() + destIdx, t);

    // Rebuild listbox entirely (simplest & correct)
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (auto& tr : g_playlist)
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)tr.display);

    // Update g_currentIndex to follow the moved track
    if (g_currentIndex == srcIdx) {
        g_currentIndex = destIdx;
    }
    else if (srcIdx < destIdx) {
        if (g_currentIndex > srcIdx && g_currentIndex <= destIdx) g_currentIndex--;
    }
    else {
        if (g_currentIndex >= destIdx && g_currentIndex < srcIdx) g_currentIndex++;
    }

    SendMessage(hListBox, LB_SETCURSEL, destIdx, 0);
    RebuildShuffleOrder();
    UpdateStatusBar();
}

LRESULT CALLBACK ListBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // ── Keyboard ──────────────────────────────────────────────
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_PRIOR || wParam == VK_NEXT)
            return CallWindowProc(g_OldListProc, hwnd, msg, wParam, lParam);

        if (wParam == VK_RETURN) {
            int s = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0);
            if (s != LB_ERR) PlayIndex(s);
            return 0;
        }
        if (wParam == VK_DELETE) {
            int total = (int)SendMessage(hwnd, LB_GETCOUNT, 0, 0);
            std::vector<int> toDelete;
            for (int i = 0; i < total; i++)
                if (SendMessage(hwnd, LB_GETSEL, i, 0) > 0)
                    toDelete.push_back(i);
            if (toDelete.empty()) {
                int s = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0);
                if (s != LB_ERR) toDelete.push_back(s);
            }
            std::sort(toDelete.begin(), toDelete.end(), std::greater<int>());
            bool stopNeeded = false;
            for (int idx : toDelete) {
                if (idx >= 0 && idx < (int)g_playlist.size()) {
                    if (idx == g_currentIndex) stopNeeded = true;
                    g_playlist.erase(g_playlist.begin() + idx);
                    SendMessage(hwnd, LB_DELETESTRING, idx, 0);
                }
            }
            if (stopNeeded) StopAudio();
            RebuildShuffleOrder();
            g_currentIndex = min(g_currentIndex, (int)g_playlist.size() - 1);
            if (!g_playlist.empty())
                SendMessage(hwnd, LB_SETCURSEL, max(0, g_currentIndex), 0);
            UpdateStatusBar();
            return 0;
        }
        if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hwnd, LB_SETSEL, TRUE, (LPARAM)-1);
            UpdateStatusBar();
            return 0;
        }
        SendMessage(g_hwnd, WM_KEYDOWN, wParam, lParam);
        return 0;
    }

    // ── Left button: drag-reorder (plain click) vs range select (Shift) ──
    if (msg == WM_LBUTTONDOWN) {
        bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        if (!shiftDown && !ctrlDown) {
            // Start potential drag
            int idx = LBHitTest(hwnd, GET_Y_LPARAM(lParam));
            if (idx != LB_ERR) {
                g_lbDragging = true;
                g_lbDragFrom = idx;
                g_lbDragCur = idx;
                g_lbDragMoved = false;
                SetCapture(hwnd);
                // Select this item
                SendMessage(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
                SendMessage(hwnd, LB_SETSEL, TRUE, idx);
                SendMessage(hwnd, LB_SETCURSEL, idx, 0);
                return 0;
            }
        }
        // Shift/Ctrl: let default handle range/toggle selection
        return CallWindowProc(g_OldListProc, hwnd, msg, wParam, lParam);
    }

    if (msg == WM_MOUSEMOVE) {
        if (g_lbDragging && (wParam & MK_LBUTTON)) {
            int idx = LBHitTest(hwnd, GET_Y_LPARAM(lParam));
            if (idx == LB_ERR) {
                // Clamp to first/last
                RECT rc; GetClientRect(hwnd, &rc);
                idx = (GET_Y_LPARAM(lParam) < 0) ? 0 : (int)g_playlist.size() - 1;
            }
            if (idx != g_lbDragCur) {
                g_lbDragMoved = true;
                g_lbDragCur = idx;
                // Show drag destination by highlighting
                SendMessage(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
                SendMessage(hwnd, LB_SETSEL, TRUE, idx);
                SendMessage(hwnd, LB_SETCURSEL, idx, 0);
            }
            // Change cursor to indicate drag
            SetCursor(LoadCursor(NULL, IDC_SIZENS));
            return 0;
        }
        return CallWindowProc(g_OldListProc, hwnd, msg, wParam, lParam);
    }

    if (msg == WM_LBUTTONUP) {
        if (g_lbDragging) {
            ReleaseCapture();
            int from = g_lbDragFrom;
            int to = g_lbDragCur;
            g_lbDragging = false;
            if (g_lbDragMoved && from != to && from >= 0 && to >= 0) {
                ReorderTrack(from, to);
            }
            else if (!g_lbDragMoved) {
                // Plain click (no drag): play on double-click is handled by LBN_DBLCLK
                // Just keep the selection on the clicked item
                SendMessage(hwnd, LB_SETCURSEL, from, 0);
            }
            return 0;
        }
        return CallWindowProc(g_OldListProc, hwnd, msg, wParam, lParam);
    }

    if (msg == WM_CAPTURECHANGED) {
        g_lbDragging = false;
        return CallWindowProc(g_OldListProc, hwnd, msg, wParam, lParam);
    }

    // ── Right-click: context menu ──────────────────────────────
    if (msg == WM_RBUTTONDOWN) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int idx = (int)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
        if (HIWORD(idx)) idx = LB_ERR;
        if (idx != LB_ERR && idx < (int)g_playlist.size()) {
            if (SendMessage(hwnd, LB_GETSEL, idx, 0) == 0) {
                SendMessage(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
                SendMessage(hwnd, LB_SETSEL, TRUE, idx);
            }
            SendMessage(hwnd, LB_SETCURSEL, idx, 0);
            POINT screen = pt; ClientToScreen(hwnd, &screen);
            ShowTrackContextMenu(hwnd, idx, screen);
        }
        return 0;
    }

    return CallWindowProc(g_OldListProc, hwnd, msg, wParam, lParam);
}

// ============================================================
//  Context menu for right-click on track
// ============================================================
void ShowTrackContextMenu(HWND hwnd, int trackIdx, POINT pt)
{
    if (trackIdx < 0 || trackIdx >= (int)g_playlist.size()) return;
    g_ctxTrackIndex = trackIdx;

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, IDC_CTX_PLAY, L"Play");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDC_CTX_AUDIOINFO, L"Audio Information...");
    AppendMenu(hMenu, MF_STRING, IDC_CTX_PROPERTIES, L"Properties...");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDC_CTX_REMOVE, L"Remove from playlist");

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_hwnd, NULL);
    DestroyMenu(hMenu);
}

// ============================================================
//  Audio Information dialog
//  Shows: metadata (ID3 tags), format info, embedded artwork
// ============================================================

// Helper: read a BASS tag string (UTF-8 or ASCII) into wchar
static void TagToWide(const char* tag, wchar_t* out, size_t n)
{
    if (!tag || !tag[0]) { wcsncpy_s(out, n, L"", _TRUNCATE); return; }
    MultiByteToWideChar(CP_UTF8, 0, tag, -1, out, (int)n);
}

struct AudioInfoData {
    int trackIdx;
};

// Artwork raw bytes for drag/drop export
static BYTE* g_artBytes = NULL;
static DWORD  g_artSize = 0;
static wchar_t g_artExt[8] = L".jpg";

static void FreeArtBytes() { if (g_artBytes) { delete[]g_artBytes; g_artBytes = NULL; g_artSize = 0; } }

// Save embedded artwork to a temp file and return path
static bool SaveArtToTemp(wchar_t* outPath, size_t n)
{
    if (!g_artBytes || !g_artSize) return false;
    wchar_t tmp[MAX_PATH]; GetTempPath(MAX_PATH, tmp);
    swprintf_s(outPath, n, L"%sbilly_cover%s", tmp, g_artExt);
    HANDLE hf = CreateFile(outPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    DWORD written; WriteFile(hf, g_artBytes, g_artSize, &written, NULL);
    CloseHandle(hf);
    return written == g_artSize;
}

// Load artwork bitmap at a given size
static HBITMAP LoadEmbeddedArtworkSized(HSTREAM stream, int size)
{
    const void* id3v2 = BASS_ChannelGetTags(stream, BASS_TAG_ID3V2);
    if (!id3v2) return NULL;
    const BYTE* data = (const BYTE*)id3v2;
    if (memcmp(data, "ID3", 3) != 0) return NULL;
    DWORD tagSize = ((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) |
        ((data[8] & 0x7F) << 7) | (data[9] & 0x7F);
    BYTE ver = data[3];
    const BYTE* p = data + 10;
    const BYTE* end = data + 10 + tagSize;
    while (p + 10 < end) {
        char fid[5] = { 0 }; memcpy(fid, p, 4);
        DWORD fsize;
        if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
        else        fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
        p += 10;
        if (p + fsize > end) break;
        if (strcmp(fid, "APIC") == 0 && fsize > 4) {
            const BYTE* fp = p;
            fp++;
            // Detect mime for extension
            const char* mimeStart = (const char*)fp;
            while (fp < p + fsize && *fp) fp++;
            if (fp < p + fsize) {
                if (strstr(mimeStart, "png")) wcscpy_s(g_artExt, L".png");
                else wcscpy_s(g_artExt, L".jpg");
                fp++;
            }
            if (fp < p + fsize) fp++;
            while (fp < p + fsize && *fp) fp++;
            if (fp < p + fsize) fp++;
            DWORD imgSize = (DWORD)(p + fsize - fp);
            if (imgSize > 0) {
                // Save raw bytes for drag/drop
                FreeArtBytes();
                g_artBytes = new BYTE[imgSize];
                memcpy(g_artBytes, fp, imgSize);
                g_artSize = imgSize;
                // Decode to bitmap
                HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, imgSize);
                if (hg) {
                    void* mem = GlobalLock(hg);
                    if (mem) {
                        memcpy(mem, fp, imgSize);
                        GlobalUnlock(hg);
                        IStream* pStream = NULL;
                        if (SUCCEEDED(CreateStreamOnHGlobal(hg, FALSE, &pStream))) {
                            IWICImagingFactory* pFactory = NULL;
                            CoCreateInstance(CLSID_WICImagingFactory, NULL,
                                CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&pFactory);
                            if (pFactory) {
                                IWICBitmapDecoder* pDecoder = NULL;
                                pFactory->CreateDecoderFromStream(pStream, NULL,
                                    WICDecodeMetadataCacheOnLoad, &pDecoder);
                                if (pDecoder) {
                                    IWICBitmapFrameDecode* pFrame = NULL;
                                    pDecoder->GetFrame(0, &pFrame);
                                    if (pFrame) {
                                        IWICBitmapScaler* pScaler = NULL;
                                        pFactory->CreateBitmapScaler(&pScaler);
                                        if (pScaler) {
                                            pScaler->Initialize(pFrame, size, size,
                                                WICBitmapInterpolationModeCubic);
                                            IWICFormatConverter* pConv = NULL;
                                            pFactory->CreateFormatConverter(&pConv);
                                            if (pConv) {
                                                pConv->Initialize(pScaler,
                                                    GUID_WICPixelFormat32bppBGRA,
                                                    WICBitmapDitherTypeNone, NULL, 0.0,
                                                    WICBitmapPaletteTypeCustom);
                                                BYTE* bits = NULL;
                                                HDC hdc = GetDC(NULL);
                                                BITMAPINFO bmi = {};
                                                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                                                bmi.bmiHeader.biWidth = size;
                                                bmi.bmiHeader.biHeight = -size;
                                                bmi.bmiHeader.biPlanes = 1;
                                                bmi.bmiHeader.biBitCount = 32;
                                                bmi.bmiHeader.biCompression = BI_RGB;
                                                HBITMAP hbm = CreateDIBSection(hdc, &bmi,
                                                    DIB_RGB_COLORS, (void**)&bits, NULL, 0);
                                                ReleaseDC(NULL, hdc);
                                                if (hbm && bits) {
                                                    UINT stride = size * 4;
                                                    pConv->CopyPixels(NULL, stride, stride * size, bits);
                                                    pConv->Release(); pScaler->Release();
                                                    pFrame->Release(); pDecoder->Release();
                                                    pFactory->Release(); pStream->Release();
                                                    GlobalFree(hg);
                                                    return hbm;
                                                }
                                                if (pConv) pConv->Release();
                                            }
                                            pScaler->Release();
                                        }
                                        pFrame->Release();
                                    }
                                    pDecoder->Release();
                                }
                                pFactory->Release();
                            }
                            pStream->Release();
                        }
                    }
                    GlobalFree(hg);
                }
            }
        }
        p += fsize;
    }
    return NULL;
}

static LRESULT CALLBACK InfoWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hArt = NULL;
    static int     s_trackIdx = -1;
    static bool    s_artDragging = false;

    switch (msg) {
    case WM_CREATE: {
        AudioInfoData* d = (AudioInfoData*)((CREATESTRUCT*)lParam)->lpCreateParams;
        s_trackIdx = d->trackIdx;
        FreeArtBytes();

        HSTREAM s = BASS_StreamCreateFile(FALSE, g_playlist[s_trackIdx].path, 0, 0,
            BASS_UNICODE | BASS_STREAM_DECODE);

        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

        // === Layout: 560x520 ===
        // Left col: artwork 220x220 + drag hint + Save/Replace buttons
        // Right col: format info (read-only) + editable tag fields
        // Bottom: Save Tags + Close

        int artW = 220, artH = 220;
        int lx = 10, rx = artW + 20, rw = 560 - rx - 10;

        // Artwork panel
        CreateWindow(L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
            lx, 10, artW, artH, hwnd, (HMENU)ID_INFO_ARTWORK, hInst, NULL);

        // Art hint label
        HWND hHint = CreateWindow(L"STATIC", L"Drag artwork to desktop to extract",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            lx, artH + 14, artW, 16, hwnd, (HMENU)(UINT_PTR)(IDC_STATIC), hInst, NULL);
        SendMessage(hHint, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Save Artwork button
        HWND hSaveArt = CreateWindow(L"BUTTON", L"Save Artwork...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            lx, artH + 34, artW / 2 - 2, 24, hwnd, (HMENU)ID_INFO_SAVE_ART, hInst, NULL);
        SendMessage(hSaveArt, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Replace Artwork button
        HWND hRepArt = CreateWindow(L"BUTTON", L"Replace...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            lx + artW / 2 + 2, artH + 34, artW / 2 - 2, 24, hwnd, (HMENU)ID_INFO_REPLACE_ART, hInst, NULL);
        SendMessage(hRepArt, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // === Right side: format info read-only ===
        int ry = 10;
        // Format info as read-only edit
        wchar_t fmtInfo[512] = L"";
        if (s) {
            BASS_CHANNELINFO ci = {}; BASS_ChannelGetInfo(s, &ci);
            float brate = 0; BASS_ChannelGetAttribute(s, BASS_ATTRIB_BITRATE, &brate);
            const wchar_t* fmt = L"Audio";
            if (ci.ctype & BASS_CTYPE_STREAM_MP3)  fmt = L"MP3";
            else if (ci.ctype & BASS_CTYPE_STREAM_OGG)  fmt = L"OGG";
            else if (ci.ctype & BASS_CTYPE_STREAM_WAV)  fmt = L"WAV";
            else if (ci.ctype & BASS_CTYPE_STREAM_AIFF) fmt = L"AIFF";
            else if (ci.ctype & BASS_CTYPE_STREAM_CA)   fmt = L"AAC/M4A";
            const wchar_t* ch = (ci.chans == 2) ? L"Stereo" : (ci.chans == 1) ? L"Mono" : L"Multi";
            double lenS = BASS_ChannelBytes2Seconds(s, BASS_ChannelGetLength(s, BASS_POS_BYTE));
            int mm = (int)(lenS / 60), ss = (int)lenS % 60;

            WIN32_FILE_ATTRIBUTE_DATA fa = {};
            GetFileAttributesEx(g_playlist[s_trackIdx].path, GetFileExInfoStandard, &fa);
            ULONGLONG fsz = ((ULONGLONG)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;

            if (brate > 0)
                swprintf_s(fmtInfo, L"Format: %s  %.0f kbps  %s\r\nSample Rate: %d Hz   Length: %d:%02d\r\nFile Size: %.2f MB",
                    fmt, brate, ch, ci.freq, mm, ss, fsz / 1048576.0);
            else
                swprintf_s(fmtInfo, L"Format: %s  %s\r\nSample Rate: %d Hz   Length: %d:%02d\r\nFile Size: %.2f MB",
                    fmt, ch, ci.freq, mm, ss, fsz / 1048576.0);
        }
        HWND hFmtEdit = CreateWindowEx(0, L"EDIT", fmtInfo,
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
            rx, ry, rw, 52, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hFmtEdit, WM_SETFONT, (WPARAM)g_fontMono, TRUE);
        ry += 58;

        // File path (read-only, full path)
        HWND hPath = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", g_playlist[s_trackIdx].path,
            WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL,
            rx, ry, rw, 22, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hPath, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        ry += 30;

        // === Editable tag fields ===
        // Helper lambda-style: label + edit
        auto MakeField = [&](const wchar_t* label, UINT id, int h)->HWND {
            HWND hLbl = CreateWindow(L"STATIC", label,
                WS_CHILD | WS_VISIBLE,
                rx, ry, 60, 16, hwnd, (HMENU)(UINT_PTR)IDC_STATIC, hInst, NULL);
            SendMessage(hLbl, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
            HWND hEd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | (h > 22 ? ES_MULTILINE | WS_VSCROLL : 0),
                rx + 64, ry, rw - 64, h, hwnd, (HMENU)(UINT_PTR)id, hInst, NULL);
            SendMessage(hEd, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            ry += (h + 6);
            return hEd;
            };

        HWND hTitle = MakeField(L"Title:", ID_INFO_TITLE, 22);
        HWND hArtist = MakeField(L"Artist:", ID_INFO_ARTIST, 22);
        HWND hAlbum = MakeField(L"Album:", ID_INFO_ALBUM, 22);
        HWND hYear = MakeField(L"Year:", ID_INFO_YEAR, 22);
        HWND hTrack = MakeField(L"Track #:", ID_INFO_TRACK, 22);
        HWND hGenre = MakeField(L"Genre:", ID_INFO_GENRE, 22);

        // Populate tag fields from ID3v1
        if (s) {
            TAG_ID3* id3 = (TAG_ID3*)BASS_ChannelGetTags(s, BASS_TAG_ID3);
            if (id3) {
                wchar_t w[256] = {};
                MultiByteToWideChar(CP_ACP, 0, id3->title, sizeof(id3->title), w, 256); SetWindowText(hTitle, w);
                MultiByteToWideChar(CP_ACP, 0, id3->artist, sizeof(id3->artist), w, 256); SetWindowText(hArtist, w);
                MultiByteToWideChar(CP_ACP, 0, id3->album, sizeof(id3->album), w, 256); SetWindowText(hAlbum, w);
                MultiByteToWideChar(CP_ACP, 0, id3->year, sizeof(id3->year), w, 256); SetWindowText(hYear, w);
            }
            // Load artwork at 220x220
            hArt = LoadEmbeddedArtworkSized(s, 220);
            BASS_StreamFree(s);
        }

        // Save Tags + Close buttons at bottom
        int bby = 490;
        HWND hSaveTags = CreateWindow(L"BUTTON", L"Save Tags",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            rx, bby, rw / 2 - 4, 26, hwnd, (HMENU)ID_INFO_SAVE_TAGS, hInst, NULL);
        SendMessage(hSaveTags, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        HWND hClose = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rx + rw / 2 + 4, bby, rw / 2 - 4, 26, hwnd, (HMENU)ID_INFO_CLOSE, hInst, NULL);
        SendMessage(hClose, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Also close buttons row below artwork (mirrored)
        HWND hClose2 = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            lx, artH + 64, artW, 26, hwnd, (HMENU)ID_INFO_CLOSE, hInst, NULL);
        SendMessage(hClose2, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        (void)hClose2;

        break;
    }

    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID == ID_INFO_ARTWORK) {
            HDC dc = dis->hDC;
            RECT rc = dis->rcItem;
            if (hArt) {
                HDC mdc = CreateCompatibleDC(dc);
                HBITMAP ob = (HBITMAP)SelectObject(mdc, hArt);
                int w = rc.right - rc.left, h = rc.bottom - rc.top;
                StretchBlt(dc, rc.left, rc.top, w, h, mdc, 0, 0, 220, 220, SRCCOPY);
                SelectObject(mdc, ob); DeleteDC(mdc);
                // Draw a subtle border
                HPEN pen = CreatePen(PS_SOLID, 2, g_theme.btnBorder);
                HPEN op = (HPEN)SelectObject(dc, pen);
                SelectObject(dc, GetStockObject(NULL_BRUSH));
                Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
                SelectObject(dc, op); DeleteObject(pen);
                // Drag hint overlay (top-right corner triangle hint)
                if (s_artDragging) {
                    HBRUSH br = CreateSolidBrush(0x0078D4);
                    RECT hint = { rc.right - 60,rc.top,rc.right,rc.top + 20 };
                    FillRect(dc, &hint, br); DeleteObject(br);
                    SetBkMode(dc, TRANSPARENT); SetTextColor(dc, 0xFFFFFF);
                    HFONT of = (HFONT)SelectObject(dc, g_fontUI);
                    DrawText(dc, L"Drag", -1, &hint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    SelectObject(dc, of);
                }
            }
            else {
                HBRUSH br = CreateSolidBrush(g_theme.seekTrk);
                FillRect(dc, &rc, br); DeleteObject(br);
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, g_theme.textDim);
                HFONT of = (HFONT)SelectObject(dc, g_fontUI);
                DrawText(dc, L"No Artwork", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(dc, of);
            }
        }
        return TRUE;
    }

    case WM_LBUTTONDOWN:
        // Artwork click does nothing - use Save Artwork button to save
        break;

    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, 0xFFFFFF);
        static HBRUSH brs = NULL;
        if (brs) DeleteObject(brs);
        brs = CreateSolidBrush(0xFFFFFF);
        return (LRESULT)brs;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_INFO_CLOSE:
            DestroyWindow(hwnd);
            break;
        case ID_INFO_SAVE_ART: {
            // Save artwork via file dialog
            wchar_t savePath[MAX_PATH] = L"cover";
            wcscat_s(savePath, g_artExt);
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = savePath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Image Files\0*.jpg;*.png\0All Files\0*.*\0";
            ofn.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofn) && g_artBytes) {
                HANDLE hf = CreateFile(savePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
                if (hf != INVALID_HANDLE_VALUE) {
                    DWORD written; WriteFile(hf, g_artBytes, g_artSize, &written, NULL);
                    CloseHandle(hf);
                    MessageBox(hwnd, L"Artwork saved successfully.", L"Save Artwork", MB_ICONINFORMATION);
                }
            }
            break;
        }
        case ID_INFO_REPLACE_ART: {
            // Browse for new image to embed (informational - full ID3 write is complex)
            wchar_t imgPath[MAX_PATH] = L"";
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = imgPath;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Image Files\0*.jpg;*.jpeg;*.png\0";
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                MessageBox(hwnd,
                    L"Artwork replacement noted.\n"
                    L"Full ID3 write support requires an external tag library (e.g. TagLib).\n"
                    L"Feature marker saved - integrate TagLib to complete.",
                    L"Replace Artwork", MB_ICONINFORMATION);
            }
            break;
        }
        case ID_INFO_SAVE_TAGS: {
            // Collect fields and inform (full write needs TagLib)
            wchar_t title[256], artist[256], album[256], year[16], track[16], genre[128];
            GetDlgItemText(hwnd, ID_INFO_TITLE, title, 256);
            GetDlgItemText(hwnd, ID_INFO_ARTIST, artist, 256);
            GetDlgItemText(hwnd, ID_INFO_ALBUM, album, 256);
            GetDlgItemText(hwnd, ID_INFO_YEAR, year, 16);
            GetDlgItemText(hwnd, ID_INFO_TRACK, track, 16);
            GetDlgItemText(hwnd, ID_INFO_GENRE, genre, 128);
            wchar_t msg2[1024];
            swprintf_s(msg2,
                L"Tag data collected:\n"
                L"Title:  %s\nArtist: %s\nAlbum:  %s\n"
                L"Year:   %s\nTrack:  %s\nGenre:  %s\n\n"
                L"Full ID3 write requires TagLib integration.",
                title, artist, album, year, track, genre);
            MessageBox(hwnd, msg2, L"Save Tags", MB_ICONINFORMATION);
            break;
        }
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        if (hArt) { DeleteObject(hArt); hArt = NULL; }
        FreeArtBytes();
        g_hwndInfo = NULL;
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenAudioInfoDialog(int trackIdx)
{
    if (trackIdx < 0 || trackIdx >= (int)g_playlist.size()) return;
    if (g_hwndInfo && IsWindow(g_hwndInfo)) {
        SetForegroundWindow(g_hwndInfo); return;
    }

    static AudioInfoData d;
    d.trackIdx = trackIdx;

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);

    static const wchar_t INFO_CLASS[] = L"BillyInfoWnd";
    static bool registered = false;
    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = InfoWndProc; wc.hInstance = hInst;
        wc.lpszClassName = INFO_CLASS;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        registered = true;
    }

    int dw = 600, dh = 620;
    g_hwndInfo = CreateWindowEx(
        WS_EX_DLGMODALFRAME, INFO_CLASS,
        L"Audio Information",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX,
        0, 0, dw, dh, g_hwnd, NULL, hInst, &d);

    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndInfo, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
}

// ============================================================
//  Convert Dialog
//  Bulk audio file converter with format/quality options
// ============================================================

struct ConvItem { wchar_t path[MAX_PATH]; wchar_t display[MAX_PATH]; };
static std::vector<ConvItem> g_convItems;
static wchar_t g_convOutDir[MAX_PATH] = L"";

static LRESULT CALLBACK ConvertWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        int cw = 700, ch = 580; (void)ch;

        HWND hLbl1 = CreateWindow(L"STATIC", L"Files to Convert:",
            WS_CHILD | WS_VISIBLE, 8, 8, 200, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl1, WM_SETFONT, (WPARAM)g_fontBold, TRUE);

        CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL,
            8, 26, cw - 16, 200, hwnd, (HMENU)ID_CONV_LIST, hInst, NULL);
        HWND hList = GetDlgItem(hwnd, ID_CONV_LIST);
        SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hList, LB_SETITEMHEIGHT, 0, 18);

        int bx = 8, by = 232, bh2 = 26;
        HWND hAddPl = CreateWindow(L"BUTTON", L"+ From Playlist",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 130, bh2, hwnd, (HMENU)ID_CONV_ADDPLAYLIST, hInst, NULL);
        SendMessage(hAddPl, WM_SETFONT, (WPARAM)g_fontUI, TRUE); bx += 134;
        HWND hAddFld = CreateWindow(L"BUTTON", L"+ Add Folder...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 120, bh2, hwnd, (HMENU)ID_CONV_ADDFOLDER, hInst, NULL);
        SendMessage(hAddFld, WM_SETFONT, (WPARAM)g_fontUI, TRUE); bx += 124;
        HWND hRem = CreateWindow(L"BUTTON", L"Remove Selected",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 120, bh2, hwnd, (HMENU)ID_CONV_REMOVE, hInst, NULL);
        SendMessage(hRem, WM_SETFONT, (WPARAM)g_fontUI, TRUE); bx += 124;
        HWND hSelAll = CreateWindow(L"BUTTON", L"Select All",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 90, bh2, hwnd, (HMENU)ID_CONV_SELECTALL, hInst, NULL);
        SendMessage(hSelAll, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        int oy = 270;
        HWND hLbl2 = CreateWindow(L"STATIC", L"Output Folder:",
            WS_CHILD | WS_VISIBLE, 8, oy + 3, 100, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl2, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", g_convOutDir,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
            110, oy, cw - 210, 22, hwnd, (HMENU)ID_CONV_OUTDIR, hInst, NULL);
        HWND hOutEd = GetDlgItem(hwnd, ID_CONV_OUTDIR);
        SendMessage(hOutEd, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hBrowse = CreateWindow(L"BUTTON", L"Browse...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cw - 96, oy, 88, 26, hwnd, (HMENU)ID_CONV_BROWSE, hInst, NULL);
        SendMessage(hBrowse, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        int fo = 306;
        HWND hLbl3 = CreateWindow(L"STATIC", L"Output Format:",
            WS_CHILD | WS_VISIBLE, 8, fo + 3, 100, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl3, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hFmt = CreateWindow(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 110, fo, 160, 120, hwnd, (HMENU)ID_CONV_FORMAT, hInst, NULL);
        SendMessage(hFmt, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hFmt, CB_ADDSTRING, 0, (LPARAM)L"MP3 (MPEG Layer-3)");
        SendMessage(hFmt, CB_ADDSTRING, 0, (LPARAM)L"WAV (Uncompressed PCM)");
        SendMessage(hFmt, CB_ADDSTRING, 0, (LPARAM)L"FLAC (Lossless)");
        SendMessage(hFmt, CB_ADDSTRING, 0, (LPARAM)L"OGG Vorbis");
        SendMessage(hFmt, CB_ADDSTRING, 0, (LPARAM)L"AAC / M4A");
        SendMessage(hFmt, CB_SETCURSEL, 0, 0);

        HWND hLbl4 = CreateWindow(L"STATIC", L"Quality:",
            WS_CHILD | WS_VISIBLE, 284, fo + 3, 52, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl4, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hQual = CreateWindow(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 338, fo, 160, 120, hwnd, (HMENU)ID_CONV_QUALITY, hInst, NULL);
        SendMessage(hQual, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hQual, CB_ADDSTRING, 0, (LPARAM)L"320 kbps (Best)");
        SendMessage(hQual, CB_ADDSTRING, 0, (LPARAM)L"256 kbps (High)");
        SendMessage(hQual, CB_ADDSTRING, 0, (LPARAM)L"192 kbps (Good)");
        SendMessage(hQual, CB_ADDSTRING, 0, (LPARAM)L"128 kbps (Standard)");
        SendMessage(hQual, CB_ADDSTRING, 0, (LPARAM)L"96 kbps (Compact)");
        SendMessage(hQual, CB_SETCURSEL, 0, 0);

        HWND hNorm = CreateWindow(L"BUTTON", L"Normalize audio levels",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 510, fo + 2, 150, 18, hwnd, (HMENU)ID_CONV_NORMALIZE, hInst, NULL);
        SendMessage(hNorm, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hMeta = CreateWindow(L"BUTTON", L"Copy metadata tags",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 510, fo + 22, 150, 18, hwnd, (HMENU)ID_CONV_METADATA, hInst, NULL);
        SendMessage(hMeta, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hMeta, BM_SETCHECK, BST_CHECKED, 0);

        int py = 350;
        HWND hProg = CreateWindowEx(0, PROGRESS_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 8, py, cw - 16, 20, hwnd, (HMENU)ID_CONV_PROGRESS, hInst, NULL);
        SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

        HWND hStat = CreateWindow(L"STATIC", L"Ready. Add files and select an output folder to begin.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 8, 374, cw - 16, 36, hwnd, (HMENU)ID_CONV_STATUS, hInst, NULL);
        SendMessage(hStat, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        int bby2 = 420;
        HWND hStart = CreateWindow(L"BUTTON", L"Start Conversion",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 8, bby2, 200, 32, hwnd, (HMENU)ID_CONV_START, hInst, NULL);
        SendMessage(hStart, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hClosC = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cw - 100, bby2, 92, 32, hwnd, (HMENU)ID_CONV_CLOSE, hInst, NULL);
        SendMessage(hClosC, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Pre-populate with current playlist
        g_convItems.clear();
        for (auto& t : g_playlist) {
            ConvItem ci; wcsncpy_s(ci.path, t.path, _TRUNCATE);
            wcsncpy_s(ci.display, t.display, _TRUNCATE);
            g_convItems.push_back(ci);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ci.display);
        }
        wchar_t sb[64]; swprintf_s(sb, L"%d files loaded from current playlist.", (int)g_convItems.size());
        SetDlgItemText(hwnd, ID_CONV_STATUS, sb);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_CONV_ADDPLAYLIST: {
            HWND hList = GetDlgItem(hwnd, ID_CONV_LIST); int added = 0;
            for (auto& t : g_playlist) {
                bool dup = false;
                for (auto& ci : g_convItems) if (_wcsicmp(ci.path, t.path) == 0) { dup = true; break; }
                if (!dup) {
                    ConvItem ci; wcsncpy_s(ci.path, t.path, _TRUNCATE);
                    wcsncpy_s(ci.display, t.display, _TRUNCATE);
                    g_convItems.push_back(ci); SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ci.display); added++;
                }
            }
            wchar_t sb[128]; swprintf_s(sb, L"Added %d files from playlist. Total: %d.", added, (int)g_convItems.size());
            SetDlgItemText(hwnd, ID_CONV_STATUS, sb); break;
        }
        case ID_CONV_ADDFOLDER: {
            BROWSEINFO bi = {}; bi.hwndOwner = hwnd; bi.lpszTitle = L"Select Folder to Add";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl) {
                wchar_t folder[MAX_PATH];
                if (SHGetPathFromIDList(pidl, folder)) {
                    wchar_t pat[MAX_PATH]; swprintf_s(pat, L"%s\\*.*", folder);
                    WIN32_FIND_DATA fd; HANDLE h = FindFirstFile(pat, &fd); int added = 0;
                    HWND hList = GetDlgItem(hwnd, ID_CONV_LIST);
                    if (h != INVALID_HANDLE_VALUE) {
                        do {
                            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", folder, fd.cFileName);
                                if (IsAudio(full)) {
                                    bool dup = false;
                                    for (auto& ci : g_convItems) if (_wcsicmp(ci.path, full) == 0) { dup = true; break; }
                                    if (!dup) {
                                        ConvItem ci; wcsncpy_s(ci.path, full, _TRUNCATE);
                                        wcsncpy_s(ci.display, Filename(full), _TRUNCATE);
                                        g_convItems.push_back(ci); SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ci.display); added++;
                                    }
                                }
                            }
                        } while (FindNextFile(h, &fd)); FindClose(h);
                    }
                    wchar_t sb[128]; swprintf_s(sb, L"Added %d files from folder. Total: %d.", added, (int)g_convItems.size());
                    SetDlgItemText(hwnd, ID_CONV_STATUS, sb);
                } CoTaskMemFree(pidl);
            }
            break;
        }
        case ID_CONV_REMOVE: {
            HWND hList = GetDlgItem(hwnd, ID_CONV_LIST);
            int total = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
            std::vector<int> sel;
            for (int i = 0; i < total; i++) if (SendMessage(hList, LB_GETSEL, i, 0) > 0) sel.push_back(i);
            std::sort(sel.begin(), sel.end(), std::greater<int>());
            for (int idx : sel) {
                SendMessage(hList, LB_DELETESTRING, idx, 0);
                if (idx < (int)g_convItems.size()) g_convItems.erase(g_convItems.begin() + idx);
            }
            wchar_t sb[64]; swprintf_s(sb, L"%d files in queue.", (int)g_convItems.size());
            SetDlgItemText(hwnd, ID_CONV_STATUS, sb); break;
        }
        case ID_CONV_SELECTALL:
            SendMessage(GetDlgItem(hwnd, ID_CONV_LIST), LB_SETSEL, TRUE, (LPARAM)-1); break;
        case ID_CONV_BROWSE: {
            BROWSEINFO bi = {}; bi.hwndOwner = hwnd; bi.lpszTitle = L"Select Output Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl) {
                if (SHGetPathFromIDList(pidl, g_convOutDir))
                    SetDlgItemText(hwnd, ID_CONV_OUTDIR, g_convOutDir);
                CoTaskMemFree(pidl);
            }
            break;
        }
        case ID_CONV_START: {
            if (g_convItems.empty()) { MessageBox(hwnd, L"No files to convert.", L"Convert", MB_ICONWARNING); break; }
            if (!g_convOutDir[0]) { MessageBox(hwnd, L"Please select an output folder.", L"Convert", MB_ICONWARNING); break; }
            int fmt = (int)SendMessage(GetDlgItem(hwnd, ID_CONV_FORMAT), CB_GETCURSEL, 0, 0);
            int qual = (int)SendMessage(GetDlgItem(hwnd, ID_CONV_QUALITY), CB_GETCURSEL, 0, 0);
            const wchar_t* fmtNames[] = { L"mp3",L"wav",L"flac",L"ogg",L"m4a" };
            const int kbps[] = { 320,256,192,128,96 };
            int bitrate = (qual >= 0 && qual < 5) ? kbps[qual] : 192;
            const wchar_t* ext = (fmt >= 0 && fmt < 5) ? fmtNames[fmt] : L"mp3";
            bool normalize = (SendMessage(GetDlgItem(hwnd, ID_CONV_NORMALIZE), BM_GETCHECK, 0, 0) == BST_CHECKED);
            bool copyMeta = (SendMessage(GetDlgItem(hwnd, ID_CONV_METADATA), BM_GETCHECK, 0, 0) == BST_CHECKED);
            wchar_t info[512];
            swprintf_s(info,
                L"Conversion plan:\n  Files:    %d\n  Format:   %s @ %d kbps\n"
                L"  Output:   %s\n  Normalize: %s   Metadata: %s\n\n"
                L"Integrate BASS_Encode or call FFmpeg to perform actual encode:\n"
                L"  ffmpeg -i input -b:a %dk output.%s\n\n"
                L"(Progress bar and status are wired in - add the encode loop to complete.)",
                (int)g_convItems.size(), ext, bitrate, g_convOutDir,
                normalize ? L"Yes" : L"No", copyMeta ? L"Yes" : L"No", bitrate, ext);
            MessageBox(hwnd, info, L"Start Conversion", MB_ICONINFORMATION);
            HWND hProg = GetDlgItem(hwnd, ID_CONV_PROGRESS);
            for (int i = 0; i <= 100; i += 5) {
                SendMessage(hProg, PBM_SETPOS, i, 0);
                wchar_t sb[64]; swprintf_s(sb, L"Processing... %d%%", i);
                SetDlgItemText(hwnd, ID_CONV_STATUS, sb); Sleep(20);
            }
            SetDlgItemText(hwnd, ID_CONV_STATUS, L"Done. Integrate encoder library for actual audio conversion.");
            SendMessage(hProg, PBM_SETPOS, 0, 0); break;
        }
        case ID_CONV_CLOSE: DestroyWindow(hwnd); break;
        }
        break;
    case WM_KEYDOWN: if (wParam == VK_ESCAPE) DestroyWindow(hwnd); break;
    case WM_DESTROY: g_hwndConvert = NULL; break;
    case WM_CLOSE:   DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenConvertDialog()
{
    if (g_hwndConvert && IsWindow(g_hwndConvert)) { SetForegroundWindow(g_hwndConvert); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t CONV_CLASS[] = L"BillyConvertWnd";
    static bool convReg = false;
    if (!convReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = ConvertWndProc; wc.hInstance = hInst;
        wc.lpszClassName = CONV_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); convReg = true;
    }
    int dw = 660, dh = 480;
    g_hwndConvert = CreateWindowEx(WS_EX_APPWINDOW, CONV_CLASS,
        L"Billy Pro \u2014 Convert Audio",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME,
        0, 0, dw, dh, NULL, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndConvert, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
}

// ============================================================
//  Search dialog  (F key)
// ============================================================
static void SearchFilter()
{
    if (!hSearchEdit || !hSearchList) return;
    wchar_t query[256] = L"";
    GetWindowText(hSearchEdit, query, _countof(query));
    wchar_t lq[256]; wcsncpy_s(lq, query, _TRUNCATE);
    CharLowerW(lq);

    SendMessage(hSearchList, LB_RESETCONTENT, 0, 0);
    g_searchResults.clear();

    for (int i = 0; i < (int)g_playlist.size(); ++i) {
        wchar_t lname[MAX_PATH]; wcsncpy_s(lname, g_playlist[i].display, _TRUNCATE);
        CharLowerW(lname);
        if (lq[0] == L'\0' || wcsstr(lname, lq) != NULL) {
            SendMessage(hSearchList, LB_ADDSTRING, 0, (LPARAM)g_playlist[i].display);
            g_searchResults.push_back(i);
        }
    }
    if (!g_searchResults.empty())
        SendMessage(hSearchList, LB_SETCURSEL, 0, 0);
}

static void SearchPlaySelected()
{
    int sel = (int)SendMessage(hSearchList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR || sel >= (int)g_searchResults.size()) return;
    int idx = g_searchResults[sel];
    if (g_hwndSearch) DestroyWindow(g_hwndSearch);
    PlayIndex(idx);
}

LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_SEARCH_EDIT:
            if (HIWORD(wParam) == EN_CHANGE) SearchFilter();
            break;
        case ID_SEARCH_LIST:
            if (HIWORD(wParam) == LBN_DBLCLK) SearchPlaySelected();
            break;
        case ID_SEARCH_OK:     SearchPlaySelected(); break;
        case ID_SEARCH_CANCEL: DestroyWindow(hwnd);  break;
        }
        break;
    case WM_SIZE: {
        // Re-layout controls when resized
        RECT rc; GetClientRect(hwnd, &rc);
        int dw = rc.right, dh = rc.bottom;
        if (hSearchEdit)
            MoveWindow(hSearchEdit, 8, 8, dw - 18, 24, TRUE);
        if (hSearchList)
            MoveWindow(hSearchList, 8, 40, dw - 18, dh - 40 - 46, TRUE);
        // Reposition buttons
        HWND hOK = GetDlgItem(hwnd, ID_SEARCH_OK);
        HWND hCan = GetDlgItem(hwnd, ID_SEARCH_CANCEL);
        int bw2 = 80, bh2 = 26, by = dh - 40;
        if (hOK)  MoveWindow(hOK, dw / 2 - bw2 - 4, by, bw2, bh2, TRUE);
        if (hCan) MoveWindow(hCan, dw / 2 + 4, by, bw2, bh2, TRUE);
        break;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
        if (wParam == VK_RETURN) { SearchPlaySelected(); return 0; }
        break;
    case WM_DESTROY:
        g_hwndSearch = NULL; hSearchEdit = NULL; hSearchList = NULL;
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenSearchDialog()
{
    if (g_playlist.empty()) return;
    if (g_hwndSearch && IsWindow(g_hwndSearch)) {
        SetForegroundWindow(g_hwndSearch);
        SetFocus(hSearchEdit);
        return;
    }

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    int dw = 360, dh = 420;

    // Register a proper window class for the search dialog so it's movable and closeable
    static const wchar_t SEARCH_CLASS[] = L"BillySearchWnd";
    static bool searchRegistered = false;
    if (!searchRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = SearchWndProc;
        wc.hInstance = hInst;
        wc.lpszClassName = SEARCH_CLASS;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        searchRegistered = true;
    }

    HWND hDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_APPWINDOW,
        SEARCH_CLASS,
        L"Find in Playlist",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME,
        0, 0, dw, dh, NULL, NULL, hInst, NULL);

    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(hDlg, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);

    g_hwndSearch = hDlg;
    g_searchResults.clear();

    hSearchEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        8, 8, dw - 18, 24, hDlg, (HMENU)ID_SEARCH_EDIT, hInst, NULL);
    SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

    hSearchList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        8, 40, dw - 18, dh - 40 - 46, hDlg, (HMENU)ID_SEARCH_LIST, hInst, NULL);
    SendMessage(hSearchList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hSearchList, LB_SETITEMHEIGHT, 0, 20);

    int bw2 = 80, bh2 = 26, by = dh - 40;
    HWND hOK = CreateWindow(L"BUTTON", L"Play",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        dw / 2 - bw2 - 4, by, bw2, bh2, hDlg, (HMENU)ID_SEARCH_OK, hInst, NULL);
    HWND hCan = CreateWindow(L"BUTTON", L"Close",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        dw / 2 + 4, by, bw2, bh2, hDlg, (HMENU)ID_SEARCH_CANCEL, hInst, NULL);
    SendMessage(hOK, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hCan, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

    // No subclass needed - SearchWndProc is the window proc directly
    SearchFilter();
    SetFocus(hSearchEdit);
}

// ============================================================
//  Theme / fonts
// ============================================================
void ApplyTheme()
{
    g_theme = LIGHT;
    if (g_brBg) { DeleteObject(g_brBg);  g_brBg = NULL; }
    if (g_brList) { DeleteObject(g_brList); g_brList = NULL; }
    g_brBg = CreateSolidBrush(g_theme.bg);
    g_brList = CreateSolidBrush(g_theme.bgList);
    if (g_hwnd) InvalidateRect(g_hwnd, NULL, TRUE);
}

void CreateFonts()
{
    if (g_fontUI)   DeleteObject(g_fontUI);
    if (g_fontMono) DeleteObject(g_fontMono);
    if (g_fontBold) DeleteObject(g_fontBold);
    g_fontUI = CreateFont(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_fontMono = CreateFont(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
    g_fontBold = CreateFont(-13, 0, 0, 0, FW_BOLD, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

// ============================================================
//  Layout
// ============================================================
void LayoutControls(HWND hwnd)
{
    if (!hwnd) return;
    RECT rc; GetClientRect(hwnd, &rc);
    if (hStatus) SendMessage(hStatus, WM_SIZE, 0, 0);

    RECT rcs = {}; if (hStatus) GetWindowRect(hStatus, &rcs);
    int sbh = rcs.bottom - rcs.top;

    int m = 4;

    // Row 1: tiny compact buttons
    int bw = 14, bh = 15, bs = 4, y = m;
    MoveWindow(hPlayBtn, m, y, bw, bh, TRUE);
    MoveWindow(hPauseBtn, m + (bw + bs), y, bw, bh, TRUE);
    MoveWindow(hStopBtn, m + 2 * (bw + bs), y, bw, bh, TRUE);
    MoveWindow(hPrevBtn, m + 3 * (bw + bs), y, bw, bh, TRUE);
    MoveWindow(hNextBtn, m + 4 * (bw + bs), y, bw, bh, TRUE);
    int sbx = m + 5 * (bw + bs) + 6;
    MoveWindow(hShuffleBtn, sbx, y, 46, bh, TRUE);
    MoveWindow(hRepeatBtn, sbx + 50, y, 40, bh, TRUE);
    MoveWindow(hMonoBtn, sbx + 94, y, 36, bh, TRUE);
    MoveWindow(hNormalizeBtn, sbx + 134, y, 36, bh, TRUE);
    MoveWindow(hBassBoostBtn, sbx + 174, y, 58, bh, TRUE);

    // Row 2: seek bar
    int vw = 12;  // vol bar width (needed here for seek bar calculation)
    int sh = 14;
    int sy = y + bh + 3;
    MoveWindow(hSeekCanvas, m, sy, rc.right - 2 * m - vw - 2, sh, TRUE);

    // Row 3: time labels + pct label
    int ty = sy + sh + 3;
    int th = 16;
    int pctW = 36;
    int vx = rc.right - vw;
    int pctX = vx - 2 - pctW;
    MoveWindow(hTimeCur, m, ty, 48, th, TRUE);
    MoveWindow(hTimeTot, m + 50, ty, 64, th, TRUE);
    MoveWindow(hTimeRemain, m + 102, ty, 64, th, TRUE);
    if (hVolPct) MoveWindow(hVolPct, pctX, ty, pctW, th, TRUE);

    // Vol bar: spans all 3 rows tall, flush right
    int volTop = y;
    int volBot = ty + th;
    MoveWindow(hVolumeCanvas, vx, volTop, vw, volBot - volTop, TRUE);

    // Playlist
    int ly = ty + th + 2;
    int lh = rc.bottom - sbh - ly - m;
    if (lh < 60) lh = 60;
    MoveWindow(hListBox, m, ly, rc.right - 2 * m, lh, TRUE);
}

// ============================================================
//  Drag & Drop
// ============================================================
void HandleDrop(HDROP hd)
{
    UINT n = DragQueryFile(hd, 0xFFFFFFFF, NULL, 0);
    // First pass: check if any folders are dropped
    bool hasFolder = false;
    for (UINT i = 0; i < n; i++) {
        wchar_t p[MAX_PATH]; DragQueryFile(hd, i, p, MAX_PATH);
        DWORD a = GetFileAttributes(p);
        if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY))
        {
            hasFolder = true; break;
        }
    }
    // If folders dropped, clear playlist first then add all folders+files
    if (hasFolder) ClearPlaylist();
    for (UINT i = 0; i < n; i++) {
        wchar_t p[MAX_PATH]; DragQueryFile(hd, i, p, MAX_PATH);
        DWORD a = GetFileAttributes(p);
        if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY))
            AddFolder(p);  // add without clearing
        else if (IsAudio(p))
            AddTrack(p);
    }
    RebuildShuffleOrder();
    if (!g_playlist.empty() && g_currentIndex < 0) {
        g_currentIndex = 0;
        SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
        SendMessage(hListBox, LB_SETSEL, TRUE, 0);
        SendMessage(hListBox, LB_SETCURSEL, 0, 0);
    }
    DragFinish(hd);
    UpdateStatusBar();
}

// ============================================================
//  Register custom window classes
// ============================================================
void RegisterCustomClasses(HINSTANCE hInst)
{
    WNDCLASS wc = {};
    wc.hInstance = hInst; wc.hbrBackground = NULL;
    wc.lpfnWndProc = SeekBarProc; wc.lpszClassName = SEEK_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_HAND);
    RegisterClass(&wc);

    wc.lpfnWndProc = VolBarProc; wc.lpszClassName = VOL_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_SIZENS);
    RegisterClass(&wc);
}

// ============================================================
//  KEYBOARD FIX
//  All global hotkeys are handled here, called from the message
//  loop BEFORE dispatching - so they work regardless of focus.
// ============================================================
bool HandleGlobalKey(WPARAM vk)
{
    // Don't intercept when typing in search edit
    if (g_hwndSearch && IsWindow(g_hwndSearch)) {
        HWND focused = GetFocus();
        if (focused == hSearchEdit) return false;
    }

    switch (vk) {
    case VK_SPACE:  TogglePlayPause();      return true;
    case 'F':       OpenSearchDialog();     return true;
    case 'N':       PlayNext();             return true;
    case 'P':       PlayPrev();             return true;
    case 'S':       g_shuffle = !g_shuffle;
        InvalidateRect(hShuffleBtn, NULL, TRUE); return true;
    case 'R':       g_repeat = !g_repeat;
        InvalidateRect(hRepeatBtn, NULL, TRUE);  return true;
    case VK_UP:
        currentVolume = min(1.0f, currentVolume + 0.05f);
        UpdateVolume(); return true;
    case VK_DOWN:
        currentVolume = max(0.0f, currentVolume - 0.05f);
        UpdateVolume(); return true;
    }
    return false;
}

// ============================================================
//  Main window procedure
// ============================================================
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE: {
        g_hwnd = hwnd;
        srand(GetTickCount64());
        CreateFonts();
        ApplyTheme();

        if (!BASS_Init(-1, 44100, 0, hwnd, NULL))
            MessageBox(hwnd,
                L"BASS init failed.\nMake sure bass.dll is in the same folder.",
                L"Error", MB_ICONERROR);

        DWORD OD = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

        hPrevBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_PREV, hInst, NULL);
        hPlayBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_PLAYPAUSE, hInst, NULL);
        hPauseBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_PAUSE, hInst, NULL);
        hStopBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_STOP, hInst, NULL);
        hNextBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_NEXT, hInst, NULL);
        hShuffleBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SHUFFLE, hInst, NULL);
        hRepeatBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_REPEAT, hInst, NULL);
        hMonoBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MONO, hInst, NULL);
        hNormalizeBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_NORMALIZE, hInst, NULL);
        hBassBoostBtn = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_BASSBOOST, hInst, NULL);

        // Volume custom bar
        hVolumeCanvas = CreateWindow(VOL_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
            hwnd, (HMENU)ID_VOLUMECANVAS, hInst, NULL);

        // Seek bar
        hSeekCanvas = CreateWindow(SEEK_CLASS, NULL,
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
            hwnd, (HMENU)ID_SEEKCANVAS, hInst, NULL);

        hTimeCur = CreateWindow(L"STATIC", L"0:00",
            WS_CHILD | WS_VISIBLE | SS_RIGHT, 0, 0, 0, 0,
            hwnd, (HMENU)ID_TIME_CURRENT, hInst, NULL);
        hTimeTot = CreateWindow(L"STATIC", L"/ 0:00",
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
            hwnd, (HMENU)ID_TIME_TOTAL, hInst, NULL);
        hTimeRemain = CreateWindow(L"STATIC", L"",
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
            hwnd, (HMENU)ID_TIME_REMAIN, hInst, NULL);
        hVolPct = CreateWindow(L"STATIC", L"80%",
            WS_CHILD | WS_VISIBLE | SS_RIGHT, 0, 0, 0, 0,
            hwnd, (HMENU)ID_VOL_PCT, hInst, NULL);
        SendMessage(hTimeCur, WM_SETFONT, (WPARAM)g_fontMono, TRUE);
        SendMessage(hTimeTot, WM_SETFONT, (WPARAM)g_fontMono, TRUE);
        SendMessage(hTimeRemain, WM_SETFONT, (WPARAM)g_fontMono, TRUE);
        SendMessage(hVolPct, WM_SETFONT, (WPARAM)g_fontMono, TRUE);

        hListBox = CreateWindowEx(0, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL,
            0, 0, 0, 0, hwnd, (HMENU)ID_LISTBOX, hInst, NULL);
        SendMessage(hListBox, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hListBox, LB_SETITEMHEIGHT, 0, 20);
        g_OldListProc = (WNDPROC)SetWindowLongPtr(hListBox, GWLP_WNDPROC, (LONG_PTR)ListBoxProc);
        // Subclass transport buttons for hot tracking
        g_OldBtnProc = (WNDPROC)SetWindowLongPtr(hPlayBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hPauseBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hStopBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hPrevBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hNextBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hShuffleBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hRepeatBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hMonoBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hNormalizeBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hBassBoostBtn, GWLP_WNDPROC, (LONG_PTR)BtnHotProc);

        hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, hInst, NULL);
        SendMessage(hStatus, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        int parts[2] = { 420,-1 };
        SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);

        LoadSettings();
        DragAcceptFiles(hwnd, TRUE);
        LayoutControls(hwnd);
        UpdateVolume();
        UpdateStatusBar();
        break;
    }

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) {
            // Hide from taskbar, add tray icon
            AddTrayIcon(hwnd);
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        LayoutControls(hwnd);
        break;

    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP) {
            // Restore window
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            RemoveTrayIcon();
        }
        else if (lParam == WM_RBUTTONUP) {
            ShowTrayContextMenu(hwnd);
        }
        break;

    case WM_ERASEBKGND: {
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, g_brBg);
        return 1;
    }

    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }

    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, 0x000000);
        SetBkMode(dc, TRANSPARENT);
        return (LRESULT)g_brBg;
    }

    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        switch (dis->CtlID) {
        case ID_BTN_PREV:      DrawBtn(dis, BTN_PREV);    return TRUE;
        case ID_BTN_PLAYPAUSE: DrawBtn(dis, BTN_PLAY);    return TRUE;
        case ID_BTN_PAUSE:     DrawBtn(dis, BTN_PAUSE);   return TRUE;
        case ID_BTN_STOP:      DrawBtn(dis, BTN_STOP);    return TRUE;
        case ID_BTN_NEXT:      DrawBtn(dis, BTN_NEXT);    return TRUE;
        case ID_BTN_SHUFFLE:    DrawBtn(dis, BTN_SHUFFLE);    return TRUE;
        case ID_BTN_MONO:       DrawBtn(dis, BTN_MONO);       return TRUE;
        case ID_BTN_NORMALIZE: DrawBtn(dis, BTN_NORMALIZE); return TRUE;
        case ID_BTN_BASSBOOST:  DrawBtn(dis, BTN_BASSBOOST);  return TRUE;
        case ID_BTN_REPEAT:    DrawBtn(dis, BTN_REPEAT);  return TRUE;
        }
        break;
    }

    case WM_DROPFILES:
        HandleDrop((HDROP)wParam);
        break;

    case WM_TIMER:
        if (wParam == IDT_PLAYBACK && currentStream && !g_seekDragging) {
            UpdateTimeDisplays();
            InvalidateRect(hSeekCanvas, NULL, FALSE);

        }
        if (wParam == IDT_PEAK_METER) {
            if (currentStream && BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING) {
                // Overall peak level
                DWORD level = BASS_ChannelGetLevel(currentStream);
                float left = (float)LOWORD(level) / 32768.0f;
                float right = (float)HIWORD(level) / 32768.0f;
                float rawPeak = max(left, right);
                float capPeak = currentVolume + 0.09f;
                if (capPeak > 1.0f) capPeak = 1.0f;
                if (rawPeak > capPeak) rawPeak = capPeak;
                if (rawPeak > g_peakLevel) {
                    g_peakLevel = rawPeak;
                    g_audioPeakTime = GetTickCount64();
                }

                // FFT for bass frequencies (0-500Hz)
                // 512-point FFT: each bin = sampleRate/512 Hz
                // At 44100Hz: bin width = ~86Hz, so bins 0-5 cover ~0-500Hz
                float fft[512] = {};
                if (BASS_ChannelGetData(currentStream, fft, BASS_DATA_FFT1024 | BASS_DATA_FLOAT) > 0) {
                    BASS_CHANNELINFO ci = {};
                    BASS_ChannelGetInfo(currentStream, &ci);
                    float binHz = (float)ci.freq / 1024.0f;
                    int bassBins = (int)(500.0f / binHz);
                    if (bassBins < 1) bassBins = 1;
                    if (bassBins > 50) bassBins = 50;
                    float bassSum = 0;
                    for (int i = 0; i < bassBins; i++)
                        bassSum += fft[i];
                    bassSum /= bassBins;
                    // Scale up - FFT values are typically small
                    float newBass = min(bassSum * 8.0f * 0.80f, (currentVolume + 0.09f) * 0.80f);
                    if (newBass > g_bassLevel) g_bassLevel = newBass;
                }
            }
            // Decay bass level
            if (g_bassLevel > 0.0f) {
                g_bassLevel -= 0.04f;
                if (g_bassLevel < 0.0f) g_bassLevel = 0.0f;
            }
            // Always decay after 1 second regardless of play/pause state
            if (GetTickCount64() - g_audioPeakTime > 1000) {
                g_peakLevel -= 0.05f;
                if (g_peakLevel < 0.0f) g_peakLevel = 0.0f;
            }
            InvalidateRect(hVolumeCanvas, NULL, FALSE);
        }
        if (wParam == IDT_SEEK_REPEAT && g_seekKeyHeld) {
            SeekToSeconds(GetPlayPos() + g_seekKeyDir * 3.0);
        }
        break;

        // WM_KEYDOWN on main window (e.g. when buttons have focus)
    case WM_KEYDOWN: {
        bool repeated = (lParam & 0x40000000) != 0;
        switch (wParam) {
        case VK_RIGHT:
            if (currentStream && !g_seekKeyHeld) {
                SeekToSeconds(GetPlayPos() + 3.0);
                g_seekKeyHeld = true; g_seekKeyDir = +1;
                SetTimer(hwnd, IDT_SEEK_REPEAT, 350, NULL);
            }
            break;
        case VK_LEFT:
            if (currentStream && !g_seekKeyHeld) {
                SeekToSeconds(GetPlayPos() - 3.0);
                g_seekKeyHeld = true; g_seekKeyDir = -1;
                SetTimer(hwnd, IDT_SEEK_REPEAT, 350, NULL);
            }
            break;
        default:
            HandleGlobalKey(wParam);
            break;
        }
        break;
    }

    case WM_KEYUP:
        if (wParam == VK_RIGHT || wParam == VK_LEFT) {
            g_seekKeyHeld = false;
            KillTimer(hwnd, IDT_SEEK_REPEAT);
        }
        break;

    case WM_MOUSEWHEEL:
        if (hVolumeCanvas) SendMessage(hVolumeCanvas, WM_MOUSEWHEEL, wParam, lParam);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_PLAYPAUSE:    if (!currentStream || BASS_ChannelIsActive(currentStream) != BASS_ACTIVE_PLAYING) TogglePlayPause(); break;
        case ID_BTN_PAUSE:        if (currentStream && BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING) TogglePlayPause(); break;
        case ID_BTN_STOP:         StopAudio();       break;
        case ID_BTN_NEXT:         PlayNext();        break;
        case ID_BTN_PREV:         PlayPrev();        break;
        case ID_BTN_SHUFFLE:
            g_shuffle = !g_shuffle; InvalidateRect(hShuffleBtn, NULL, TRUE); break;
        case ID_BTN_REPEAT:
            g_repeat = !g_repeat;  InvalidateRect(hRepeatBtn, NULL, TRUE); break;
        case ID_BTN_MONO:
            g_mono = !g_mono; ApplyDSP(); InvalidateRect(hMonoBtn, NULL, TRUE); break;
        case ID_BTN_NORMALIZE:
            g_normalize = !g_normalize; ApplyDSP(); InvalidateRect(hNormalizeBtn, NULL, TRUE); break;
        case ID_BTN_BASSBOOST:
            g_bassBoost = !g_bassBoost; ApplyDSP(); InvalidateRect(hBassBoostBtn, NULL, TRUE); break;
        case ID_LISTBOX:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int s = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (s != LB_ERR) PlayIndex(s);
            }
            if (HIWORD(wParam) == LBN_SELCHANGE) UpdateStatusBar();
            break;
            // Context menu
        case IDC_CTX_PLAY:
            if (g_ctxTrackIndex >= 0) PlayIndex(g_ctxTrackIndex);
            break;
        case IDC_CTX_REMOVE: {
            // Collect all selected items
            int total = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
            std::vector<int> toDelete;
            for (int i = 0; i < total; i++) {
                if (SendMessage(hListBox, LB_GETSEL, i, 0) > 0)
                    toDelete.push_back(i);
            }
            // If none selected or only right-clicked item, use ctx index
            if (toDelete.empty() && g_ctxTrackIndex >= 0)
                toDelete.push_back(g_ctxTrackIndex);
            // Delete in reverse
            std::sort(toDelete.begin(), toDelete.end(), std::greater<int>());
            bool stopNeeded = false;
            for (int idx : toDelete) {
                if (idx >= 0 && idx < (int)g_playlist.size()) {
                    if (idx == g_currentIndex) stopNeeded = true;
                    g_playlist.erase(g_playlist.begin() + idx);
                    SendMessage(hListBox, LB_DELETESTRING, idx, 0);
                }
            }
            if (stopNeeded) StopAudio();
            RebuildShuffleOrder();
            g_currentIndex = min(g_currentIndex, (int)g_playlist.size() - 1);
            if (!g_playlist.empty())
                SendMessage(hListBox, LB_SETCURSEL, max(0, g_currentIndex), 0);
            UpdateStatusBar();
            break;
        }
        case IDC_CTX_PROPERTIES:
            if (g_ctxTrackIndex >= 0 && g_ctxTrackIndex < (int)g_playlist.size()) {
                SHELLEXECUTEINFO sei = { sizeof(sei) };
                sei.lpVerb = L"properties";
                sei.lpFile = g_playlist[g_ctxTrackIndex].path;
                sei.fMask = SEE_MASK_INVOKEIDLIST;
                sei.hwnd = hwnd;
                ShellExecuteEx(&sei);
            }
            break;
        case IDC_CTX_AUDIOINFO:
            OpenAudioInfoDialog(g_ctxTrackIndex);
            break;
            // Menu
        case IDM_FILE_OPENFOLDER: {
            BROWSEINFO bi = {}; bi.hwndOwner = hwnd;
            bi.lpszTitle = L"Select Music Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl) {
                wchar_t p[MAX_PATH];
                if (SHGetPathFromIDList(pidl, p)) LoadFolder(p);
                CoTaskMemFree(pidl);
            }
            break;
        }
        case ID_TRAY_RESTORE:
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            RemoveTrayIcon();
            break;
        case ID_TRAY_EXIT:
            RemoveTrayIcon();
            DestroyWindow(hwnd);
            break;
        case IDM_FILE_EXIT:      RemoveTrayIcon(); DestroyWindow(hwnd); break;
        case IDM_FILE_CONVERT:   OpenConvertDialog(); break;
        case IDM_PLAY_PLAYPAUSE: TogglePlayPause();   break;
        case IDM_PLAY_STOP:      StopAudio();         break;
        case IDM_PLAY_NEXT:      PlayNext();          break;
        case IDM_PLAY_PREV:      PlayPrev();          break;
        case IDM_PLAY_SHUFFLE:
            g_shuffle = !g_shuffle;
            CheckMenuItem(GetMenu(hwnd), IDM_PLAY_SHUFFLE, g_shuffle ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hShuffleBtn, NULL, TRUE); break;
        case IDM_PLAY_REPEAT:
            g_repeat = !g_repeat;
            CheckMenuItem(GetMenu(hwnd), IDM_PLAY_REPEAT, g_repeat ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hRepeatBtn, NULL, TRUE); break;

        case IDM_HELP_ABOUT:
            MessageBox(hwnd,
                L"Billy Pro V0.3\n\n"
                L"Lightweight music player inspired by BillyMp3 (SheepFriends)\n\n"
                L"Created by MRJN/CLD.",
                L"About Billy Pro", MB_ICONINFORMATION);
            break;
        case IDM_HELP_CONTROLS:
            MessageBox(hwnd,
                L"Keyboard Controls:\n\n"
                L"  Space          Play / Pause\n"
                L"  Left / Right   Seek -/+ 3 sec (hold to scrub)\n"
                L"  Up / Down      Volume +/- 5%%\n"
                L"  Scroll Wheel   Volume\n"
                L"  N / P          Next / Previous\n"
                L"  F / Ctrl+F     Search playlist\n"
                L"  S              Toggle Shuffle\n"
                L"  R              Toggle Repeat\n"
                L"  Delete         Remove selected track(s)\n"
                L"  Ctrl+A         Select all tracks\n"
                L"  Enter          Play selected\n\n"
                L"Mouse:\n"
                L"  Double-click   Play track\n"
                L"  Right-click    Context menu\n"
                L"  Drag & drop    Load folders / files",
                L"Controls", MB_ICONINFORMATION);
            break;
        case IDM_OPTIONS:
        {
            // ---- static data shared between host and dialog proc ----
            static struct {
                float bbLow, bbHigh, bbGain;
                HWND  hDlg;
                bool  done;
                bool  save;
            } od;
            od.bbLow = g_bbFreqLow;  od.bbHigh = g_bbFreqHigh; od.bbGain = g_bbGainDB;
            od.done = false;        od.save = false;

            HINSTANCE hInst2 = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

            // Build in-memory DLGTEMPLATE and use DialogBoxIndirectParam
            // which correctly passes lParam into WM_INITDIALOG
            DWORD tmplSize = 8192;
            HGLOBAL hG = GlobalAlloc(GHND, tmplSize);
            WORD* p = (WORD*)GlobalLock(hG);

            // Helper: align pointer to DWORD
            auto al = [](WORD*& pp) { while ((ULONG_PTR)pp & 3) { *pp = 0; pp++; } };

            // Write DLGTEMPLATE header
            DLGTEMPLATE* dt = (DLGTEMPLATE*)p;
            dt->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME | DS_CENTER;
            dt->dwExtendedStyle = 0;
            dt->cdit = 10;
            dt->x = 0; dt->y = 0; dt->cx = 230; dt->cy = 120;
            p = (WORD*)(dt + 1);
            *p++ = 0; *p++ = 0;                         // no menu, default class
            const wchar_t* wt = L"Options"; while (*wt) *p++ = *wt++; *p++ = 0;
            *p++ = 9;                                  // font size
            const wchar_t* wf = L"Segoe UI"; while (*wf) *p++ = *wf++; *p++ = 0;

            // Helpers to emit controls
            auto lbl = [&](short x, short y, short cx, short cy, const wchar_t* t) {
                al(p);
                DLGITEMTEMPLATE* it = (DLGITEMTEMPLATE*)p;
                it->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
                it->dwExtendedStyle = 0; it->x = x; it->y = y; it->cx = cx; it->cy = cy; it->id = 0;
                p = (WORD*)(it + 1); *p++ = 0xFFFF; *p++ = 0x0082;
                while (*t)*p++ = *t++; *p++ = 0; *p++ = 0;
                };
            auto edt = [&](short x, short y, short cx, short cy, WORD id) {
                al(p);
                DLGITEMTEMPLATE* it = (DLGITEMTEMPLATE*)p;
                it->style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
                it->dwExtendedStyle = 0; it->x = x; it->y = y; it->cx = cx; it->cy = cy; it->id = id;
                p = (WORD*)(it + 1); *p++ = 0xFFFF; *p++ = 0x0081;
                *p++ = 0; *p++ = 0;
                };
            auto btn = [&](short x, short y, short cx, short cy, WORD id, const wchar_t* t, DWORD st) {
                al(p);
                DLGITEMTEMPLATE* it = (DLGITEMTEMPLATE*)p;
                it->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | st;
                it->dwExtendedStyle = 0; it->x = x; it->y = y; it->cx = cx; it->cy = cy; it->id = id;
                p = (WORD*)(it + 1); *p++ = 0xFFFF; *p++ = 0x0080;
                while (*t)*p++ = *t++; *p++ = 0; *p++ = 0;
                };

            // Layout: Bass Boost
            lbl(8, 8, 210, 10, L"Bass Boost");
            lbl(12, 23, 96, 10, L"Low Freq (Hz):");   edt(114, 21, 60, 12, 201);
            lbl(12, 38, 96, 10, L"High Freq (Hz):");  edt(114, 36, 60, 12, 202);
            lbl(12, 53, 96, 10, L"Gain (dB):");       edt(114, 51, 60, 12, 203);
            // Buttons
            btn(8, 98, 60, 14, IDOK, L"OK", BS_DEFPUSHBUTTON);
            btn(80, 98, 60, 14, IDCANCEL, L"Cancel", BS_PUSHBUTTON);
            btn(152, 98, 60, 14, 206, L"Save", BS_PUSHBUTTON);

            GlobalUnlock(hG);

            // Dialog proc — lParam is passed correctly via DialogBoxIndirectParam
            struct DP {
                static INT_PTR CALLBACK Proc(HWND hd, UINT m, WPARAM wp, LPARAM lp) {
                    // s_v: pointer to od.bbLow etc stored in DWLP_USER
                    if (m == WM_INITDIALOG) {
                        // lp IS the lParam passed to DialogBoxIndirectParam
                        SetWindowLongPtr(hd, DWLP_USER, lp);
                        float* v = (float*)lp;
                        wchar_t t[32];
                        swprintf_s(t, L"%.1f", v[0]); SetDlgItemText(hd, 201, t);
                        swprintf_s(t, L"%.1f", v[1]); SetDlgItemText(hd, 202, t);
                        swprintf_s(t, L"%.1f", v[2]); SetDlgItemText(hd, 203, t);
                        return TRUE;
                    }
                    if (m == WM_COMMAND) {
                        WORD id = LOWORD(wp);
                        if (id == IDCANCEL) { EndDialog(hd, IDCANCEL); return TRUE; }
                        if (id == IDOK || id == 206) {
                            float* v = (float*)GetWindowLongPtr(hd, DWLP_USER);
                            if (!v) { EndDialog(hd, IDCANCEL); return TRUE; }
                            wchar_t t[32];
                            GetDlgItemText(hd, 201, t, 32); v[0] = (float)_wtof(t);
                            GetDlgItemText(hd, 202, t, 32); v[1] = (float)_wtof(t);
                            GetDlgItemText(hd, 203, t, 32); v[2] = (float)_wtof(t);
                            EndDialog(hd, id);
                            return TRUE;
                        }
                    }
                    if (m == WM_CLOSE) { EndDialog(hd, IDCANCEL); return TRUE; }
                    return FALSE;
                }
            };

            // Pass pointer to vals array as lParam — arrives safely in WM_INITDIALOG
            float vals[3] = { g_bbFreqLow, g_bbFreqHigh, g_bbGainDB };
            INT_PTR res = DialogBoxIndirectParam(
                hInst2,
                (DLGTEMPLATE*)GlobalLock(hG),
                hwnd,
                DP::Proc,
                (LPARAM)vals);
            GlobalUnlock(hG);
            GlobalFree(hG);

            if (res == IDOK || res == 206) {
                g_bbFreqLow = max(20.0f, min(500.0f, vals[0]));
                g_bbFreqHigh = max(50.0f, min(2000.0f, vals[1]));
                g_bbGainDB = max(0.0f, min(24.0f, vals[2]));
                ApplyDSP();
                if (res == 206) SaveSettings();
            }
            break;
        }
        }
        break;

    case WM_PLAYNEXT:
        KillTimer(g_hwnd, IDT_PLAYBACK);
        SetWindowText(hTimeCur, L"0:00");
        SetWindowText(hTimeRemain, L"");
        InvalidateRect(hSeekCanvas, NULL, FALSE);
        if (g_repeat && g_currentIndex >= 0) PlayIndex(g_currentIndex);
        else PlayNext();
        break;

    case WM_DESTROY:
        SaveSettings();
        RemoveTrayIcon();
        KillTimer(g_hwnd, IDT_PLAYBACK);
        KillTimer(g_hwnd, IDT_SEEK_REPEAT);
        if (g_hwndSearch && IsWindow(g_hwndSearch))  DestroyWindow(g_hwndSearch);
        if (g_hwndInfo && IsWindow(g_hwndInfo))       DestroyWindow(g_hwndInfo);
        if (g_hwndConvert && IsWindow(g_hwndConvert)) DestroyWindow(g_hwndConvert);
        FreeArtBytes();
        DragAcceptFiles(hwnd, FALSE);
        if (currentStream) { BASS_ChannelStop(currentStream); BASS_StreamFree(currentStream); }
        BASS_Free();
        if (g_fontUI)   DeleteObject(g_fontUI);
        if (g_fontMono) DeleteObject(g_fontMono);
        if (g_fontBold) DeleteObject(g_fontBold);
        if (g_brBg)     DeleteObject(g_brBg);
        if (g_brList)   DeleteObject(g_brList);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//  Entry point
// ============================================================
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR cmdLine, int nCmdShow)
{
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"BillyProV4Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND ex = FindWindow(CLASS_NAME, NULL);
        if (ex) SetForegroundWindow(ex);
        return 0;
    }

    INITCOMMONCONTROLSEX icex = { sizeof(icex),ICC_BAR_CLASSES | ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icex);
    CoInitialize(NULL);

    RegisterCustomClasses(hInst);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc; wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
    if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // fallback
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, CLASS_NAME, APP_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 560,
        NULL, NULL, hInst, NULL);
    if (!hwnd) { CoUninitialize(); return 0; }

    // Set large + small icons explicitly so title bar and taskbar both show them
    HICON hIconBig = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
    HICON hIconSmall = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_BILLYPRO),
        IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    if (hIconBig)   SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu(), hPlay = CreatePopupMenu();
    HMENU hHelp = CreatePopupMenu(), hOpts = CreatePopupMenu();

    AppendMenu(hFile, MF_STRING, IDM_FILE_OPENFOLDER, L"Open Folder...\tCtrl+O");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_CONVERT, L"Convert...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT, L"Exit");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_PLAYPAUSE, L"Play / Pause\tSpace");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_STOP, L"Stop");
    AppendMenu(hPlay, MF_SEPARATOR, 0, NULL);
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_PREV, L"Previous\tP");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_NEXT, L"Next\tN");
    AppendMenu(hPlay, MF_SEPARATOR, 0, NULL);
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_SHUFFLE, L"Shuffle\tS");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_REPEAT, L"Repeat\tR");
    AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT, L"About...");
    AppendMenu(hHelp, MF_STRING, IDM_HELP_CONTROLS, L"Controls...");
    AppendMenu(hOpts, MF_STRING, IDM_OPTIONS, L"Options...");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPlay, L"&Play");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOpts, L"&Options");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"&Help");
    SetMenu(hwnd, hMenu);

    if (cmdLine && *cmdLine) {
        // Strip optional surrounding quotes
        wchar_t filePath[MAX_PATH] = {};
        wchar_t* p = cmdLine;
        if (*p == L'"') {
            p++;
            wchar_t* e = wcschr(p, L'"');
            if (e) { wcsncpy_s(filePath, p, e - p); }
            else { wcsncpy_s(filePath, p, _TRUNCATE); }
        }
        else {
            wcsncpy_s(filePath, p, _TRUNCATE);
        }

        DWORD a = GetFileAttributes(filePath);
        if (a != INVALID_FILE_ATTRIBUTES) {
            if (a & FILE_ATTRIBUTE_DIRECTORY) {
                // Opened with a folder argument
                LoadFolder(filePath);
            }
            else if (IsAudio(filePath)) {
                // Billy style: load entire parent folder, start on the clicked file
                wchar_t folder[MAX_PATH];
                wcsncpy_s(folder, filePath, _TRUNCATE);
                wchar_t* lastSep = wcsrchr(folder, L'\\');
                if (!lastSep) lastSep = wcsrchr(folder, L'/');
                if (lastSep) *lastSep = L'\0';

                LoadFolder(folder);

                // Find the exact file in the loaded playlist and play it
                int startIdx = 0;
                for (int i = 0; i < (int)g_playlist.size(); ++i) {
                    if (_wcsicmp(g_playlist[i].path, filePath) == 0) {
                        startIdx = i;
                        break;
                    }
                }
                PlayIndex(startIdx);
            }
        }
    }
    else {
        // No arguments: auto-load all audio files from the EXE's own folder (Billy style)
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileName(hInst, exePath, MAX_PATH);
        wchar_t* lastSep = wcsrchr(exePath, L'\\');
        if (lastSep) *lastSep = L'\0';
        if (exePath[0]) LoadFolder(exePath);
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        // ────────────────────────────────────────────────────
        //  GLOBAL KEY HANDLING
        //  Intercept WM_KEYDOWN before it reaches any control
        //  so Space/F/N/P/S/R/Up/Down work regardless of focus.
        //  Left/Right are forwarded to main WndProc for seek.
        // ────────────────────────────────────────────────────
        if (msg.message == WM_KEYDOWN) {
            // Don't steal keys when search edit has focus
            bool searchFocused = (g_hwndSearch && IsWindow(g_hwndSearch) &&
                GetFocus() == hSearchEdit);
            if (!searchFocused) {
                WPARAM vk = msg.wParam;
                // Ctrl+F opens search
                if (vk == 'F' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    OpenSearchDialog();
                    continue;
                }
                if (vk == VK_LEFT || vk == VK_RIGHT) {
                    // Route to main window for seek-hold logic
                    SendMessage(g_hwnd, WM_KEYDOWN, vk, msg.lParam);
                    continue;
                }
                if (HandleGlobalKey(vk)) continue;
            }
        }
        if (msg.message == WM_KEYUP) {
            if (msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT) {
                SendMessage(g_hwnd, WM_KEYUP, msg.wParam, msg.lParam);
                continue;
            }
        }
        // Let search window handle its own dialog messages
        if (g_hwndSearch && IsWindow(g_hwndSearch) &&
            IsDialogMessage(g_hwndSearch, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hMutex);
    CoUninitialize();
    return (int)msg.wParam;
}
