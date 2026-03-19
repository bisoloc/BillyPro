// BillyPro.cpp  -  Win32 + BASS audio player  v0.4
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
#include <string>
#include <vector>
#include <algorithm>
#include "bass.h"
#include "Resource.h"

// DTS decoding via dcadec
extern "C" {
#include "dca_context.h"
#include "dca_frame.h"
}


#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Uxtheme.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "winhttp.lib")
#include <winhttp.h>
#include <dwmapi.h>
#include <shobjidl.h>

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
#define ID_BTN_MEDIA        126
#define ID_BTN_DSP          127
#define IDM_HELP_CONTROLS   2302
#define IDM_OPTIONS         2401
#define IDM_OPTIONS_SHOW    2402

// Context menu
#define IDC_CTX_PLAY        301
#define IDC_CTX_REMOVE      302
#define IDC_CTX_PROPERTIES    303  // Windows shell Properties
#define IDC_CTX_AUDIOINFO     304  // Our custom audio info dialog
#define IDC_CTX_OPENLOCATION  305  // Open containing folder in Explorer
#define IDC_CTX_FAV_ADD       306  // Add to Favorites
#define IDC_CTX_FAV_REMOVE    307  // Remove from Favorites
#define IDC_CTX_PL_NEW        308  // Add to new playlist
#define IDC_CTX_PL_BASE       310  // 310..399 = existing playlists

// Library menu
#define IDM_LIB_FAVORITES     2501
#define IDM_LIB_PL_MANAGE     2502
#define IDM_LIB_PL_BASE       2510  // 2510..2599 = playlist entries

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
#define ID_CONV_SRATE       516

#define IDT_PLAYBACK        1
#define IDT_SEEK_REPEAT     2
#define IDT_PEAK_DECAY      3   // volume peak meter decay
#define IDT_PEAK_METER      4   // audio peak meter always-running
#define IDT_STATUS_SCROLL   5   // status bar text scroll
#define WM_PLAYNEXT         (WM_APP + 1)
#define WM_TRAYICON         (WM_APP + 2)
#define WM_CONV_PROGRESS    (WM_APP + 3)   // wParam=files done, lParam=MAKELPARAM(fileIdx,total)
#define WM_CONV_DONE        (WM_APP + 4)   // wParam=ok count, lParam=total
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
#define IDM_HELP_UPDATE     2303
// View menu
#define IDM_VIEW_DARKMODE       2201
// M3U playlist
#define IDM_FILE_OPENM3U        2005
#define IDM_FILE_SAVEM3U        2006
// Options extras
#define IDM_OPTIONS_FILEASSOC   2403
#define IDM_OPTIONS_MULTIINST   2404
// Hotkey IDs (for RegisterHotKey - must be unique)
#define ID_HOTKEY_PLAYPAUSE     801
#define ID_HOTKEY_NEXT          802
#define ID_HOTKEY_PREV          803
#define ID_HOTKEY_STOP          804
// File association dialog controls
#define ID_ASSOC_LIST           701
#define ID_ASSOC_SELALL         702
#define ID_ASSOC_NONE           703
#define ID_ASSOC_APPLY          704
#define ID_ASSOC_CLOSE          705
// WM_COPYDATA payload ID for IPC (open file in existing instance)
#define COPYDATAID_OPENFILE     1

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
// Dark mode: charcoal background, off-white text, blue accent
// COLORREFs are 0x00BBGGRR
static const Theme DARK = {
    0x00202020,  // bg       #202020
    0x002A2A2A,  // bgList   #2A2A2A
    0x00F0F0F0,  // text     #F0F0F0
    0x00909090,  // textDim  #909090
    0x00D2781E,  // accent   #1E78D2  (same blue)
    0x00404040,  // seekTrk  #404040
    0x00303030,  // btnFace  #303030
    0x00585858,  // btnBorder #585858
    0x00E8E8E8   // btnSym   #E8E8E8
};


// ============================================================
//  Globals
// ============================================================
static const wchar_t CLASS_NAME[] = L"BillyProWnd";
static const wchar_t SEEK_CLASS[] = L"BillySeekBar";
static const wchar_t VOL_CLASS[] = L"BillyVolBar";
static const wchar_t APP_TITLE[] = L"Billy Pro";
static const wchar_t APP_VERSION[] = L"0.6";

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
HWND hMediaBtn     = NULL;
HWND hVolumeCanvas = NULL;
HWND hSeekCanvas = NULL;
HWND hTimeCur = NULL;
HWND hTimeTot = NULL;
HWND hTimeRemain = NULL;
HWND hVolPct = NULL;
HWND hStatus = NULL;

HSTREAM currentStream = 0;
DWORD   g_masterFreq  = 0;     // master output stream sample rate
DWORD   g_masterChans = 0;     // master output stream channel count
float   currentVolume = 0.8f;
bool    g_rememberVolume = false;
bool    g_shuffle = false;
bool    g_repeat = false;
bool    g_mono = false;
bool    g_normalize = false;
wchar_t g_iniPath[MAX_PATH] = L"";
bool    g_bassBoost = false;
wchar_t g_mediaFolder[MAX_PATH] = L"";   // first folder (kept in sync with g_mediaFolders[0])
std::vector<std::wstring> g_mediaFolders; // all configured media root folders
bool    g_mediaActive = false;
bool    g_pitchEnabled = false;
float   g_pitchSemitones = 0.0f;  // -12 to +12 semitones (CDJ-style: changes pitch + tempo)
float   g_bbFreqLow = 30.0f;   // Hz
float   g_bbFreqHigh = 100.0f;  // Hz
float   g_bbGainDB = 5.0f;    // dB
// Recording
wchar_t g_recOutDir[MAX_PATH] = L"";
// Extra DSP effects
bool  g_dspReverb   = false;
bool  g_dspSaturate = false;
bool  g_dspVinyl    = false;
bool  g_dspHifi     = false;
bool  g_dspBypass   = false;   // true = DSP button bypasses all extra effects
HWND  hDspBtn       = NULL;
// Reverb params
float g_revMix    = 8.0f;    // 0-100  wet %
float g_revRoom   = 75.0f;   // 0-100  room size (maps to feedback 0.5-0.96)
float g_revWidth  = 80.0f;   // 0-100  stereo width (0=mono reverb, 100=full stereo)
// Saturation params
float g_satDrive  = 1.6f;    // 1.0-5.0  drive
float g_satLevel  = 72.0f;   // 0-100  output level %
// Vinyl params
float g_vinLpFreq = 17500.0f; // 2000-20000 Hz  LP cutoff
float g_vinCrackle= 15.0f;   // 0-100  crackle amount
// HiFi Amplifier params
float g_hfiBassDb = 3.0f;    // 0-12 dB  bass shelf boost
float g_hfiWarmth = 33.0f;   // 0-100  tube saturation %
// DSP handles
HDSP    g_dspMono   = 0;
HDSP    g_dspBass   = 0;
HDSP    g_dspRevHdl = 0;
HDSP    g_dspSatHdl = 0;
HDSP    g_dspVinHdl = 0;
HDSP    g_dspHfiHdl = 0;

// Gapless playback: decode streams fed into a master output via STREAMPROC
static HSTREAM g_decStream  = 0;     // current decode stream (BASS_STREAM_DECODE)
static HSTREAM g_decNext    = 0;     // pre-loaded next decode stream
static int     g_decNextIdx = -1;    // playlist index of next decode stream

// DTS decode state (forward-declared, defined in DTS section below)
struct DcaDecoder;
static DcaDecoder* g_dcaDec = nullptr;     // non-null when playing DTS stream
static double g_dcaDuration = 0;
static int    g_dcaSampleRate = 0;
static int    g_dcaChannels = 0;
static volatile int64_t g_dcaFramesOut = 0; // PCM frames decoded (for position)
static DcaDecoder* g_lastDcaDec = nullptr;  // set by CreateDtsStream, consumed by caller
static DcaDecoder* g_dcaDecNext = nullptr;  // preloaded next DTS decoder

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
int     g_seekRepeatCount = 0;   // how many IDT_SEEK_REPEAT ticks have fired (for fast-seek)
float   g_seekStep = 5.0f;   // seconds to skip with left/right arrow keys

Theme   g_theme = LIGHT;

static WNDPROC g_OldListProc   = NULL;
static WNDPROC g_OldBtnProc    = NULL;
static WNDPROC g_OldStatusProc = NULL;
static HWND    g_hoveredBtn = NULL;

static WNDPROC g_OldSearchProc = NULL;

struct Track { wchar_t path[MAX_PATH]; wchar_t display[MAX_PATH]; };
std::vector<Track> g_playlist;
std::vector<Track> g_savedPlaylist;  // saved when media view is active
std::vector<int>   g_shuffleOrder;
struct UndoRemove { Track t; int idx; };
std::vector<UndoRemove> g_undoRemove;  // last batch of removed tracks (for Ctrl+Z)
int g_currentIndex = -1;

struct MediaBrowserItem { wchar_t path[MAX_PATH]; wchar_t display[MAX_PATH]; bool isDir; };
std::vector<MediaBrowserItem> g_browserItems;
bool   g_browserActive = false;
wchar_t g_browserPath[MAX_PATH] = {};

// Right-click context track index
int  g_ctxTrackIndex  = -1;
bool g_ctxIsBrowser   = false;          // true when context menu was opened in browser mode
wchar_t g_ctxFilePath[MAX_PATH] = {};   // file path for the right-clicked item (works in both modes)

HFONT  g_fontUI = NULL;
HFONT  g_fontMono = NULL;
HFONT  g_fontBold = NULL;
HBRUSH g_brBg   = NULL;
HBRUSH g_brList = NULL;
HBRUSH g_brMenu = NULL;

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
static HANDLE g_convThread    = NULL;
static bool   g_convRunning   = false;
static bool   g_convAbort     = false;
static DWORD  g_convStartTick = 0;
bool   g_trayAdded    = false;   // system tray icon active
bool   g_darkMode     = false;   // dark theme enabled
bool   g_multiInst    = false;   // allow multiple instances (default: replace session)
bool   g_dropAppend   = true;    // drag & drop appends to playlist (false = replace)
HWND   g_hwndAssoc    = NULL;    // file associations dialog
HWND   g_hwndOptions  = NULL;    // options dialog

// Taskbar thumbnail toolbar (play/pause/skip controls)
static ITaskbarList3* g_pTaskbar          = NULL;
static HICON          g_thumbIcons[4]     = {};  // 0=prev, 1=play, 2=pause, 3=next
static UINT           g_WM_TASKBARBUTTONCREATED = 0;
#define THUMB_BTN_PREV   0
#define THUMB_BTN_PLAY   1
#define THUMB_BTN_NEXT   2

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

// libFLAC decoder struct + globals (implementation after DTS section)
#include <FLAC/stream_decoder.h>
struct FlacDecoder {
    FLAC__StreamDecoder* dec;
    int      sampleRate;
    int      nChannels;
    int      bitsPerSample;
    FLAC__uint64 totalSamples;
    double   totalDuration;
    float*   pcmBuf;
    int      pcmLen;
    int      pcmPos;
    int      pcmCap;
    volatile FLAC__uint64 seekTarget;
    FLAC__uint64 samplesOut;
    wchar_t  encoder[128];  // encoder tag from VORBIS_COMMENT
};
static FlacDecoder*  g_flacDec      = nullptr;
static FlacDecoder*  g_flacDecNext  = nullptr;
static FlacDecoder*  g_lastFlacDec  = nullptr;
static double        g_flacDuration = 0;
static int           g_flacSampleRate = 0;
static FLAC__uint64  g_flacSamplesOut = 0;
static void FlacSeek(double sec);
static double FlacGetPosition();
static bool IsFlacFile(const wchar_t* path);
static bool IsFavorite(const wchar_t* path);
static bool g_favActive = false;
// Status bar scroll state
static int    g_stScrollX = 0;
static int    g_stNameW   = 0;
static int    g_stNameAreaW = 0;
static bool   g_stScrolling = false;
static int    g_stPause = 0;
static wchar_t g_stLastText[512] = L"";
void TogglePlayPause();
void PlayNext();
void PlayPrev();
void SeekToSeconds(double sec);
void UpdateVolume();
void LayoutControls(HWND hwnd);
void PreloadNext();

void ApplyTheme();
static void MenuInitOwnerDraw(HMENU hm);
void RebuildShuffleOrder();
void UpdateWindowTitle();
void OpenSearchDialog();
void OpenAudioInfoDialog(int trackIdx);
void OpenAudioInfoForPath(const wchar_t* path);
void ShowTrackContextMenu(HWND hwnd, int trackIdx, POINT pt);
LRESULT CALLBACK SearchWndProc(HWND, UINT, WPARAM, LPARAM);
void OpenConvertDialog();
void OpenFileAssocDialog();
void OpenOptionsDialog();
void UpdateThumbButtons();
// KEY FIX: global accelerator-style message pre-filter
bool HandleGlobalKey(WPARAM vk);

// ============================================================
//  Helpers
// ============================================================
static void FormatTime(double sec, wchar_t* out, size_t n)
{
    if (sec < 0 || !_finite(sec)) { wcsncpy_s(out, n, L"--:--", _TRUNCATE); return; }
    int t = (int)sec;
    int h = t / 3600, m = (t % 3600) / 60, s = t % 60;
    if (h > 0)
        swprintf_s(out, n, L"%d:%02d:%02d", h, m, s);
    else
        swprintf_s(out, n, L"%d:%02d", m, s);
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

// Returns true if 'dir' is one of the configured media root folders
static bool IsMediaRoot(const wchar_t* dir)
{
    for (auto& f : g_mediaFolders)
        if (_wcsicmp(dir, f.c_str()) == 0) return true;
    return false;
}

// Media folder browser — fills listbox with folder contents (dirs first, then audio files).
// Pass empty string "" to show the virtual root (all configured media folders).
static void FillBrowser(const wchar_t* dirIn)
{
    // Make a local copy BEFORE clearing g_browserItems — the caller may have passed
    // a pointer into the vector's storage (e.g. it.path), which would be invalidated
    // by the clear() / push_back() calls below.
    wchar_t dir[MAX_PATH];
    wcsncpy_s(dir, dirIn, _TRUNCATE);

    wcsncpy_s(g_browserPath, dir, _TRUNCATE);
    g_browserItems.clear();
    if (hListBox) SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

    // Virtual root: list all configured media folders as directory entries
    if (dir[0] == L'\0') {
        for (auto& f : g_mediaFolders) {
            MediaBrowserItem it = {};
            wcsncpy_s(it.path, f.c_str(), _TRUNCATE);
            _snwprintf_s(it.display, _countof(it.display), _TRUNCATE, L"[+] %s", f.c_str());
            it.isDir = true;
            g_browserItems.push_back(it);
            if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)it.display);
        }
        return;
    }

    // "[..] go back" entry
    if (IsMediaRoot(dir)) {
        // At a root folder: go back to virtual root (only meaningful when multiple folders)
        if (g_mediaFolders.size() > 1) {
            MediaBrowserItem up = {}; wcscpy_s(up.display, L"[..] (go back)");
            up.isDir = true; up.path[0] = L'\0'; // empty = virtual root
            g_browserItems.push_back(up);
            if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)up.display);
        }
    } else {
        // Inside a subfolder: go up to parent
        MediaBrowserItem up = {}; wcscpy_s(up.display, L"[..] (go back)"); up.isDir = true;
        wchar_t parent[MAX_PATH]; wcsncpy_s(parent, dir, _TRUNCATE);
        wchar_t* sl = wcsrchr(parent, L'\\');
        if (sl) *sl = L'\0';
        wcsncpy_s(up.path, parent, _TRUNCATE);
        g_browserItems.push_back(up);
        if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)up.display);
    }

    // Enumerate directories first
    wchar_t pat[MAX_PATH + 4];
    _snwprintf_s(pat, _countof(pat), _TRUNCATE, L"%s\\*", dir);
    WIN32_FIND_DATA fd; HANDLE hf = FindFirstFile(pat, &fd);
    if (hf != INVALID_HANDLE_VALUE) {
        do {
            if (fd.cFileName[0] == L'.') continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                MediaBrowserItem it = {};
                _snwprintf_s(it.path, _countof(it.path), _TRUNCATE, L"%s\\%s", dir, fd.cFileName);
                _snwprintf_s(it.display, _countof(it.display), _TRUNCATE, L"[+] %s", fd.cFileName);
                it.isDir = true;
                g_browserItems.push_back(it);
                if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)it.display);
            }
        } while (FindNextFile(hf, &fd));
        FindClose(hf);
    }

    // Then audio files
    hf = FindFirstFile(pat, &fd);
    if (hf != INVALID_HANDLE_VALUE) {
        do {
            if (fd.cFileName[0] == L'.') continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            wchar_t full[MAX_PATH];
            _snwprintf_s(full, _countof(full), _TRUNCATE, L"%s\\%s", dir, fd.cFileName);
            if (IsAudio(full)) {
                MediaBrowserItem it = {};
                wcsncpy_s(it.path, full, _TRUNCATE);
                wcsncpy_s(it.display, fd.cFileName, _TRUNCATE);
                it.isDir = false;
                g_browserItems.push_back(it);
                if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)it.display);
            }
        } while (FindNextFile(hf, &fd));
        FindClose(hf);
    }
}

// Navigate a browser item: enter folder or load/play audio file
static void BrowserNavigate(int idx)
{
    if (idx < 0 || idx >= (int)g_browserItems.size()) return;
    // Copy item by value — FillBrowser will clear g_browserItems, invalidating any reference/pointer into it
    MediaBrowserItem it = g_browserItems[idx];
    if (it.isDir) {
        FillBrowser(it.path);
    } else {
        // Build playlist silently from browser audio files (for next/prev), stay in browser mode
        wchar_t clickedPath[MAX_PATH]; wcsncpy_s(clickedPath, it.path, _TRUNCATE);
        g_playlist.clear();
        g_shuffleOrder.clear();
        g_currentIndex = -1;
        int foundIdx = -1;
        for (auto& bi : g_browserItems) {
            if (!bi.isDir) {
                Track t = {}; wcsncpy_s(t.path, bi.path, _TRUNCATE);
                wcsncpy_s(t.display, bi.display, _TRUNCATE);
                if (_wcsicmp(t.path, clickedPath) == 0) foundIdx = (int)g_playlist.size();
                g_playlist.push_back(t);
            }
        }
        RebuildShuffleOrder();
        // Play — g_browserActive stays true, so PlayIndex won't touch the listbox
        if (foundIdx >= 0) PlayIndex(foundIdx);
    }
}

void AddTrack(const wchar_t* path)
{
    Track t; wcsncpy_s(t.path, path, _TRUNCATE);
    wcsncpy_s(t.display, Filename(path), _TRUNCATE);
    g_playlist.push_back(t);
    if (hListBox) {
        wchar_t disp[MAX_PATH + 4];
        if (!g_favActive && IsFavorite(path))
            swprintf_s(disp, L"\u2605 %s", t.display);
        else
            wcsncpy_s(disp, t.display, _TRUNCATE);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
    }
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
//  Extra DSP effects
// ============================================================

// --- Reverb (4 comb + 2 all-pass per channel, Freeverb stereo) ---
// R channel uses +23-sample delay offsets so L and R tails diverge even on mono input
static const int REV_COMB_L[4] = { 1557, 1617, 1491, 1422 };
static const int REV_COMB_R[4] = { 1580, 1640, 1514, 1445 }; // +23 Freeverb offset
static const int REV_AP_L[2]   = { 225, 341 };
static const int REV_AP_R[2]   = { 236, 352 };                // +11 offset
static float g_revCombBufL[4][1700]={}, g_revCombBufR[4][1700]={};
static float g_revApBufL[2][400]={},    g_revApBufR[2][400]={};
static int   g_revCombPosL[4]={}, g_revCombPosR[4]={};
static int   g_revApPosL[2]={},   g_revApPosR[2]={};

void CALLBACK DSP_Reverb(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf  = (float*)buffer;
    DWORD  n    = length / sizeof(float);
    float wet    = g_revMix   / 100.0f;
    float fb     = 0.50f + (g_revRoom  / 100.0f) * 0.46f;  // 0.50-0.96
    float wscale = g_revWidth / 200.0f;   // 0-0.5 for width matrix
    float dry    = 1.0f - wet;
    for (DWORD i = 0; i + 1 < n; i += 2) {
        float inL = buf[i], inR = buf[i + 1];
        float outL = 0, outR = 0;
        for (int c = 0; c < 4; c++) {
            float dL = g_revCombBufL[c][g_revCombPosL[c]];
            float dR = g_revCombBufR[c][g_revCombPosR[c]];
            g_revCombBufL[c][g_revCombPosL[c]] = inL + dL * fb;
            g_revCombBufR[c][g_revCombPosR[c]] = inR + dR * fb;
            if (++g_revCombPosL[c] >= REV_COMB_L[c]) g_revCombPosL[c] = 0;
            if (++g_revCombPosR[c] >= REV_COMB_R[c]) g_revCombPosR[c] = 0;
            outL += dL; outR += dR;
        }
        outL *= 0.25f; outR *= 0.25f;
        for (int a = 0; a < 2; a++) {
            float dL = g_revApBufL[a][g_revApPosL[a]];
            float dR = g_revApBufR[a][g_revApPosR[a]];
            g_revApBufL[a][g_revApPosL[a]] = outL + dL * 0.5f;
            g_revApBufR[a][g_revApPosR[a]] = outR + dR * 0.5f;
            if (++g_revApPosL[a] >= REV_AP_L[a]) g_revApPosL[a] = 0;
            if (++g_revApPosR[a] >= REV_AP_R[a]) g_revApPosR[a] = 0;
            outL = dL - outL * 0.5f;
            outR = dR - outR * 0.5f;
        }
        // Width matrix: 0=mono reverb, 100=full L/R separation (great for mono→stereo)
        float wL = outL * (0.5f + wscale) + outR * (0.5f - wscale);
        float wR = outR * (0.5f + wscale) + outL * (0.5f - wscale);
        buf[i]     = dry * inL + wet * wL;
        buf[i + 1] = dry * inR + wet * wR;
    }
}

// --- Saturation (soft-clip waveshaper, tube warmth) ---
// Crossover LP state for saturation (200 Hz split)
static float g_satLpL = 0, g_satLpR = 0;

void CALLBACK DSP_Saturate(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf  = (float*)buffer;
    DWORD  n    = length / sizeof(float);
    float drive = g_satDrive;
    float wet   = (g_satLevel / 100.0f) * 0.35f;  // full wet for highs
    float dry   = 1.0f - wet;
    // 1-pole LP at 200 Hz — splits bass from mids/highs
    const float lpA = expf(-6.28318f * 200.0f / 44100.0f);  // ~0.972
    for (DWORD i = 0; i + 1 < n; i += 2) {
        float inL = buf[i], inR = buf[i + 1];
        // Low band (under 200 Hz)
        g_satLpL = lpA * g_satLpL + (1.0f - lpA) * inL;
        g_satLpR = lpA * g_satLpR + (1.0f - lpA) * inR;
        float loL = g_satLpL, loR = g_satLpR;
        // High band (200 Hz and up)
        float hiL = inL - loL, hiR = inR - loR;
        // Saturate high band normally, low band at only 5%
        float satHiL = hiL * drive; satHiL = satHiL / (1.0f + fabsf(satHiL));
        float satHiR = hiR * drive; satHiR = satHiR / (1.0f + fabsf(satHiR));
        float satLoL = loL * drive; satLoL = satLoL / (1.0f + fabsf(satLoL));
        float satLoR = loR * drive; satLoR = satLoR / (1.0f + fabsf(satLoR));
        buf[i]     = (hiL * dry + satHiL * wet) + (loL * 0.95f + satLoL * 0.05f);
        buf[i + 1] = (hiR * dry + satHiR * wet) + (loR * 0.95f + satLoR * 0.05f);
    }
}

// --- Vinyl Emulation (high-freq roll-off + smooth crackle pops + surface hiss + M-S stereo) ---
static float g_vinLpL        = 0, g_vinLpR       = 0;
static float g_vinHissL      = 0, g_vinHissR      = 0;
static DWORD g_vinCrackTimer = 0;
static float g_vinCrackEnvL  = 0, g_vinCrackEnvR  = 0;
static float g_vinCrackDecay = 0.993f;
// Subtle chorus on side channel (M-S stereo field)
static float g_vinChorusBuf[1024] = {};
static int   g_vinChorusPos  = 0;
static float g_vinChorusLFO  = 0.0f;  // phase 0..1
// Subtle pitch wobble on mid (mono)
static float g_vinMidBuf[512] = {};
static int   g_vinMidPos  = 0;
static float g_vinMidLFO  = 0.5f;  // offset from side LFO to avoid sync

void CALLBACK DSP_Vinyl(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf = (float*)buffer;
    DWORD  n   = length / sizeof(float);

    float freq       = max(500.0f, min(20000.0f, g_vinLpFreq));
    float alpha      = expf(-6.28318f * freq / 44100.0f);
    float crackScale = g_vinCrackle / 100.0f;
    // Surface hiss amplitude: subtle baseline + crackle-scaled component
    float hissAmp    = 0.0003f + crackScale * 0.0018f;
    // Hiss LP alpha: ~5 kHz rolloff for warm surface noise character
    const float hA   = expf(-6.28318f * 5000.0f / 44100.0f);

    for (DWORD i = 0; i + 1 < n; i += 2) {
        // HF roll-off on signal
        g_vinLpL = alpha * g_vinLpL + (1.0f - alpha) * buf[i];
        g_vinLpR = alpha * g_vinLpR + (1.0f - alpha) * buf[i + 1];
        buf[i]     = g_vinLpL;
        buf[i + 1] = g_vinLpR;

        // ── M-S stereo field ──────────────────────────────────────────────
        float mid  = (buf[i] + buf[i + 1]) * 0.5f;
        float side = (buf[i] - buf[i + 1]) * 0.5f;

        // Gentle saturation on mid (warm mono core, like a slightly overdriven tube)
        mid = mid / (1.0f + fabsf(mid) * 0.5f) * 1.08f;

        // Barely noticeable pitch wobble on mid — slow warble like an old turntable
        g_vinMidBuf[g_vinMidPos] = mid;
        float midLfo = sinf(g_vinMidLFO * 6.28318f);
        g_vinMidLFO += 0.6f / 44100.0f;    // 0.6 Hz — slow wow, more noticeable
        if (g_vinMidLFO >= 1.0f) g_vinMidLFO -= 1.0f;
        float midDelay = 20.0f + midLfo * 18.0f;  // wider sweep, more pitch movement
        int   md0  = (int)midDelay;
        float mfrac = midDelay - (float)md0;
        float m0 = g_vinMidBuf[(g_vinMidPos - md0 + 512) & 511];
        float m1 = g_vinMidBuf[(g_vinMidPos - md0 - 1 + 512) & 511];
        mid = m0 + mfrac * (m1 - m0);
        g_vinMidPos = (g_vinMidPos + 1) & 511;

        // Subtle chorus on side channel only (~2% depth, barely noticeable)
        g_vinChorusBuf[g_vinChorusPos] = side;
        float lfo = sinf(g_vinChorusLFO * 6.28318f);
        g_vinChorusLFO += 0.35f / 44100.0f;   // 0.35 Hz LFO rate
        if (g_vinChorusLFO >= 1.0f) g_vinChorusLFO -= 1.0f;
        float delaySmp = 485.0f + lfo * 88.0f;  // ~11ms base ± 2ms depth
        int   d0   = (int)delaySmp;
        float frac = delaySmp - (float)d0;
        float s0   = g_vinChorusBuf[(g_vinChorusPos - d0 + 1024) & 1023];
        float s1   = g_vinChorusBuf[(g_vinChorusPos - d0 - 1 + 1024) & 1023];
        float chorSide = s0 + frac * (s1 - s0);
        g_vinChorusPos = (g_vinChorusPos + 1) & 1023;
        side = side * 0.90f + chorSide * 0.10f;  // 10% wet

        // Reconstruct L/R
        buf[i]     = mid + side;
        buf[i + 1] = mid - side;

        if (crackScale > 0.0f) {
            // Surface hiss: independent L/R noise through warm LP
            float nL = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            float nR = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            g_vinHissL = hA * g_vinHissL + (1.0f - hA) * nL;
            g_vinHissR = hA * g_vinHissR + (1.0f - hA) * nR;
            buf[i]     += g_vinHissL * hissAmp;
            buf[i + 1] += g_vinHissR * hissAmp;

            // Crackle/pop envelope decay
            g_vinCrackEnvL *= g_vinCrackDecay;
            g_vinCrackEnvR *= g_vinCrackDecay;

            // Apply crackle with warm soft-clip (removes harsh transient edge)
            float cL = g_vinCrackEnvL, cR = g_vinCrackEnvR;
            buf[i]     += cL / (1.0f + fabsf(cL) * 6.0f);
            buf[i + 1] += cR / (1.0f + fabsf(cR) * 6.0f);

            // Schedule next crackle event
            if (g_vinCrackTimer == 0) {
                // Interval: sparse at low crackle, denser at high — with wide randomness
                DWORD baseInt = (DWORD)(44100.0f * 1.8f / max(0.01f, crackScale));
                g_vinCrackTimer = baseInt / 3 + (DWORD)(rand() % (baseInt * 2 + 1));

                // Amplitude varies per pop
                float amp = crackScale * (0.03f + (rand() % 90) * 0.001f);

                // Decay time: 20-100ms so crackles fade naturally, not click
                int decaySmp = 880 + rand() % 3530;  // 20ms to 100ms @ 44100
                g_vinCrackDecay = expf(-1.0f / (float)decaySmp);

                // Side pop: one channel dominant, other gets warm bleed
                float dominant = 0.55f + (rand() % 45) * 0.01f;   // 0.55-1.0
                float bleed    = (rand() % 35) * 0.01f;            // 0.0-0.34
                if (rand() & 1) {
                    g_vinCrackEnvL += amp * dominant;
                    g_vinCrackEnvR += amp * bleed;
                } else {
                    g_vinCrackEnvL += amp * bleed;
                    g_vinCrackEnvR += amp * dominant;
                }
            }
            if (g_vinCrackTimer > 0) g_vinCrackTimer--;
        }
    }
}

// --- HiFi Amplifier (warm bass shelf + soft saturation) ---
static float g_hfiBssL = 0, g_hfiBssR = 0;

void CALLBACK DSP_Hifi(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf = (float*)buffer;
    DWORD  n   = length / sizeof(float);
    const float alpha   = 0.9888f;   // 1-pole LP ~80 Hz @ 44100 Hz (fixed shelf freq)
    float bassAdd = (powf(10.0f, g_hfiBassDb / 20.0f) - 1.0f);  // linear gain to add
    float warmth  = (g_hfiWarmth / 100.0f) * 0.8f;               // 0-0.8 soft-clip coeff
    for (DWORD i = 0; i + 1 < n; i += 2) {
        float inL = buf[i], inR = buf[i + 1];
        g_hfiBssL = alpha * g_hfiBssL + (1.0f - alpha) * inL;
        g_hfiBssR = alpha * g_hfiBssR + (1.0f - alpha) * inR;
        float outL = inL + g_hfiBssL * bassAdd;
        float outR = inR + g_hfiBssR * bassAdd;
        if (warmth > 0.0f) {
            buf[i]     = outL / (1.0f + fabsf(outL) * warmth);
            buf[i + 1] = outR / (1.0f + fabsf(outR) * warmth);
        } else {
            buf[i]     = outL;
            buf[i + 1] = outR;
        }
    }
}

// ============================================================
//  INI settings
// ============================================================
static void GetIniPath()
{
    if (g_iniPath[0]) return;
    wchar_t dir[MAX_PATH] = {};
    // Store in %APPDATA%\BillyPro\ — no elevation needed
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, dir))) {
        // Fallback: same directory as exe
        GetModuleFileName(NULL, dir, MAX_PATH);
        wchar_t* sl = wcsrchr(dir, L'\\'); if (sl) sl[1] = 0;
    } else {
        wcscat_s(dir, L"\\BillyPro");
        CreateDirectoryW(dir, NULL);  // creates if not exists; no-op if already there
        wcscat_s(dir, L"\\");
    }
    wcscpy_s(g_iniPath, dir);
    wcscat_s(g_iniPath, L"BillyPro.ini");
}

// ============================================================
//  Library: Favorites + Playlists (.bpp files)
//  Stored in %APPDATA%\BillyPro\library\{favorites,playlists}\
// ============================================================
static wchar_t g_libDir[MAX_PATH] = L"";    // %APPDATA%/BillyPro/library
static wchar_t g_plDir[MAX_PATH]  = L"";    // .../library/playlists
static wchar_t g_favDir[MAX_PATH] = L"";    // .../library/favorites
static HMENU   g_hLibMenu = NULL;            // Library popup (for dynamic rebuild)
static HMENU   g_hPlSubMenu = NULL;          // Playlists cascading submenu

static void GetLibDir()
{
    if (g_libDir[0]) return;
    GetIniPath();
    // Extract base dir from ini path (%APPDATA%\BillyPro\)
    wchar_t base[MAX_PATH];
    wcsncpy_s(base, g_iniPath, _TRUNCATE);
    wchar_t* sl = wcsrchr(base, L'\\');
    if (sl) sl[1] = 0;
    swprintf_s(g_libDir, L"%slibrary", base);
    CreateDirectoryW(g_libDir, NULL);
    swprintf_s(g_plDir, L"%s\\playlists", g_libDir);
    CreateDirectoryW(g_plDir, NULL);
    swprintf_s(g_favDir, L"%s\\favorites", g_libDir);
    CreateDirectoryW(g_favDir, NULL);
}

// --- Favorites ---
static std::vector<std::wstring> g_favorites;  // paths of favorited tracks

static void LoadFavorites()
{
    GetLibDir();
    wchar_t favPath[MAX_PATH];
    swprintf_s(favPath, L"%s\\favorites.bpp", g_favDir);
    g_favorites.clear();
    FILE* f = _wfopen(favPath, L"r,ccs=UTF-8");
    if (!f) return;
    wchar_t line[MAX_PATH];
    while (fgetws(line, MAX_PATH, f)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len > 0) g_favorites.push_back(line);
    }
    fclose(f);
}

static void SaveFavorites()
{
    GetLibDir();
    wchar_t favPath[MAX_PATH];
    swprintf_s(favPath, L"%s\\favorites.bpp", g_favDir);
    FILE* f = _wfopen(favPath, L"w,ccs=UTF-8");
    if (!f) return;
    for (auto& p : g_favorites) fwprintf(f, L"%s\n", p.c_str());
    fclose(f);
}

static bool IsFavorite(const wchar_t* path)
{
    for (auto& p : g_favorites)
        if (_wcsicmp(p.c_str(), path) == 0) return true;
    return false;
}

// Rebuild listbox display text with ★ prefix for favorited tracks
static void RefreshListboxStars()
{
    if (!hListBox) return;
    int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
    // Preserve multi-selection state
    int count = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    std::vector<bool> selState(count);
    for (int i = 0; i < count; i++)
        selState[i] = (SendMessage(hListBox, LB_GETSEL, i, 0) > 0);

    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < (int)g_playlist.size(); i++) {
        wchar_t disp[MAX_PATH + 4];
        if (!g_favActive && IsFavorite(g_playlist[i].path))
            swprintf_s(disp, L"\u2605 %s", g_playlist[i].display);  // ★
        else
            wcsncpy_s(disp, g_playlist[i].display, _TRUNCATE);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
    }
    // Restore selection
    for (int i = 0; i < min(count, (int)g_playlist.size()); i++)
        if (selState[i]) SendMessage(hListBox, LB_SETSEL, TRUE, i);
    if (sel >= 0 && sel < (int)g_playlist.size())
        SendMessage(hListBox, LB_SETCURSEL, sel, 0);
}

static void AddFavorite(const wchar_t* path)
{
    if (!IsFavorite(path)) {
        g_favorites.push_back(path);
        SaveFavorites();
        RefreshListboxStars();
    }
}

static void RemoveFavorite(const wchar_t* path)
{
    for (auto it = g_favorites.begin(); it != g_favorites.end(); ++it) {
        if (_wcsicmp(it->c_str(), path) == 0) {
            g_favorites.erase(it);
            SaveFavorites();
            RefreshListboxStars();
            return;
        }
    }
}

// --- Playlists (.bpp) ---
struct BppPlaylist {
    std::wstring name;      // display name (filename without extension)
    std::wstring filePath;  // full path to .bpp file
};
static std::vector<BppPlaylist> g_bppPlaylists;

static void ScanPlaylists()
{
    GetLibDir();
    g_bppPlaylists.clear();
    wchar_t pattern[MAX_PATH];
    swprintf_s(pattern, L"%s\\*.bpp", g_plDir);
    WIN32_FIND_DATAW fd;
    HANDLE hf = FindFirstFileW(pattern, &fd);
    if (hf == INVALID_HANDLE_VALUE) return;
    do {
        BppPlaylist pl;
        pl.name = fd.cFileName;
        // Strip .bpp extension for display
        size_t dot = pl.name.rfind(L'.');
        if (dot != std::wstring::npos) pl.name = pl.name.substr(0, dot);
        wchar_t full[MAX_PATH];
        swprintf_s(full, L"%s\\%s", g_plDir, fd.cFileName);
        pl.filePath = full;
        g_bppPlaylists.push_back(pl);
    } while (FindNextFileW(hf, &fd));
    FindClose(hf);
}

static void LoadBppPlaylist(const wchar_t* path)
{
    FILE* f = _wfopen(path, L"r,ccs=UTF-8");
    if (!f) return;
    std::vector<std::wstring> paths;
    wchar_t line[MAX_PATH];
    while (fgetws(line, MAX_PATH, f)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len > 0) paths.push_back(line);
    }
    fclose(f);
    if (paths.empty()) return;

    // Load into playlist
    g_playlist.clear();
    for (auto& p : paths) {
        Track t;
        wcsncpy_s(t.path, MAX_PATH, p.c_str(), _TRUNCATE);
        const wchar_t* fn = wcsrchr(t.path, L'\\');
        wcsncpy_s(t.display, MAX_PATH, fn ? fn + 1 : t.path, _TRUNCATE);
        g_playlist.push_back(t);
    }
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
    }
    g_currentIndex = -1;
    UpdateStatusBar();
    UpdateWindowTitle();
}

static void AddTrackToPlaylist(const wchar_t* bppPath, const wchar_t* trackPath)
{
    // Append track path to .bpp file
    FILE* f = _wfopen(bppPath, L"a,ccs=UTF-8");
    if (!f) return;
    fwprintf(f, L"%s\n", trackPath);
    fclose(f);
}

static void CreateNewPlaylist(const wchar_t* name, const wchar_t* initialTrack)
{
    GetLibDir();
    wchar_t path[MAX_PATH];
    swprintf_s(path, MAX_PATH, L"%s\\%s.bpp", g_plDir, name);
    FILE* f = _wfopen(path, L"w,ccs=UTF-8");
    if (!f) return;
    if (initialTrack && initialTrack[0])
        fwprintf(f, L"%s\n", initialTrack);
    fclose(f);
    ScanPlaylists();
}

static void ShowNewPlaylistDialog(HWND hwnd, const wchar_t* trackPath)
{
    wchar_t name[128] = L"";
    // Simple input: use a prompt-like approach with a small dialog
    // For now, use a simple input box via a helper
    wchar_t prompt[256];
    swprintf_s(prompt, L"Enter playlist name:");
    // Use a basic hack: set window text and get it back
    // Actually, just use a simple approach with GetSaveFileName
    OPENFILENAMEW ofn = {};
    wchar_t file[MAX_PATH] = L"New Playlist.bpp";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Billy Pro Playlist (*.bpp)\0*.bpp\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    GetLibDir();
    ofn.lpstrInitialDir = g_plDir;
    ofn.lpstrTitle = L"Create New Playlist";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"bpp";
    if (GetSaveFileName(&ofn)) {
        FILE* f = _wfopen(file, L"w,ccs=UTF-8");
        if (f) {
            if (trackPath && trackPath[0])
                fwprintf(f, L"%s\n", trackPath);
            fclose(f);
        }
        ScanPlaylists();
    }
}

static std::vector<Track> g_libSavedPlaylist;  // saved playlist before favorites/playlist view
static int g_libSavedIndex = -1;
static int g_libActivePlaylist = -1;           // -1=none, -2=favorites, 0+=playlist index

// Restore the original playlist from before any library view was activated
static void LibRestorePlaylist()
{
    if (g_libActivePlaylist == -1) return; // nothing to restore
    g_favActive = false;
    g_libActivePlaylist = -1;
    g_playlist = g_libSavedPlaylist;
    g_libSavedPlaylist.clear();
    g_currentIndex = g_libSavedIndex;
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        if (g_currentIndex >= 0) {
            SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
            SendMessage(hListBox, LB_SETSEL, TRUE, g_currentIndex);
            SendMessage(hListBox, LB_SETCURSEL, g_currentIndex, 0);
        }
    }
    RefreshListboxStars();
    UpdateStatusBar();
    UpdateWindowTitle();
    HMENU hm = GetMenu(g_hwnd);
    if (hm) CheckMenuItem(hm, IDM_LIB_FAVORITES, MF_BYCOMMAND | MF_UNCHECKED);
}

// Save current playlist if not already saved by a library view
static void LibSaveIfNeeded()
{
    if (g_libActivePlaylist == -1) {
        g_libSavedPlaylist = g_playlist;
        g_libSavedIndex = g_currentIndex;
    }
}

// Load paths into the playlist listbox
static void LibShowTracks(const std::vector<std::wstring>& paths)
{
    g_playlist.clear();
    for (auto& p : paths) {
        Track t;
        wcsncpy_s(t.path, MAX_PATH, p.c_str(), _TRUNCATE);
        const wchar_t* fn = wcsrchr(t.path, L'\\');
        wcsncpy_s(t.display, MAX_PATH, fn ? fn + 1 : t.path, _TRUNCATE);
        g_playlist.push_back(t);
    }
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
    }
    g_currentIndex = -1;
    g_browserActive = false;
    RefreshListboxStars();
    UpdateStatusBar();
    UpdateWindowTitle();
}

static void LoadFavoritesIntoPlaylist()
{
    // Toggle OFF if already showing favorites
    if (g_libActivePlaylist == -2) {
        LibRestorePlaylist();
        return;
    }

    if (g_favorites.empty()) {
        MessageBox(g_hwnd, L"No favorites saved yet.\n\nRight-click a track and select 'Add to Favorites'.",
            L"Favorites", MB_ICONINFORMATION);
        return;
    }

    LibSaveIfNeeded();
    g_favActive = true;
    g_libActivePlaylist = -2;
    LibShowTracks(g_favorites);
    HMENU hm = GetMenu(g_hwnd);
    if (hm) CheckMenuItem(hm, IDM_LIB_FAVORITES, MF_BYCOMMAND | MF_CHECKED);
}

static void LoadBppIntoPlaylist(int plIdx)
{
    // Toggle OFF if already showing this playlist
    if (g_libActivePlaylist == plIdx) {
        LibRestorePlaylist();
        return;
    }

    if (plIdx < 0 || plIdx >= (int)g_bppPlaylists.size()) return;

    // Read .bpp file
    FILE* f = _wfopen(g_bppPlaylists[plIdx].filePath.c_str(), L"r,ccs=UTF-8");
    if (!f) return;
    std::vector<std::wstring> paths;
    wchar_t line[MAX_PATH];
    while (fgetws(line, MAX_PATH, f)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len > 0) paths.push_back(line);
    }
    fclose(f);
    if (paths.empty()) return;

    LibSaveIfNeeded();
    g_favActive = false;
    g_libActivePlaylist = plIdx;
    LibShowTracks(paths);
    // Uncheck favorites if it was checked
    HMENU hm = GetMenu(g_hwnd);
    if (hm) CheckMenuItem(hm, IDM_LIB_FAVORITES, MF_BYCOMMAND | MF_UNCHECKED);
}

void SaveSettings()
{
    GetIniPath();
    wchar_t buf[64];
    swprintf_s(buf, L"%d", (int)g_bassBoost);  WritePrivateProfileString(L"DSP", L"BassBoost", buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_bbFreqLow);        WritePrivateProfileString(L"DSP", L"BBFreqLow", buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_bbFreqHigh);       WritePrivateProfileString(L"DSP", L"BBFreqHigh", buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_bbGainDB);         WritePrivateProfileString(L"DSP", L"BBGainDB", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_mono);        WritePrivateProfileString(L"DSP", L"Mono",     buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_normalize);   WritePrivateProfileString(L"DSP", L"Normalize",buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspReverb);   WritePrivateProfileString(L"DSP", L"Reverb",   buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspSaturate); WritePrivateProfileString(L"DSP", L"Saturate", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspVinyl);    WritePrivateProfileString(L"DSP", L"Vinyl",    buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspHifi);     WritePrivateProfileString(L"DSP", L"Hifi",     buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_revMix);     WritePrivateProfileString(L"DSP", L"RevMix",    buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_revRoom);    WritePrivateProfileString(L"DSP", L"RevRoom",   buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_revWidth);   WritePrivateProfileString(L"DSP", L"RevWidth",  buf, g_iniPath);
    swprintf_s(buf, L"%.2f", g_satDrive);   WritePrivateProfileString(L"DSP", L"SatDrive",  buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_satLevel);   WritePrivateProfileString(L"DSP", L"SatLevel",  buf, g_iniPath);
    swprintf_s(buf, L"%.0f", g_vinLpFreq);  WritePrivateProfileString(L"DSP", L"VinFreq",   buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_vinCrackle); WritePrivateProfileString(L"DSP", L"VinCrackle",buf, g_iniPath);
    swprintf_s(buf, L"%.2f", g_hfiBassDb);  WritePrivateProfileString(L"DSP", L"HfiBass",   buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_hfiWarmth);  WritePrivateProfileString(L"DSP", L"HfiWarmth", buf, g_iniPath);
    // Recording output dir
    WritePrivateProfileString(L"Paths", L"RecordOutDir", g_recOutDir, g_iniPath);
    // UI prefs
    swprintf_s(buf, L"%d", (int)g_darkMode);   WritePrivateProfileString(L"UI", L"DarkMode",      buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_multiInst);  WritePrivateProfileString(L"UI", L"MultiInstance", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dropAppend); WritePrivateProfileString(L"UI", L"DropAppend",    buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_shuffle);    WritePrivateProfileString(L"UI", L"Shuffle",       buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_repeat);     WritePrivateProfileString(L"UI", L"Repeat",        buf, g_iniPath);
    swprintf_s(buf, L"%.1f", g_seekStep);      WritePrivateProfileString(L"UI", L"SeekStep",      buf, g_iniPath);
    // Save all media folders as pipe-separated list
    {
        std::wstring joined;
        for (auto& f : g_mediaFolders) { if (!joined.empty()) joined += L'|'; joined += f; }
        WritePrivateProfileString(L"Paths", L"MediaFolders", joined.c_str(), g_iniPath);
        // Keep legacy key in sync (first folder)
        WritePrivateProfileString(L"Paths", L"MediaFolder", g_mediaFolder, g_iniPath);
    }
    swprintf_s(buf, L"%d", (int)g_pitchEnabled); WritePrivateProfileString(L"Pitch", L"Enabled", buf, g_iniPath);
    swprintf_s(buf, L"%.2f", g_pitchSemitones);  WritePrivateProfileString(L"Pitch", L"Semitones", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_rememberVolume); WritePrivateProfileString(L"UI", L"RememberVolume", buf, g_iniPath);
    if (g_rememberVolume) {
        swprintf_s(buf, L"%.3f", currentVolume);
        WritePrivateProfileString(L"UI", L"Volume", buf, g_iniPath);
    }
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
    g_mono        = GETI(L"DSP", L"Mono",     0) != 0;
    g_normalize   = GETI(L"DSP", L"Normalize",0) != 0;
    g_dspReverb   = GETI(L"DSP", L"Reverb",   0) != 0;
    g_dspSaturate = GETI(L"DSP", L"Saturate", 0) != 0;
    g_dspVinyl    = GETI(L"DSP", L"Vinyl",    0) != 0;
    g_dspHifi     = GETI(L"DSP", L"Hifi",     0) != 0;
    g_revMix     = GETF(L"DSP", L"RevMix",    8.0);
    g_revRoom    = GETF(L"DSP", L"RevRoom",   75.0);
    g_revWidth   = GETF(L"DSP", L"RevWidth",  80.0);
    g_satDrive   = GETF(L"DSP", L"SatDrive",  1.6);
    g_satLevel   = GETF(L"DSP", L"SatLevel",  72.0);
    g_vinLpFreq  = GETF(L"DSP", L"VinFreq",   17500.0);
    g_vinCrackle = GETF(L"DSP", L"VinCrackle",15.0);
    g_hfiBassDb  = GETF(L"DSP", L"HfiBass",   3.0);
    g_hfiWarmth  = GETF(L"DSP", L"HfiWarmth", 33.0);
    GetPrivateProfileString(L"Paths", L"RecordOutDir", L"", g_recOutDir, MAX_PATH, g_iniPath);
    g_darkMode   = GETI(L"UI", L"DarkMode",      0) != 0;
    g_multiInst  = GETI(L"UI", L"MultiInstance", 0) != 0;
    g_dropAppend = GETI(L"UI", L"DropAppend",    1) != 0;
    g_shuffle    = GETI(L"UI", L"Shuffle",       0) != 0;
    g_repeat     = GETI(L"UI", L"Repeat",        0) != 0;
    g_seekStep   = GETF(L"UI", L"SeekStep",      5.0);
    if (g_seekStep < 1.0f) g_seekStep = 1.0f;
    // Load media folders — try new pipe-separated key first, fall back to legacy single key
    g_mediaFolders.clear();
    {
        wchar_t foldersRaw[4096] = {};
        GetPrivateProfileString(L"Paths", L"MediaFolders", L"", foldersRaw, _countof(foldersRaw), g_iniPath);
        if (foldersRaw[0]) {
            // Split on '|'
            wchar_t* ctx = nullptr;
            wchar_t* tok = wcstok_s(foldersRaw, L"|", &ctx);
            while (tok) {
                if (tok[0]) g_mediaFolders.push_back(tok);
                tok = wcstok_s(nullptr, L"|", &ctx);
            }
        } else {
            // Legacy: single MediaFolder key
            GetPrivateProfileString(L"Paths", L"MediaFolder", L"", g_mediaFolder, MAX_PATH, g_iniPath);
            if (g_mediaFolder[0]) g_mediaFolders.push_back(g_mediaFolder);
        }
        // Keep g_mediaFolder in sync with first entry
        if (!g_mediaFolders.empty())
            wcsncpy_s(g_mediaFolder, g_mediaFolders[0].c_str(), _TRUNCATE);
        else
            g_mediaFolder[0] = L'\0';
    }
    g_pitchEnabled  = GETI(L"Pitch", L"Enabled",   0) != 0;
    g_pitchSemitones = GETF(L"Pitch", L"Semitones", 0.0);
    g_pitchSemitones = max(-12.0f, min(12.0f, g_pitchSemitones));
    g_rememberVolume = GETI(L"UI", L"RememberVolume", 0) != 0;
    if (g_rememberVolume) {
        float vol = GETF(L"UI", L"Volume", 0.8f);
        currentVolume = max(0.0f, min(1.0f, vol));
    }
#undef GETI
#undef GETF
}


// ============================================================
//  M3U Playlist save / load
// ============================================================
static void SavePlaylistM3U(HWND hwndParent)
{
    if (g_playlist.empty()) {
        MessageBox(hwndParent, L"Playlist is empty.", L"Save Playlist", MB_ICONWARNING);
        return;
    }
    OPENFILENAME ofn = {}; ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndParent;
    wchar_t path[MAX_PATH] = L"playlist.m3u";
    ofn.lpstrFilter  = L"M3U Playlist\0*.m3u\0All Files\0*.*\0";
    ofn.lpstrFile    = path;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrDefExt  = L"m3u";
    ofn.Flags        = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!GetSaveFileName(&ofn)) return;

    FILE* f = nullptr; _wfopen_s(&f, path, L"w,ccs=UTF-8");
    if (!f) { MessageBox(hwndParent, L"Cannot open file for writing.", L"Save Playlist", MB_ICONERROR); return; }
    fwprintf(f, L"#EXTM3U\r\n");
    for (auto& t : g_playlist)
        fwprintf(f, L"#EXTINF:-1,%s\r\n%s\r\n", t.display, t.path);
    fclose(f);
    wchar_t msg[MAX_PATH + 64];
    swprintf_s(msg, L"Saved %d track(s) to playlist.", (int)g_playlist.size());
    MessageBox(hwndParent, msg, L"Save Playlist", MB_ICONINFORMATION);
}

static bool IsM3U(const wchar_t* p)
{
    const wchar_t* e = wcsrchr(p, L'.');
    return e && (_wcsicmp(e, L".m3u") == 0 || _wcsicmp(e, L".m3u8") == 0);
}

// Load an M3U file from a known path; appends or replaces based on g_dropAppend
static void LoadM3UFromPath(const wchar_t* path, bool append = false)
{
    FILE* f = nullptr; _wfopen_s(&f, path, L"r,ccs=UTF-8");
    if (!f) return;
    if (!append) ClearPlaylist();
    wchar_t line[MAX_PATH * 2];
    while (fgetws(line, _countof(line), f)) {
        int len = (int)wcslen(line);
        while (len > 0 && (line[len-1] == L'\r' || line[len-1] == L'\n')) line[--len] = 0;
        if (len == 0 || line[0] == L'#') continue;
        if (GetFileAttributesW(line) != INVALID_FILE_ATTRIBUTES && IsAudio(line))
            AddTrack(line);
    }
    fclose(f);
    RebuildShuffleOrder();
    if (!g_playlist.empty() && g_currentIndex < 0) {
        g_currentIndex = 0; SendMessage(hListBox, LB_SETCURSEL, 0, 0);
    }
    UpdateStatusBar();
}

static void LoadPlaylistM3U(HWND hwndParent)
{
    OPENFILENAME ofn = {}; ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndParent;
    wchar_t path[MAX_PATH] = L"";
    ofn.lpstrFilter = L"M3U Playlist\0*.m3u;*.m3u8\0All Files\0*.*\0";
    ofn.lpstrFile   = path;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileName(&ofn)) return;
    LoadM3UFromPath(path, false);
}

// Scan peak for normalization on any file path (returns peak level)
static float ScanPeak(const wchar_t* path)
{
    float peak = 0.001f;
    HSTREAM scan = BASS_StreamCreateFile(FALSE, path, 0, 0,
        BASS_UNICODE | BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
    if (scan) {
        float fbuf[4096]; DWORD got; int chunks = 0;
        while (chunks < 200 &&
            (got = BASS_ChannelGetData(scan, fbuf, sizeof(fbuf))) != (DWORD)-1 && got > 0) {
            DWORD n = got / sizeof(float);
            for (DWORD j = 0; j < n; j++) {
                float a = fabsf(fbuf[j]);
                if (a > peak) peak = a;
            }
            chunks++;
        }
        BASS_StreamFree(scan);
    }
    return peak;
}

void ApplyDSP()
{
    if (!currentStream) return;
    // Remove all DSP
    if (g_dspMono)   { BASS_ChannelRemoveDSP(currentStream, g_dspMono);   g_dspMono   = 0; }
    if (g_dspBass)   { BASS_ChannelRemoveDSP(currentStream, g_dspBass);   g_dspBass   = 0; }
    if (g_dspRevHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspRevHdl); g_dspRevHdl = 0; }
    if (g_dspSatHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspSatHdl); g_dspSatHdl = 0; }
    if (g_dspVinHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspVinHdl); g_dspVinHdl = 0; }
    if (g_dspHfiHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspHfiHdl); g_dspHfiHdl = 0; }
    // Reapply
    if (g_mono)        g_dspMono   = BASS_ChannelSetDSP(currentStream, DSP_Mono,     NULL, 1);
    if (g_bassBoost)   g_dspBass   = BASS_ChannelSetDSP(currentStream, DSP_BassBoost,NULL, 2);
    if (!g_dspBypass) {
        if (g_dspReverb)   g_dspRevHdl = BASS_ChannelSetDSP(currentStream, DSP_Reverb,  NULL, 3);
        if (g_dspSaturate) g_dspSatHdl = BASS_ChannelSetDSP(currentStream, DSP_Saturate,NULL, 4);
        if (g_dspVinyl)    g_dspVinHdl = BASS_ChannelSetDSP(currentStream, DSP_Vinyl,   NULL, 5);
        if (g_dspHifi)     g_dspHfiHdl = BASS_ChannelSetDSP(currentStream, DSP_Hifi,    NULL, 6);
    }
    // Update DSP button visibility
    if (g_hwnd) LayoutControls(g_hwnd);
    // Normalization on decode stream, user volume on master output
    if (g_decStream && g_normalize && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
        float peak = ScanPeak(g_playlist[g_currentIndex].path);
        float normScale = 1.0f / peak;
        if (normScale > 4.0f) normScale = 4.0f;
        BASS_ChannelSetAttribute(g_decStream, BASS_ATTRIB_VOL, normScale);
    }
    BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
}

// ============================================================
//  Pitch (CDJ-style: changes pitch + tempo together via sample rate)
// ============================================================
void ApplyPitch()
{
    if (!currentStream) return;
    if (g_pitchEnabled && g_pitchSemitones != 0.0f) {
        BASS_CHANNELINFO ci = {};
        BASS_ChannelGetInfo(currentStream, &ci);
        float origFreq = (float)ci.freq;
        float newFreq  = origFreq * (float)pow(2.0, g_pitchSemitones / 12.0);
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_FREQ, newFreq);
    } else {
        // Reset to default (0 = original freq)
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_FREQ, 0);
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
    // Free decode streams first (before master, since GaplessProc reads them)
    g_dcaDec = nullptr;
    g_dcaDecNext = nullptr;
    g_lastDcaDec = nullptr;
    g_dcaDuration = 0;
    g_dcaSampleRate = 0;
    g_dcaChannels = 0;
    g_dcaFramesOut = 0;
    g_flacDec = nullptr;
    g_flacDecNext = nullptr;
    g_lastFlacDec = nullptr;
    g_flacDuration = 0;
    g_flacSampleRate = 0;
    g_flacSamplesOut = 0;
    if (g_decNext) { BASS_StreamFree(g_decNext); g_decNext = 0; }
    g_decNextIdx = -1;
    if (g_decStream) { BASS_StreamFree(g_decStream); g_decStream = 0; }
    if (currentStream) {
        BASS_ChannelStop(currentStream);
        BASS_StreamFree(currentStream);
        currentStream = 0;
    }
    g_masterFreq = 0;
    g_masterChans = 0;
    g_peakLevel = 0.0f; g_peakHold = 0.0f; g_audioPeakHold = 0.0f; g_audioPeakTime = 0; g_bassLevel = 0.0f;
    if (hTimeCur)    SetWindowText(hTimeCur, L"0:00");
    if (hTimeTot)    SetWindowText(hTimeTot, L"/ 0:00");
    if (hTimeRemain) SetWindowText(hTimeRemain, L"");
    if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
    if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
    UpdatePlayBtn();
    UpdateStatusBar();
    UpdateThumbButtons();
    SetWindowText(g_hwnd, APP_TITLE);
}

// Stream handle that the UI thread should free (deferred from callback)
static volatile HSTREAM g_decOldToFree = 0;
// Set during seek to prevent GaplessProc from misinterpreting -1 as end-of-stream
static volatile LONG g_seeking = 0;

// Master output STREAMPROC: reads from decode streams, seamless switch on track end
static DWORD CALLBACK GaplessProc(HSTREAM handle, void* buffer, DWORD length, void* user)
{
    BYTE* buf = (BYTE*)buffer;
    DWORD total = 0;

    while (total < length) {
        if (!g_decStream) break;

        DWORD got = BASS_ChannelGetData(g_decStream, buf + total, length - total);
        if (got == (DWORD)-1) {
            // If a seek is in progress, BASS_ChannelGetData may fail transiently.
            // Output silence and retry on next callback instead of switching tracks.
            if (g_seeking) {
                memset(buf + total, 0, length - total);
                return length;
            }
            got = 0; // genuine end of stream
        }

        total += got;

        if (total < length) {
            // Current decode stream exhausted — check if next track format matches
            // Don't free here (we're on the mixing thread) — defer to UI thread
            HSTREAM oldDec = g_decStream;
            g_decStream = 0;
            if (g_decNext) {
                BASS_CHANNELINFO ni = {};
                BASS_ChannelGetInfo(g_decNext, &ni);
                if (ni.freq == g_masterFreq && ni.chans == g_masterChans) {
                    // Same format — seamless gapless transition
                    g_decStream = g_decNext;
                    g_decNext = 0;
                    g_decOldToFree = oldDec;
                    PostMessage((HWND)user, WM_PLAYNEXT, 1, 0);
                } else {
                    // Format mismatch — can't feed into current master stream.
                    g_decOldToFree = oldDec;
                    PostMessage((HWND)user, WM_PLAYNEXT, 2, 0);
                    break;
                }
            } else {
                g_decOldToFree = oldDec;
                PostMessage((HWND)user, WM_PLAYNEXT, 0, 0);
                break;
            }
        }
    }

    if (total == 0) return BASS_STREAMPROC_END;
    return total;
}

// Fired when the master output finishes (end of playlist, no more data)
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

// Speed ratio introduced by pitch shift (>1 = faster, <1 = slower)
static double GetPitchSpeedRatio()
{
    if (!g_pitchEnabled || g_pitchSemitones == 0.0f) return 1.0;
    return pow(2.0, g_pitchSemitones / 12.0);
}

void SeekToSeconds(double sec)
{
    if (!currentStream) return;
    if (sec < 0) sec = 0;
    double rawSec = sec * GetPitchSpeedRatio();

    // DTS seek via dcadec direct file I/O
    if (g_dcaDec) {
        extern void DcaSeek(double sec);
        DcaSeek(rawSec);
        if (currentStream) BASS_ChannelPlay(currentStream, TRUE);
        UpdateTimeDisplays();
        if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
        return;
    }

    // FLAC seek via libFLAC (fast binary search, ~20ms)
    if (g_flacDec && g_flacDec->dec && g_flacSampleRate > 0) {
        FLAC__uint64 target = (FLAC__uint64)(rawSec * g_flacSampleRate);
        if (g_flacDec->totalSamples > 0 && target >= g_flacDec->totalSamples)
            target = g_flacDec->totalSamples - 1;
        // Lock master stream so mixing thread can't read stale data during seek
        BASS_ChannelLock(currentStream, TRUE);
        g_flacDec->pcmLen = 0;
        g_flacDec->pcmPos = 0;
        g_flacDec->seekTarget = (FLAC__uint64)-1; // clear any pending async seek
        FLAC__stream_decoder_seek_absolute(g_flacDec->dec, target);
        g_flacDec->samplesOut = target;
        g_flacSamplesOut = target;
        // Flush master output buffer so new position plays immediately
        BASS_ChannelSetPosition(currentStream, 0, BASS_POS_BYTE);
        BASS_ChannelLock(currentStream, FALSE);
        UpdateTimeDisplays();
        if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
        return;
    }

    // Normal: seek on decode stream (MP3/WAV/OGG — BASS seeking is fast).
    // Lock to prevent GaplessProc race, flush to discard stale buffer.
    HSTREAM seekTarget = g_decStream ? g_decStream : currentStream;
    double rawLen = BASS_ChannelBytes2Seconds(seekTarget,
        BASS_ChannelGetLength(seekTarget, BASS_POS_BYTE));
    if (rawSec > rawLen) rawSec = rawLen;
    BASS_ChannelLock(currentStream, TRUE);
    InterlockedExchange(&g_seeking, 1);
    BASS_ChannelSetPosition(seekTarget,
        BASS_ChannelSeconds2Bytes(seekTarget, rawSec), BASS_POS_BYTE);
    InterlockedExchange(&g_seeking, 0);
    BASS_ChannelSetPosition(currentStream, 0, BASS_POS_BYTE);
    BASS_ChannelLock(currentStream, FALSE);
    UpdateTimeDisplays();
    if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
}

static double DcaGetPosition()
{
    if (!g_dcaDec || g_dcaSampleRate <= 0) return 0;
    return (double)g_dcaFramesOut / g_dcaSampleRate;
}

static double GetTrackLength()
{
    if (g_dcaDec && g_dcaDuration > 0)
        return g_dcaDuration / GetPitchSpeedRatio();
    if (g_flacDec && g_flacDuration > 0)
        return g_flacDuration / GetPitchSpeedRatio();
    HSTREAM s = g_decStream ? g_decStream : currentStream;
    if (!s) return 1.0;
    double len = BASS_ChannelBytes2Seconds(s,
        BASS_ChannelGetLength(s, BASS_POS_BYTE));
    len /= GetPitchSpeedRatio();
    return (len > 0) ? len : 1.0;
}

static double GetPlayPos()
{
    if (g_dcaDec) return DcaGetPosition() / GetPitchSpeedRatio();
    if (g_flacDec) return FlacGetPosition() / GetPitchSpeedRatio();
    HSTREAM s = g_decStream ? g_decStream : currentStream;
    if (!s) return 0.0;
    return BASS_ChannelBytes2Seconds(s,
        BASS_ChannelGetPosition(s, BASS_POS_BYTE))
        / GetPitchSpeedRatio();
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

// ============================================================
//  DTS-WAV decoding via dcadec with direct FILE* I/O (seekable)
// ============================================================

// DTS sync words
#define DTS_SYNC_CORE_BE   0x7FFE8001u
#define DTS_SYNC_CORE_LE   0xFE7F0180u
#define DTS_SYNC_CORE_BE14 0x1FFFE800u
#define DTS_SYNC_CORE_LE14 0xFF1F00E8u

struct DcaDecoder {
    FILE*            fp;
    dcadec_context*  ctx;
    int64_t          dataOffset;     // WAV 'data' chunk file offset
    int64_t          dataSize;       // WAV 'data' chunk size
    int              sampleRate;
    int              nChannels;      // output channels (2)
    int              bitsPerSample;
    int              samplesPerFrame;
    int              avgFrameBytes;  // average DTS frame size in bytes
    double           totalDuration;
    // PCM output buffer
    float*           pcmBuf;
    int              pcmLen;
    int              pcmPos;
    // Frame buffer
    uint8_t*         frameBuf;
    size_t           frameBufCap;
    bool             eof;
    int64_t          seekTarget;     // seek target in PCM frames (-1 = none)
    int64_t          pcmFramesOut;   // total PCM frames output
};

static bool IsDtsWav(const wchar_t* path)
{
    FILE* f = nullptr;
    _wfopen_s(&f, path, L"rb");
    if (!f) return false;
    uint8_t hdr[12];
    if (fread(hdr, 1, 12, f) != 12) { fclose(f); return false; }
    if (memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) { fclose(f); return false; }
    while (true) {
        uint8_t ch[8];
        if (fread(ch, 1, 8, f) != 8) { fclose(f); return false; }
        uint32_t sz = ch[4] | (ch[5] << 8) | (ch[6] << 16) | (ch[7] << 24);
        if (memcmp(ch, "data", 4) == 0) break;
        fseek(f, sz, SEEK_CUR);
    }
    uint8_t sync[4];
    if (fread(sync, 1, 4, f) != 4) { fclose(f); return false; }
    fclose(f);
    uint32_t s = ((uint32_t)sync[0] << 24) | ((uint32_t)sync[1] << 16) | ((uint32_t)sync[2] << 8) | sync[3];
    return (s == DTS_SYNC_CORE_BE || s == DTS_SYNC_CORE_LE || s == DTS_SYNC_CORE_BE14 || s == DTS_SYNC_CORE_LE14);
}

// Scan forward from current file position to find next DTS sync word.
// Returns true if found, file positioned at start of sync word.
static bool DcaScanSync(FILE* fp, int64_t maxBytes)
{
    uint32_t sync = 0;
    for (int64_t i = 0; i < maxBytes; i++) {
        int c = fgetc(fp);
        if (c == EOF) return false;
        sync = (sync << 8) | (uint8_t)c;
        if (sync == DTS_SYNC_CORE_BE || sync == DTS_SYNC_CORE_LE ||
            sync == DTS_SYNC_CORE_BE14 || sync == DTS_SYNC_CORE_LE14) {
            _fseeki64(fp, -4, SEEK_CUR); // back up to start of sync
            return true;
        }
    }
    return false;
}

// Read one DTS frame from file, convert bitstream, return size (0 = EOF/error)
static size_t DcaReadFrame(DcaDecoder* d)
{
    if (!DcaScanSync(d->fp, 65536)) return 0;

    uint8_t header[DCADEC_FRAME_HEADER_SIZE];
    if (fread(header, 1, DCADEC_FRAME_HEADER_SIZE, d->fp) != DCADEC_FRAME_HEADER_SIZE)
        return 0;

    size_t frameSize = 0;
    if (dcadec_frame_parse_header(header, &frameSize) < 0) return 0;

    size_t bufSize = dcadec_frame_buffer_size(frameSize);
    if (bufSize > d->frameBufCap) {
        free(d->frameBuf);
        d->frameBuf = (uint8_t*)malloc(bufSize);
        d->frameBufCap = bufSize;
    }
    memset(d->frameBuf, 0, bufSize);
    memcpy(d->frameBuf, header, DCADEC_FRAME_HEADER_SIZE);
    size_t rest = frameSize - DCADEC_FRAME_HEADER_SIZE;
    if (fread(d->frameBuf + DCADEC_FRAME_HEADER_SIZE, 1, rest, d->fp) != rest)
        return 0;

    size_t convSize = 0;
    if (dcadec_frame_convert_bitstream(d->frameBuf, &convSize, d->frameBuf, frameSize) < 0)
        return 0;

    return convSize;
}

// Decode one frame and fill pcmBuf. Returns true if got samples.
static bool DcaDecodeFrame(DcaDecoder* d)
{
    size_t frameSize = DcaReadFrame(d);
    if (frameSize == 0) { d->eof = true; return false; }

    if (dcadec_context_parse(d->ctx, d->frameBuf, frameSize) < 0)
        return false; // skip bad frame, caller retries

    int** samples = nullptr;
    int ns = 0, cm = 0, sr = 0, bps = 0, prof = 0;
    if (dcadec_context_filter(d->ctx, &samples, &ns, &cm, &sr, &bps, &prof) < 0)
        return false;
    if (ns <= 0 || !samples) return false;

    d->sampleRate = sr;
    d->bitsPerSample = bps;
    if (d->samplesPerFrame == 0) d->samplesPerFrame = ns;

    int nch = 0;
    for (int b = 0; b < 32; b++) if (cm & (1 << b)) nch++;
    if (nch < 1) return false;
    int outCh = min(nch, 2);
    d->nChannels = outCh;

    int needed = ns * outCh;
    if (d->pcmLen < needed) {
        delete[] d->pcmBuf;
        d->pcmBuf = new float[needed];
    }
    d->pcmLen = needed;
    d->pcmPos = 0;
    d->pcmFramesOut += ns;
    g_dcaFramesOut = d->pcmFramesOut;

    float scale = 1.0f / (float)(1 << (bps - 1));
    for (int i = 0; i < ns; i++) {
        d->pcmBuf[i * outCh] = samples[0][i] * scale;
        if (outCh >= 2)
            d->pcmBuf[i * outCh + 1] = (nch >= 2) ? samples[1][i] * scale : samples[0][i] * scale;
    }
    return true;
}

void DcaSeek(double sec)
{
    DcaDecoder* d = g_dcaDec;
    if (!d || d->dataSize <= 0 || d->totalDuration <= 0) return;

    // Calculate byte offset in WAV data chunk
    double frac = sec / d->totalDuration;
    if (frac < 0) frac = 0; if (frac > 1) frac = 1;
    int64_t byteOff = (int64_t)(frac * d->dataSize);

    // Seek to that position and scan for next sync word
    _fseeki64(d->fp, d->dataOffset + byteOff, SEEK_SET);

    // Reset decoder state
    dcadec_context_clear(d->ctx);
    d->pcmPos = d->pcmLen = 0;
    d->eof = false;
    d->pcmFramesOut = (int64_t)(sec * d->sampleRate);
    g_dcaFramesOut = d->pcmFramesOut;
}

static void DcaFreeData(DcaDecoder* d)
{
    if (!d) return;
    if (d == g_dcaDec) {
        g_dcaDec = nullptr;
        g_dcaDuration = 0;
        g_dcaSampleRate = 0;
        g_dcaChannels = 0;
    }
    if (d->ctx) dcadec_context_destroy(d->ctx);
    if (d->fp)  fclose(d->fp);
    free(d->frameBuf);
    delete[] d->pcmBuf;
    delete d;
}

// BASS STREAMPROC: on-the-fly DTS decode
static DWORD CALLBACK DcaStreamProc(HSTREAM handle, void* buffer, DWORD length, void* user)
{
    DcaDecoder* d = (DcaDecoder*)user;
    float* out = (float*)buffer;
    DWORD wanted = length / sizeof(float);
    DWORD written = 0;

    // Handle pending seek
    if (d->seekTarget >= 0) {
        DcaSeek((double)d->seekTarget / d->sampleRate);
        d->seekTarget = -1;
    }

    while (written < wanted && !d->eof) {
        if (d->pcmPos < d->pcmLen) {
            DWORD avail = (DWORD)(d->pcmLen - d->pcmPos);
            DWORD copy = min(avail, wanted - written);
            memcpy(out + written, d->pcmBuf + d->pcmPos, copy * sizeof(float));
            d->pcmPos += copy;
            written += copy;
        } else {
            if (!DcaDecodeFrame(d)) {
                if (!d->eof) continue; // skip bad frame
                break;
            }
        }
    }

    if (written == 0) return BASS_STREAMPROC_END;
    return written * sizeof(float);
}

// Open WAV file, find data chunk, estimate duration, create BASS decode stream
static HSTREAM CreateDtsStream(const wchar_t* path)
{
    FILE* fp = nullptr;
    _wfopen_s(&fp, path, L"rb");
    if (!fp) return 0;

    // Parse WAV header to find 'data' chunk
    uint8_t hdr[12];
    if (fread(hdr, 1, 12, fp) != 12) { fclose(fp); return 0; }
    if (memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) { fclose(fp); return 0; }

    int64_t dataOffset = 0, dataSize = 0;
    while (true) {
        uint8_t ch[8];
        if (fread(ch, 1, 8, fp) != 8) { fclose(fp); return 0; }
        uint32_t sz = ch[4] | (ch[5] << 8) | (ch[6] << 16) | (ch[7] << 24);
        if (memcmp(ch, "data", 4) == 0) {
            dataOffset = _ftelli64(fp);
            dataSize = sz;
            break;
        }
        _fseeki64(fp, sz, SEEK_CUR);
    }

    // Create decoder context
    dcadec_context* ctx = dcadec_context_create(DCADEC_FLAG_KEEP_DMIX_2CH);
    if (!ctx) { fclose(fp); return 0; }

    DcaDecoder* d = new DcaDecoder{};
    d->fp = fp;
    d->ctx = ctx;
    d->dataOffset = dataOffset;
    d->dataSize = dataSize;
    d->frameBuf = nullptr;
    d->frameBufCap = 0;
    d->pcmBuf = nullptr;
    d->pcmLen = 0;
    d->pcmPos = 0;
    d->eof = false;
    d->seekTarget = -1;
    d->pcmFramesOut = 0;
    d->samplesPerFrame = 0;
    d->avgFrameBytes = 0;

    // Decode first frame to get format info
    if (!DcaDecodeFrame(d)) {
        DcaFreeData(d); return 0;
    }

    // Estimate duration from file size
    int64_t firstFrameFilePos = _ftelli64(fp);
    int64_t firstFrameBytes = firstFrameFilePos - dataOffset;
    d->avgFrameBytes = (int)firstFrameBytes;
    if (d->avgFrameBytes > 0 && d->samplesPerFrame > 0 && d->sampleRate > 0) {
        int64_t totalFrames = dataSize / d->avgFrameBytes;
        d->totalDuration = (double)(totalFrames * d->samplesPerFrame) / d->sampleRate;
    }

    HSTREAM h = BASS_StreamCreate(d->sampleRate, d->nChannels,
        BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE, DcaStreamProc, d);
    if (!h) { DcaFreeData(d); return 0; }

    BASS_ChannelSetSync(h, BASS_SYNC_FREE, 0,
        [](HSYNC, DWORD, DWORD, void* user) {
            DcaFreeData((DcaDecoder*)user);
        }, d);

    // Don't set g_dcaDec here — caller decides when this becomes the active stream
    // Store info for caller to pick up
    g_lastDcaDec = d;
    return h;
}

// Activate DTS decoder as the current playing stream
static void DcaActivate()
{
    g_dcaDec = g_lastDcaDec;
    if (g_dcaDec) {
        g_dcaDuration = g_dcaDec->totalDuration;
        g_dcaSampleRate = g_dcaDec->sampleRate;
        g_dcaChannels = g_dcaDec->nChannels;
        g_dcaFramesOut = g_dcaDec->pcmFramesOut;
    }
    g_lastDcaDec = nullptr;
}

// ============================================================
//  libFLAC direct decoder — fast seeking for FLAC files
// ============================================================

static FLAC__StreamDecoderWriteStatus FlacWriteCb(
    const FLAC__StreamDecoder*, const FLAC__Frame* frame,
    const FLAC__int32* const buffer[], void* client_data)
{
    FlacDecoder* d = (FlacDecoder*)client_data;
    int ns = (int)frame->header.blocksize;
    int ch = d->nChannels;
    int bps = d->bitsPerSample;
    float scale = 1.0f / (float)(1 << (bps - 1));

    // Ensure buffer capacity
    int need = ns * ch;
    if (d->pcmLen + need > d->pcmCap) {
        int newCap = max(d->pcmCap * 2, d->pcmLen + need + 8192);
        float* nb = (float*)realloc(d->pcmBuf, newCap * sizeof(float));
        if (!nb) return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        d->pcmBuf = nb;
        d->pcmCap = newCap;
    }

    // Interleave channels into float buffer
    float* out = d->pcmBuf + d->pcmLen;
    for (int i = 0; i < ns; i++) {
        for (int c = 0; c < ch; c++) {
            *out++ = (float)buffer[min(c, (int)frame->header.channels - 1)][i] * scale;
        }
    }
    d->pcmLen += need;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void FlacMetaCb(const FLAC__StreamDecoder*, const FLAC__StreamMetadata* metadata, void* client_data)
{
    FlacDecoder* d = (FlacDecoder*)client_data;
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        d->sampleRate    = (int)metadata->data.stream_info.sample_rate;
        d->nChannels     = (int)metadata->data.stream_info.channels;
        d->bitsPerSample = (int)metadata->data.stream_info.bits_per_sample;
        d->totalSamples  = metadata->data.stream_info.total_samples;
        if (d->sampleRate > 0 && d->totalSamples > 0)
            d->totalDuration = (double)d->totalSamples / d->sampleRate;
    }
    else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        const FLAC__StreamMetadata_VorbisComment& vc = metadata->data.vorbis_comment;
        for (FLAC__uint32 i = 0; i < vc.num_comments; i++) {
            const char* entry = (const char*)vc.comments[i].entry;
            if (_strnicmp(entry, "ENCODER=", 8) == 0) {
                MultiByteToWideChar(CP_UTF8, 0, entry + 8, -1, d->encoder, 127);
                d->encoder[127] = 0;
            }
        }
    }
}

static void FlacErrorCb(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void*) {}

static void FlacFreeData(FlacDecoder* d)
{
    if (!d) return;
    if (d->dec) {
        FLAC__stream_decoder_finish(d->dec);
        FLAC__stream_decoder_delete(d->dec);
    }
    free(d->pcmBuf);
    delete d;
}

// BASS STREAMPROC: reads decoded PCM from FlacDecoder
static DWORD CALLBACK FlacStreamProc(HSTREAM handle, void* buffer, DWORD length, void* user)
{
    FlacDecoder* d = (FlacDecoder*)user;
    if (!d || !d->dec) return BASS_STREAMPROC_END;

    // Handle pending seek
    if (d->seekTarget != (FLAC__uint64)-1) {
        FLAC__uint64 target = d->seekTarget;
        d->seekTarget = (FLAC__uint64)-1;
        d->pcmLen = 0;
        d->pcmPos = 0;
        FLAC__stream_decoder_seek_absolute(d->dec, target);
        d->samplesOut = target;
    }

    BYTE* buf = (BYTE*)buffer;
    DWORD total = 0;

    while (total < length) {
        // Copy available decoded data
        int avail = (d->pcmLen - d->pcmPos) * (int)sizeof(float);
        if (avail > 0) {
            DWORD copy = min((DWORD)avail, length - total);
            memcpy(buf + total, d->pcmBuf + d->pcmPos, copy);
            d->pcmPos += (int)(copy / sizeof(float));
            d->samplesOut += copy / sizeof(float) / d->nChannels;
            total += copy;
            // Compact buffer when fully consumed
            if (d->pcmPos >= d->pcmLen) {
                d->pcmLen = 0;
                d->pcmPos = 0;
            }
        }

        if (total >= length) break;

        // Decode more frames
        d->pcmLen = 0;
        d->pcmPos = 0;
        if (!FLAC__stream_decoder_process_single(d->dec)) break;
        FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(d->dec);
        if (state == FLAC__STREAM_DECODER_END_OF_STREAM) break;
        if (state >= FLAC__STREAM_DECODER_OGG_ERROR) break;
    }

    if (total == 0) return BASS_STREAMPROC_END;
    return total;
}

static bool IsFlacFile(const wchar_t* path)
{
    const wchar_t* ext = wcsrchr(path, L'.');
    return ext && _wcsicmp(ext, L".flac") == 0;
}

static HSTREAM CreateFlacStream(const wchar_t* path)
{
    FlacDecoder* d = new FlacDecoder();
    memset(d, 0, sizeof(*d));
    d->seekTarget = (FLAC__uint64)-1;
    d->pcmCap = 65536;
    d->pcmBuf = (float*)malloc(d->pcmCap * sizeof(float));

    d->dec = FLAC__stream_decoder_new();
    if (!d->dec) { FlacFreeData(d); return 0; }
    // Request VORBIS_COMMENT metadata (for encoder tag)
    FLAC__stream_decoder_set_metadata_respond(d->dec, FLAC__METADATA_TYPE_VORBIS_COMMENT);

    // Convert wide path to UTF-8 for libFLAC (avoids CRT mismatch with FILE*)
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    char* utf8Path = (char*)_alloca(utf8Len + 1);
    WideCharToMultiByte(CP_UTF8, 0, path, -1, utf8Path, utf8Len + 1, NULL, NULL);

    FLAC__StreamDecoderInitStatus status = FLAC__stream_decoder_init_file(
        d->dec, utf8Path, FlacWriteCb, FlacMetaCb, FlacErrorCb, d);
    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        FlacFreeData(d);
        return 0;
    }

    // Read metadata to get format info
    FLAC__stream_decoder_process_until_end_of_metadata(d->dec);
    if (d->sampleRate <= 0 || d->nChannels <= 0) {
        FlacFreeData(d);
        return 0;
    }

    // Clamp to stereo output
    int outCh = min(d->nChannels, 2);

    // Create BASS decode stream matching the FLAC format
    HSTREAM h = BASS_StreamCreate(d->sampleRate, outCh, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE,
        FlacStreamProc, d);
    if (!h) { FlacFreeData(d); return 0; }

    // Free on stream destroy
    BASS_ChannelSetSync(h, BASS_SYNC_FREE, 0,
        [](HSYNC, DWORD, DWORD, void* user) { FlacFreeData((FlacDecoder*)user); }, d);

    g_lastFlacDec = d;
    return h;
}

static void FlacSeek(double sec)
{
    if (!g_flacDec || !g_flacDec->dec || g_flacSampleRate <= 0) return;
    FLAC__uint64 target = (FLAC__uint64)(sec * g_flacSampleRate);
    if (target >= g_flacDec->totalSamples) target = g_flacDec->totalSamples > 0 ? g_flacDec->totalSamples - 1 : 0;
    // Set pending seek — FlacStreamProc will execute it on the mixing thread
    g_flacDec->seekTarget = target;
    g_flacSamplesOut = target;
}

static double FlacGetPosition()
{
    if (!g_flacDec || g_flacSampleRate <= 0) return 0;
    return (double)g_flacDec->samplesOut / g_flacSampleRate;
}

static void FlacActivate()
{
    g_flacDec = g_lastFlacDec;
    if (g_flacDec) {
        g_flacDuration = g_flacDec->totalDuration;
        g_flacSampleRate = g_flacDec->sampleRate;
        g_flacSamplesOut = 0;
    }
    g_lastFlacDec = nullptr;
}

void PlayIndex(int idx)
{
    if (idx < 0 || idx >= (int)g_playlist.size()) return;
    StopAudio();
    g_currentIndex = idx;
    if (!g_browserActive) {
        SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
        SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)idx);
        SendMessage(hListBox, LB_SETCURSEL, idx, 0);
        SendMessage(hListBox, LB_SETTOPINDEX, max(0, idx - 3), 0);
    }

    // Create decode stream (DTS-WAV, FLAC via libFLAC, or normal BASS)
    HSTREAM dec = 0;
    g_lastDcaDec = nullptr;
    g_lastFlacDec = nullptr;
    if (IsDtsWav(g_playlist[idx].path))
        dec = CreateDtsStream(g_playlist[idx].path);
    if (dec && g_lastDcaDec) {
        DcaActivate();
    } else {
        g_lastDcaDec = nullptr;
    }
    if (!dec && IsFlacFile(g_playlist[idx].path))
        dec = CreateFlacStream(g_playlist[idx].path);
    if (dec && g_lastFlacDec) {
        FlacActivate();
    } else if (!g_flacDec) {
        g_lastFlacDec = nullptr;
    }
    if (!dec)
        dec = BASS_StreamCreateFile(FALSE, g_playlist[idx].path, 0, 0,
            BASS_UNICODE | BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE);
    if (!dec) {
        wchar_t msg[MAX_PATH + 80];
        swprintf_s(msg, L"Cannot open:\n%s\n\nBASS error: %d",
            g_playlist[idx].path, BASS_ErrorGetCode());
        MessageBox(g_hwnd, msg, L"Playback Error", MB_ICONERROR);
        return;
    }

    // Get format from decode stream to create matching master output
    BASS_CHANNELINFO ci = {};
    BASS_ChannelGetInfo(dec, &ci);

    g_decStream = dec;

    // Create master output stream with GaplessProc
    currentStream = BASS_StreamCreate(ci.freq, ci.chans, BASS_SAMPLE_FLOAT,
        GaplessProc, g_hwnd);
    if (!currentStream) {
        BASS_StreamFree(dec);
        g_decStream = 0;
        return;
    }
    g_masterFreq  = ci.freq;
    g_masterChans = ci.chans;

    // Apply normalization to decode stream
    if (g_normalize && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
        float peak = ScanPeak(g_playlist[g_currentIndex].path);
        float normScale = 1.0f / peak;
        if (normScale > 4.0f) normScale = 4.0f;
        BASS_ChannelSetAttribute(g_decStream, BASS_ATTRIB_VOL, normScale);
    }

    // User volume on master output
    BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
    BASS_ChannelSetSync(currentStream, BASS_SYNC_END, 0, EndSyncProc, g_hwnd);
    ApplyDSP();
    ApplyPitch();
    if (g_hwnd) LayoutControls(g_hwnd);
    if (hTimeTot) {
        wchar_t buf[32], tot[40];
        FormatTime(GetTrackLength(), buf, _countof(buf));
        swprintf_s(tot, L"/ %s", buf);
        SetWindowText(hTimeTot, tot);
    }
    BASS_ChannelPlay(currentStream, FALSE);
    SetTimer(g_hwnd, IDT_PLAYBACK, 16, NULL);
    SetTimer(g_hwnd, IDT_PEAK_METER, 50, NULL);

    UpdatePlayBtn();
    UpdateStatusBar();
    UpdateWindowTitle();
    UpdateThumbButtons();
    UpdateTimeDisplays();
    if (hSeekCanvas)   InvalidateRect(hSeekCanvas, NULL, FALSE);
    if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);

    // Pre-load next track as decode stream
    PreloadNext();
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
        SetTimer(g_hwnd, IDT_PLAYBACK, 16, NULL);
    }
    UpdatePlayBtn();
    UpdateStatusBar();
    UpdateThumbButtons();
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

// Compute the next track index (-1 if none)
static int ComputeNextIndex()
{
    if (g_playlist.empty() || g_currentIndex < 0) return -1;
    if (g_repeat) return g_currentIndex;
    if (g_shuffle) {
        int p = 0;
        for (int i = 0; i < (int)g_shuffleOrder.size(); ++i)
            if (g_shuffleOrder[i] == g_currentIndex) { p = i; break; }
        return g_shuffleOrder[(p + 1) % g_shuffleOrder.size()];
    }
    int next = g_currentIndex + 1;
    if (next >= (int)g_playlist.size()) {
        if (g_repeat) return 0;
        return -1; // end of playlist, no repeat
    }
    return next;
}

void PreloadNext()
{
    // Free any previously pre-loaded decode stream
    if (g_decNext) { BASS_StreamFree(g_decNext); g_decNext = 0; }
    g_decNextIdx = -1;
    int next = ComputeNextIndex();
    if (next < 0 || next >= (int)g_playlist.size()) return;

    // Create decode stream for next track
    HSTREAM dec = 0;
    g_lastDcaDec = nullptr;
    g_dcaDecNext = nullptr;
    g_lastFlacDec = nullptr;
    g_flacDecNext = nullptr;
    if (IsDtsWav(g_playlist[next].path))
        dec = CreateDtsStream(g_playlist[next].path);
    if (dec && g_lastDcaDec) {
        g_dcaDecNext = g_lastDcaDec;
        g_lastDcaDec = nullptr;
    }
    if (!dec && IsFlacFile(g_playlist[next].path))
        dec = CreateFlacStream(g_playlist[next].path);
    if (dec && g_lastFlacDec) {
        g_flacDecNext = g_lastFlacDec;
        g_lastFlacDec = nullptr;
    }
    if (!dec)
        dec = BASS_StreamCreateFile(FALSE, g_playlist[next].path, 0, 0,
            BASS_UNICODE | BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE);
    if (!dec) return;

    // Pre-apply normalization to decode stream
    if (g_normalize) {
        float peak = ScanPeak(g_playlist[next].path);
        float normScale = 1.0f / peak;
        if (normScale > 4.0f) normScale = 4.0f;
        BASS_ChannelSetAttribute(dec, BASS_ATTRIB_VOL, normScale);
    }

    g_decNext = dec;
    g_decNextIdx = next;
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
    // Reset scroll state so it re-evaluates on every status update
    g_stLastText[0] = 0;
    g_stScrollX = 0;
    g_stPause = 30;
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
                    else if (_wcsicmp(ext, L".flac") == 0) fmt = g_flacDec ? L"FLAC (libFLAC)" : L"FLAC";
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

            // Try to get encoder info
            wchar_t enc[128] = L"";
            if (g_flacDec && g_flacDec->encoder[0]) {
                wcsncpy_s(enc, g_flacDec->encoder, _TRUNCATE);
            } else {
                // Try BASS tags (OGG/FLAC vorbis comments, then MP3 ID3v2)
                HSTREAM tagSrc = g_decStream ? g_decStream : currentStream;
                const char* tags = BASS_ChannelGetTags(tagSrc, BASS_TAG_OGG);
                if (tags) {
                    for (const char* t = tags; *t; t += strlen(t) + 1) {
                        if (_strnicmp(t, "ENCODER=", 8) == 0) {
                            MultiByteToWideChar(CP_UTF8, 0, t + 8, -1, enc, 127);
                            break;
                        }
                    }
                }
                if (!enc[0]) {
                    // Try LAME/encoder tag from MP3
                    const char* lame = BASS_ChannelGetTags(tagSrc, BASS_TAG_MP4);
                    if (!lame) lame = BASS_ChannelGetTags(tagSrc, BASS_TAG_MUSIC_NAME);
                }
            }

            // For WAV: show bit depth
            if (wcscmp(fmt, L"WAV") == 0) {
                int bits = (int)info.origres;
                if (bits <= 0) bits = (info.flags & BASS_SAMPLE_FLOAT) ? 32 :
                    (info.flags & BASS_SAMPLE_8BITS) ? 8 : 16;
                if (enc[0])
                    swprintf_s(right, L"WAV %d-bit  %d Hz  %s  [%s]", bits, info.freq, ch, enc);
                else
                    swprintf_s(right, L"WAV %d-bit  %d Hz  %s", bits, info.freq, ch);
            }
            else if (brate > 0) {
                if (enc[0])
                    swprintf_s(right, L"%s %.0f kbps  %d Hz  %s  [%s]", fmt, brate, info.freq, ch, enc);
                else
                    swprintf_s(right, L"%s %.0f kbps  %d Hz  %s", fmt, brate, info.freq, ch);
            }
            else {
                if (enc[0])
                    swprintf_s(right, L"%s  %d Hz  %s  [%s]", fmt, info.freq, ch, enc);
                else
                    swprintf_s(right, L"%s  %d Hz  %s", fmt, info.freq, ch);
            }
        }
        if (BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PAUSED) {
            wchar_t tmp[256]; swprintf_s(tmp, L"%s  [Paused]", right);
            wcsncpy_s(right, tmp, _TRUNCATE);
        }
    }
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)left);
    SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)right);
    if (g_darkMode) InvalidateRect(hStatus, NULL, FALSE);  // force subclass repaint
    UpdateTrayTooltip();
}

// ============================================================
//  Custom seek bar
// ============================================================
static double GetPlaybackFrac()
{
    if (g_dcaDec && g_dcaDuration > 0)
        return DcaGetPosition() / g_dcaDuration;
    if (g_flacDec && g_flacDuration > 0)
        return FlacGetPosition() / g_flacDuration;
    HSTREAM s = g_decStream ? g_decStream : currentStream;
    if (!s) return 0.0;
    QWORD pos = BASS_ChannelGetPosition(s, BASS_POS_BYTE);
    QWORD len = BASS_ChannelGetLength(s, BASS_POS_BYTE);
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
            HPEN pen = CreatePen(PS_SOLID, 1, g_darkMode ? 0x404040 : 0xBBBBBB);
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

enum BtnType { BTN_PREV, BTN_PLAY, BTN_PAUSE, BTN_STOP, BTN_NEXT, BTN_SHUFFLE, BTN_REPEAT, BTN_MONO, BTN_NORMALIZE, BTN_BASSBOOST, BTN_MEDIA, BTN_DSP };

static void DrawBtn(DRAWITEMSTRUCT* dis, BtnType type)
{
    HDC  dc = dis->hDC; RECT rc = dis->rcItem;
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    bool toggled = (type == BTN_SHUFFLE && g_shuffle)
        || (type == BTN_REPEAT && g_repeat)
        || (type == BTN_MONO && g_mono)
        || (type == BTN_NORMALIZE && g_normalize)
        || (type == BTN_BASSBOOST && g_bassBoost)
        || (type == BTN_MEDIA && g_mediaActive)
        || (type == BTN_DSP && !g_dspBypass && (g_dspReverb || g_dspSaturate || g_dspVinyl || g_dspHifi))
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
    case BTN_DSP:
    case BTN_SHUFFLE:
    case BTN_REPEAT:
    case BTN_MEDIA: {
        sym = (pressed || toggled) ? 0xFFFFFF : g_theme.text;  // use theme color so dark mode works
        // Buttons keep a visible border
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
        case BTN_NORMALIZE:  lbl = L"Norm";    break;
        case BTN_BASSBOOST:  lbl = L"BassBoost"; break;
        case BTN_MEDIA:      lbl = g_mediaActive ? L"\u2605" : L"\u2606"; break;  // ★ / ☆
        case BTN_DSP:        lbl = L"DSP";     break;
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

    // Save scroll position so the view doesn't reset to top
    int topIdx = (int)SendMessage(hListBox, LB_GETTOPINDEX, 0, 0);

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

    // Rebuild listbox with star prefixes
    RefreshListboxStars();
    SendMessage(hListBox, LB_SETCURSEL, destIdx, 0);
    SendMessage(hListBox, LB_SETTOPINDEX, topIdx, 0);
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
            if (s != LB_ERR) {
                if (g_browserActive) BrowserNavigate(s);
                else PlayIndex(s);
            }
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
            // Save undo state (forward order before reverse-sort)
            g_undoRemove.clear();
            std::sort(toDelete.begin(), toDelete.end());
            for (int idx : toDelete)
                if (idx >= 0 && idx < (int)g_playlist.size())
                    g_undoRemove.push_back({g_playlist[idx], idx});
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
        if (wParam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            // Undo last remove — re-insert tracks in forward order
            if (!g_undoRemove.empty()) {
                for (auto& u : g_undoRemove) {
                    int ins = min(u.idx, (int)g_playlist.size());
                    g_playlist.insert(g_playlist.begin() + ins, u.t);
                    SendMessage(hwnd, LB_INSERTSTRING, ins, (LPARAM)u.t.display);
                }
                g_undoRemove.clear();
                RebuildShuffleOrder();
                UpdateStatusBar();
            }
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
                // Plain click (no drag)
                SendMessage(hwnd, LB_SETCURSEL, from, 0);
                // In browser mode, single-click navigates into folders
                if (g_browserActive && from >= 0 && from < (int)g_browserItems.size()
                    && g_browserItems[from].isDir) {
                    BrowserNavigate(from);
                }
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
        if (idx == LB_ERR) return 0;

        if (g_browserActive) {
            // Browser mode: work against g_browserItems, skip dir items
            if (idx >= (int)g_browserItems.size() || g_browserItems[idx].isDir) return 0;
            SendMessage(hwnd, LB_SETCURSEL, idx, 0);
            g_ctxIsBrowser = true;
            g_ctxTrackIndex = idx;
            wcsncpy_s(g_ctxFilePath, g_browserItems[idx].path, _TRUNCATE);
            POINT screen = pt; ClientToScreen(hwnd, &screen);
            // Browser context: Play, Audio Info, Open File Location (no Remove)
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, IDC_CTX_PLAY,        L"Play");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, IDC_CTX_AUDIOINFO,    L"Audio Information...");
            AppendMenu(hMenu, MF_STRING, IDC_CTX_PROPERTIES,   L"Properties...");
            AppendMenu(hMenu, MF_STRING, IDC_CTX_OPENLOCATION, L"Open File Location");
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, screen.x, screen.y, 0, g_hwnd, NULL);
            DestroyMenu(hMenu);
        } else {
            // Normal mode: work against g_playlist
            if (idx >= (int)g_playlist.size()) return 0;
            if (SendMessage(hwnd, LB_GETSEL, idx, 0) == 0) {
                SendMessage(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
                SendMessage(hwnd, LB_SETSEL, TRUE, idx);
            }
            SendMessage(hwnd, LB_SETCURSEL, idx, 0);
            g_ctxIsBrowser = false;
            wcsncpy_s(g_ctxFilePath, g_playlist[idx].path, _TRUNCATE);
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
    g_ctxIsBrowser  = false;
    wcsncpy_s(g_ctxFilePath, g_playlist[trackIdx].path, _TRUNCATE);

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, IDC_CTX_PLAY, L"Play");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    // Favorites
    if (IsFavorite(g_ctxFilePath))
        AppendMenu(hMenu, MF_STRING, IDC_CTX_FAV_REMOVE, L"Remove from Favorites");
    else
        AppendMenu(hMenu, MF_STRING, IDC_CTX_FAV_ADD, L"Add to Favorites");
    // Playlists submenu
    ScanPlaylists();
    if (!g_bppPlaylists.empty() || true) {
        HMENU hPlSub = CreatePopupMenu();
        for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++)
            AppendMenu(hPlSub, MF_STRING, IDC_CTX_PL_BASE + i, g_bppPlaylists[i].name.c_str());
        if (!g_bppPlaylists.empty())
            AppendMenu(hPlSub, MF_SEPARATOR, 0, NULL);
        AppendMenu(hPlSub, MF_STRING, IDC_CTX_PL_NEW, L"New Playlist...");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPlSub, L"Add to Playlist");
    }
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDC_CTX_AUDIOINFO,    L"Audio Information...");
    AppendMenu(hMenu, MF_STRING, IDC_CTX_PROPERTIES,   L"Properties...");
    AppendMenu(hMenu, MF_STRING, IDC_CTX_OPENLOCATION, L"Open File Location");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDC_CTX_REMOVE, L"Remove");

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
    int     trackIdx;               // playlist index (-1 when opened from browser)
    wchar_t path[MAX_PATH];         // file path (always valid)
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

// Write ID3v2.3 tags to an MP3 file (re-writes the header, preserves audio data)
static bool WriteID3v2ToMP3(const wchar_t* path,
    const wchar_t* title, const wchar_t* artist, const wchar_t* album,
    const wchar_t* year,  const wchar_t* track,  const wchar_t* genre)
{
    // Read file
    HANDLE hf = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER fs; GetFileSizeEx(hf, &fs);
    if (fs.QuadPart > 500LL * 1024 * 1024) { CloseHandle(hf); return false; } // 500 MB guard

    // Find start of audio data — skip existing ID3v2 header
    BYTE hdr[10]; DWORD rd;
    ReadFile(hf, hdr, 10, &rd, NULL);
    DWORD audioOffset = 0;
    if (rd == 10 && memcmp(hdr, "ID3", 3) == 0) {
        DWORD sz = ((DWORD)(hdr[6] & 0x7F) << 21) | ((DWORD)(hdr[7] & 0x7F) << 14) |
                   ((DWORD)(hdr[8] & 0x7F) << 7)  | (hdr[9] & 0x7F);
        audioOffset = 10 + sz;
    }
    DWORD audioSize = (audioOffset < (DWORD)fs.QuadPart) ? ((DWORD)fs.QuadPart - audioOffset) : 0;
    std::vector<BYTE> audio(audioSize);
    SetFilePointer(hf, audioOffset, NULL, FILE_BEGIN);
    ReadFile(hf, audio.data(), audioSize, &rd, NULL);
    CloseHandle(hf);

    // Build ID3v2.3 frames (UTF-8 encoding byte = 3)
    std::vector<BYTE> frames;
    struct TagEntry { const char* id; const wchar_t* val; };
    TagEntry tags[] = {
        {"TIT2", title}, {"TPE1", artist}, {"TALB", album},
        {"TYER", year},  {"TRCK", track},  {"TCON", genre}
    };
    for (auto& te : tags) {
        if (!te.val || !te.val[0]) continue;
        int utfN = WideCharToMultiByte(CP_UTF8, 0, te.val, -1, NULL, 0, NULL, NULL);
        if (utfN <= 1) continue;
        std::vector<char> utf(utfN);
        WideCharToMultiByte(CP_UTF8, 0, te.val, -1, utf.data(), utfN, NULL, NULL);
        DWORD fds = 1 + (DWORD)(utfN - 1); // enc byte + string (no null term)
        for (int c = 0; c < 4; c++) frames.push_back((BYTE)te.id[c]);
        frames.push_back((BYTE)((fds >> 24) & 0xFF));
        frames.push_back((BYTE)((fds >> 16) & 0xFF));
        frames.push_back((BYTE)((fds >> 8)  & 0xFF));
        frames.push_back((BYTE)(fds & 0xFF));
        frames.push_back(0); frames.push_back(0); // flags
        frames.push_back(3); // UTF-8
        for (int i = 0; i < utfN - 1; i++) frames.push_back((BYTE)utf[i]);
    }

    // 256-byte padding
    DWORD padSz = 256;
    DWORD bodySize = (DWORD)frames.size() + padSz;
    BYTE ss[4] = { (BYTE)((bodySize >> 21) & 0x7F), (BYTE)((bodySize >> 14) & 0x7F),
                   (BYTE)((bodySize >> 7)  & 0x7F), (BYTE)(bodySize & 0x7F) };

    hf = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    DWORD wr;
    BYTE id3hdr[10] = {'I','D','3', 3, 0, 0, ss[0], ss[1], ss[2], ss[3]};
    WriteFile(hf, id3hdr, 10, &wr, NULL);
    if (!frames.empty()) WriteFile(hf, frames.data(), (DWORD)frames.size(), &wr, NULL);
    std::vector<BYTE> pad(padSz, 0);
    WriteFile(hf, pad.data(), padSz, &wr, NULL);
    if (!audio.empty()) WriteFile(hf, audio.data(), audioSize, &wr, NULL);
    CloseHandle(hf);
    return true;
}

static LRESULT CALLBACK InfoWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hArt = NULL;
    static int     s_trackIdx = -1;
    static wchar_t s_filePath[MAX_PATH] = {};
    static bool    s_artDragging = false;

    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL d2 = TRUE; DwmSetWindowAttribute(hwnd, 20, &d2, sizeof(d2)); DwmSetWindowAttribute(hwnd, 19, &d2, sizeof(d2)); }
        AudioInfoData* d = (AudioInfoData*)((CREATESTRUCT*)lParam)->lpCreateParams;
        s_trackIdx = d->trackIdx;
        wcsncpy_s(s_filePath, d->path, _TRUNCATE);
        FreeArtBytes();

        HSTREAM s = BASS_StreamCreateFile(FALSE, s_filePath, 0, 0,
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
            GetFileAttributesEx(s_filePath, GetFileExInfoStandard, &fa);
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
        HWND hPath = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", s_filePath,
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

    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
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
            if (!s_filePath[0]) break;
            wchar_t title[256], artist[256], album[256], year[16], track[16], genre[128];
            GetDlgItemText(hwnd, ID_INFO_TITLE,  title,  256);
            GetDlgItemText(hwnd, ID_INFO_ARTIST, artist, 256);
            GetDlgItemText(hwnd, ID_INFO_ALBUM,  album,  256);
            GetDlgItemText(hwnd, ID_INFO_YEAR,   year,   16);
            GetDlgItemText(hwnd, ID_INFO_TRACK,  track,  16);
            GetDlgItemText(hwnd, ID_INFO_GENRE,  genre,  128);
            const wchar_t* ext = wcsrchr(s_filePath, L'.');
            if (ext && _wcsicmp(ext, L".mp3") == 0) {
                // Stop playback if this file is currently open (can't write while open)
                bool wasPlaying = (s_trackIdx >= 0 && g_currentIndex == s_trackIdx && currentStream != 0);
                if (wasPlaying) StopAudio();
                if (WriteID3v2ToMP3(s_filePath, title, artist, album, year, track, genre)) {
                    MessageBox(hwnd, L"Tags saved successfully.", L"Save Tags", MB_ICONINFORMATION);
                    if (wasPlaying) PlayIndex(s_trackIdx);
                } else {
                    MessageBox(hwnd, L"Failed to write tags.\nMake sure the file is not read-only.", L"Save Tags", MB_ICONERROR);
                    if (wasPlaying) PlayIndex(s_trackIdx);
                }
            } else {
                MessageBox(hwnd, L"Tag writing is supported for MP3 files only.\nFLAC/OGG require an external tool.", L"Save Tags", MB_ICONWARNING);
            }
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

static void OpenAudioInfoForData(AudioInfoData& d)
{
    if (g_hwndInfo && IsWindow(g_hwndInfo)) {
        SetForegroundWindow(g_hwndInfo); return;
    }

    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);

    static const wchar_t INFO_CLASS[] = L"BillyInfoWnd";
    static bool registered = false;
    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = InfoWndProc; wc.hInstance = hInst;
        wc.lpszClassName = INFO_CLASS;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
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

void OpenAudioInfoDialog(int trackIdx)
{
    if (trackIdx < 0 || trackIdx >= (int)g_playlist.size()) return;
    static AudioInfoData d;
    d.trackIdx = trackIdx;
    wcsncpy_s(d.path, g_playlist[trackIdx].path, _TRUNCATE);
    OpenAudioInfoForData(d);
}

void OpenAudioInfoForPath(const wchar_t* path)
{
    if (!path || !path[0]) return;
    static AudioInfoData d;
    d.trackIdx = -1;
    wcsncpy_s(d.path, path, _TRUNCATE);
    OpenAudioInfoForData(d);
}

// ============================================================
//  Convert Dialog
//  Bulk audio file converter with format/quality options
// ============================================================

struct ConvItem { wchar_t path[MAX_PATH]; wchar_t display[MAX_PATH]; };
static std::vector<ConvItem> g_convItems;
static wchar_t g_convOutDir[MAX_PATH] = L"";

// ---- Conversion thread data ----
struct ConvThreadData {
    HWND                  hwnd;
    std::vector<ConvItem> items;
    wchar_t               outDir[MAX_PATH];
    int                   fmt;           // 0=mp3,1=wav,2=flac,3=ogg,4=m4a
    wchar_t               qualArgs[64];  // ffmpeg quality args, e.g. "-b:a 320k"
    int                   wavBitDepth;   // WAV only: 16, 24, or 32
    int                   outSampleRate; // 0=original, else target Hz (44100, 48000, etc.)
    bool                  normalize;
    bool                  copyMeta;
};

// Decode src to a 16-bit PCM WAV file via BASS (no external encoder needed)
static bool ConvWriteWav(const wchar_t* src, const wchar_t* dst, bool normalize, int bitDepth = 16)
{
    if (bitDepth != 16 && bitDepth != 24 && bitDepth != 32) bitDepth = 16;
    HSTREAM stream = BASS_StreamCreateFile(FALSE, src, 0, 0,
        BASS_STREAM_DECODE | BASS_UNICODE | BASS_SAMPLE_FLOAT);
    if (!stream) return false;

    BASS_CHANNELINFO info = {};
    BASS_ChannelGetInfo(stream, &info);

    WORD  channels      = (WORD)info.chans;
    DWORD sampleRate    = info.freq;
    WORD  bitsPerSample = (WORD)bitDepth;
    DWORD byteRate      = sampleRate * channels * (bitsPerSample / 8);
    WORD  blockAlign    = channels * (bitsPerSample / 8);
    WORD  fmtTag        = (bitDepth == 32) ? 3 : 1; // 3=IEEE_FLOAT, 1=PCM

    // Buffer: 32768 bytes of float data at a time (safe: no overlap with BASS_DATA_FLOAT=0x40000000)
    static const DWORD FBUF_BYTES = 32768;
    float fbuf[FBUF_BYTES / sizeof(float)];

    // Optional first pass: find peak level for normalization
    float gainFactor = 1.0f;
    if (normalize) {
        float peak = 0.0f;
        DWORD read;
        while ((read = BASS_ChannelGetData(stream, fbuf, FBUF_BYTES | BASS_DATA_FLOAT)) != (DWORD)-1 && read > 0) {
            int count = (int)(read / sizeof(float));
            for (int i = 0; i < count; i++) {
                float s = fabsf(fbuf[i]);
                if (s > peak) peak = s;
            }
        }
        if (peak > 0.001f) gainFactor = 0.98f / peak;
        // Reopen for the writing pass
        BASS_StreamFree(stream);
        stream = BASS_StreamCreateFile(FALSE, src, 0, 0,
            BASS_STREAM_DECODE | BASS_UNICODE | BASS_SAMPLE_FLOAT);
        if (!stream) return false;
    }

    FILE* f = nullptr;
    _wfopen_s(&f, dst, L"wb");
    if (!f) { BASS_StreamFree(stream); return false; }

    // Write placeholder WAV header (sizes filled in after encoding)
    fwrite("RIFF", 1, 4, f);
    DWORD riffSize = 0;          fwrite(&riffSize, 4, 1, f);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    DWORD fmtSize = 16;          fwrite(&fmtSize,  4, 1, f);
                                 fwrite(&fmtTag,    2, 1, f);
    fwrite(&channels,      2, 1, f);
    fwrite(&sampleRate,    4, 1, f);
    fwrite(&byteRate,      4, 1, f);
    fwrite(&blockAlign,    2, 1, f);
    fwrite(&bitsPerSample, 2, 1, f);
    fwrite("data", 1, 4, f);
    DWORD dataSize = 0;
    long  dataSizePos = ftell(f); fwrite(&dataSize, 4, 1, f);

    // Second (or only) pass: decode and write samples at requested bit depth
    DWORD read;
    DWORD totalData = 0;
    while ((read = BASS_ChannelGetData(stream, fbuf, FBUF_BYTES | BASS_DATA_FLOAT)) != (DWORD)-1 && read > 0) {
        int count = (int)(read / sizeof(float));
        if (bitDepth == 32) {
            // 32-bit IEEE float — write float buffer directly (apply gain)
            for (int i = 0; i < count; i++) {
                fbuf[i] *= gainFactor;
                if (fbuf[i] >  1.0f) fbuf[i] =  1.0f;
                if (fbuf[i] < -1.0f) fbuf[i] = -1.0f;
            }
            DWORD written = (DWORD)fwrite(fbuf, sizeof(float), count, f) * sizeof(float);
            totalData += written;
        } else if (bitDepth == 24) {
            // 24-bit PCM — 3 bytes per sample
            for (int i = 0; i < count; i++) {
                float s = fbuf[i] * gainFactor;
                if (s >  1.0f) s =  1.0f;
                if (s < -1.0f) s = -1.0f;
                int32_t v = (int32_t)(s * 8388607.0f);
                uint8_t b[3] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)((v >> 16) & 0xFF) };
                fwrite(b, 1, 3, f);
                totalData += 3;
            }
        } else {
            // 16-bit PCM
            short sbuf[FBUF_BYTES / sizeof(float)];
            for (int i = 0; i < count; i++) {
                float s = fbuf[i] * gainFactor;
                if (s >  1.0f) s =  1.0f;
                if (s < -1.0f) s = -1.0f;
                sbuf[i] = (short)(s * 32767.0f);
            }
            DWORD written = (DWORD)fwrite(sbuf, 2, count, f) * 2;
            totalData += written;
        }
    }

    // Patch RIFF and data chunk sizes
    fseek(f, 4, SEEK_SET);
    DWORD riffFinal = 36 + totalData; fwrite(&riffFinal, 4, 1, f);
    fseek(f, dataSizePos, SEEK_SET);  fwrite(&totalData, 4, 1, f);

    fclose(f);
    BASS_StreamFree(stream);
    return (totalData > 0);
}

// Search for ffmpeg.exe next to the exe, then in PATH
static bool ConvFindFfmpeg(wchar_t* outPath, int maxLen)
{
    wchar_t exeDir[MAX_PATH];
    GetModuleFileNameW(NULL, exeDir, MAX_PATH);
    wchar_t* slash = wcsrchr(exeDir, L'\\');
    if (slash) { slash[1] = 0; swprintf_s(outPath, maxLen, L"%sffmpeg.exe", exeDir); }
    else        { wcsncpy_s(outPath, maxLen, L"ffmpeg.exe", _TRUNCATE); }
    if (GetFileAttributesW(outPath) != INVALID_FILE_ATTRIBUTES) return true;

    wchar_t found[MAX_PATH];
    if (SearchPathW(NULL, L"ffmpeg", L".exe", MAX_PATH, found, NULL)) {
        wcsncpy_s(outPath, maxLen, found, _TRUNCATE);
        return true;
    }
    return false;
}

// Run ffmpeg to encode src->dst with the given bitrate (and optional loudnorm)
static bool ConvRunFfmpeg(const wchar_t* ffmpeg, const wchar_t* src, const wchar_t* dst,
                           const wchar_t* qualArgs, bool normalize)
{
    wchar_t cmd[MAX_PATH * 3 + 256];
    if (normalize)
        swprintf_s(cmd, ARRAYSIZE(cmd),
            L"\"%s\" -y -i \"%s\" -af loudnorm %s \"%s\"",
            ffmpeg, src, qualArgs, dst);
    else
        swprintf_s(cmd, ARRAYSIZE(cmd),
            L"\"%s\" -y -i \"%s\" %s \"%s\"",
            ffmpeg, src, qualArgs, dst);

    STARTUPINFOW si = {}; si.cb = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return false;
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (exitCode == 0);
}

// Background conversion thread
static DWORD WINAPI ConvThreadProc(LPVOID param)
{
    ConvThreadData* d = (ConvThreadData*)param;
    int total = (int)d->items.size();
    int ok    = 0;

    const wchar_t* exts[] = { L"mp3", L"wav", L"flac", L"ogg", L"m4a" };
    const wchar_t* ext    = exts[d->fmt];
    bool needFfmpeg = (d->fmt != 1); // WAV=1 is handled natively

    wchar_t ffmpegPath[MAX_PATH] = L"";
    bool hasFfmpeg = needFfmpeg ? ConvFindFfmpeg(ffmpegPath, MAX_PATH) : false;

    for (int i = 0; i < total; i++) {
        if (g_convAbort) break;

        // Post progress (percent = files completed so far)
        int pct = (i * 100) / total;
        PostMessage(d->hwnd, WM_CONV_PROGRESS, (WPARAM)pct,
                    (LPARAM)MAKELPARAM((WORD)(i + 1), (WORD)total));

        // Build output path: strip extension, replace with target ext
        const wchar_t* fn = Filename(d->items[i].path);
        wchar_t base[MAX_PATH]; wcsncpy_s(base, fn, _TRUNCATE);
        wchar_t* dot = wcsrchr(base, L'.'); if (dot) *dot = 0;

        wchar_t outPath[MAX_PATH];
        swprintf_s(outPath, ARRAYSIZE(outPath), L"%s\\%s.%s", d->outDir, base, ext);

        bool success = false;
        if (d->fmt == 1 && d->outSampleRate == 0) {
            // WAV native (no resample needed)
            success = ConvWriteWav(d->items[i].path, outPath, d->normalize, d->wavBitDepth);
        } else if (d->fmt == 1 && d->outSampleRate > 0 && hasFfmpeg) {
            // WAV with resample — use ffmpeg
            wchar_t wavArgs[64];
            const wchar_t* pcmFmt = (d->wavBitDepth == 32) ? L"f32le" :
                                    (d->wavBitDepth == 24) ? L"s24le" : L"s16le";
            swprintf_s(wavArgs, L"-ar %d -c:a pcm_%s", d->outSampleRate, pcmFmt);
            success = ConvRunFfmpeg(ffmpegPath, d->items[i].path, outPath, wavArgs, d->normalize);
        } else if (d->fmt == 1) {
            // WAV no ffmpeg — native without resample
            success = ConvWriteWav(d->items[i].path, outPath, d->normalize, d->wavBitDepth);
        } else if (hasFfmpeg) {
            success = ConvRunFfmpeg(ffmpegPath, d->items[i].path, outPath, d->qualArgs, d->normalize);
        } else {
            // No ffmpeg — fall back to WAV
            wchar_t wavOut[MAX_PATH];
            swprintf_s(wavOut, ARRAYSIZE(wavOut), L"%s\\%s.wav", d->outDir, base);
            success = ConvWriteWav(d->items[i].path, wavOut, d->normalize, 16);
        }
        if (success) ok++;
    }

    PostMessage(d->hwnd, WM_CONV_DONE, (WPARAM)ok, (LPARAM)total);
    delete d;
    return 0;
}

// Repopulate the Quality combobox based on selected output format
static void UpdateConvQuality(HWND hwndConv, int fmt)
{
    HWND hQ = GetDlgItem(hwndConv, ID_CONV_QUALITY);
    if (!hQ) return;
    SendMessage(hQ, CB_RESETCONTENT, 0, 0);
    switch (fmt) {
    case 0: // MP3
    case 4: // AAC/M4A — same kbps options
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"320 kbps (Best)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"256 kbps (High)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"192 kbps (Good)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"128 kbps (Standard)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"96 kbps (Compact)");
        break;
    case 1: // WAV
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"16-bit PCM (Standard)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"24-bit PCM (High Quality)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"32-bit Float (Lossless)");
        break;
    case 2: // FLAC
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Level 8 - Best compression");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Level 5 - Default");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Level 0 - Fastest");
        break;
    case 3: // OGG Vorbis
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Q10 (Best)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Q8 (High)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Q6 (Good)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Q4 (Standard)");
        SendMessage(hQ, CB_ADDSTRING, 0, (LPARAM)L"Q2 (Compact)");
        break;
    }
    SendMessage(hQ, CB_SETCURSEL, 0, 0);
}

static LRESULT CALLBACK ConvertWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
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
        UpdateConvQuality(hwnd, 0); // populate quality options for MP3 (default)

        HWND hLbl5 = CreateWindow(L"STATIC", L"Sample Rate:",
            WS_CHILD | WS_VISIBLE, 510, fo + 3, 80, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl5, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hSR = CreateWindow(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 594, fo, 96, 150, hwnd, (HMENU)ID_CONV_SRATE, hInst, NULL);
        SendMessage(hSR, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hSR, CB_ADDSTRING, 0, (LPARAM)L"Original");
        SendMessage(hSR, CB_ADDSTRING, 0, (LPARAM)L"44.1 kHz");
        SendMessage(hSR, CB_ADDSTRING, 0, (LPARAM)L"48.0 kHz");
        SendMessage(hSR, CB_ADDSTRING, 0, (LPARAM)L"88.2 kHz");
        SendMessage(hSR, CB_ADDSTRING, 0, (LPARAM)L"96.0 kHz");
        SendMessage(hSR, CB_SETCURSEL, 0, 0);

        int optY = fo + 28;
        HWND hNorm = CreateWindow(L"BUTTON", L"Normalize audio levels",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 8, optY, 150, 18, hwnd, (HMENU)ID_CONV_NORMALIZE, hInst, NULL);
        SendMessage(hNorm, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hMeta = CreateWindow(L"BUTTON", L"Copy metadata tags",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 170, optY, 150, 18, hwnd, (HMENU)ID_CONV_METADATA, hInst, NULL);
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

        DragAcceptFiles(hwnd, TRUE);  // enable drag & drop onto the convert window
        break;
    }
    case WM_DROPFILES: {
        HDROP hd = (HDROP)wParam;
        UINT n = DragQueryFile(hd, 0xFFFFFFFF, NULL, 0);
        HWND hList = GetDlgItem(hwnd, ID_CONV_LIST);
        int added = 0;
        for (UINT i = 0; i < n; i++) {
            wchar_t p[MAX_PATH]; DragQueryFile(hd, i, p, MAX_PATH);
            DWORD attr = GetFileAttributes(p);
            if (attr == INVALID_FILE_ATTRIBUTES) continue;
            if (attr & FILE_ATTRIBUTE_DIRECTORY) {
                // Add all audio files in the dropped folder
                wchar_t pat[MAX_PATH]; swprintf_s(pat, L"%s\\*.*", p);
                WIN32_FIND_DATA fd; HANDLE hf = FindFirstFile(pat, &fd);
                if (hf != INVALID_HANDLE_VALUE) {
                    do {
                        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                            wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", p, fd.cFileName);
                            if (IsAudio(full)) {
                                bool dup = false;
                                for (auto& ci : g_convItems) if (_wcsicmp(ci.path, full) == 0) { dup = true; break; }
                                if (!dup) {
                                    ConvItem ci; wcsncpy_s(ci.path, full, _TRUNCATE);
                                    wcsncpy_s(ci.display, Filename(full), _TRUNCATE);
                                    g_convItems.push_back(ci);
                                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ci.display);
                                    added++;
                                }
                            }
                        }
                    } while (FindNextFile(hf, &fd));
                    FindClose(hf);
                }
            } else if (IsAudio(p)) {
                bool dup = false;
                for (auto& ci : g_convItems) if (_wcsicmp(ci.path, p) == 0) { dup = true; break; }
                if (!dup) {
                    ConvItem ci; wcsncpy_s(ci.path, p, _TRUNCATE);
                    wcsncpy_s(ci.display, Filename(p), _TRUNCATE);
                    g_convItems.push_back(ci);
                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ci.display);
                    added++;
                }
            }
        }
        DragFinish(hd);
        wchar_t sb2[128]; swprintf_s(sb2, L"Added %d file(s) via drag & drop. Total: %d.", added, (int)g_convItems.size());
        SetDlgItemText(hwnd, ID_CONV_STATUS, sb2);
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_CONV_FORMAT && HIWORD(wParam) == CBN_SELCHANGE) {
            int fmt = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            if (fmt < 0) fmt = 0;
            UpdateConvQuality(hwnd, fmt);
            break;
        }
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
            if (g_convRunning) { MessageBox(hwnd, L"Conversion already in progress.", L"Convert", MB_ICONWARNING); break; }
            if (g_convItems.empty()) { MessageBox(hwnd, L"No files to convert.", L"Convert", MB_ICONWARNING); break; }
            if (!g_convOutDir[0]) { MessageBox(hwnd, L"Please select an output folder.", L"Convert", MB_ICONWARNING); break; }

            int fmt  = (int)SendMessage(GetDlgItem(hwnd, ID_CONV_FORMAT),  CB_GETCURSEL, 0, 0);
            int qual = (int)SendMessage(GetDlgItem(hwnd, ID_CONV_QUALITY), CB_GETCURSEL, 0, 0);
            int srSel = (int)SendMessage(GetDlgItem(hwnd, ID_CONV_SRATE), CB_GETCURSEL, 0, 0);
            if (fmt  < 0 || fmt  >= 5) fmt  = 0;
            if (qual < 0) qual = 0;
            // 0=Original, 1=44100, 2=48000, 3=88200, 4=96000
            const int srOpts[] = { 0, 44100, 48000, 88200, 96000 };
            int outSR = (srSel >= 0 && srSel < 5) ? srOpts[srSel] : 0;
            bool normalize = (SendMessage(GetDlgItem(hwnd, ID_CONV_NORMALIZE), BM_GETCHECK, 0, 0) == BST_CHECKED);
            bool copyMeta  = (SendMessage(GetDlgItem(hwnd, ID_CONV_METADATA),  BM_GETCHECK, 0, 0) == BST_CHECKED);

            // Warn if a compressed format was chosen but ffmpeg is not available
            if (fmt != 1) {
                wchar_t ffPath[MAX_PATH];
                if (!ConvFindFfmpeg(ffPath, MAX_PATH)) {
                    const wchar_t* fmtLabels[] = { L"MP3", L"WAV", L"FLAC", L"OGG", L"AAC/M4A" };
                    wchar_t msg[512];
                    swprintf_s(msg, ARRAYSIZE(msg),
                        L"FFmpeg was not found on this system.\n\n"
                        L"%s encoding requires FFmpeg (ffmpeg.exe).\n\n"
                        L"Files will be converted to WAV (lossless PCM) instead.\n\n"
                        L"To enable %s output, place ffmpeg.exe in the same folder as BillyPro.exe "
                        L"or add it to your system PATH.\n\nContinue with WAV output?",
                        fmtLabels[fmt], fmtLabels[fmt]);
                    if (MessageBox(hwnd, msg, L"FFmpeg Not Found", MB_YESNO | MB_ICONWARNING) != IDYES)
                        break;
                }
            }

            // Package data for the worker thread (owns a copy of the item list)
            ConvThreadData* d = new ConvThreadData();
            d->hwnd        = hwnd;
            d->items       = g_convItems;
            wcsncpy_s(d->outDir, g_convOutDir, _TRUNCATE);
            d->fmt           = fmt;
            d->normalize     = normalize;
            d->copyMeta      = copyMeta;
            d->wavBitDepth   = 16;
            d->outSampleRate = outSR;
            d->qualArgs[0]   = L'\0';
            switch (fmt) {
            case 0: { // MP3
                const wchar_t* a[] = { L"-b:a 320k", L"-b:a 256k", L"-b:a 192k", L"-b:a 128k", L"-b:a 96k" };
                int q = min(qual, 4); wcsncpy_s(d->qualArgs, a[q], _TRUNCATE); break;
            }
            case 1: { // WAV
                const int bits[] = { 16, 24, 32 };
                int q = min(qual, 2); d->wavBitDepth = bits[q]; break;
            }
            case 2: { // FLAC
                const wchar_t* a[] = { L"-compression_level 8", L"-compression_level 5", L"-compression_level 0" };
                int q = min(qual, 2); wcsncpy_s(d->qualArgs, a[q], _TRUNCATE); break;
            }
            case 3: { // OGG Vorbis
                const wchar_t* a[] = { L"-q:a 10", L"-q:a 8", L"-q:a 6", L"-q:a 4", L"-q:a 2" };
                int q = min(qual, 4); wcsncpy_s(d->qualArgs, a[q], _TRUNCATE); break;
            }
            case 4: { // AAC/M4A
                const wchar_t* a[] = { L"-b:a 320k", L"-b:a 256k", L"-b:a 192k", L"-b:a 128k", L"-b:a 96k" };
                int q = min(qual, 4); wcsncpy_s(d->qualArgs, a[q], _TRUNCATE); break;
            }
            }
            // Append sample rate arg for ffmpeg (non-WAV formats)
            if (outSR > 0 && fmt != 1) {
                wchar_t arBuf[32];
                swprintf_s(arBuf, L" -ar %d", outSR);
                wcsncat_s(d->qualArgs, arBuf, _TRUNCATE);
            }

            g_convAbort      = false;
            g_convRunning    = true;
            g_convStartTick  = GetTickCount();
            EnableWindow(GetDlgItem(hwnd, ID_CONV_START), FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_CONV_CLOSE), FALSE);
            SendMessage(GetDlgItem(hwnd, ID_CONV_PROGRESS), PBM_SETPOS, 0, 0);
            SetDlgItemText(hwnd, ID_CONV_STATUS, L"Starting conversion...");

            if (g_convThread) { CloseHandle(g_convThread); g_convThread = NULL; }
            g_convThread = CreateThread(NULL, 0, ConvThreadProc, d, 0, NULL);
            if (!g_convThread) {
                g_convRunning = false;
                EnableWindow(GetDlgItem(hwnd, ID_CONV_START), TRUE);
                EnableWindow(GetDlgItem(hwnd, ID_CONV_CLOSE), TRUE);
                delete d;
                MessageBox(hwnd, L"Failed to start conversion thread.", L"Error", MB_ICONERROR);
            }
            break;
        }
        case ID_CONV_CLOSE:
            if (g_convRunning) {
                if (MessageBox(hwnd, L"A conversion is in progress. Abort and close?",
                               L"Convert", MB_YESNO | MB_ICONWARNING) != IDYES) break;
                g_convAbort = true;
            }
            DestroyWindow(hwnd);
            break;
        }
        break;

    case WM_CONV_PROGRESS: {
        int pct       = (int)wParam;
        int fileIdx   = (int)LOWORD(lParam);
        int fileTotal = (int)HIWORD(lParam);
        SendMessage(GetDlgItem(hwnd, ID_CONV_PROGRESS), PBM_SETPOS, pct, 0);
        DWORD elapsed = (GetTickCount() - g_convStartTick) / 1000;
        int eMin = (int)(elapsed / 60), eSec = (int)(elapsed % 60);
        const wchar_t* fn = (fileIdx >= 1 && fileIdx <= (int)g_convItems.size())
            ? Filename(g_convItems[fileIdx - 1].path) : L"";
        wchar_t sb[320];
        if (eMin > 0)
            swprintf_s(sb, ARRAYSIZE(sb), L"[%d:%02d] (%d/%d, %d%%) %s",
                       eMin, eSec, fileIdx, fileTotal, pct, fn);
        else
            swprintf_s(sb, ARRAYSIZE(sb), L"[%ds] (%d/%d, %d%%) %s",
                       eSec, fileIdx, fileTotal, pct, fn);
        SetDlgItemText(hwnd, ID_CONV_STATUS, sb);
        return 0;
    }
    case WM_CONV_DONE: {
        int ok    = (int)wParam;
        int total = (int)lParam;
        SendMessage(GetDlgItem(hwnd, ID_CONV_PROGRESS), PBM_SETPOS, 100, 0);
        wchar_t sb[128];
        if (g_convAbort)
            swprintf_s(sb, ARRAYSIZE(sb), L"Conversion cancelled. %d of %d file(s) completed.", ok, total);
        else
            swprintf_s(sb, ARRAYSIZE(sb), L"Done. %d of %d file(s) converted successfully.", ok, total);
        SetDlgItemText(hwnd, ID_CONV_STATUS, sb);
        EnableWindow(GetDlgItem(hwnd, ID_CONV_START), TRUE);
        EnableWindow(GetDlgItem(hwnd, ID_CONV_CLOSE), TRUE);
        if (g_convThread) { CloseHandle(g_convThread); g_convThread = NULL; }
        g_convRunning = false;
        return 0;
    }
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_KEYDOWN: if (wParam == VK_ESCAPE && !g_convRunning) DestroyWindow(hwnd); break;
    case WM_DESTROY:
        g_convAbort = true;
        if (g_convThread) {
            WaitForSingleObject(g_convThread, 8000);
            CloseHandle(g_convThread);
            g_convThread = NULL;
        }
        g_convRunning = false;
        g_hwndConvert = NULL;
        break;
    case WM_CLOSE:
        if (g_convRunning) {
            if (MessageBox(hwnd, L"A conversion is in progress. Abort and close?",
                           L"Convert", MB_YESNO | MB_ICONWARNING) != IDYES) return 0;
            g_convAbort = true;
        }
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//  File Associations dialog
// ============================================================
static const wchar_t* s_audioExts[] = {
    L".mp3", L".wav", L".flac", L".ogg", L".m4a",
    L".aac", L".wma", L".opus", L".ape", L".aiff", nullptr
};

static void RegisterFileAssoc(const wchar_t* ext)
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // ProgID: BillyPro.audio
    // HKCU\Software\Classes\BillyPro.audio\shell\open\command = "exePath" "%1"
    wchar_t cmd[MAX_PATH + 8];
    swprintf_s(cmd, L"\"%s\" \"%%1\"", exePath);
    HKEY hk;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\BillyPro.audio\\shell\\open\\command",
            0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) {
        RegSetValueExW(hk, NULL, 0, REG_SZ, (BYTE*)cmd, (DWORD)((wcslen(cmd)+1)*sizeof(wchar_t)));
        RegCloseKey(hk);
    }
    // Friendly name
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\BillyPro.audio",
            0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) {
        const wchar_t* desc = L"Billy Pro Audio File";
        RegSetValueExW(hk, NULL, 0, REG_SZ, (BYTE*)desc, (DWORD)((wcslen(desc)+1)*sizeof(wchar_t)));
        RegCloseKey(hk);
    }
    // Map extension: HKCU\Software\Classes\.ext = BillyPro.audio
    wchar_t keyPath[64]; swprintf_s(keyPath, L"Software\\Classes\\%s", ext);
    if (RegCreateKeyExW(HKEY_CURRENT_USER, keyPath,
            0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) {
        const wchar_t* prog = L"BillyPro.audio";
        RegSetValueExW(hk, NULL, 0, REG_SZ, (BYTE*)prog, (DWORD)((wcslen(prog)+1)*sizeof(wchar_t)));
        RegCloseKey(hk);
    }
}

static void UnregisterFileAssoc(const wchar_t* ext)
{
    wchar_t keyPath[64]; swprintf_s(keyPath, L"Software\\Classes\\%s", ext);
    HKEY hk;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hk) == ERROR_SUCCESS) {
        wchar_t cur[64] = L""; DWORD sz = sizeof(cur);
        RegQueryValueExW(hk, NULL, NULL, NULL, (BYTE*)cur, &sz);
        if (_wcsicmp(cur, L"BillyPro.audio") == 0)
            RegDeleteValueW(hk, NULL);
        RegCloseKey(hk);
    }
}

static bool IsExtAssociated(const wchar_t* ext)
{
    wchar_t keyPath[64]; swprintf_s(keyPath, L"Software\\Classes\\%s", ext);
    HKEY hk;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_QUERY_VALUE, &hk) != ERROR_SUCCESS)
        return false;
    wchar_t cur[64] = L""; DWORD sz = sizeof(cur);
    RegQueryValueExW(hk, NULL, NULL, NULL, (BYTE*)cur, &sz);
    RegCloseKey(hk);
    return (_wcsicmp(cur, L"BillyPro.audio") == 0);
}

static LRESULT CALLBACK AssocDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        RECT rc; GetClientRect(hwnd, &rc);
        int cw = rc.right;

        HWND hLbl = CreateWindow(L"STATIC",
            L"Select file types to always open with Billy Pro:",
            WS_CHILD | WS_VISIBLE, 8, 8, cw - 16, 18, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_HASSTRINGS |
            LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
            8, 30, cw - 16, 220, hwnd, (HMENU)ID_ASSOC_LIST, hInst, NULL);
        SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        for (int i = 0; s_audioExts[i]; i++) {
            int idx = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)s_audioExts[i]);
            SendMessage(hList, LB_SETITEMDATA, idx, (LPARAM)i);
            // Use LB_SETSEL to mark checked state — we store checked as ItemData bit
            // We'll use WM_DRAWITEM, but LBS_CHECKBOX is not a real Windows style.
            // Instead use simple LBS_MULTIPLESEL:
        }
        // Re-create as a checklist using owner-draw or just show checkmarks as text
        // Simplest: just rebuild with check status in the string
        SendMessage(hList, LB_RESETCONTENT, 0, 0);
        for (int i = 0; s_audioExts[i]; i++) {
            bool on = IsExtAssociated(s_audioExts[i]);
            wchar_t item[32]; swprintf_s(item, L"[%c]  %s", on ? L'X' : L' ', s_audioExts[i]);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)item);
        }

        int bx = 8, by = 258, bh = 26;
        HWND hAll  = CreateWindow(L"BUTTON", L"Select All",   WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, bx, by, 90, bh, hwnd, (HMENU)ID_ASSOC_SELALL, hInst, NULL);
        HWND hNone = CreateWindow(L"BUTTON", L"Select None",  WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, bx+94, by, 90, bh, hwnd, (HMENU)ID_ASSOC_NONE, hInst, NULL);
        HWND hApp  = CreateWindow(L"BUTTON", L"Apply",        WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON, cw-196, by, 88, bh, hwnd, (HMENU)ID_ASSOC_APPLY, hInst, NULL);
        HWND hClo  = CreateWindow(L"BUTTON", L"Close",        WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, cw-104, by, 88, bh, hwnd, (HMENU)ID_ASSOC_CLOSE, hInst, NULL);
        SendMessage(hAll,  WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hNone, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hApp,  WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        SendMessage(hClo,  WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_ASSOC_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                // Toggle the clicked item's [X][ ] marker
                HWND hList = GetDlgItem(hwnd, ID_ASSOC_LIST);
                int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel == LB_ERR) break;
                wchar_t buf[64]; SendMessage(hList, LB_GETTEXT, sel, (LPARAM)buf);
                bool wasOn = (buf[1] == L'X');
                buf[1] = wasOn ? L' ' : L'X';
                // Replace item in-place
                SendMessage(hList, LB_DELETESTRING, sel, 0);
                SendMessage(hList, LB_INSERTSTRING, sel, (LPARAM)buf);
                SendMessage(hList, LB_SETCURSEL, sel, 0);
            }
            break;
        case ID_ASSOC_SELALL: {
            HWND hList = GetDlgItem(hwnd, ID_ASSOC_LIST);
            int cnt = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
            for (int i = 0; i < cnt; i++) {
                wchar_t buf[64]; SendMessage(hList, LB_GETTEXT, i, (LPARAM)buf);
                buf[1] = L'X';
                SendMessage(hList, LB_DELETESTRING, i, 0);
                SendMessage(hList, LB_INSERTSTRING, i, (LPARAM)buf);
            }
            break;
        }
        case ID_ASSOC_NONE: {
            HWND hList = GetDlgItem(hwnd, ID_ASSOC_LIST);
            int cnt = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
            for (int i = 0; i < cnt; i++) {
                wchar_t buf[64]; SendMessage(hList, LB_GETTEXT, i, (LPARAM)buf);
                buf[1] = L' ';
                SendMessage(hList, LB_DELETESTRING, i, 0);
                SendMessage(hList, LB_INSERTSTRING, i, (LPARAM)buf);
            }
            break;
        }
        case ID_ASSOC_APPLY: {
            HWND hList = GetDlgItem(hwnd, ID_ASSOC_LIST);
            int cnt = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
            for (int i = 0; i < cnt && s_audioExts[i]; i++) {
                wchar_t buf[64]; SendMessage(hList, LB_GETTEXT, i, (LPARAM)buf);
                if (buf[1] == L'X') RegisterFileAssoc(s_audioExts[i]);
                else                UnregisterFileAssoc(s_audioExts[i]);
            }
            // Notify Explorer
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
            MessageBox(hwnd, L"File associations updated.\n\nAudio files will now open with Billy Pro when double-clicked.",
                L"File Associations", MB_ICONINFORMATION);
            break;
        }
        case ID_ASSOC_CLOSE: DestroyWindow(hwnd); break;
        }
        break;
    case WM_DESTROY: g_hwndAssoc = NULL; break;
    case WM_CLOSE:   DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenFileAssocDialog()
{
    if (g_hwndAssoc && IsWindow(g_hwndAssoc)) { SetForegroundWindow(g_hwndAssoc); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t ASSOC_CLASS[] = L"BillyAssocWnd";
    static bool assocReg = false;
    if (!assocReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = AssocDlgProc; wc.hInstance = hInst;
        wc.lpszClassName = ASSOC_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); assocReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 340, 294 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndAssoc = CreateWindowEx(WS_EX_APPWINDOW, ASSOC_CLASS,
        L"Billy Pro \u2014 File Associations", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndAssoc, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top  + (pr.bottom - pr.top  - dh) / 2,
        dw, dh, SWP_NOZORDER);
}

// ============================================================
//  Taskbar thumbnail toolbar (prev / play-pause / next)
// ============================================================
static HICON CreateThumbIcon(const wchar_t* symbol)
{
    // Draw a simple 20x20 icon with a Unicode symbol using GDI
    int sz = 20;
    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    ReleaseDC(NULL, hdcScreen);

    BITMAPINFO bi = {}; bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = sz; bi.bmiHeader.biHeight = -sz;
    bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = NULL;
    HBITMAP hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    HBITMAP hOld = (HBITMAP)SelectObject(hdc, hbm);

    // Fill transparent (alpha=0, black)
    memset(bits, 0, sz * sz * 4);

    // Draw symbol in white
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    HFONT hf = CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
    HFONT hOldF = (HFONT)SelectObject(hdc, hf);
    RECT rc = { 0, 0, sz, sz };
    DrawText(hdc, symbol, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldF); DeleteObject(hf);

    // Create mask (all-black = fully use color bitmap)
    HBITMAP hMask = CreateBitmap(sz, sz, 1, 1, NULL);
    ICONINFO ii = {}; ii.fIcon = TRUE; ii.hbmColor = hbm; ii.hbmMask = hMask;
    HICON hico = CreateIconIndirect(&ii);

    SelectObject(hdc, hOld);
    DeleteObject(hbm); DeleteObject(hMask); DeleteDC(hdc);
    return hico;
}

void UpdateThumbButtons()
{
    if (!g_pTaskbar || !g_hwnd) return;
    bool playing = (currentStream && BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING);

    THUMBBUTTON tb[3] = {};
    // Prev
    tb[0].dwMask  = THB_ICON | THB_TOOLTIP | THB_FLAGS;
    tb[0].iId     = THUMB_BTN_PREV;
    tb[0].hIcon   = g_thumbIcons[0];
    wcscpy_s(tb[0].szTip, L"Previous");
    tb[0].dwFlags = THBF_ENABLED;
    // Play/Pause — icon switches based on current play state
    tb[1].dwMask  = THB_ICON | THB_TOOLTIP | THB_FLAGS;
    tb[1].iId     = THUMB_BTN_PLAY;
    tb[1].hIcon   = playing ? g_thumbIcons[2] : g_thumbIcons[1]; // ⏸ if playing, ▶ if paused/stopped
    wcscpy_s(tb[1].szTip, playing ? L"Pause" : L"Play");
    tb[1].dwFlags = THBF_ENABLED;
    // Next
    tb[2].dwMask  = THB_ICON | THB_TOOLTIP | THB_FLAGS;
    tb[2].iId     = THUMB_BTN_NEXT;
    tb[2].hIcon   = g_thumbIcons[3];
    wcscpy_s(tb[2].szTip, L"Next");
    tb[2].dwFlags = THBF_ENABLED;

    g_pTaskbar->ThumbBarUpdateButtons(g_hwnd, 3, tb);
}

// ============================================================
//  Options dialog  (replaces the old ugly DLGTEMPLATE version)
//  Contains: Bass Boost settings, Multi-instance, Drop mode,
//  File Associations button, Dark Mode toggle
// ============================================================
// ─────────────────────────────────────────────────────────────────────────────
//  Options dialog — Billy-style sidebar layout
// ─────────────────────────────────────────────────────────────────────────────
#define ID_OPT_NAVLIST      612
#define ID_OPT_BB_ENABLE    601
#define ID_OPT_BB_LOW       602
#define ID_OPT_BB_HIGH      603
#define ID_OPT_BB_GAIN      604
#define ID_OPT_MULTIINST    605
#define ID_OPT_DROP_APPEND  606
#define ID_OPT_DROP_REPLACE 607
#define ID_OPT_FILEASSOC    608
#define ID_OPT_DARKMODE     609
#define ID_OPT_SAVE         610
#define ID_OPT_CANCEL       611
#define ID_OPT_FT_WAV       620
#define ID_OPT_FT_MP3       621
#define ID_OPT_FT_OGG       622
#define ID_OPT_FT_FLAC      623
#define ID_OPT_FT_M3U       624
#define ID_OPT_DEVICE_COMBO  626
#define ID_OPT_DEVICE_RESET  627
#define ID_OPT_REMEMBER_VOL  628
#define ID_OPT_MEDIA_PATH    628
#define ID_OPT_MEDIA_BROWSE  629
#define ID_OPT_MEDIA_LIST    633
#define ID_OPT_MEDIA_ADD     634
#define ID_OPT_MEDIA_REMOVE  635
#define ID_OPT_PITCH_ENABLE  630
#define ID_OPT_PITCH_VALUE   631
#define ID_OPT_RESET         632
#define ID_OPT_PITCH_SLIDER  636
#define ID_OPT_SEEK_STEP     637
#define ID_OPT_REVERB        640
#define ID_OPT_SATURATE      641
#define ID_OPT_VINYL         642
#define ID_OPT_HIFI          643
#define ID_OPT_REV_MIX       644
#define ID_OPT_REV_ROOM      645
#define ID_OPT_REV_WIDTH     652
#define ID_OPT_SAT_DRIVE     646
#define ID_OPT_SAT_LEVEL     647
#define ID_OPT_VIN_FREQ      648
#define ID_OPT_VIN_CRACK     649
#define ID_OPT_HFI_BASS      650
#define ID_OPT_HFI_WARM      651

static HWND s_optPanels[6] = {};
static int  s_optPage = 0;

static LRESULT CALLBACK OptPanelProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wp; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_HSCROLL:
    case WM_COMMAND:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORLISTBOX:
        return SendMessage(GetParent(hwnd), msg, wp, lp);
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

static void OptSwitchPage(int page)
{
    for (int i = 0; i < 6; i++)
        if (s_optPanels[i]) ShowWindow(s_optPanels[i], i == page ? SW_SHOW : SW_HIDE);
    s_optPage = page;
}

static LRESULT CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Layout constants (client area 490 x 380)
    static const int CL_W  = 490;
    static const int CL_H  = 548;
    static const int SB_W  = 130;   // sidebar width
    static const int GAP   = 8;
    static const int BTN_H = 28;
    static const int PNL_X = SB_W + GAP * 2;
    static const int PNL_Y = GAP;
    static const int PNL_W = CL_W - PNL_X - GAP;
    static const int PNL_H = CL_H - GAP * 2 - BTN_H - GAP;

    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        s_optPage = 0;
        for (auto& p : s_optPanels) p = NULL;

        // Register panel class once
        static bool panelReg = false;
        if (!panelReg) {
            WNDCLASS wcp = {};
            wcp.lpfnWndProc   = OptPanelProc;
            wcp.hInstance     = hInst;
            wcp.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wcp.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
            wcp.lpszClassName = L"BillyOptPanel";
            RegisterClass(&wcp);
            panelReg = true;
        }

        // ── Control creation helpers ─────────────────────────────────────
        auto lbl = [&](HWND par, int x, int y, int w, int h, const wchar_t* t) -> HWND {
            HWND hL = CreateWindow(L"STATIC", t, WS_CHILD|WS_VISIBLE|SS_LEFT,
                x, y, w, h, par, (HMENU)IDC_STATIC, hInst, NULL);
            SendMessage(hL, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            return hL;
        };
        auto chk = [&](HWND par, int x, int y, int w, int h, WORD id, const wchar_t* t, bool v) -> HWND {
            HWND hC = CreateWindow(L"BUTTON", t,
                WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|WS_TABSTOP,
                x, y, w, h, par, (HMENU)(UINT_PTR)id, hInst, NULL);
            SendMessage(hC, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            SendMessage(hC, BM_SETCHECK, v ? BST_CHECKED : BST_UNCHECKED, 0);
            return hC;
        };
        auto rad = [&](HWND par, int x, int y, int w, int h, WORD id, const wchar_t* t, bool v) -> HWND {
            HWND hR = CreateWindow(L"BUTTON", t,
                WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|WS_TABSTOP,
                x, y, w, h, par, (HMENU)(UINT_PTR)id, hInst, NULL);
            SendMessage(hR, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            SendMessage(hR, BM_SETCHECK, v ? BST_CHECKED : BST_UNCHECKED, 0);
            return hR;
        };
        auto edt = [&](HWND par, int x, int y, int w, int h, WORD id, const wchar_t* val) -> HWND {
            HWND hE = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", val,
                WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP,
                x, y, w, h, par, (HMENU)(UINT_PTR)id, hInst, NULL);
            SendMessage(hE, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            return hE;
        };
        auto grp = [&](HWND par, int x, int y, int w, int h, const wchar_t* t) -> HWND {
            HWND hG = CreateWindow(L"BUTTON", t, WS_CHILD|WS_VISIBLE|BS_GROUPBOX,
                x, y, w, h, par, (HMENU)IDC_STATIC, hInst, NULL);
            SendMessage(hG, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            return hG;
        };
        auto btn = [&](HWND par, int x, int y, int w, int h, WORD id, const wchar_t* t, bool def=false) -> HWND {
            HWND hB = CreateWindow(L"BUTTON", t,
                WS_CHILD|WS_VISIBLE|WS_TABSTOP|(def?BS_DEFPUSHBUTTON:BS_PUSHBUTTON),
                x, y, w, h, par, (HMENU)(UINT_PTR)id, hInst, NULL);
            SendMessage(hB, WM_SETFONT, (WPARAM)(def?g_fontBold:g_fontUI), TRUE);
            return hB;
        };
        auto mkPanel = [&](bool visible) -> HWND {
            return CreateWindow(L"BillyOptPanel", L"",
                WS_CHILD | (visible ? WS_VISIBLE : 0),
                PNL_X, PNL_Y, PNL_W, PNL_H, hwnd, NULL, hInst, NULL);
        };

        // ── Sidebar navigation listbox ───────────────────────────────────
        HWND hNav = CreateWindow(L"LISTBOX", NULL,
            WS_CHILD|WS_VISIBLE|WS_BORDER|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_TABSTOP,
            GAP, GAP, SB_W, PNL_H, hwnd, (HMENU)ID_OPT_NAVLIST, hInst, NULL);
        SendMessage(hNav, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Setup");
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Display");
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Audio");
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Device");
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Media");
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Timestretch");
        SendMessage(hNav, LB_SETCURSEL, 0, 0);

        // ── Bottom: Save / Cancel / Reset (always visible in main window) ─
        int btnY = CL_H - GAP - BTN_H;
        btn(hwnd, CL_W - GAP - 90 - GAP - 90,             btnY, 90, BTN_H, ID_OPT_SAVE,   L"Save",   true);
        btn(hwnd, CL_W - GAP - 90,                        btnY, 90, BTN_H, ID_OPT_CANCEL, L"Cancel", false);
        btn(hwnd, GAP,                                     btnY, 120, BTN_H, ID_OPT_RESET, L"Reset to Defaults", false);

        // ── Page 0: Setup ─────────────────────────────────────────────────
        {
            HWND p = mkPanel(true);
            s_optPanels[0] = p;
            int w = PNL_W - 4;

            grp(p, 0, 0, w, 108, L"Open Billy Pro with these file types");
            chk(p, 10, 20, 80, 20, ID_OPT_FT_WAV,  L"WAV",  false);
            chk(p, 95, 20, 80, 20, ID_OPT_FT_MP3,  L"MP3",  false);
            chk(p, 180, 20, 80, 20, ID_OPT_FT_OGG,  L"OGG",  false);
            chk(p, 10, 42, 80, 20, ID_OPT_FT_FLAC, L"FLAC", false);
            chk(p, 95, 42, 80, 20, ID_OPT_FT_M3U,  L"M3U",  false);
            btn(p, 10, 68, w - 20, 26, ID_OPT_FILEASSOC, L"Associate selected types with Billy Pro");

            grp(p, 0, 118, w, 82, L"General");
            chk(p, 10, 136, w - 20, 20, ID_OPT_MULTIINST,
                L"Allow multiple instances of Billy Pro", g_multiInst);
            lbl(p, 10, 162, 160, 16, L"Arrow key seek (seconds):");
            {
                wchar_t sbuf[16]; swprintf_s(sbuf, L"%.0f", g_seekStep);
                edt(p, 174, 160, 50, 20, ID_OPT_SEEK_STEP, sbuf);
            }

            grp(p, 0, 190, w, 68, L"Drag && Drop");
            rad(p, 10, 208, w - 20, 20, ID_OPT_DROP_APPEND,
                L"Add dropped files to current playlist", g_dropAppend);
            rad(p, 10, 228, w - 20, 20, ID_OPT_DROP_REPLACE,
                L"Replace playlist with dropped files", !g_dropAppend);
        }

        // ── Page 1: Display ───────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[1] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 52, L"Theme");
            chk(p, 10, 18, w - 20, 24, ID_OPT_DARKMODE, L"Dark Mode", g_darkMode);
        }

        // ── Page 2: Audio ─────────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[2] = p;
            int w = PNL_W - 4;
            wchar_t buf[32];
            grp(p, 0, 0, w, 130, L"Bass Boost");
            chk(p, 10, 18, w - 20, 22, ID_OPT_BB_ENABLE, L"Enable Bass Boost", g_bassBoost);
            lbl(p, 20, 46, 120, 16, L"Low Freq (Hz):");
            swprintf_s(buf, L"%.1f", g_bbFreqLow);
            edt(p, 155, 44, 90, 20, ID_OPT_BB_LOW, buf);
            lbl(p, 20, 70, 120, 16, L"High Freq (Hz):");
            swprintf_s(buf, L"%.1f", g_bbFreqHigh);
            edt(p, 155, 68, 90, 20, ID_OPT_BB_HIGH, buf);
            lbl(p, 20, 94, 120, 16, L"Gain (dB):");
            swprintf_s(buf, L"%.1f", g_bbGainDB);
            edt(p, 155, 92, 90, 20, ID_OPT_BB_GAIN, buf);

            // Reverb
            grp(p, 0, 138, w, 96, L"Reverb");
            chk(p, 10, 154, w - 20, 20, ID_OPT_REVERB, L"Enable Reverb (Room Ambience)", g_dspReverb);
            lbl(p, 10, 180, 44, 16, L"Mix %:");
            swprintf_s(buf, L"%.0f", g_revMix);   edt(p, 56,  178, 46, 20, ID_OPT_REV_MIX,   buf);
            lbl(p, 108, 180, 52, 16, L"Room %:");
            swprintf_s(buf, L"%.0f", g_revRoom);  edt(p, 162, 178, 46, 20, ID_OPT_REV_ROOM,  buf);
            lbl(p, 214, 180, 54, 16, L"Width %:");
            swprintf_s(buf, L"%.0f", g_revWidth); edt(p, 270, 178, 50, 20, ID_OPT_REV_WIDTH, buf);
            lbl(p, 20, 202, w - 30, 14, L"Width 0 = mono reverb, 100 = full stereo.");

            // Saturation
            grp(p, 0, 242, w, 70, L"Saturation");
            chk(p, 10, 258, w - 20, 20, ID_OPT_SATURATE, L"Enable Saturation (Tube Warmth)", g_dspSaturate);
            lbl(p, 20, 284, 44, 16, L"Drive:");
            swprintf_s(buf, L"%.1f", g_satDrive);  edt(p, 66, 282, 52, 20, ID_OPT_SAT_DRIVE, buf);
            lbl(p, 130, 284, 54, 16, L"Level %:");
            swprintf_s(buf, L"%.0f", g_satLevel);  edt(p, 186, 282, 52, 20, ID_OPT_SAT_LEVEL, buf);

            // Vinyl
            grp(p, 0, 320, w, 70, L"Vinyl Emulation");
            chk(p, 10, 336, w - 20, 20, ID_OPT_VINYL, L"Enable Vinyl (HF Roll-off + Crackles)", g_dspVinyl);
            lbl(p, 20, 362, 44, 16, L"LP Hz:");
            swprintf_s(buf, L"%.0f", g_vinLpFreq);  edt(p, 66, 360, 60, 20, ID_OPT_VIN_FREQ,  buf);
            lbl(p, 138, 362, 58, 16, L"Crackle %:");
            swprintf_s(buf, L"%.0f", g_vinCrackle); edt(p, 198, 360, 52, 20, ID_OPT_VIN_CRACK, buf);

            // HiFi Amplifier
            grp(p, 0, 398, w, 70, L"HiFi Amplifier");
            chk(p, 10, 414, w - 20, 20, ID_OPT_HIFI, L"Enable HiFi Amplifier (Bass Shelf + Warmth)", g_dspHifi);
            lbl(p, 20, 440, 52, 16, L"Bass dB:");
            swprintf_s(buf, L"%.1f", g_hfiBassDb); edt(p, 74, 438, 52, 20, ID_OPT_HFI_BASS, buf);
            lbl(p, 138, 440, 60, 16, L"Warmth %:");
            swprintf_s(buf, L"%.0f", g_hfiWarmth); edt(p, 200, 438, 52, 20, ID_OPT_HFI_WARM, buf);

        }

        // ── Page 3: Device ────────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[3] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 130, L"Audio Output");
            lbl(p, 10, 20, w - 20, 16, L"Billy output:");
            HWND hCombo = CreateWindow(L"COMBOBOX", L"",
                WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_HASSTRINGS,
                10, 38, w - 20, 200, p, (HMENU)ID_OPT_DEVICE_COMBO, hInst, NULL);
            SendMessage(hCombo, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            // Enumerate BASS devices
            BASS_DEVICEINFO di;
            int selIdx = 0, devCount = 0;
            DWORD curDev = BASS_GetDevice();
            for (int d = 1; BASS_GetDeviceInfo(d, &di); d++) {
                if (di.flags & BASS_DEVICE_ENABLED) {
                    wchar_t wname[256];
                    MultiByteToWideChar(CP_ACP, 0, di.name, -1, wname, 256);
                    int idx = (int)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)wname);
                    SendMessage(hCombo, CB_SETITEMDATA, idx, (LPARAM)d);
                    if ((DWORD)d == curDev) selIdx = devCount;
                    devCount++;
                }
            }
            if (devCount > 0) SendMessage(hCombo, CB_SETCURSEL, selIdx, 0);
            btn(p, 10, 68, w - 20, 26, ID_OPT_DEVICE_RESET, L"Reset sound driver mixer");

            chk(p, 10, 102, w - 20, 20, ID_OPT_REMEMBER_VOL, L"Remember volume level", g_rememberVolume);

            // Sound info
            lbl(p, 10, 128, w - 20, 16, L"Sound details:");
            wchar_t detail[128];
            DWORD bassVer = BASS_GetVersion();
            swprintf_s(detail, L"BASS v%d.%d.%d  |  Buffer: 3000 ms",
                HIBYTE(HIWORD(bassVer)), LOBYTE(HIWORD(bassVer)), HIBYTE(LOWORD(bassVer)));
            lbl(p, 10, 144, w - 20, 16, detail);
        }

        // ── Page 4: Media ─────────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[4] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 180, L"Media Folders");
            lbl(p, 10, 20, w - 20, 16, L"Folders shown in the \u2605 browser (add one or more):");
            // Listbox of configured folders
            HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_TABSTOP,
                10, 38, w - 20, 100, p, (HMENU)ID_OPT_MEDIA_LIST, hInst, NULL);
            SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            for (auto& f : g_mediaFolders)
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)f.c_str());
            btn(p, 10,        144, 90, 24, ID_OPT_MEDIA_ADD,    L"Add Folder...");
            btn(p, 106,       144, 90, 24, ID_OPT_MEDIA_REMOVE, L"Remove");
            lbl(p, 10, 168, w - 20, 14, L"Press the \u2605 button on the main toolbar to browse.");
        }

        // ── Page 5: Timestretch ───────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[5] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 150, L"Pitch / Speed (CDJ-style)");
            chk(p, 10, 20, w - 20, 20, ID_OPT_PITCH_ENABLE, L"Enable pitch shift (changes pitch + tempo)", g_pitchEnabled);

            // Slider: -120 to +120 in units of 0.1 semitone
            int sliderVal = (int)roundf(g_pitchSemitones * 10.0f);
            HWND hSlider = CreateWindow(TRACKBAR_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | WS_TABSTOP,
                10, 46, w - 80, 28, p, (HMENU)ID_OPT_PITCH_SLIDER, hInst, NULL);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(-120, 120));
            SendMessage(hSlider, TBM_SETTICFREQ, 10, 0);  // tick every 1 semitone
            SendMessage(hSlider, TBM_SETPAGESIZE, 0, 10); // PgUp/Dn = 1 semitone
            SendMessage(hSlider, TBM_SETPOS, TRUE, sliderVal);

            // Value edit: shows semitones with 1 decimal place
            wchar_t pbuf[16]; swprintf_s(pbuf, L"%.1f", g_pitchSemitones);
            edt(p, w - 66, 48, 60, 22, ID_OPT_PITCH_VALUE, pbuf);

            lbl(p, 10, 80, 40, 14, L"-12");
            lbl(p, w/2 - 4, 80, 20, 14, L"0");
            lbl(p, w - 72, 80, 24, 14, L"+12");
            lbl(p, 10, 100, w - 20, 32,
                L"Negative = lower pitch/slower.  Positive = higher pitch/faster.\n"
                L"Move slider or type a value (e.g. -8.4).");
            lbl(p, 10, 132, w - 20, 14, L"Time display updates to reflect changed duration.");
        }

        break;
    }

    case WM_COMMAND:
        // EN_CHANGE on pitch value edit — sync slider position
        if (LOWORD(wParam) == ID_OPT_PITCH_VALUE && HIWORD(wParam) == EN_CHANGE && s_optPanels[5]) {
            HWND hEdit   = GetDlgItem(s_optPanels[5], ID_OPT_PITCH_VALUE);
            HWND hSlider = GetDlgItem(s_optPanels[5], ID_OPT_PITCH_SLIDER);
            if (hEdit && hSlider) {
                wchar_t buf[16] = {}; GetWindowText(hEdit, buf, 16);
                float v = max(-12.0f, min(12.0f, (float)_wtof(buf)));
                int newPos = (int)roundf(v * 10.0f);
                if ((int)SendMessage(hSlider, TBM_GETPOS, 0, 0) != newPos)
                    SendMessage(hSlider, TBM_SETPOS, TRUE, newPos);
            }
        }
        switch (LOWORD(wParam)) {
        case ID_OPT_NAVLIST:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int sel = (int)SendMessage(GetDlgItem(hwnd, ID_OPT_NAVLIST), LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < 6) OptSwitchPage(sel);
            }
            break;

        case ID_OPT_MEDIA_ADD: {
            BROWSEINFO bi = {}; bi.hwndOwner = hwnd; bi.lpszTitle = L"Select Media Folder";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
            if (pidl) {
                wchar_t folder[MAX_PATH] = {};
                if (SHGetPathFromIDList(pidl, folder) && folder[0]) {
                    // Avoid duplicates
                    bool dup = false;
                    for (auto& f : g_mediaFolders) if (_wcsicmp(f.c_str(), folder) == 0) { dup = true; break; }
                    if (!dup) {
                        g_mediaFolders.push_back(folder);
                        // Keep g_mediaFolder = first entry
                        wcsncpy_s(g_mediaFolder, g_mediaFolders[0].c_str(), _TRUNCATE);
                        if (s_optPanels[4]) {
                            HWND hList = GetDlgItem(s_optPanels[4], ID_OPT_MEDIA_LIST);
                            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)folder);
                        }
                        LayoutControls(g_hwnd);
                    }
                }
                CoTaskMemFree(pidl);
            }
            break;
        }
        case ID_OPT_MEDIA_REMOVE: {
            if (!s_optPanels[4]) break;
            HWND hList = GetDlgItem(s_optPanels[4], ID_OPT_MEDIA_LIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
            if (sel == LB_ERR) break;
            if (sel >= 0 && sel < (int)g_mediaFolders.size()) {
                g_mediaFolders.erase(g_mediaFolders.begin() + sel);
                SendMessage(hList, LB_DELETESTRING, sel, 0);
                // Sync g_mediaFolder
                if (!g_mediaFolders.empty())
                    wcsncpy_s(g_mediaFolder, g_mediaFolders[0].c_str(), _TRUNCATE);
                else
                    g_mediaFolder[0] = L'\0';
                // If browser was active and now no folders, deactivate it
                if (g_mediaFolders.empty() && g_mediaActive) {
                    g_mediaActive = false;
                    g_browserActive = false;
                    g_browserItems.clear();
                }
                LayoutControls(g_hwnd);
            }
            break;
        }

        case ID_OPT_FILEASSOC: {
            // Register checked file types directly
            wchar_t exe[MAX_PATH];
            GetModuleFileName(NULL, exe, MAX_PATH);
            wchar_t cmd[MAX_PATH + 12];
            swprintf_s(cmd, L"\"%s\" \"%%1\"", exe);
            struct { WORD id; const wchar_t* ext; } types[] = {
                {ID_OPT_FT_WAV, L".wav"}, {ID_OPT_FT_MP3, L".mp3"},
                {ID_OPT_FT_OGG, L".ogg"}, {ID_OPT_FT_FLAC, L".flac"},
                {ID_OPT_FT_M3U, L".m3u"},
            };
            int count = 0;
            for (auto& t : types) {
                HWND hC = GetDlgItem(s_optPanels[0], t.id);
                if (!hC || SendMessage(hC, BM_GETCHECK, 0, 0) != BST_CHECKED) continue;
                wchar_t key[MAX_PATH];
                HKEY hk;
                swprintf_s(key, L"Software\\Classes\\%s", t.ext);
                if (RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL) == ERROR_SUCCESS) {
                    RegSetValueEx(hk, NULL, 0, REG_SZ, (const BYTE*)L"BillyProFile",
                        (DWORD)sizeof(L"BillyProFile")); RegCloseKey(hk);
                }
                swprintf_s(key, L"Software\\Classes\\BillyProFile\\shell\\open\\command");
                if (RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL) == ERROR_SUCCESS) {
                    RegSetValueEx(hk, NULL, 0, REG_SZ, (const BYTE*)cmd,
                        (DWORD)((wcslen(cmd)+1)*sizeof(wchar_t))); RegCloseKey(hk);
                }
                count++;
            }
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
            if (count > 0) MessageBox(hwnd, L"File associations updated.", L"Billy Pro", MB_OK|MB_ICONINFORMATION);
            else MessageBox(hwnd, L"Select at least one file type first.", L"Billy Pro", MB_OK|MB_ICONWARNING);
            break;
        }

        case ID_OPT_DEVICE_RESET: {
            if (!s_optPanels[3]) break;
            HWND hCombo = GetDlgItem(s_optPanels[3], ID_OPT_DEVICE_COMBO);
            int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            if (sel == CB_ERR) break;
            int devIdx = (int)SendMessage(hCombo, CB_GETITEMDATA, sel, 0);
            if ((DWORD)devIdx == BASS_GetDevice()) break;
            if (currentStream) {
                BASS_ChannelStop(currentStream);
                BASS_StreamFree(currentStream);
                currentStream = 0;
            }
            BASS_Free();
            BASS_Init(devIdx, 44100, 0, g_hwnd, NULL);
            ApplyDSP();
            UpdateStatusBar();
            break;
        }

        case ID_OPT_SAVE: {
            wchar_t buf[32];
            // Audio page
            GetWindowText(GetDlgItem(s_optPanels[2], ID_OPT_BB_LOW),  buf, 32); float lo = (float)_wtof(buf);
            GetWindowText(GetDlgItem(s_optPanels[2], ID_OPT_BB_HIGH), buf, 32); float hi = (float)_wtof(buf);
            GetWindowText(GetDlgItem(s_optPanels[2], ID_OPT_BB_GAIN), buf, 32); float gn = (float)_wtof(buf);
            g_bbFreqLow  = max(20.0f,  min(500.0f,  lo));
            g_bbFreqHigh = max(50.0f,  min(2000.0f, hi));
            g_bbGainDB   = max(0.0f,   min(24.0f,   gn));
            g_bassBoost   = (SendMessage(GetDlgItem(s_optPanels[2], ID_OPT_BB_ENABLE), BM_GETCHECK,0,0)==BST_CHECKED);
            g_dspReverb   = (SendMessage(GetDlgItem(s_optPanels[2], ID_OPT_REVERB),   BM_GETCHECK,0,0)==BST_CHECKED);
            g_dspSaturate = (SendMessage(GetDlgItem(s_optPanels[2], ID_OPT_SATURATE), BM_GETCHECK,0,0)==BST_CHECKED);
            g_dspVinyl    = (SendMessage(GetDlgItem(s_optPanels[2], ID_OPT_VINYL),    BM_GETCHECK,0,0)==BST_CHECKED);
            g_dspHifi     = (SendMessage(GetDlgItem(s_optPanels[2], ID_OPT_HIFI),     BM_GETCHECK,0,0)==BST_CHECKED);
            {
                wchar_t tb[16];
                GetDlgItemText(s_optPanels[2], ID_OPT_REV_MIX,   tb,16); g_revMix    = max(0.0f,  min(100.0f,  (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_REV_ROOM,  tb,16); g_revRoom   = max(0.0f,  min(100.0f,  (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_REV_WIDTH, tb,16); g_revWidth  = max(0.0f,  min(100.0f,  (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_SAT_DRIVE, tb,16); g_satDrive  = max(1.0f,  min(10.0f,   (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_SAT_LEVEL, tb,16); g_satLevel  = max(0.0f,  min(100.0f,  (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_VIN_FREQ,  tb,16); g_vinLpFreq = max(500.0f,min(20000.0f,(float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_VIN_CRACK, tb,16); g_vinCrackle= max(0.0f,  min(100.0f,  (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_HFI_BASS,  tb,16); g_hfiBassDb = max(0.0f,  min(24.0f,   (float)_wtof(tb)));
                GetDlgItemText(s_optPanels[2], ID_OPT_HFI_WARM,  tb,16); g_hfiWarmth = max(0.0f,  min(100.0f,  (float)_wtof(tb)));
            }
            // Display page
            bool wasDark = g_darkMode;
            g_darkMode   = (SendMessage(GetDlgItem(s_optPanels[1], ID_OPT_DARKMODE),  BM_GETCHECK,0,0)==BST_CHECKED);
            // Setup page
            g_multiInst  = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_MULTIINST),    BM_GETCHECK,0,0)==BST_CHECKED);
            g_dropAppend = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_DROP_APPEND),  BM_GETCHECK,0,0)==BST_CHECKED);
            { wchar_t sb[16]; GetDlgItemText(s_optPanels[0], ID_OPT_SEEK_STEP, sb, 16);
              float sv = (float)_wtof(sb); if (sv >= 1.0f) g_seekStep = sv; }
            // Device page
            if (s_optPanels[3])
                g_rememberVolume = (SendMessage(GetDlgItem(s_optPanels[3], ID_OPT_REMEMBER_VOL), BM_GETCHECK,0,0)==BST_CHECKED);
            ApplyDSP();
            if (g_hwnd) LayoutControls(g_hwnd);
            // Timestretch page
            if (s_optPanels[5]) {
                bool wasPitch = g_pitchEnabled;
                float wasAmt  = g_pitchSemitones;
                g_pitchEnabled = (SendMessage(GetDlgItem(s_optPanels[5], ID_OPT_PITCH_ENABLE), BM_GETCHECK,0,0)==BST_CHECKED);
                wchar_t pbuf[16]; GetWindowText(GetDlgItem(s_optPanels[5], ID_OPT_PITCH_VALUE), pbuf, 16);
                g_pitchSemitones = max(-12.0f, min(12.0f, (float)_wtof(pbuf)));
                if (wasPitch != g_pitchEnabled || wasAmt != g_pitchSemitones) ApplyPitch();
            }
            if (g_darkMode != wasDark) ApplyTheme();
            SaveSettings();
            DestroyWindow(hwnd);
            break;
        }
        case ID_OPT_CANCEL:
            DestroyWindow(hwnd);
            break;
        case ID_OPT_RESET:
            if (MessageBox(hwnd,
                    L"Reset all settings to defaults? This will take effect immediately.",
                    L"Reset to Defaults", MB_YESNO | MB_ICONWARNING) == IDYES) {
                // Delete INI so LoadSettings reads all defaults
                GetIniPath();
                DeleteFile(g_iniPath);
                g_iniPath[0] = L'\0';
                LoadSettings();
                // Apply all effects
                ApplyDSP();
                ApplyPitch();
                ApplyTheme();
                // Repaint toggle buttons on main window
                if (hShuffleBtn)    InvalidateRect(hShuffleBtn,    NULL, TRUE);
                if (hRepeatBtn)     InvalidateRect(hRepeatBtn,     NULL, TRUE);
                if (hMonoBtn)       InvalidateRect(hMonoBtn,       NULL, TRUE);
                if (hNormalizeBtn)  InvalidateRect(hNormalizeBtn,  NULL, TRUE);
                if (hBassBoostBtn)  InvalidateRect(hBassBoostBtn,  NULL, TRUE);
                if (hDspBtn)        InvalidateRect(hDspBtn,        NULL, TRUE);
                LayoutControls(g_hwnd); // update star + FX button visibility
                // Save defaults so exit doesn't overwrite with stale values
                SaveSettings();
                DestroyWindow(hwnd);
            }
            break;
        }
        break;

    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_CTLCOLORBTN: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_HSCROLL: {
        // Pitch slider moved — update the value edit field
        if (!s_optPanels[5]) break;
        HWND hSlider = GetDlgItem(s_optPanels[5], ID_OPT_PITCH_SLIDER);
        if ((HWND)lParam != hSlider) break;
        int pos = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
        float semitones = pos / 10.0f;
        wchar_t buf[16]; swprintf_s(buf, L"%.1f", semitones);
        // Update edit without triggering EN_CHANGE feedback loop
        HWND hEdit = GetDlgItem(s_optPanels[5], ID_OPT_PITCH_VALUE);
        SetWindowText(hEdit, buf);
        break;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        g_hwndOptions = NULL;
        for (auto& p : s_optPanels) p = NULL;
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenOptionsDialog()
{
    if (g_hwndOptions && IsWindow(g_hwndOptions)) { SetForegroundWindow(g_hwndOptions); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t OPT_CLASS[] = L"BillyOptionsDlg";
    static bool optReg = false;
    if (!optReg) {
        WNDCLASS wc = {};
        wc.lpfnWndProc   = OptionsDlgProc;
        wc.hInstance     = hInst;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.lpszClassName = OPT_CLASS;
        RegisterClass(&wc);
        optReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 490, 548 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndOptions = CreateWindowEx(WS_EX_APPWINDOW, OPT_CLASS,
        L"Settings", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndOptions, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top  + (pr.bottom - pr.top  - dh) / 2,
        dw, dh, SWP_NOZORDER);
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
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); convReg = true;
    }
    // Compute window size so the client area exactly fits the layout (client = 716 x 470)
    // Controls: cw=700 wide, bottom of buttons at y=452, +18px bottom padding = 470 client height
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME;
    RECT cr = { 0, 0, 716, 470 };
    AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right  - cr.left;
    int dh = cr.bottom - cr.top;

    g_hwndConvert = CreateWindowEx(WS_EX_APPWINDOW, CONV_CLASS,
        L"Convert",
        style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);

    // Centre over the main window
    RECT pr; GetWindowRect(g_hwnd, &pr);
    int cx = pr.left + (pr.right  - pr.left - dw) / 2;
    int cy = pr.top  + (pr.bottom - pr.top  - dh) / 2;
    SetWindowPos(g_hwndConvert, NULL, cx, cy, dw, dh, SWP_NOZORDER);
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

// Subclass proc for the search edit box — routes Enter/Delete to the parent
static WNDPROC g_OldSearchEditProc = NULL;
static LRESULT CALLBACK SearchEditSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            SearchPlaySelected();
            return 0;
        }
        if (wParam == VK_ESCAPE) {
            if (g_hwndSearch && IsWindow(g_hwndSearch)) DestroyWindow(g_hwndSearch);
            return 0;
        }
    }
    return CallWindowProc(g_OldSearchEditProc, hwnd, msg, wParam, lParam);
}

// Delete selected item(s) from search results and from the real playlist
static void SearchDeleteSelected()
{
    if (!hSearchList) return;
    int sel = (int)SendMessage(hSearchList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR || sel >= (int)g_searchResults.size()) return;
    int playlistIdx = g_searchResults[sel];
    if (playlistIdx < 0 || playlistIdx >= (int)g_playlist.size()) return;

    bool wasPlaying = (playlistIdx == g_currentIndex);
    g_playlist.erase(g_playlist.begin() + playlistIdx);
    SendMessage(hListBox, LB_DELETESTRING, playlistIdx, 0);
    if (wasPlaying) StopAudio();
    RebuildShuffleOrder();
    g_currentIndex = min(g_currentIndex, (int)g_playlist.size() - 1);
    if (!g_playlist.empty())
        SendMessage(hListBox, LB_SETCURSEL, max(0, g_currentIndex), 0);
    UpdateStatusBar();

    // Refresh search results
    SearchFilter();
    int newSel = min(sel, (int)SendMessage(hSearchList, LB_GETCOUNT, 0, 0) - 1);
    if (newSel >= 0) SendMessage(hSearchList, LB_SETCURSEL, newSel, 0);
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
        RECT rc; GetClientRect(hwnd, &rc);
        int cw = rc.right, ch = rc.bottom;
        if (hSearchEdit) MoveWindow(hSearchEdit, 8, 8, cw - 16, 26, TRUE);
        if (hSearchList) MoveWindow(hSearchList, 8, 42, cw - 16, ch - 42 - 46, TRUE);
        HWND hOK  = GetDlgItem(hwnd, ID_SEARCH_OK);
        HWND hCan = GetDlgItem(hwnd, ID_SEARCH_CANCEL);
        int bw = 88, bh = 28, by = ch - 38;
        if (hOK)  MoveWindow(hOK,  cw / 2 - bw - 4, by, bw, bh, TRUE);
        if (hCan) MoveWindow(hCan, cw / 2 + 4,       by, bw, bh, TRUE);
        break;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
        if (wParam == VK_RETURN) { SearchPlaySelected(); return 0; }
        if (wParam == VK_DELETE) { SearchDeleteSelected(); return 0; }
        break;
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
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

    static const wchar_t SEARCH_CLASS[] = L"BillySearchWnd";
    static bool searchRegistered = false;
    if (!searchRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc   = SearchWndProc;
        wc.hInstance     = hInst;
        wc.lpszClassName = SEARCH_CLASS;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        searchRegistered = true;
    }

    // Desired client area: 360 x 420
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME;
    RECT cr = { 0, 0, 360, 420 };
    AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left;
    int dh = cr.bottom - cr.top;

    HWND hDlg = CreateWindowEx(WS_EX_APPWINDOW, SEARCH_CLASS,
        L"Find in Playlist", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
    if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hDlg, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hDlg, 19, &dk, sizeof(dk)); }

    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(hDlg, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top  + (pr.bottom - pr.top  - dh) / 2,
        dw, dh, SWP_NOZORDER);

    g_hwndSearch = hDlg;
    g_searchResults.clear();

    // Create controls — WM_SIZE will position them correctly on first paint
    hSearchEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        8, 8, 344, 26, hDlg, (HMENU)ID_SEARCH_EDIT, hInst, NULL);
    SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

    // Subclass edit to catch Enter / Escape
    g_OldSearchEditProc = (WNDPROC)SetWindowLongPtr(hSearchEdit, GWLP_WNDPROC,
        (LONG_PTR)SearchEditSubProc);

    hSearchList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        8, 42, 344, 330, hDlg, (HMENU)ID_SEARCH_LIST, hInst, NULL);
    SendMessage(hSearchList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hSearchList, LB_SETITEMHEIGHT, 0, 20);

    HWND hOK = CreateWindow(L"BUTTON", L"Play",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        84, 380, 88, 28, hDlg, (HMENU)ID_SEARCH_OK, hInst, NULL);
    HWND hCan = CreateWindow(L"BUTTON", L"Close",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        188, 380, 88, 28, hDlg, (HMENU)ID_SEARCH_CANCEL, hInst, NULL);
    SendMessage(hOK,  WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hCan, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

    // Trigger layout via WM_SIZE
    RECT rcClient; GetClientRect(hDlg, &rcClient);
    SendMessage(hDlg, WM_SIZE, SIZE_RESTORED,
        MAKELPARAM(rcClient.right, rcClient.bottom));

    SearchFilter();
    SetFocus(hSearchEdit);
}

// ============================================================
//  Theme / fonts
// ============================================================
void ApplyTheme()
{
    g_theme = g_darkMode ? DARK : LIGHT;
    if (g_brBg)   { DeleteObject(g_brBg);   g_brBg   = NULL; }
    if (g_brList) { DeleteObject(g_brList); g_brList = NULL; }
    if (g_brMenu) { DeleteObject(g_brMenu); g_brMenu = NULL; }
    g_brBg   = CreateSolidBrush(g_theme.bg);
    g_brList = CreateSolidBrush(g_theme.bgList);
    g_brMenu = CreateSolidBrush(g_darkMode ? (COLORREF)0x2A2A2A : RGB(255, 255, 255));

    if (g_hwnd) {
        // Dark title bar — attribute 20 (Win10 1903+/Win11), 19 (Win10 1809)
        BOOL dark = g_darkMode ? TRUE : FALSE;
        DwmSetWindowAttribute(g_hwnd, 20, &dark, sizeof(dark));
        DwmSetWindowAttribute(g_hwnd, 19, &dark, sizeof(dark));

        // Listbox scrollbar style
        if (hListBox)
            SetWindowTheme(hListBox, g_darkMode ? L"DarkMode_Explorer" : L"Explorer", NULL);

        // Status bar: strip visual style so background + custom painting work
        if (hStatus) {
            SetWindowTheme(hStatus, g_darkMode ? L"" : NULL, g_darkMode ? L"" : NULL);
            SendMessage(hStatus, SB_SETBKCOLOR, 0,
                (LPARAM)(g_darkMode ? (COLORREF)g_theme.bg : CLR_DEFAULT));
            InvalidateRect(hStatus, NULL, TRUE);
        }

        // Menus — owner-draw applied once (idempotent); colors adapt via g_darkMode in WM_DRAWITEM
        HMENU hm = GetMenu(g_hwnd);
        if (hm) {
            MenuInitOwnerDraw(hm);  // safe to call every time — skips already-converted items
            // Set popup background so system border/gaps match dark background
            MENUINFO mi = {}; mi.cbSize = sizeof(mi);
            mi.fMask    = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
            mi.hbrBack  = g_darkMode ? g_brMenu : NULL;  // NULL = system default white in light mode
            SetMenuInfo(hm, &mi);
            CheckMenuItem(hm, IDM_VIEW_DARKMODE,
                g_darkMode ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);
            DrawMenuBar(g_hwnd);
        }

        // Repaint everything cleanly
        SetClassLongPtr(g_hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)g_brBg);
        RedrawWindow(g_hwnd, NULL, NULL,
            RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_FRAME);
    }
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
    if (hStatus) {
        SendMessage(hStatus, WM_SIZE, 0, 0);
        int parts[2] = { rc.right - 280, -1 };
        SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);
    }

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
    // FX (DSP) button: right of BassBoost, visible only when any extra effect is enabled
    if (hDspBtn) {
        bool showDsp = g_dspReverb || g_dspSaturate || g_dspVinyl || g_dspHifi;
        MoveWindow(hDspBtn, sbx + 236, y, 30, bh, TRUE);
        ShowWindow(hDspBtn, showDsp ? SW_SHOW : SW_HIDE);
    }
    // Row 2: seek bar
    int vw = 12;  // vol bar width

    // Star (media) button: far right, next to vol bar — only visible when media folders are set
    if (hMediaBtn) {
        if (!g_mediaFolders.empty()) {
            MoveWindow(hMediaBtn, rc.right - vw - 4 - 22, y, 22, bh, TRUE);
            ShowWindow(hMediaBtn, SW_SHOW);
        } else {
            ShowWindow(hMediaBtn, SW_HIDE);
        }
    }
    int sh = 14;
    int sy = y + bh + 3;
    MoveWindow(hSeekCanvas, m, sy, rc.right - 2 * m - vw - 2, sh, TRUE);

    // Row 3: time labels + pct label — sizes computed from actual font/track length
    int ty = sy + sh + 3;
    int th = 16;
    int pctW = 36;
    int vx = rc.right - vw;
    int pctX = vx - 2 - pctW;
    {
        // Measure how wide the time strings will be with the monospace font
        double tlen = GetTrackLength();
        wchar_t tstr[32];
        FormatTime(tlen, tstr, _countof(tstr));
        HDC hdc = GetDC(hTimeCur ? hTimeCur : hwnd);
        HFONT oldF = (HFONT)SelectObject(hdc, g_fontMono);
        SIZE sz = {};
        GetTextExtentPoint32(hdc, tstr, (int)wcslen(tstr), &sz);
        int baseW = sz.cx + 8;
        SelectObject(hdc, oldF);
        ReleaseDC(hTimeCur ? hTimeCur : hwnd, hdc);

        int curW = max(baseW, 44);
        int totW = max(baseW + 18, 58);  // "/ " prefix
        int remW = max(baseW + 10, 58);  // "-"  prefix
        MoveWindow(hTimeCur,    m,                          ty, curW, th, TRUE);
        MoveWindow(hTimeTot,    m + curW + 2,               ty, totW, th, TRUE);
        MoveWindow(hTimeRemain, m + curW + 2 + totW + 2,   ty, remW, th, TRUE);
    }
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

    // If replace mode, clear before adding anything
    if (!g_dropAppend) ClearPlaylist();

    for (UINT i = 0; i < n; i++) {
        wchar_t p[MAX_PATH]; DragQueryFile(hd, i, p, MAX_PATH);
        DWORD a = GetFileAttributes(p);
        if (a == INVALID_FILE_ATTRIBUTES) continue;
        if (a & FILE_ATTRIBUTE_DIRECTORY)
            AddFolder(p);
        else if (IsM3U(p))
            LoadM3UFromPath(p, true);  // always append M3U contents on drop
        else if (_wcsicmp(wcsrchr(p, L'.') ? wcsrchr(p, L'.') : L"", L".bpp") == 0) {
            LoadBppPlaylist(p);
            DragFinish(hd);
            UpdateStatusBar();
            return;
        }
        else if (IsAudio(p))
            AddTrack(p);
    }
    RebuildShuffleOrder();
    if (!g_playlist.empty() && g_currentIndex < 0) {
        g_currentIndex = 0;
        SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
        SendMessage(hListBox, LB_SETCURSEL, 0, 0);
    }
    DragFinish(hd);
    RefreshListboxStars();
    UpdateStatusBar();
    if (hListBox) SetFocus(hListBox);
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
    case 'S':
        g_shuffle = !g_shuffle;
        CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_SHUFFLE,
            MF_BYCOMMAND | (g_shuffle ? MF_CHECKED : MF_UNCHECKED));
        InvalidateRect(hShuffleBtn, NULL, TRUE); return true;
    case 'R':
        g_repeat = !g_repeat;
        CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_REPEAT,
            MF_BYCOMMAND | (g_repeat ? MF_CHECKED : MF_UNCHECKED));
        InvalidateRect(hRepeatBtn, NULL, TRUE);  return true;
    case VK_UP:
        if (!(GetKeyState(VK_CONTROL) & 0x8000)) return false; // only Ctrl+Up for volume
        currentVolume = min(1.0f, currentVolume + 0.05f);
        UpdateVolume(); return true;
    case VK_DOWN:
        if (!(GetKeyState(VK_CONTROL) & 0x8000)) return false; // only Ctrl+Down for volume
        currentVolume = max(0.0f, currentVolume - 0.05f);
        UpdateVolume(); return true;
    }
    return false;
}

// ============================================================
//  Status bar subclass — paints text in theme color for dark mode
// (scroll variables declared in forward section above)

// ============================================================
static LRESULT CALLBACK StatusBarProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, g_darkMode ? g_brBg : (HBRUSH)GetStockObject(WHITE_BRUSH));
        HFONT oldFont = (HFONT)SelectObject(hdc, g_fontUI);
        SetTextColor(hdc, g_darkMode ? g_theme.text : GetSysColor(COLOR_WINDOWTEXT));
        SetBkMode(hdc, TRANSPARENT);
        int n = (int)SendMessage(hwnd, SB_GETPARTS, 0, 0);
        for (int i = 0; i < n; i++) {
            RECT prc = {};
            SendMessage(hwnd, SB_GETRECT, i, (LPARAM)&prc);
            wchar_t text[512] = {};
            LRESULT lr = SendMessage(hwnd, SB_GETTEXT, i, (LPARAM)text);
            if (HIWORD(lr) & SBT_OWNERDRAW) continue;
            prc.left += 4; prc.right -= 2;

            SIZE sz = {};
            GetTextExtentPoint32(hdc, text, (int)wcslen(text), &sz);
            int panelW = prc.right - prc.left;

            if (i == 0 && sz.cx > panelW && text[0]) {
                // Split into prefix ("N / M  ") and filename
                // Find the filename part: after the second space following digits
                const wchar_t* nameStart = text;
                if (g_currentIndex >= 0 && currentStream) {
                    // Format is "N / M  filename" — find the second double-space
                    const wchar_t* ds = wcsstr(text, L"  ");
                    if (ds) nameStart = ds + 2;
                }
                int prefixLen = (int)(nameStart - text);
                SIZE prefSz = {};
                if (prefixLen > 0)
                    GetTextExtentPoint32(hdc, text, prefixLen, &prefSz);
                SIZE nameSz = {};
                GetTextExtentPoint32(hdc, nameStart, (int)wcslen(nameStart), &nameSz);
                int nameAreaW = panelW - prefSz.cx;

                if (nameSz.cx > nameAreaW && nameAreaW > 20) {
                    // Need scrolling on filename part
                    if (wcscmp(text, g_stLastText) != 0) {
                        wcsncpy_s(g_stLastText, text, _TRUNCATE);
                        g_stNameW = nameSz.cx; g_stNameAreaW = nameAreaW;
                        g_stScrollX = 0; g_stPause = 30;
                        if (!g_stScrolling) {
                            SetTimer(g_hwnd, IDT_STATUS_SCROLL, 50, NULL);
                            g_stScrolling = true;
                        }
                    }
                    // Draw prefix (static)
                    if (prefixLen > 0) {
                        RECT pr2 = prc; pr2.right = pr2.left + prefSz.cx;
                        DrawText(hdc, text, prefixLen, &pr2, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
                    }
                    // Draw filename (scrolling, clipped)
                    RECT nameRect = prc;
                    nameRect.left += prefSz.cx;
                    HRGN clip = CreateRectRgn(nameRect.left, nameRect.top, nameRect.right, nameRect.bottom);
                    SelectClipRgn(hdc, clip);
                    RECT tr = nameRect; tr.left -= g_stScrollX; tr.right = tr.left + nameSz.cx + 60;
                    DrawText(hdc, nameStart, -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
                    SelectClipRgn(hdc, NULL);
                    DeleteObject(clip);
                } else {
                    // Fits or no meaningful split — just draw with ellipsis
                    DrawText(hdc, text, -1, &prc,
                        DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
                }
            } else {
                if (i == 0 && g_stScrolling) {
                    KillTimer(g_hwnd, IDT_STATUS_SCROLL);
                    g_stScrolling = false; g_stScrollX = 0;
                }
                DrawText(hdc, text, -1, &prc,
                    DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
            }

            if (i < n - 1) {
                COLORREF divCol = g_darkMode ? g_theme.btnBorder : RGB(200, 200, 200);
                HPEN pen = CreatePen(PS_SOLID, 1, divCol);
                HPEN op = (HPEN)SelectObject(hdc, pen);
                MoveToEx(hdc, prc.right, prc.top + 2, NULL);
                LineTo(hdc, prc.right, prc.bottom - 2);
                SelectObject(hdc, op); DeleteObject(pen);
            }
        }
        SelectObject(hdc, oldFont);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return CallWindowProc(g_OldStatusProc, hwnd, msg, wp, lp);
}

// ============================================================
//  Owner-draw menus — set up ONCE, never removed.
//  Colors adapt to g_darkMode in WM_DRAWITEM; no toggle = no glitch.
// ============================================================
static void MenuInitOwnerDraw(HMENU hm)
{
    int n = GetMenuItemCount(hm);
    for (int i = 0; i < n; i++) {
        MENUITEMINFO mii = { sizeof(mii),
            MIIM_FTYPE | MIIM_STRING | MIIM_SUBMENU | MIIM_DATA };
        wchar_t buf[256] = {};
        mii.dwTypeData = buf; mii.cch = 255;
        if (!GetMenuItemInfo(hm, i, TRUE, &mii)) continue;
        if (mii.hSubMenu) MenuInitOwnerDraw(mii.hSubMenu); // recurse first
        if (mii.fType & MFT_SEPARATOR) continue;
        if (mii.fType & MFT_OWNERDRAW) continue;  // already done
        auto* str = new wchar_t[256];
        wcscpy_s(str, 256, buf);
        mii.fMask      = MIIM_FTYPE | MIIM_DATA;
        mii.fType     |= MFT_OWNERDRAW;
        mii.dwItemData = (ULONG_PTR)str;
        SetMenuItemInfo(hm, i, TRUE, &mii);
    }
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
        // Keep default 500ms playback buffer for smooth playback

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
        hDspBtn       = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DSP,       hInst, NULL);
        hMediaBtn     = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MEDIA,     hInst, NULL);

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
        SetWindowLongPtr(hDspBtn,       GWLP_WNDPROC, (LONG_PTR)BtnHotProc);

        hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, hInst, NULL);
        SendMessage(hStatus, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        { RECT cr; GetClientRect(hwnd, &cr);
          int parts[2] = { cr.right - 280, -1 };
          SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts); }
        // Subclass for dark mode text color
        g_OldStatusProc = (WNDPROC)SetWindowLongPtr(hStatus, GWLP_WNDPROC, (LONG_PTR)StatusBarProc);

        LoadSettings();
        LoadFavorites();
        ApplyTheme();  // re-apply after LoadSettings so dark mode is applied on startup
        DragAcceptFiles(hwnd, TRUE);
        LayoutControls(hwnd);
        UpdateVolume();
        UpdateStatusBar();

        // Register global media key hotkeys
        RegisterHotKey(hwnd, ID_HOTKEY_PLAYPAUSE, 0, VK_MEDIA_PLAY_PAUSE);
        RegisterHotKey(hwnd, ID_HOTKEY_NEXT,      0, VK_MEDIA_NEXT_TRACK);
        RegisterHotKey(hwnd, ID_HOTKEY_PREV,      0, VK_MEDIA_PREV_TRACK);
        RegisterHotKey(hwnd, ID_HOTKEY_STOP,      0, VK_MEDIA_STOP);

        // Taskbar thumbnail toolbar (Win7+)
        g_WM_TASKBARBUTTONCREATED = RegisterWindowMessage(L"TaskbarButtonCreated");
        g_thumbIcons[0] = CreateThumbIcon(L"\u23EE");  // ⏮ previous
        g_thumbIcons[1] = CreateThumbIcon(L"\u25B6");  // ▶ play
        g_thumbIcons[2] = CreateThumbIcon(L"\u23F8");  // ⏸ pause
        g_thumbIcons[3] = CreateThumbIcon(L"\u23ED");  // ⏭ next
        CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
            IID_ITaskbarList3, (void**)&g_pTaskbar);
        if (g_pTaskbar) g_pTaskbar->HrInit();
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
        SetTextColor(dc, g_theme.text);
        SetBkMode(dc, TRANSPARENT);
        return (LRESULT)g_brBg;
    }

    case WM_MEASUREITEM: {
        MEASUREITEMSTRUCT* ms = (MEASUREITEMSTRUCT*)lParam;
        if (ms->CtlType == ODT_LISTBOX && ms->CtlID == ID_LISTBOX) {
            ms->itemHeight = 20;
            return TRUE;
        }
        if (ms->CtlType == ODT_MENU) {
            const wchar_t* text = (const wchar_t*)ms->itemData;
            if (!text || !*text) { ms->itemHeight = 5; ms->itemWidth = 40; return TRUE; } // separator
            HDC dc = GetDC(hwnd);
            HFONT old = (HFONT)SelectObject(dc, g_fontUI ? g_fontUI : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
            SIZE sz = {};
            GetTextExtentPoint32(dc, text, (int)wcslen(text), &sz);
            SelectObject(dc, old); ReleaseDC(hwnd, dc);
            // Match system default menu bar item sizing
            ms->itemWidth  = (UINT)(sz.cx + 10);
            ms->itemHeight = (UINT)max(sz.cy + 4, GetSystemMetrics(SM_CYMENUSIZE));
            return TRUE;
        }
        break;
    }

    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        // Owner-draw menu items — handles both dark and light (safe fallback)
        if (dis->CtlType == ODT_MENU) {
            const wchar_t* text = (const wchar_t*)dis->itemData;
            bool sep    = (!text || !*text);
            bool sel    = (dis->itemState & ODS_SELECTED) != 0;
            bool grayed = (dis->itemState & ODS_GRAYED)   != 0;
            bool chk    = (dis->itemState & ODS_CHECKED)  != 0;

            COLORREF bgCol, fgCol;
            if (g_darkMode) {
                bgCol = sel ? 0x505858 : 0x2A2A2A;
                fgCol = grayed ? 0x666666 : 0xE8E8E8;
            } else {
                // Light mode fallback (should rarely fire — system normally draws these)
                bgCol = sel ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_MENU);
                fgCol = grayed ? GetSysColor(COLOR_GRAYTEXT)
                               : (sel ? GetSysColor(COLOR_HIGHLIGHTTEXT)
                                      : GetSysColor(COLOR_MENUTEXT));
            }

            HBRUSH br = CreateSolidBrush(bgCol);
            FillRect(dis->hDC, &dis->rcItem, br); DeleteObject(br);

            if (sep) {
                int mid = (dis->rcItem.top + dis->rcItem.bottom) / 2;
                COLORREF ln = g_darkMode ? 0x585858 : GetSysColor(COLOR_GRAYTEXT);
                HPEN pen = CreatePen(PS_SOLID, 1, ln);
                HPEN op = (HPEN)SelectObject(dis->hDC, pen);
                MoveToEx(dis->hDC, dis->rcItem.left  + 4, mid, NULL);
                LineTo  (dis->hDC, dis->rcItem.right - 4, mid);
                SelectObject(dis->hDC, op); DeleteObject(pen);
                return TRUE;
            }

            SetTextColor(dis->hDC, fgCol);
            SetBkMode(dis->hDC, TRANSPARENT);
            HFONT fntOld = (HFONT)SelectObject(dis->hDC,
                g_fontUI ? g_fontUI : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
            RECT rc = dis->rcItem;
            if (chk) {
                RECT cr = rc; cr.right = cr.left + 18;
                DrawText(dis->hDC, L"\u2713", 1, &cr, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
                rc.left += 18;
            } else {
                rc.left += 8;
            }
            if (text) {
                const wchar_t* tab = wcschr(text, L'\t');
                if (tab) {
                    // Left part: item name
                    DrawText(dis->hDC, text, (int)(tab - text), &rc,
                        DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOCLIP);
                    // Right part: shortcut — right-aligned with padding
                    RECT rr = dis->rcItem; rr.right -= 8;
                    DrawText(dis->hDC, tab + 1, -1, &rr,
                        DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
                } else {
                    DrawText(dis->hDC, text, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
                }
            }
            SelectObject(dis->hDC, fntOld);
            return TRUE;
        }
        // Owner-draw listbox — highlight favorites with subtle tint
        if (dis->CtlType == ODT_LISTBOX && dis->CtlID == ID_LISTBOX) {
            int idx = (int)dis->itemID;
            if (idx < 0 || idx >= (int)g_playlist.size()) return TRUE;
            bool selected = (dis->itemState & ODS_SELECTED) != 0;
            bool isCurrent = (idx == g_currentIndex);
            // Don't highlight favorites when in favorites view (all are favorites)
            bool isFav = !g_favActive && IsFavorite(g_playlist[idx].path);

            // Background
            COLORREF bgCol, txCol;
            if (g_darkMode) {
                bgCol = selected ? RGB(30, 80, 160) : (isFav ? RGB(38, 55, 60) : g_theme.bg);
                txCol = selected ? RGB(255, 255, 255) : (isFav ? RGB(140, 200, 220) : g_theme.text);
            } else {
                bgCol = selected ? GetSysColor(COLOR_HIGHLIGHT) : (isFav ? RGB(220, 235, 250) : GetSysColor(COLOR_WINDOW));
                txCol = selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : (isFav ? RGB(30, 80, 140) : GetSysColor(COLOR_WINDOWTEXT));
            }
            HBRUSH br = CreateSolidBrush(bgCol);
            FillRect(dis->hDC, &dis->rcItem, br);
            DeleteObject(br);

            // Text
            SetTextColor(dis->hDC, txCol);
            SetBkMode(dis->hDC, TRANSPARENT);
            HFONT oldFont = (HFONT)SelectObject(dis->hDC, g_fontUI);
            wchar_t text[MAX_PATH + 4];
            SendMessage(hListBox, LB_GETTEXT, idx, (LPARAM)text);
            RECT rc = dis->rcItem; rc.left += 4;
            DrawText(dis->hDC, text, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
            SelectObject(dis->hDC, oldFont);

            // Focus rect
            if (dis->itemState & ODS_FOCUS)
                DrawFocusRect(dis->hDC, &dis->rcItem);
            return TRUE;
        }
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
        case ID_BTN_REPEAT:     DrawBtn(dis, BTN_REPEAT);     return TRUE;
        case ID_BTN_MEDIA:      DrawBtn(dis, BTN_MEDIA);      return TRUE;
        case ID_BTN_DSP:        DrawBtn(dis, BTN_DSP);        return TRUE;
        }
        break;
    }

    case WM_DROPFILES:
        HandleDrop((HDROP)wParam);
        break;

    case WM_HOTKEY:
        switch ((int)wParam) {
        case ID_HOTKEY_PLAYPAUSE: TogglePlayPause(); break;
        case ID_HOTKEY_NEXT:      PlayNext();        break;
        case ID_HOTKEY_PREV:      PlayPrev();        break;
        case ID_HOTKEY_STOP:      StopAudio();       break;
        }
        UpdateThumbButtons();
        break;

    case WM_COPYDATA: {
        // Another instance sent us a file path to open
        COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;
        if (cds && cds->dwData == COPYDATAID_OPENFILE && cds->lpData) {
            wchar_t* path = (wchar_t*)cds->lpData;
            DWORD a = GetFileAttributes(path);
            if (a != INVALID_FILE_ATTRIBUTES) {
                if (a & FILE_ATTRIBUTE_DIRECTORY) {
                    LoadFolder(path);
                } else if (IsM3U(path)) {
                    LoadM3UFromPath(path);
                    if (!g_playlist.empty()) PlayIndex(0);
                } else if (IsAudio(path)) {
                    wchar_t folder[MAX_PATH]; wcsncpy_s(folder, path, _TRUNCATE);
                    wchar_t* sep = wcsrchr(folder, L'\\');
                    if (sep) *sep = 0;
                    LoadFolder(folder);
                    for (int i = 0; i < (int)g_playlist.size(); i++)
                        if (_wcsicmp(g_playlist[i].path, path) == 0) { PlayIndex(i); break; }
                }
            }
            SetForegroundWindow(hwnd);
            if (IsIconic(hwnd)) { ShowWindow(hwnd, SW_RESTORE); RemoveTrayIcon(); }
        }
        return TRUE;
    }

    case WM_TIMER:
        if (wParam == IDT_PLAYBACK && currentStream && !g_seekDragging) {
            static DWORD s_lastTimeTick = 0;
            DWORD now = GetTickCount();
            if (now - s_lastTimeTick >= 200) { s_lastTimeTick = now; UpdateTimeDisplays(); }
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
            g_seekRepeatCount++;
            if (g_seekRepeatCount == 3) {
                KillTimer(hwnd, IDT_SEEK_REPEAT);
                SetTimer(hwnd, IDT_SEEK_REPEAT, 80, NULL);
            }
            float step = (g_seekRepeatCount >= 3) ? g_seekStep * 4.0f : g_seekStep;
            SeekToSeconds(GetPlayPos() + g_seekKeyDir * step);
        }
        if (wParam == IDT_STATUS_SCROLL && g_stScrolling) {
            if (g_stPause > 0) {
                g_stPause--;
            } else {
                g_stScrollX += 2;
                if (g_stScrollX > g_stNameW - g_stNameAreaW + 40) {
                    g_stScrollX = 0;
                    g_stPause = 30;
                }
            }
            if (hStatus) InvalidateRect(hStatus, NULL, FALSE);
        }
        break;

        // WM_KEYDOWN on main window (e.g. when buttons have focus)
    case WM_KEYDOWN: {
        bool repeated = (lParam & 0x40000000) != 0;
        switch (wParam) {
        case VK_RIGHT:
            if (currentStream && !g_seekKeyHeld) {
                SeekToSeconds(GetPlayPos() + g_seekStep);
                g_seekKeyHeld = true; g_seekKeyDir = +1; g_seekRepeatCount = 0;
                SetTimer(hwnd, IDT_SEEK_REPEAT, 350, NULL);
            }
            break;
        case VK_LEFT:
            if (currentStream && !g_seekKeyHeld) {
                SeekToSeconds(GetPlayPos() - g_seekStep);
                g_seekKeyHeld = true; g_seekKeyDir = -1; g_seekRepeatCount = 0;
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

    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE && hListBox) {
            // Don't steal focus from child dialogs (search, options, etc.)
            bool childActive = (g_hwndSearch && IsWindow(g_hwndSearch)) ||
                               (g_hwndInfo && IsWindow(g_hwndInfo)) ||
                               (g_hwndOptions && IsWindow(g_hwndOptions)) ||
                               (g_hwndConvert && IsWindow(g_hwndConvert));
            if (!childActive) SetFocus(hListBox);
        }
        break;
    case WM_SETFOCUS:
        if (hListBox) SetFocus(hListBox);
        return 0;

    case WM_MOUSEWHEEL:
        if (hVolumeCanvas) SendMessage(hVolumeCanvas, WM_MOUSEWHEEL, wParam, lParam);
        break;

    case WM_INITMENUPOPUP:
        // Dynamically rebuild Playlists submenu when the Library menu opens
        if ((HMENU)wParam == g_hPlSubMenu && g_hPlSubMenu) {
            // Clear existing items
            while (GetMenuItemCount(g_hPlSubMenu) > 0)
                DeleteMenu(g_hPlSubMenu, 0, MF_BYPOSITION);
            ScanPlaylists();
            for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++)
                AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_BASE + i, g_bppPlaylists[i].name.c_str());
            if (!g_bppPlaylists.empty())
                AppendMenu(g_hPlSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_MANAGE, L"Open Playlists Folder...");
            // Checkmark the active playlist
            for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++)
                CheckMenuItem(g_hPlSubMenu, IDM_LIB_PL_BASE + i,
                    MF_BYCOMMAND | (g_libActivePlaylist == i ? MF_CHECKED : MF_UNCHECKED));
            // Convert new items to owner-draw for dark mode
            if (g_darkMode) MenuInitOwnerDraw(g_hPlSubMenu);
        }
        // Checkmark Favorites when Library menu opens
        if ((HMENU)wParam == g_hLibMenu && g_hLibMenu) {
            CheckMenuItem(g_hLibMenu, IDM_LIB_FAVORITES,
                MF_BYCOMMAND | (g_libActivePlaylist == -2 ? MF_CHECKED : MF_UNCHECKED));
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_PLAYPAUSE:    if (!currentStream || BASS_ChannelIsActive(currentStream) != BASS_ACTIVE_PLAYING) TogglePlayPause(); break;
        case ID_BTN_PAUSE:        if (currentStream && BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING) TogglePlayPause(); break;
        case ID_BTN_STOP:         StopAudio();       break;
        case ID_BTN_NEXT:         PlayNext();        break;
        case ID_BTN_PREV:         PlayPrev();        break;
        case ID_BTN_SHUFFLE:
            g_shuffle = !g_shuffle;
            CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_SHUFFLE,
                MF_BYCOMMAND | (g_shuffle ? MF_CHECKED : MF_UNCHECKED));
            InvalidateRect(hShuffleBtn, NULL, TRUE); break;
        case ID_BTN_REPEAT:
            g_repeat = !g_repeat;
            CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_REPEAT,
                MF_BYCOMMAND | (g_repeat ? MF_CHECKED : MF_UNCHECKED));
            InvalidateRect(hRepeatBtn, NULL, TRUE); break;
        case ID_BTN_MONO:
            g_mono = !g_mono; ApplyDSP(); InvalidateRect(hMonoBtn, NULL, TRUE); break;
        case ID_BTN_NORMALIZE:
            g_normalize = !g_normalize; ApplyDSP(); InvalidateRect(hNormalizeBtn, NULL, TRUE); break;
        case ID_BTN_BASSBOOST:
            g_bassBoost = !g_bassBoost; ApplyDSP(); InvalidateRect(hBassBoostBtn, NULL, TRUE); break;
        case ID_BTN_DSP:
            g_dspBypass = !g_dspBypass;
            ApplyDSP();
            if (hDspBtn) InvalidateRect(hDspBtn, NULL, TRUE);
            break;
        case ID_BTN_MEDIA: {
            if (g_mediaFolders.empty()) break;
            if (!g_mediaActive) {
                // Activate: save current playlist, show folder browser — keep audio playing
                g_savedPlaylist = g_playlist;
                g_mediaActive = true;
                g_browserActive = true;
                // Multiple roots: show virtual root; single root: go directly into it
                if (g_mediaFolders.size() > 1)
                    FillBrowser(L"");
                else
                    FillBrowser(g_mediaFolders[0].c_str());
            } else {
                // Deactivate: restore old playlist view — keep audio playing
                g_mediaActive = false;
                g_browserActive = false;
                g_browserItems.clear();
                // Restore saved playlist to listbox without stopping playback
                g_playlist = g_savedPlaylist;
                g_savedPlaylist.clear();
                g_currentIndex = -1;  // position in restored playlist is unknown
                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                for (auto& t : g_playlist)
                    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
                RefreshListboxStars();
                RebuildShuffleOrder();
            }
            InvalidateRect(hMediaBtn, NULL, TRUE);
            UpdateStatusBar();
            break;
        }
        case ID_LISTBOX:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int s = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (s != LB_ERR) {
                    if (g_browserActive) BrowserNavigate(s);
                    else PlayIndex(s);
                }
            }
            if (HIWORD(wParam) == LBN_SELCHANGE) UpdateStatusBar();
            break;
            // Context menu
        case IDC_CTX_PLAY:
            if (g_ctxIsBrowser) {
                if (g_ctxTrackIndex >= 0) BrowserNavigate(g_ctxTrackIndex);
            } else {
                if (g_ctxTrackIndex >= 0) PlayIndex(g_ctxTrackIndex);
            }
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
            // Save undo state (forward order)
            g_undoRemove.clear();
            std::sort(toDelete.begin(), toDelete.end());
            for (int idx : toDelete)
                if (idx >= 0 && idx < (int)g_playlist.size())
                    g_undoRemove.push_back({g_playlist[idx], idx});
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
            if (g_ctxFilePath[0]) {
                SHELLEXECUTEINFO sei = { sizeof(sei) };
                sei.lpVerb = L"properties";
                sei.lpFile = g_ctxFilePath;
                sei.fMask = SEE_MASK_INVOKEIDLIST;
                sei.hwnd = hwnd;
                ShellExecuteEx(&sei);
            }
            break;
        case IDC_CTX_AUDIOINFO:
            if (g_ctxIsBrowser)
                OpenAudioInfoForPath(g_ctxFilePath);
            else
                OpenAudioInfoDialog(g_ctxTrackIndex);
            break;
        case IDC_CTX_OPENLOCATION:
            if (g_ctxFilePath[0]) {
                wchar_t args[MAX_PATH + 16];
                _snwprintf_s(args, _countof(args), _TRUNCATE, L"/select,\"%s\"", g_ctxFilePath);
                ShellExecute(hwnd, L"open", L"explorer.exe", args, NULL, SW_SHOW);
            }
            break;
        case IDC_CTX_FAV_ADD:
            if (g_ctxFilePath[0]) AddFavorite(g_ctxFilePath);
            break;
        case IDC_CTX_FAV_REMOVE:
            if (g_ctxFilePath[0]) RemoveFavorite(g_ctxFilePath);
            break;
        case IDC_CTX_PL_NEW:
            if (g_ctxFilePath[0]) ShowNewPlaylistDialog(hwnd, g_ctxFilePath);
            break;
        case IDM_LIB_FAVORITES:
            LoadFavoritesIntoPlaylist();
            break;
        case IDM_LIB_PL_MANAGE: {
            // Open playlists folder in Explorer
            GetLibDir();
            ShellExecute(hwnd, L"open", g_plDir, NULL, NULL, SW_SHOW);
            break;
        }
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
        case IDM_FILE_OPENM3U:   LoadPlaylistM3U(hwnd); break;
        case IDM_FILE_SAVEM3U:   SavePlaylistM3U(hwnd); break;

        case IDM_VIEW_DARKMODE:
            g_darkMode = !g_darkMode;
            ApplyTheme();
            SaveSettings();
            break;

        case IDM_OPTIONS_FILEASSOC: OpenFileAssocDialog(); break;

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

        case IDM_HELP_UPDATE: {
            // Fetch latest release tag from GitHub API with 5s timeout
            wchar_t latestVer[64] = L"";
            bool ok = false;
            HINTERNET hSess = WinHttpOpen(L"BillyPro/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (hSess) {
                WinHttpSetTimeouts(hSess, 5000, 5000, 5000, 5000);
                HINTERNET hConn = WinHttpConnect(hSess, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
                if (hConn) {
                    HINTERNET hReq = WinHttpOpenRequest(hConn, L"GET",
                        L"/repos/bisoloc/BillyPro/releases/latest",
                        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
                    if (hReq) {
                        if (WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0) &&
                            WinHttpReceiveResponse(hReq, NULL)) {
                            char buf[4096] = {};
                            DWORD read = 0;
                            WinHttpReadData(hReq, buf, sizeof(buf) - 1, &read);
                            buf[read] = 0;
                            // Parse "tag_name":"vX.Y" or "tag_name":"X.Y"
                            const char* tag = strstr(buf, "\"tag_name\"");
                            if (tag) {
                                const char* q1 = strchr(tag + 10, '"');
                                if (q1) {
                                    q1++;
                                    if (*q1 == 'v' || *q1 == 'V') q1++;
                                    const char* q2 = strchr(q1, '"');
                                    if (q2 && q2 - q1 < 60) {
                                        MultiByteToWideChar(CP_UTF8, 0, q1, (int)(q2 - q1), latestVer, 63);
                                        ok = true;
                                    }
                                }
                            }
                        }
                        WinHttpCloseHandle(hReq);
                    }
                    WinHttpCloseHandle(hConn);
                }
                WinHttpCloseHandle(hSess);
            }
            wchar_t msg[512];
            if (!ok) {
                swprintf_s(msg, L"Installed: V%s\n\nCould not reach GitHub. Check your internet connection.", APP_VERSION);
                MessageBox(hwnd, msg, L"Check for Updates", MB_ICONWARNING);
            } else if (_wcsicmp(latestVer, APP_VERSION) == 0) {
                swprintf_s(msg, L"Installed: V%s\n\nYou have the latest version.", APP_VERSION);
                MessageBox(hwnd, msg, L"Check for Updates", MB_ICONINFORMATION);
            } else {
                swprintf_s(msg, L"Installed: V%s\nLatest: V%s\n\nOpen the download page?", APP_VERSION, latestVer);
                if (MessageBox(hwnd, msg, L"Update Available", MB_YESNO | MB_ICONINFORMATION) == IDYES)
                    ShellExecute(hwnd, L"open", L"https://github.com/bisoloc/BillyPro/releases", NULL, NULL, SW_SHOW);
            }
            break;
        }
        case IDM_HELP_ABOUT:
            MessageBox(hwnd,
                L"Billy Pro V0.6\n\n"
                L"Lightweight Music Player\n\n"
                L"Created by MRJN/CLD.",
                L"About Billy Pro", MB_ICONINFORMATION);
            break;
        case IDM_HELP_CONTROLS:
            MessageBox(hwnd,
                L"Keyboard Controls:\n\n"
                L"  Space          Play / Pause\n"
                L"  Left/Right   Seek -/+ 3 sec (hold to scrub)\n"
                L"  Up/Down      Volume +/- 5%%\n"
                L"  Scroll Wheel   Volume\n"
                L"  N/P          Next / Previous\n"
                L"  F/Ctrl+F     Search playlist\n"
                L"  S              Toggle Shuffle\n"
                L"  R              Toggle Repeat\n"
                L"  Delete         Remove selected track(s)\n"
                L"  Ctrl+A         Select all tracks\n"
                L"  Ctrl+Up/Down   Change audio volume\n"
                L"  Enter          Play selected\n\n"
                L"Mouse:\n"
                L"  Double-click   Play track\n"
                L"  Right-click    Context menu\n"
                L"  Drag & drop    Load folders / files",
                L"Controls", MB_ICONINFORMATION);
            break;
        case IDM_OPTIONS:
        {
            OpenOptionsDialog();
            break;
        }
        default: {
            WORD cmd = LOWORD(wParam);
            // Context menu: Add to existing playlist (IDC_CTX_PL_BASE + i)
            if (cmd >= IDC_CTX_PL_BASE && cmd < IDC_CTX_PL_BASE + 90) {
                int pi = cmd - IDC_CTX_PL_BASE;
                if (pi < (int)g_bppPlaylists.size() && g_ctxFilePath[0]) {
                    AddTrackToPlaylist(g_bppPlaylists[pi].filePath.c_str(), g_ctxFilePath);
                }
            }
            // Library menu: Load playlist (toggle — click again to go back)
            if (cmd >= IDM_LIB_PL_BASE && cmd < IDM_LIB_PL_BASE + 90) {
                LoadBppIntoPlaylist(cmd - IDM_LIB_PL_BASE);
            }
            break;
        }
        }
        break;

    case WM_PLAYNEXT:
        if (wParam == 1) {
            // Gapless transition: GaplessProc already swapped decode streams
            // Activate DTS decoder for the new track (if it's DTS)
            g_dcaDec = g_dcaDecNext;
            g_dcaDecNext = nullptr;
            if (g_dcaDec) {
                g_dcaDuration = g_dcaDec->totalDuration;
                g_dcaSampleRate = g_dcaDec->sampleRate;
                g_dcaChannels = g_dcaDec->nChannels;
                g_dcaFramesOut = g_dcaDec->pcmFramesOut;
            } else {
                g_dcaDuration = 0; g_dcaSampleRate = 0; g_dcaChannels = 0; g_dcaFramesOut = 0;
            }
            // Activate FLAC decoder for the new track (if it's FLAC)
            g_flacDec = g_flacDecNext;
            g_flacDecNext = nullptr;
            if (g_flacDec) {
                g_flacDuration = g_flacDec->totalDuration;
                g_flacSampleRate = g_flacDec->sampleRate;
                g_flacSamplesOut = 0;
            } else {
                g_flacDuration = 0; g_flacSampleRate = 0; g_flacSamplesOut = 0;
            }
            g_currentIndex = g_decNextIdx;
            g_decNextIdx = -1;
            if (g_hwnd) LayoutControls(g_hwnd);
            if (hTimeTot) {
                wchar_t buf[32], tot[40];
                FormatTime(GetTrackLength(), buf, _countof(buf));
                swprintf_s(tot, L"/ %s", buf);
                SetWindowText(hTimeTot, tot);
            }
            if (!g_browserActive) {
                SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
                SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)g_currentIndex);
                SendMessage(hListBox, LB_SETCURSEL, g_currentIndex, 0);
                SendMessage(hListBox, LB_SETTOPINDEX, max(0, g_currentIndex - 3), 0);
            }
            UpdatePlayBtn();
            UpdateStatusBar();
            UpdateWindowTitle();
            UpdateTimeDisplays();
            if (hSeekCanvas)   InvalidateRect(hSeekCanvas, NULL, FALSE);
            if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
            PreloadNext();
        }
        else if (wParam == 2) {
            // Format mismatch — next track has different sample rate or channels.
            // g_decNext still holds the pre-loaded decode stream.
            // Stop the current master and do a full PlayIndex using g_decNext.
            if (currentStream) { BASS_StreamFree(currentStream); currentStream = 0; }
            // g_decStream already cleared by GaplessProc
            int nextIdx = g_decNextIdx;
            g_decNextIdx = -1;
            if (nextIdx >= 0 && nextIdx < (int)g_playlist.size()) {
                // PlayIndex will create a new master stream matching the new format.
                // Pass g_decNext as the pre-created decode stream.
                g_currentIndex = nextIdx;
                HSTREAM dec = g_decNext;
                g_decNext = 0;
                // Activate DTS if applicable
                g_dcaDec = g_dcaDecNext;
                g_dcaDecNext = nullptr;
                if (g_dcaDec) {
                    g_dcaDuration = g_dcaDec->totalDuration;
                    g_dcaSampleRate = g_dcaDec->sampleRate;
                    g_dcaChannels = g_dcaDec->nChannels;
                    g_dcaFramesOut = 0;
                } else {
                    g_dcaDuration = 0; g_dcaSampleRate = 0; g_dcaChannels = 0; g_dcaFramesOut = 0;
                }
                // Activate FLAC if applicable
                g_flacDec = g_flacDecNext;
                g_flacDecNext = nullptr;
                if (g_flacDec) {
                    g_flacDuration = g_flacDec->totalDuration;
                    g_flacSampleRate = g_flacDec->sampleRate;
                    g_flacSamplesOut = 0;
                } else {
                    g_flacDuration = 0; g_flacSampleRate = 0; g_flacSamplesOut = 0;
                }
                // Create new master output matching the decode stream format
                BASS_CHANNELINFO ci = {};
                BASS_ChannelGetInfo(dec, &ci);
                g_decStream = dec;
                currentStream = BASS_StreamCreate(ci.freq, ci.chans, BASS_SAMPLE_FLOAT,
                    GaplessProc, g_hwnd);
                g_masterFreq  = ci.freq;
                g_masterChans = ci.chans;
                if (currentStream) {
                    if (g_normalize) {
                        float peak = ScanPeak(g_playlist[nextIdx].path);
                        float normScale = 1.0f / peak;
                        if (normScale > 4.0f) normScale = 4.0f;
                        BASS_ChannelSetAttribute(dec, BASS_ATTRIB_VOL, normScale);
                    }
                    ApplyDSP();
                    ApplyPitch();
                    UpdateVolume();
                    BASS_ChannelPlay(currentStream, FALSE);
                    SetTimer(g_hwnd, IDT_PLAYBACK, 16, NULL);
                }
                if (g_hwnd) LayoutControls(g_hwnd);
                if (!g_browserActive) {
                    SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
                    SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)g_currentIndex);
                    SendMessage(hListBox, LB_SETCURSEL, g_currentIndex, 0);
                    SendMessage(hListBox, LB_SETTOPINDEX, max(0, g_currentIndex - 3), 0);
                }
                UpdatePlayBtn(); UpdateStatusBar(); UpdateWindowTitle(); UpdateTimeDisplays();
                if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
                if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
                PreloadNext();
            }
        }
        else {
            // End of playlist or no pre-loaded stream
            KillTimer(g_hwnd, IDT_PLAYBACK);
            SetWindowText(hTimeCur, L"0:00");
            SetWindowText(hTimeRemain, L"");
            InvalidateRect(hSeekCanvas, NULL, FALSE);
            if (g_repeat && g_currentIndex >= 0) PlayIndex(g_currentIndex);
            else PlayNext();
        }
        UpdateThumbButtons();
        break;

    case WM_DESTROY:
        SaveSettings();
        RemoveTrayIcon();
        KillTimer(g_hwnd, IDT_PLAYBACK);
        KillTimer(g_hwnd, IDT_SEEK_REPEAT);
        if (g_hwndSearch && IsWindow(g_hwndSearch))   DestroyWindow(g_hwndSearch);
        if (g_hwndInfo && IsWindow(g_hwndInfo))        DestroyWindow(g_hwndInfo);
        if (g_hwndConvert && IsWindow(g_hwndConvert))  DestroyWindow(g_hwndConvert);
        if (g_hwndAssoc && IsWindow(g_hwndAssoc))      DestroyWindow(g_hwndAssoc);
        FreeArtBytes();
        DragAcceptFiles(hwnd, FALSE);
        if (g_decNext) { BASS_StreamFree(g_decNext); g_decNext = 0; }
        if (g_decStream) { BASS_StreamFree(g_decStream); g_decStream = 0; }
        if (currentStream) { BASS_ChannelStop(currentStream); BASS_StreamFree(currentStream); }
        BASS_Free();
        // Unregister media key hotkeys
        UnregisterHotKey(hwnd, ID_HOTKEY_PLAYPAUSE);
        UnregisterHotKey(hwnd, ID_HOTKEY_NEXT);
        UnregisterHotKey(hwnd, ID_HOTKEY_PREV);
        UnregisterHotKey(hwnd, ID_HOTKEY_STOP);
        // Release taskbar thumbnail
        if (g_pTaskbar) { g_pTaskbar->Release(); g_pTaskbar = NULL; }
        for (int i = 0; i < 4; i++) if (g_thumbIcons[i]) { DestroyIcon(g_thumbIcons[i]); g_thumbIcons[i] = NULL; }
        if (g_fontUI)   DeleteObject(g_fontUI);
        if (g_fontMono) DeleteObject(g_fontMono);
        if (g_fontBold) DeleteObject(g_fontBold);
        if (g_brBg)     DeleteObject(g_brBg);
        if (g_brList)   DeleteObject(g_brList);
        if (g_brMenu)   DeleteObject(g_brMenu);
        PostQuitMessage(0);
        return 0;
    }

    // Taskbar button created — add thumbnail toolbar buttons
    if (g_WM_TASKBARBUTTONCREATED && msg == g_WM_TASKBARBUTTONCREATED) {
        if (g_pTaskbar) {
            THUMBBUTTON tb[3] = {};
            tb[0].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS; tb[0].iId = THUMB_BTN_PREV;
            tb[0].hIcon = g_thumbIcons[0]; wcscpy_s(tb[0].szTip, L"Previous"); tb[0].dwFlags = THBF_ENABLED;
            tb[1].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS; tb[1].iId = THUMB_BTN_PLAY;
            tb[1].hIcon = g_thumbIcons[1]; wcscpy_s(tb[1].szTip, L"Play"); tb[1].dwFlags = THBF_ENABLED;
            tb[2].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS; tb[2].iId = THUMB_BTN_NEXT;
            tb[2].hIcon = g_thumbIcons[3]; wcscpy_s(tb[2].szTip, L"Next"); tb[2].dwFlags = THBF_ENABLED;
            g_pTaskbar->ThumbBarAddButtons(hwnd, 3, tb);
            UpdateThumbButtons(); // set correct play/pause icon from the start
        }
        return 0;
    }

    // Thumbnail toolbar button clicks (WM_COMMAND from taskbar)
    if (msg == WM_COMMAND && HIWORD(wParam) == THBN_CLICKED) {
        switch (LOWORD(wParam)) {
        case THUMB_BTN_PREV: PlayPrev();        break;
        case THUMB_BTN_PLAY: TogglePlayPause(); break;
        case THUMB_BTN_NEXT: PlayNext();        break;
        }
        UpdateThumbButtons();
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//  Entry point
// ============================================================
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR cmdLine, int nCmdShow)
{
    // Load multi-instance preference from INI before the mutex check
    // Must use the same path as GetIniPath() — %APPDATA%\BillyPro\BillyPro.ini
    bool multiInstPref = false;
    {
        wchar_t iniPath[MAX_PATH] = L"";
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, iniPath))) {
            wcscat_s(iniPath, L"\\BillyPro\\BillyPro.ini");
        } else {
            // Fallback: exe directory
            GetModuleFileName(hInst, iniPath, MAX_PATH);
            wchar_t* sl = wcsrchr(iniPath, L'\\'); if (sl) sl[1] = 0;
            wcscat_s(iniPath, L"BillyPro.ini");
        }
        multiInstPref = (GetPrivateProfileInt(L"UI", L"MultiInstance", 0, iniPath) != 0);
    }

    HANDLE hMutex = CreateMutex(NULL, TRUE, L"BillyProV4Mutex");
    bool alreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
    if (alreadyRunning) {
        HWND ex = FindWindow(CLASS_NAME, NULL);
        if (!ex) { CloseHandle(hMutex); return 0; }

        if (!multiInstPref && cmdLine && *cmdLine) {
            // Replace mode: send the file path to the existing instance via WM_COPYDATA
            wchar_t filePath[MAX_PATH] = {};
            wchar_t* p = cmdLine;
            if (*p == L'"') { p++; wchar_t* e = wcschr(p, L'"'); if (e) { wcsncpy_s(filePath, p, e-p); } else wcsncpy_s(filePath, p, _TRUNCATE); }
            else wcsncpy_s(filePath, p, _TRUNCATE);
            COPYDATASTRUCT cds = {};
            cds.dwData = COPYDATAID_OPENFILE;
            cds.cbData = (DWORD)((wcslen(filePath) + 1) * sizeof(wchar_t));
            cds.lpData = filePath;
            SendMessage(ex, WM_COPYDATA, 0, (LPARAM)&cds);
        } else if (!multiInstPref) {
            SetForegroundWindow(ex);
            if (IsIconic(ex)) ShowWindow(ex, SW_RESTORE);
        }
        // In multi-instance mode, fall through and launch a new instance
        if (!multiInstPref) { CloseHandle(hMutex); return 0; }
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
    HMENU hView = CreatePopupMenu();

    AppendMenu(hFile, MF_STRING, IDM_FILE_OPENFOLDER, L"Open Folder...\tCtrl+O");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_OPENM3U,   L"Load Playlist (M3U)...");
    AppendMenu(hFile, MF_STRING, IDM_FILE_SAVEM3U,   L"Save Playlist (M3U)...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_CONVERT,   L"Convert...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT,      L"Exit");

    AppendMenu(hPlay, MF_STRING, IDM_PLAY_PLAYPAUSE, L"Play / Pause\tSpace");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_STOP,      L"Stop");
    AppendMenu(hPlay, MF_SEPARATOR, 0, NULL);
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_PREV,      L"Previous\tP");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_NEXT,      L"Next\tN");
    AppendMenu(hPlay, MF_SEPARATOR, 0, NULL);
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_SHUFFLE,   L"Shuffle\tS");
    AppendMenu(hPlay, MF_STRING, IDM_PLAY_REPEAT,    L"Repeat\tR");

    AppendMenu(hView, MF_STRING, IDM_VIEW_DARKMODE,  L"Dark Mode");

    AppendMenu(hOpts, MF_STRING, IDM_OPTIONS, L"Options...");

    AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT,    L"About...");
    AppendMenu(hHelp, MF_STRING, IDM_HELP_CONTROLS, L"Controls...");
    AppendMenu(hHelp, MF_SEPARATOR, 0, NULL);
    AppendMenu(hHelp, MF_STRING, IDM_HELP_UPDATE,  L"Check for Updates...");

    // Library menu (Favorites + cascading Playlists submenu)
    HMENU hLib = CreatePopupMenu();
    g_hLibMenu = hLib;
    AppendMenu(hLib, MF_STRING, IDM_LIB_FAVORITES, L"Favorites");
    AppendMenu(hLib, MF_SEPARATOR, 0, NULL);
    // Playlists as cascading submenu (rebuilt dynamically in WM_INITMENUPOPUP)
    g_hPlSubMenu = CreatePopupMenu();
    ScanPlaylists();
    for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++)
        AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_BASE + i, g_bppPlaylists[i].name.c_str());
    if (!g_bppPlaylists.empty())
        AppendMenu(g_hPlSubMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_MANAGE, L"Open Playlists Folder...");
    AppendMenu(hLib, MF_POPUP, (UINT_PTR)g_hPlSubMenu, L"Playlists");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPlay, L"&Play");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hView, L"&View");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLib,  L"&Library");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOpts, L"&Options");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"&Help");
    SetMenu(hwnd, hMenu);

    // Now that the menu exists, apply persisted checkmarks
    if (g_darkMode)  CheckMenuItem(hMenu, IDM_VIEW_DARKMODE,  MF_BYCOMMAND | MF_CHECKED);
    if (g_shuffle)   CheckMenuItem(hMenu, IDM_PLAY_SHUFFLE,   MF_BYCOMMAND | MF_CHECKED);
    if (g_repeat)    CheckMenuItem(hMenu, IDM_PLAY_REPEAT,    MF_BYCOMMAND | MF_CHECKED);
    ApplyTheme();  // re-run so dark mode title bar / controls are applied with menu present

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
            else if (IsM3U(filePath)) {
                LoadM3UFromPath(filePath);
                if (!g_playlist.empty()) PlayIndex(0);
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
        // No arguments: if media folder is configured, auto-load it; otherwise load from exe folder
        if (!g_mediaFolders.empty() && GetFileAttributes(g_mediaFolders[0].c_str()) != INVALID_FILE_ATTRIBUTES) {
            LoadFolder(g_mediaFolders[0].c_str());
        } else {
            wchar_t exePath[MAX_PATH] = {};
            GetModuleFileName(hInst, exePath, MAX_PATH);
            wchar_t* lastSep = wcsrchr(exePath, L'\\');
            if (lastSep) *lastSep = L'\0';
            if (exePath[0]) LoadFolder(exePath);
        }
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
            HWND focusWnd = GetFocus();
            // Don't steal keys when any dialog (info, options, convert) or search has focus
            bool searchFocused = (g_hwndSearch && IsWindow(g_hwndSearch) &&
                focusWnd == hSearchEdit);
            bool dialogFocused =
                (g_hwndInfo    && IsWindow(g_hwndInfo)    && (focusWnd == g_hwndInfo    || IsChild(g_hwndInfo,    focusWnd))) ||
                (g_hwndOptions && IsWindow(g_hwndOptions) && (focusWnd == g_hwndOptions || IsChild(g_hwndOptions, focusWnd))) ||
                (g_hwndConvert && IsWindow(g_hwndConvert) && (focusWnd == g_hwndConvert || IsChild(g_hwndConvert, focusWnd)));
            if (!searchFocused && !dialogFocused) {
                WPARAM vk = msg.wParam;
                // Ctrl+F opens search
                if (vk == 'F' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    OpenSearchDialog();
                    continue;
                }
                // Ctrl+A selects all tracks in the playlist listbox
                if (vk == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    if (hListBox) {
                        SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)-1);
                        UpdateStatusBar();
                    }
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
        // Let each secondary window handle its own dialog messages (Tab, Enter, etc.)
        if (g_hwndSearch && IsWindow(g_hwndSearch)) {
            // Handle Ctrl+key shortcuts for the search edit before IsDialogMessage eats them
            if (msg.message == WM_KEYDOWN && GetFocus() == hSearchEdit &&
                (GetKeyState(VK_CONTROL) & 0x8000)) {
                if (msg.wParam == 'A') {
                    // Ctrl+A: select all text
                    SendMessage(hSearchEdit, EM_SETSEL, 0, -1);
                    continue;
                }
                if (msg.wParam == VK_DELETE || msg.wParam == VK_BACK) {
                    // Ctrl+Delete: delete word forward; Ctrl+Backspace: delete word backward
                    wchar_t txt[512]; GetWindowText(hSearchEdit, txt, 512);
                    DWORD selStart = 0, selEnd = 0;
                    SendMessage(hSearchEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
                    int len = (int)wcslen(txt);
                    int pos = (int)selStart;
                    if (msg.wParam == VK_DELETE) {
                        // Find end of next word
                        int e = pos;
                        while (e < len && txt[e] == L' ') e++;  // skip spaces
                        while (e < len && txt[e] != L' ') e++;  // skip word
                        SendMessage(hSearchEdit, EM_SETSEL, pos, e);
                        SendMessage(hSearchEdit, EM_REPLACESEL, TRUE, (LPARAM)L"");
                    } else {
                        // Find start of previous word
                        int s = pos;
                        while (s > 0 && txt[s-1] == L' ') s--;  // skip spaces
                        while (s > 0 && txt[s-1] != L' ') s--;  // skip word
                        SendMessage(hSearchEdit, EM_SETSEL, s, pos);
                        SendMessage(hSearchEdit, EM_REPLACESEL, TRUE, (LPARAM)L"");
                    }
                    continue;
                }
            }
            if (IsDialogMessage(g_hwndSearch, &msg)) continue;
        }
        if (g_hwndInfo    && IsWindow(g_hwndInfo)    && IsDialogMessage(g_hwndInfo,    &msg)) continue;
        if (g_hwndOptions && IsWindow(g_hwndOptions) && IsDialogMessage(g_hwndOptions, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hMutex);
    CoUninitialize();
    return (int)msg.wParam;
}