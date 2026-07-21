// BillyPro.cpp  -  Win32 + BASS audio player  v0.9.1
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
#include <random>
#include <map>
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
#pragma comment(lib, "runtimeobject.lib")
#include <winhttp.h>
#include <dwmapi.h>
#include <shobjidl.h>

// SMTC (System Media Transport Controls) for lock screen / media overlay
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Storage.Streams.h>
#include <SystemMediaTransportControlsInterop.h>

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
#define ID_BTN_RECORD       129
#define ID_BTN_BACK         128
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
#define IDC_CTX_DOWNLOAD      309  // Download a Sets.LIVE stream to disk
#define IDC_CTX_PL_BASE       310  // 310..399 = existing playlists
#define IDC_CTX_OPENINBILLY   400  // Open containing folder in Billy's media browser

// Library menu
#define IDM_LIB_FAVORITES     2501
#define IDM_LIB_PL_MANAGE     2502
#define IDM_LIB_RADIO         2503
#define IDM_LIB_PL_MANAGER    2504
#define IDM_LIB_SETSLIVE      2505  // Sets.LIVE browser (shown when enabled)
#define IDM_LIB_PL_BASE       2510  // 2510..2599 = playlist entries

// Sets.LIVE browser dialog
#define ID_SETS_LIST     780
#define ID_SETS_PATH     781
#define ID_SETS_BACK     782
#define ID_SETS_PLAY     783
#define ID_SETS_REFRESH  784
#define ID_SETS_CLOSE    785
#define ID_SETS_STATUS   786

// Playlist Manager dialog
#define ID_PMGR_LIST     740
#define ID_PMGR_LOAD     741
#define ID_PMGR_NEW      742
#define ID_PMGR_RENAME   743
#define ID_PMGR_DELETE   744
#define ID_PMGR_UP       745
#define ID_PMGR_DOWN     746
#define ID_PMGR_ART      747
#define ID_PMGR_EXPORT   748
#define ID_PMGR_IMPORT   749
#define ID_PMGR_CLOSE    750

// Internet Radio dialog
#define ID_RADIO_SEARCH       440
#define ID_RADIO_QUERY        441
#define ID_RADIO_LIST         442
#define ID_RADIO_PLAY         443
#define ID_RADIO_STOP         444
#define ID_RADIO_CLOSE        445
#define ID_RADIO_STATUS       446
#define ID_RADIO_GENRE        447

// Search dialog
#define ID_SEARCH_EDIT      201
#define ID_SEARCH_LIST      202
#define ID_SEARCH_OK        203
#define ID_SEARCH_CANCEL    204
#define ID_SEARCH_SAVEPL    205

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
#define ID_INFO_DISCOGS     417

// Discogs tag browser dialog
#define ID_DCG_ARTIST       420
#define ID_DCG_TITLE        421
#define ID_DCG_SEARCH       422
#define ID_DCG_LIST         423
#define ID_DCG_APPLY        424
#define ID_DCG_CLOSE        425
#define ID_DCG_TOKEN        426
#define ID_DCG_STATUS       427
#define ID_DCG_HELP         428
#define ID_DCG_OPEN_URL     429
#define ID_DCG_ARTWORK      430
#define ID_DCG_FILLALL      431

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
#define IDT_RADIO_META      6   // poll ICY metadata for radio streams
#define IDT_RADIO_DEBOUNCE  7   // auto-search after typing pause
#define IDT_RADIO_RECONN    8   // reconnect countdown tick (1s)
#define IDT_LOUD_HINT       9   // auto-hide the first-time Loud hint balloon
#define IDC_RADIO_FAV       448
#define IDC_RADIO_UNFAV     449
#define IDC_RADIO_PRESET_BASE 460  // 460..479 = genre preset buttons
#define IDT_STATUS_SCROLL   5   // status bar text scroll
#define WM_PLAYNEXT         (WM_APP + 1)
#define WM_TRAYICON         (WM_APP + 2)
#define WM_CONV_PROGRESS    (WM_APP + 3)   // wParam=files done, lParam=MAKELPARAM(fileIdx,total)
#define WM_CONV_DONE        (WM_APP + 4)   // wParam=ok count, lParam=total
#define WM_RADIO_CONNECTED  (WM_APP + 5)   // wParam=HSTREAM (0 on failure)
#define WM_THUMB_READY      (WM_APP + 6)   // wParam=ThumbUrlJob* (URL artwork fetched)
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
#define IDM_SHAZAM          2350   // Shazam menu -> Song Recognition
#define IDM_SHAZAM_HISTORY  2351   // Shazam menu -> History
// Shazam dialog controls
#define ID_SHAZ_RESULT      760
#define ID_SHAZ_GOOGLE      761
#define ID_SHAZ_COPY        762
#define ID_SHAZ_CLOSE       763
#define ID_SHAZ_ARTWORK     764
#define ID_SHAZ_RETRY       765
#define ID_SHAZ_HISTORY     766
#define ID_SHAZ_HISTLBL     767
// View menu
#define IDM_VIEW_COLUMNS        2203
#define ID_COLUMNVIEW           131
#define IDM_VIEW_DARKMODE       2201
#define IDM_VIEW_MODERN         2202
// M3U playlist
#define IDM_FILE_OPENM3U        2005
#define IDM_FILE_SAVEM3U        2006
#define IDM_FILE_REFRESH        2007
#define IDM_FILE_OPENSTREAM     2008
#define IDM_FILE_PLAYHISTORY    2009
// Open Stream Link dialog
#define ID_STREAM_EDIT          720
#define ID_STREAM_OK            721
#define ID_STREAM_CANCEL        722
// New Playlist name prompt
#define ID_NAME_EDIT            730
#define ID_NAME_OK              731
#define ID_NAME_CANCEL          732
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
static const wchar_t APP_TITLE[] = L"BillyPro";
static const wchar_t APP_VERSION[] = L"0.9.1";

HWND g_hwnd = NULL;
HWND hListBox = NULL;
HWND hColumnHeader = NULL; // column header bar for column view
bool g_columnView = false; // column view enabled
bool g_colPathRelative = false; // show file path column relative to playlist root
bool g_rememberColumns = true; // persist column layout across sessions
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
wchar_t g_discogsToken[128] = L"";  // Discogs personal access token
bool    g_shuffle = false;
// Repeat mode: 0=off, 1=repeat track, 2=repeat playlist, 3=play once (stop after current)
int     g_repeatMode = 0;
bool    g_mono = false;
bool    g_normalize = false;
int     g_loudMode = 1;   // Loud button mode: 0=Soft 1=Normal 2=Modern 3=Loud
int     g_loudStreamStep = 2;  // stream loudness intensity 1..5 (right-click slider)
bool    g_loudHintShown = false;  // one-time hint balloon after first Loud click (ini-tracked)
// Makeup gain for the current track, applied inside DSP_Limiter.
// (BASS ignores the VOL attribute when reading decode streams, so the
// gain MUST be applied in the DSP on the master output stream.)
volatile float g_loudNormGain = 1.0f;
wchar_t g_iniPath[MAX_PATH] = L"";
bool    g_bassBoost = false;
// Remember session: 0=off, 1=playlist+track only, 2=playlist+track+seek position
int     g_rememberSession = 0;
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
HWND  hBackBtn      = NULL;
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
HDSP    g_dspLimit  = 0;   // Loud output limiter
HDSP    g_dspGlobalLimit = 0;   // always-on master brickwall limiter (option)
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

// ── Customizable keyboard shortcuts ─────────────────────────
// Modifier bits: 1=Shift, 2=Ctrl, 4=Alt.  vk=0 means unbound.
enum HotkeyAction {
    HK_PLAYPAUSE, HK_STOP, HK_NEXT, HK_PREV, HK_SEARCH,
    HK_SHUFFLE, HK_REPEAT, HK_MONO, HK_BASSBOOST, HK_NORMALIZE,
    HK_INFO, HK_REFRESH,
    HK_COUNT
};
struct HotkeyDef {
    const wchar_t* name;      // display name in Options
    BYTE defVk; BYTE defMods; // factory default
    BYTE vk;    BYTE mods;    // current binding
};
static HotkeyDef g_hotkeys[HK_COUNT] = {
    { L"Play / Pause",      VK_SPACE, 0, VK_SPACE, 0 },
    { L"Stop",              'Q', 0, 'Q', 0 },
    { L"Next Track",        'N', 0, 'N', 0 },
    { L"Previous Track",    'P', 0, 'P', 0 },
    { L"Find / Search",     'F', 0, 'F', 0 },
    { L"Toggle Shuffle",    'S', 0, 'S', 0 },
    { L"Cycle Repeat Mode", 'R', 0, 'R', 0 },
    { L"Toggle Mono",       'M', 0, 'M', 0 },
    { L"Toggle Bass Boost", 'B', 0, 'B', 0 },
    { L"Toggle Normalize",  'L', 0, 'L', 0 },
    { L"Audio Information", 'I', 0, 'I', 0 },
    { L"Refresh Playlist",  'R', 1, 'R', 1 },   // Shift+R
};

Theme   g_theme = LIGHT;

static WNDPROC g_OldListProc   = NULL;
static WNDPROC g_OldBtnProc    = NULL;
static WNDPROC g_OldStatusProc = NULL;
static HWND    g_hoveredBtn = NULL;

static WNDPROC g_OldSearchProc = NULL;

// path is oversized (not MAX_PATH) so long stream URLs — e.g. Sets.LIVE
// "…/stream.php?id=<encoded path>&token=<64 chars>" — aren't truncated.
struct Track { wchar_t path[1024]; wchar_t display[MAX_PATH]; };
std::vector<Track> g_playlist;
std::vector<Track> g_savedPlaylist;  // saved when media view is active
wchar_t g_playlistRoot[MAX_PATH] = L"";  // folder the playlist was loaded from (refresh / relative paths)
std::vector<int>   g_shuffleOrder;
struct UndoRemove { Track t; int idx; };
std::vector<UndoRemove> g_undoRemove;  // last batch of removed tracks (for Ctrl+Z)
int g_currentIndex = -1;

struct MediaBrowserItem { wchar_t path[MAX_PATH]; wchar_t display[MAX_PATH]; bool isDir; };
std::vector<MediaBrowserItem> g_browserItems;
bool   g_browserActive = false;
bool   g_browserReturn = false;  // true = came from browser, show ← to go back
wchar_t g_browserPath[MAX_PATH] = {};
wchar_t g_browserReturnPath[MAX_PATH] = {};  // saved path to return to

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
bool   g_modernStyle  = false;  // modern style: artwork thumbnails in playlist
int    g_modernSize   = 0;      // 0=small(32), 1=medium(48), 2=large(64)
// Recording state
static bool    g_recording = false;
static HDSP    g_recDsp = 0;
static wchar_t g_recPath[MAX_PATH] = L"";
static wchar_t g_recSaveDir[MAX_PATH] = L"";  // default recording save directory
static volatile ULONGLONG g_recBytesIn = 0;    // PCM bytes fed to encoder (for size estimate)
static FILE*   g_recLog = NULL;                // .txt log file for radio recording
static int     g_recFormat = 0;                // 0=FLAC, 1=WAV
static FILE*   g_recWavFile = NULL;            // WAV file handle when recording to WAV
static DWORD   g_recWavDataBytes = 0;          // total data bytes written (for header fixup)
static HWND    hRecordBtn = NULL;

// Internet Radio state (used by StopAudio/UpdateStatusBar before radio code is defined)
static bool g_radioPlaying = false;
static HSTREAM g_radioStream = 0;
static wchar_t g_radioStationName[256] = L"";
static wchar_t g_radioNowPlaying[512] = L"";
static ULONGLONG g_radioPlayStart = 0;  // tick when radio playback started (for elapsed time)
static int    g_radioReconnAttempt = 0;   // reconnect attempt counter for backoff
static int    g_radioReconnCountdown = 0; // seconds remaining before next retry
static wchar_t g_radioReconnUrl[1024] = L""; // saved URL for reconnect (long enough for tokenized Sets.LIVE URLs)
static volatile LONG g_radioConnGen = 0;  // connect generation token — stale worker results are discarded
bool   g_multiInst    = false;   // allow multiple instances (default: replace session)
bool   g_dropAppend   = true;    // drag & drop appends to playlist (false = replace)
bool   g_dropLoadDir  = false;   // when dropping a file, load its entire parent folder
bool   g_dropLoadTree = false;  // when dropping a folder, load entire directory tree recursively
bool   g_scrollToPlaying = false; // scroll playlist to the playing track on track change
bool   g_shazAllowLocal  = false; // allow Song Search (Shazam) on local files, not just streams
bool   g_globalLimiter   = true;  // master output limiter (prevents clipping even at max Loud)
// Sets.LIVE integration (browse/stream a remote liveset library via a PHP API)
bool    g_setsEnabled = false;
wchar_t g_setsServer[128] = L"app.sets.live";  // host (no scheme)
wchar_t g_setsToken[160]  = L"";               // per-user bearer token
wchar_t g_setsExpiry[32]  = L"";               // "YYYY-MM-DD" from server (display only)
wchar_t g_setsDownloadDir[MAX_PATH] = L"";     // where "Download" saves (asked once if empty)
bool         g_setsReturnActive = false;       // main list holds a Sets folder -> show Back
std::wstring g_setsReturnPath;                 // Sets folder to return to via Back
HWND   g_hwndAssoc    = NULL;    // file associations dialog
HWND   g_hwndOptions  = NULL;    // options dialog

// Taskbar thumbnail toolbar (play/pause/skip controls)
static ITaskbarList3* g_pTaskbar          = NULL;
static HICON          g_thumbIcons[4]     = {};  // 0=prev, 1=play, 2=pause, 3=next
static UINT           g_WM_TASKBARBUTTONCREATED = 0;
#define THUMB_BTN_PREV   0
#define THUMB_BTN_PLAY   1
#define THUMB_BTN_NEXT   2

// SMTC (System Media Transport Controls)
static winrt::Windows::Media::SystemMediaTransportControls g_smtc{ nullptr };
static winrt::event_token g_smtcToken{};

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
static void RadioConnectAsync(const wchar_t* url);
static void RadioPlayFavOffset(int dir);

// libFLAC decoder struct + globals (implementation after DTS section)
#include <FLAC/stream_decoder.h>
#include <FLAC/metadata.h>
#include <FLAC/stream_encoder.h>
struct FlacDecoder {
    FLAC__StreamDecoder* dec;
    int      sampleRate;
    int      nChannels;      // source channels from STREAMINFO (may be >2)
    int      outChannels;    // channels actually emitted (clamped to 2)
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
static wchar_t g_stRightBase[256] = L"";  // right panel text without buffer info (for live refresh)
void TogglePlayPause();
void PlayNext();
void PlayPrev();
void SeekToSeconds(double sec);
void UpdateVolume();
void LayoutControls(HWND hwnd);
static double GetPlayPos();
static void SyncColumnView();
static void RebuildColumnHeader();
static void SaveColumnOrderFromHeader();
// Column View definitions (used by SaveSettings/LoadSettings)
enum ColID {
    COL_NAME, COL_EXT, COL_DURATION, COL_PATH, COL_SIZE,
    COL_ARTIST, COL_TITLE, COL_ALBUM, COL_TRACK, COL_YEAR,
    COL_GENRE, COL_BITRATE, COL_SAMPLERATE, COL_CHANNELS,
    COL_COUNT
};
static bool g_colVisible[COL_COUNT] = {
    true, true, true, false, false,       // name, ext, dur, path, size
    false, false, false, false, false,    // artist, title, album, track#, year
    false, false, false, false            // genre, bitrate, samplerate, channels
};
static int  g_colOrder[COL_COUNT];  // initialized in code
static int  g_colWidth[COL_COUNT] = {
    300, 60, 65, 200, 80,
    150, 200, 150, 45, 45,
    80, 65, 65, 55
};
static void InitColOrder() { for (int i = 0; i < COL_COUNT; i++) g_colOrder[i] = i; }
static int  g_sortCol = -1;
static bool g_sortAsc = true;
static int  g_hdrHotItem = -1;
static std::vector<Track> g_unsortedPlaylist;
void PreloadNext();
void SaveSettings();

void ApplyTheme();
static void MenuInitOwnerDraw(HMENU hm);
void RebuildShuffleOrder();
void UpdateWindowTitle();
static void RefreshListboxStars();
static void SmtcInit(HWND hwnd);
static void SmtcUpdatePlaybackStatus();
static void SmtcUpdateMetadata();
static void SmtcCleanup();
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
    if (sec < 1.0 && sec > 0.0) {
        swprintf_s(out, n, L"0.%03d", (int)(sec * 1000));
        return;
    }
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

// Escape '&' for menu display (menus treat & as an accelerator prefix,
// so "D&B" would render as "DB" — it must become "D&&B")
static void MenuEscapeAmp(const wchar_t* in, wchar_t* out, int n)
{
    int o = 0;
    for (int i = 0; in[i] && o < n - 2; i++) {
        out[o++] = in[i];
        if (in[i] == L'&') out[o++] = L'&';
    }
    out[o] = 0;
}

// Friendly display name from a URL: last path segment, percent-decoded
// (".../A%20State%20Of%20Trance/asot-102.mp3" -> "asot-102.mp3")
static void UrlToDisplayName(const wchar_t* url, wchar_t* out, int n)
{
    // Prefer an "id=<encoded path>" query parameter (Sets.LIVE stream links):
    // its decoded last segment is the real filename, whereas the URL's path
    // segment would just be "stream.php".
    const wchar_t* idv = NULL;
    const wchar_t* qm = wcschr(url, L'?');
    if (qm) {
        for (const wchar_t* s = qm + 1; s && *s; ) {
            if (wcsncmp(s, L"id=", 3) == 0) { idv = s + 3; break; }
            s = wcschr(s, L'&');
            if (s) s++;
        }
    }
    wchar_t raw[512] = {};
    int ri = 0;
    if (idv) {
        for (const wchar_t* q = idv; *q && *q != L'&' && *q != L'#' && ri < 511; q++)
            raw[ri++] = *q;
    } else {
        const wchar_t* p = wcsstr(url, L"://");
        p = p ? p + 3 : url;
        const wchar_t* seg = p;
        for (const wchar_t* q = p; *q; q++)
            if (*q == L'/' && *(q + 1)) seg = q + 1;
        // Copy the segment up to any query/fragment
        for (const wchar_t* q = seg; *q && *q != L'?' && *q != L'#' && ri < 511; q++)
            raw[ri++] = *q;
    }
    if (!raw[0]) { wcsncpy_s(out, n, url, _TRUNCATE); return; }
    // Percent-decode into UTF-8 bytes, then convert to wide
    auto hexv = [](wchar_t c) -> int {
        if (c >= L'0' && c <= L'9') return c - L'0';
        if (c >= L'A' && c <= L'F') return c - L'A' + 10;
        if (c >= L'a' && c <= L'f') return c - L'a' + 10;
        return -1;
    };
    char utf8[1024];
    int ui = 0;
    for (int i = 0; raw[i] && ui < 1023; i++) {
        int h1, h2;
        if (raw[i] == L'%' && (h1 = hexv(raw[i+1])) >= 0 && (h2 = hexv(raw[i+2])) >= 0) {
            utf8[ui++] = (char)((h1 << 4) | h2);
            i += 2;
        } else {
            utf8[ui++] = (char)raw[i];  // URLs are ASCII outside %XX escapes
        }
    }
    utf8[ui] = 0;
    wchar_t decoded[512] = {};
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, decoded, 511);
    if (decoded[0]) {
        // The decoded id param is a relative path ("Livesets/.../file.mp3") —
        // show just the filename part.
        const wchar_t* base = decoded;
        if (idv) {
            const wchar_t* sl = wcsrchr(decoded, L'/');
            if (!sl) sl = wcsrchr(decoded, L'\\');
            if (sl && sl[1]) base = sl + 1;
        }
        wcsncpy_s(out, n, base, _TRUNCATE);
    }
    else wcsncpy_s(out, n, url, _TRUNCATE);
}

// A Sets.LIVE stream URL (points at the download/stream endpoint)
static bool IsSetsUrl(const wchar_t* p)
{
    return p && wcsstr(p, L"/api/stream.php") != NULL;
}

// Any URL-style track path (stream link / radio / Sets.LIVE)
static bool IsUrlPath(const wchar_t* p)
{
    return p && (_wcsnicmp(p, L"http://", 7) == 0 ||
                 _wcsnicmp(p, L"https://", 8) == 0 ||
                 _wcsnicmp(p, L"ftp://", 6) == 0);
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
static void LibResetViewState();  // defined in the Library section below

void ClearPlaylist()
{
    g_playlist.clear(); g_shuffleOrder.clear(); g_currentIndex = -1;
    g_unsortedPlaylist.clear(); g_sortCol = -1; g_sortAsc = true;
    g_playlistRoot[0] = L'\0';
    g_setsReturnActive = false;   // leaving any Sets.LIVE view
    // Loading something new means we left any Favorites/playlist library view —
    // clear it so the Library menu checkmark disappears.
    LibResetViewState();
    if (hListBox) SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
}

// Modern folder picker (IFileOpenDialog with FOS_PICKFOLDERS)
// The common item dialog is an STA-only COM object, but the main thread runs
// in an MTA (required by WinRT/SMTC). Showing it directly from the MTA thread
// deadlocks, so the dialog runs on its own STA thread while the calling
// thread pumps messages (the dialog SendMessages to its owner window).
struct PickFolderParams {
    HWND owner;
    const wchar_t* title;
    wchar_t* outPath;
    int maxLen;
    bool result;
};

static DWORD WINAPI PickFolderThreadProc(LPVOID param)
{
    PickFolderParams* p = (PickFolderParams*)param;
    p->result = false;
    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    IFileOpenDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_IFileOpenDialog, (void**)&pfd);
    if (SUCCEEDED(hr) && pfd) {
        DWORD opts = 0;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        if (p->title) pfd->SetTitle(p->title);
        if (SUCCEEDED(pfd->Show(p->owner))) {
            IShellItem* psi = NULL;
            if (SUCCEEDED(pfd->GetResult(&psi)) && psi) {
                PWSTR pszPath = NULL;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
                    wcsncpy_s(p->outPath, p->maxLen, pszPath, _TRUNCATE);
                    CoTaskMemFree(pszPath);
                    p->result = true;
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    if (SUCCEEDED(hrInit)) CoUninitialize();
    return 0;
}

// Returns true if user picked a folder, writes path to outPath
static bool PickFolder(HWND hwndOwner, const wchar_t* title, wchar_t* outPath, int maxLen)
{
    PickFolderParams p = { hwndOwner, title, outPath, maxLen, false };
    HANDLE hThread = CreateThread(NULL, 0, PickFolderThreadProc, &p, 0, NULL);
    if (!hThread) return false;
    // Pump messages while the dialog is up — it sends messages to the owner
    // window on this thread; blocking without pumping would deadlock.
    for (;;) {
        DWORD w = MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);
        if (w == WAIT_OBJECT_0 + 1) {
            MSG m;
            while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&m);
                DispatchMessage(&m);
            }
        } else {
            break; // thread finished (or wait failed)
        }
    }
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return p.result;
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
        // Inside a subfolder: go up to parent. At a top-level dir with no
        // parent separator (e.g. "C:"), fall back to the virtual root instead
        // of relisting the same folder forever ([..] self-loop).
        MediaBrowserItem up = {}; wcscpy_s(up.display, L"[..] (go back)"); up.isDir = true;
        wchar_t parent[MAX_PATH]; wcsncpy_s(parent, dir, _TRUNCATE);
        wchar_t* sl = wcsrchr(parent, L'\\');
        if (sl) *sl = L'\0';
        else parent[0] = L'\0';   // no parent -> virtual root (media folders list)
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
                if (hListBox) {
                    wchar_t disp[MAX_PATH + 4];
                    if (IsFavorite(full))
                        swprintf_s(disp, L"\u2605 %s", it.display);
                    else
                        wcsncpy_s(disp, it.display, _TRUNCATE);
                    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
                }
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
        if (g_hwnd) LayoutControls(g_hwnd); // update back button visibility
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
        // Switch to playlist view, but save browser path so ← can go back
        wcsncpy_s(g_browserReturnPath, g_browserPath, _TRUNCATE);
        g_browserReturn = true;
        g_browserActive = false;
        g_browserItems.clear();
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        RefreshListboxStars();
        if (g_hwnd) LayoutControls(g_hwnd);
        if (foundIdx >= 0) PlayIndex(foundIdx);
    }
}

// Map a playlist index to the corresponding browser listbox index (returns -1 if not found)
static int BrowserListboxIndex(int playlistIdx)
{
    if (!g_browserActive || playlistIdx < 0 || playlistIdx >= (int)g_playlist.size()) return -1;
    const wchar_t* path = g_playlist[playlistIdx].path;
    for (int i = 0; i < (int)g_browserItems.size(); i++) {
        if (!g_browserItems[i].isDir && _wcsicmp(g_browserItems[i].path, path) == 0)
            return i;
    }
    return -1;
}

// Update listbox selection for the playing track, works in both normal and browser mode
static void SelectPlayingTrack(int idx)
{
    if (!hListBox) return;
    if (g_browserActive) {
        int lbIdx = BrowserListboxIndex(idx);
        if (lbIdx >= 0) {
            SendMessage(hListBox, LB_SETCURSEL, lbIdx, 0);
        }
    } else {
        SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
        SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)idx);
        SendMessage(hListBox, LB_SETCURSEL, idx, 0);
        if (g_scrollToPlaying)
            SendMessage(hListBox, LB_SETTOPINDEX, max(0, idx - 3), 0);
    }
}

void AddTrack(const wchar_t* path)
{
    Track t; wcsncpy_s(t.path, path, _TRUNCATE);
    if (_wcsnicmp(path, L"http://", 7) == 0 || _wcsnicmp(path, L"https://", 8) == 0 ||
        _wcsnicmp(path, L"ftp://", 6) == 0)
        UrlToDisplayName(path, t.display, _countof(t.display));
    else
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

static std::mt19937 g_rng(std::random_device{}());

void RebuildShuffleOrder()
{
    g_shuffleOrder.resize(g_playlist.size());
    for (size_t i = 0; i < g_shuffleOrder.size(); ++i) g_shuffleOrder[i] = (int)i;
    for (size_t i = g_shuffleOrder.size(); i > 1; --i) {
        std::uniform_int_distribution<size_t> dist(0, i - 1);
        std::swap(g_shuffleOrder[i - 1], g_shuffleOrder[dist(g_rng)]);
    }
}

void LoadFolder(const wchar_t* folder)
{
    ClearPlaylist();
    wcsncpy_s(g_playlistRoot, folder, _TRUNCATE);
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
    if (g_columnView) SyncColumnView();
    UpdateStatusBar();
}

// ============================================================
//  DSP: Mono mix-down
// ============================================================
void CALLBACK DSP_Mono(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    // Get actual channel count from the stream
    BASS_CHANNELINFO ci = {};
    BASS_ChannelGetInfo(channel, &ci);
    int nch = ci.chans;
    if (nch < 2) return; // already mono, nothing to do

    float* buf = (float*)buffer;
    DWORD floats = length / sizeof(float);
    // Mix all channels to mono, write back to all channels
    for (DWORD i = 0; i + (DWORD)nch <= floats; i += nch) {
        float m = 0;
        for (int c = 0; c < nch; c++) m += buf[i + c];
        m /= nch;
        for (int c = 0; c < nch; c++) buf[i + c] = m;
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
    BASS_CHANNELINFO ci = {}; BASS_ChannelGetInfo(channel, &ci);
    float sr = ci.freq > 0 ? (float)ci.freq : 44100.0f;
    int nch = ci.chans > 0 ? ci.chans : 2;
    float alpha = expf(-6.28318f * 120.0f / sr);
    float bassGain = powf(10.0f, g_bbGainDB / 20.0f) - 1.0f;
    const float limAttack = 0.001f;
    const float limRelease = 0.00001f;
    const float limThresh = 0.95f;

    for (DWORD i = 0; i + (DWORD)nch <= floats; i += nch) {
        // Process L (ch0) and R (ch1 if present), pass through extra channels
        s_lpL = alpha * s_lpL + (1.0f - alpha) * buf[i];
        float outL = buf[i] + s_lpL * bassGain;
        float outR = outL;
        if (nch >= 2) {
            s_lpR = alpha * s_lpR + (1.0f - alpha) * buf[i + 1];
            outR = buf[i + 1] + s_lpR * bassGain;
        }
        float peak = max(fabsf(outL), nch >= 2 ? fabsf(outR) : 0.0f);
        if (peak * s_limGain > limThresh && peak > 0.0f) {
            float targetGain = limThresh / peak;
            s_limGain += (targetGain - s_limGain) * limAttack;
            if (s_limGain > targetGain) s_limGain = targetGain;
        } else {
            s_limGain += (1.0f - s_limGain) * limRelease;
            if (s_limGain > 1.0f) s_limGain = 1.0f;
        }
        buf[i] = outL * s_limGain;
        if (nch >= 2) buf[i + 1] = outR * s_limGain;
        // Extra channels (surround) get limiter gain only
        for (int c = 2; c < nch; c++) buf[i + c] *= s_limGain;
    }
}

// DSP: output processor for the Loud button — FabFilter Pro-L style:
// a fixed input gain pushed into a transparent peak limiter (0 dB ceiling).
//  - Local files: per-track makeup gain (g_loudNormGain, pre-scanned).
//  - Streams (radio / direct links): gain from the 5-step slider
//    (right-click the Loud button while streaming). No auto-gain.
static float s_loudEnv    = 1.0f;   // limiter envelope
static bool  s_loudStream = false;  // set by ApplyDSP: URL/radio playback

void CALLBACK DSP_Limiter(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf = (float*)buffer;
    DWORD n = length / sizeof(float);
    const float thresh  = 0.98f;
    const float attack  = 0.35f;
    const float release = 0.0004f;

    // Input gain: slider steps 1..5 = +2/+4/+6/+9/+12 dB for streams,
    // pre-scanned per-track gain for local files.
    static const float STRM_GAIN[5] = { 1.26f, 1.59f, 2.0f, 2.82f, 4.0f };
    float gain;
    if (s_loudStream) {
        int st = g_loudStreamStep;
        if (st < 1) st = 1; if (st > 5) st = 5;
        gain = STRM_GAIN[st - 1];
    } else {
        gain = g_loudNormGain;
    }

    for (DWORD i = 0; i < n; i++) {
        float s = buf[i] * gain;
        float a = fabsf(s);
        if (a * s_loudEnv > thresh && a > 0.0f) {
            float target = thresh / a;
            s_loudEnv += (target - s_loudEnv) * attack;
            if (s_loudEnv > target) s_loudEnv = target;
        } else {
            s_loudEnv += (1.0f - s_loudEnv) * release;
            if (s_loudEnv > 1.0f) s_loudEnv = 1.0f;
        }
        buf[i] = s * s_loudEnv;
    }
}

// Master brickwall limiter — always-on safety net (Settings → Audio) that
// runs LAST in the chain to stop clipping/distortion from any source
// (Loud pushed to max, Bass Boost, effects). No gain, just a hard ceiling.
static float s_glEnv = 1.0f;
void CALLBACK DSP_GlobalLimiter(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf = (float*)buffer;
    DWORD n = length / sizeof(float);
    const float thresh  = 0.985f;
    const float attack  = 0.4f;
    const float release = 0.0006f;
    for (DWORD i = 0; i < n; i++) {
        float a = fabsf(buf[i]);
        if (a * s_glEnv > thresh && a > 0.0f) {
            float target = thresh / a;
            s_glEnv += (target - s_glEnv) * attack;
            if (s_glEnv > target) s_glEnv = target;
        } else {
            s_glEnv += (1.0f - s_glEnv) * release;
            if (s_glEnv > 1.0f) s_glEnv = 1.0f;
        }
        buf[i] *= s_glEnv;
        // Absolute hard clip as a final guarantee
        if (buf[i] > 1.0f) buf[i] = 1.0f;
        if (buf[i] < -1.0f) buf[i] = -1.0f;
    }
}

// ============================================================
//  Extra DSP effects
// ============================================================

// --- Reverb (4 comb + 2 all-pass per channel, Freeverb stereo) ---
// Base delay lengths at 44100 Hz — scaled at runtime for other sample rates
static const int REV_COMB_BASE_L[4] = { 1557, 1617, 1491, 1422 };
static const int REV_COMB_BASE_R[4] = { 1580, 1640, 1514, 1445 }; // +23 Freeverb offset
static const int REV_AP_BASE_L[2]   = { 225, 341 };
static const int REV_AP_BASE_R[2]   = { 236, 352 };                // +11 offset
// Buffers sized for up to 192 kHz (max scale ~4.35x of base lengths)
static float g_revCombBufL[4][7200]={}, g_revCombBufR[4][7200]={};
static float g_revApBufL[2][1600]={},   g_revApBufR[2][1600]={};
static int   g_revCombPosL[4]={}, g_revCombPosR[4]={};
static int   g_revApPosL[2]={},   g_revApPosR[2]={};

void CALLBACK DSP_Reverb(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    float* buf  = (float*)buffer;
    DWORD  n    = length / sizeof(float);
    BASS_CHANNELINFO ci = {}; BASS_ChannelGetInfo(channel, &ci);
    float srScale = ci.freq > 0 ? (float)ci.freq / 44100.0f : 1.0f;
    int nch = ci.chans > 0 ? ci.chans : 2;

    // Scale delay lengths by sample rate, clamped to buffer sizes
    int combL[4], combR[4], apL[2], apR[2];
    for (int c = 0; c < 4; c++) {
        combL[c] = min((int)(REV_COMB_BASE_L[c] * srScale), 7199);
        combR[c] = min((int)(REV_COMB_BASE_R[c] * srScale), 7199);
    }
    for (int a = 0; a < 2; a++) {
        apL[a] = min((int)(REV_AP_BASE_L[a] * srScale), 1599);
        apR[a] = min((int)(REV_AP_BASE_R[a] * srScale), 1599);
    }

    float wet    = g_revMix   / 100.0f;
    float fb     = 0.50f + (g_revRoom  / 100.0f) * 0.46f;
    float wscale = g_revWidth / 200.0f;
    float dry    = 1.0f - wet;
    for (DWORD i = 0; i + (DWORD)nch <= n; i += nch) {
        float inL = buf[i], inR = (nch >= 2) ? buf[i + 1] : buf[i];
        float outL = 0, outR = 0;
        for (int c = 0; c < 4; c++) {
            float dL = g_revCombBufL[c][g_revCombPosL[c]];
            float dR = g_revCombBufR[c][g_revCombPosR[c]];
            g_revCombBufL[c][g_revCombPosL[c]] = inL + dL * fb;
            g_revCombBufR[c][g_revCombPosR[c]] = inR + dR * fb;
            if (++g_revCombPosL[c] >= combL[c]) g_revCombPosL[c] = 0;
            if (++g_revCombPosR[c] >= combR[c]) g_revCombPosR[c] = 0;
            outL += dL; outR += dR;
        }
        outL *= 0.25f; outR *= 0.25f;
        for (int a = 0; a < 2; a++) {
            float dL = g_revApBufL[a][g_revApPosL[a]];
            float dR = g_revApBufR[a][g_revApPosR[a]];
            g_revApBufL[a][g_revApPosL[a]] = outL + dL * 0.5f;
            g_revApBufR[a][g_revApPosR[a]] = outR + dR * 0.5f;
            if (++g_revApPosL[a] >= apL[a]) g_revApPosL[a] = 0;
            if (++g_revApPosR[a] >= apR[a]) g_revApPosR[a] = 0;
            outL = dL - outL * 0.5f;
            outR = dR - outR * 0.5f;
        }
        float wL = outL * (0.5f + wscale) + outR * (0.5f - wscale);
        float wR = outR * (0.5f + wscale) + outL * (0.5f - wscale);
        buf[i] = dry * inL + wet * wL;
        if (nch >= 2) buf[i + 1] = dry * inR + wet * wR;
        // Extra channels pass through unchanged
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
    float wet   = (g_satLevel / 100.0f) * 0.35f;
    float dry   = 1.0f - wet;
    BASS_CHANNELINFO ci = {}; BASS_ChannelGetInfo(channel, &ci);
    float sr = ci.freq > 0 ? (float)ci.freq : 44100.0f;
    int nch = ci.chans > 0 ? ci.chans : 2;
    const float lpA = expf(-6.28318f * 200.0f / sr);
    for (DWORD i = 0; i + (DWORD)nch <= n; i += nch) {
        float inL = buf[i], inR = (nch >= 2) ? buf[i + 1] : buf[i];
        g_satLpL = lpA * g_satLpL + (1.0f - lpA) * inL;
        float loL = g_satLpL;
        float hiL = inL - loL;
        float satHiL = hiL * drive; satHiL = satHiL / (1.0f + fabsf(satHiL));
        float satLoL = loL * drive; satLoL = satLoL / (1.0f + fabsf(satLoL));
        buf[i] = (hiL * dry + satHiL * wet) + (loL * 0.95f + satLoL * 0.05f);
        if (nch >= 2) {
            g_satLpR = lpA * g_satLpR + (1.0f - lpA) * inR;
            float loR = g_satLpR, hiR = inR - loR;
            float satHiR = hiR * drive; satHiR = satHiR / (1.0f + fabsf(satHiR));
            float satLoR = loR * drive; satLoR = satLoR / (1.0f + fabsf(satLoR));
            buf[i + 1] = (hiR * dry + satHiR * wet) + (loR * 0.95f + satLoR * 0.05f);
        }
        // Extra channels pass through unchanged
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

    BASS_CHANNELINFO ci = {}; BASS_ChannelGetInfo(channel, &ci);
    float sr = ci.freq > 0 ? (float)ci.freq : 44100.0f;

    float freq       = max(500.0f, min(20000.0f, g_vinLpFreq));
    float alpha      = expf(-6.28318f * freq / sr);
    float crackScale = g_vinCrackle / 100.0f;
    // Surface hiss amplitude: subtle baseline + crackle-scaled component
    float hissAmp    = 0.0003f + crackScale * 0.0018f;
    // Hiss LP alpha: ~5 kHz rolloff for warm surface noise character
    const float hA   = expf(-6.28318f * 5000.0f / sr);

    int nch = ci.chans > 0 ? ci.chans : 2;
    for (DWORD i = 0; i + (DWORD)nch <= n; i += nch) {
        // HF roll-off on signal
        g_vinLpL = alpha * g_vinLpL + (1.0f - alpha) * buf[i];
        buf[i] = g_vinLpL;
        if (nch >= 2) {
            g_vinLpR = alpha * g_vinLpR + (1.0f - alpha) * buf[i + 1];
            buf[i + 1] = g_vinLpR;
        }

        // ── M-S stereo field (stereo only; mono skips side processing) ───
        float mid, side;
        if (nch >= 2) {
            mid  = (buf[i] + buf[i + 1]) * 0.5f;
            side = (buf[i] - buf[i + 1]) * 0.5f;
        } else {
            mid = buf[i];
            side = 0;
        }

        // Gentle saturation on mid (warm mono core, like a slightly overdriven tube)
        mid = mid / (1.0f + fabsf(mid) * 0.5f) * 1.08f;

        // Barely noticeable pitch wobble on mid — slow warble like an old turntable
        g_vinMidBuf[g_vinMidPos] = mid;
        float midLfo = sinf(g_vinMidLFO * 6.28318f);
        g_vinMidLFO += 0.6f / sr;    // 0.6 Hz — slow wow, more noticeable
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
        g_vinChorusLFO += 0.35f / sr;   // 0.35 Hz LFO rate
        if (g_vinChorusLFO >= 1.0f) g_vinChorusLFO -= 1.0f;
        float delaySmp = 485.0f + lfo * 88.0f;  // ~11ms base ± 2ms depth
        int   d0   = (int)delaySmp;
        float frac = delaySmp - (float)d0;
        float s0   = g_vinChorusBuf[(g_vinChorusPos - d0 + 1024) & 1023];
        float s1   = g_vinChorusBuf[(g_vinChorusPos - d0 - 1 + 1024) & 1023];
        float chorSide = s0 + frac * (s1 - s0);
        g_vinChorusPos = (g_vinChorusPos + 1) & 1023;
        side = side * 0.90f + chorSide * 0.10f;  // 10% wet

        // Reconstruct L/R (or write mono)
        buf[i] = mid + side;
        if (nch >= 2) buf[i + 1] = mid - side;

        if (crackScale > 0.0f) {
            // Surface hiss: independent L/R noise through warm LP
            float nL = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            g_vinHissL = hA * g_vinHissL + (1.0f - hA) * nL;
            buf[i] += g_vinHissL * hissAmp;
            if (nch >= 2) {
                float nR = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                g_vinHissR = hA * g_vinHissR + (1.0f - hA) * nR;
                buf[i + 1] += g_vinHissR * hissAmp;
            }

            // Crackle/pop envelope decay
            g_vinCrackEnvL *= g_vinCrackDecay;
            g_vinCrackEnvR *= g_vinCrackDecay;

            // Apply crackle with warm soft-clip (removes harsh transient edge)
            float cL = g_vinCrackEnvL, cR = g_vinCrackEnvR;
            buf[i] += cL / (1.0f + fabsf(cL) * 6.0f);
            if (nch >= 2) buf[i + 1] += cR / (1.0f + fabsf(cR) * 6.0f);

            // Schedule next crackle event
            if (g_vinCrackTimer == 0) {
                // Interval: sparse at low crackle, denser at high — with wide randomness
                DWORD baseInt = (DWORD)(sr * 1.8f / max(0.01f, crackScale));
                g_vinCrackTimer = baseInt / 3 + (DWORD)(rand() % (baseInt * 2 + 1));

                // Amplitude varies per pop
                float amp = crackScale * (0.03f + (rand() % 90) * 0.001f);

                // Decay time: 20-100ms so crackles fade naturally, not click
                int decaySmp = (int)(sr * 0.02f) + rand() % (int)(sr * 0.08f + 1);  // 20ms to 100ms
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
    BASS_CHANNELINFO ci = {}; BASS_ChannelGetInfo(channel, &ci);
    float sr = ci.freq > 0 ? (float)ci.freq : 44100.0f;
    float alpha = expf(-6.28318f * 80.0f / sr);   // 1-pole LP ~80 Hz shelf
    int nch = ci.chans > 0 ? ci.chans : 2;
    float bassAdd = (powf(10.0f, g_hfiBassDb / 20.0f) - 1.0f);
    float warmth  = (g_hfiWarmth / 100.0f) * 0.8f;
    for (DWORD i = 0; i + (DWORD)nch <= n; i += nch) {
        float inL = buf[i];
        g_hfiBssL = alpha * g_hfiBssL + (1.0f - alpha) * inL;
        float outL = inL + g_hfiBssL * bassAdd;
        buf[i] = warmth > 0.0f ? outL / (1.0f + fabsf(outL) * warmth) : outL;
        if (nch >= 2) {
            float inR = buf[i + 1];
            g_hfiBssR = alpha * g_hfiBssR + (1.0f - alpha) * inR;
            float outR = inR + g_hfiBssR * bassAdd;
            buf[i + 1] = warmth > 0.0f ? outR / (1.0f + fabsf(outR) * warmth) : outR;
        }
        // Extra channels pass through unchanged
    }
}

// --- Recording: real-time FLAC encoding via libFLAC stream encoder ---
static FLAC__StreamEncoder* g_recEncoder = NULL;
// Pre-allocated conversion buffer — avoids heap allocation on every DSP callback
static FLAC__int32* g_recPcmBuf = NULL;
static DWORD        g_recPcmCap = 0;  // capacity in FLAC__int32 elements

void CALLBACK DSP_Record(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    if (!g_recording || length == 0) return;
    float* buf = (float*)buffer;
    DWORD floats = length / sizeof(float);
    BASS_CHANNELINFO ci = {};
    BASS_ChannelGetInfo(channel, &ci);
    int nch = ci.chans > 0 ? ci.chans : 2;
    DWORD frames = floats / nch;

    if (g_recFormat == 1 && g_recWavFile) {
        // WAV: write 16-bit PCM directly
        for (DWORD i = 0; i < floats; i++) {
            float s = buf[i];
            if (s > 1.0f) s = 1.0f; if (s < -1.0f) s = -1.0f;
            short sample = (short)(s * 32767.0f);
            fwrite(&sample, 2, 1, g_recWavFile);
        }
        g_recWavDataBytes += floats * 2;
        g_recBytesIn += (ULONGLONG)frames * nch * 2;
    } else if (g_recEncoder) {
        // FLAC
        if (floats > g_recPcmCap) {
            free(g_recPcmBuf);
            g_recPcmCap = floats + 1024;
            g_recPcmBuf = (FLAC__int32*)malloc(g_recPcmCap * sizeof(FLAC__int32));
            if (!g_recPcmBuf) { g_recPcmCap = 0; return; }
        }
        for (DWORD i = 0; i < floats; i++) {
            float s = buf[i];
            if (s > 1.0f) s = 1.0f; if (s < -1.0f) s = -1.0f;
            g_recPcmBuf[i] = (FLAC__int32)(s * 32767.0f);
        }
        FLAC__stream_encoder_process_interleaved(g_recEncoder, g_recPcmBuf, frames);
        g_recBytesIn += (ULONGLONG)frames * nch * 2;
    }
}

static void StartRecording()
{
    if (g_recording || !currentStream) return;

    // Build filename: name_YYYYMMDD_HHMMSS.flac
    SYSTEMTIME st; GetLocalTime(&st);
    wchar_t nameBase[256] = L"recording";
    if (g_radioPlaying && g_radioStationName[0])
        wcsncpy_s(nameBase, g_radioStationName, _TRUNCATE);
    else if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
        wcsncpy_s(nameBase, g_playlist[g_currentIndex].display, _TRUNCATE);
        wchar_t* dot = wcsrchr(nameBase, L'.'); if (dot) *dot = 0;
    }
    // Sanitize
    for (wchar_t* p = nameBase; *p; p++)
        if (*p==L'\\'||*p==L'/'||*p==L':'||*p==L'*'||*p==L'?'||*p==L'"'||*p==L'<'||*p==L'>'||*p==L'|') *p=L'_';

    const wchar_t* recExt = (g_recFormat == 1) ? L"wav" : L"flac";
    wchar_t fileName[MAX_PATH];
    _snwprintf_s(fileName, MAX_PATH, _TRUNCATE, L"%s_%04d%02d%02d_%02d%02d%02d.%s",
        nameBase, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, recExt);

    wchar_t savePath[MAX_PATH];
    if (g_recSaveDir[0] && GetFileAttributesW(g_recSaveDir) != INVALID_FILE_ATTRIBUTES) {
        _snwprintf_s(savePath, MAX_PATH, _TRUNCATE, L"%s\\%s", g_recSaveDir, fileName);
    } else {
        wcsncpy_s(savePath, fileName, _TRUNCATE);
        OPENFILENAME ofn = { sizeof(ofn) };
        ofn.hwndOwner = g_hwnd; ofn.lpstrFile = savePath; ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = (g_recFormat == 1) ? L"WAV Audio\0*.wav\0All Files\0*.*\0"
                                             : L"FLAC Audio\0*.flac\0All Files\0*.*\0";
        ofn.lpstrDefExt = recExt; ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        if (!GetSaveFileName(&ofn)) return;
        wcsncpy_s(g_recSaveDir, savePath, _TRUNCATE);
        wchar_t* lastSep = wcsrchr(g_recSaveDir, L'\\');
        if (lastSep) *lastSep = L'\0';
        SaveSettings();
    }

    BASS_CHANNELINFO ci = {};
    BASS_ChannelGetInfo(currentStream, &ci);
    int nch = ci.chans > 0 ? ci.chans : 2;
    int sr = ci.freq > 0 ? ci.freq : 44100;

    if (g_recFormat == 1) {
        // WAV recording: write header, fill data size on stop
        g_recWavFile = _wfopen(savePath, L"wb");
        if (!g_recWavFile) { MessageBox(g_hwnd, L"Failed to create WAV file.", L"Record", MB_ICONERROR); return; }
        WORD bitsPerSample = 16;
        DWORD byteRate = sr * nch * (bitsPerSample / 8);
        WORD blockAlign = nch * (bitsPerSample / 8);
        WORD fmtTag = 1;
        fwrite("RIFF", 1, 4, g_recWavFile);
        DWORD placeholder = 0; fwrite(&placeholder, 4, 1, g_recWavFile);
        fwrite("WAVE", 1, 4, g_recWavFile);
        fwrite("fmt ", 1, 4, g_recWavFile);
        DWORD fmtSize = 16; fwrite(&fmtSize, 4, 1, g_recWavFile);
        fwrite(&fmtTag, 2, 1, g_recWavFile);
        WORD wNch = (WORD)nch; fwrite(&wNch, 2, 1, g_recWavFile);
        DWORD dwSr = (DWORD)sr; fwrite(&dwSr, 4, 1, g_recWavFile);
        fwrite(&byteRate, 4, 1, g_recWavFile);
        fwrite(&blockAlign, 2, 1, g_recWavFile);
        fwrite(&bitsPerSample, 2, 1, g_recWavFile);
        fwrite("data", 1, 4, g_recWavFile);
        fwrite(&placeholder, 4, 1, g_recWavFile);
        g_recWavDataBytes = 0;
    } else {
        // FLAC recording
        g_recEncoder = FLAC__stream_encoder_new();
        if (!g_recEncoder) { MessageBox(g_hwnd, L"Failed to create FLAC encoder.", L"Record", MB_ICONERROR); return; }
        FLAC__stream_encoder_set_channels(g_recEncoder, nch);
        FLAC__stream_encoder_set_bits_per_sample(g_recEncoder, 16);
        FLAC__stream_encoder_set_sample_rate(g_recEncoder, sr);
        FLAC__stream_encoder_set_compression_level(g_recEncoder, 5);
        int uLen = WideCharToMultiByte(CP_UTF8, 0, savePath, -1, NULL, 0, NULL, NULL);
        std::vector<char> u8buf(uLen > 0 ? uLen : 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, savePath, -1, u8buf.data(), uLen, NULL, NULL);
        FLAC__StreamEncoderInitStatus initStatus = FLAC__stream_encoder_init_file(
            g_recEncoder, u8buf.data(), NULL, NULL);
        if (initStatus != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            FLAC__stream_encoder_delete(g_recEncoder); g_recEncoder = NULL;
            MessageBox(g_hwnd, L"Failed to init FLAC encoder.", L"Record", MB_ICONERROR);
            return;
        }
    }

    wcsncpy_s(g_recPath, savePath, _TRUNCATE);
    g_recBytesIn = 0;
    g_recDsp = BASS_ChannelSetDSP(currentStream, DSP_Record, NULL, 10);
    g_recording = true;
    // Open .txt log file for radio recordings
    if (g_radioPlaying) {
        wchar_t logPath[MAX_PATH];
        wcsncpy_s(logPath, savePath, _TRUNCATE);
        wchar_t* dot = wcsrchr(logPath, L'.');
        if (dot) wcscpy_s(dot, MAX_PATH - (dot - logPath), L".txt");
        else wcscat_s(logPath, L".txt");
        g_recLog = _wfopen(logPath, L"w,ccs=UTF-8");
        if (g_recLog) {
            SYSTEMTIME st; GetLocalTime(&st);
            fwprintf(g_recLog, L"Recording started: %04d-%02d-%02d %02d:%02d:%02d\n",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
            fwprintf(g_recLog, L"Station: %s\n", g_radioStationName);
            if (g_radioNowPlaying[0])
                fwprintf(g_recLog, L"[%02d:%02d:%02d] %s\n", st.wHour, st.wMinute, st.wSecond, g_radioNowPlaying);
            fwprintf(g_recLog, L"---\n");
            fflush(g_recLog);
        }
    }
    if (hRecordBtn) InvalidateRect(hRecordBtn, NULL, TRUE);
    // Trigger re-layout so remaining-time label gets wider for rec size display
    if (g_hwnd) { RECT wr; GetClientRect(g_hwnd, &wr); SendMessage(g_hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(wr.right, wr.bottom)); }
}

static void StopRecording()
{
    if (!g_recording) return;
    g_recording = false;  // stop DSP callback from writing more data
    if (g_recDsp && currentStream) { BASS_ChannelRemoveDSP(currentStream, g_recDsp); g_recDsp = 0; }
    if (g_recEncoder) {
        FLAC__stream_encoder_finish(g_recEncoder);
        FLAC__stream_encoder_delete(g_recEncoder);
        g_recEncoder = NULL;
    }
    if (g_recWavFile) {
        // Fix RIFF and data chunk sizes in WAV header
        DWORD riffSize = 36 + g_recWavDataBytes;
        fseek(g_recWavFile, 4, SEEK_SET);  fwrite(&riffSize, 4, 1, g_recWavFile);
        fseek(g_recWavFile, 40, SEEK_SET); fwrite(&g_recWavDataBytes, 4, 1, g_recWavFile);
        fclose(g_recWavFile);
        g_recWavFile = NULL;
        g_recWavDataBytes = 0;
    }
    // Close recording log
    if (g_recLog) {
        SYSTEMTIME st; GetLocalTime(&st);
        fwprintf(g_recLog, L"---\nRecording stopped: %04d-%02d-%02d %02d:%02d:%02d\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        fclose(g_recLog);
        g_recLog = NULL;
    }
    // Free pre-allocated conversion buffer
    if (g_recPcmBuf) { free(g_recPcmBuf); g_recPcmBuf = NULL; g_recPcmCap = 0; }
    if (hRecordBtn) InvalidateRect(hRecordBtn, NULL, TRUE);
    if (hTimeRemain) SetWindowText(hTimeRemain, L"");
    // Restore normal label width
    if (g_hwnd) { RECT wr; GetClientRect(g_hwnd, &wr); SendMessage(g_hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(wr.right, wr.bottom)); }
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
    // Custom playlists folder (Options -> Media); falls back to default if unset/missing
    wchar_t customPl[MAX_PATH] = {};
    GetPrivateProfileString(L"Paths", L"PlaylistsDir", L"", customPl, MAX_PATH, g_iniPath);
    if (customPl[0] && GetFileAttributesW(customPl) != INVALID_FILE_ATTRIBUTES) {
        wcsncpy_s(g_plDir, customPl, _TRUNCATE);
    } else {
        swprintf_s(g_plDir, L"%s\\playlists", g_libDir);
        CreateDirectoryW(g_plDir, NULL);
    }
    swprintf_s(g_favDir, L"%s\\favorites", g_libDir);
    CreateDirectoryW(g_favDir, NULL);
}

// --- Favorites ---
static std::vector<std::wstring> g_favorites;  // paths of favorited tracks
// Display names for favorites whose path isn't a readable name (stream URLs):
// path -> name shown in the favorites view. Persisted as "path\tname" lines.
static std::map<std::wstring, std::wstring> g_favNames;

static void LoadFavorites()
{
    GetLibDir();
    wchar_t favPath[MAX_PATH];
    swprintf_s(favPath, L"%s\\favorites.bpp", g_favDir);
    g_favorites.clear();
    g_favNames.clear();
    FILE* f = _wfopen(favPath, L"r,ccs=UTF-8");
    if (!f) return;
    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len == 0) continue;
        // New format: "path\tdisplay". Old files are just "path" (no tab).
        wchar_t* tab = wcschr(line, L'\t');
        if (tab) {
            *tab = 0;
            if (line[0]) {
                g_favorites.push_back(line);
                if (tab[1]) g_favNames[line] = tab + 1;
            }
        } else {
            g_favorites.push_back(line);
        }
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
    for (auto& p : g_favorites) {
        auto it = g_favNames.find(p);
        if (it != g_favNames.end() && !it->second.empty())
            fwprintf(f, L"%s\t%s\n", p.c_str(), it->second.c_str());
        else
            fwprintf(f, L"%s\n", p.c_str());
    }
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
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);

    // Browser mode: rebuild from browser items
    if (g_browserActive) {
        int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < (int)g_browserItems.size(); i++) {
            auto& bi = g_browserItems[i];
            wchar_t disp[MAX_PATH + 4];
            if (!bi.isDir && IsFavorite(bi.path))
                swprintf_s(disp, L"\u2605 %s", bi.display);
            else
                wcsncpy_s(disp, bi.display, _TRUNCATE);
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
        }
        if (sel >= 0 && sel < (int)g_browserItems.size())
            SendMessage(hListBox, LB_SETCURSEL, sel, 0);
        SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(hListBox, NULL, FALSE);
        return;
    }

    // Normal playlist mode
    int sel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
    int count = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    std::vector<bool> selState(count);
    for (int i = 0; i < count; i++)
        selState[i] = (SendMessage(hListBox, LB_GETSEL, i, 0) > 0);

    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < (int)g_playlist.size(); i++) {
        wchar_t disp[MAX_PATH + 4];
        if (!g_favActive && IsFavorite(g_playlist[i].path))
            swprintf_s(disp, L"\u2605 %s", g_playlist[i].display);
        else
            wcsncpy_s(disp, g_playlist[i].display, _TRUNCATE);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
    }
    for (int i = 0; i < min(count, (int)g_playlist.size()); i++)
        if (selState[i]) SendMessage(hListBox, LB_SETSEL, TRUE, i);
    if (sel >= 0 && sel < (int)g_playlist.size())
        SendMessage(hListBox, LB_SETCURSEL, sel, 0);
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hListBox, NULL, FALSE);
    if (g_columnView) SyncColumnView();
}

static void AddFavorite(const wchar_t* path, const wchar_t* display = nullptr)
{
    if (!IsFavorite(path)) {
        g_favorites.push_back(path);
        // Remember a friendly name for URL favorites — the raw URL is neither
        // readable nor safe to use as a display string (length, tokens).
        if (display && display[0] && IsUrlPath(path))
            g_favNames[path] = display;
        SaveFavorites();
        RefreshListboxStars();
    }
}

static void RemoveFavorite(const wchar_t* path)
{
    for (auto it = g_favorites.begin(); it != g_favorites.end(); ++it) {
        if (_wcsicmp(it->c_str(), path) == 0) {
            g_favNames.erase(*it);
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

// Custom playlist ordering (set via Playlist Manager) — stored as one name
// per line in playlists.order next to the .bpp files.
static void ApplyPlaylistOrder()
{
    wchar_t orderPath[MAX_PATH];
    swprintf_s(orderPath, L"%s\\playlists.order", g_plDir);
    FILE* f = _wfopen(orderPath, L"r,ccs=UTF-8");
    if (!f) return;
    std::vector<std::wstring> order;
    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len > 0) order.push_back(line);
    }
    fclose(f);
    if (order.empty()) return;
    std::vector<BppPlaylist> sorted;
    for (auto& nm : order) {
        for (auto it = g_bppPlaylists.begin(); it != g_bppPlaylists.end(); ++it)
            if (_wcsicmp(it->name.c_str(), nm.c_str()) == 0) {
                sorted.push_back(*it);
                g_bppPlaylists.erase(it);
                break;
            }
    }
    // Playlists not in the order file keep their scan (alphabetical) order at the end
    for (auto& rest : g_bppPlaylists) sorted.push_back(rest);
    g_bppPlaylists = sorted;
}

static void SavePlaylistOrder()
{
    GetLibDir();
    wchar_t orderPath[MAX_PATH];
    swprintf_s(orderPath, L"%s\\playlists.order", g_plDir);
    FILE* f = _wfopen(orderPath, L"w,ccs=UTF-8");
    if (!f) return;
    for (auto& pl : g_bppPlaylists)
        fwprintf(f, L"%s\n", pl.name.c_str());
    fclose(f);
}

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
    ApplyPlaylistOrder();
}

static void LoadBppPlaylist(const wchar_t* path)
{
    FILE* f = _wfopen(path, L"r,ccs=UTF-8");
    if (!f) return;
    std::vector<std::wstring> paths;
    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
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
        wcsncpy_s(t.path, _countof(t.path), p.c_str(), _TRUNCATE);
        const wchar_t* fn = wcsrchr(t.path, L'\\');
        wcsncpy_s(t.display, _countof(t.display), fn ? fn + 1 : t.path, _TRUNCATE);
        g_playlist.push_back(t);
    }
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
    }
    g_currentIndex = -1;
    RefreshListboxStars();
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

static std::vector<Track> g_libSavedPlaylist;  // saved playlist before favorites/playlist view
static int g_libSavedIndex = -1;
static int g_libActivePlaylist = -1;           // -1=none, -2=favorites, 0+=playlist index

// Leaving a library view by loading something else — clear the view state
// so the Library menu checkmark doesn't stay on stale entries.
static void LibResetViewState()
{
    g_favActive = false;
    g_libActivePlaylist = -1;
    g_libSavedPlaylist.clear();
    g_libSavedIndex = -1;
}

// Restore the original playlist from before any library view was activated
static void LibRestorePlaylist()
{
    if (g_libActivePlaylist == -1) return; // nothing to restore
    // Whatever is playing right now must stay visible after the switch —
    // capture it BEFORE swapping the playlist out. (The user may have started
    // a favorite or a Ctrl+N stream while the library view was open; the
    // saved playlist knows nothing about it.)
    Track nowPlaying = {};
    bool havePlaying = false;
    if (currentStream && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
        nowPlaying = g_playlist[g_currentIndex];
        havePlaying = true;
    }
    g_favActive = false;
    g_libActivePlaylist = -1;
    g_playlist = g_libSavedPlaylist;
    g_libSavedPlaylist.clear();
    g_currentIndex = g_libSavedIndex;
    if (havePlaying) {
        // Find the playing track in the restored playlist; if it isn't there,
        // append it so the view is never "empty while something plays".
        g_currentIndex = -1;
        for (int i = 0; i < (int)g_playlist.size(); i++)
            if (_wcsicmp(g_playlist[i].path, nowPlaying.path) == 0) { g_currentIndex = i; break; }
        if (g_currentIndex < 0) {
            g_playlist.push_back(nowPlaying);
            g_currentIndex = (int)g_playlist.size() - 1;
        }
        RebuildShuffleOrder();
    }
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        if (g_currentIndex >= 0) {
            SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
            SendMessage(hListBox, LB_SETSEL, TRUE, g_currentIndex);
            SendMessage(hListBox, LB_SETCURSEL, g_currentIndex, 0);
            SendMessage(hListBox, LB_SETTOPINDEX, max(0, g_currentIndex - 3), 0);
        }
        SetFocus(hListBox);
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
        wcsncpy_s(t.path, _countof(t.path), p.c_str(), _TRUNCATE);
        auto nit = g_favNames.find(p);
        if (nit != g_favNames.end() && !nit->second.empty()) {
            // Stored friendly name (stream favorites saved with their title)
            wcsncpy_s(t.display, _countof(t.display), nit->second.c_str(), _TRUNCATE);
        } else if (IsUrlPath(t.path)) {
            // URL with no stored name: derive one instead of showing the raw
            // URL (unreadable, and its length overflows display-format buffers)
            UrlToDisplayName(t.path, t.display, _countof(t.display));
        } else {
            const wchar_t* fn = wcsrchr(t.path, L'\\');
            wcsncpy_s(t.display, _countof(t.display), fn ? fn + 1 : t.path, _TRUNCATE);
        }
        g_playlist.push_back(t);
    }
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        SetFocus(hListBox);
    }
    g_currentIndex = -1;
    g_browserActive = false;
    // Try to find the currently playing track in this list and select it
    if (currentStream && g_libSavedIndex >= 0 && g_libSavedIndex < (int)g_libSavedPlaylist.size()) {
        const wchar_t* playingPath = g_libSavedPlaylist[g_libSavedIndex].path;
        for (int i = 0; i < (int)g_playlist.size(); i++) {
            if (_wcsicmp(g_playlist[i].path, playingPath) == 0) {
                g_currentIndex = i;
                SendMessage(hListBox, LB_SETSEL, TRUE, i);
                SendMessage(hListBox, LB_SETCURSEL, i, 0);
                SendMessage(hListBox, LB_SETTOPINDEX, max(0, i - 3), 0);
                break;
            }
        }
    }
    RefreshListboxStars();
    UpdateStatusBar();
    UpdateWindowTitle();
}

// ============================================================
//  Play History — every track / stream / radio station played,
//  saved to library\play_history.txt, viewable from File menu.
// ============================================================
struct PlayHistItem { std::wstring path, display, when; double pos = 0; };
static std::vector<PlayHistItem> g_playHistory;
static bool g_playHistLoaded = false;

static void PlayHistPath(wchar_t* out, int n)
{
    GetLibDir();
    swprintf_s(out, n, L"%s\\play_history.txt", g_libDir);
}

static void LoadPlayHistoryFile()
{
    g_playHistory.clear();
    g_playHistLoaded = true;
    wchar_t fp[MAX_PATH]; PlayHistPath(fp, MAX_PATH);
    FILE* f = _wfopen(fp, L"r,ccs=UTF-8");
    if (!f) return;
    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
        int len = (int)wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len == 0) continue;
        // Format: when \t path \t display [\t pos]
        wchar_t* t1 = wcschr(line, L'\t');
        if (!t1) continue;
        *t1 = 0;
        wchar_t* t2 = wcschr(t1 + 1, L'\t');
        PlayHistItem it;
        it.when = line;
        if (t2) {
            *t2 = 0; it.path = t1 + 1;
            wchar_t* t3 = wcschr(t2 + 1, L'\t');
            if (t3) { *t3 = 0; it.display = t2 + 1; it.pos = _wtof(t3 + 1); }
            else    { it.display = t2 + 1; }
        } else {
            it.path = t1 + 1; it.display = t1 + 1;
        }
        g_playHistory.push_back(it);
    }
    fclose(f);
}

static void SavePlayHistoryFile()
{
    wchar_t fp[MAX_PATH]; PlayHistPath(fp, MAX_PATH);
    FILE* f = _wfopen(fp, L"w,ccs=UTF-8");
    if (!f) return;
    for (auto& h : g_playHistory)
        fwprintf(f, L"%s\t%s\t%s\t%.1f\n", h.when.c_str(), h.path.c_str(), h.display.c_str(), h.pos);
    fclose(f);
}

static void RecordPlayHistory(const wchar_t* path, const wchar_t* display)
{
    if (!path || !path[0]) return;
    if (!g_playHistLoaded) LoadPlayHistoryFile();
    // Skip if identical to the most recent entry (avoid dup on replay/reconnect)
    if (!g_playHistory.empty() && _wcsicmp(g_playHistory.back().path.c_str(), path) == 0)
        return;
    SYSTEMTIME st; GetLocalTime(&st);
    wchar_t when[32];
    swprintf_s(when, L"%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    PlayHistItem it;
    it.path = path;
    it.display = (display && display[0]) ? display : path;
    it.when = when;
    g_playHistory.push_back(it);
    wchar_t fp[MAX_PATH]; PlayHistPath(fp, MAX_PATH);
    FILE* f = _wfopen(fp, L"a,ccs=UTF-8");
    if (f) { fwprintf(f, L"%s\t%s\t%s\t0\n", when, path, it.display.c_str()); fclose(f); }
}

// Remember where a local track stopped so it can be resumed from Play History
static void UpdatePlayHistoryPos(const wchar_t* path, double pos)
{
    if (!path || !path[0] || pos < 3.0) return;
    if (!g_playHistLoaded) LoadPlayHistoryFile();
    for (int i = (int)g_playHistory.size() - 1; i >= 0; i--) {
        if (_wcsicmp(g_playHistory[i].path.c_str(), path) == 0) {
            g_playHistory[i].pos = pos;
            SavePlayHistoryFile();
            return;
        }
    }
}

// Toggle the Play History view in the main list (select again to close it)
static void ShowPlayHistoryView()
{
    if (g_libActivePlaylist == -3) { LibRestorePlaylist(); return; }
    LoadPlayHistoryFile();
    if (g_playHistory.empty()) {
        MessageBox(g_hwnd, L"No play history yet.\n\nTracks, streams and radio stations you play\nare recorded here automatically.",
            L"Play History", MB_ICONINFORMATION);
        return;
    }
    LibSaveIfNeeded();
    g_favActive = false;
    g_libActivePlaylist = -3;
    g_browserActive = false;
    // Most recent first. Local tracks show date/time + last stop position
    // (playing from here resumes at that point); streams show just the time.
    g_playlist.clear();
    for (int i = (int)g_playHistory.size() - 1; i >= 0; i--) {
        auto& h = g_playHistory[i];
        Track t = {};
        wcsncpy_s(t.path, h.path.c_str(), _TRUNCATE);
        bool isUrl = (_wcsnicmp(h.path.c_str(), L"http://", 7) == 0 ||
                      _wcsnicmp(h.path.c_str(), L"https://", 8) == 0 ||
                      _wcsnicmp(h.path.c_str(), L"ftp://", 6) == 0);
        // _TRUNCATE: old history files stored the raw URL (up to ~1023 chars)
        // as display — plain swprintf_s would abort the whole process here.
        wchar_t line[600];
        if (!isUrl && h.pos > 3.0) {
            wchar_t pb[32]; FormatTime(h.pos, pb, _countof(pb));
            _snwprintf_s(line, _countof(line), _TRUNCATE,
                L"%s    [%s  ·  at %s]", h.display.c_str(), h.when.c_str(), pb);
        } else {
            _snwprintf_s(line, _countof(line), _TRUNCATE,
                L"%s    [%s]", h.display.c_str(), h.when.c_str());
        }
        wcsncpy_s(t.display, line, _TRUNCATE);
        g_playlist.push_back(t);
    }
    g_currentIndex = -1;
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (auto& t : g_playlist)
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        SetFocus(hListBox);
    }
    RebuildShuffleOrder();
    RefreshListboxStars();
    UpdateStatusBar();
    if (hStatus) SendMessage(hStatus, SB_SETTEXT, 0,
        (LPARAM)L"Play History  —  double-click or Enter to play (local tracks resume), select Play History again to close");
}

// After playing an entry from the Play History view, resume local tracks
// at the position they were stopped at.
static void PlayHistResumeSeek(int viewIdx)
{
    if (g_libActivePlaylist != -3) return;
    int hi = (int)g_playHistory.size() - 1 - viewIdx;
    if (hi < 0 || hi >= (int)g_playHistory.size()) return;
    double p = g_playHistory[hi].pos;
    if (p > 3.0 && currentStream && g_decStream)
        SeekToSeconds(p);
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
    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
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
    swprintf_s(buf, L"%d", g_loudMode);         WritePrivateProfileString(L"DSP", L"LoudMode", buf, g_iniPath);
    // Stream loudness step is intentionally NOT persisted — every launch and
    // every fresh stream defaults to 2 (user can still adjust the live slider).
    swprintf_s(buf, L"%d", (int)g_dspReverb);   WritePrivateProfileString(L"DSP", L"Reverb",   buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspSaturate); WritePrivateProfileString(L"DSP", L"Saturate", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspVinyl);    WritePrivateProfileString(L"DSP", L"Vinyl",    buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspHifi);     WritePrivateProfileString(L"DSP", L"Hifi",     buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dspBypass); WritePrivateProfileString(L"DSP", L"Bypass",   buf, g_iniPath);
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
    WritePrivateProfileString(L"Paths", L"RecordSaveDir", g_recSaveDir, g_iniPath);
    swprintf_s(buf, L"%d", g_recFormat); WritePrivateProfileString(L"Recording", L"Format", buf, g_iniPath);
    WritePrivateProfileString(L"Discogs", L"Token", g_discogsToken, g_iniPath);
    // UI prefs
    swprintf_s(buf, L"%d", (int)g_darkMode);   WritePrivateProfileString(L"UI", L"DarkMode",      buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_modernStyle);WritePrivateProfileString(L"UI", L"ModernStyle",   buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_columnView); WritePrivateProfileString(L"UI", L"ColumnView",    buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_colPathRelative); WritePrivateProfileString(L"UI", L"ColPathRelative", buf, g_iniPath);
    // Always save column layout globally
    {
        if (hColumnHeader && Header_GetItemCount(hColumnHeader) > 0)
            SaveColumnOrderFromHeader();
        wchar_t vis[64] = L"", widths[128] = L"", ord[128] = L"";
        for (int i = 0; i < COL_COUNT; i++) {
            if (i > 0) { wcscat_s(vis, L","); wcscat_s(widths, L","); wcscat_s(ord, L","); }
            wchar_t tmp[16];
            swprintf_s(tmp, L"%d", g_colVisible[i] ? 1 : 0); wcscat_s(vis, tmp);
            swprintf_s(tmp, L"%d", g_colWidth[i]); wcscat_s(widths, tmp);
            swprintf_s(tmp, L"%d", g_colOrder[i]); wcscat_s(ord, tmp);
        }
        WritePrivateProfileString(L"UI", L"ColVisible", vis, g_iniPath);
        WritePrivateProfileString(L"UI", L"ColWidths", widths, g_iniPath);
        WritePrivateProfileString(L"UI", L"ColOrder", ord, g_iniPath);
    }
    swprintf_s(buf, L"%d", g_modernSize);      WritePrivateProfileString(L"UI", L"ModernSize",    buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_multiInst);  WritePrivateProfileString(L"UI", L"MultiInstance", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dropAppend); WritePrivateProfileString(L"UI", L"DropAppend",    buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dropLoadDir); WritePrivateProfileString(L"UI", L"DropLoadDir",  buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_dropLoadTree); WritePrivateProfileString(L"UI", L"DropLoadTree", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_scrollToPlaying); WritePrivateProfileString(L"UI", L"ScrollToPlaying", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_shazAllowLocal); WritePrivateProfileString(L"UI", L"ShazamAllowLocal", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_loudHintShown); WritePrivateProfileString(L"UI", L"LoudHintShown", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_globalLimiter);  WritePrivateProfileString(L"DSP", L"GlobalLimiter", buf, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_setsEnabled); WritePrivateProfileString(L"SetsLive", L"Enabled", buf, g_iniPath);
    WritePrivateProfileString(L"SetsLive", L"Server", g_setsServer, g_iniPath);
    WritePrivateProfileString(L"SetsLive", L"Token", g_setsToken, g_iniPath);
    WritePrivateProfileString(L"SetsLive", L"DownloadDir", g_setsDownloadDir, g_iniPath);
    swprintf_s(buf, L"%d", (int)g_shuffle);    WritePrivateProfileString(L"UI", L"Shuffle",       buf, g_iniPath);
    swprintf_s(buf, L"%d", g_repeatMode);       WritePrivateProfileString(L"UI", L"RepeatMode",    buf, g_iniPath);
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
    swprintf_s(buf, L"%d", g_rememberSession); WritePrivateProfileString(L"UI", L"RememberSession", buf, g_iniPath);
    // Customizable hotkeys: "vk,mods" per action
    for (int i = 0; i < HK_COUNT; i++) {
        wchar_t key[16]; swprintf_s(key, L"HK%d", i);
        swprintf_s(buf, L"%d,%d", g_hotkeys[i].vk, g_hotkeys[i].mods);
        WritePrivateProfileString(L"Hotkeys", key, buf, g_iniPath);
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
    g_loudMode    = GETI(L"DSP", L"LoudMode", 1);
    if (g_loudMode < 0 || g_loudMode > 3) g_loudMode = 1;
    g_loudStreamStep = 2;   // always default to 2 (not persisted)
    g_dspReverb   = GETI(L"DSP", L"Reverb",   0) != 0;
    g_dspSaturate = GETI(L"DSP", L"Saturate", 0) != 0;
    g_dspVinyl    = GETI(L"DSP", L"Vinyl",    0) != 0;
    g_dspHifi     = GETI(L"DSP", L"Hifi",     0) != 0;
    g_dspBypass   = GETI(L"DSP", L"Bypass",   0) != 0;
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
    GetPrivateProfileString(L"Paths", L"RecordSaveDir", L"", g_recSaveDir, MAX_PATH, g_iniPath);
    g_recFormat = GETI(L"Recording", L"Format", 0);
    if (g_recFormat < 0 || g_recFormat > 1) g_recFormat = 0;
    GetPrivateProfileString(L"Discogs", L"Token", L"", g_discogsToken, _countof(g_discogsToken), g_iniPath);
    g_darkMode   = GETI(L"UI", L"DarkMode",      0) != 0;
    g_modernStyle= GETI(L"UI", L"ModernStyle",   0) != 0;
    g_columnView = GETI(L"UI", L"ColumnView",   0) != 0;
    g_colPathRelative = GETI(L"UI", L"ColPathRelative", 0) != 0;
    // Always load column layout globally
    {
        wchar_t vis[64] = {}, widths[128] = {}, ord[128] = {};
        GetPrivateProfileString(L"UI", L"ColVisible", L"", vis, 64, g_iniPath);
        GetPrivateProfileString(L"UI", L"ColWidths", L"", widths, 128, g_iniPath);
        GetPrivateProfileString(L"UI", L"ColOrder", L"", ord, 128, g_iniPath);
        if (ord[0]) {
            wchar_t* ctx2 = nullptr; int j = 0;
            for (wchar_t* tok = wcstok_s(ord, L",", &ctx2); tok && j < COL_COUNT; tok = wcstok_s(nullptr, L",", &ctx2), j++) {
                int v = _wtoi(tok);
                if (v >= 0 && v < COL_COUNT) g_colOrder[j] = v;
            }
        }
        if (vis[0]) {
            wchar_t* ctx = nullptr; int i = 0;
            for (wchar_t* tok = wcstok_s(vis, L",", &ctx); tok && i < COL_COUNT; tok = wcstok_s(nullptr, L",", &ctx), i++)
                g_colVisible[i] = (_wtoi(tok) != 0);
            g_colVisible[COL_NAME] = true;
        }
        if (widths[0]) {
            wchar_t* ctx = nullptr; int i = 0;
            for (wchar_t* tok = wcstok_s(widths, L",", &ctx); tok && i < COL_COUNT; tok = wcstok_s(nullptr, L",", &ctx), i++) {
                int w = _wtoi(tok);
                if (w > 10 && w < 2000) g_colWidth[i] = w;
            }
        }
    }
    g_modernSize = max(0, min(2, GETI(L"UI", L"ModernSize", 0)));
    g_multiInst  = GETI(L"UI", L"MultiInstance", 0) != 0;
    g_dropAppend = GETI(L"UI", L"DropAppend",    1) != 0;
    g_dropLoadDir = GETI(L"UI", L"DropLoadDir", 0) != 0;
    g_dropLoadTree = GETI(L"UI", L"DropLoadTree", 0) != 0;
    g_scrollToPlaying = GETI(L"UI", L"ScrollToPlaying", 0) != 0;
    g_shazAllowLocal  = GETI(L"UI", L"ShazamAllowLocal", 0) != 0;
    g_loudHintShown   = GETI(L"UI", L"LoudHintShown", 0) != 0;
    g_globalLimiter   = GETI(L"DSP", L"GlobalLimiter", 1) != 0;
    g_setsEnabled = GETI(L"SetsLive", L"Enabled", 0) != 0;
    GetPrivateProfileString(L"SetsLive", L"Server", L"app.sets.live", g_setsServer, _countof(g_setsServer), g_iniPath);
    GetPrivateProfileString(L"SetsLive", L"Token", L"", g_setsToken, _countof(g_setsToken), g_iniPath);
    GetPrivateProfileString(L"SetsLive", L"DownloadDir", L"", g_setsDownloadDir, _countof(g_setsDownloadDir), g_iniPath);
    if (!g_setsServer[0]) wcscpy_s(g_setsServer, L"app.sets.live");
    g_shuffle    = GETI(L"UI", L"Shuffle",       0) != 0;
    g_repeatMode = GETI(L"UI", L"RepeatMode",    0);
    if (g_repeatMode < 0 || g_repeatMode > 3) g_repeatMode = 0;
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
    g_rememberSession = GETI(L"UI", L"RememberSession", 0);
    if (g_rememberSession < 0 || g_rememberSession > 2) g_rememberSession = 0;
    // Customizable hotkeys — missing keys fall back to factory defaults
    for (int i = 0; i < HK_COUNT; i++) {
        wchar_t key[16]; swprintf_s(key, L"HK%d", i);
        wchar_t val[32] = {};
        GetPrivateProfileString(L"Hotkeys", key, L"", val, 32, g_iniPath);
        int vk = 0, mods = 0;
        if (val[0] && swscanf_s(val, L"%d,%d", &vk, &mods) == 2 && vk >= 0 && vk < 256) {
            g_hotkeys[i].vk = (BYTE)vk;
            g_hotkeys[i].mods = (BYTE)(mods & 7);
        } else {
            g_hotkeys[i].vk = g_hotkeys[i].defVk;
            g_hotkeys[i].mods = g_hotkeys[i].defMods;
        }
    }
#undef GETI
#undef GETF
}

// ============================================================
//  Session save / restore (remember playlist on exit)
//  Writes a simple text file: line 1 = track index, line 2 = seek seconds,
//  remaining lines = file paths.  Stored next to the INI.
// ============================================================
static void GetSessionPath(wchar_t* out, size_t n)
{
    GetIniPath();
    wcsncpy_s(out, n, g_iniPath, _TRUNCATE);
    wchar_t* dot = wcsrchr(out, L'.');
    if (dot) wcscpy_s(dot, n - (dot - out), L".session");
    else     wcscat_s(out, n, L".session");
}

static void SaveSession()
{
    if (g_rememberSession == 0) return;
    wchar_t sessPath[MAX_PATH];
    GetSessionPath(sessPath, MAX_PATH);
    FILE* f = _wfopen(sessPath, L"w,ccs=UTF-8");
    if (!f) return;
    // Line 1: current track index
    fwprintf(f, L"%d\n", g_currentIndex);
    // Line 2: seek position in seconds (0 if mode 1)
    double pos = (g_rememberSession >= 2 && currentStream) ? GetPlayPos() : 0.0;
    fwprintf(f, L"%.3f\n", pos);
    // Remaining lines: path<TAB>display per track (TAB separates path from display name)
    for (auto& t : g_playlist)
        fwprintf(f, L"%s\t%s\n", t.path, t.display);
    fclose(f);
}

static bool RestoreSession()
{
    if (g_rememberSession == 0) return false;
    wchar_t sessPath[MAX_PATH];
    GetSessionPath(sessPath, MAX_PATH);
    FILE* f = _wfopen(sessPath, L"r,ccs=UTF-8");
    if (!f) return false;
    wchar_t line[2048];
    // Line 1: track index
    if (!fgetws(line, _countof(line), f)) { fclose(f); return false; }
    int savedIdx = _wtoi(line);
    // Line 2: seek position
    if (!fgetws(line, _countof(line), f)) { fclose(f); return false; }
    double savedPos = _wtof(line);
    // Remaining lines: path<TAB>display per track
    struct SessEntry { std::wstring path, display; };
    std::vector<SessEntry> entries;
    while (fgetws(line, _countof(line), f)) {
        int len = (int)wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len == 0) continue;
        // Split on first TAB — if no tab, treat entire line as path (backward compat)
        wchar_t* tab = wcschr(line, L'\t');
        SessEntry e;
        if (tab) {
            *tab = 0;
            e.path = line;
            e.display = tab + 1;
        } else {
            e.path = line;
        }
        entries.push_back(e);
    }
    fclose(f);
    if (entries.empty()) return false;
    // Rebuild playlist — accept both local files and URLs (radio streams)
    ClearPlaylist();
    for (auto& e : entries) {
        bool isUrl = (_wcsnicmp(e.path.c_str(), L"http://", 7) == 0 || _wcsnicmp(e.path.c_str(), L"https://", 8) == 0 ||
                      _wcsnicmp(e.path.c_str(), L"ftp://", 6) == 0);
        if (isUrl || GetFileAttributesW(e.path.c_str()) != INVALID_FILE_ATTRIBUTES) {
            Track t = {};
            wcsncpy_s(t.path, e.path.c_str(), _TRUNCATE);
            // Use saved display name if available, otherwise derive from path
            if (!e.display.empty())
                wcsncpy_s(t.display, e.display.c_str(), _TRUNCATE);
            else
                wcsncpy_s(t.display, Filename(t.path), _TRUNCATE);
            g_playlist.push_back(t);
            if (hListBox)
                SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        }
    }
    RebuildShuffleOrder();
    if (g_playlist.empty()) return false;
    // Clamp saved index
    if (savedIdx < 0) savedIdx = 0;
    if (savedIdx >= (int)g_playlist.size()) savedIdx = (int)g_playlist.size() - 1;
    g_currentIndex = savedIdx;
    SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
    SendMessage(hListBox, LB_SETSEL, TRUE, savedIdx);
    SendMessage(hListBox, LB_SETCURSEL, savedIdx, 0);
    SendMessage(hListBox, LB_SETTOPINDEX, max(0, savedIdx - 3), 0);
    // Check if the selected track is a URL (radio) — don't auto-play or seek URLs
    bool selectedIsUrl = (_wcsnicmp(g_playlist[savedIdx].path, L"http://", 7) == 0 ||
                          _wcsnicmp(g_playlist[savedIdx].path, L"https://", 8) == 0 ||
                          _wcsnicmp(g_playlist[savedIdx].path, L"ftp://", 6) == 0);
    if (!selectedIsUrl) {
        // Open the track paused (never auto-play)
        PlayIndex(savedIdx);
        if (currentStream) {
            BASS_ChannelPause(currentStream);
            KillTimer(g_hwnd, IDT_PLAYBACK);
            // Seek to saved position if mode 2
            if (g_rememberSession >= 2 && savedPos > 0.5)
                SeekToSeconds(savedPos);
            UpdatePlayBtn();
            UpdateStatusBar();
        }
    }
    // For URLs: just show the playlist, user can double-click to play
    RefreshListboxStars();
    UpdateStatusBar();
    UpdateWindowTitle();
    return true;
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
    // Extract M3U directory for resolving relative paths
    wchar_t m3uDir[MAX_PATH] = {};
    wcsncpy_s(m3uDir, path, _TRUNCATE);
    wchar_t* sep = wcsrchr(m3uDir, L'\\');
    if (!sep) sep = wcsrchr(m3uDir, L'/');
    if (sep) sep[1] = 0; else m3uDir[0] = 0;

    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
        int len = (int)wcslen(line);
        while (len > 0 && (line[len-1] == L'\r' || line[len-1] == L'\n')) line[--len] = 0;
        if (len == 0 || line[0] == L'#') continue;
        // URLs (http/https) — add directly
        if (_wcsnicmp(line, L"http://", 7) == 0 || _wcsnicmp(line, L"https://", 8) == 0) {
            AddTrack(line);
            continue;
        }
        // Try as absolute path first
        if (GetFileAttributesW(line) != INVALID_FILE_ATTRIBUTES && IsAudio(line)) {
            AddTrack(line);
            continue;
        }
        // Try as relative path resolved against the M3U directory
        if (m3uDir[0] && line[0] != L'\\' && !(len >= 2 && line[1] == L':')) {
            wchar_t resolved[MAX_PATH];
            _snwprintf_s(resolved, MAX_PATH, _TRUNCATE, L"%s%s", m3uDir, line);
            if (GetFileAttributesW(resolved) != INVALID_FILE_ATTRIBUTES && IsAudio(resolved)) {
                AddTrack(resolved);
                continue;
            }
        }
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

// Loudness normalization (Spotify-style): scan the file's loudness (RMS) and
// compute a gain that brings it to a consistent target level — quiet tracks
// get boosted, loud tracks get tamed. The gain is capped so boosted peaks
// can't clip. Applied per decode stream, so gapless playback is unaffected.
// Caches the last result to avoid rescanning the same file repeatedly.
static wchar_t s_normCachePath[MAX_PATH] = {};
static float   s_normCacheGain = 1.0f;

static float ComputeNormGain(const wchar_t* path)
{
    if (s_normCachePath[0] && _wcsicmp(path, s_normCachePath) == 0)
        return s_normCacheGain;

    float peak = 0.001f;
    double sumSq = 0.0;
    ULONGLONG nSamp = 0;
    HSTREAM scan = BASS_StreamCreateFile(FALSE, path, 0, 0,
        BASS_UNICODE | BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
    if (scan) {
        float fbuf[4096]; DWORD got; int chunks = 0;
        // ~28 seconds of audio — enough to judge overall loudness
        while (chunks < 600 &&
            (got = BASS_ChannelGetData(scan, fbuf, sizeof(fbuf))) != (DWORD)-1 && got > 0) {
            DWORD n = got / sizeof(float);
            for (DWORD j = 0; j < n; j++) {
                float a = fabsf(fbuf[j]);
                if (a > peak) peak = a;
                sumSq += (double)fbuf[j] * fbuf[j];
            }
            nSamp += n;
            chunks++;
        }
        BASS_StreamFree(scan);
    }

    // Loud modes (right-click the Loud button): target loudness + max boost.
    // Quiet tracks are raised at least to ~0 dB peak; the RMS target pushes
    // low-loudness material (e.g. classical) further, and the output limiter
    // (DSP_Limiter) catches anything driven past full scale. Loud tracks get
    // gain 1.0 — i.e. untouched except for peak limiting.
    static const float LOUD_TARGET_RMS[4] = { 0.10f, 0.15f, 0.20f, 0.28f };
    static const float LOUD_MAX_BOOST[4]  = { 3.0f,  8.0f,  12.0f, 16.0f };
    int m = max(0, min(3, g_loudMode));

    float gain = 1.0f;
    if (nSamp > 0) {
        float rms = (float)sqrt(sumSq / (double)nSamp);
        float byPeak = 0.95f / peak;                                    // peaks to ~0 dB
        float byRms  = (rms > 0.0005f) ? LOUD_TARGET_RMS[m] / rms : 1.0f;
        gain = max(byPeak, byRms);
        if (gain < 1.0f) gain = 1.0f;              // never turn tracks down
        if (gain > LOUD_MAX_BOOST[m]) gain = LOUD_MAX_BOOST[m];
    }

    wcsncpy_s(s_normCachePath, path, _TRUNCATE);
    s_normCacheGain = gain;
    return gain;
}

void ApplyDSP()
{
    if (!currentStream) return;
    // Remove all DSP
    if (g_dspMono)   { BASS_ChannelRemoveDSP(currentStream, g_dspMono);   g_dspMono   = 0; }
    if (g_dspBass)   { BASS_ChannelRemoveDSP(currentStream, g_dspBass);   g_dspBass   = 0; }
    if (g_dspLimit)  { BASS_ChannelRemoveDSP(currentStream, g_dspLimit);  g_dspLimit  = 0; }
    if (g_dspGlobalLimit) { BASS_ChannelRemoveDSP(currentStream, g_dspGlobalLimit); g_dspGlobalLimit = 0; }
    if (g_dspRevHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspRevHdl); g_dspRevHdl = 0; }
    if (g_dspSatHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspSatHdl); g_dspSatHdl = 0; }
    if (g_dspVinHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspVinHdl); g_dspVinHdl = 0; }
    if (g_dspHfiHdl) { BASS_ChannelRemoveDSP(currentStream, g_dspHfiHdl); g_dspHfiHdl = 0; }
    // Reapply
    if (g_mono)        g_dspMono   = BASS_ChannelSetDSP(currentStream, DSP_Mono,     NULL, 1);
    if (g_bassBoost)   g_dspBass   = BASS_ChannelSetDSP(currentStream, DSP_BassBoost,NULL, 2);
    if (g_normalize) {
        // Loud: output processor runs last in the chain (lowest priority).
        // No decode stream = playing a URL directly (radio / stream link)
        // -> adaptive stream mode instead of the pre-scanned file gain.
        s_loudEnv = 1.0f;
        s_loudStream = (g_decStream == 0);
        g_dspLimit = BASS_ChannelSetDSP(currentStream, DSP_Limiter, NULL, 0);
    }
    // Master brickwall limiter — absolute last stage (lowest priority) so it
    // catches clipping from Loud, Bass Boost, or any effect.
    if (g_globalLimiter) {
        s_glEnv = 1.0f;
        g_dspGlobalLimit = BASS_ChannelSetDSP(currentStream, DSP_GlobalLimiter, NULL, -1000000);
    }
    if (!g_dspBypass) {
        if (g_dspReverb)   g_dspRevHdl = BASS_ChannelSetDSP(currentStream, DSP_Reverb,  NULL, 3);
        if (g_dspSaturate) g_dspSatHdl = BASS_ChannelSetDSP(currentStream, DSP_Saturate,NULL, 4);
        if (g_dspVinyl)    g_dspVinHdl = BASS_ChannelSetDSP(currentStream, DSP_Vinyl,   NULL, 5);
        if (g_dspHifi)     g_dspHfiHdl = BASS_ChannelSetDSP(currentStream, DSP_Hifi,    NULL, 6);
    }
    // Update DSP button visibility
    if (g_hwnd) LayoutControls(g_hwnd);
    // Loud makeup gain — read by DSP_Limiter on the master stream. Setting
    // VOL on the decode stream does nothing (BASS ignores it for decoding).
    if (g_normalize && g_decStream && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
        g_loudNormGain = ComputeNormGain(g_playlist[g_currentIndex].path);
    else
        g_loudNormGain = 1.0f;   // radio/URL streams: limiter only
    BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
}

// ============================================================
//  Pitch (CDJ-style: changes pitch + tempo together via sample rate)
// ============================================================
void ApplyPitch()
{
    if (!currentStream || g_masterFreq == 0) return;
    if (g_pitchEnabled && g_pitchSemitones != 0.0f) {
        // Always calculate from the stored base frequency, never from the
        // current (possibly already pitched) stream frequency
        float newFreq = (float)g_masterFreq * (float)pow(2.0, g_pitchSemitones / 12.0);
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_FREQ, newFreq);
    } else {
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_FREQ, (float)g_masterFreq);
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
    if (g_playlist.empty() && !g_playlistRoot[0])
        wcsncpy_s(g_playlistRoot, folder, _TRUNCATE);
    wchar_t pat[MAX_PATH]; swprintf_s(pat, L"%s\\*.*", folder);
    WIN32_FIND_DATA fd; HANDLE h = FindFirstFile(pat, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.cFileName[0] == L'.') continue;
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", folder, fd.cFileName);
                if (IsAudio(full)) AddTrack(full);
            } else if (g_dropLoadTree) {
                wchar_t sub[MAX_PATH]; swprintf_s(sub, L"%s\\%s", folder, fd.cFileName);
                AddFolder(sub);
            }
        } while (FindNextFile(h, &fd));
        FindClose(h);
    }
}

// Rescan a folder and append only audio files not already in the playlist
static void AddFolderNewOnly(const wchar_t* folder, int& added)
{
    wchar_t pat[MAX_PATH]; swprintf_s(pat, L"%s\\*.*", folder);
    WIN32_FIND_DATA fd; HANDLE h = FindFirstFile(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.cFileName[0] == L'.') continue;
        wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", folder, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (g_dropLoadTree) AddFolderNewOnly(full, added);
        } else if (IsAudio(full)) {
            bool exists = false;
            for (auto& t : g_playlist)
                if (_wcsicmp(t.path, full) == 0) { exists = true; break; }
            if (!exists) { AddTrack(full); added++; }
        }
    } while (FindNextFile(h, &fd));
    FindClose(h);
}

// Refresh Playlist (Shift+R / File menu / right-click): rescan the source
// folder — append new tracks AND remove entries whose files were deleted,
// without interrupting playback (unless the playing file itself is gone).
static void RefreshPlaylistFolder()
{
    if (g_browserActive || !g_playlistRoot[0]) return;
    int added = 0;
    AddFolderNewOnly(g_playlistRoot, added);
    // Drop entries whose files no longer exist on disk (keep URLs)
    int removed = 0;
    for (int i = (int)g_playlist.size() - 1; i >= 0; i--) {
        const wchar_t* p = g_playlist[i].path;
        if (_wcsnicmp(p, L"http://", 7) == 0 || _wcsnicmp(p, L"https://", 8) == 0 ||
            _wcsnicmp(p, L"ftp://", 6) == 0)
            continue;
        if (GetFileAttributesW(p) == INVALID_FILE_ATTRIBUTES) {
            if (i == g_currentIndex) StopAudio();
            g_playlist.erase(g_playlist.begin() + i);
            if (hListBox) SendMessage(hListBox, LB_DELETESTRING, i, 0);
            if (g_currentIndex > i) g_currentIndex--;
            else if (g_currentIndex == i) g_currentIndex = -1;
            removed++;
        }
    }
    if (added > 0 || removed > 0) {
        RebuildShuffleOrder();
        RefreshListboxStars();
        if (g_columnView) SyncColumnView();
        UpdateStatusBar();
    }
    if (hStatus) {
        wchar_t msg[128];
        if (added > 0 || removed > 0)
            swprintf_s(msg, L"Refreshed: %d added, %d removed (%d total)",
                added, removed, (int)g_playlist.size());
        else
            swprintf_s(msg, L"Refreshed: no changes found");
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)msg);
    }
}

// Decode streams that the UI thread must free (deferred from the mixing-thread
// callback, which can't call BASS_StreamFree). A single slot could be
// overwritten when two gapless transitions happen before the UI thread drains
// it (e.g. a zero/tiny next track), leaking the earlier stream. Use a small
// single-producer (mixing thread) / single-consumer (UI thread) ring instead.
#define DEC_FREE_RING 16
static volatile HSTREAM g_decFreeRing[DEC_FREE_RING] = { 0 };
static volatile LONG    g_decFreeHead = 0;   // advanced by the mixing thread
static volatile LONG    g_decFreeTail = 0;   // advanced by the UI thread

// Mixing thread: queue a stream for the UI thread to free.
static void DecDeferFree(HSTREAM h)
{
    if (!h) return;
    LONG head = g_decFreeHead;
    // If the consumer has fallen 16 behind, free inline as a last resort rather
    // than clobber an unread slot (should never happen in practice).
    if (head - g_decFreeTail >= DEC_FREE_RING) { BASS_StreamFree(h); return; }
    g_decFreeRing[head % DEC_FREE_RING] = h;
    InterlockedExchange(&g_decFreeHead, head + 1);   // publish after the store
}

// UI thread: free every stream queued so far.
static void DecDrainFrees()
{
    LONG head = g_decFreeHead;   // snapshot the producer index
    while (g_decFreeTail != head) {
        LONG i = g_decFreeTail % DEC_FREE_RING;
        HSTREAM h = g_decFreeRing[i];
        g_decFreeRing[i] = 0;
        if (h) BASS_StreamFree(h);
        InterlockedExchange(&g_decFreeTail, g_decFreeTail + 1);
    }
}

void StopAudio()
{
    // Remember where this local track stopped (Play History resume point)
    if (currentStream && g_decStream && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
        UpdatePlayHistoryPos(g_playlist[g_currentIndex].path, GetPlayPos());
    KillTimer(g_hwnd, IDT_PLAYBACK);
    KillTimer(g_hwnd, IDT_SEEK_REPEAT);
    KillTimer(g_hwnd, IDT_PEAK_METER);
    KillTimer(g_hwnd, IDT_RADIO_META);
    KillTimer(g_hwnd, IDT_RADIO_RECONN);
    g_seekKeyHeld = false;
    // Stop recording if active
    if (g_recording) StopRecording();
    // Clean up radio state
    if (g_radioPlaying) {
        g_radioPlaying = false;
        g_radioStream = 0; // will be freed with currentStream below
        g_radioStationName[0] = L'\0';
        g_radioNowPlaying[0] = L'\0';
    }
    // Cancel pending reconnect countdown and invalidate any in-flight connect
    // thread — a late WM_RADIO_CONNECTED with a stale generation is discarded.
    g_radioReconnAttempt = 0;
    g_radioReconnCountdown = 0;
    g_radioReconnUrl[0] = L'\0';
    g_radioPlayStart = 0;
    InterlockedIncrement(&g_radioConnGen);
    // Lock the master stream so GaplessProc can't read decode streams while we free them
    if (currentStream) BASS_ChannelLock(currentStream, TRUE);
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
    DecDrainFrees();
    if (currentStream) {
        BASS_ChannelLock(currentStream, FALSE);
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
    SmtcUpdatePlaybackStatus();
    if (hListBox) InvalidateRect(hListBox, NULL, FALSE); // clear now-playing highlight
    SetWindowText(g_hwnd, APP_TITLE);
}

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
                    DecDeferFree(oldDec);
                    PostMessage((HWND)user, WM_PLAYNEXT, 1, 0);
                } else {
                    // Format mismatch — can't feed into current master stream.
                    DecDeferFree(oldDec);
                    PostMessage((HWND)user, WM_PLAYNEXT, 2, 0);
                    break;
                }
            } else {
                // No next track preloaded — don't post WM_PLAYNEXT here.
                // The master stream still has buffered audio that hasn't
                // reached the speakers yet. BASS_SYNC_END on the master
                // will fire after the buffer drains naturally.
                DecDeferFree(oldDec);
                break;
            }
        }
    }

    if (total == 0) return BASS_STREAMPROC_END;
    return total;
}

// Fired when the master output finishes (end of playlist, no more data).
// `channel` is the master stream that ended — passed as lParam so the handler
// can discard a stale end-sync from a master that's already been replaced
// (e.g. after a format-mismatch switch), which would otherwise double-advance.
void CALLBACK EndSyncProc(HSYNC, DWORD channel, DWORD, void* user)
{
    PostMessage((HWND)user, WM_PLAYNEXT, 0, (LPARAM)channel);
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
        // Lock master stream so GaplessProc (mixing thread) can't read the
        // dca decoder mid-seek and pull garbage / crash on a half-updated state.
        BASS_ChannelLock(currentStream, TRUE);
        DcaSeek(rawSec);
        BASS_ChannelSetPosition(currentStream, 0, BASS_POS_BYTE);
        BASS_ChannelLock(currentStream, FALSE);
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
    // Clamp to 0.05s before end — seeking to the exact last byte fails in BASS,
    // the remaining audio plays out naturally and triggers next track via GaplessProc
    if (rawLen > 0.1 && rawSec > rawLen - 0.05) rawSec = rawLen - 0.05;
    else if (rawSec > rawLen) rawSec = rawLen;
    BASS_ChannelLock(currentStream, TRUE);
    InterlockedExchange(&g_seeking, 1);
    BASS_ChannelSetPosition(seekTarget,
        BASS_ChannelSeconds2Bytes(seekTarget, rawSec), BASS_POS_BYTE);
    InterlockedExchange(&g_seeking, 0);
    // Flush the master's buffer only when it's a separate stream — when
    // playing a URL directly (no decode stream) this would undo the seek!
    if (g_decStream && seekTarget != currentStream)
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
    // Show recording file size when recording
    if (g_recording) {
        // Estimate FLAC size from PCM bytes fed (~60% compression)
        ULONGLONG sz = (ULONGLONG)(g_recBytesIn * 0.6);
        // Try actual file size if available
        if (g_recPath[0]) {
            WIN32_FILE_ATTRIBUTE_DATA fa = {};
            if (GetFileAttributesEx(g_recPath, GetFileExInfoStandard, &fa)) {
                ULONGLONG diskSz = ((ULONGLONG)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;
                if (diskSz > 0) sz = diskSz;
            }
        }
        wchar_t szBuf[32];
        if (sz > 1024ULL * 1024 * 1024)
            swprintf_s(szBuf, L"\u23FA %.1f GB", sz / (1024.0 * 1024.0 * 1024.0));
        else if (sz > 1024ULL * 1024)
            swprintf_s(szBuf, L"\u23FA %.1f MB", sz / (1024.0 * 1024.0));
        else
            swprintf_s(szBuf, L"\u23FA %.0f KB", max(1.0, sz / 1024.0));
        if (hTimeRemain) SetWindowText(hTimeRemain, szBuf);
    }
    if (g_radioPlaying) {
        if (hTimeCur) SetWindowText(hTimeCur, L"LIVE");
        if (hTimeTot && g_radioPlayStart > 0) {
            ULONGLONG elapsedMs = GetTickCount64() - g_radioPlayStart;
            int t = (int)(elapsedMs / 1000);
            int h = t / 3600, m = (t % 3600) / 60, s = t % 60;
            wchar_t buf[32];
            if (h > 0) swprintf_s(buf, L" %d:%02d:%02d", h, m, s);
            else swprintf_s(buf, L" %d:%02d", m, s);
            SetWindowText(hTimeTot, buf);
        } else if (hTimeTot) {
            SetWindowText(hTimeTot, L"");
        }
        if (!g_recording && hTimeRemain) SetWindowText(hTimeRemain, L"");
        return;
    }
    double pos = GetPlayPos(), len = GetTrackLength(), rem = len - pos;
    wchar_t buf[32];
    if (hTimeCur) { FormatTime(pos, buf, _countof(buf)); SetWindowText(hTimeCur, buf); }
    if (!g_recording && hTimeRemain && rem >= 0) {
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

    // Seek to that position
    _fseeki64(d->fp, d->dataOffset + byteOff, SEEK_SET);

    // Scan forward to find the next valid DTS sync word so we don't
    // land in the middle of a frame (which would produce noise/errors)
    int64_t remaining = d->dataSize - byteOff;
    if (remaining > 0 && !DcaScanSync(d->fp, min(remaining, (int64_t)65536))) {
        // If no sync found, rewind to start of data as fallback
        _fseeki64(d->fp, d->dataOffset, SEEK_SET);
    }

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
    // Emit the OUTPUT channel count (clamped to 2), not the source count — the
    // BASS stream is stereo, so writing 6 interleaved channels here would be
    // read back as scrambled/garbage stereo. Fall back to nChannels if the
    // stream hasn't been created yet (shouldn't happen: meta runs first).
    int ch = d->outChannels > 0 ? d->outChannels : min(d->nChannels, 2);
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

    // Interleave the first `ch` channels into the float buffer (drops surround
    // channels beyond stereo; clamps if the frame has fewer than `ch`).
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
            int ec = d->outChannels > 0 ? d->outChannels : d->nChannels;
            d->samplesOut += copy / sizeof(float) / ec;
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

    // Clamp to stereo output — the write callback emits exactly this many
    // channels, so the BASS stream and the PCM it's fed always agree.
    int outCh = min(d->nChannels, 2);
    d->outChannels = outCh;

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
    SelectPlayingTrack(idx);

    // Create decode stream (DTS-WAV, FLAC via libFLAC, or normal BASS)
    HSTREAM dec = 0;
    // Check if this is a URL (radio stream) — handle differently
    bool isUrl = (wcsncmp(g_playlist[idx].path, L"http://", 7) == 0 ||
                  wcsncmp(g_playlist[idx].path, L"https://", 8) == 0 ||
                  _wcsnicmp(g_playlist[idx].path, L"ftp://", 6) == 0);
    if (isUrl) {
        // Stream URL — connect on a worker thread so slow/dead streams don't
        // freeze the UI. RadioOnConnected finishes setup when the result arrives.
        // Extract station name from display (strip ▶ prefix)
        const wchar_t* dispName = g_playlist[idx].display;
        if (dispName[0] == L'\u25B6' && dispName[1] == L' ') dispName += 2;
        wcsncpy_s(g_radioStationName, dispName, _TRUNCATE);
        g_radioNowPlaying[0] = L'\0';
        wcsncpy_s(g_radioReconnUrl, g_playlist[idx].path, _TRUNCATE);
        g_radioReconnAttempt = 0;
        g_radioReconnCountdown = 0;
        g_loudStreamStep = 2;   // fresh stream always starts at step 2
        wchar_t connTitle[512];
        _snwprintf_s(connTitle, _countof(connTitle), _TRUNCATE, L"%s (connecting...) - BillyPro", g_radioStationName);
        SetWindowText(g_hwnd, connTitle);
        if (!g_browserActive) {
            SendMessage(hListBox, LB_SETSEL, FALSE, (LPARAM)-1);
            SendMessage(hListBox, LB_SETSEL, TRUE, (LPARAM)idx);
            SendMessage(hListBox, LB_SETCURSEL, idx, 0);
        }
        UpdateStatusBar();
        RecordPlayHistory(g_playlist[idx].path, dispName);
        RadioConnectAsync(g_playlist[idx].path);
        return;
    }

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
        // Track.path is up to 1023 chars — buffer must fit it, and truncate
        // rather than abort if it somehow doesn't.
        wchar_t msg[1200];
        _snwprintf_s(msg, _countof(msg), _TRUNCATE, L"Cannot open:\n%s\n\nBASS error: %d",
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

    // (Loud makeup gain is set inside ApplyDSP below — decode-stream VOL is
    // ignored by BASS, so the gain lives in DSP_Limiter on the master.)

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

    // SMTC: update lock screen / media overlay
    SmtcUpdateMetadata();
    SmtcUpdatePlaybackStatus();

    // Pre-load next track as decode stream
    PreloadNext();

    // Record this local track in the play history
    RecordPlayHistory(g_playlist[idx].path, g_playlist[idx].display);

    // Repaint listbox to update current-track highlight
    if (hListBox) InvalidateRect(hListBox, NULL, FALSE);
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
    SmtcUpdatePlaybackStatus();
}

void PlayNext()
{
    // Radio has no queue — step to the next saved radio favorite instead
    if (g_radioPlaying) { RadioPlayFavOffset(+1); return; }
    if (g_playlist.empty()) return;
    // Play Once: stop after current track finishes
    if (g_repeatMode == 3) { StopAudio(); return; }
    // Repeat Track: replay current
    if (g_repeatMode == 1) { PlayIndex(g_currentIndex); return; }
    int next;
    if (g_shuffle && !g_shuffleOrder.empty()) {
        int p = 0;
        for (int i = 0; i < (int)g_shuffleOrder.size(); ++i)
            if (g_shuffleOrder[i] == g_currentIndex) { p = i; break; }
        next = g_shuffleOrder[(p + 1) % g_shuffleOrder.size()];
    }
    else {
        next = g_currentIndex + 1;
        if (next >= (int)g_playlist.size()) {
            if (g_repeatMode == 2) next = 0; else { StopAudio(); return; }
        }
    }
    PlayIndex(next);
}

void PlayPrev()
{
    // Radio has no queue — step to the previous saved radio favorite instead
    if (g_radioPlaying) { RadioPlayFavOffset(-1); return; }
    if (g_playlist.empty()) return;
    if (currentStream && GetPlayPos() > 3.0) { SeekToSeconds(0); return; }
    int prev;
    if (g_shuffle && !g_shuffleOrder.empty()) {
        int p = 0;
        for (int i = 0; i < (int)g_shuffleOrder.size(); ++i)
            if (g_shuffleOrder[i] == g_currentIndex) { p = i; break; }
        prev = g_shuffleOrder[(p - 1 + (int)g_shuffleOrder.size()) % g_shuffleOrder.size()];
    }
    else {
        prev = g_currentIndex - 1;
        if (prev < 0) prev = (g_repeatMode == 2) ? (int)g_playlist.size() - 1 : 0;
    }
    PlayIndex(prev);
}

// Compute the next track index (-1 if none)
static int ComputeNextIndex()
{
    if (g_playlist.empty() || g_currentIndex < 0) return -1;
    if (g_repeatMode == 3) return -1; // play once: no next track
    if (g_repeatMode == 1) return g_currentIndex; // repeat track
    if (g_shuffle && !g_shuffleOrder.empty()) {
        int p = 0;
        for (int i = 0; i < (int)g_shuffleOrder.size(); ++i)
            if (g_shuffleOrder[i] == g_currentIndex) { p = i; break; }
        return g_shuffleOrder[(p + 1) % g_shuffleOrder.size()];
    }
    int next = g_currentIndex + 1;
    if (next >= (int)g_playlist.size()) {
        if (g_repeatMode == 2) return 0; // repeat playlist
        return -1; // end of playlist, no repeat
    }
    return next;
}

void PreloadNext()
{
    // Free any previously pre-loaded decode stream. Do it under the master lock:
    // GaplessProc (mixing thread) reads g_decNext and may adopt it into
    // g_decStream, so freeing/clearing it unlocked is a use-after-free race.
    // While g_decNext == 0 the mixing thread just takes the "no next" path.
    HSTREAM master = currentStream;
    if (master) BASS_ChannelLock(master, TRUE);
    if (g_decNext) { BASS_StreamFree(g_decNext); g_decNext = 0; }
    g_decNextIdx = -1;
    if (master) BASS_ChannelLock(master, FALSE);
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

    // Warm the loudness-scan cache for the next track so the gapless
    // transition can pick up its makeup gain instantly
    if (g_normalize)
        ComputeNormGain(g_playlist[next].path);

    // Publish the new pre-loaded stream under the master lock so GaplessProc
    // sees g_decNext and g_decNextIdx update together, never a torn pair.
    if (master) BASS_ChannelLock(master, TRUE);
    g_decNext = dec;
    g_decNextIdx = next;
    if (master) BASS_ChannelLock(master, FALSE);
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
    wcsncpy_s(nid.szTip, L"BillyPro", _TRUNCATE);
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
        // szTip is only WCHAR[128]; display can be 259 — must truncate, not abort
        _snwprintf_s(nid.szTip, _countof(nid.szTip), _TRUNCATE,
            L"BillyPro - %s", g_playlist[g_currentIndex].display);
    else
        wcsncpy_s(nid.szTip, L"BillyPro", _TRUNCATE);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void ShowTrayContextMenu(HWND hwnd)
{
    POINT pt; GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_RESTORE, L"Restore BillyPro");
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

// Append "buffering X / Y MB (Z%)" (or "cached Y MB") when a direct
// stream link is playing — BASS caches the download progressively.
static void AppendStreamBufferInfo(wchar_t* buf, size_t n)
{
    if (!currentStream || g_decStream || g_radioPlaying) return;
    QWORD dl = BASS_StreamGetFilePosition(currentStream, BASS_FILEPOS_DOWNLOAD);
    QWORD sz = BASS_StreamGetFilePosition(currentStream, BASS_FILEPOS_SIZE);
    if (dl == (QWORD)-1 || sz == (QWORD)-1 || sz == 0) return;
    wchar_t dbuf[96];
    if (dl >= sz)
        swprintf_s(dbuf, L"  |  cached %.1f MB", sz / 1048576.0);
    else
        swprintf_s(dbuf, L"  |  buffering %.1f / %.1f MB (%d%%)",
            dl / 1048576.0, sz / 1048576.0, (int)(dl * 100 / sz));
    wcsncat_s(buf, n, dbuf, _TRUNCATE);
}

// Size the status bar panels: the right panel gets extra room while a
// direct stream is buffering so the cache info isn't cut off.
static void StatusSetParts()
{
    if (!hStatus || !g_hwnd) return;
    RECT rc; GetClientRect(g_hwnd, &rc);
    int rw = (currentStream && !g_decStream && !g_radioPlaying) ? 460 : 280;
    if (rc.right - rw < 120) rw = rc.right - 120;   // keep some left panel
    int parts[2] = { rc.right - rw, -1 };
    SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)parts);
}

void UpdateStatusBar()
{
    if (!hStatus) return;
    StatusSetParts();
    // Reset scroll state so it re-evaluates on every status update
    g_stLastText[0] = 0;
    g_stScrollX = 0;
    g_stPause = 30;
    wchar_t left[256] = L"Ready", right[256] = L"";

    if (!g_playlist.empty() && !currentStream)
        swprintf_s(left, L"%d Files", (int)g_playlist.size());

    if (currentStream) {
        if (g_radioPlaying) {
            // Radio mode: show station + now playing
            if (g_radioNowPlaying[0])
                _snwprintf_s(left, _countof(left), _TRUNCATE, L"\u25B6 %s  |  %s", g_radioStationName, g_radioNowPlaying);
            else
                _snwprintf_s(left, _countof(left), _TRUNCATE, L"\u25B6 %s", g_radioStationName);
        } else if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
            // _TRUNCATE: display can be up to 259 chars (URL-derived names) —
            // plain swprintf_s would abort the process on overflow. This was
            // the "play a stream favorite -> crash" bug.
            _snwprintf_s(left, _countof(left), _TRUNCATE, L"%d / %d  %s",
                g_currentIndex + 1, (int)g_playlist.size(),
                g_playlist[g_currentIndex].display);
        // Query the decode stream — the master output stream is always 32-bit
        // float and carries no file bitrate; the decode stream reflects the file.
        HSTREAM infoSrc = g_decStream ? g_decStream : currentStream;
        BASS_CHANNELINFO info = {};
        if (BASS_ChannelGetInfo(infoSrc, &info)) {
            float brate = 0;
            BASS_ChannelGetAttribute(infoSrc, BASS_ATTRIB_BITRATE, &brate);

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

            // For WAV: show bit depth (mask out BASS_ORIGRES_FLOAT flag 0x10000)
            if (wcscmp(fmt, L"WAV") == 0) {
                int bits = (int)(info.origres & 0xFFFF);
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
    // Keep the base text so the buffer info can be refreshed live (WM_TIMER)
    // without resetting the left panel's scrolling
    wcsncpy_s(g_stRightBase, right, _TRUNCATE);
    AppendStreamBufferInfo(right, _countof(right));
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

    // Direct stream: show the downloaded/cached portion as a dimmer band
    // behind the playback fill (like a video player's buffer bar)
    if (currentStream && !g_decStream && !g_radioPlaying) {
        QWORD dl = BASS_StreamGetFilePosition(currentStream, BASS_FILEPOS_DOWNLOAD);
        QWORD sz = BASS_StreamGetFilePosition(currentStream, BASS_FILEPOS_SIZE);
        if (dl != (QWORD)-1 && sz != (QWORD)-1 && sz > 0) {
            int bw = (int)((double)dl / (double)sz * W);
            if (bw > W) bw = W;
            if (bw > 0) {
                RECT br2 = { 0, 0, bw, H };
                // Muted slate blue — clearly visible against the empty track
                HBRUSH brB = CreateSolidBrush(g_darkMode ? (COLORREF)0x008A5A34
                                                         : (COLORREF)0x00F0C896);
                FillRect(dc, &br2, brB); DeleteObject(brB);
            }
        }
    }

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

enum BtnType { BTN_PREV, BTN_PLAY, BTN_PAUSE, BTN_STOP, BTN_NEXT, BTN_SHUFFLE, BTN_REPEAT, BTN_MONO, BTN_NORMALIZE, BTN_BASSBOOST, BTN_MEDIA, BTN_DSP, BTN_BACK, BTN_RECORD };

static void DrawBtn(DRAWITEMSTRUCT* dis, BtnType type)
{
    HDC  dc = dis->hDC; RECT rc = dis->rcItem;
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    bool toggled = (type == BTN_SHUFFLE && g_shuffle)
        || (type == BTN_REPEAT && g_repeatMode != 0)
        || (type == BTN_MONO && g_mono)
        || (type == BTN_NORMALIZE && g_normalize)
        || (type == BTN_BASSBOOST && g_bassBoost)
        || (type == BTN_MEDIA && g_mediaActive)
        || (type == BTN_DSP && !g_dspBypass && (g_dspReverb || g_dspSaturate || g_dspVinyl || g_dspHifi))
        || (type == BTN_RECORD && g_recording)
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
    case BTN_RECORD: {
        // Record circle — hover highlight matches transport buttons (blue tint)
        bool recHover = (g_hoveredBtn == dis->hwndItem);
        COLORREF recFace = toggled ? g_theme.accent
            : pressed ? 0xC8DCF4
            : recHover ? 0xF0D2B4
            : g_theme.bg;
        HBRUSH recBg = CreateSolidBrush(recFace); FillRect(dc, &rc, recBg); DeleteObject(recBg);
        COLORREF circCol = g_recording ? 0x0000FF
            : (recHover || pressed) ? 0x1A1A1A
            : (g_darkMode ? 0x909090 : 0x1A1A1A);
        HBRUSH rb = CreateSolidBrush(circCol);
        HPEN rp = CreatePen(PS_SOLID, 1, circCol);
        HBRUSH orb = (HBRUSH)SelectObject(dc, rb);
        HPEN orp = (HPEN)SelectObject(dc, rp);
        Ellipse(dc, cx - 4, cy - 4, cx + 5, cy + 5);
        SelectObject(dc, orb); SelectObject(dc, orp);
        DeleteObject(rb); DeleteObject(rp);
        break;
    }
    case BTN_MONO:
    case BTN_NORMALIZE:
    case BTN_BASSBOOST:
    case BTN_DSP:
    case BTN_SHUFFLE:
    case BTN_REPEAT:
    case BTN_MEDIA:
    case BTN_BACK: {
        sym = (pressed || toggled) ? 0xFFFFFF : (hovered && g_darkMode) ? 0x1A1A1A : g_theme.text;  // black text on hover in dark mode for contrast
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
        case BTN_REPEAT:     lbl = (g_repeatMode == 1) ? L"Rep.1" : (g_repeatMode == 2) ? L"Rep.All" : (g_repeatMode == 3) ? L"Once" : L"Repeat"; break;
        case BTN_MONO:       lbl = L"Mono";    break;
        case BTN_NORMALIZE:  lbl = L"Loud";    break;
        case BTN_BASSBOOST:  lbl = L"BassBoost"; break;
        case BTN_MEDIA:      lbl = L"Library"; break;
        case BTN_DSP:        lbl = L"DSP";     break;
        case BTN_BACK:       lbl = L"\u2190";  break;  // ←
        // BTN_RECORD handled above as transport button (no border)
        default:             lbl = L"";        break;
        }
        DrawText(dc, lbl, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, of); DeleteObject(hf);
        break;
    }
    }
}

// ============================================================
//  Loud stream-intensity slider popup (right-click Loud while
//  streaming) — 5 steps, up = more compression/boost
// ============================================================
#define ID_LOUDSLIDER_TRACK 770
static HWND g_hwndLoudSlider = NULL;

static LRESULT CALLBACK LoudSliderProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        HWND hPlus = CreateWindow(L"STATIC", L"+", WS_CHILD | WS_VISIBLE | SS_CENTER,
            4, 4, 38, 14, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hPlus, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        // Vertical trackbar increases downward, so positions are inverted (top = 5)
        HWND hT = CreateWindow(TRACKBAR_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | TBS_VERT | TBS_AUTOTICKS | TBS_BOTH,
            6, 20, 34, 106, hwnd, (HMENU)ID_LOUDSLIDER_TRACK, hInst, NULL);
        SendMessage(hT, TBM_SETRANGE, TRUE, MAKELONG(1, 5));
        SendMessage(hT, TBM_SETTICFREQ, 1, 0);
        SendMessage(hT, TBM_SETPOS, TRUE, 6 - max(1, min(5, g_loudStreamStep)));
        HWND hMinus = CreateWindow(L"STATIC", L"-", WS_CHILD | WS_VISIBLE | SS_CENTER,
            4, 128, 38, 14, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hMinus, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        SetFocus(hT);
        break;
    }
    case WM_VSCROLL: {
        HWND hT = GetDlgItem(hwnd, ID_LOUDSLIDER_TRACK);
        int pos = (int)SendMessage(hT, TBM_GETPOS, 0, 0);
        g_loudStreamStep = max(1, min(5, 6 - pos));   // live — DSP reads it directly
        break;
    }
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) DestroyWindow(hwnd);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
        break;
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_DESTROY:
        g_hwndLoudSlider = NULL;
        SaveSettings();
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenLoudSliderPopup()
{
    if (g_hwndLoudSlider && IsWindow(g_hwndLoudSlider)) { DestroyWindow(g_hwndLoudSlider); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t LOUDSLIDER_CLASS[] = L"BillyLoudSlider";
    static bool lsReg = false;
    if (!lsReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = LoudSliderProc; wc.hInstance = hInst;
        wc.lpszClassName = LOUDSLIDER_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.style = CS_DROPSHADOW;
        wc.hbrBackground = NULL; RegisterClass(&wc); lsReg = true;
    }
    RECT br; GetWindowRect(hNormalizeBtn, &br);
    int w = 46, h = 148;
    int x = br.left + ((br.right - br.left) - w) / 2;
    int y = br.bottom + 2;
    g_hwndLoudSlider = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, LOUDSLIDER_CLASS,
        NULL, WS_POPUP | WS_BORDER | WS_VISIBLE, x, y, w, h, g_hwnd, NULL, hInst, NULL);
    SetForegroundWindow(g_hwndLoudSlider);
}

// One-time balloon after the first Loud click: tells the user about
// the right-click options (hidden automatically via IDT_LOUD_HINT).
static HWND g_hwndLoudHint = NULL;

static void ShowLoudHintBalloon()
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    if (!g_hwndLoudHint) {
        g_hwndLoudHint = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
            WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            g_hwnd, NULL, hInst, NULL);
        if (!g_hwndLoudHint) return;
        TOOLINFO ti = { sizeof(ti) };
        ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
        ti.hwnd = g_hwnd;
        ti.uId = 1;
        ti.lpszText = (LPWSTR)L"Right-click this button for more options:\nloudness modes for files, intensity slider for streams.";
        SendMessage(g_hwndLoudHint, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(g_hwndLoudHint, TTM_SETTITLE, TTI_INFO, (LPARAM)L"Loud");
        SendMessage(g_hwndLoudHint, TTM_SETMAXTIPWIDTH, 0, 300);
    }
    RECT br; GetWindowRect(hNormalizeBtn, &br);
    SendMessage(g_hwndLoudHint, TTM_TRACKPOSITION, 0,
        MAKELPARAM((br.left + br.right) / 2, br.bottom + 4));
    TOOLINFO ti = { sizeof(ti) };
    ti.hwnd = g_hwnd; ti.uId = 1;
    SendMessage(g_hwndLoudHint, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
    SetTimer(g_hwnd, IDT_LOUD_HINT, 7000, NULL);
}

static void HideLoudHintBalloon()
{
    if (!g_hwndLoudHint) return;
    TOOLINFO ti = { sizeof(ti) };
    ti.hwnd = g_hwnd; ti.uId = 1;
    SendMessage(g_hwndLoudHint, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
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
    // Right-click on Loud button: intensity slider while streaming,
    // otherwise the Soft/Normal/Modern/Loud mode picker
    if (msg == WM_RBUTTONUP && hwnd == hNormalizeBtn) {
        if (currentStream && !g_decStream) {   // radio or direct stream link
            OpenLoudSliderPopup();
            return 0;
        }
        static const wchar_t* loudModes[4] = { L"Soft", L"Normal", L"Modern", L"Loud" };
        HMENU hm = CreatePopupMenu();
        for (int i = 0; i < 4; i++)
            AppendMenu(hm, MF_STRING | (g_loudMode == i ? MF_CHECKED : 0), i + 1, loudModes[i]);
        POINT pt; GetCursorPos(&pt);
        int cmd = TrackPopupMenu(hm, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_hwnd, NULL);
        DestroyMenu(hm);
        if (cmd >= 1 && cmd <= 4 && g_loudMode != cmd - 1) {
            g_loudMode = cmd - 1;
            s_normCachePath[0] = 0;   // re-scan the current track with the new target
            ApplyDSP();
            SaveSettings();
            InvalidateRect(hNormalizeBtn, NULL, TRUE);
        }
        return 0;
    }
    // Right-click on Repeat button: show mode picker
    if (msg == WM_RBUTTONUP && hwnd == hRepeatBtn) {
        HMENU hm = CreatePopupMenu();
        AppendMenu(hm, MF_STRING | (g_repeatMode == 0 ? MF_CHECKED : 0), 1, L"Repeat Off");
        AppendMenu(hm, MF_STRING | (g_repeatMode == 1 ? MF_CHECKED : 0), 2, L"Repeat Track");
        AppendMenu(hm, MF_STRING | (g_repeatMode == 2 ? MF_CHECKED : 0), 3, L"Repeat Playlist");
        AppendMenu(hm, MF_STRING | (g_repeatMode == 3 ? MF_CHECKED : 0), 4, L"Play Once");
        POINT pt; GetCursorPos(&pt);
        int cmd = TrackPopupMenu(hm, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_hwnd, NULL);
        DestroyMenu(hm);
        if (cmd >= 1 && cmd <= 4) {
            g_repeatMode = cmd - 1;
            CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_REPEAT,
                MF_BYCOMMAND | (g_repeatMode ? MF_CHECKED : MF_UNCHECKED));
            InvalidateRect(hRepeatBtn, NULL, TRUE);
            PreloadNext();
        }
        return 0;
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

    // Rebuild listbox with star prefixes (RefreshListboxStars handles WM_SETREDRAW internally)
    RefreshListboxStars();
    SendMessage(hListBox, LB_SETCURSEL, destIdx, 0);
    SendMessage(hListBox, LB_SETTOPINDEX, topIdx, 0);
    RebuildShuffleOrder();
    UpdateStatusBar();
    // Favorites view: persist the new order to the favorites file
    if (g_favActive) {
        g_favorites.clear();
        for (auto& t : g_playlist) g_favorites.push_back(t.path);
        SaveFavorites();
    }
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
                else { PlayIndex(s); PlayHistResumeSeek(s); }
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
            bool favChanged = false;
            for (int idx : toDelete) {
                if (idx >= 0 && idx < (int)g_playlist.size()) {
                    if (idx == g_currentIndex) stopNeeded = true;
                    // Favorites view: deleting a row also removes the favorite itself
                    if (g_favActive) {
                        for (auto it = g_favorites.begin(); it != g_favorites.end(); ++it)
                            if (_wcsicmp(it->c_str(), g_playlist[idx].path) == 0) {
                                g_favNames.erase(*it);
                                g_favorites.erase(it); favChanged = true; break;
                            }
                    }
                    g_playlist.erase(g_playlist.begin() + idx);
                    SendMessage(hwnd, LB_DELETESTRING, idx, 0);
                }
            }
            if (favChanged) SaveFavorites();
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
                // Clamp to first/last of whichever list is showing
                RECT rc; GetClientRect(hwnd, &rc);
                int count = (int)(g_browserActive ? g_browserItems.size() : g_playlist.size());
                idx = (GET_Y_LPARAM(lParam) < 0) ? 0 : count - 1;
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
            // Only the real playlist can be reordered. In browser mode the list
            // shows g_browserItems, so ReorderTrack(from,to) would scramble the
            // hidden playlist by browser-row indices — a browser drag is a no-op.
            if (!g_browserActive && g_lbDragMoved && from != to && from >= 0 && to >= 0) {
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
        if (idx == LB_ERR) {
            // Empty area (or empty playlist): quick actions
            if (!g_browserActive) {
                POINT screen = pt; ClientToScreen(hwnd, &screen);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, IDM_FILE_OPENSTREAM, L"Open Stream Link...");
                AppendMenu(hMenu, MF_STRING, IDM_FILE_REFRESH,    L"Refresh Playlist\tShift+R");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, IDM_FILE_OPENFOLDER, L"Open Folder...");
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, screen.x, screen.y, 0, g_hwnd, NULL);
                DestroyMenu(hMenu);
            }
            return 0;
        }

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
        for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++) {
            wchar_t esc[300]; MenuEscapeAmp(g_bppPlaylists[i].name.c_str(), esc, 300);
            AppendMenu(hPlSub, MF_STRING, IDC_CTX_PL_BASE + i, esc);
        }
        if (!g_bppPlaylists.empty())
            AppendMenu(hPlSub, MF_SEPARATOR, 0, NULL);
        AppendMenu(hPlSub, MF_STRING, IDC_CTX_PL_NEW, L"New Playlist...");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPlSub, L"Add to Playlist");
    }
    // Case-insensitive + all schemes, else "HTTP://..." rows get the
    // local-file menu (Explorer /select on a URL, dead Open-in-Billy).
    bool isUrl = IsUrlPath(g_ctxFilePath);
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    // Sets.LIVE streams can be downloaded to disk
    if (IsSetsUrl(g_ctxFilePath))
        AppendMenu(hMenu, MF_STRING, IDC_CTX_DOWNLOAD, L"Download...");
    if (!isUrl) {
        AppendMenu(hMenu, MF_STRING, IDC_CTX_AUDIOINFO,    L"Audio Information...");
        AppendMenu(hMenu, MF_STRING, IDC_CTX_PROPERTIES,   L"Properties...");
        AppendMenu(hMenu, MF_STRING, IDC_CTX_OPENLOCATION, L"Open File Location");
        AppendMenu(hMenu, MF_STRING, IDC_CTX_OPENINBILLY,  L"Open File Location in Billy");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    } else {
        // Stream links: format/tag info can still be read from the stream
        AppendMenu(hMenu, MF_STRING, IDC_CTX_AUDIOINFO, L"Audio Information...");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    }
    AppendMenu(hMenu, MF_STRING, IDC_CTX_REMOVE, L"Remove");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_FILE_REFRESH, L"Refresh Playlist\tShift+R");

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

// ============================================================
//  Artwork thumbnail cache for Modern Style listbox
// ============================================================
static int GetThumbSz() { static const int sizes[] = {32, 48, 64}; return sizes[max(0, min(2, g_modernSize))]; }
#define THUMB_SZ GetThumbSz()
// Cache: path -> HBITMAP (NULL = no artwork, use placeholder)
// Special sentinel: (HBITMAP)1 means "not yet loaded"
static std::map<std::wstring, HBITMAP> g_thumbCache;
static HBITMAP g_thumbPlaceholder = NULL;  // music note placeholder

static void ClearThumbCache()
{
    for (auto& p : g_thumbCache)
        if (p.second && p.second != (HBITMAP)1) DeleteObject(p.second);
    g_thumbCache.clear();
}

static HBITMAP CreatePlaceholderBitmap()
{
    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = THUMB_SZ;
    bi.bmiHeader.biHeight = -THUMB_SZ;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = NULL;
    HBITMAP hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    HBITMAP old = (HBITMAP)SelectObject(hdc, hbm);
    // Fill with contrasting background
    RECT rc = { 0, 0, THUMB_SZ, THUMB_SZ };
    HBRUSH br = CreateSolidBrush(g_darkMode ? 0x484848 : 0xCCCCCC);
    FillRect(hdc, &rc, br); DeleteObject(br);
    // Border for visibility
    HPEN pen = CreatePen(PS_SOLID, 1, g_darkMode ? 0x606060 : 0xAAAAAA);
    HPEN op = (HPEN)SelectObject(hdc, pen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, THUMB_SZ, THUMB_SZ);
    SelectObject(hdc, op); DeleteObject(pen);
    // Draw music note symbol
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, g_darkMode ? 0xBBBBBB : 0x666666);
    HFONT hf = CreateFont(-20, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
    HFONT of = (HFONT)SelectObject(hdc, hf);
    DrawText(hdc, L"\u266B", 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE); // ♫
    SelectObject(hdc, of); DeleteObject(hf);
    SelectObject(hdc, old);
    DeleteDC(hdc);
    ReleaseDC(NULL, hdcScreen);
    return hbm;
}

// Get or load artwork thumbnail for a file path (returns placeholder if no artwork)
// Decode raw image bytes (JPEG/PNG) to an HBITMAP scaled to size×size via WIC
static HBITMAP DecodeImageToThumb(const BYTE* imgData, DWORD imgSize, int size)
{
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, imgSize);
    if (!hg) return NULL;
    void* mem = GlobalLock(hg);
    if (!mem) { GlobalFree(hg); return NULL; }
    memcpy(mem, imgData, imgSize);
    GlobalUnlock(hg);

    HBITMAP result = NULL;
    IStream* pStream = NULL;
    if (SUCCEEDED(CreateStreamOnHGlobal(hg, FALSE, &pStream))) {
        IWICImagingFactory* pFact = NULL;
        CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory, (void**)&pFact);
        if (pFact) {
            IWICBitmapDecoder* pDec = NULL;
            pFact->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDec);
            if (pDec) {
                IWICBitmapFrameDecode* pFrame = NULL;
                pDec->GetFrame(0, &pFrame);
                if (pFrame) {
                    IWICBitmapScaler* pScale = NULL;
                    pFact->CreateBitmapScaler(&pScale);
                    if (pScale) {
                        pScale->Initialize(pFrame, size, size, WICBitmapInterpolationModeCubic);
                        IWICFormatConverter* pConv = NULL;
                        pFact->CreateFormatConverter(&pConv);
                        if (pConv) {
                            pConv->Initialize(pScale, GUID_WICPixelFormat32bppBGRA,
                                WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
                            HDC hdc = GetDC(NULL);
                            BITMAPINFO bi = {};
                            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                            bi.bmiHeader.biWidth = size; bi.bmiHeader.biHeight = -size;
                            bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32;
                            BYTE* bits = NULL;
                            result = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
                            ReleaseDC(NULL, hdc);
                            if (result && bits)
                                pConv->CopyPixels(NULL, size * 4, size * size * 4, bits);
                            pConv->Release();
                        }
                        pScale->Release();
                    }
                    pFrame->Release();
                }
                pDec->Release();
            }
            pFact->Release();
        }
        pStream->Release();
    }
    GlobalFree(hg);
    return result;
}

// Read ID3v2 APIC artwork directly from file (no BASS, no temp files, no globals)
// Try to load artwork from ID3v2 tags (MP3, some WAV)
static HBITMAP LoadThumbFromID3v2(FILE* f, int size)
{
    BYTE hdr[10];
    fseek(f, 0, SEEK_SET);
    if (fread(hdr, 1, 10, f) != 10 || memcmp(hdr, "ID3", 3) != 0) return NULL;
    DWORD tagSize = ((hdr[6] & 0x7F) << 21) | ((hdr[7] & 0x7F) << 14) |
        ((hdr[8] & 0x7F) << 7) | (hdr[9] & 0x7F);
    BYTE ver = hdr[3];
    if (tagSize > 8 * 1024 * 1024) return NULL;
    std::vector<BYTE> tag(tagSize);
    if (fread(tag.data(), 1, tagSize, f) != tagSize) return NULL;

    const BYTE* p = tag.data();
    const BYTE* end = p + tagSize;
    while (p + 10 < end) {
        char fid[5] = {}; memcpy(fid, p, 4);
        DWORD fsize;
        if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
        else          fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
        p += 10;
        if (fsize == 0 || p + fsize > end) break;
        if (strcmp(fid, "APIC") == 0 && fsize > 10) {
            const BYTE* fp = p; fp++;
            while (fp < p + fsize && *fp) fp++;
            if (fp < p + fsize) fp++;
            if (fp < p + fsize) fp++;
            while (fp < p + fsize && *fp) fp++;
            if (fp < p + fsize) fp++;
            DWORD imgSize = (DWORD)(p + fsize - fp);
            if (imgSize > 100) return DecodeImageToThumb(fp, imgSize, size);
        }
        p += fsize;
    }
    return NULL;
}

// Try to load artwork from FLAC metadata (METADATA_BLOCK_PICTURE, type 6)
static HBITMAP LoadThumbFromFLAC(FILE* f, int size)
{
    BYTE magic[4];
    fseek(f, 0, SEEK_SET);
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "fLaC", 4) != 0) return NULL;

    // Parse metadata blocks
    while (true) {
        BYTE blockHdr[4];
        if (fread(blockHdr, 1, 4, f) != 4) return NULL;
        bool isLast = (blockHdr[0] & 0x80) != 0;
        int blockType = blockHdr[0] & 0x7F;
        DWORD blockLen = ((DWORD)blockHdr[1] << 16) | ((DWORD)blockHdr[2] << 8) | blockHdr[3];

        if (blockType == 6 && blockLen > 32) { // PICTURE block
            std::vector<BYTE> block(blockLen);
            if (fread(block.data(), 1, blockLen, f) != blockLen) return NULL;
            const BYTE* bp = block.data();
            // PICTURE block format: pictureType(4) + mimeLen(4) + mime + descLen(4) + desc +
            //                       width(4) + height(4) + depth(4) + colors(4) + dataLen(4) + data
            if (blockLen < 32) return NULL;
            DWORD mimeLen = ((DWORD)bp[4] << 24) | ((DWORD)bp[5] << 16) | ((DWORD)bp[6] << 8) | bp[7];
            DWORD off = 8 + mimeLen;
            if (off + 4 > blockLen) return NULL;
            DWORD descLen = ((DWORD)bp[off] << 24) | ((DWORD)bp[off+1] << 16) | ((DWORD)bp[off+2] << 8) | bp[off+3];
            off += 4 + descLen;
            if (off + 20 > blockLen) return NULL;
            off += 16; // skip width, height, depth, colors
            DWORD dataLen = ((DWORD)bp[off] << 24) | ((DWORD)bp[off+1] << 16) | ((DWORD)bp[off+2] << 8) | bp[off+3];
            off += 4;
            if (off + dataLen > blockLen || dataLen < 100) return NULL;
            return DecodeImageToThumb(bp + off, dataLen, size);
        } else {
            fseek(f, blockLen, SEEK_CUR);
        }
        if (isLast) break;
    }
    return NULL;
}

// Try to load artwork from WAV file (check for ID3v2 chunk inside RIFF)
static HBITMAP LoadThumbFromWAV(FILE* f, int size)
{
    BYTE hdr[12];
    fseek(f, 0, SEEK_SET);
    if (fread(hdr, 1, 12, f) != 12 || memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) return NULL;

    // Search for "id3 " or "ID3 " or "ID32" chunk inside the WAV
    while (true) {
        BYTE ch[8];
        if (fread(ch, 1, 8, f) != 8) break;
        DWORD sz = ch[4] | (ch[5] << 8) | (ch[6] << 16) | (ch[7] << 24);
        if (sz == 0 || sz > 50 * 1024 * 1024) break; // sanity check
        bool isId3 = (memcmp(ch, "id3 ", 4) == 0 || memcmp(ch, "ID3 ", 4) == 0 || memcmp(ch, "ID32", 4) == 0);
        if (isId3 && sz > 10) {
            // Read the ID3v2 data from the chunk directly (don't seek to file start)
            long chunkStart = ftell(f);
            BYTE id3hdr[10];
            if (fread(id3hdr, 1, 10, f) == 10 && memcmp(id3hdr, "ID3", 3) == 0) {
                DWORD tagSize = ((id3hdr[6] & 0x7F) << 21) | ((id3hdr[7] & 0x7F) << 14) |
                    ((id3hdr[8] & 0x7F) << 7) | (id3hdr[9] & 0x7F);
                BYTE ver = id3hdr[3];
                if (tagSize > 0 && tagSize <= sz && tagSize <= 8 * 1024 * 1024) {
                    std::vector<BYTE> tag(tagSize);
                    if (fread(tag.data(), 1, tagSize, f) == tagSize) {
                        const BYTE* p = tag.data();
                        const BYTE* end = p + tagSize;
                        while (p + 10 < end) {
                            char fid[5] = {}; memcpy(fid, p, 4);
                            DWORD fsize;
                            if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
                            else          fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
                            p += 10;
                            if (fsize == 0 || p + fsize > end) break;
                            if (strcmp(fid, "APIC") == 0 && fsize > 10) {
                                const BYTE* fp = p; fp++;
                                while (fp < p + fsize && *fp) fp++;
                                if (fp < p + fsize) fp++;
                                if (fp < p + fsize) fp++;
                                while (fp < p + fsize && *fp) fp++;
                                if (fp < p + fsize) fp++;
                                DWORD imgSize = (DWORD)(p + fsize - fp);
                                if (imgSize > 100) return DecodeImageToThumb(fp, imgSize, size);
                            }
                            p += fsize;
                        }
                    }
                }
            }
            fseek(f, chunkStart + ((sz + 1) & ~1), SEEK_SET);
        } else {
            fseek(f, (sz + 1) & ~1, SEEK_CUR); // chunks are word-aligned
        }
    }
    return NULL;
}

static HBITMAP LoadThumbFromFile(const wchar_t* path, int size)
{
    FILE* f = NULL;
    _wfopen_s(&f, path, L"rb");
    if (!f) return NULL;

    // Detect format from file header
    BYTE peek[4] = {};
    fread(peek, 1, 4, f);
    fseek(f, 0, SEEK_SET);

    HBITMAP result = NULL;
    if (memcmp(peek, "ID3", 3) == 0) {
        // MP3 with ID3v2
        result = LoadThumbFromID3v2(f, size);
    } else if (memcmp(peek, "fLaC", 4) == 0) {
        // FLAC
        result = LoadThumbFromFLAC(f, size);
    } else if (memcmp(peek, "RIFF", 4) == 0) {
        // WAV — check for embedded ID3v2 chunk
        result = LoadThumbFromWAV(f, size);
    }
    fclose(f);

    // Fallback: use BASS to read ID3v2 tags (handles non-standard tag locations in WAV/etc.)
    if (!result) {
        HSTREAM s = BASS_StreamCreateFile(FALSE, path, 0, 0,
            BASS_UNICODE | BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
        if (s) {
            const void* id3v2 = BASS_ChannelGetTags(s, BASS_TAG_ID3V2);
            if (id3v2) {
                const BYTE* data = (const BYTE*)id3v2;
                if (memcmp(data, "ID3", 3) == 0) {
                    DWORD tagSize = ((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) |
                        ((data[8] & 0x7F) << 7) | (data[9] & 0x7F);
                    BYTE ver = data[3];
                    const BYTE* p = data + 10;
                    const BYTE* end = data + 10 + tagSize;
                    while (p + 10 < end) {
                        char fid[5] = {}; memcpy(fid, p, 4);
                        DWORD fsize;
                        if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
                        else          fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
                        p += 10;
                        if (fsize == 0 || p + fsize > end) break;
                        if (strcmp(fid, "APIC") == 0 && fsize > 10) {
                            const BYTE* fp = p; fp++;
                            while (fp < p + fsize && *fp) fp++;
                            if (fp < p + fsize) fp++;
                            if (fp < p + fsize) fp++;
                            while (fp < p + fsize && *fp) fp++;
                            if (fp < p + fsize) fp++;
                            DWORD imgSize = (DWORD)(p + fsize - fp);
                            if (imgSize > 100) {
                                result = DecodeImageToThumb(fp, imgSize, size);
                                if (result) { BASS_StreamFree(s); return result; }
                            }
                        }
                        p += fsize;
                    }
                }
            }
            BASS_StreamFree(s);
        }
    }

    return result;
}

// Fetch embedded artwork from a stream URL on a worker thread — the draw
// handler must never block on the network. Result is posted back to the UI
// thread via WM_THUMB_READY, which stores it in g_thumbCache.
struct ThumbUrlJob { std::wstring key; int size; HBITMAP bm; };
static DWORD WINAPI ThumbUrlThread(LPVOID param)
{
    ThumbUrlJob* j = (ThumbUrlJob*)param;
    int uLen = WideCharToMultiByte(CP_UTF8, 0, j->key.c_str(), -1, NULL, 0, NULL, NULL);
    std::string url8(uLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, j->key.c_str(), -1, &url8[0], uLen, NULL, NULL);
    BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 8000);
    HSTREAM s = BASS_StreamCreateURL(url8.c_str(), 0, 0, NULL, NULL);
    if (s) {
        j->bm = LoadEmbeddedArtworkSized(s, j->size);
        BASS_StreamFree(s);
    }
    if (g_hwnd) PostMessage(g_hwnd, WM_THUMB_READY, (WPARAM)j, 0);
    else delete j;   // app shutting down — drop the result
    return 0;
}

static HBITMAP GetThumbForPath(const wchar_t* path)
{
    std::wstring key(path);
    auto it = g_thumbCache.find(key);
    if (it != g_thumbCache.end())
        return (it->second && it->second != (HBITMAP)1) ? it->second : g_thumbPlaceholder;

    // Stream URLs (Sets.LIVE favorites etc.): the artwork lives in the remote
    // file's tags — fetch it async, draw the placeholder meanwhile. The
    // (HBITMAP)1 sentinel marks "fetch in flight" so we only spawn once.
    if (IsUrlPath(path)) {
        g_thumbCache[key] = (HBITMAP)1;
        ThumbUrlJob* j = new ThumbUrlJob{ key, THUMB_SZ, NULL };
        HANDLE th = CreateThread(NULL, 0, ThumbUrlThread, j, 0, NULL);
        if (th) CloseHandle(th);
        else { g_thumbCache[key] = NULL; delete j; }   // thread failed — give up quietly
        return g_thumbPlaceholder;
    }

    // Read artwork directly from file — no BASS, no temp files
    HBITMAP bm = LoadThumbFromFile(path, THUMB_SZ);
    g_thumbCache[key] = bm;
    return bm ? bm : g_thumbPlaceholder;
}

static void RecreateListbox()
{
    if (!hListBox || !g_hwnd) return;
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);

    // Save state
    int topIdx = (int)SendMessage(hListBox, LB_GETTOPINDEX, 0, 0);
    int curSel = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);

    // Destroy old
    DestroyWindow(hListBox);

    // Create new with or without owner-draw
    DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL;
    style |= LBS_OWNERDRAWFIXED | LBS_HASSTRINGS; // always owner-draw for now-playing highlight

    hListBox = CreateWindowEx(0, L"LISTBOX", NULL, style,
        0, 0, 0, 0, g_hwnd, (HMENU)ID_LISTBOX, hInst, NULL);
    SendMessage(hListBox, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hListBox, LB_SETITEMHEIGHT, 0, g_modernStyle ? (THUMB_SZ + 4) : 20);

    // Subclass
    g_OldListProc = (WNDPROC)SetWindowLongPtr(hListBox, GWLP_WNDPROC, (LONG_PTR)ListBoxProc);

    // Theme
    SetWindowTheme(hListBox, g_darkMode ? L"DarkMode_Explorer" : L"Explorer", NULL);

    // Repopulate
    if (g_browserActive) {
        for (auto& bi : g_browserItems) {
            wchar_t disp[MAX_PATH + 4];
            if (!bi.isDir && IsFavorite(bi.path))
                swprintf_s(disp, L"\u2605 %s", bi.display);
            else
                wcsncpy_s(disp, bi.display, _TRUNCATE);
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
        }
    } else {
        for (auto& t : g_playlist) {
            wchar_t disp[MAX_PATH + 4];
            if (!g_favActive && IsFavorite(t.path))
                swprintf_s(disp, L"\u2605 %s", t.display);
            else
                wcsncpy_s(disp, t.display, _TRUNCATE);
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
        }
    }

    // Restore state
    if (curSel >= 0) {
        SendMessage(hListBox, LB_SETSEL, TRUE, curSel);
        SendMessage(hListBox, LB_SETCURSEL, curSel, 0);
    }
    if (topIdx >= 0) SendMessage(hListBox, LB_SETTOPINDEX, topIdx, 0);

    LayoutControls(g_hwnd);
    SetFocus(hListBox);
}

void ToggleModernStyle()
{
    g_modernStyle = !g_modernStyle;
    if (g_modernStyle && !g_thumbPlaceholder)
        g_thumbPlaceholder = CreatePlaceholderBitmap();
    CheckMenuItem(GetMenu(g_hwnd), IDM_VIEW_MODERN,
        g_modernStyle ? MF_CHECKED : MF_UNCHECKED);
    RecreateListbox();
}

// New Playlist prompt: tracks to prefill + opener (defined after the manager)
static std::vector<std::wstring> s_newPlTracks;
static void OpenNamePromptDialog(int renamePlIdx = -1);

// ============================================================
//  Playlist Manager — Spotify-style library management for .bpp
//  playlists: artwork mosaics, rename, reorder, export, import
// ============================================================
static HWND g_hwndPMgr = NULL;
static std::map<std::wstring, HBITMAP> s_pmgrArt;   // filePath -> 60x60 artwork
static std::vector<int> s_pmgrCounts;               // track counts, parallel to g_bppPlaylists

static void PMgrClearArt()
{
    for (auto& p : s_pmgrArt)
        if (p.second) DeleteObject(p.second);
    s_pmgrArt.clear();
}

static int PMgrCountTracks(const wchar_t* bppPath)
{
    FILE* f = _wfopen(bppPath, L"r,ccs=UTF-8");
    if (!f) return 0;
    int count = 0;
    wchar_t line[2048];
    while (fgetws(line, _countof(line), f)) {
        size_t len = wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len > 0) count++;
    }
    fclose(f);
    return count;
}

// Path of a custom artwork image for a playlist ("<name>.jpg/png" next to the .bpp)
static bool PMgrCustomArtPath(const BppPlaylist& pl, wchar_t* out, int n)
{
    const wchar_t* exts[] = { L"jpg", L"png", L"jpeg" };
    for (auto ext : exts) {
        swprintf_s(out, n, L"%s\\%s.%s", g_plDir, pl.name.c_str(), ext);
        if (GetFileAttributesW(out) != INVALID_FILE_ATTRIBUTES) return true;
    }
    out[0] = 0;
    return false;
}

// Build 60x60 artwork: custom image if set, else 2x2 mosaic of the first
// four tracks' embedded artwork, else a music-note placeholder.
static HBITMAP PMgrBuildArt(const BppPlaylist& pl)
{
    const int SZ = 60, HALF = 30;

    // Custom playlist artwork file wins
    wchar_t artPath[MAX_PATH];
    if (PMgrCustomArtPath(pl, artPath, MAX_PATH)) {
        HANDLE hf = CreateFile(artPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hf != INVALID_HANDLE_VALUE) {
            DWORD sz = GetFileSize(hf, NULL);
            if (sz > 100 && sz < 20 * 1024 * 1024) {
                std::vector<BYTE> data(sz);
                DWORD rd = 0;
                ReadFile(hf, data.data(), sz, &rd, NULL);
                CloseHandle(hf);
                if (rd == sz) {
                    HBITMAP bm = DecodeImageToThumb(data.data(), sz, SZ);
                    if (bm) return bm;
                }
            } else {
                CloseHandle(hf);
            }
        }
    }

    // Read the first 4 track paths for the mosaic
    std::vector<std::wstring> tracks;
    FILE* f = _wfopen(pl.filePath.c_str(), L"r,ccs=UTF-8");
    if (f) {
        wchar_t line[2048];
        while (fgetws(line, _countof(line), f) && (int)tracks.size() < 4) {
            size_t len = wcslen(line);
            while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
            if (len > 0) tracks.push_back(line);
        }
        fclose(f);
    }

    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = SZ; bi.bmiHeader.biHeight = -SZ;
    bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32;
    void* bits = NULL;
    HBITMAP hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    HBITMAP old = (HBITMAP)SelectObject(hdc, hbm);
    RECT rc = { 0, 0, SZ, SZ };
    HBRUSH br = CreateSolidBrush(g_darkMode ? (COLORREF)0x383838 : (COLORREF)0xCCCCCC);
    FillRect(hdc, &rc, br); DeleteObject(br);

    int quad = 0;
    for (auto& t : tracks) {
        HBITMAP art = LoadThumbFromFile(t.c_str(), HALF);
        if (!art) continue;
        int qx = (quad % 2) * HALF, qy = (quad / 2) * HALF;
        HDC mdc = CreateCompatibleDC(hdc);
        HBITMAP ob = (HBITMAP)SelectObject(mdc, art);
        BitBlt(hdc, qx, qy, HALF, HALF, mdc, 0, 0, SRCCOPY);
        SelectObject(mdc, ob); DeleteDC(mdc);
        DeleteObject(art);
        if (++quad >= 4) break;
    }
    if (quad == 0) {
        // No artwork anywhere: music note placeholder
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, g_darkMode ? (COLORREF)0xBBBBBB : (COLORREF)0x666666);
        HFONT hf2 = CreateFont(-30, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
        HFONT of = (HFONT)SelectObject(hdc, hf2);
        wchar_t note[2] = { 0x266B, 0 };
        DrawText(hdc, note, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, of); DeleteObject(hf2);
    }
    // Thin border
    HPEN pen = CreatePen(PS_SOLID, 1, g_darkMode ? (COLORREF)0x555555 : (COLORREF)0xAAAAAA);
    HPEN op = (HPEN)SelectObject(hdc, pen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, SZ, SZ);
    SelectObject(hdc, op); DeleteObject(pen);

    SelectObject(hdc, old);
    DeleteDC(hdc);
    ReleaseDC(NULL, hdcScreen);
    return hbm;
}

static HBITMAP PMgrGetArt(const BppPlaylist& pl)
{
    auto it = s_pmgrArt.find(pl.filePath);
    if (it != s_pmgrArt.end()) return it->second;
    HBITMAP bm = PMgrBuildArt(pl);
    s_pmgrArt[pl.filePath] = bm;
    return bm;
}

static void PMgrRefresh(HWND hwnd, bool rescan = true)
{
    if (rescan) ScanPlaylists();
    HWND hList = GetDlgItem(hwnd, ID_PMGR_LIST);
    int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
    SendMessage(hList, WM_SETREDRAW, FALSE, 0);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    s_pmgrCounts.clear();
    for (auto& pl : g_bppPlaylists) {
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)pl.name.c_str());
        s_pmgrCounts.push_back(PMgrCountTracks(pl.filePath.c_str()));
    }
    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hList, NULL, TRUE);
    if (sel >= 0 && sel < (int)g_bppPlaylists.size())
        SendMessage(hList, LB_SETCURSEL, sel, 0);
}

// Manager list subclass: mouse drag-reorder, right-click menu, DEL to delete
static WNDPROC g_OldPMgrListProc = NULL;
static bool s_pmgrDragging = false;
static int  s_pmgrDragFrom = -1, s_pmgrDragCur = -1;
static bool s_pmgrDragMoved = false;

static LRESULT CALLBACK PMgrListSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN && wParam == VK_DELETE) {
        SendMessage(GetParent(hwnd), WM_COMMAND, ID_PMGR_DELETE, 0);
        return 0;
    }
    if (msg == WM_LBUTTONDOWN) {
        int y = GET_Y_LPARAM(lParam);
        int idx = (int)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(0, y));
        if (!HIWORD(idx) && LOWORD(idx) < (UINT)g_bppPlaylists.size()) {
            s_pmgrDragging = true;
            s_pmgrDragFrom = s_pmgrDragCur = LOWORD(idx);
            s_pmgrDragMoved = false;
            SetCapture(hwnd);
            SendMessage(hwnd, LB_SETCURSEL, s_pmgrDragFrom, 0);
            SetFocus(hwnd);
            return 0;
        }
        return CallWindowProc(g_OldPMgrListProc, hwnd, msg, wParam, lParam);
    }
    if (msg == WM_MOUSEMOVE && s_pmgrDragging && (wParam & MK_LBUTTON)) {
        int y = GET_Y_LPARAM(lParam);
        int idx = (int)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(0, y));
        if (HIWORD(idx))
            idx = (y < 0) ? 0 : (int)g_bppPlaylists.size() - 1;
        else
            idx = LOWORD(idx);
        if (idx != s_pmgrDragCur && idx >= 0 && idx < (int)g_bppPlaylists.size()) {
            s_pmgrDragMoved = true;
            s_pmgrDragCur = idx;
            SendMessage(hwnd, LB_SETCURSEL, idx, 0);
        }
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
        return 0;
    }
    if (msg == WM_LBUTTONUP && s_pmgrDragging) {
        ReleaseCapture();
        s_pmgrDragging = false;
        int from = s_pmgrDragFrom, to = s_pmgrDragCur;
        if (s_pmgrDragMoved && from != to &&
            from >= 0 && from < (int)g_bppPlaylists.size() &&
            to >= 0 && to < (int)g_bppPlaylists.size()) {
            BppPlaylist moved = g_bppPlaylists[from];
            g_bppPlaylists.erase(g_bppPlaylists.begin() + from);
            g_bppPlaylists.insert(g_bppPlaylists.begin() + to, moved);
            SavePlaylistOrder();
            PMgrRefresh(GetParent(hwnd), false);
            SendMessage(hwnd, LB_SETCURSEL, to, 0);
        }
        return 0;
    }
    if (msg == WM_CAPTURECHANGED) {
        s_pmgrDragging = false;
        return CallWindowProc(g_OldPMgrListProc, hwnd, msg, wParam, lParam);
    }
    if (msg == WM_RBUTTONDOWN) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int idx = (int)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
        if (!HIWORD(idx) && LOWORD(idx) < (UINT)g_bppPlaylists.size()) {
            SendMessage(hwnd, LB_SETCURSEL, LOWORD(idx), 0);
            POINT sc = pt; ClientToScreen(hwnd, &sc);
            HMENU hm = CreatePopupMenu();
            AppendMenu(hm, MF_STRING, ID_PMGR_LOAD,   L"Load / Play");
            AppendMenu(hm, MF_SEPARATOR, 0, NULL);
            AppendMenu(hm, MF_STRING, ID_PMGR_RENAME, L"Rename...");
            AppendMenu(hm, MF_STRING, ID_PMGR_ART,    L"Set Artwork...");
            AppendMenu(hm, MF_STRING, ID_PMGR_EXPORT, L"Export...");
            AppendMenu(hm, MF_SEPARATOR, 0, NULL);
            AppendMenu(hm, MF_STRING, ID_PMGR_DELETE, L"Delete\tDel");
            TrackPopupMenu(hm, TPM_RIGHTBUTTON, sc.x, sc.y, 0, GetParent(hwnd), NULL);
            DestroyMenu(hm);
        }
        return 0;
    }
    return CallWindowProc(g_OldPMgrListProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK PMgrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        int cw = 560, ch = 420;

        HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT
            | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_TABSTOP,
            8, 8, cw - 128, ch - 16, hwnd, (HMENU)ID_PMGR_LIST, hInst, NULL);
        SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hList, LB_SETITEMHEIGHT, 0, 68);
        g_OldPMgrListProc = (WNDPROC)SetWindowLongPtr(hList, GWLP_WNDPROC, (LONG_PTR)PMgrListSubProc);

        int bx = cw - 112, bw = 104, bh = 26, by = 8;
        struct { int id; const wchar_t* txt; } btns[] = {
            { ID_PMGR_LOAD,   L"Load / Play" },
            { ID_PMGR_NEW,    L"New..." },
            { ID_PMGR_RENAME, L"Rename..." },
            { ID_PMGR_DELETE, L"Delete" },
            { ID_PMGR_ART,    L"Set Artwork..." },
            { ID_PMGR_EXPORT, L"Export..." },
            { ID_PMGR_IMPORT, L"Import..." },
        };
        for (auto& b : btns) {
            HWND hB = CreateWindow(L"BUTTON", b.txt,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                bx, by, bw, bh, hwnd, (HMENU)(UINT_PTR)b.id, hInst, NULL);
            SendMessage(hB, WM_SETFONT, (WPARAM)(b.id == ID_PMGR_LOAD ? g_fontBold : g_fontUI), TRUE);
            by += bh + 6;
            if (b.id == ID_PMGR_DELETE || b.id == ID_PMGR_ART) by += 8; // group gaps
        }
        HWND hClose = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            bx, ch - 34, bw, bh, hwnd, (HMENU)ID_PMGR_CLOSE, hInst, NULL);
        SendMessage(hClose, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        PMgrRefresh(hwnd);
        break;
    }

    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID != ID_PMGR_LIST) break;
        int idx = (int)dis->itemID;
        HDC dc = dis->hDC;
        RECT rc = dis->rcItem;
        bool selected = (dis->itemState & ODS_SELECTED) != 0;
        COLORREF bgCol, txCol, dimCol;
        if (g_darkMode) {
            bgCol = selected ? RGB(30, 80, 160) : g_theme.bgList;
            txCol = selected ? RGB(255, 255, 255) : g_theme.text;
            dimCol = selected ? RGB(200, 215, 235) : g_theme.textDim;
        } else {
            bgCol = selected ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW);
            txCol = selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT);
            dimCol = selected ? RGB(220, 230, 245) : GetSysColor(COLOR_GRAYTEXT);
        }
        HBRUSH br = CreateSolidBrush(bgCol);
        FillRect(dc, &rc, br); DeleteObject(br);
        if (idx < 0 || idx >= (int)g_bppPlaylists.size()) return TRUE;

        // 60x60 artwork
        HBITMAP art = PMgrGetArt(g_bppPlaylists[idx]);
        if (art) {
            HDC mdc = CreateCompatibleDC(dc);
            HBITMAP ob = (HBITMAP)SelectObject(mdc, art);
            BitBlt(dc, rc.left + 4, rc.top + 4, 60, 60, mdc, 0, 0, SRCCOPY);
            SelectObject(mdc, ob); DeleteDC(mdc);
        }
        // Name + track count
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, txCol);
        HFONT of = (HFONT)SelectObject(dc, g_fontBold);
        RECT tr = { rc.left + 72, rc.top + 14, rc.right - 6, rc.top + 34 };
        DrawText(dc, g_bppPlaylists[idx].name.c_str(), -1, &tr,
            DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(dc, g_fontUI);
        SetTextColor(dc, dimCol);
        wchar_t cnt[48];
        int nTracks = (idx < (int)s_pmgrCounts.size()) ? s_pmgrCounts[idx] : 0;
        swprintf_s(cnt, L"%d track%s", nTracks, nTracks == 1 ? L"" : L"s");
        RECT cr = { rc.left + 72, rc.top + 36, rc.right - 6, rc.top + 54 };
        DrawText(dc, cnt, -1, &cr, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);
        SelectObject(dc, of);
        if (dis->itemState & ODS_FOCUS) DrawFocusRect(dc, &rc);
        return TRUE;
    }

    case WM_COMMAND: {
        HWND hList = GetDlgItem(hwnd, ID_PMGR_LIST);
        int sel = hList ? (int)SendMessage(hList, LB_GETCURSEL, 0, 0) : LB_ERR;
        bool haveSel = (sel >= 0 && sel < (int)g_bppPlaylists.size());
        switch (LOWORD(wParam)) {
        case ID_PMGR_LIST:
            if (HIWORD(wParam) == LBN_DBLCLK && haveSel) {
                LoadBppIntoPlaylist(sel);
                DestroyWindow(hwnd);
            }
            break;
        case ID_PMGR_LOAD:
            if (haveSel) { LoadBppIntoPlaylist(sel); DestroyWindow(hwnd); }
            break;
        case ID_PMGR_NEW:
            // Name-only prompt — saves straight into the playlists folder
            s_newPlTracks.clear();
            OpenNamePromptDialog();
            break;
        case ID_PMGR_RENAME:
            // In-place rename via the small name prompt (pre-filled with the
            // current name) — renames the .bpp file directly, no save dialog.
            if (!haveSel) break;
            OpenNamePromptDialog(sel);
            break;
        case ID_PMGR_DELETE: {
            if (!haveSel) break;
            wchar_t msg[320];
            swprintf_s(msg, L"Delete playlist \"%s\"?\n\n(Audio files are not deleted.)",
                g_bppPlaylists[sel].name.c_str());
            if (MessageBox(hwnd, msg, L"Delete Playlist", MB_YESNO | MB_ICONWARNING) == IDYES) {
                wchar_t artPath[MAX_PATH];
                if (PMgrCustomArtPath(g_bppPlaylists[sel], artPath, MAX_PATH))
                    DeleteFileW(artPath);
                DeleteFileW(g_bppPlaylists[sel].filePath.c_str());
                g_bppPlaylists.erase(g_bppPlaylists.begin() + sel);
                SavePlaylistOrder();
                PMgrClearArt();
                PMgrRefresh(hwnd);
            }
            break;
        }
        case ID_PMGR_ART: {
            if (!haveSel) break;
            wchar_t img[MAX_PATH] = L"";
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"Image Files (*.jpg;*.jpeg;*.png)\0*.jpg;*.jpeg;*.png\0";
            ofn.lpstrFile = img; ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Choose Playlist Artwork";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileName(&ofn)) {
                GetLibDir();
                // Remove any previous custom art, then copy the new image in
                wchar_t oldArt[MAX_PATH];
                if (PMgrCustomArtPath(g_bppPlaylists[sel], oldArt, MAX_PATH))
                    DeleteFileW(oldArt);
                const wchar_t* ext = wcsrchr(img, L'.');
                wchar_t dst[MAX_PATH];
                swprintf_s(dst, L"%s\\%s%s", g_plDir, g_bppPlaylists[sel].name.c_str(),
                    ext ? ext : L".jpg");
                CopyFileW(img, dst, FALSE);
                PMgrClearArt();
                InvalidateRect(hList, NULL, TRUE);
            }
            break;
        }
        case ID_PMGR_EXPORT: {
            if (!haveSel) break;
            wchar_t file[MAX_PATH];
            swprintf_s(file, L"%s.bpp", g_bppPlaylists[sel].name.c_str());
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"BillyPro Playlist (*.bpp)\0*.bpp\0M3U Playlist (*.m3u)\0*.m3u\0";
            ofn.lpstrFile = file; ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Export Playlist";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
            ofn.lpstrDefExt = L"bpp";
            if (GetSaveFileName(&ofn)) {
                const wchar_t* ext = wcsrchr(file, L'.');
                if (ext && _wcsicmp(ext, L".m3u") == 0) {
                    // Convert to M3U
                    FILE* src = _wfopen(g_bppPlaylists[sel].filePath.c_str(), L"r,ccs=UTF-8");
                    FILE* dst = src ? _wfopen(file, L"w,ccs=UTF-8") : NULL;
                    if (src && dst) {
                        fwprintf(dst, L"#EXTM3U\r\n");
                        wchar_t line[2048];
                        while (fgetws(line, _countof(line), src)) {
                            size_t len = wcslen(line);
                            while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
                            if (len > 0) fwprintf(dst, L"%s\r\n", line);
                        }
                    }
                    if (src) fclose(src);
                    if (dst) fclose(dst);
                } else {
                    CopyFileW(g_bppPlaylists[sel].filePath.c_str(), file, FALSE);
                }
            }
            break;
        }
        case ID_PMGR_IMPORT: {
            wchar_t file[MAX_PATH] = L"";
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"Playlists (*.bpp;*.m3u;*.m3u8)\0*.bpp;*.m3u;*.m3u8\0";
            ofn.lpstrFile = file; ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Import Playlist";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileName(&ofn)) {
                GetLibDir();
                // Destination: playlists dir, same base name, .bpp
                const wchar_t* fn = wcsrchr(file, L'\\');
                wchar_t base[MAX_PATH];
                wcsncpy_s(base, fn ? fn + 1 : file, _TRUNCATE);
                wchar_t* d2 = wcsrchr(base, L'.'); if (d2) *d2 = 0;
                wchar_t dst[MAX_PATH];
                swprintf_s(dst, L"%s\\%s.bpp", g_plDir, base);
                const wchar_t* ext = wcsrchr(file, L'.');
                if (ext && (_wcsicmp(ext, L".m3u") == 0 || _wcsicmp(ext, L".m3u8") == 0)) {
                    // Convert M3U -> .bpp: keep file paths + URLs, resolve relative paths
                    wchar_t m3uDir[MAX_PATH];
                    wcsncpy_s(m3uDir, file, _TRUNCATE);
                    wchar_t* sep = wcsrchr(m3uDir, L'\\'); if (sep) sep[1] = 0; else m3uDir[0] = 0;
                    FILE* src = _wfopen(file, L"r,ccs=UTF-8");
                    FILE* out = src ? _wfopen(dst, L"w,ccs=UTF-8") : NULL;
                    if (src && out) {
                        wchar_t line[2048];
                        while (fgetws(line, _countof(line), src)) {
                            size_t len = wcslen(line);
                            while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
                            if (len == 0 || line[0] == L'#') continue;
                            if (_wcsnicmp(line, L"http://", 7) == 0 || _wcsnicmp(line, L"https://", 8) == 0) {
                                fwprintf(out, L"%s\n", line);
                            } else if (GetFileAttributesW(line) != INVALID_FILE_ATTRIBUTES) {
                                fwprintf(out, L"%s\n", line);
                            } else if (m3uDir[0] && line[0] != L'\\' && !(len >= 2 && line[1] == L':')) {
                                wchar_t resolved[MAX_PATH];
                                _snwprintf_s(resolved, MAX_PATH, _TRUNCATE, L"%s%s", m3uDir, line);
                                if (GetFileAttributesW(resolved) != INVALID_FILE_ATTRIBUTES)
                                    fwprintf(out, L"%s\n", resolved);
                            }
                        }
                    }
                    if (src) fclose(src);
                    if (out) fclose(out);
                } else {
                    CopyFileW(file, dst, FALSE);
                }
                PMgrRefresh(hwnd);
            }
            break;
        }
        case ID_PMGR_CLOSE:
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        }
        break;
    }

    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_KEYDOWN: if (wParam == VK_ESCAPE) DestroyWindow(hwnd); break;
    case WM_DESTROY:
        PMgrClearArt();
        s_pmgrCounts.clear();
        g_hwndPMgr = NULL;
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenPlaylistManager()
{
    if (g_hwndPMgr && IsWindow(g_hwndPMgr)) { SetForegroundWindow(g_hwndPMgr); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t PMGR_CLASS[] = L"BillyPMgrWnd";
    static bool pmgrReg = false;
    if (!pmgrReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = PMgrWndProc; wc.hInstance = hInst;
        wc.lpszClassName = PMGR_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); pmgrReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 560, 420 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndPMgr = CreateWindowEx(WS_EX_APPWINDOW, PMGR_CLASS,
        L"Playlist Manager", style, 0, 0, dw, dh, g_hwnd, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndPMgr, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
}

// Write ID3v2.3 tags to an MP3 file (re-writes the header, preserves audio data)
// Helper: build ID3v2.3 text frames from tag fields
static void BuildID3v2TextFrames(std::vector<BYTE>& frames,
    const wchar_t* title, const wchar_t* artist, const wchar_t* album,
    const wchar_t* year, const wchar_t* track, const wchar_t* genre)
{
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
        DWORD fds = 1 + (DWORD)(utfN - 1);
        for (int c = 0; c < 4; c++) frames.push_back((BYTE)te.id[c]);
        frames.push_back((BYTE)((fds >> 24) & 0xFF));
        frames.push_back((BYTE)((fds >> 16) & 0xFF));
        frames.push_back((BYTE)((fds >> 8) & 0xFF));
        frames.push_back((BYTE)(fds & 0xFF));
        frames.push_back(0); frames.push_back(0);
        frames.push_back(3); // UTF-8
        for (int i = 0; i < utfN - 1; i++) frames.push_back((BYTE)utf[i]);
    }
}

// Helper: extract existing APIC frame from an ID3v2 tag block
static std::vector<BYTE> ExtractAPICFrame(const BYTE* tagData, DWORD tagSize, BYTE ver)
{
    const BYTE* p = tagData;
    const BYTE* end = p + tagSize;
    while (p + 10 < end) {
        char fid[5] = {}; memcpy(fid, p, 4);
        DWORD fsize;
        if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
        else          fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
        if (fsize == 0 || p + 10 + fsize > end) break;
        if (strcmp(fid, "APIC") == 0 && fsize > 10) {
            // Return the entire frame (header + data)
            std::vector<BYTE> frame(p, p + 10 + fsize);
            return frame;
        }
        p += 10 + fsize;
    }
    return {};
}

static std::vector<BYTE> BuildAPICFrame(const BYTE* imgData, DWORD imgSize); // forward decl

static bool WriteID3v2ToMP3(const wchar_t* path,
    const wchar_t* title, const wchar_t* artist, const wchar_t* album,
    const wchar_t* year,  const wchar_t* track,  const wchar_t* genre)
{
    HANDLE hf = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER fs; GetFileSizeEx(hf, &fs);
    if (fs.QuadPart > 500LL * 1024 * 1024) { CloseHandle(hf); return false; }

    BYTE hdr[10]; DWORD rd;
    ReadFile(hf, hdr, 10, &rd, NULL);
    DWORD audioOffset = 0;
    std::vector<BYTE> existingAPIC;
    if (rd == 10 && memcmp(hdr, "ID3", 3) == 0) {
        DWORD sz = ((DWORD)(hdr[6] & 0x7F) << 21) | ((DWORD)(hdr[7] & 0x7F) << 14) |
                   ((DWORD)(hdr[8] & 0x7F) << 7)  | (hdr[9] & 0x7F);
        audioOffset = 10 + sz;
        // Read existing tag to preserve APIC
        std::vector<BYTE> oldTag(sz);
        SetFilePointer(hf, 10, NULL, FILE_BEGIN);
        ReadFile(hf, oldTag.data(), sz, &rd, NULL);
        existingAPIC = ExtractAPICFrame(oldTag.data(), sz, hdr[3]);
    }
    DWORD audioSize = (audioOffset < (DWORD)fs.QuadPart) ? ((DWORD)fs.QuadPart - audioOffset) : 0;
    std::vector<BYTE> audio(audioSize);
    SetFilePointer(hf, audioOffset, NULL, FILE_BEGIN);
    ReadFile(hf, audio.data(), audioSize, &rd, NULL);
    CloseHandle(hf);

    // Build new text frames
    std::vector<BYTE> frames;
    BuildID3v2TextFrames(frames, title, artist, album, year, track, genre);
    // Artwork: prefer g_artBytes (new from Discogs), fall back to existing APIC from file
    if (g_artBytes && g_artSize > 100) {
        auto apicFrame = BuildAPICFrame(g_artBytes, g_artSize);
        frames.insert(frames.end(), apicFrame.begin(), apicFrame.end());
    } else if (!existingAPIC.empty()) {
        frames.insert(frames.end(), existingAPIC.begin(), existingAPIC.end());
    }

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

// Write Vorbis Comment tags to a FLAC file using libFLAC metadata API
static bool WriteTagsToFLAC(const wchar_t* path,
    const wchar_t* title, const wchar_t* artist, const wchar_t* album,
    const wchar_t* year, const wchar_t* track, const wchar_t* genre)
{
    // libFLAC uses char* paths — convert to UTF-8
    int utfLen = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    std::string utf8path(utfLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, path, -1, &utf8path[0], utfLen, NULL, NULL);

    FLAC__Metadata_Chain* chain = FLAC__metadata_chain_new();
    if (!chain) return false;
    if (!FLAC__metadata_chain_read(chain, utf8path.c_str())) {
        FLAC__metadata_chain_delete(chain);
        return false;
    }

    FLAC__Metadata_Iterator* iter = FLAC__metadata_iterator_new();
    if (!iter) { FLAC__metadata_chain_delete(chain); return false; }
    FLAC__metadata_iterator_init(iter, chain);

    // Find existing VORBIS_COMMENT block, or create one
    FLAC__StreamMetadata* vc = NULL;
    do {
        FLAC__StreamMetadata* block = FLAC__metadata_iterator_get_block(iter);
        if (block->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
            vc = block;
            break;
        }
    } while (FLAC__metadata_iterator_next(iter));

    if (!vc) {
        // No existing VC block — create and insert one
        vc = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
        if (!vc) { FLAC__metadata_iterator_delete(iter); FLAC__metadata_chain_delete(chain); return false; }
        FLAC__metadata_iterator_insert_block_after(iter, vc);
    }

    // Helper: set or replace a comment field
    auto setField = [&](const char* fieldName, const wchar_t* value) {
        if (!value || !value[0]) return;
        int vLen = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
        std::string utf8val(vLen, '\0');
        WideCharToMultiByte(CP_UTF8, 0, value, -1, &utf8val[0], vLen, NULL, NULL);
        utf8val.resize(vLen - 1); // remove null

        std::string entry = std::string(fieldName) + "=" + utf8val;
        FLAC__StreamMetadata_VorbisComment_Entry e;
        e.length = (FLAC__uint32)entry.size();
        e.entry = (FLAC__byte*)entry.c_str();

        // Find and replace existing, or append
        int idx = FLAC__metadata_object_vorbiscomment_find_entry_from(vc, 0, fieldName);
        if (idx >= 0)
            FLAC__metadata_object_vorbiscomment_set_comment(vc, (unsigned)idx, e, /*copy=*/true);
        else
            FLAC__metadata_object_vorbiscomment_append_comment(vc, e, /*copy=*/true);
    };

    setField("TITLE", title);
    setField("ARTIST", artist);
    setField("ALBUM", album);
    setField("DATE", year);
    setField("TRACKNUMBER", track);
    setField("GENRE", genre);

    // Add artwork if available (from Discogs download or existing)
    if (g_artBytes && g_artSize > 100) {
        // Remove existing PICTURE blocks first
        FLAC__metadata_iterator_init(iter, chain);
        do {
            FLAC__StreamMetadata* block = FLAC__metadata_iterator_get_block(iter);
            if (block->type == FLAC__METADATA_TYPE_PICTURE) {
                FLAC__metadata_iterator_delete_block(iter, false);
                break; // restart would be safer but one is enough for most files
            }
        } while (FLAC__metadata_iterator_next(iter));
        // Create new PICTURE block
        FLAC__StreamMetadata* pic = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
        if (pic) {
            const char* mime = "image/jpeg";
            if (g_artBytes[0] == 0x89 && g_artBytes[1] == 'P') mime = "image/png";
            FLAC__metadata_object_picture_set_mime_type(pic, (char*)mime, true);
            FLAC__metadata_object_picture_set_description(pic, (FLAC__byte*)"", true);
            pic->data.picture.type = FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER;
            pic->data.picture.width = 0;
            pic->data.picture.height = 0;
            pic->data.picture.depth = 0;
            pic->data.picture.colors = 0;
            FLAC__metadata_object_picture_set_data(pic, g_artBytes, g_artSize, true);
            // Seek to end of chain and insert
            FLAC__metadata_iterator_init(iter, chain);
            while (FLAC__metadata_iterator_next(iter)) {}
            FLAC__metadata_iterator_insert_block_after(iter, pic);
        }
    }

    FLAC__metadata_chain_sort_padding(chain);
    bool ok = FLAC__metadata_chain_write(chain, /*padding=*/true, /*preserve_stat=*/false);

    FLAC__metadata_iterator_delete(iter);
    FLAC__metadata_chain_delete(chain);
    return ok;
}

// Write ID3v2 tags to a WAV file (embeds an "id3 " chunk in the RIFF structure)
static bool WriteID3v2ToWAV(const wchar_t* path,
    const wchar_t* title, const wchar_t* artist, const wchar_t* album,
    const wchar_t* year, const wchar_t* track, const wchar_t* genre)
{
    // Read entire file
    HANDLE hf = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER fs; GetFileSizeEx(hf, &fs);
    if (fs.QuadPart > 2LL * 1024 * 1024 * 1024) { CloseHandle(hf); return false; } // 2GB guard

    std::vector<BYTE> fileData((size_t)fs.QuadPart);
    DWORD rd; ReadFile(hf, fileData.data(), (DWORD)fs.QuadPart, &rd, NULL);
    CloseHandle(hf);
    if (rd != (DWORD)fs.QuadPart) return false;

    // Verify RIFF/WAVE
    if (rd < 12 || memcmp(fileData.data(), "RIFF", 4) != 0 || memcmp(fileData.data() + 8, "WAVE", 4) != 0)
        return false;

    // Build ID3v2 text frames + preserve existing artwork from old id3 chunk
    std::vector<BYTE> frames;
    BuildID3v2TextFrames(frames, title, artist, album, year, track, genre);
    // Artwork: prefer g_artBytes (new from Discogs), fall back to existing APIC
    if (g_artBytes && g_artSize > 100) {
        auto apicFrame = BuildAPICFrame(g_artBytes, g_artSize);
        frames.insert(frames.end(), apicFrame.begin(), apicFrame.end());
    } else {
        size_t scanPos = 12;
        while (scanPos + 8 <= fileData.size()) {
            DWORD chSz = fileData[scanPos+4] | (fileData[scanPos+5] << 8) | (fileData[scanPos+6] << 16) | (fileData[scanPos+7] << 24);
            if (chSz == 0 || chSz > 50 * 1024 * 1024) break;
            bool isId3Ch = (memcmp(&fileData[scanPos], "id3 ", 4) == 0 || memcmp(&fileData[scanPos], "ID3 ", 4) == 0 || memcmp(&fileData[scanPos], "ID32", 4) == 0);
            if (isId3Ch && chSz > 10) {
                size_t dataStart = scanPos + 8;
                if (dataStart + 10 <= fileData.size() && memcmp(&fileData[dataStart], "ID3", 3) == 0) {
                    DWORD oldTagSz = ((fileData[dataStart+6] & 0x7F) << 21) | ((fileData[dataStart+7] & 0x7F) << 14) |
                        ((fileData[dataStart+8] & 0x7F) << 7) | (fileData[dataStart+9] & 0x7F);
                    if (dataStart + 10 + oldTagSz <= fileData.size()) {
                        auto apic = ExtractAPICFrame(&fileData[dataStart + 10], oldTagSz, fileData[dataStart + 3]);
                        if (!apic.empty()) frames.insert(frames.end(), apic.begin(), apic.end());
                    }
                }
                break;
            }
            scanPos += 8 + ((chSz + 1) & ~1);
        }
    }

    DWORD padSz = 64;
    DWORD bodySize = (DWORD)frames.size() + padSz;
    BYTE ss[4] = { (BYTE)((bodySize >> 21) & 0x7F), (BYTE)((bodySize >> 14) & 0x7F),
                   (BYTE)((bodySize >> 7) & 0x7F), (BYTE)(bodySize & 0x7F) };

    // Build id3 chunk: "id3 " + size(4 LE) + ID3v2 header + frames + padding
    std::vector<BYTE> id3Chunk;
    // ID3v2 header
    BYTE id3hdr[10] = {'I','D','3', 3, 0, 0, ss[0], ss[1], ss[2], ss[3]};
    for (int i = 0; i < 10; i++) id3Chunk.push_back(id3hdr[i]);
    for (auto b : frames) id3Chunk.push_back(b);
    for (DWORD i = 0; i < padSz; i++) id3Chunk.push_back(0);

    // Remove existing "id3 " or "ID3 " or "ID32" chunk from the WAV
    std::vector<BYTE> newFile;
    newFile.insert(newFile.end(), fileData.begin(), fileData.begin() + 12); // RIFF header
    size_t pos = 12;
    while (pos + 8 <= fileData.size()) {
        DWORD chunkSz = fileData[pos+4] | (fileData[pos+5] << 8) | (fileData[pos+6] << 16) | (fileData[pos+7] << 24);
        DWORD alignedSz = (chunkSz + 1) & ~1;
        bool isId3 = (memcmp(&fileData[pos], "id3 ", 4) == 0 || memcmp(&fileData[pos], "ID3 ", 4) == 0 || memcmp(&fileData[pos], "ID32", 4) == 0);
        if (!isId3) {
            size_t end = pos + 8 + alignedSz;
            if (end > fileData.size()) end = fileData.size();
            newFile.insert(newFile.end(), fileData.begin() + pos, fileData.begin() + end);
        }
        pos += 8 + alignedSz;
    }

    // Append new id3 chunk
    DWORD id3ChunkSz = (DWORD)id3Chunk.size();
    BYTE chunkHdr[8] = {'i','d','3',' ', (BYTE)(id3ChunkSz&0xFF), (BYTE)((id3ChunkSz>>8)&0xFF),
                        (BYTE)((id3ChunkSz>>16)&0xFF), (BYTE)((id3ChunkSz>>24)&0xFF)};
    newFile.insert(newFile.end(), chunkHdr, chunkHdr + 8);
    newFile.insert(newFile.end(), id3Chunk.begin(), id3Chunk.end());
    if (id3ChunkSz & 1) newFile.push_back(0); // pad to word boundary

    // Fix RIFF size
    DWORD riffSize = (DWORD)(newFile.size() - 8);
    newFile[4] = (BYTE)(riffSize & 0xFF);
    newFile[5] = (BYTE)((riffSize >> 8) & 0xFF);
    newFile[6] = (BYTE)((riffSize >> 16) & 0xFF);
    newFile[7] = (BYTE)((riffSize >> 24) & 0xFF);

    // Write back
    hf = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return false;
    DWORD wr;
    WriteFile(hf, newFile.data(), (DWORD)newFile.size(), &wr, NULL);
    CloseHandle(hf);
    return wr == (DWORD)newFile.size();
}

// ============================================================
//  Discogs Tag Database lookup dialog
// ============================================================
static HWND g_hwndDiscogs = NULL;
static HBITMAP g_dcgPreviewBmp = NULL; // artwork preview bitmap
static HWND g_hwndInfoParent = NULL; // the Audio Info dialog that opened us

struct DiscogsResult {
    int id;
    wchar_t title[256];
    wchar_t artist[256];
    wchar_t year[16];
    wchar_t genre[128];
    wchar_t label[128];
    wchar_t coverUrl[512];
    wchar_t resourceUrl[512]; // API URL for full release details
    wchar_t display[512]; // for listbox
};
static std::vector<DiscogsResult> g_dcgResults;

// Helper: URL-encode a UTF-8 string
static std::string UrlEncode(const wchar_t* wide)
{
    int utfLen = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
    std::string utf(utfLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &utf[0], utfLen, NULL, NULL);
    utf.resize(utfLen - 1); // remove null
    std::string out;
    for (char c : utf) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~')
            out += c;
        else {
            char hex[4]; sprintf_s(hex, "%%%02X", (unsigned char)c);
            out += hex;
        }
    }
    return out;
}

// Simple JSON value extractor — finds "key":"value" or "key":number
// Returns empty string if not found. Only works for flat/shallow JSON.
static std::wstring JsonGet(const char* json, const char* key)
{
    char needle[128]; sprintf_s(needle, "\"%s\"", key);
    const char* p = strstr(json, needle);
    if (!p) return L"";
    p += strlen(needle);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (!*p) return L"";
    std::string utf8val;
    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && *(p+1)) {
                p++;
                if (*p == 'n') utf8val += '\n';
                else if (*p == 't') utf8val += '\t';
                else if (*p == 'u' && p[1] && p[2] && p[3] && p[4]) {
                    // \uXXXX unicode escape
                    char hex[5] = {p[1],p[2],p[3],p[4],0};
                    wchar_t wc = (wchar_t)strtol(hex, NULL, 16);
                    char mb[8] = {};
                    WideCharToMultiByte(CP_UTF8, 0, &wc, 1, mb, 8, NULL, NULL);
                    utf8val += mb;
                    p += 4;
                }
                else utf8val += *p;
            } else {
                utf8val += *p;
            }
            p++;
        }
    } else {
        while (*p && *p != ',' && *p != '}' && *p != ']' && *p != ' ') {
            utf8val += *p; p++;
        }
    }
    // Proper UTF-8 to wchar_t conversion
    if (utf8val.empty()) return L"";
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8val.c_str(), -1, NULL, 0);
    std::wstring result(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8val.c_str(), -1, &result[0], wlen);
    if (!result.empty() && result.back() == L'\0') result.pop_back();
    return result;
}

// HTTP GET via WinHTTP, returns UTF-8 response body (empty on error)
static std::string DiscogsHttpGet(const wchar_t* url, int timeoutSec = 8)
{
    std::string result;
    // Parse URL: skip "https://"
    const wchar_t* p = url;
    if (wcsncmp(p, L"https://", 8) == 0) p += 8;
    else if (wcsncmp(p, L"http://", 7) == 0) p += 7;
    const wchar_t* pathStart = wcschr(p, L'/');
    std::wstring host(p, pathStart ? pathStart - p : wcslen(p));
    std::wstring path(pathStart ? pathStart : L"/");

    HINTERNET hSession = WinHttpOpen(L"BillyPro/0.5", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return result;
    WinHttpSetTimeouts(hSession, timeoutSec * 1000, timeoutSec * 1000, timeoutSec * 1000, timeoutSec * 1000);
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return result; }
    HINTERNET hReq = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hReq) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }

    // Add auth header if token is set
    if (g_discogsToken[0]) {
        wchar_t auth[256]; swprintf_s(auth, L"Authorization: Discogs token=%s", g_discogsToken);
        WinHttpAddRequestHeaders(hReq, auth, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    }

    if (WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hReq, NULL)) {
        DWORD size = 0;
        while (WinHttpQueryDataAvailable(hReq, &size) && size > 0) {
            std::string buf(size, '\0');
            DWORD read = 0;
            WinHttpReadData(hReq, &buf[0], size, &read);
            result.append(buf, 0, read);
        }
    }
    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

// Download binary data (image) from a URL
static std::vector<BYTE> HttpGetBinary(const wchar_t* url, int timeoutSec = 10)
{
    std::vector<BYTE> result;
    const wchar_t* p = url;
    bool https = (wcsncmp(p, L"https://", 8) == 0);
    if (https) p += 8;
    else if (wcsncmp(p, L"http://", 7) == 0) p += 7;
    else return result;
    const wchar_t* pathStart = wcschr(p, L'/');
    std::wstring host(p, pathStart ? pathStart - p : wcslen(p));
    std::wstring path(pathStart ? pathStart : L"/");

    HINTERNET hSession = WinHttpOpen(L"BillyPro/0.5", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return result;
    WinHttpSetTimeouts(hSession, timeoutSec * 1000, timeoutSec * 1000, timeoutSec * 1000, timeoutSec * 1000);
    INTERNET_PORT port = https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return result; }
    DWORD flags = https ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hReq = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hReq) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }
    if (WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hReq, NULL)) {
        DWORD size = 0;
        while (WinHttpQueryDataAvailable(hReq, &size) && size > 0) {
            size_t oldSz = result.size();
            result.resize(oldSz + size);
            DWORD read = 0;
            WinHttpReadData(hReq, &result[oldSz], size, &read);
            result.resize(oldSz + read);
        }
    }
    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

// Build an APIC (ID3v2 picture) frame from raw JPEG/PNG bytes
static std::vector<BYTE> BuildAPICFrame(const BYTE* imgData, DWORD imgSize)
{
    std::vector<BYTE> frame;
    if (!imgData || imgSize < 100) return frame;
    // Detect mime type
    const char* mime = "image/jpeg";
    if (imgData[0] == 0x89 && imgData[1] == 'P' && imgData[2] == 'N' && imgData[3] == 'G')
        mime = "image/png";
    size_t mimeLen = strlen(mime);
    // APIC frame: encoding(1) + mime(null-term) + pictType(1) + desc(null-term) + imgData
    DWORD fsize = 1 + (DWORD)mimeLen + 1 + 1 + 1 + imgSize;
    // Frame header: "APIC" + size(4 BE) + flags(2)
    frame.push_back('A'); frame.push_back('P'); frame.push_back('I'); frame.push_back('C');
    frame.push_back((BYTE)((fsize >> 24) & 0xFF));
    frame.push_back((BYTE)((fsize >> 16) & 0xFF));
    frame.push_back((BYTE)((fsize >> 8) & 0xFF));
    frame.push_back((BYTE)(fsize & 0xFF));
    frame.push_back(0); frame.push_back(0); // flags
    frame.push_back(0); // encoding: Latin-1
    for (size_t i = 0; i < mimeLen; i++) frame.push_back((BYTE)mime[i]);
    frame.push_back(0); // null term for mime
    frame.push_back(3); // picture type: Cover (front)
    frame.push_back(0); // empty description null term
    frame.insert(frame.end(), imgData, imgData + imgSize);
    return frame;
}

// Parse filename into artist and title: "Artist - Title.ext" -> artist, title
// Check if a string is a vinyl side marker: A, B, C, D, A1, A2, B1, B2, AA, BB, etc.
static bool IsVinylSide(const wchar_t* s, int len)
{
    if (len < 1 || len > 3) return false;
    // Single letter A-D
    if (len == 1 && s[0] >= L'A' && s[0] <= L'D') return true;
    // Letter + digit: A1, B2, C3, D1
    if (len == 2 && s[0] >= L'A' && s[0] <= L'D' && s[1] >= L'0' && s[1] <= L'9') return true;
    // Double letter: AA, BB
    if (len == 2 && s[0] == s[1] && s[0] >= L'A' && s[0] <= L'D') return true;
    // Letter + digit + digit: A10, B12
    if (len == 3 && s[0] >= L'A' && s[0] <= L'D' && s[1] >= L'0' && s[1] <= L'9' && s[2] >= L'0' && s[2] <= L'9') return true;
    return false;
}

static void ParseFilename(const wchar_t* filename, wchar_t* artist, int aLen, wchar_t* title, int tLen)
{
    artist[0] = title[0] = L'\0';
    // Copy filename without extension
    wchar_t name[MAX_PATH];
    wcsncpy_s(name, filename, _TRUNCATE);
    wchar_t* dot = wcsrchr(name, L'.'); if (dot) *dot = L'\0';
    // Skip leading track numbers like "01. " or "01 - "
    wchar_t* p = name;
    while (*p >= L'0' && *p <= L'9') p++;
    while (*p == L'.' || *p == L' ' || *p == L'-') p++;

    // Find all " - " separators
    std::vector<wchar_t*> seps;
    {
        wchar_t* s = p;
        while ((s = wcsstr(s, L" - ")) != NULL) {
            seps.push_back(s);
            s += 3;
        }
    }

    if (seps.empty()) {
        // No separator — entire string is the title
        wcsncpy_s(title, tLen, p, _TRUNCATE);
    } else if (seps.size() == 1) {
        // One separator: check if left part is a vinyl side marker
        *seps[0] = L'\0';
        if (IsVinylSide(p, (int)wcslen(p))) {
            // "A - Artist - Title" with only one sep means "A - Rest"
            // Rest is the title (no artist detected)
            wcsncpy_s(title, tLen, seps[0] + 3, _TRUNCATE);
        } else {
            wcsncpy_s(artist, aLen, p, _TRUNCATE);
            wcsncpy_s(title, tLen, seps[0] + 3, _TRUNCATE);
        }
    } else {
        // Multiple separators: check if first part is a vinyl side marker
        *seps[0] = L'\0';
        if (IsVinylSide(p, (int)wcslen(p))) {
            // Skip the side marker, split on the NEXT separator
            // "C - Nu Moon - Stone Venus" → artist="Nu Moon", title="Stone Venus"
            *seps[1] = L'\0';
            wcsncpy_s(artist, aLen, seps[0] + 3, _TRUNCATE);
            wcsncpy_s(title, tLen, seps[1] + 3, _TRUNCATE);
        } else {
            // First part is the artist, rest is the title
            wcsncpy_s(artist, aLen, p, _TRUNCATE);
            // Rejoin remaining parts as title (everything after first " - ")
            *seps[0] = L' '; // restore
            wcsncpy_s(title, tLen, seps[0] + 3, _TRUNCATE);
        }
    }
}

// Search Discogs and populate g_dcgResults
static void DiscogsSearch(const wchar_t* artist, const wchar_t* title, HWND hStatus)
{
    g_dcgResults.clear();
    if (hStatus) SetWindowText(hStatus, L"Searching Discogs...");

    // Build search query
    std::wstring query;
    if (artist[0] && title[0])
        query = std::wstring(artist) + L" " + title;
    else if (title[0])
        query = title;
    else if (artist[0])
        query = artist;
    else { if (hStatus) SetWindowText(hStatus, L"Enter artist or title to search."); return; }

    std::string enc = UrlEncode(query.c_str());
    wchar_t url[1024];
    // Convert encoded query to wide
    wchar_t wenc[512] = {};
    MultiByteToWideChar(CP_UTF8, 0, enc.c_str(), -1, wenc, 512);
    swprintf_s(url, L"https://api.discogs.com/database/search?q=%s&type=release&per_page=20", wenc);

    std::string resp = DiscogsHttpGet(url);
    if (resp.empty()) {
        if (hStatus) SetWindowText(hStatus, L"Failed to connect. Check internet/token.");
        return;
    }

    // Parse results — find each "results":[...] entry
    // Simple approach: scan for "id": patterns within the results array
    const char* results = strstr(resp.c_str(), "\"results\"");
    if (!results) {
        if (hStatus) SetWindowText(hStatus, L"No results found.");
        return;
    }
    const char* arr = strchr(results, '[');
    if (!arr) { if (hStatus) SetWindowText(hStatus, L"No results found."); return; }

    // Scan for individual result objects within the array
    const char* p2 = arr + 1;
    int count = 0;
    while (count < 20) {
        const char* objStart = strchr(p2, '{');
        if (!objStart) break;
        // Find matching closing brace (simple depth tracking)
        int depth = 1;
        const char* q = objStart + 1;
        while (*q && depth > 0) {
            if (*q == '{') depth++;
            else if (*q == '}') depth--;
            q++;
        }
        if (depth != 0) break;

        // Extract fields from this object
        std::string obj(objStart, q - objStart);
        DiscogsResult dr = {};
        std::wstring wid = JsonGet(obj.c_str(), "id");
        dr.id = wid.empty() ? 0 : _wtoi(wid.c_str());
        std::wstring wtitle = JsonGet(obj.c_str(), "title");
        wcsncpy_s(dr.title, wtitle.c_str(), _TRUNCATE);
        std::wstring wyear = JsonGet(obj.c_str(), "year");
        wcsncpy_s(dr.year, wyear.c_str(), _TRUNCATE);
        std::wstring wcountry = JsonGet(obj.c_str(), "country");
        std::wstring wlabel = JsonGet(obj.c_str(), "label");
        std::wstring wcover = JsonGet(obj.c_str(), "cover_image");
        wcsncpy_s(dr.coverUrl, wcover.c_str(), _TRUNCATE);
        // Build Discogs web URL from ID
        swprintf_s(dr.resourceUrl, L"https://www.discogs.com/release/%d", dr.id);

        // The "title" field from search is usually "Artist - Title"
        // Parse it
        wchar_t* dashSep = wcsstr(dr.title, L" - ");
        if (dashSep) {
            *dashSep = L'\0';
            wcsncpy_s(dr.artist, dr.title, _TRUNCATE);
            wcsncpy_s(dr.title, dashSep + 3, _TRUNCATE);
        }

        // Genre: search results have "genre":["Electronic",...]
        const char* genreArr = strstr(obj.c_str(), "\"genre\"");
        if (genreArr) {
            const char* gb = strchr(genreArr, '[');
            if (gb) {
                const char* gs = strchr(gb, '"');
                if (gs) {
                    gs++;
                    const char* ge = strchr(gs, '"');
                    if (ge) {
                        std::string g(gs, ge - gs);
                        wchar_t wg[128] = {};
                        MultiByteToWideChar(CP_UTF8, 0, g.c_str(), -1, wg, 128);
                        wcsncpy_s(dr.genre, wg, _TRUNCATE);
                    }
                }
            }
        }

        // Build display string
        _snwprintf_s(dr.display, _countof(dr.display), _TRUNCATE, L"%s - %s%s%s%s%s",
            dr.artist[0] ? dr.artist : L"?",
            dr.title,
            dr.year[0] ? L" (" : L"",
            dr.year,
            dr.year[0] ? L")" : L"",
            dr.genre[0] ? (std::wstring(L" [") + dr.genre + L"]").c_str() : L"");

        g_dcgResults.push_back(dr);
        p2 = q;
        count++;
    }

    if (hStatus) {
        wchar_t msg[64]; swprintf_s(msg, L"Found %d result(s).", count);
        SetWindowText(hStatus, msg);
    }
}

// Fetch full release details from Discogs and extract more precise track info
static void DiscogsFetchRelease(int releaseId, DiscogsResult* out)
{
    wchar_t url[256]; swprintf_s(url, L"https://api.discogs.com/releases/%d", releaseId);
    std::string resp = DiscogsHttpGet(url);
    if (resp.empty()) return;

    // Extract fields
    std::wstring wtitle = JsonGet(resp.c_str(), "title");
    if (!wtitle.empty()) wcsncpy_s(out->title, wtitle.c_str(), _TRUNCATE);
    std::wstring wyear = JsonGet(resp.c_str(), "year");
    if (!wyear.empty()) wcsncpy_s(out->year, wyear.c_str(), _TRUNCATE);

    // Artists array: get first artist name
    const char* artists = strstr(resp.c_str(), "\"artists\"");
    if (artists) {
        std::wstring aname = JsonGet(artists, "name");
        if (!aname.empty()) wcsncpy_s(out->artist, aname.c_str(), _TRUNCATE);
    }

    // Genres
    const char* genres = strstr(resp.c_str(), "\"genres\"");
    if (genres) {
        const char* gb = strchr(genres, '[');
        if (gb) {
            const char* gs = strchr(gb, '"');
            if (gs) {
                gs++;
                const char* ge = strchr(gs, '"');
                if (ge) {
                    std::string g(gs, ge - gs);
                    wchar_t wg[128] = {};
                    MultiByteToWideChar(CP_UTF8, 0, g.c_str(), -1, wg, 128);
                    wcsncpy_s(out->genre, wg, _TRUNCATE);
                }
            }
        }
    }

    // Labels
    const char* labels = strstr(resp.c_str(), "\"labels\"");
    if (labels) {
        std::wstring lname = JsonGet(labels, "name");
        if (!lname.empty()) wcsncpy_s(out->label, lname.c_str(), _TRUNCATE);
    }
}

// Fetch tracklist from a Discogs release, returns vector of track titles
static std::vector<std::wstring> DiscogsFetchTracklist(int releaseId)
{
    std::vector<std::wstring> tracks;
    wchar_t url[256]; swprintf_s(url, L"https://api.discogs.com/releases/%d", releaseId);
    std::string resp = DiscogsHttpGet(url);
    if (resp.empty()) return tracks;

    // Find "tracklist" array
    const char* tl = strstr(resp.c_str(), "\"tracklist\"");
    if (!tl) return tracks;
    const char* arr = strchr(tl, '[');
    if (!arr) return tracks;

    // Parse each track object — look for "title" within each {...}
    const char* p = arr + 1;
    while (true) {
        const char* objStart = strchr(p, '{');
        if (!objStart) break;
        int depth = 1;
        const char* q = objStart + 1;
        while (*q && depth > 0) {
            if (*q == '{') depth++;
            else if (*q == '}') depth--;
            q++;
        }
        if (depth != 0) break;

        std::string obj(objStart, q - objStart);
        // Only include actual tracks (type_ == "track" or no type_)
        std::wstring type = JsonGet(obj.c_str(), "type_");
        if (type.empty() || type == L"track") {
            std::wstring title = JsonGet(obj.c_str(), "title");
            if (!title.empty())
                tracks.push_back(title);
        }
        p = q;
    }
    return tracks;
}

// Batch write tags to playlist files from a Discogs release
static void DiscogsFillAllTracks(int releaseId, DiscogsResult* release, HWND hStatus)
{
    // Build file list from either playlist or browser items (whichever is active)
    std::vector<std::wstring> files;
    if (g_browserActive) {
        for (auto& bi : g_browserItems)
            if (!bi.isDir) files.push_back(bi.path);
    } else {
        for (auto& t : g_playlist)
            files.push_back(t.path);
    }
    if (files.empty()) {
        if (hStatus) SetWindowText(hStatus, L"No audio files found in current view.");
        return;
    }

    if (hStatus) SetWindowText(hStatus, L"Fetching tracklist...");
    std::vector<std::wstring> tracklist = DiscogsFetchTracklist(releaseId);
    if (tracklist.empty()) {
        if (hStatus) SetWindowText(hStatus, L"No tracklist found for this release.");
        return;
    }

    int fileCount = (int)files.size();
    int trackCount = (int)tracklist.size();
    int toWrite = min(fileCount, trackCount);

    wchar_t msg[256];
    swprintf_s(msg, L"Release has %d track(s), playlist has %d file(s).\nWill tag %d file(s). Continue?",
        trackCount, fileCount, toWrite);
    if (MessageBox(g_hwndDiscogs, msg, L"Fill All Tracks", MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    // Stop playback if any of these files is playing
    bool wasPlaying = (currentStream != 0);
    int savedIdx = g_currentIndex;
    double savedPos = GetPlayPos();
    if (wasPlaying) StopAudio();

    // Download artwork once
    std::vector<BYTE> artImg;
    if (release->coverUrl[0]) {
        if (hStatus) SetWindowText(hStatus, L"Downloading artwork...");
        artImg = HttpGetBinary(release->coverUrl);
        if (!artImg.empty() && artImg.size() > 100) {
            FreeArtBytes();
            g_artSize = (DWORD)artImg.size();
            g_artBytes = new BYTE[g_artSize];
            memcpy(g_artBytes, artImg.data(), g_artSize);
        }
    }

    // Strip disambiguation from artist
    wchar_t cleanArtist[256];
    wcsncpy_s(cleanArtist, release->artist, _TRUNCATE);
    {
        wchar_t* paren = wcsrchr(cleanArtist, L'(');
        if (paren && paren > cleanArtist) {
            wchar_t* inner = paren + 1;
            bool isNum = true;
            while (*inner && *inner != L')') { if (*inner < L'0' || *inner > L'9') isNum = false; inner++; }
            if (isNum && *inner == L')') {
                wchar_t* trim = paren;
                while (trim > cleanArtist && *(trim - 1) == L' ') trim--;
                *trim = L'\0';
            }
        }
    }

    int okCount = 0;
    for (int i = 0; i < toWrite; i++) {
        wchar_t statusMsg[128];
        swprintf_s(statusMsg, L"Writing tags %d/%d...", i + 1, toWrite);
        if (hStatus) SetWindowText(hStatus, statusMsg);

        const wchar_t* path = files[i].c_str();
        const wchar_t* ext = wcsrchr(path, L'.');
        wchar_t trackNum[8]; swprintf_s(trackNum, L"%d", i + 1);
        const wchar_t* trackTitle = tracklist[i].c_str();

        bool ok = false;
        if (ext && _wcsicmp(ext, L".mp3") == 0)
            ok = WriteID3v2ToMP3(path, trackTitle, cleanArtist, release->title, release->year, trackNum, release->genre);
        else if (ext && _wcsicmp(ext, L".flac") == 0)
            ok = WriteTagsToFLAC(path, trackTitle, cleanArtist, release->title, release->year, trackNum, release->genre);
        else if (ext && _wcsicmp(ext, L".wav") == 0)
            ok = WriteID3v2ToWAV(path, trackTitle, cleanArtist, release->title, release->year, trackNum, release->genre);
        if (ok) okCount++;
    }

    // Clear thumbnail cache for all files
    ClearThumbCache();
    if (hListBox) InvalidateRect(hListBox, NULL, FALSE);

    // Restart playback
    if (wasPlaying && savedIdx >= 0 && savedIdx < (int)g_playlist.size()) {
        PlayIndex(savedIdx);
        if (savedPos > 1.0) SeekToSeconds(savedPos);
    }

    swprintf_s(msg, L"Done. Tagged %d/%d file(s) successfully.", okCount, toWrite);
    if (hStatus) SetWindowText(hStatus, msg);
    FreeArtBytes(); // clean up after batch write
}

static LRESULT CALLBACK DiscogsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        int cw = 580, y = 8;

        // Token field + "?" help button
        HWND hTokLbl = CreateWindow(L"STATIC", L"API Token:",
            WS_CHILD | WS_VISIBLE, 8, y + 2, 70, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hTokLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hTok = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", g_discogsToken,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            80, y, cw - 118, 22, hwnd, (HMENU)ID_DCG_TOKEN, hInst, NULL);
        SendMessage(hTok, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hHelp = CreateWindow(L"BUTTON", L"?",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            cw - 32, y, 24, 22, hwnd, (HMENU)ID_DCG_HELP, hInst, NULL);
        SendMessage(hHelp, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        y += 28;

        // Artist / Title fields + Search
        HWND hArtLbl = CreateWindow(L"STATIC", L"Artist:",
            WS_CHILD | WS_VISIBLE, 8, y + 2, 44, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hArtLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hArt = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            54, y, 200, 22, hwnd, (HMENU)ID_DCG_ARTIST, hInst, NULL);
        SendMessage(hArt, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hTitLbl = CreateWindow(L"STATIC", L"Title:",
            WS_CHILD | WS_VISIBLE, 260, y + 2, 36, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hTitLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hTit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            298, y, cw - 306 - 76, 22, hwnd, (HMENU)ID_DCG_TITLE, hInst, NULL);
        SendMessage(hTit, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hSrch = CreateWindow(L"BUTTON", L"Search",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            cw - 76, y, 68, 24, hwnd, (HMENU)ID_DCG_SEARCH, hInst, NULL);
        SendMessage(hSrch, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 30;

        // Left side: Results list. Right side: artwork preview
        int artSz = 120;
        int listW = cw - 16 - artSz - 8;
        HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
            8, y, listW, 220, hwnd, (HMENU)ID_DCG_LIST, hInst, NULL);
        SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Artwork preview (owner-draw static)
        CreateWindow(L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
            8 + listW + 8, y, artSz, artSz, hwnd, (HMENU)ID_DCG_ARTWORK, hInst, NULL);
        // "Open in Browser" button below artwork
        HWND hOpenUrl = CreateWindow(L"BUTTON", L"\u2197 Open in Browser",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            8 + listW + 8, y + artSz + 4, artSz, 24, hwnd, (HMENU)ID_DCG_OPEN_URL, hInst, NULL);
        SendMessage(hOpenUrl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 226;

        // Status
        HWND hStat = CreateWindow(L"STATIC", L"Enter search terms or auto-parsed from filename.",
            WS_CHILD | WS_VISIBLE, 8, y, cw - 16, 16, hwnd, (HMENU)ID_DCG_STATUS, hInst, NULL);
        SendMessage(hStat, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 22;

        // Apply + Fill All + Close
        HWND hApply = CreateWindow(L"BUTTON", L"Apply Tags",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            8, y, 100, 26, hwnd, (HMENU)ID_DCG_APPLY, hInst, NULL);
        SendMessage(hApply, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hFillAll = CreateWindow(L"BUTTON", L"Fill All Tracks",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            114, y, 120, 26, hwnd, (HMENU)ID_DCG_FILLALL, hInst, NULL);
        SendMessage(hFillAll, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hClo = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            cw - 78, y, 70, 26, hwnd, (HMENU)ID_DCG_CLOSE, hInst, NULL);
        SendMessage(hClo, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Auto-parse filename into fields
        if (g_hwndInfoParent) {
            wchar_t fn[MAX_PATH] = {};
            // Get path from Audio Info static (the path edit control)
            // Use the static s_filePath stored in InfoWndProc
            GetWindowText(g_hwndInfoParent, fn, MAX_PATH); // window title has filename
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_DCG_SEARCH: {
            // Save token
            GetDlgItemText(hwnd, ID_DCG_TOKEN, g_discogsToken, _countof(g_discogsToken));
            SaveSettings();
            // Get search terms
            wchar_t artist[256] = {}, title[256] = {};
            GetDlgItemText(hwnd, ID_DCG_ARTIST, artist, 256);
            GetDlgItemText(hwnd, ID_DCG_TITLE, title, 256);
            HWND hStatus = GetDlgItem(hwnd, ID_DCG_STATUS);
            DiscogsSearch(artist, title, hStatus);
            // Populate list
            HWND hList = GetDlgItem(hwnd, ID_DCG_LIST);
            SendMessage(hList, LB_RESETCONTENT, 0, 0);
            for (auto& r : g_dcgResults)
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)r.display);
            if (!g_dcgResults.empty())
                SendMessage(hList, LB_SETCURSEL, 0, 0);
            break;
        }
        case ID_DCG_APPLY: {
            HWND hList = GetDlgItem(hwnd, ID_DCG_LIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
            if (sel == LB_ERR || sel >= (int)g_dcgResults.size()) {
                MessageBox(hwnd, L"Select a result first.", L"Discogs", MB_ICONWARNING);
                break;
            }
            SetDlgItemText(hwnd, ID_DCG_STATUS, L"Fetching release details...");
            DiscogsResult& r = g_dcgResults[sel];
            DiscogsFetchRelease(r.id, &r);
            // Strip Discogs disambiguation "(N)" from artist name
            {
                wchar_t* paren = wcsrchr(r.artist, L'(');
                if (paren && paren > r.artist) {
                    // Check it's just a number in parens at the end, e.g. " (8)"
                    wchar_t* inner = paren + 1;
                    bool isNum = true;
                    while (*inner && *inner != L')') { if (*inner < L'0' || *inner > L'9') isNum = false; inner++; }
                    if (isNum && *inner == L')') {
                        // Trim trailing space before the paren
                        wchar_t* trim = paren;
                        while (trim > r.artist && *(trim-1) == L' ') trim--;
                        *trim = L'\0';
                    }
                }
            }
            // Populate Audio Info fields
            if (g_hwndInfoParent && IsWindow(g_hwndInfoParent)) {
                SetDlgItemText(g_hwndInfoParent, ID_INFO_TITLE, r.title);
                SetDlgItemText(g_hwndInfoParent, ID_INFO_ARTIST, r.artist);
                SetDlgItemText(g_hwndInfoParent, ID_INFO_ALBUM, r.title); // release title = album
                SetDlgItemText(g_hwndInfoParent, ID_INFO_YEAR, r.year);
                SetDlgItemText(g_hwndInfoParent, ID_INFO_GENRE, r.genre);
            }
            // Download cover artwork from Discogs
            if (r.coverUrl[0]) {
                SetDlgItemText(hwnd, ID_DCG_STATUS, L"Downloading cover artwork...");
                std::vector<BYTE> imgData = HttpGetBinary(r.coverUrl);
                if (!imgData.empty() && imgData.size() > 100) {
                    // Store as global artwork bytes for Save Tags to embed
                    FreeArtBytes();
                    g_artSize = (DWORD)imgData.size();
                    g_artBytes = new BYTE[g_artSize];
                    memcpy(g_artBytes, imgData.data(), g_artSize);
                    // Detect extension
                    if (imgData[0] == 0x89 && imgData[1] == 'P') wcscpy_s(g_artExt, L".png");
                    else wcscpy_s(g_artExt, L".jpg");
                    // Refresh artwork display in Audio Info
                    if (g_hwndInfoParent && IsWindow(g_hwndInfoParent)) {
                        HWND hArtCtl = GetDlgItem(g_hwndInfoParent, ID_INFO_ARTWORK);
                        if (hArtCtl) InvalidateRect(hArtCtl, NULL, TRUE);
                    }
                    SetDlgItemText(hwnd, ID_DCG_STATUS, L"Tags + artwork applied. Click Save Tags to write.");
                } else {
                    SetDlgItemText(hwnd, ID_DCG_STATUS, L"Tags applied (artwork download failed). Click Save Tags to write.");
                }
            } else {
                SetDlgItemText(hwnd, ID_DCG_STATUS, L"Tags applied (no cover available). Click Save Tags to write.");
            }
            break;
        }
        case ID_DCG_LIST:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                SendMessage(hwnd, WM_COMMAND, ID_DCG_APPLY, 0);
            }
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                // Load artwork preview for selected result
                HWND hList2 = GetDlgItem(hwnd, ID_DCG_LIST);
                int sel2 = (int)SendMessage(hList2, LB_GETCURSEL, 0, 0);
                if (g_dcgPreviewBmp) { DeleteObject(g_dcgPreviewBmp); g_dcgPreviewBmp = NULL; }
                if (sel2 >= 0 && sel2 < (int)g_dcgResults.size() && g_dcgResults[sel2].coverUrl[0]) {
                    SetDlgItemText(hwnd, ID_DCG_STATUS, L"Loading preview...");
                    std::vector<BYTE> img = HttpGetBinary(g_dcgResults[sel2].coverUrl);
                    if (!img.empty() && img.size() > 100) {
                        g_dcgPreviewBmp = DecodeImageToThumb(img.data(), (DWORD)img.size(), 120);
                        // Store image bytes so Apply Tags can embed them
                        FreeArtBytes();
                        g_artSize = (DWORD)img.size();
                        g_artBytes = new BYTE[g_artSize];
                        memcpy(g_artBytes, img.data(), g_artSize);
                        if (img[0] == 0x89 && img[1] == 'P') wcscpy_s(g_artExt, L".png");
                        else wcscpy_s(g_artExt, L".jpg");
                        // Update Audio Info artwork preview in real-time
                        if (g_hwndInfoParent && IsWindow(g_hwndInfoParent)) {
                            // Send a custom refresh: decode at 220px for the info dialog
                            HBITMAP hNewArt = DecodeImageToThumb(img.data(), (DWORD)img.size(), 220);
                            if (hNewArt) {
                                // The hArt static is in InfoWndProc — post a message to refresh it
                                // Simplest: just invalidate the artwork control
                                SendMessage(g_hwndInfoParent, WM_APP + 10, (WPARAM)hNewArt, 0);
                            }
                        }
                    }
                    SetDlgItemText(hwnd, ID_DCG_STATUS,
                        g_dcgPreviewBmp ? L"Preview loaded. Double-click or Apply to use." : L"No preview available.");
                }
                HWND hArtPrev = GetDlgItem(hwnd, ID_DCG_ARTWORK);
                if (hArtPrev) InvalidateRect(hArtPrev, NULL, TRUE);
            }
            break;
        case ID_DCG_HELP:
            ShellExecute(hwnd, L"open", L"https://www.discogs.com/settings/developers", NULL, NULL, SW_SHOW);
            break;
        case ID_DCG_OPEN_URL: {
            HWND hList3 = GetDlgItem(hwnd, ID_DCG_LIST);
            int sel3 = (int)SendMessage(hList3, LB_GETCURSEL, 0, 0);
            if (sel3 >= 0 && sel3 < (int)g_dcgResults.size() && g_dcgResults[sel3].resourceUrl[0])
                ShellExecute(hwnd, L"open", g_dcgResults[sel3].resourceUrl, NULL, NULL, SW_SHOW);
            else
                MessageBox(hwnd, L"Select a result first.", L"Discogs", MB_ICONWARNING);
            break;
        }
        case ID_DCG_FILLALL: {
            HWND hList4 = GetDlgItem(hwnd, ID_DCG_LIST);
            int sel4 = (int)SendMessage(hList4, LB_GETCURSEL, 0, 0);
            if (sel4 == LB_ERR || sel4 >= (int)g_dcgResults.size()) {
                MessageBox(hwnd, L"Select a release first.", L"Fill All", MB_ICONWARNING);
                break;
            }
            DiscogsResult& r4 = g_dcgResults[sel4];
            DiscogsFetchRelease(r4.id, &r4);
            // Strip disambiguation
            {
                wchar_t* pa = wcsrchr(r4.artist, L'(');
                if (pa && pa > r4.artist) {
                    wchar_t* in = pa + 1; bool num = true;
                    while (*in && *in != L')') { if (*in < L'0' || *in > L'9') num = false; in++; }
                    if (num && *in == L')') { wchar_t* t = pa; while (t > r4.artist && *(t-1)==L' ') t--; *t = L'\0'; }
                }
            }
            DiscogsFillAllTracks(r4.id, &r4, GetDlgItem(hwnd, ID_DCG_STATUS));
            break;
        }
        case ID_DCG_CLOSE:
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        }
        break;
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID == ID_DCG_ARTWORK) {
            HDC dc = dis->hDC; RECT rc = dis->rcItem;
            if (g_dcgPreviewBmp) {
                HDC mdc = CreateCompatibleDC(dc);
                HBITMAP ob = (HBITMAP)SelectObject(mdc, g_dcgPreviewBmp);
                int w = rc.right - rc.left, h = rc.bottom - rc.top;
                StretchBlt(dc, rc.left, rc.top, w, h, mdc, 0, 0, 120, 120, SRCCOPY);
                SelectObject(mdc, ob); DeleteDC(mdc);
                HPEN pen = CreatePen(PS_SOLID, 1, g_darkMode ? 0x555555 : 0xBBBBBB);
                HPEN op = (HPEN)SelectObject(dc, pen);
                SelectObject(dc, GetStockObject(NULL_BRUSH));
                Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
                SelectObject(dc, op); DeleteObject(pen);
            } else {
                HBRUSH br = CreateSolidBrush(g_darkMode ? 0x383838 : 0xDDDDDD);
                FillRect(dc, &rc, br); DeleteObject(br);
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, g_darkMode ? 0x888888 : 0x777777);
                HFONT of = (HFONT)SelectObject(dc, g_fontUI);
                DrawText(dc, L"No Preview", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(dc, of);
            }
            return TRUE;
        }
        break;
    }
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_KEYDOWN: if (wParam == VK_ESCAPE) DestroyWindow(hwnd); break;
    case WM_DESTROY:
        if (g_dcgPreviewBmp) { DeleteObject(g_dcgPreviewBmp); g_dcgPreviewBmp = NULL; }
        g_hwndDiscogs = NULL; g_dcgResults.clear(); break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenDiscogsDialog(HWND hwndInfoParent, const wchar_t* filename)
{
    if (g_hwndDiscogs && IsWindow(g_hwndDiscogs)) { SetForegroundWindow(g_hwndDiscogs); return; }
    g_hwndInfoParent = hwndInfoParent;
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t DCG_CLASS[] = L"BillyDiscogsWnd";
    static bool dcgReg = false;
    if (!dcgReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = DiscogsWndProc; wc.hInstance = hInst;
        wc.lpszClassName = DCG_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); dcgReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 580, 380 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndDiscogs = CreateWindowEx(WS_EX_APPWINDOW, DCG_CLASS,
        L"Tag Database (Discogs)", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
    // Center over info dialog
    RECT pr; GetWindowRect(hwndInfoParent, &pr);
    SetWindowPos(g_hwndDiscogs, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
    // Auto-fill artist/title from filename
    if (filename && filename[0]) {
        wchar_t artist[256] = {}, title[256] = {};
        ParseFilename(Filename(filename), artist, 256, title, 256);
        SetDlgItemText(g_hwndDiscogs, ID_DCG_ARTIST, artist);
        SetDlgItemText(g_hwndDiscogs, ID_DCG_TITLE, title);
    }
}

// ============================================================
//  Internet Radio (radio-browser.info API)
// ============================================================
static HWND g_hwndRadio = NULL;
// g_radioStream, g_radioPlaying, g_radioStationName, g_radioNowPlaying declared in globals section

// Saved radio favorites
struct RadioFav { std::wstring name; std::wstring url; std::wstring uuid; };
static std::vector<RadioFav> g_radioFavs;
static bool g_radioShowingFavs = false;

static void GetRadioFavPath(wchar_t* out, int maxLen)
{
    GetIniPath();
    wcsncpy_s(out, maxLen, g_iniPath, _TRUNCATE);
    wchar_t* sl = wcsrchr(out, L'\\'); if (sl) sl[1] = L'\0';
    wcscat_s(out, maxLen, L"library\\");
    CreateDirectoryW(out, NULL);
    wcscat_s(out, maxLen, L"radio_favorites.txt");
}

static void LoadRadioFavs()
{
    g_radioFavs.clear();
    wchar_t path[MAX_PATH]; GetRadioFavPath(path, MAX_PATH);
    FILE* f = NULL; _wfopen_s(&f, path, L"r,ccs=UTF-8");
    if (!f) return;
    wchar_t line[1024];
    while (fgetws(line, _countof(line), f)) {
        int len = (int)wcslen(line);
        while (len > 0 && (line[len-1] == L'\r' || line[len-1] == L'\n')) line[--len] = 0;
        if (len == 0) continue;
        // Format: name|url|uuid
        wchar_t* p1 = wcschr(line, L'|');
        if (!p1) continue;
        *p1 = 0; wchar_t* p2 = wcschr(p1 + 1, L'|');
        RadioFav rf;
        rf.name = line;
        if (p2) { *p2 = 0; rf.url = p1 + 1; rf.uuid = p2 + 1; }
        else { rf.url = p1 + 1; }
        g_radioFavs.push_back(rf);
    }
    fclose(f);
}

static void SaveRadioFavs()
{
    wchar_t path[MAX_PATH]; GetRadioFavPath(path, MAX_PATH);
    FILE* f = NULL; _wfopen_s(&f, path, L"w,ccs=UTF-8");
    if (!f) return;
    for (auto& rf : g_radioFavs)
        fwprintf(f, L"%s|%s|%s\n", rf.name.c_str(), rf.url.c_str(), rf.uuid.c_str());
    fclose(f);
}

static bool IsRadioFav(const wchar_t* url)
{
    for (auto& rf : g_radioFavs) if (rf.url == url) return true;
    return false;
}

struct RadioStation {
    wchar_t name[256];
    wchar_t country[64];
    wchar_t codec[32];      // MP3, AAC+, OGG, etc.
    wchar_t tags[256];      // genre/tags
    wchar_t url[512];       // stream URL
    wchar_t uuid[64];       // station UUID
    int     bitrate;
    int     votes;
    wchar_t display[512];
};
static std::vector<RadioStation> g_radioResults;

// radio-browser.info servers (try multiple on failure)
static const wchar_t* g_radioServers[] = {
    L"de1.api.radio-browser.info",
    L"de2.api.radio-browser.info",
    L"at1.api.radio-browser.info",
    L"nl1.api.radio-browser.info",
    L"fi1.api.radio-browser.info",
};
static const int g_radioServerCount = 5;

// HTTP GET for radio API — tries multiple servers on failure
static std::string RadioHttpGet(const wchar_t* path)
{
    // Randomize server order
    int order[5] = {0,1,2,3,4};
    for (int i = 4; i > 0; i--) std::swap(order[i], order[rand() % (i+1)]);

    for (int attempt = 0; attempt < g_radioServerCount; attempt++) {
        const wchar_t* host = g_radioServers[order[attempt]];
        std::string result;
        HINTERNET hSession = WinHttpOpen(L"BillyPro/0.5", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) continue;
        WinHttpSetTimeouts(hSession, 5000, 5000, 5000, 5000);
        HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); continue; }
        HINTERNET hReq = WinHttpOpenRequest(hConnect, L"GET", path, NULL,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!hReq) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); continue; }
        if (WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
            WinHttpReceiveResponse(hReq, NULL)) {
        DWORD size = 0;
        while (WinHttpQueryDataAvailable(hReq, &size) && size > 0) {
            std::string buf(size, '\0');
            DWORD read = 0;
            WinHttpReadData(hReq, &buf[0], size, &read);
            result.append(buf, 0, read);
        }
        }
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        if (!result.empty()) return result; // success — stop retrying
    }
    return ""; // all servers failed
}

static void RadioSearch(const wchar_t* query, const wchar_t* genre, HWND hStatus)
{
    g_radioResults.clear();
    if (hStatus) SetWindowText(hStatus, L"Searching stations...");

    std::string encQuery = UrlEncode(query && query[0] ? query : L"");
    std::string encGenre = UrlEncode(genre && genre[0] ? genre : L"");

    wchar_t path[1024];
    wchar_t wq[512] = {}, wg[256] = {};
    MultiByteToWideChar(CP_UTF8, 0, encQuery.c_str(), -1, wq, 512);
    MultiByteToWideChar(CP_UTF8, 0, encGenre.c_str(), -1, wg, 256);

    if (wq[0] && wg[0])
        swprintf_s(path, L"/json/stations/search?name=%s&tag=%s&limit=50&order=votes&reverse=true", wq, wg);
    else if (wq[0])
        swprintf_s(path, L"/json/stations/search?name=%s&limit=50&order=votes&reverse=true", wq);
    else if (wg[0])
        swprintf_s(path, L"/json/stations/search?tag=%s&limit=50&order=votes&reverse=true", wg);
    else
        swprintf_s(path, L"/json/stations/topvote?limit=50");

    std::string resp = RadioHttpGet(path);
    if (resp.empty()) {
        if (hStatus) SetWindowText(hStatus, L"Failed to connect. Try again.");
        return;
    }

    // Parse JSON array of station objects
    const char* p = resp.c_str();
    const char* arrStart = strchr(p, '[');
    if (!arrStart) { if (hStatus) SetWindowText(hStatus, L"No results."); return; }
    p = arrStart + 1;
    int count = 0;
    while (count < 50) {
        const char* objStart = strchr(p, '{');
        if (!objStart) break;
        int depth = 1;
        const char* q = objStart + 1;
        while (*q && depth > 0) { if (*q == '{') depth++; else if (*q == '}') depth--; q++; }
        if (depth != 0) break;
        std::string obj(objStart, q - objStart);

        RadioStation rs = {};
        std::wstring wname = JsonGet(obj.c_str(), "name");
        wcsncpy_s(rs.name, wname.c_str(), _TRUNCATE);
        std::wstring wcountry = JsonGet(obj.c_str(), "countrycode");
        wcsncpy_s(rs.country, wcountry.c_str(), _TRUNCATE);
        std::wstring wtags = JsonGet(obj.c_str(), "tags");
        wcsncpy_s(rs.tags, wtags.c_str(), _TRUNCATE);
        std::wstring wurl = JsonGet(obj.c_str(), "url_resolved");
        if (wurl.empty()) wurl = JsonGet(obj.c_str(), "url");
        wcsncpy_s(rs.url, wurl.c_str(), _TRUNCATE);
        std::wstring wuuid = JsonGet(obj.c_str(), "stationuuid");
        wcsncpy_s(rs.uuid, wuuid.c_str(), _TRUNCATE);
        std::wstring wbr = JsonGet(obj.c_str(), "bitrate");
        rs.bitrate = wbr.empty() ? 0 : _wtoi(wbr.c_str());
        std::wstring wcodec = JsonGet(obj.c_str(), "codec");
        wcsncpy_s(rs.codec, wcodec.empty() ? L"?" : wcodec.c_str(), _TRUNCATE);
        std::wstring wvotes = JsonGet(obj.c_str(), "votes");
        rs.votes = wvotes.empty() ? 0 : _wtoi(wvotes.c_str());

        if (rs.url[0] && rs.name[0]) {
            // Build display: Name | Codec | Bitrate | Country
            bool fav = IsRadioFav(rs.url);
            _snwprintf_s(rs.display, _countof(rs.display), _TRUNCATE,
                L"%s%s  |  %s  %dkbps  [%s]",
                fav ? L"\u2605 " : L"", rs.name, rs.codec, rs.bitrate, rs.country);
            g_radioResults.push_back(rs);
            count++;
        }
        p = q;
    }

    if (hStatus) {
        wchar_t msg[64]; swprintf_s(msg, L"Found %d station(s).", count);
        SetWindowText(hStatus, msg);
    }
}

// Register a click with the API (as per guidelines)
static void RadioRegisterClick(const wchar_t* uuid)
{
    wchar_t path[256]; swprintf_s(path, L"/json/url/%s", uuid);
    RadioHttpGet(path); // fire and forget
}

struct RadioConnectData { std::string url8; std::string uuid8; HWND hwnd; LONG gen; };
static HANDLE g_radioConnThread = NULL;

static DWORD WINAPI RadioConnectThread(LPVOID param)
{
    RadioConnectData* d = (RadioConnectData*)param;
    // Register click on background thread (was blocking UI)
    if (!d->uuid8.empty()) {
        wchar_t uuidW[64] = {};
        MultiByteToWideChar(CP_UTF8, 0, d->uuid8.c_str(), -1, uuidW, 64);
        wchar_t path[256]; swprintf_s(path, L"/json/url/%s", uuidW);
        RadioHttpGet(path);
    }
    // Set a connection timeout so dead streams don't hang forever
    BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 10000);
    HSTREAM stream = BASS_StreamCreateURL(d->url8.c_str(), 0,
        BASS_SAMPLE_FLOAT, NULL, NULL);
    PostMessage(d->hwnd, WM_RADIO_CONNECTED, (WPARAM)stream, (LPARAM)d->gen);
    delete d;
    return 0;
}

// Launch an async radio connect for a plain URL (no click registration).
// Each launch gets a fresh generation token; results from older launches
// (or launches cancelled by StopAudio) are discarded in RadioOnConnected.
static void RadioConnectAsync(const wchar_t* url)
{
    RadioConnectData* d = new RadioConnectData();
    int uLen = WideCharToMultiByte(CP_UTF8, 0, url, -1, NULL, 0, NULL, NULL);
    d->url8.resize(uLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, url, -1, &d->url8[0], uLen, NULL, NULL);
    d->hwnd = g_hwnd;
    d->gen = InterlockedIncrement(&g_radioConnGen);
    if (g_radioConnThread) { CloseHandle(g_radioConnThread); g_radioConnThread = NULL; }
    g_radioConnThread = CreateThread(NULL, 0, RadioConnectThread, d, 0, NULL);
    if (!g_radioConnThread) delete d;   // thread never ran — don't leak the payload
}

// Next/Prev while radio plays: no queue exists, so step through the saved
// radio favorites instead (media keys / touchpad gestures / SMTC included).
static void RadioPlayFavOffset(int dir)
{
    LoadRadioFavs();
    if (g_radioFavs.empty()) return;
    // Find the current station among favorites BEFORE StopAudio clears the URL
    int cur = -1;
    for (int i = 0; i < (int)g_radioFavs.size(); i++)
        if (_wcsicmp(g_radioFavs[i].url.c_str(), g_radioReconnUrl) == 0) { cur = i; break; }
    int next = (cur < 0) ? 0 : (cur + dir + (int)g_radioFavs.size()) % (int)g_radioFavs.size();
    RadioFav rf = g_radioFavs[next];  // copy — StopAudio may trigger reloads

    StopAudio();
    if (g_radioStream) { BASS_StreamFree(g_radioStream); g_radioStream = 0; }
    g_radioPlaying = false;
    wcsncpy_s(g_radioStationName, rf.name.c_str(), _TRUNCATE);
    wcsncpy_s(g_radioReconnUrl, rf.url.c_str(), _TRUNCATE);
    g_radioNowPlaying[0] = L'\0';

    ClearPlaylist();
    Track t = {};
    wcsncpy_s(t.path, rf.url.c_str(), _TRUNCATE);
    { wchar_t pfx[4] = { 0x25B6, L' ', 0 }; _snwprintf_s(t.display, _countof(t.display), _TRUNCATE, L"%s%s", pfx, rf.name.c_str()); }
    g_playlist.push_back(t);
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        SendMessage(hListBox, LB_SETCURSEL, 0, 0);
    }
    g_currentIndex = 0;
    wchar_t title[512];
    _snwprintf_s(title, _countof(title), _TRUNCATE, L"%s (connecting...) - BillyPro", rf.name.c_str());
    SetWindowText(g_hwnd, title);
    UpdateStatusBar();
    RadioConnectAsync(rf.url.c_str());
}

static int RadioGetReconnDelay()
{
    if (g_radioReconnAttempt <= 5) return 1;
    if (g_radioReconnAttempt <= 10) return 2;
    if (g_radioReconnAttempt <= 15) return 3;
    return 5;
}

static void RadioStartReconnCountdown()
{
    g_radioReconnAttempt++;
    g_radioReconnCountdown = RadioGetReconnDelay();
    wchar_t title[512];
    _snwprintf_s(title, _countof(title), _TRUNCATE, L"%s (reconnecting %d...) - BillyPro",
        g_radioStationName, g_radioReconnCountdown);
    SetWindowText(g_hwnd, title);
    wchar_t status[256];
    swprintf_s(status, L"Connection lost. Reconnecting in %d... (attempt %d)",
        g_radioReconnCountdown, g_radioReconnAttempt);
    if (g_hwndRadio) SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, status);
    g_radioPlaying = true;
    SetTimer(g_hwnd, IDT_RADIO_RECONN, 1000, NULL);
}

static void RadioLaunchReconnect()
{
    KillTimer(g_hwnd, IDT_RADIO_RECONN);
    if (!g_radioReconnUrl[0]) return;
    wchar_t title[512];
    _snwprintf_s(title, _countof(title), _TRUNCATE, L"%s (connecting...) - BillyPro", g_radioStationName);
    SetWindowText(g_hwnd, title);
    if (g_hwndRadio) SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, L"Connecting...");
    RadioConnectAsync(g_radioReconnUrl);
}

static void RadioOnConnected(HSTREAM stream, LONG gen)
{
    if (g_radioConnThread) { CloseHandle(g_radioConnThread); g_radioConnThread = NULL; }
    // Stale result: user stopped radio or started a different station while
    // this connect thread was in flight. Discard so it can't hijack playback.
    if (gen != g_radioConnGen) {
        if (stream) BASS_StreamFree(stream);
        return;
    }
    if (!stream) {
        RadioStartReconnCountdown();
        return;
    }
    // Record the stream/station in play history (dedup skips reconnects and the
    // direct-link entry already logged by PlayIndex)
    RecordPlayHistory(g_radioReconnUrl, g_radioStationName[0] ? g_radioStationName : g_radioReconnUrl);
    g_radioReconnAttempt = 0;
    g_radioReconnCountdown = 0;
    // NOTE: keep g_radioReconnUrl — it identifies the current station so
    // Next/Prev (media keys) can find it among the radio favorites and skip
    // to the neighbouring station instead of replaying the same one.
    KillTimer(g_hwnd, IDT_RADIO_RECONN);

    // Direct media file over HTTP (finite length, e.g. a link to an .mp3):
    // play as a normal seekable track — with position, duration and seeking —
    // instead of an endless "LIVE" radio stream.
    QWORD urlLen = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
    if (urlLen != (QWORD)-1 && urlLen > 0) {
        g_radioPlaying = false;
        g_radioStationName[0] = L'\0';
        g_radioNowPlaying[0] = L'\0';
        currentStream = stream;
        BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
        ApplyDSP();
        BASS_ChannelSetSync(currentStream, BASS_SYNC_END, 0, EndSyncProc, g_hwnd);
        BASS_ChannelPlay(currentStream, FALSE);
        SetTimer(g_hwnd, IDT_PLAYBACK, 16, NULL);
        SetTimer(g_hwnd, IDT_PEAK_METER, 50, NULL);
        if (hTimeTot) {
            wchar_t tbuf[32], tot[40];
            FormatTime(GetTrackLength(), tbuf, _countof(tbuf));
            swprintf_s(tot, L"/ %s", tbuf);
            SetWindowText(hTimeTot, tot);
        }
        UpdatePlayBtn();
        UpdateStatusBar();
        UpdateWindowTitle();
        UpdateThumbButtons();
        UpdateTimeDisplays();
        SmtcUpdateMetadata();
        SmtcUpdatePlaybackStatus();
        if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
        if (g_hwndRadio) SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, L"Playing direct stream file.");
        return;
    }

    g_radioPlayStart = GetTickCount64();
    currentStream = stream;
    g_radioStream = stream;
    g_radioPlaying = true;
    BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL, currentVolume);
    ApplyDSP();
    ApplyPitch();
    BASS_ChannelPlay(currentStream, FALSE);
    SetTimer(g_hwnd, IDT_PLAYBACK, 16, NULL);
    SetTimer(g_hwnd, IDT_PEAK_METER, 50, NULL);
    SetTimer(g_hwnd, IDT_RADIO_META, 2000, NULL);
    UpdatePlayBtn();
    UpdateStatusBar();
    UpdateThumbButtons();
    // Lock screen / media overlay (SMTC): show the station like foobar does
    SmtcUpdateMetadata();
    SmtcUpdatePlaybackStatus();
    wchar_t title[512];
    _snwprintf_s(title, _countof(title), _TRUNCATE, L"%s - BillyPro", g_radioStationName);
    SetWindowText(g_hwnd, title);
    if (g_hwndRadio) {
        wchar_t status[512];
        wchar_t pfx2[4] = { 0x25B6, L' ', 0 };
        _snwprintf_s(status, _countof(status), _TRUNCATE, L"%s%s", pfx2, g_radioStationName);
        SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, status);
    }
}

static void RadioPlayStation(int idx)
{
    if (idx < 0 || idx >= (int)g_radioResults.size()) return;
    RadioStation& rs = g_radioResults[idx];

    StopAudio();
    if (g_radioStream) { BASS_StreamFree(g_radioStream); g_radioStream = 0; }
    g_radioPlaying = false;

    g_radioReconnAttempt = 0;
    g_radioReconnCountdown = 0;
    g_loudStreamStep = 2;   // fresh stream always starts at step 2
    KillTimer(g_hwnd, IDT_RADIO_RECONN);
    wcsncpy_s(g_radioStationName, rs.name, _TRUNCATE);
    wcsncpy_s(g_radioReconnUrl, rs.url, _TRUNCATE);
    g_radioNowPlaying[0] = L'\0';
    if (g_hwndRadio) SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, L"Connecting...");

    ClearPlaylist();
    Track t = {};
    wcsncpy_s(t.path, rs.url, _TRUNCATE);
    { wchar_t pfx[4] = { 0x25B6, L' ', 0 }; _snwprintf_s(t.display, _countof(t.display), _TRUNCATE, L"%s%s", pfx, rs.name); }
    g_playlist.push_back(t);
    if (hListBox) {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        SendMessage(hListBox, LB_SETCURSEL, 0, 0);
    }
    g_currentIndex = 0;
    wchar_t title[512];
    _snwprintf_s(title, _countof(title), _TRUNCATE, L"%s (connecting...) - BillyPro", rs.name);
    SetWindowText(g_hwnd, title);
    UpdateStatusBar();

    RadioConnectData* d = new RadioConnectData();
    int uLen = WideCharToMultiByte(CP_UTF8, 0, rs.url, -1, NULL, 0, NULL, NULL);
    d->url8.resize(uLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, rs.url, -1, &d->url8[0], uLen, NULL, NULL);
    int uuidLen = WideCharToMultiByte(CP_UTF8, 0, rs.uuid, -1, NULL, 0, NULL, NULL);
    d->uuid8.resize(uuidLen, '\0');
    WideCharToMultiByte(CP_UTF8, 0, rs.uuid, -1, &d->uuid8[0], uuidLen, NULL, NULL);
    d->hwnd = g_hwnd;
    d->gen = InterlockedIncrement(&g_radioConnGen);
    if (g_radioConnThread) { CloseHandle(g_radioConnThread); g_radioConnThread = NULL; }
    g_radioConnThread = CreateThread(NULL, 0, RadioConnectThread, d, 0, NULL);
    if (!g_radioConnThread) delete d;   // thread never ran — don't leak the payload
}
static WNDPROC g_OldRadioEditProc = NULL;
static LRESULT CALLBACK RadioEditSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
        if (wParam == 'A') {
            SendMessage(hwnd, EM_SETSEL, 0, -1);
            return 0;
        }
        if (wParam == VK_BACK || wParam == VK_DELETE) {
            wchar_t txt[512]; GetWindowText(hwnd, txt, 512);
            DWORD selStart = 0, selEnd = 0;
            SendMessage(hwnd, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
            int len = (int)wcslen(txt);
            int pos = (int)selStart;
            if (wParam == VK_DELETE) {
                int e = pos;
                while (e < len && txt[e] == L' ') e++;
                while (e < len && txt[e] != L' ') e++;
                SendMessage(hwnd, EM_SETSEL, pos, e);
                SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)L"");
            } else {
                int s = pos;
                while (s > 0 && txt[s-1] == L' ') s--;
                while (s > 0 && txt[s-1] != L' ') s--;
                SendMessage(hwnd, EM_SETSEL, s, pos);
                SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)L"");
            }
            return 0;
        }
    }
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, ID_RADIO_SEARCH, 0);
        return 0;
    }
    // Suppress the 0x7F character that Ctrl+Backspace generates
    if (msg == WM_CHAR && wParam == 0x7F) return 0;
    return CallWindowProc(g_OldRadioEditProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK RadioWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        int cw = 560, y = 8;

        // Search fields
        HWND hLbl = CreateWindow(L"STATIC", L"Search:",
            WS_CHILD | WS_VISIBLE, 8, y + 2, 48, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hQuery = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            58, y, 200, 22, hwnd, (HMENU)ID_RADIO_QUERY, hInst, NULL);
        SendMessage(hQuery, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hGLbl = CreateWindow(L"STATIC", L"Genre:",
            WS_CHILD | WS_VISIBLE, 264, y + 2, 42, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hGLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hGenre = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            308, y, 160, 22, hwnd, (HMENU)ID_RADIO_GENRE, hInst, NULL);
        SendMessage(hGenre, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        // Subclass both edit fields for Ctrl+A, Ctrl+Backspace, Ctrl+Delete, Enter
        g_OldRadioEditProc = (WNDPROC)SetWindowLongPtr(hQuery, GWLP_WNDPROC, (LONG_PTR)RadioEditSubProc);
        SetWindowLongPtr(hGenre, GWLP_WNDPROC, (LONG_PTR)RadioEditSubProc);
        HWND hSrch = CreateWindow(L"BUTTON", L"Search",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            cw - 76, y, 68, 24, hwnd, (HMENU)ID_RADIO_SEARCH, hInst, NULL);
        SendMessage(hSrch, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 28;

        // Genre preset buttons (two rows)
        {
            static const struct { const wchar_t* label; const wchar_t* tag; } presets[] = {
                {L"\u2605 Favs", L""},  {L"Top", L"__top__"},  {L"Electronic", L"electronic"},
                {L"House", L"house"},  {L"Techno", L"techno"},  {L"DnB", L"drum and bass"},
                {L"Ambient", L"ambient"},  {L"Chillout", L"chillout"},  {L"Trance", L"trance"},
                {L"Jazz", L"jazz"},  {L"Rock", L"rock"},  {L"Pop", L"pop"},
                {L"Classical", L"classical"},  {L"Hip-Hop", L"hip hop"},  {L"Lounge", L"lounge"},
                {L"News", L"news"},  {L"Talk", L"talk"},  {L"80s", L"80s"},
            };
            HFONT hSmall = CreateFont(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            int bx = 8, bw = 56, bh2 = 20, gap = 2;
            for (int i = 0; i < 18; i++) {
                if (i == 9) { bx = 8; y += bh2 + gap; } // second row
                HWND hPre = CreateWindow(L"BUTTON", presets[i].label,
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    bx, y, bw, bh2, hwnd, (HMENU)(UINT_PTR)(IDC_RADIO_PRESET_BASE + i), hInst, NULL);
                SendMessage(hPre, WM_SETFONT, (WPARAM)hSmall, TRUE);
                bx += bw + gap;
            }
            // Don't delete hSmall — it's used by the buttons for their lifetime
            y += bh2 + 4;
        }

        // Results list
        HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
            8, y, cw - 16, 260, hwnd, (HMENU)ID_RADIO_LIST, hInst, NULL);
        SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 286;

        // Status
        HWND hStat = CreateWindow(L"STATIC", L"Search for stations or leave empty for top stations.",
            WS_CHILD | WS_VISIBLE, 8, y, cw - 16, 16, hwnd, (HMENU)ID_RADIO_STATUS, hInst, NULL);
        SendMessage(hStat, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 22;

        // Play / Stop / Close
        HWND hPlay = CreateWindow(L"BUTTON", L"\u25B6 Play",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            8, y, 80, 26, hwnd, (HMENU)ID_RADIO_PLAY, hInst, NULL);
        SendMessage(hPlay, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hStop = CreateWindow(L"BUTTON", L"\u25A0 Stop",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            94, y, 80, 26, hwnd, (HMENU)ID_RADIO_STOP, hInst, NULL);
        SendMessage(hStop, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hClo = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            cw - 78, y, 70, 26, hwnd, (HMENU)ID_RADIO_CLOSE, hInst, NULL);
        SendMessage(hClo, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 30;

        // Credit link
        HWND hCredit = CreateWindow(L"STATIC", L"Powered by radio-browser.info",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOTIFY,
            8, y, cw - 16, 14, hwnd, (HMENU)499, hInst, NULL);
        {
            HFONT hSmallF = CreateFont(-11, 0, 0, 0, FW_NORMAL, 0, TRUE/*underline*/, 0,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            SendMessage(hCredit, WM_SETFONT, (WPARAM)hSmallF, TRUE);
        }

        // Load and show saved radio favorites
        g_radioShowingFavs = true;
        LoadRadioFavs();
        if (!g_radioFavs.empty()) {
            HWND hListInit = GetDlgItem(hwnd, ID_RADIO_LIST);
            g_radioResults.clear();
            for (auto& rf : g_radioFavs) {
                RadioStation rs = {};
                wcsncpy_s(rs.name, rf.name.c_str(), _TRUNCATE);
                wcsncpy_s(rs.url, rf.url.c_str(), _TRUNCATE);
                wcsncpy_s(rs.uuid, rf.uuid.c_str(), _TRUNCATE);
                _snwprintf_s(rs.display, _countof(rs.display), _TRUNCATE, L"\u2605 %s", rs.name);
                g_radioResults.push_back(rs);
                SendMessage(hListInit, LB_ADDSTRING, 0, (LPARAM)rs.display);
            }
            SetDlgItemText(hwnd, ID_RADIO_STATUS,
                L"Showing saved favorites. Search to find new stations.");
        }
        break;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
        if (wParam == VK_RETURN) {
            SendMessage(hwnd, WM_COMMAND, ID_RADIO_SEARCH, 0);
            return 0;
        }
        if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            HWND hFoc = GetFocus();
            if (hFoc) SendMessage(hFoc, EM_SETSEL, 0, -1);
            return 0;
        }
        break;
    case WM_COMMAND:
        // Handle Enter in edit fields (EN_KILLFOCUS won't work, use IDOK default button)
        if ((LOWORD(wParam) == ID_RADIO_QUERY || LOWORD(wParam) == ID_RADIO_GENRE) && HIWORD(wParam) == EN_CHANGE) {
            // Reset debounce timer — search triggers 1s after last keystroke
            KillTimer(hwnd, IDT_RADIO_DEBOUNCE);
            wchar_t q[8] = {}; GetDlgItemText(hwnd, ID_RADIO_QUERY, q, 8);
            wchar_t g[8] = {}; GetDlgItemText(hwnd, ID_RADIO_GENRE, g, 8);
            if (q[0] || g[0]) // only auto-search if something is typed
                SetTimer(hwnd, IDT_RADIO_DEBOUNCE, 1000, NULL);
            break;
        }
        // Genre preset buttons
        {
            int cmd = LOWORD(wParam);
            if (cmd >= IDC_RADIO_PRESET_BASE && cmd < IDC_RADIO_PRESET_BASE + 20) {
                static const wchar_t* presetTags[] = {
                    L"", L"__top__", L"electronic", L"house", L"techno", L"drum and bass",
                    L"ambient", L"chillout", L"trance", L"jazz", L"rock", L"pop",
                    L"classical", L"hip hop", L"lounge", L"news", L"talk", L"80s",
                };
                int pi = cmd - IDC_RADIO_PRESET_BASE;
                if (pi == 0) {
                    // Favorites
                    g_radioShowingFavs = true;
                    LoadRadioFavs();
                    g_radioResults.clear();
                    HWND hList = GetDlgItem(hwnd, ID_RADIO_LIST);
                    SendMessage(hList, LB_RESETCONTENT, 0, 0);
                    for (auto& rf : g_radioFavs) {
                        RadioStation rs = {};
                        wcsncpy_s(rs.name, rf.name.c_str(), _TRUNCATE);
                        wcsncpy_s(rs.url, rf.url.c_str(), _TRUNCATE);
                        wcsncpy_s(rs.uuid, rf.uuid.c_str(), _TRUNCATE);
                        _snwprintf_s(rs.display, _countof(rs.display), _TRUNCATE, L"\u2605 %s", rs.name);
                        g_radioResults.push_back(rs);
                        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)rs.display);
                    }
                    SetDlgItemText(hwnd, ID_RADIO_STATUS, L"Showing saved favorites.");
                } else if (pi == 1) {
                    // Top stations
                    g_radioShowingFavs = false;
                    SetDlgItemText(hwnd, ID_RADIO_QUERY, L"");
                    SetDlgItemText(hwnd, ID_RADIO_GENRE, L"");
                    SendMessage(hwnd, WM_COMMAND, ID_RADIO_SEARCH, 0);
                } else if (pi < 18) {
                    g_radioShowingFavs = false;
                    SetDlgItemText(hwnd, ID_RADIO_QUERY, L"");
                    SetDlgItemText(hwnd, ID_RADIO_GENRE, presetTags[pi]);
                    SendMessage(hwnd, WM_COMMAND, ID_RADIO_SEARCH, 0);
                }
                break;
            }
        }
        switch (LOWORD(wParam)) {
        case ID_RADIO_SEARCH: {
            g_radioShowingFavs = false;
            wchar_t query[256] = {}, genre[128] = {};
            GetDlgItemText(hwnd, ID_RADIO_QUERY, query, 256);
            GetDlgItemText(hwnd, ID_RADIO_GENRE, genre, 128);
            RadioSearch(query, genre, GetDlgItem(hwnd, ID_RADIO_STATUS));
            HWND hList = GetDlgItem(hwnd, ID_RADIO_LIST);
            SendMessage(hList, LB_RESETCONTENT, 0, 0);
            for (auto& rs : g_radioResults)
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)rs.display);
            if (!g_radioResults.empty())
                SendMessage(hList, LB_SETCURSEL, 0, 0);
            break;
        }
        case ID_RADIO_LIST:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int sel = (int)SendMessage(GetDlgItem(hwnd, ID_RADIO_LIST), LB_GETCURSEL, 0, 0);
                if (sel >= 0) RadioPlayStation(sel);
            }
            break;
        case ID_RADIO_PLAY: {
            int sel = (int)SendMessage(GetDlgItem(hwnd, ID_RADIO_LIST), LB_GETCURSEL, 0, 0);
            if (sel >= 0) RadioPlayStation(sel);
            else SetDlgItemText(hwnd, ID_RADIO_STATUS, L"Select a station first.");
            break;
        }
        case ID_RADIO_STOP:
            if (g_radioPlaying) {
                StopAudio();
                SetDlgItemText(hwnd, ID_RADIO_STATUS, L"Stopped.");
            }
            break;
        case 499: // Credit link click
            ShellExecute(hwnd, L"open", L"https://www.radio-browser.info/", NULL, NULL, SW_SHOW);
            break;
        case IDC_RADIO_FAV: {
            // Add selected station to radio favorites
            HWND hList = GetDlgItem(hwnd, ID_RADIO_LIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < (int)g_radioResults.size()) {
                RadioStation& rs = g_radioResults[sel];
                if (!IsRadioFav(rs.url)) {
                    RadioFav rf; rf.name = rs.name; rf.url = rs.url; rf.uuid = rs.uuid;
                    g_radioFavs.push_back(rf);
                    SaveRadioFavs();
                    // Update display to show star
                    _snwprintf_s(rs.display, _countof(rs.display), _TRUNCATE,
                        L"\u2605 %s  |  %s  %dkbps  [%s]", rs.name, rs.codec, rs.bitrate, rs.country);
                    SendMessage(hList, LB_DELETESTRING, sel, 0);
                    SendMessage(hList, LB_INSERTSTRING, sel, (LPARAM)rs.display);
                    SendMessage(hList, LB_SETCURSEL, sel, 0);
                    SetDlgItemText(hwnd, ID_RADIO_STATUS, L"\u2605 Added to favorites.");
                }
            }
            break;
        }
        case IDC_RADIO_UNFAV: {
            HWND hList = GetDlgItem(hwnd, ID_RADIO_LIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < (int)g_radioResults.size()) {
                RadioStation& rs = g_radioResults[sel];
                for (auto it = g_radioFavs.begin(); it != g_radioFavs.end(); ++it) {
                    if (it->url == rs.url) { g_radioFavs.erase(it); SaveRadioFavs(); break; }
                }
                if (g_radioShowingFavs) {
                    // In favorites view: remove entry entirely
                    SendMessage(hList, LB_DELETESTRING, sel, 0);
                    g_radioResults.erase(g_radioResults.begin() + sel);
                    int count = (int)g_radioResults.size();
                    if (count > 0) SendMessage(hList, LB_SETCURSEL, min(sel, count - 1), 0);
                } else {
                    // In search view: just remove star from display
                    _snwprintf_s(rs.display, _countof(rs.display), _TRUNCATE,
                        L"%s  |  %s  %dkbps  [%s]", rs.name, rs.codec, rs.bitrate, rs.country);
                    SendMessage(hList, LB_DELETESTRING, sel, 0);
                    SendMessage(hList, LB_INSERTSTRING, sel, (LPARAM)rs.display);
                    SendMessage(hList, LB_SETCURSEL, sel, 0);
                }
                SetDlgItemText(hwnd, ID_RADIO_STATUS, L"Removed from favorites.");
            }
            break;
        }
        case ID_RADIO_CLOSE:
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        }
        break;
    case WM_TIMER:
        if (wParam == IDT_RADIO_DEBOUNCE) {
            KillTimer(hwnd, IDT_RADIO_DEBOUNCE);
            // Auto-search after 1s typing pause
            SendMessage(hwnd, WM_COMMAND, ID_RADIO_SEARCH, 0);
        }
        break;
    case WM_NOTIFY: break; // placeholder
    case WM_CONTEXTMENU: {
        // Right-click on results list
        HWND hList = GetDlgItem(hwnd, ID_RADIO_LIST);
        if ((HWND)wParam == hList) {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            // Convert to client coords for hit test
            POINT cpt = pt; ScreenToClient(hList, &cpt);
            int idx = (int)SendMessage(hList, LB_ITEMFROMPOINT, 0, MAKELPARAM(cpt.x, cpt.y));
            if (!HIWORD(idx) && LOWORD(idx) < (UINT)g_radioResults.size()) {
                SendMessage(hList, LB_SETCURSEL, LOWORD(idx), 0);
                HMENU hMenu = CreatePopupMenu();
                bool isFav = IsRadioFav(g_radioResults[LOWORD(idx)].url);
                AppendMenu(hMenu, MF_STRING, ID_RADIO_PLAY, L"\u25B6 Play");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                if (isFav)
                    AppendMenu(hMenu, MF_STRING, IDC_RADIO_UNFAV, L"Remove from Favorites");
                else
                    AppendMenu(hMenu, MF_STRING, IDC_RADIO_FAV, L"\u2605 Save as Favorite");
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
        }
        return 0;
    }
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_DESTROY:
        // Don't stop radio on dialog close — keep playing
        g_hwndRadio = NULL;
        g_radioResults.clear();
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenRadioDialog()
{
    if (g_hwndRadio && IsWindow(g_hwndRadio)) { SetForegroundWindow(g_hwndRadio); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t RADIO_CLASS[] = L"BillyRadioWnd";
    static bool radioReg = false;
    if (!radioReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = RadioWndProc; wc.hInstance = hInst;
        wc.lpszClassName = RADIO_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); radioReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 560, 460 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndRadio = CreateWindowEx(WS_EX_APPWINDOW, RADIO_CLASS,
        L"Internet Radio", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndRadio, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
}

// ============================================================
//  Sets.LIVE integration — browse & stream a remote liveset
//  library through the PHP API (see setslive-server/).
// ============================================================
static HWND g_hwndSets = NULL;
struct SetsEntry { bool isDir; std::wstring name; std::wstring id; long long size; };
static std::vector<SetsEntry> g_setsEntries;
static std::wstring g_setsCurPath;   // current relative path ("" = root)
static std::wstring g_setsErrDetail; // first bytes of the server body on error

// Strip an optional scheme from the configured server, leaving the host.
static void SetsHost(wchar_t* out, int n)
{
    const wchar_t* p = g_setsServer;
    if (_wcsnicmp(p, L"https://", 8) == 0) p += 8;
    else if (_wcsnicmp(p, L"http://", 7) == 0) p += 7;
    wcsncpy_s(out, n, p, _TRUNCATE);
    // trim trailing slash
    int len = (int)wcslen(out);
    while (len > 0 && out[len-1] == L'/') out[--len] = 0;
}

// HTTPS GET to the Sets.LIVE API with the bearer token. Returns the body;
// *status receives the HTTP status code (0 on connection failure).
static std::string SetsHttpGet(const wchar_t* pathQuery, int* status)
{
    if (status) *status = 0;
    std::string result;
    wchar_t host[128]; SetsHost(host, 128);
    if (!host[0]) return result;
    HINTERNET hSession = WinHttpOpen(L"BillyPro/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return result;
    WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 15000);
    HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return result; }
    HINTERNET hReq = WinHttpOpenRequest(hConnect, L"GET", pathQuery, NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hReq) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }
    wchar_t auth[220];
    swprintf_s(auth, L"Authorization: Bearer %s\r\n", g_setsToken);
    WinHttpAddRequestHeaders(hReq, auth, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    if (WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hReq, NULL)) {
        DWORD code = 0, sz = sizeof(code);
        WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &code, &sz, WINHTTP_NO_HEADER_INDEX);
        if (status) *status = (int)code;
        DWORD avail = 0;
        while (WinHttpQueryDataAvailable(hReq, &avail) && avail > 0) {
            std::string chunk(avail, '\0');
            DWORD read = 0;
            WinHttpReadData(hReq, &chunk[0], avail, &read);
            result.append(chunk, 0, read);
        }
    }
    WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
    return result;
}

// Validate the current token via whoami.php. Returns HTTP status and, on
// success, fills g_setsExpiry + optional user name.
static int SetsValidateToken(wchar_t* outUser, int userN)
{
    if (outUser && userN) outUser[0] = 0;
    int status = 0;
    std::string resp = SetsHttpGet(L"/api/whoami.php", &status);
    if (status == 200) {
        std::wstring exp = JsonGet(resp.c_str(), "expires");
        if (!exp.empty()) wcsncpy_s(g_setsExpiry, exp.c_str(), _TRUNCATE);
        std::wstring u = JsonGet(resp.c_str(), "user");
        if (outUser && userN && !u.empty()) wcsncpy_s(outUser, userN, u.c_str(), _TRUNCATE);
    }
    return status;
}

// Build the stream URL for a file id (relative path), token in the query.
static std::wstring SetsStreamUrl(const std::wstring& id)
{
    wchar_t host[128]; SetsHost(host, 128);
    std::string enc = UrlEncode(id.c_str());
    wchar_t wenc[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, enc.c_str(), -1, wenc, 1024);
    wchar_t url[1400];
    swprintf_s(url, L"https://%s/api/stream.php?id=%s&token=%s", host, wenc, g_setsToken);
    return url;
}

// Fetch and parse a folder listing. Returns HTTP status (200 = ok).
static int SetsLoadPath(const wchar_t* relPath)
{
    std::string enc = UrlEncode(relPath ? relPath : L"");
    wchar_t wenc[1024] = {};
    MultiByteToWideChar(CP_UTF8, 0, enc.c_str(), -1, wenc, 1024);
    wchar_t pq[1100];
    swprintf_s(pq, L"/api/list.php?path=%s", wenc);
    int status = 0;
    std::string resp = SetsHttpGet(pq, &status);
    if (status != 200) {
        // Keep a snippet of the body so the UI can show WHY it failed
        // (nginx HTML 404 vs. a PHP JSON error tells us where to look).
        g_setsErrDetail.clear();
        if (!resp.empty()) {
            std::string s = resp.substr(0, 200);
            int wl = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
            if (wl > 1) { std::wstring w(wl, 0); MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], wl); g_setsErrDetail = w.c_str(); }
        }
        return status;
    }

    g_setsEntries.clear();
    g_setsCurPath = relPath ? relPath : L"";
    // Update token expiry from the response
    std::wstring exp = JsonGet(resp.c_str(), "expires");
    if (!exp.empty()) wcsncpy_s(g_setsExpiry, exp.c_str(), _TRUNCATE);
    // Parse the "entries" array of {type,name,id,size}
    const char* arr = strstr(resp.c_str(), "\"entries\"");
    if (arr) arr = strchr(arr, '[');
    if (arr) {
        const char* p = arr + 1;
        while (true) {
            const char* obj = strchr(p, '{');
            if (!obj) break;
            int depth = 1; const char* q = obj + 1;
            while (*q && depth > 0) { if (*q == '{') depth++; else if (*q == '}') depth--; q++; }
            if (depth != 0) break;
            std::string o(obj, q - obj);
            SetsEntry e = {};
            std::wstring type = JsonGet(o.c_str(), "type");
            e.isDir = (type == L"dir");
            e.name = JsonGet(o.c_str(), "name");
            e.id = JsonGet(o.c_str(), "id");
            std::wstring sz = JsonGet(o.c_str(), "size");
            e.size = sz.empty() ? 0 : _wtoi64(sz.c_str());
            if (!e.name.empty()) g_setsEntries.push_back(e);
            p = q;
            // stop at end of array
            const char* nextObj = strchr(q, '{');
            const char* endArr = strchr(q, ']');
            if (!nextObj || (endArr && endArr < nextObj)) break;
        }
    }
    return 200;
}

static void SetsFillList(HWND hList)
{
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    // Back entry (unless at root)
    if (!g_setsCurPath.empty())
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)L"[..]  (back)");
    for (auto& e : g_setsEntries) {
        wchar_t line[600];
        if (e.isDir)
            swprintf_s(line, L"[+]  %s", e.name.c_str());
        else if (e.size > 1024 * 1024)
            swprintf_s(line, L"     %s   (%.1f MB)", e.name.c_str(), e.size / 1048576.0);
        else
            swprintf_s(line, L"     %s", e.name.c_str());
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)line);
    }
}

// Map a listbox row to a g_setsEntries index (accounting for the back row)
static int SetsEntryIndex(int listRow)
{
    if (!g_setsCurPath.empty()) {
        if (listRow == 0) return -2;   // back
        return listRow - 1;
    }
    return listRow;
}

static void SetsRefresh(HWND hwnd)
{
    HWND hList = GetDlgItem(hwnd, ID_SETS_LIST);
    SetDlgItemText(hwnd, ID_SETS_STATUS, L"Loading...");
    int st = SetsLoadPath(g_setsCurPath.c_str());
    if (st == 200) {
        SetsFillList(hList);
        SetDlgItemText(hwnd, ID_SETS_PATH, g_setsCurPath.empty() ? L"/" : g_setsCurPath.c_str());
        wchar_t s[128];
        swprintf_s(s, L"%d item(s)   ·   token valid until %s",
            (int)g_setsEntries.size(), g_setsExpiry[0] ? g_setsExpiry : L"?");
        SetDlgItemText(hwnd, ID_SETS_STATUS, s);
    } else if (st == 401) {
        SetDlgItemText(hwnd, ID_SETS_STATUS, L"Invalid or expired token. Update it in Options → Sets.LIVE.");
    } else if (st == 429) {
        SetDlgItemText(hwnd, ID_SETS_STATUS, L"Rate limited — please wait a few minutes and try again.");
    } else if (st == 0) {
        SetDlgItemText(hwnd, ID_SETS_STATUS, L"Cannot reach the server. Check your connection / server address.");
    } else {
        // Show the server's own message so 404s are diagnosable:
        //  JSON like {"error":"Folder not found"} = PHP ran (fix media paths)
        //  HTML "<html>...404..." = nginx didn't route /api/ (fix nginx)
        wchar_t s[320];
        swprintf_s(s, L"HTTP %d — %s", st,
            g_setsErrDetail.empty() ? L"(no response body)" : g_setsErrDetail.c_str());
        SetDlgItemText(hwnd, ID_SETS_STATUS, s);
    }
}

static void SetsNavigate(HWND hwnd, int listRow)
{
    int ei = SetsEntryIndex(listRow);
    if (ei == -2) {
        // Go up one level
        std::wstring p = g_setsCurPath;
        size_t sl = p.find_last_of(L'/');
        g_setsCurPath = (sl == std::wstring::npos) ? L"" : p.substr(0, sl);
        SetsRefresh(hwnd);
        return;
    }
    if (ei < 0 || ei >= (int)g_setsEntries.size()) return;
    SetsEntry& e = g_setsEntries[ei];
    if (e.isDir) {
        // Dirs carry only a name; build the new relative path from it
        g_setsCurPath = g_setsCurPath.empty() ? e.name : (g_setsCurPath + L"/" + e.name);
        SetsRefresh(hwnd);
    } else {
        // Build a playlist from all files in this folder, play the clicked one
        ClearPlaylist();
        int playIdx = -1;
        for (auto& fe : g_setsEntries) {
            if (fe.isDir) continue;
            Track t = {};
            std::wstring url = SetsStreamUrl(fe.id);
            wcsncpy_s(t.path, url.c_str(), _TRUNCATE);
            wcsncpy_s(t.display, fe.name.c_str(), _TRUNCATE);
            if (fe.id == e.id) playIdx = (int)g_playlist.size();
            g_playlist.push_back(t);
        }
        RebuildShuffleOrder();
        if (hListBox) {
            SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
            for (auto& t : g_playlist)
                SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
        }
        // Remember this folder so the main window's Back button can return here
        g_setsReturnActive = true;
        g_setsReturnPath = g_setsCurPath;
        if (g_hwnd) LayoutControls(g_hwnd);
        if (playIdx >= 0) PlayIndex(playIdx);
        SetDlgItemText(hwnd, ID_SETS_STATUS, L"Streaming — you can keep browsing.");
    }
}

// --- Download a Sets.LIVE file to disk (worker thread) ---
#define WM_SETS_DLPROGRESS (WM_APP + 30)   // wParam = percent (0..100)
#define WM_SETS_DLDONE     (WM_APP + 31)   // wParam: 1=ok 0=fail
struct SetsDlData { std::wstring url; std::wstring dest; HWND hwnd; };
static volatile bool g_setsDlActive = false;
static wchar_t g_setsDlName[MAX_PATH] = L"";   // display name of the current download

static DWORD WINAPI SetsDlThread(LPVOID param)
{
    SetsDlData* d = (SetsDlData*)param;
    bool ok = false;
    // Split URL -> host + path
    const wchar_t* p = d->url.c_str();
    if (_wcsnicmp(p, L"https://", 8) == 0) p += 8;
    else if (_wcsnicmp(p, L"http://", 7) == 0) p += 7;
    const wchar_t* slash = wcschr(p, L'/');
    std::wstring host(p, slash ? slash - p : wcslen(p));
    std::wstring path(slash ? slash : L"/");

    HINTERNET hS = WinHttpOpen(L"BillyPro/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hS) {
        WinHttpSetTimeouts(hS, 15000, 15000, 15000, 30000);
        HINTERNET hC = WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (hC) {
            HINTERNET hR = WinHttpOpenRequest(hC, L"GET", path.c_str(), NULL,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            if (hR && WinHttpSendRequest(hR, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hR, NULL)) {
                DWORD code = 0, cs = sizeof(code);
                WinHttpQueryHeaders(hR, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                    WINHTTP_HEADER_NAME_BY_INDEX, &code, &cs, WINHTTP_NO_HEADER_INDEX);
                ULONGLONG total = 0;
                DWORD cl = 0, cls = sizeof(cl);
                if (WinHttpQueryHeaders(hR, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &cl, &cls, WINHTTP_NO_HEADER_INDEX))
                    total = cl;
                if (code == 200) {
                    HANDLE hf = CreateFile(d->dest.c_str(), GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hf != INVALID_HANDLE_VALUE) {
                        std::vector<char> buf(65536);
                        ULONGLONG got = 0; DWORD avail = 0; int lastPct = -1;
                        ok = true;
                        while (WinHttpQueryDataAvailable(hR, &avail) && avail > 0) {
                            if (avail > buf.size()) buf.resize(avail);
                            DWORD read = 0;
                            if (!WinHttpReadData(hR, buf.data(), avail, &read) || read == 0) break;
                            DWORD wr = 0;
                            if (!WriteFile(hf, buf.data(), read, &wr, NULL) || wr != read) { ok = false; break; }
                            got += read;
                            if (total > 0) {
                                int pct = (int)(got * 100 / total);
                                if (pct != lastPct) { lastPct = pct;
                                    PostMessage(d->hwnd, WM_SETS_DLPROGRESS, (WPARAM)pct, 0); }
                            }
                        }
                        CloseHandle(hf);
                        if (!ok) DeleteFile(d->dest.c_str());
                    }
                }
            }
            if (hR) WinHttpCloseHandle(hR);
            WinHttpCloseHandle(hC);
        }
        WinHttpCloseHandle(hS);
    }
    if (IsWindow(d->hwnd)) PostMessage(d->hwnd, WM_SETS_DLDONE, ok ? 1 : 0, 0);
    g_setsDlActive = false;
    delete d;
    return 0;
}

// Show a Sets download status line on the right window (Sets dialog uses its
// own status label; the main window uses the status bar).
static void SetsDlStatus(HWND target, const wchar_t* text)
{
    if (target == g_hwnd) { if (hStatus) SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)text); }
    else SetDlgItemText(target, ID_SETS_STATUS, text);
}

// Download a Sets.LIVE stream URL to the configured folder (asks once).
// Progress/completion are posted to `target` — the Sets dialog OR the main
// window (so it works from the playlist right-click too).
static void SetsDownloadUrl(HWND target, const wchar_t* streamUrl, const wchar_t* nameIn)
{
    if (g_setsDlActive) { SetsDlStatus(target, L"A Sets.LIVE download is already in progress."); return; }
    // Strip any ★ / ▶ prefix from the display name
    const wchar_t* name = nameIn;
    if (name && (name[0] == L'★' || name[0] == L'▶') && name[1] == L' ') name += 2;
    if (!name || !name[0]) name = L"download.mp3";
    // Ask for a folder the first time; remember it in the INI afterwards
    if (!g_setsDownloadDir[0] || GetFileAttributesW(g_setsDownloadDir) == INVALID_FILE_ATTRIBUTES) {
        wchar_t folder[MAX_PATH] = {};
        if (!PickFolder(target, L"Choose a download folder for Sets.LIVE", folder, MAX_PATH))
            return;
        wcsncpy_s(g_setsDownloadDir, folder, _TRUNCATE);
        SaveSettings();
    }
    // _snwprintf_s(_TRUNCATE) never aborts on overflow (plain swprintf_s does);
    // dest is oversized so a long folder+name won't be clipped in practice.
    wchar_t dest[1024];
    _snwprintf_s(dest, _countof(dest), _TRUNCATE, L"%s\\%s", g_setsDownloadDir, name);
    if (GetFileAttributesW(dest) != INVALID_FILE_ATTRIBUTES) {
        wchar_t base[MAX_PATH]; wcsncpy_s(base, name, _TRUNCATE);
        wchar_t* dot = wcsrchr(base, L'.'); std::wstring ext;
        if (dot) { ext = dot; *dot = 0; }
        for (int i = 1; i <= 99; i++) {
            _snwprintf_s(dest, _countof(dest), _TRUNCATE, L"%s\\%s_%02d%s", g_setsDownloadDir, base, i, ext.c_str());
            if (GetFileAttributesW(dest) == INVALID_FILE_ATTRIBUTES) break;
        }
    }
    SetsDlData* d = new SetsDlData();
    d->url = streamUrl;
    d->dest = dest;
    d->hwnd = target;
    wcsncpy_s(g_setsDlName, name, _TRUNCATE);
    g_setsDlActive = true;
    wchar_t s[400];
    _snwprintf_s(s, _countof(s), _TRUNCATE, L"Downloading “%s” ... 0%%", g_setsDlName);
    SetsDlStatus(target, s);
    HANDLE th = CreateThread(NULL, 0, SetsDlThread, d, 0, NULL);
    if (th) {
        CloseHandle(th);
    } else {
        // Thread never started — free the payload and clear the busy flag,
        // otherwise g_setsDlActive stays true forever and blocks all downloads.
        delete d;
        g_setsDlActive = false;
        SetsDlStatus(target, L"✗ Could not start the download (out of resources).");
    }
}

static void SetsStartDownload(HWND hwnd, const SetsEntry& e)
{
    SetsDownloadUrl(hwnd, SetsStreamUrl(e.id).c_str(), e.name.c_str());
}

static LRESULT CALLBACK SetsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        int cw = 560, y = 8;
        HWND hBack = CreateWindow(L"BUTTON", L"◄ Back",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 8, y, 70, 24, hwnd, (HMENU)ID_SETS_BACK, hInst, NULL);
        SendMessage(hBack, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hPath = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"/",
            WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL,
            84, y + 1, cw - 84 - 82, 22, hwnd, (HMENU)ID_SETS_PATH, hInst, NULL);
        SendMessage(hPath, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hRef = CreateWindow(L"BUTTON", L"Refresh",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cw - 78, y, 70, 24, hwnd, (HMENU)ID_SETS_REFRESH, hInst, NULL);
        SendMessage(hRef, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 32;
        HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS,
            8, y, cw - 16, 300, hwnd, (HMENU)ID_SETS_LIST, hInst, NULL);
        SendMessage(hList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 308;
        HWND hStat = CreateWindow(L"STATIC", L"Loading...",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX | SS_PATHELLIPSIS, 8, y, cw - 16, 16, hwnd, (HMENU)ID_SETS_STATUS, hInst, NULL);
        SendMessage(hStat, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        y += 22;
        HWND hPlay = CreateWindow(L"BUTTON", L"Play",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 8, y, 80, 26, hwnd, (HMENU)ID_SETS_PLAY, hInst, NULL);
        SendMessage(hPlay, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hClo = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cw - 78, y, 70, 26, hwnd, (HMENU)ID_SETS_CLOSE, hInst, NULL);
        SendMessage(hClo, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        // g_setsCurPath is set by the opener (root, or a folder to return to)
        SetsRefresh(hwnd);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_SETS_LIST:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int sel = (int)SendMessage(GetDlgItem(hwnd, ID_SETS_LIST), LB_GETCURSEL, 0, 0);
                if (sel >= 0) SetsNavigate(hwnd, sel);
            }
            break;
        case ID_SETS_PLAY: {
            int sel = (int)SendMessage(GetDlgItem(hwnd, ID_SETS_LIST), LB_GETCURSEL, 0, 0);
            if (sel >= 0) SetsNavigate(hwnd, sel);
            break;
        }
        case ID_SETS_BACK:
            if (!g_setsCurPath.empty()) {
                size_t sl = g_setsCurPath.find_last_of(L'/');
                g_setsCurPath = (sl == std::wstring::npos) ? L"" : g_setsCurPath.substr(0, sl);
                SetsRefresh(hwnd);
            }
            break;
        case ID_SETS_REFRESH:
            SetsRefresh(hwnd);
            break;
        case ID_SETS_CLOSE:
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        }
        break;
    case WM_CONTEXTMENU: {
        HWND hList = GetDlgItem(hwnd, ID_SETS_LIST);
        if ((HWND)wParam != hList) break;
        int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
        int ei = SetsEntryIndex(sel);
        if (ei < 0 || ei >= (int)g_setsEntries.size()) break;
        SetsEntry& e = g_setsEntries[ei];
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (pt.x == -1 && pt.y == -1) { RECT r; GetWindowRect(hList, &r); pt.x = r.left + 20; pt.y = r.top + 20; }
        HMENU hm = CreatePopupMenu();
        AppendMenu(hm, MF_STRING, 1, e.isDir ? L"Open" : L"Play");
        if (!e.isDir) AppendMenu(hm, MF_STRING, 2, L"Download...");
        int cmd = TrackPopupMenu(hm, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hm);
        if (cmd == 1) SetsNavigate(hwnd, sel);
        else if (cmd == 2) SetsStartDownload(hwnd, e);
        return 0;
    }
    case WM_SETS_DLPROGRESS: {
        wchar_t s[300]; swprintf_s(s, L"Downloading “%s” ... %d%%", g_setsDlName, (int)wParam);
        SetDlgItemText(hwnd, ID_SETS_STATUS, s);
        return 0;
    }
    case WM_SETS_DLDONE: {
        wchar_t s[400];
        if (wParam) swprintf_s(s, L"✓ Downloaded “%s” to %s", g_setsDlName, g_setsDownloadDir);
        else        swprintf_s(s, L"✗ Download failed. Check the folder / connection.");
        SetDlgItemText(hwnd, ID_SETS_STATUS, s);
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
        break;
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_DESTROY:
        g_hwndSets = NULL;
        g_setsEntries.clear();
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenSetsLiveDialog(const wchar_t* startPath = L"")
{
    if (!g_setsToken[0]) {
        if (MessageBox(g_hwnd,
            L"No Sets.LIVE access token set.\r\n\r\n"
            L"Register on the website to get a token, then paste it in\r\n"
            L"Options → Sets.LIVE. Open the registration page now?",
            L"Sets.LIVE", MB_YESNO | MB_ICONINFORMATION) == IDYES) {
            wchar_t host[128]; SetsHost(host, 128);
            wchar_t url[256]; swprintf_s(url, L"https://%s/register.php", host);
            ShellExecute(g_hwnd, L"open", url, NULL, NULL, SW_SHOW);
        }
        return;
    }
    g_setsCurPath = startPath ? startPath : L"";   // WM_CREATE opens at this path
    if (g_hwndSets && IsWindow(g_hwndSets)) {
        SetForegroundWindow(g_hwndSets);
        SetsRefresh(g_hwndSets);
        return;
    }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t SETS_CLASS[] = L"BillySetsWnd";
    static bool setsReg = false;
    if (!setsReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = SetsWndProc; wc.hInstance = hInst;
        wc.lpszClassName = SETS_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); setsReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 560, 430 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndSets = CreateWindowEx(WS_EX_APPWINDOW, SETS_CLASS,
        L"Sets.LIVE", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndSets, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
}

// Add/remove the "Sets.LIVE" item in the Library menu to match the option
static void UpdateSetsLibMenu()
{
    if (!g_hLibMenu) return;
    bool present = (GetMenuState(g_hLibMenu, IDM_LIB_SETSLIVE, MF_BYCOMMAND) != (UINT)-1);
    if (g_setsEnabled && !present) {
        int n = GetMenuItemCount(g_hLibMenu), radioPos = -1;
        for (int i = 0; i < n; i++)
            if (GetMenuItemID(g_hLibMenu, i) == IDM_LIB_RADIO) { radioPos = i; break; }
        InsertMenu(g_hLibMenu, radioPos >= 0 ? radioPos + 1 : (UINT)-1,
            MF_BYPOSITION | MF_STRING, IDM_LIB_SETSLIVE, L"Sets.LIVE");
        if (g_darkMode) MenuInitOwnerDraw(g_hLibMenu);
    } else if (!g_setsEnabled && present) {
        DeleteMenu(g_hLibMenu, IDM_LIB_SETSLIVE, MF_BYCOMMAND);
    }
}

// ============================================================
//  Open Stream Link dialog — paste a http/https/ftp stream URL
// ============================================================
static HWND g_hwndStream = NULL;
static WNDPROC g_OldStreamEditProc = NULL;

static LRESULT CALLBACK StreamEditSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, ID_STREAM_OK, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProc(g_OldStreamEditProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK StreamDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        HWND hLbl = CreateWindow(L"STATIC", L"Stream URL (http / https / ftp):",
            WS_CHILD | WS_VISIBLE, 8, 8, 384, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hEd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
            8, 28, 384, 22, hwnd, (HMENU)ID_STREAM_EDIT, hInst, NULL);
        SendMessage(hEd, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        g_OldStreamEditProc = (WNDPROC)SetWindowLongPtr(hEd, GWLP_WNDPROC, (LONG_PTR)StreamEditSubProc);
        HWND hOk = CreateWindow(L"BUTTON", L"Play",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
            232, 58, 76, 24, hwnd, (HMENU)ID_STREAM_OK, hInst, NULL);
        SendMessage(hOk, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hCa = CreateWindow(L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            316, 58, 76, 24, hwnd, (HMENU)ID_STREAM_CANCEL, hInst, NULL);
        SendMessage(hCa, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:          // Enter key routes here via IsDialogMessage (no registered default id)
        case ID_STREAM_OK: {
            wchar_t url[1024] = {};
            GetDlgItemText(hwnd, ID_STREAM_EDIT, url, _countof(url));
            // Trim surrounding whitespace
            wchar_t* s = url;
            while (*s == L' ' || *s == L'\t') s++;
            size_t len = wcslen(s);
            while (len > 0 && (s[len-1] == L' ' || s[len-1] == L'\t' || s[len-1] == L'\r' || s[len-1] == L'\n')) s[--len] = 0;
            if (!wcsstr(s, L"://")) {
                MessageBox(hwnd, L"Enter a valid stream URL, e.g.\nhttps://server.example.com/stream",
                    L"Open Stream Link", MB_ICONWARNING);
                break;
            }
            // If a library view (Favorites / playlist / history) is showing,
            // leave it first — otherwise the stream would be appended into the
            // view's temporary playlist, pollute it, and vanish when the view
            // closes. The stream belongs to the real playlist.
            LibRestorePlaylist();
            Track t = {};
            wcsncpy_s(t.path, s, _TRUNCATE);
            UrlToDisplayName(s, t.display, _countof(t.display));
            g_playlist.push_back(t);
            if (hListBox && !g_browserActive)
                SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)t.display);
            RebuildShuffleOrder();
            int idx = (int)g_playlist.size() - 1;
            DestroyWindow(hwnd);
            PlayIndex(idx);
            break;
        }
        case ID_STREAM_CANCEL:
        case IDCANCEL:
            DestroyWindow(hwnd);
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
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_KEYDOWN: if (wParam == VK_ESCAPE) DestroyWindow(hwnd); break;
    case WM_DESTROY: g_hwndStream = NULL; break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenStreamLinkDialog()
{
    if (g_hwndStream && IsWindow(g_hwndStream)) { SetForegroundWindow(g_hwndStream); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t STREAM_CLASS[] = L"BillyStreamDlg";
    static bool streamReg = false;
    if (!streamReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = StreamDlgProc; wc.hInstance = hInst;
        wc.lpszClassName = STREAM_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); streamReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 400, 92 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndStream = CreateWindowEx(WS_EX_APPWINDOW, STREAM_CLASS,
        L"Open Stream Link", style, 0, 0, dw, dh, g_hwnd, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndStream, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
    SetFocus(GetDlgItem(g_hwndStream, ID_STREAM_EDIT));
}

// ============================================================
//  New Playlist name prompt — asks ONLY for the name and creates
//  <name>.bpp in the playlists folder, pre-filled with the tracks
//  captured in s_newPlTracks. No file dialog.
// ============================================================
static HWND g_hwndNamePrompt = NULL;
static WNDPROC g_OldNameEditProc = NULL;
// Rename mode: >= 0 while the prompt renames g_bppPlaylists[idx] IN PLACE
// (renames the .bpp file itself) instead of creating a new playlist.
static int          s_renamePlIdx = -1;
static std::wstring s_renamePlPath;   // original .bpp path (guards stale index)
static std::wstring s_renamePlName;   // original display name

static LRESULT CALLBACK NameEditSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, ID_NAME_OK, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProc(g_OldNameEditProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK NamePromptProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        wchar_t info[192];
        if (s_renamePlIdx >= 0)
            _snwprintf_s(info, _countof(info), _TRUNCATE, L"Rename \"%s\" to:", s_renamePlName.c_str());
        else if (s_newPlTracks.empty())
            wcscpy_s(info, L"Playlist name:");
        else
            swprintf_s(info, L"Playlist name (%d track%s will be added):",
                (int)s_newPlTracks.size(), s_newPlTracks.size() == 1 ? L"" : L"s");
        HWND hLbl = CreateWindow(L"STATIC", info,
            WS_CHILD | WS_VISIBLE | SS_NOPREFIX, 8, 8, 384, 16, hwnd, (HMENU)IDC_STATIC, hInst, NULL);
        SendMessage(hLbl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hEd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
            8, 28, 384, 22, hwnd, (HMENU)ID_NAME_EDIT, hInst, NULL);
        SendMessage(hEd, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        g_OldNameEditProc = (WNDPROC)SetWindowLongPtr(hEd, GWLP_WNDPROC, (LONG_PTR)NameEditSubProc);
        if (s_renamePlIdx >= 0) {
            // Pre-fill with the current name, selected for quick overtype
            SetWindowText(hEd, s_renamePlName.c_str());
            SendMessage(hEd, EM_SETSEL, 0, -1);
        }
        HWND hOk = CreateWindow(L"BUTTON", s_renamePlIdx >= 0 ? L"Rename" : L"Create",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
            232, 58, 76, 24, hwnd, (HMENU)ID_NAME_OK, hInst, NULL);
        SendMessage(hOk, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hCa = CreateWindow(L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            316, 58, 76, 24, hwnd, (HMENU)ID_NAME_CANCEL, hInst, NULL);
        SendMessage(hCa, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:          // Enter key routes here via IsDialogMessage
        case ID_NAME_OK: {
            wchar_t name[128] = {};
            GetDlgItemText(hwnd, ID_NAME_EDIT, name, _countof(name));
            // Trim whitespace, sanitize illegal filename characters
            wchar_t* s = name;
            while (*s == L' ' || *s == L'\t') s++;
            size_t len = wcslen(s);
            while (len > 0 && (s[len-1] == L' ' || s[len-1] == L'\t')) s[--len] = 0;
            if (!*s) {
                MessageBox(hwnd, L"Enter a playlist name.", L"New Playlist", MB_ICONWARNING);
                break;
            }
            for (wchar_t* p = s; *p; p++)
                if (*p==L'\\'||*p==L'/'||*p==L':'||*p==L'*'||*p==L'?'||*p==L'"'||*p==L'<'||*p==L'>'||*p==L'|') *p = L'_';
            GetLibDir();
            wchar_t path[MAX_PATH];
            swprintf_s(path, L"%s\\%s.bpp", g_plDir, s);

            // ── Rename mode: rename the existing .bpp in place ─────────
            if (s_renamePlIdx >= 0) {
                // Re-validate the index (the list may have changed since the
                // prompt opened) — fall back to a path lookup.
                int idx = s_renamePlIdx;
                if (idx >= (int)g_bppPlaylists.size() ||
                    _wcsicmp(g_bppPlaylists[idx].filePath.c_str(), s_renamePlPath.c_str()) != 0) {
                    idx = -1;
                    for (int i = 0; i < (int)g_bppPlaylists.size(); i++)
                        if (_wcsicmp(g_bppPlaylists[i].filePath.c_str(), s_renamePlPath.c_str()) == 0) { idx = i; break; }
                }
                if (idx < 0 || GetFileAttributesW(s_renamePlPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
                    MessageBox(hwnd, L"The playlist no longer exists.", L"Rename Playlist", MB_ICONERROR);
                    DestroyWindow(hwnd);
                    break;
                }
                if (wcscmp(s, s_renamePlName.c_str()) == 0) { DestroyWindow(hwnd); break; } // unchanged
                // Never silently clobber another playlist via rename
                if (_wcsicmp(path, g_bppPlaylists[idx].filePath.c_str()) != 0 &&
                    GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
                    MessageBox(hwnd, L"A playlist with this name already exists.",
                        L"Rename Playlist", MB_ICONWARNING);
                    break;
                }
                // Move the .bpp and any custom artwork alongside it
                wchar_t oldArt[MAX_PATH];
                bool hadArt = PMgrCustomArtPath(g_bppPlaylists[idx], oldArt, MAX_PATH);
                if (!MoveFileW(g_bppPlaylists[idx].filePath.c_str(), path)) {
                    MessageBox(hwnd, L"Rename failed. Is the playlist file in use?",
                        L"Rename Playlist", MB_ICONERROR);
                    break;
                }
                if (hadArt) {
                    wchar_t newArt[MAX_PATH];
                    wcsncpy_s(newArt, path, _TRUNCATE);
                    wchar_t* d1 = wcsrchr(newArt, L'.'); if (d1) *d1 = 0;
                    const wchar_t* oldExt = wcsrchr(oldArt, L'.');
                    if (oldExt) wcscat_s(newArt, oldExt);
                    MoveFileW(oldArt, newArt);
                }
                // Keep this playlist's position in the custom order
                g_bppPlaylists[idx].name = s;
                g_bppPlaylists[idx].filePath = path;
                SavePlaylistOrder();
                ScanPlaylists();
                if (g_hwndPMgr && IsWindow(g_hwndPMgr)) {
                    PMgrClearArt();
                    PMgrRefresh(g_hwndPMgr);
                }
                if (hStatus) {
                    wchar_t sbmsg[256];
                    _snwprintf_s(sbmsg, _countof(sbmsg), _TRUNCATE,
                        L"Renamed playlist \"%s\" to \"%s\"", s_renamePlName.c_str(), s);
                    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)sbmsg);
                }
                DestroyWindow(hwnd);
                break;
            }

            if (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
                if (MessageBox(hwnd, L"A playlist with this name already exists.\nOverwrite it?",
                        L"New Playlist", MB_YESNO | MB_ICONWARNING) != IDYES)
                    break;
            }
            FILE* f = _wfopen(path, L"w,ccs=UTF-8");
            if (!f) {
                MessageBox(hwnd, L"Could not create the playlist file.", L"New Playlist", MB_ICONERROR);
                break;
            }
            for (auto& t : s_newPlTracks)
                fwprintf(f, L"%s\n", t.c_str());
            fclose(f);
            int added = (int)s_newPlTracks.size();
            s_newPlTracks.clear();
            ScanPlaylists();
            if (g_hwndPMgr && IsWindow(g_hwndPMgr)) {
                PMgrClearArt();
                PMgrRefresh(g_hwndPMgr);
            }
            if (hStatus) {
                wchar_t sbmsg[256];
                swprintf_s(sbmsg, L"Created playlist \"%s\" with %d track(s)", s, added);
                SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)sbmsg);
            }
            DestroyWindow(hwnd);
            break;
        }
        case ID_NAME_CANCEL:
        case IDCANCEL:
            DestroyWindow(hwnd);
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
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_KEYDOWN: if (wParam == VK_ESCAPE) DestroyWindow(hwnd); break;
    case WM_DESTROY:
        g_hwndNamePrompt = NULL;
        s_renamePlIdx = -1; s_renamePlPath.clear(); s_renamePlName.clear();
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void OpenNamePromptDialog(int renamePlIdx)
{
    if (g_hwndNamePrompt && IsWindow(g_hwndNamePrompt)) { SetForegroundWindow(g_hwndNamePrompt); return; }
    // Arm rename mode BEFORE CreateWindowEx — WM_CREATE reads these
    s_renamePlIdx = -1; s_renamePlPath.clear(); s_renamePlName.clear();
    if (renamePlIdx >= 0 && renamePlIdx < (int)g_bppPlaylists.size()) {
        s_renamePlIdx  = renamePlIdx;
        s_renamePlPath = g_bppPlaylists[renamePlIdx].filePath;
        s_renamePlName = g_bppPlaylists[renamePlIdx].name;
    }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t NAME_CLASS[] = L"BillyNameDlg";
    static bool nameReg = false;
    if (!nameReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = NamePromptProc; wc.hInstance = hInst;
        wc.lpszClassName = NAME_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); nameReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 400, 92 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndNamePrompt = CreateWindowEx(WS_EX_APPWINDOW, NAME_CLASS,
        s_renamePlIdx >= 0 ? L"Rename Playlist" : L"New Playlist",
        style, 0, 0, dw, dh, g_hwnd, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndNamePrompt, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
    SetFocus(GetDlgItem(g_hwndNamePrompt, ID_NAME_EDIT));
}

// ============================================================
//  Shazam-style audio recognition (radio / stream links only)
//  Ports the reverse-engineered SongRec/shazamio signature
//  algorithm: 16 kHz mono -> spectrogram peaks -> binary
//  fingerprint -> POST to amp.shazam.com -> Artist / Title.
//  No API key; unofficial endpoint.
// ============================================================

// --- CRC-32 (IEEE 802.3, same as zlib/crc32fast) ---
static uint32_t Crc32(const uint8_t* data, size_t len)
{
    static uint32_t table[256];
    static bool init = false;
    if (!init) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        init = true;
    }
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++)
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

// --- Base64 encode ---
static std::string Base64Encode(const uint8_t* data, size_t len)
{
    static const char* T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    size_t i = 0;
    for (; i + 3 <= len; i += 3) {
        uint32_t v = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out += T[(v >> 18) & 63]; out += T[(v >> 12) & 63];
        out += T[(v >> 6) & 63];  out += T[v & 63];
    }
    if (len - i == 1) {
        uint32_t v = data[i] << 16;
        out += T[(v >> 18) & 63]; out += T[(v >> 12) & 63]; out += "==";
    } else if (len - i == 2) {
        uint32_t v = (data[i] << 16) | (data[i+1] << 8);
        out += T[(v >> 18) & 63]; out += T[(v >> 12) & 63]; out += T[(v >> 6) & 63]; out += "=";
    }
    return out;
}

// --- In-place iterative radix-2 complex FFT (size must be power of 2) ---
static void Fft2048(float* re, float* im, int n)
{
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * 3.14159265358979323846 / len;
        float wr = (float)cos(ang), wi = (float)sin(ang);
        for (int i = 0; i < n; i += len) {
            float cr = 1.0f, ci = 0.0f;
            for (int k = 0; k < len / 2; k++) {
                int a = i + k, b = i + k + len / 2;
                float tr = re[b] * cr - im[b] * ci;
                float ti = re[b] * ci + im[b] * cr;
                re[b] = re[a] - tr; im[b] = im[a] - ti;
                re[a] += tr;        im[a] += ti;
                float ncr = cr * wr - ci * wi;
                ci = cr * wi + ci * wr; cr = ncr;
            }
        }
    }
}

struct ShazPeak { uint32_t pass; uint16_t mag; uint16_t bin; };

// Generate the Shazam binary signature from a 16 kHz mono float buffer.
static std::vector<uint8_t> ShazMakeSignature(const std::vector<float>& mono16k)
{
    // Precomputed Hann window: 0.5*(1-cos(2*pi*(i+1)/2049))
    static float hann[2048];
    static bool hannInit = false;
    if (!hannInit) {
        for (int i = 0; i < 2048; i++)
            hann[i] = (float)(0.5 * (1.0 - cos(2.0 * 3.14159265358979323846 * (i + 1) / 2049.0)));
        hannInit = true;
    }

    // s16 samples
    int total = (int)mono16k.size();
    std::vector<int16_t> s16(total);
    for (int i = 0; i < total; i++) {
        float s = mono16k[i] * 32767.0f;
        if (s > 32767.0f) s = 32767.0f;
        if (s < -32768.0f) s = -32768.0f;
        s16[i] = (int16_t)lrintf(s);
    }

    int16_t ring[2048] = {};
    int ringIdx = 0;
    static float fftOut[256][1025];
    static float spread[256][1025];
    memset(fftOut, 0, sizeof(fftOut));
    memset(spread, 0, sizeof(spread));
    int fftIdx = 0, spreadIdx = 0;
    uint32_t numSpread = 0;

    std::vector<ShazPeak> bands[4];

    float re[2048], im[2048];

    int chunks = total / 128;
    for (int c = 0; c < chunks; c++) {
        // do_fft: copy 128 samples into ring
        memcpy(&ring[ringIdx], &s16[c * 128], 128 * sizeof(int16_t));
        ringIdx = (ringIdx + 128) & 2047;
        // reorder + Hann
        for (int i = 0; i < 2048; i++) {
            re[i] = (float)ring[(i + ringIdx) & 2047] * hann[i];
            im[i] = 0.0f;
        }
        Fft2048(re, im, 2048);
        float* out = fftOut[fftIdx];
        for (int i = 0; i < 1025; i++) {
            float v = (re[i] * re[i] + im[i] * im[i]) / (float)(1 << 17);
            out[i] = v > 0.0000000001f ? v : 0.0000000001f;
        }
        fftIdx = (fftIdx + 1) & 255;

        // do_peak_spreading
        {
            float* src = fftOut[(fftIdx - 1) & 255];
            float* sp = spread[spreadIdx];
            memcpy(sp, src, 1025 * sizeof(float));
            for (int p = 0; p <= 1022; p++) {
                float m = sp[p];
                if (sp[p+1] > m) m = sp[p+1];
                if (sp[p+2] > m) m = sp[p+2];
                sp[p] = m;
            }
            static float spCopy[1025];
            memcpy(spCopy, sp, sizeof(spCopy));
            int formers[3] = { 1, 3, 6 };
            for (int p = 0; p <= 1024; p++) {
                for (int fi = 0; fi < 3; fi++) {
                    float* fo = spread[(spreadIdx - formers[fi]) & 255];
                    if (spCopy[p] > fo[p]) fo[p] = spCopy[p];
                }
            }
            spreadIdx = (spreadIdx + 1) & 255;
        }
        numSpread++;

        // do_peak_recognition
        if (numSpread >= 46) {
            float* fftM46 = fftOut[(fftIdx - 46) & 255];
            float* fftM49 = spread[(spreadIdx - 49) & 255];
            static const int nb[8] = { -10, -7, -4, -3, 1, 2, 5, 8 };
            static const int ob[14] = { -53, -45, 165, 172, 179, 186, 193, 200, 214, 221, 228, 235, 242, 249 };
            for (int bin = 10; bin <= 1014; bin++) {
                if (fftM46[bin] >= 1.0f / 64.0f && fftM46[bin] >= fftM49[bin - 1]) {
                    float maxN = 0.0f;
                    for (int k = 0; k < 8; k++) {
                        float v = fftM49[bin + nb[k]];
                        if (v > maxN) maxN = v;
                    }
                    if (fftM46[bin] > maxN) {
                        float maxO = maxN;
                        for (int k = 0; k < 14; k++) {
                            float* of = spread[(spreadIdx + ob[k]) & 255];
                            if (of[bin - 1] > maxO) maxO = of[bin - 1];
                        }
                        if (fftM46[bin] > maxO) {
                            uint32_t pass = numSpread - 46;
                            auto lm = [](float v) { float l = logf(v); if (l < 1.0f/64.0f) l = 1.0f/64.0f; return l * 1477.3f + 6144.0f; };
                            float pm = lm(fftM46[bin]);
                            float pmb = lm(fftM46[bin - 1]);
                            float pma = lm(fftM46[bin + 1]);
                            float var1 = pm * 2.0f - pmb - pma;
                            float var2 = (pma - pmb) * 32.0f / var1;
                            uint16_t cbin = (uint16_t)((int)(bin * 64) + (int)var2);
                            float freq = cbin * (16000.0f / 2.0f / 1024.0f / 64.0f);
                            int fi = (int)freq;
                            int band = -1;
                            if (fi >= 250 && fi <= 519) band = 0;
                            else if (fi >= 520 && fi <= 1449) band = 1;
                            else if (fi >= 1450 && fi <= 3499) band = 2;
                            else if (fi >= 3500 && fi <= 5500) band = 3;
                            if (band >= 0) {
                                ShazPeak pk = { pass, (uint16_t)pm, cbin };
                                bands[band].push_back(pk);
                            }
                        }
                    }
                }
            }
        }
    }

    // --- Serialize (encode_to_binary) ---
    auto wr32 = [](std::vector<uint8_t>& b, uint32_t v) {
        b.push_back(v & 0xFF); b.push_back((v >> 8) & 0xFF);
        b.push_back((v >> 16) & 0xFF); b.push_back((v >> 24) & 0xFF);
    };
    auto wr16 = [](std::vector<uint8_t>& b, uint16_t v) {
        b.push_back(v & 0xFF); b.push_back((v >> 8) & 0xFF);
    };

    std::vector<uint8_t> buf;
    wr32(buf, 0xcafe2580);        // magic1
    wr32(buf, 0);                 // crc32 (later)
    wr32(buf, 0);                 // size_minus_header (later)
    wr32(buf, 0x94119c00);        // magic2
    wr32(buf, 0); wr32(buf, 0); wr32(buf, 0);  // void1
    wr32(buf, 3u << 27);          // sample rate id 3 = 16000
    wr32(buf, 0); wr32(buf, 0);   // void2
    wr32(buf, (uint32_t)(total + (int)(16000.0f * 0.24f)));  // number_samples + sr*0.24
    wr32(buf, (15u << 19) + 0x40000);  // fixed
    wr32(buf, 0x40000000);
    wr32(buf, 0);                 // size (later)

    for (int band = 0; band < 4; band++) {
        if (bands[band].empty()) continue;
        std::vector<uint8_t> pk;
        uint32_t passNo = 0;
        for (auto& p : bands[band]) {
            if (p.pass - passNo >= 255) {
                pk.push_back(0xff);
                wr32(pk, p.pass);
                passNo = p.pass;
            }
            pk.push_back((uint8_t)(p.pass - passNo));
            wr16(pk, p.mag);
            wr16(pk, p.bin);
            passNo = p.pass;
        }
        wr32(buf, 0x60030040 + band);
        wr32(buf, (uint32_t)pk.size());
        buf.insert(buf.end(), pk.begin(), pk.end());
        for (uint32_t pad = 0; pad < (4 - (pk.size() % 4)) % 4; pad++)
            buf.push_back(0);
    }

    uint32_t bufSize = (uint32_t)buf.size();
    // size_minus_header at offset 8 and 48+4
    uint32_t smh = bufSize - 48;
    buf[8]  = smh & 0xFF; buf[9]  = (smh >> 8) & 0xFF; buf[10] = (smh >> 16) & 0xFF; buf[11] = (smh >> 24) & 0xFF;
    buf[52] = smh & 0xFF; buf[53] = (smh >> 8) & 0xFF; buf[54] = (smh >> 16) & 0xFF; buf[55] = (smh >> 24) & 0xFF;
    // crc32 over bytes [8..end], stored at offset 4
    uint32_t crc = Crc32(buf.data() + 8, buf.size() - 8);
    buf[4] = crc & 0xFF; buf[5] = (crc >> 8) & 0xFF; buf[6] = (crc >> 16) & 0xFF; buf[7] = (crc >> 24) & 0xFF;

    return buf;
}

// --- POST the signature to Shazam, return raw JSON response ---
static std::string ShazRecognize(const std::vector<uint8_t>& sig, int numSamples)
{
    std::string b64 = Base64Encode(sig.data(), sig.size());
    std::string uri = "data:audio/vnd.shazam.sig;base64," + b64;

    // Build JSON body
    int samplems = (int)((float)numSamples / 16000.0f * 1000.0f);
    // A fixed timestamp is fine; the server doesn't validate it strictly.
    unsigned int ts = GetTickCount();
    char body[64];
    std::string json = "{\"geolocation\":{\"altitude\":300,\"latitude\":45,\"longitude\":2},";
    json += "\"signature\":{\"samplems\":";
    sprintf_s(body, "%d", samplems); json += body;
    json += ",\"timestamp\":";
    sprintf_s(body, "%u", ts); json += body;
    json += ",\"uri\":\"" + uri + "\"},\"timestamp\":";
    json += body;
    json += ",\"timezone\":\"Europe/Paris\"}";

    // Two random UUIDs for the tag URL
    auto uuid = [](bool upper) {
        const char* hx = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        char s[37]; int p = 0;
        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) s[p++] = '-';
            else if (i == 14) s[p++] = '4';
            else if (i == 19) s[p++] = hx[8 + (rand() & 3)];
            else s[p++] = hx[rand() & 15];
        }
        s[p] = 0; return std::string(s);
    };
    std::string u1 = uuid(true), u2 = uuid(false);
    std::wstring path = L"/discovery/v5/en/US/android/-/tag/";
    wchar_t wu[80];
    MultiByteToWideChar(CP_ACP, 0, u1.c_str(), -1, wu, 80); path += wu; path += L"/";
    MultiByteToWideChar(CP_ACP, 0, u2.c_str(), -1, wu, 80); path += wu;
    path += L"?sync=true&webv3=true&sampling=true&connected=&shazamapiversion=v3&sharehub=true&video=v3";

    std::string result;
    HINTERNET hSession = WinHttpOpen(L"Dalvik/2.1.0 (Linux; U; Android 5.0.2; VS980 4G Build/LRX22G)",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return result;
    WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 15000);
    HINTERNET hConnect = WinHttpConnect(hSession, L"amp.shazam.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return result; }
    HINTERNET hReq = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hReq) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return result; }
    const wchar_t* hdrs = L"Content-Type: application/json\r\nContent-Language: en_US\r\n";
    if (WinHttpSendRequest(hReq, hdrs, (DWORD)-1, (LPVOID)json.data(), (DWORD)json.size(),
            (DWORD)json.size(), 0) && WinHttpReceiveResponse(hReq, NULL)) {
        DWORD size = 0;
        while (WinHttpQueryDataAvailable(hReq, &size) && size > 0) {
            std::string chunk(size, '\0');
            DWORD read = 0;
            WinHttpReadData(hReq, &chunk[0], size, &read);
            result.append(chunk, 0, read);
        }
    }
    WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
    return result;
}

// --- Recognition state + capture ---
static HWND     g_hwndShazam = NULL;
static HDSP     g_shazDsp = 0;
static std::vector<float> g_shazBuf;      // native-rate mono capture
static volatile LONG g_shazPos = 0;
static int      g_shazNeeded = 0;         // native samples to capture
static int      g_shazNativeRate = 44100;
static int      g_shazNativeChans = 2;
static volatile bool g_shazCapturing = false;
static HANDLE   g_shazThread = NULL;
static wchar_t  g_shazTitle[256] = L"";
static wchar_t  g_shazArtist[256] = L"";
static wchar_t  g_shazCoverUrl[512] = L"";
static HBITMAP  g_shazCoverBmp = NULL;
#define WM_SHAZ_DONE  (WM_APP + 20)   // wParam: 1=match 0=no match -1=error

void CALLBACK DSP_ShazCapture(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
{
    if (!g_shazCapturing) return;
    float* buf = (float*)buffer;
    int nch = g_shazNativeChans;
    DWORD frames = (length / sizeof(float)) / nch;
    LONG pos = g_shazPos;
    for (DWORD i = 0; i < frames && pos < g_shazNeeded; i++) {
        float m = 0;
        for (int c = 0; c < nch; c++) m += buf[i * nch + c];
        g_shazBuf[pos++] = m / nch;
    }
    g_shazPos = pos;
    if (pos >= g_shazNeeded) g_shazCapturing = false;
}

// Worker: resample -> signature -> HTTP -> parse -> notify dialog
static DWORD WINAPI ShazWorker(LPVOID)
{
    int captured = g_shazPos;
    int rate = g_shazNativeRate;

    // Anti-alias low-pass (two cascaded 2nd-order Butterworth biquads ~7.5 kHz)
    {
        double fc = 7500.0, fs = rate, Q = 0.7071;
        double w0 = 2.0 * 3.14159265358979323846 * fc / fs;
        double cw = cos(w0), sw = sin(w0), alpha = sw / (2.0 * Q);
        double a0 = 1 + alpha;
        float b0 = (float)((1 - cw) / 2 / a0), b1 = (float)((1 - cw) / a0), b2 = b0;
        float a1 = (float)(-2 * cw / a0), a2 = (float)((1 - alpha) / a0);
        for (int stage = 0; stage < 2; stage++) {
            float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
            for (int i = 0; i < captured; i++) {
                float x = g_shazBuf[i];
                float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                x2 = x1; x1 = x; y2 = y1; y1 = y;
                g_shazBuf[i] = y;
            }
        }
    }

    // Resample native -> 16000 (linear)
    double ratio = (double)rate / 16000.0;
    int outLen = (int)(captured / ratio);
    if (outLen > 16000 * 12) outLen = 16000 * 12;
    std::vector<float> mono16k(outLen);
    for (int i = 0; i < outLen; i++) {
        double sp = i * ratio;
        int i0 = (int)sp;
        double fr = sp - i0;
        float a = g_shazBuf[i0];
        float b = (i0 + 1 < captured) ? g_shazBuf[i0 + 1] : a;
        mono16k[i] = a + (float)fr * (b - a);
    }

    std::vector<uint8_t> sig = ShazMakeSignature(mono16k);
    std::string resp = ShazRecognize(sig, outLen);

    g_shazTitle[0] = 0; g_shazArtist[0] = 0; g_shazCoverUrl[0] = 0;
    LRESULT outcome = -1;
    if (!resp.empty()) {
        const char* track = strstr(resp.c_str(), "\"track\"");
        if (track) {
            std::wstring t = JsonGet(track, "title");
            std::wstring a = JsonGet(track, "subtitle");
            if (!t.empty()) {
                wcsncpy_s(g_shazTitle, t.c_str(), _TRUNCATE);
                wcsncpy_s(g_shazArtist, a.c_str(), _TRUNCATE);
                const char* imgs = strstr(track, "\"images\"");
                if (imgs) {
                    std::wstring cov = JsonGet(imgs, "coverart");
                    wcsncpy_s(g_shazCoverUrl, cov.c_str(), _TRUNCATE);
                }
                outcome = 1;
            } else {
                outcome = 0;
            }
        } else {
            outcome = 0;   // valid response, no match
        }
    }
    if (g_hwndShazam && IsWindow(g_hwndShazam))
        PostMessage(g_hwndShazam, WM_SHAZ_DONE, (WPARAM)outcome, 0);
    return 0;
}

static void ShazStartCapture()
{
    if (g_shazCapturing) return;
    BASS_CHANNELINFO ci = {};
    BASS_ChannelGetInfo(currentStream, &ci);
    g_shazNativeRate = ci.freq > 0 ? ci.freq : 44100;
    g_shazNativeChans = ci.chans > 0 ? ci.chans : 2;
    g_shazNeeded = g_shazNativeRate * 12;   // 12 seconds
    g_shazBuf.assign(g_shazNeeded, 0.0f);
    g_shazPos = 0;
    g_shazCapturing = true;
    g_shazDsp = BASS_ChannelSetDSP(currentStream, DSP_ShazCapture, NULL, 20);
}

static void ShazStopCapture()
{
    g_shazCapturing = false;
    if (g_shazDsp && currentStream) { BASS_ChannelRemoveDSP(currentStream, g_shazDsp); g_shazDsp = 0; }
}

// --- Shazam recognition history (library\shazam_history.txt) ---
// Each match's cover art is saved as a thumbnail in library\shazam_art\.
struct ShazHistItem { std::wstring when, artist, title, art; };  // art = filename in shazam_art
static std::vector<ShazHistItem> g_shazHistory;
static std::map<std::wstring, HBITMAP> g_shazArtCache;   // art filename -> 40px bitmap
static bool g_shazHistoryMode = false;                    // dialog opened via History menu
#define SHAZ_HIST_THUMB 40

static void ShazGetHistoryPath(wchar_t* out, int n)
{
    GetLibDir();
    swprintf_s(out, n, L"%s\\shazam_history.txt", g_libDir);
}

static void ShazGetArtDir(wchar_t* out, int n)
{
    GetLibDir();
    swprintf_s(out, n, L"%s\\shazam_art", g_libDir);
    CreateDirectoryW(out, NULL);
}

// Load a saved art thumbnail (cached). Returns NULL if none.
static HBITMAP ShazArtThumb(const std::wstring& artFile)
{
    if (artFile.empty()) return NULL;
    auto it = g_shazArtCache.find(artFile);
    if (it != g_shazArtCache.end()) return it->second;
    wchar_t dir[MAX_PATH]; ShazGetArtDir(dir, MAX_PATH);
    wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", dir, artFile.c_str());
    HBITMAP bm = NULL;
    HANDLE hf = CreateFile(full, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hf != INVALID_HANDLE_VALUE) {
        DWORD sz = GetFileSize(hf, NULL);
        if (sz > 100 && sz < 8 * 1024 * 1024) {
            std::vector<BYTE> data(sz); DWORD rd = 0;
            ReadFile(hf, data.data(), sz, &rd, NULL);
            if (rd == sz) bm = DecodeImageToThumb(data.data(), sz, SHAZ_HIST_THUMB);
        }
        CloseHandle(hf);
    }
    g_shazArtCache[artFile] = bm;   // cache even NULL to avoid re-reading
    return bm;
}

static void ShazClearArtCache()
{
    for (auto& p : g_shazArtCache) if (p.second) DeleteObject(p.second);
    g_shazArtCache.clear();
}

static void ShazLoadHistory()
{
    g_shazHistory.clear();
    wchar_t path[MAX_PATH]; ShazGetHistoryPath(path, MAX_PATH);
    FILE* f = _wfopen(path, L"r,ccs=UTF-8");
    if (!f) return;
    wchar_t line[1024];
    while (fgetws(line, _countof(line), f)) {
        int len = (int)wcslen(line);
        while (len > 0 && (line[len-1] == L'\n' || line[len-1] == L'\r')) line[--len] = 0;
        if (len == 0) continue;
        // Format: when \t artist \t title [\t artfile]
        ShazHistItem it;
        wchar_t* t1 = wcschr(line, L'\t');
        if (!t1) continue;
        *t1 = 0; it.when = line;
        wchar_t* t2 = wcschr(t1 + 1, L'\t');
        if (t2) {
            *t2 = 0; it.artist = t1 + 1;
            wchar_t* t3 = wcschr(t2 + 1, L'\t');
            if (t3) { *t3 = 0; it.title = t2 + 1; it.art = t3 + 1; }
            else    { it.title = t2 + 1; }
        } else {
            it.artist = t1 + 1;
        }
        g_shazHistory.push_back(it);
    }
    fclose(f);
}

// Save raw cover-art bytes to shazam_art and return the filename (empty on fail)
static std::wstring ShazSaveArt(const std::vector<BYTE>& img)
{
    if (img.size() < 100) return L"";
    wchar_t dir[MAX_PATH]; ShazGetArtDir(dir, MAX_PATH);
    SYSTEMTIME st; GetLocalTime(&st);
    bool png = (img[0] == 0x89 && img.size() > 1 && img[1] == 'P');
    wchar_t name[64];
    swprintf_s(name, L"shaz_%04d%02d%02d_%02d%02d%02d.%s",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, png ? L"png" : L"jpg");
    wchar_t full[MAX_PATH]; swprintf_s(full, L"%s\\%s", dir, name);
    HANDLE hf = CreateFile(full, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE) return L"";
    DWORD wr = 0; WriteFile(hf, img.data(), (DWORD)img.size(), &wr, NULL);
    CloseHandle(hf);
    return (wr == img.size()) ? std::wstring(name) : L"";
}

static void ShazAppendHistory(const wchar_t* artist, const wchar_t* title, const std::wstring& artFile)
{
    SYSTEMTIME st; GetLocalTime(&st);
    wchar_t when[32];
    swprintf_s(when, L"%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    // Skip if identical to the most recent entry (avoid duplicate spam on retry)
    if (!g_shazHistory.empty() &&
        _wcsicmp(g_shazHistory.back().title.c_str(), title) == 0 &&
        _wcsicmp(g_shazHistory.back().artist.c_str(), artist ? artist : L"") == 0)
        return;
    wchar_t path[MAX_PATH]; ShazGetHistoryPath(path, MAX_PATH);
    FILE* f = _wfopen(path, L"a,ccs=UTF-8");
    if (f) {
        fwprintf(f, L"%s\t%s\t%s\t%s\n", when, artist ? artist : L"", title, artFile.c_str());
        fclose(f);
    }
    ShazHistItem it; it.when = when; it.artist = artist ? artist : L""; it.title = title; it.art = artFile;
    g_shazHistory.push_back(it);
}

static void ShazFillHistoryList(HWND hList)
{
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    // Most recent first — item data is the history index (owner-drawn)
    for (int i = (int)g_shazHistory.size() - 1; i >= 0; i--) {
        int idx = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)g_shazHistory[i].title.c_str());
        SendMessage(hList, LB_SETITEMDATA, idx, (LPARAM)i);
    }
}

static LRESULT CALLBACK ShazamWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Dialog phase: 0=listening 1=identifying 2=result 3=nomatch 4=error
    static int  phase = 0;
    static int  animTick = 0;
    switch (msg) {
    case WM_CREATE: {
        if (g_darkMode) { BOOL dk = TRUE; DwmSetWindowAttribute(hwnd, 20, &dk, sizeof(dk)); DwmSetWindowAttribute(hwnd, 19, &dk, sizeof(dk)); }
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        phase = 0; animTick = 0;
        // Artwork placeholder (owner-draw static)
        CreateWindow(L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
            20, 20, 96, 96, hwnd, (HMENU)ID_SHAZ_ARTWORK, hInst, NULL);
        HWND hRes = CreateWindow(L"STATIC", L"Listening...",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX, 130, 30, 250, 76, hwnd, (HMENU)ID_SHAZ_RESULT, hInst, NULL);
        SendMessage(hRes, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hG = CreateWindow(L"BUTTON", L"Search on Google",
            WS_CHILD | BS_PUSHBUTTON, 20, 132, 130, 28, hwnd, (HMENU)ID_SHAZ_GOOGLE, hInst, NULL);
        SendMessage(hG, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hC = CreateWindow(L"BUTTON", L"Copy",
            WS_CHILD | BS_PUSHBUTTON, 156, 132, 70, 28, hwnd, (HMENU)ID_SHAZ_COPY, hInst, NULL);
        SendMessage(hC, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hR = CreateWindow(L"BUTTON", L"Try Again",
            WS_CHILD | BS_PUSHBUTTON, 232, 132, 80, 28, hwnd, (HMENU)ID_SHAZ_RETRY, hInst, NULL);
        SendMessage(hR, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hX = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 318, 132, 62, 28, hwnd, (HMENU)ID_SHAZ_CLOSE, hInst, NULL);
        SendMessage(hX, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        // History list (owner-drawn with cover thumbnails)
        HWND hHl = CreateWindow(L"STATIC", L"History  (double-click to search on Google)",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 172, 360, 16, hwnd, (HMENU)ID_SHAZ_HISTLBL, hInst, NULL);
        SendMessage(hHl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        HWND hHist = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT
            | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
            20, 190, 360, 140, hwnd, (HMENU)ID_SHAZ_HISTORY, hInst, NULL);
        SendMessage(hHist, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SendMessage(hHist, LB_SETITEMHEIGHT, 0, SHAZ_HIST_THUMB + 6);
        ShazLoadHistory();
        ShazFillHistoryList(hHist);
        if (g_shazHistoryMode) {
            // Opened from Shazam -> History: show history, don't auto-listen
            phase = 5;
            SetDlgItemText(hwnd, ID_SHAZ_RESULT,
                L"Recognition history.\r\nPress “Recognize” to identify what's playing.");
            SetDlgItemText(hwnd, ID_SHAZ_RETRY, L"Recognize");
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_RETRY), SW_SHOW);
        } else {
            ShazStartCapture();
            SetTimer(hwnd, 1, 100, NULL);
        }
        break;
    }
    case WM_TIMER: {
        animTick++;
        if (phase == 0) {
            int pct = g_shazNeeded ? (g_shazPos * 100 / g_shazNeeded) : 0;
            wchar_t s[64]; swprintf_s(s, L"Listening...  %d%%", min(100, pct));
            SetDlgItemText(hwnd, ID_SHAZ_RESULT, s);
            InvalidateRect(GetDlgItem(hwnd, ID_SHAZ_ARTWORK), NULL, FALSE);
            if (!g_shazCapturing || g_shazPos >= g_shazNeeded) {
                ShazStopCapture();
                phase = 1;
                SetDlgItemText(hwnd, ID_SHAZ_RESULT, L"Identifying...");
                if (g_shazThread) { CloseHandle(g_shazThread); g_shazThread = NULL; }
                g_shazThread = CreateThread(NULL, 0, ShazWorker, NULL, 0, NULL);
            }
        } else if (phase == 1) {
            InvalidateRect(GetDlgItem(hwnd, ID_SHAZ_ARTWORK), NULL, FALSE);
        }
        break;
    }
    case WM_SHAZ_DONE: {
        KillTimer(hwnd, 1);
        if (g_shazThread) { CloseHandle(g_shazThread); g_shazThread = NULL; }
        LRESULT r = (LRESULT)wParam;
        if (r == 1) {
            phase = 2;
            wchar_t s[600];
            swprintf_s(s, L"%s\r\n%s", g_shazArtist[0] ? g_shazArtist : L"(unknown artist)",
                g_shazTitle);
            SetDlgItemText(hwnd, ID_SHAZ_RESULT, s);
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_GOOGLE), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_COPY), SW_SHOW);
            SetDlgItemText(hwnd, ID_SHAZ_RETRY, L"Try Again");
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_RETRY), SW_SHOW);
            // Fetch cover art, save it to disk, then record history with the filename
            std::wstring artFile;
            if (g_shazCoverUrl[0]) {
                std::vector<BYTE> img = HttpGetBinary(g_shazCoverUrl);
                if (!img.empty() && img.size() > 100) {
                    if (g_shazCoverBmp) { DeleteObject(g_shazCoverBmp); g_shazCoverBmp = NULL; }
                    g_shazCoverBmp = DecodeImageToThumb(img.data(), (DWORD)img.size(), 96);
                    artFile = ShazSaveArt(img);
                }
            }
            ShazAppendHistory(g_shazArtist, g_shazTitle, artFile);
            ShazFillHistoryList(GetDlgItem(hwnd, ID_SHAZ_HISTORY));
            InvalidateRect(GetDlgItem(hwnd, ID_SHAZ_ARTWORK), NULL, TRUE);
        } else if (r == 0) {
            phase = 3;
            SetDlgItemText(hwnd, ID_SHAZ_RESULT, L"No match found.\r\nTry again during a clear part of the track.");
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_RETRY), SW_SHOW);
        } else {
            phase = 4;
            SetDlgItemText(hwnd, ID_SHAZ_RESULT, L"Recognition failed.\r\nCheck your internet connection.");
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_RETRY), SW_SHOW);
        }
        InvalidateRect(GetDlgItem(hwnd, ID_SHAZ_ARTWORK), NULL, TRUE);
        break;
    }
    case WM_MEASUREITEM: {
        MEASUREITEMSTRUCT* ms = (MEASUREITEMSTRUCT*)lParam;
        if (ms->CtlID == ID_SHAZ_HISTORY) { ms->itemHeight = SHAZ_HIST_THUMB + 6; return TRUE; }
        break;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID == ID_SHAZ_ARTWORK) {
            HDC dc = dis->hDC; RECT rc = dis->rcItem;
            if (phase == 2 && g_shazCoverBmp) {
                HDC mdc = CreateCompatibleDC(dc);
                HBITMAP ob = (HBITMAP)SelectObject(mdc, g_shazCoverBmp);
                StretchBlt(dc, rc.left, rc.top, 96, 96, mdc, 0, 0, 96, 96, SRCCOPY);
                SelectObject(mdc, ob); DeleteDC(mdc);
            } else {
                // Animated pulsing circles while listening/identifying
                HBRUSH bg = CreateSolidBrush(g_theme.bg);
                FillRect(dc, &rc, bg); DeleteObject(bg);
                int cx = (rc.left + rc.right) / 2, cy = (rc.top + rc.bottom) / 2;
                for (int ring = 0; ring < 3; ring++) {
                    int r = 12 + ((animTick * 3 + ring * 14) % 42);
                    int alpha = 255 - (r * 5);
                    if (alpha < 40) alpha = 40;
                    COLORREF col = (phase >= 2) ? g_theme.textDim
                        : RGB(0x1E + (ring * 10), 0x78, 0xD2);
                    HPEN pen = CreatePen(PS_SOLID, 2, col);
                    HPEN op = (HPEN)SelectObject(dc, pen);
                    SelectObject(dc, GetStockObject(NULL_BRUSH));
                    Ellipse(dc, cx - r, cy - r, cx + r, cy + r);
                    SelectObject(dc, op); DeleteObject(pen);
                }
                // Center "S"
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, RGB(0x1E, 0x78, 0xD2));
                HFONT hf = CreateFont(-34, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    DEFAULT_PITCH, L"Segoe UI");
                HFONT of = (HFONT)SelectObject(dc, hf);
                RECT tr = { cx - 20, cy - 22, cx + 20, cy + 22 };
                DrawText(dc, L"S", 1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(dc, of); DeleteObject(hf);
            }
            // Border
            HPEN pen = CreatePen(PS_SOLID, 1, g_theme.btnBorder);
            HPEN op = (HPEN)SelectObject(dc, pen);
            SelectObject(dc, GetStockObject(NULL_BRUSH));
            Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(dc, op); DeleteObject(pen);
            return TRUE;
        }
        if (dis->CtlID == ID_SHAZ_HISTORY) {
            HDC dc = dis->hDC; RECT rc = dis->rcItem;
            int hi = (int)dis->itemData;   // index into g_shazHistory
            bool seld = (dis->itemState & ODS_SELECTED) != 0;
            COLORREF bgc = seld ? (g_darkMode ? RGB(30, 80, 160) : GetSysColor(COLOR_HIGHLIGHT)) : g_theme.bgList;
            COLORREF txc = seld ? RGB(255, 255, 255) : g_theme.text;
            COLORREF dmc = seld ? RGB(210, 225, 245) : g_theme.textDim;
            HBRUSH bb = CreateSolidBrush(bgc); FillRect(dc, &rc, bb); DeleteObject(bb);
            if (hi >= 0 && hi < (int)g_shazHistory.size()) {
                auto& h = g_shazHistory[hi];
                int thumbX = rc.left + 3, thumbY = rc.top + 3;
                HBITMAP th = ShazArtThumb(h.art);
                if (th) {
                    HDC mdc = CreateCompatibleDC(dc);
                    HBITMAP ob = (HBITMAP)SelectObject(mdc, th);
                    BitBlt(dc, thumbX, thumbY, SHAZ_HIST_THUMB, SHAZ_HIST_THUMB, mdc, 0, 0, SRCCOPY);
                    SelectObject(mdc, ob); DeleteDC(mdc);
                } else {
                    HBRUSH pb = CreateSolidBrush(g_darkMode ? RGB(56,56,56) : RGB(210,210,210));
                    RECT pr = { thumbX, thumbY, thumbX + SHAZ_HIST_THUMB, thumbY + SHAZ_HIST_THUMB };
                    FillRect(dc, &pr, pb); DeleteObject(pb);
                }
                int tx = thumbX + SHAZ_HIST_THUMB + 8;
                SetBkMode(dc, TRANSPARENT);
                // Line 1: Artist - Title (bold)
                HFONT of = (HFONT)SelectObject(dc, g_fontBold ? g_fontBold : g_fontUI);
                SetTextColor(dc, txc);
                wchar_t l1[540];
                swprintf_s(l1, L"%s%s%s", h.artist.empty() ? L"" : h.artist.c_str(),
                    h.artist.empty() ? L"" : L" - ", h.title.c_str());
                RECT r1 = { tx, rc.top + 4, rc.right - 4, rc.top + 22 };
                DrawText(dc, l1, -1, &r1, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
                // Line 2: timestamp (dim)
                SelectObject(dc, g_fontUI);
                SetTextColor(dc, dmc);
                RECT r2 = { tx, rc.top + 22, rc.right - 4, rc.bottom - 2 };
                DrawText(dc, h.when.c_str(), -1, &r2, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);
                SelectObject(dc, of);
            }
            if (dis->itemState & ODS_FOCUS) DrawFocusRect(dc, &rc);
            return TRUE;
        }
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_SHAZ_HISTORY:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                HWND hL = GetDlgItem(hwnd, ID_SHAZ_HISTORY);
                int sel = (int)SendMessage(hL, LB_GETCURSEL, 0, 0);
                if (sel < 0) break;
                int idx = (int)SendMessage(hL, LB_GETITEMDATA, sel, 0);
                if (idx >= 0 && idx < (int)g_shazHistory.size()) {
                    std::wstring q = g_shazHistory[idx].artist + L" " + g_shazHistory[idx].title;
                    std::string enc = UrlEncode(q.c_str());
                    wchar_t wenc[512] = {};
                    MultiByteToWideChar(CP_UTF8, 0, enc.c_str(), -1, wenc, 512);
                    wchar_t url[600]; swprintf_s(url, L"https://www.google.com/search?q=%s", wenc);
                    ShellExecute(hwnd, L"open", url, NULL, NULL, SW_SHOW);
                }
            }
            break;
        case ID_SHAZ_GOOGLE:
            if (g_shazTitle[0]) {
                std::wstring q = std::wstring(g_shazArtist) + L" " + g_shazTitle;
                std::string enc = UrlEncode(q.c_str());
                wchar_t wenc[512] = {};
                MultiByteToWideChar(CP_UTF8, 0, enc.c_str(), -1, wenc, 512);
                wchar_t url[600]; swprintf_s(url, L"https://www.google.com/search?q=%s", wenc);
                ShellExecute(hwnd, L"open", url, NULL, NULL, SW_SHOW);
            }
            break;
        case ID_SHAZ_COPY:
            if (g_shazTitle[0]) {
                std::wstring txt = std::wstring(g_shazArtist) + L" - " + g_shazTitle;
                if (OpenClipboard(hwnd)) {
                    EmptyClipboard();
                    size_t bytes = (txt.size() + 1) * sizeof(wchar_t);
                    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, bytes);
                    if (hg) {
                        memcpy(GlobalLock(hg), txt.c_str(), bytes);
                        GlobalUnlock(hg);
                        SetClipboardData(CF_UNICODETEXT, hg);
                    }
                    CloseClipboard();
                }
            }
            break;
        case ID_SHAZ_RETRY: {
            // Validate there's something to listen to (stream always; local only if allowed)
            bool isLocal = (currentStream && g_decStream != 0);
            if (!currentStream || (isLocal && !g_shazAllowLocal)) {
                MessageBox(hwnd,
                    L"Play internet radio or a stream link first.\r\n\r\n"
                    L"To identify local files too, enable it in Settings → Setup.",
                    L"Recognize", MB_ICONINFORMATION);
                break;
            }
            if (g_shazThread) { WaitForSingleObject(g_shazThread, 3000); CloseHandle(g_shazThread); g_shazThread = NULL; }
            if (g_shazCoverBmp) { DeleteObject(g_shazCoverBmp); g_shazCoverBmp = NULL; }
            phase = 0; animTick = 0;
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_GOOGLE), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_COPY), SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, ID_SHAZ_RETRY), SW_HIDE);
            SetDlgItemText(hwnd, ID_SHAZ_RESULT, L"Listening...");
            ShazStartCapture();
            SetTimer(hwnd, 1, 100, NULL);
            break;
        }
        case ID_SHAZ_CLOSE:
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        }
        break;
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bg); SetBkMode(dc, TRANSPARENT);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text); SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_KEYDOWN: if (wParam == VK_ESCAPE) DestroyWindow(hwnd); break;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        ShazStopCapture();
        if (g_shazThread) { WaitForSingleObject(g_shazThread, 3000); CloseHandle(g_shazThread); g_shazThread = NULL; }
        if (g_shazCoverBmp) { DeleteObject(g_shazCoverBmp); g_shazCoverBmp = NULL; }
        ShazClearArtCache();
        g_shazBuf.clear(); g_shazBuf.shrink_to_fit();
        g_hwndShazam = NULL;
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// historyMode = open straight to the History view (no auto-listen)
static void OpenShazamDialog(bool historyMode)
{
    // Recognition needs audio playing. Streams always allowed; local files
    // only when the option is enabled (Settings → Setup).
    if (!historyMode) {
        bool isStream = (currentStream && g_decStream == 0);
        bool isLocal  = (currentStream && g_decStream != 0);
        if (!currentStream || (isLocal && !g_shazAllowLocal)) {
            MessageBox(g_hwnd,
                L"Song recognition works while internet radio or a stream link is playing.\r\n\r\n"
                L"To identify local files too, enable it in Settings → Setup.",
                L"Recognize", MB_ICONINFORMATION);
            return;
        }
        (void)isStream;
    }
    g_shazHistoryMode = historyMode;
    if (g_hwndShazam && IsWindow(g_hwndShazam)) { SetForegroundWindow(g_hwndShazam); return; }
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE);
    static const wchar_t SHAZ_CLASS[] = L"BillyShazamWnd";
    static bool shazReg = false;
    if (!shazReg) {
        WNDCLASS wc = {}; wc.lpfnWndProc = ShazamWndProc; wc.hInstance = hInst;
        wc.lpszClassName = SHAZ_CLASS; wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); RegisterClass(&wc); shazReg = true;
    }
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
    RECT cr = { 0, 0, 400, 350 }; AdjustWindowRect(&cr, style, FALSE);
    int dw = cr.right - cr.left, dh = cr.bottom - cr.top;
    g_hwndShazam = CreateWindowEx(WS_EX_APPWINDOW, SHAZ_CLASS,
        L"Recognize Song", style, 0, 0, dw, dh, g_hwnd, NULL, hInst, NULL);
    RECT pr; GetWindowRect(g_hwnd, &pr);
    SetWindowPos(g_hwndShazam, NULL,
        pr.left + (pr.right - pr.left - dw) / 2,
        pr.top + (pr.bottom - pr.top - dh) / 2,
        dw, dh, SWP_NOZORDER);
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

        // Stream links open via URL so format/tags come from the stream
        bool infoIsUrl = (_wcsnicmp(s_filePath, L"http://", 7) == 0 ||
                          _wcsnicmp(s_filePath, L"https://", 8) == 0 ||
                          _wcsnicmp(s_filePath, L"ftp://", 6) == 0);
        HSTREAM s = 0;
        if (infoIsUrl) {
            int uLen = WideCharToMultiByte(CP_UTF8, 0, s_filePath, -1, NULL, 0, NULL, NULL);
            std::string url8(uLen, '\0');
            WideCharToMultiByte(CP_UTF8, 0, s_filePath, -1, &url8[0], uLen, NULL, NULL);
            BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 8000);
            s = BASS_StreamCreateURL(url8.c_str(), 0, 0, NULL, NULL);
        } else {
            s = BASS_StreamCreateFile(FALSE, s_filePath, 0, 0,
                BASS_UNICODE | BASS_STREAM_DECODE);
        }

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

        // (artwork buttons removed — artwork is managed via Tag Database / Discogs)

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

            ULONGLONG fsz = 0;
            if (infoIsUrl) {
                QWORD netSz = BASS_StreamGetFilePosition(s, BASS_FILEPOS_SIZE);
                if (netSz != (QWORD)-1) fsz = netSz;
            } else {
                WIN32_FILE_ATTRIBUTE_DATA fa = {};
                GetFileAttributesEx(s_filePath, GetFileExInfoStandard, &fa);
                fsz = ((ULONGLONG)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;
            }

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

        // Populate tag fields — try ID3v2 first (what we write), fall back to ID3v1
        if (s) {
            bool tagsRead = false;
            // ID3v2: parse TIT2, TPE1, TALB, TYER/TDRC, TRCK, TCON frames
            const void* id3v2raw = BASS_ChannelGetTags(s, BASS_TAG_ID3V2);
            if (id3v2raw) {
                const BYTE* data = (const BYTE*)id3v2raw;
                if (memcmp(data, "ID3", 3) == 0) {
                    DWORD tagSize = ((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) |
                        ((data[8] & 0x7F) << 7) | (data[9] & 0x7F);
                    BYTE ver = data[3];
                    const BYTE* p = data + 10;
                    const BYTE* end = data + 10 + tagSize;
                    while (p + 10 < end) {
                        char fid[5] = {}; memcpy(fid, p, 4);
                        DWORD fsize;
                        if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
                        else          fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
                        p += 10;
                        if (fsize == 0 || p + fsize > end) break;
                        // Text frames: first byte = encoding (0=Latin1, 1=UTF16, 2=UTF16BE, 3=UTF8)
                        HWND target = NULL;
                        if (strcmp(fid, "TIT2") == 0) target = hTitle;
                        else if (strcmp(fid, "TPE1") == 0) target = hArtist;
                        else if (strcmp(fid, "TALB") == 0) target = hAlbum;
                        else if (strcmp(fid, "TYER") == 0 || strcmp(fid, "TDRC") == 0) target = hYear;
                        else if (strcmp(fid, "TRCK") == 0) target = hTrack;
                        else if (strcmp(fid, "TCON") == 0) target = hGenre;
                        if (target && fsize > 1) {
                            BYTE enc = p[0];
                            wchar_t val[256] = {};
                            if (enc == 3) { // UTF-8
                                MultiByteToWideChar(CP_UTF8, 0, (const char*)(p + 1), fsize - 1, val, 255);
                            } else if (enc == 0) { // Latin-1
                                MultiByteToWideChar(CP_ACP, 0, (const char*)(p + 1), fsize - 1, val, 255);
                            } else if (enc == 1 || enc == 2) { // UTF-16
                                int charCount = min((int)(fsize - 1) / 2, 255);
                                const wchar_t* src = (const wchar_t*)(p + 1);
                                // Skip BOM if present
                                if (charCount > 0 && (src[0] == 0xFEFF || src[0] == 0xFFFE)) { src++; charCount--; }
                                wcsncpy_s(val, src, charCount);
                            }
                            if (val[0]) { SetWindowText(target, val); tagsRead = true; }
                        }
                        p += fsize;
                    }
                }
            }
            // Fallback: Vorbis Comments via BASS_TAG_OGG (FLAC/OGG)
            if (!tagsRead) {
                const char* ogg = BASS_ChannelGetTags(s, BASS_TAG_OGG);
                if (ogg) {
                    while (*ogg) {
                        const char* eq = strchr(ogg, '=');
                        if (eq) {
                            int keyLen = (int)(eq - ogg);
                            const char* val = eq + 1;
                            wchar_t wval[256] = {};
                            MultiByteToWideChar(CP_UTF8, 0, val, -1, wval, 255);
                            HWND target = NULL;
                            if (_strnicmp(ogg, "TITLE", keyLen) == 0 && keyLen == 5) target = hTitle;
                            else if (_strnicmp(ogg, "ARTIST", keyLen) == 0 && keyLen == 6) target = hArtist;
                            else if (_strnicmp(ogg, "ALBUM", keyLen) == 0 && keyLen == 5) target = hAlbum;
                            else if (_strnicmp(ogg, "DATE", keyLen) == 0 && keyLen == 4) target = hYear;
                            else if (_strnicmp(ogg, "TRACKNUMBER", keyLen) == 0 && keyLen == 11) target = hTrack;
                            else if (_strnicmp(ogg, "GENRE", keyLen) == 0 && keyLen == 5) target = hGenre;
                            if (target && wval[0]) { SetWindowText(target, wval); tagsRead = true; }
                        }
                        ogg += strlen(ogg) + 1;
                    }
                }
            }
            // Fallback: read FLAC Vorbis Comments directly via libFLAC metadata API
            if (!tagsRead) {
                const wchar_t* fext = wcsrchr(s_filePath, L'.');
                if (fext && _wcsicmp(fext, L".flac") == 0) {
                    int uLen = WideCharToMultiByte(CP_UTF8, 0, s_filePath, -1, NULL, 0, NULL, NULL);
                    std::vector<char> u8buf(uLen > 0 ? uLen : 1, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, s_filePath, -1, u8buf.data(), uLen, NULL, NULL);
                    FLAC__Metadata_Chain* fc = FLAC__metadata_chain_new();
                    if (fc && FLAC__metadata_chain_read(fc, u8buf.data())) {
                        FLAC__Metadata_Iterator* fi = FLAC__metadata_iterator_new();
                        if (fi) {
                            FLAC__metadata_iterator_init(fi, fc);
                            do {
                                FLAC__StreamMetadata* blk = FLAC__metadata_iterator_get_block(fi);
                                if (blk->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
                                    FLAC__StreamMetadata_VorbisComment& vc = blk->data.vorbis_comment;
                                    for (FLAC__uint32 ci = 0; ci < vc.num_comments; ci++) {
                                        const char* entry = (const char*)vc.comments[ci].entry;
                                        const char* eq = strchr(entry, '=');
                                        if (!eq) continue;
                                        int kl = (int)(eq - entry);
                                        const char* val = eq + 1;
                                        wchar_t wv[256] = {};
                                        MultiByteToWideChar(CP_UTF8, 0, val, -1, wv, 255);
                                        HWND tgt = NULL;
                                        if (_strnicmp(entry, "TITLE", kl) == 0 && kl == 5) tgt = hTitle;
                                        else if (_strnicmp(entry, "ARTIST", kl) == 0 && kl == 6) tgt = hArtist;
                                        else if (_strnicmp(entry, "ALBUM", kl) == 0 && kl == 5) tgt = hAlbum;
                                        else if (_strnicmp(entry, "DATE", kl) == 0 && kl == 4) tgt = hYear;
                                        else if (_strnicmp(entry, "TRACKNUMBER", kl) == 0 && kl == 11) tgt = hTrack;
                                        else if (_strnicmp(entry, "GENRE", kl) == 0 && kl == 5) tgt = hGenre;
                                        if (tgt && wv[0]) { SetWindowText(tgt, wv); tagsRead = true; }
                                    }
                                    break;
                                }
                            } while (FLAC__metadata_iterator_next(fi));
                            FLAC__metadata_iterator_delete(fi);
                        }
                    }
                    if (fc) FLAC__metadata_chain_delete(fc);
                }
            }
            // Fallback for WAV: parse RIFF chunks for "id3 " chunk containing ID3v2
            if (!tagsRead) {
                FILE* wavf = NULL;
                _wfopen_s(&wavf, s_filePath, L"rb");
                if (wavf) {
                    BYTE whdr[12];
                    if (fread(whdr, 1, 12, wavf) == 12 && memcmp(whdr, "RIFF", 4) == 0 && memcmp(whdr + 8, "WAVE", 4) == 0) {
                        while (true) {
                            BYTE ch[8];
                            if (fread(ch, 1, 8, wavf) != 8) break;
                            DWORD sz = ch[4] | (ch[5] << 8) | (ch[6] << 16) | (ch[7] << 24);
                            if (sz == 0 || sz > 50 * 1024 * 1024) break;
                            bool isId3 = (memcmp(ch, "id3 ", 4) == 0 || memcmp(ch, "ID3 ", 4) == 0 || memcmp(ch, "ID32", 4) == 0);
                            if (isId3 && sz > 10) {
                                BYTE id3h[10];
                                if (fread(id3h, 1, 10, wavf) == 10 && memcmp(id3h, "ID3", 3) == 0) {
                                    DWORD tagSz = ((id3h[6] & 0x7F) << 21) | ((id3h[7] & 0x7F) << 14) |
                                        ((id3h[8] & 0x7F) << 7) | (id3h[9] & 0x7F);
                                    BYTE wver = id3h[3];
                                    if (tagSz > 0 && tagSz <= sz && tagSz <= 8 * 1024 * 1024) {
                                        std::vector<BYTE> wtag(tagSz);
                                        if (fread(wtag.data(), 1, tagSz, wavf) == tagSz) {
                                            const BYTE* wp = wtag.data();
                                            const BYTE* wend = wp + tagSz;
                                            while (wp + 10 < wend) {
                                                char wfid[5] = {}; memcpy(wfid, wp, 4);
                                                DWORD wfsize;
                                                if (wver >= 4) wfsize = ((wp[4] & 0x7F) << 21) | ((wp[5] & 0x7F) << 14) | ((wp[6] & 0x7F) << 7) | (wp[7] & 0x7F);
                                                else           wfsize = (wp[4] << 24) | (wp[5] << 16) | (wp[6] << 8) | wp[7];
                                                wp += 10;
                                                if (wfsize == 0 || wp + wfsize > wend) break;
                                                HWND wtarget = NULL;
                                                if (strcmp(wfid, "TIT2") == 0) wtarget = hTitle;
                                                else if (strcmp(wfid, "TPE1") == 0) wtarget = hArtist;
                                                else if (strcmp(wfid, "TALB") == 0) wtarget = hAlbum;
                                                else if (strcmp(wfid, "TYER") == 0 || strcmp(wfid, "TDRC") == 0) wtarget = hYear;
                                                else if (strcmp(wfid, "TRCK") == 0) wtarget = hTrack;
                                                else if (strcmp(wfid, "TCON") == 0) wtarget = hGenre;
                                                if (wtarget && wfsize > 1) {
                                                    BYTE wenc = wp[0];
                                                    wchar_t wval[256] = {};
                                                    if (wenc == 3) MultiByteToWideChar(CP_UTF8, 0, (const char*)(wp + 1), wfsize - 1, wval, 255);
                                                    else if (wenc == 0) MultiByteToWideChar(CP_ACP, 0, (const char*)(wp + 1), wfsize - 1, wval, 255);
                                                    if (wval[0]) { SetWindowText(wtarget, wval); tagsRead = true; }
                                                }
                                                wp += wfsize;
                                            }
                                        }
                                    }
                                }
                                break; // found id3 chunk, done
                            } else {
                                fseek(wavf, (sz + 1) & ~1, SEEK_CUR);
                            }
                        }
                    }
                    fclose(wavf);
                }
            }
            // Fallback: ID3v1 (old fixed-length tags at end of MP3)
            if (!tagsRead) {
                TAG_ID3* id3 = (TAG_ID3*)BASS_ChannelGetTags(s, BASS_TAG_ID3);
                if (id3) {
                    wchar_t w[256] = {};
                    MultiByteToWideChar(CP_ACP, 0, id3->title, sizeof(id3->title), w, 256); SetWindowText(hTitle, w);
                    MultiByteToWideChar(CP_ACP, 0, id3->artist, sizeof(id3->artist), w, 256); SetWindowText(hArtist, w);
                    MultiByteToWideChar(CP_ACP, 0, id3->album, sizeof(id3->album), w, 256); SetWindowText(hAlbum, w);
                    MultiByteToWideChar(CP_ACP, 0, id3->year, sizeof(id3->year), w, 256); SetWindowText(hYear, w);
                }
            }
            // Load artwork at 220x220 — try BASS ID3v2 first, then our multi-format loader
            hArt = LoadEmbeddedArtworkSized(s, 220);
            BASS_StreamFree(s);
            if (!hArt) {
                // Fallback: LoadThumbFromFile handles FLAC PICTURE, WAV id3 chunks, etc.
                hArt = LoadThumbFromFile(s_filePath, 220);
            }
        }

        // Discogs lookup button
        int dcgY = 460;
        HWND hDiscogs = CreateWindow(L"BUTTON", L"\u2B07 Tag Database (Discogs)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rx, dcgY, rw, 26, hwnd, (HMENU)ID_INFO_DISCOGS, hInst, NULL);
        SendMessage(hDiscogs, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Save Tags + Close buttons at bottom
        int bby = 492;
        HWND hSaveTags = CreateWindow(L"BUTTON", L"Save Tags",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            rx, bby, rw / 2 - 4, 26, hwnd, (HMENU)ID_INFO_SAVE_TAGS, hInst, NULL);
        SendMessage(hSaveTags, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        HWND hClose = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            rx + rw / 2 + 4, bby, rw / 2 - 4, 26, hwnd, (HMENU)ID_INFO_CLOSE, hInst, NULL);
        SendMessage(hClose, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // (close button below artwork removed)

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
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        SetBkMode(dc, TRANSPARENT);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_INFO_CLOSE:
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        case ID_INFO_DISCOGS:
            OpenDiscogsDialog(hwnd, s_filePath);
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
            bool wasPlaying = (s_trackIdx >= 0 && g_currentIndex == s_trackIdx && currentStream != 0);
            if (wasPlaying) StopAudio();
            bool ok = false;
            if (ext && _wcsicmp(ext, L".mp3") == 0) {
                ok = WriteID3v2ToMP3(s_filePath, title, artist, album, year, track, genre);
            } else if (ext && _wcsicmp(ext, L".flac") == 0) {
                ok = WriteTagsToFLAC(s_filePath, title, artist, album, year, track, genre);
            } else if (ext && _wcsicmp(ext, L".wav") == 0) {
                ok = WriteID3v2ToWAV(s_filePath, title, artist, album, year, track, genre);
            } else {
                MessageBox(hwnd, L"Tag writing is supported for MP3, FLAC, and WAV files.\nOGG/M4A require an external tool.", L"Save Tags", MB_ICONWARNING);
                if (wasPlaying) PlayIndex(s_trackIdx);
                break;
            }
            if (ok) {
                // Artwork: keep current hArt (already correct from Discogs preview or original).
                // Only reload if hArt is NULL (no artwork was showing before save).
                if (!hArt) {
                    hArt = LoadThumbFromFile(s_filePath, 220);
                    if (!hArt) {
                        HSTREAM rs = BASS_StreamCreateFile(FALSE, s_filePath, 0, 0,
                            BASS_UNICODE | BASS_STREAM_DECODE);
                        if (rs) {
                            hArt = LoadEmbeddedArtworkSized(rs, 220);
                            BASS_StreamFree(rs);
                        }
                    }
                }
                HWND hArtCtl = GetDlgItem(hwnd, ID_INFO_ARTWORK);
                if (hArtCtl) InvalidateRect(hArtCtl, NULL, TRUE);
                // Clear thumbnail cache for this file so playlist view refreshes
                {
                    std::wstring cacheKey(s_filePath);
                    auto it = g_thumbCache.find(cacheKey);
                    if (it != g_thumbCache.end()) {
                        if (it->second && it->second != (HBITMAP)1) DeleteObject(it->second);
                        g_thumbCache.erase(it);
                    }
                }
                // Refresh playlist listbox
                if (hListBox) InvalidateRect(hListBox, NULL, FALSE);
                MessageBox(hwnd, L"Tags saved successfully.", L"Save Tags", MB_ICONINFORMATION);
            } else {
                MessageBox(hwnd, L"Failed to write tags.\nMake sure the file is not read-only.", L"Save Tags", MB_ICONERROR);
            }
            if (wasPlaying) PlayIndex(s_trackIdx);
            break;
        }
        }
        break;

    case WM_APP + 10: {
        // Discogs artwork preview update: wParam = new HBITMAP at 220px
        HBITMAP hNewArt = (HBITMAP)wParam;
        if (hNewArt) {
            if (hArt) DeleteObject(hArt);
            hArt = hNewArt;
            HWND hArtCtl = GetDlgItem(hwnd, ID_INFO_ARTWORK);
            if (hArtCtl) InvalidateRect(hArtCtl, NULL, TRUE);
        }
        return 0;
    }

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

// Decode src to a PCM WAV file via BASS (no external encoder needed).
// outSampleRate: 0 = keep original, else resample to the target rate
// (anti-alias lowpass on downsampling + linear interpolation).
static bool ConvWriteWav(const wchar_t* src, const wchar_t* dst, bool normalize,
                         int bitDepth = 16, int outSampleRate = 0)
{
    if (bitDepth != 16 && bitDepth != 24 && bitDepth != 32) bitDepth = 16;
    HSTREAM stream = BASS_StreamCreateFile(FALSE, src, 0, 0,
        BASS_STREAM_DECODE | BASS_UNICODE | BASS_SAMPLE_FLOAT);
    if (!stream) return false;

    BASS_CHANNELINFO info = {};
    BASS_ChannelGetInfo(stream, &info);

    WORD  channels = (WORD)info.chans;
    if (channels < 1) channels = 2;
    bool  resample = (outSampleRate > 0 && (DWORD)outSampleRate != info.freq && channels <= 8);
    DWORD sampleRate = resample ? (DWORD)outSampleRate : info.freq;
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

    DWORD totalData = 0;

    // Write `count` interleaved float samples at the requested bit depth
    auto writeSamples = [&](const float* data, int count) {
        if (bitDepth == 32) {
            static std::vector<float> tmp;
            tmp.assign(data, data + count);
            for (int i = 0; i < count; i++) {
                tmp[i] *= gainFactor;
                if (tmp[i] >  1.0f) tmp[i] =  1.0f;
                if (tmp[i] < -1.0f) tmp[i] = -1.0f;
            }
            totalData += (DWORD)fwrite(tmp.data(), sizeof(float), count, f) * sizeof(float);
        } else if (bitDepth == 24) {
            for (int i = 0; i < count; i++) {
                float s = data[i] * gainFactor;
                if (s >  1.0f) s =  1.0f;
                if (s < -1.0f) s = -1.0f;
                int32_t v = (int32_t)(s * 8388607.0f);
                uint8_t b[3] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)((v >> 16) & 0xFF) };
                fwrite(b, 1, 3, f);
                totalData += 3;
            }
        } else {
            static std::vector<short> sb;
            sb.resize(count);
            for (int i = 0; i < count; i++) {
                float s = data[i] * gainFactor;
                if (s >  1.0f) s =  1.0f;
                if (s < -1.0f) s = -1.0f;
                sb[i] = (short)(s * 32767.0f);
            }
            totalData += (DWORD)fwrite(sb.data(), 2, count, f) * 2;
        }
    };

    // Resampler state: linear interpolation with the previous chunk's last
    // frame carried over; anti-alias biquad lowpass when downsampling.
    // Frame positions: 0 = carried frame, 1..frames = this chunk's frames.
    double rsPhase = 1.0;
    double rsStep = resample ? (double)info.freq / (double)sampleRate : 1.0;
    float  rsPrev[8] = {};
    bool   lp = resample && sampleRate < info.freq;
    float lb0 = 0, lb1 = 0, lb2 = 0, la1 = 0, la2 = 0;
    float lpx1[2][8] = {}, lpx2[2][8] = {}, lpy1[2][8] = {}, lpy2[2][8] = {};
    if (lp) {
        double fc = 0.45 * sampleRate, fs = info.freq, Q = 0.7071;
        double w0 = 2.0 * 3.14159265358979323846 * fc / fs;
        double cw = cos(w0), sw = sin(w0), alpha = sw / (2.0 * Q);
        double a0 = 1 + alpha;
        lb0 = (float)((1 - cw) / 2 / a0); lb1 = (float)((1 - cw) / a0); lb2 = lb0;
        la1 = (float)(-2 * cw / a0); la2 = (float)((1 - alpha) / a0);
    }
    std::vector<float> rsOut;

    DWORD read;
    while ((read = BASS_ChannelGetData(stream, fbuf, FBUF_BYTES | BASS_DATA_FLOAT)) != (DWORD)-1 && read > 0) {
        int count = (int)(read / sizeof(float));
        if (!resample) {
            writeSamples(fbuf, count);
            continue;
        }
        int frames = count / channels;
        if (frames <= 0) continue;
        // Anti-alias lowpass (2 cascaded biquads) when downsampling
        if (lp) {
            for (int st = 0; st < 2; st++) {
                for (int c = 0; c < channels; c++) {
                    float x1 = lpx1[st][c], x2 = lpx2[st][c], y1 = lpy1[st][c], y2 = lpy2[st][c];
                    for (int i = 0; i < frames; i++) {
                        float x = fbuf[i * channels + c];
                        float y = lb0 * x + lb1 * x1 + lb2 * x2 - la1 * y1 - la2 * y2;
                        x2 = x1; x1 = x; y2 = y1; y1 = y;
                        fbuf[i * channels + c] = y;
                    }
                    lpx1[st][c] = x1; lpx2[st][c] = x2; lpy1[st][c] = y1; lpy2[st][c] = y2;
                }
            }
        }
        // Linear interpolation to the target rate
        rsOut.clear();
        while (rsPhase < (double)frames) {
            int i0 = (int)rsPhase;
            float fr = (float)(rsPhase - i0);
            for (int c = 0; c < channels; c++) {
                float f0 = (i0 == 0) ? rsPrev[c] : fbuf[(i0 - 1) * channels + c];
                float f1 = fbuf[i0 * channels + c];
                rsOut.push_back(f0 + fr * (f1 - f0));
            }
            rsPhase += rsStep;
        }
        for (int c = 0; c < channels; c++)
            rsPrev[c] = fbuf[(frames - 1) * channels + c];
        rsPhase -= (double)frames;
        if (!rsOut.empty())
            writeSamples(rsOut.data(), (int)rsOut.size());
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
        // Target exists (e.g. converting into the source folder with the same
        // name): append _01, _02, ... instead of overwriting or failing
        if (GetFileAttributesW(outPath) != INVALID_FILE_ATTRIBUTES) {
            for (int suffix = 1; suffix <= 99; suffix++) {
                swprintf_s(outPath, ARRAYSIZE(outPath), L"%s\\%s_%02d.%s", d->outDir, base, suffix, ext);
                if (GetFileAttributesW(outPath) == INVALID_FILE_ATTRIBUTES) break;
            }
        }

        bool success = false;
        if (d->fmt == 1 && d->outSampleRate > 0 && hasFfmpeg) {
            // WAV with resample — prefer ffmpeg's high-quality resampler
            wchar_t wavArgs[64];
            const wchar_t* pcmFmt = (d->wavBitDepth == 32) ? L"f32le" :
                                    (d->wavBitDepth == 24) ? L"s24le" : L"s16le";
            swprintf_s(wavArgs, L"-ar %d -c:a pcm_%s", d->outSampleRate, pcmFmt);
            success = ConvRunFfmpeg(ffmpegPath, d->items[i].path, outPath, wavArgs, d->normalize);
        } else if (d->fmt == 1) {
            // WAV native (resamples internally when a target rate is set)
            success = ConvWriteWav(d->items[i].path, outPath, d->normalize, d->wavBitDepth, d->outSampleRate);
        } else if (hasFfmpeg) {
            success = ConvRunFfmpeg(ffmpegPath, d->items[i].path, outPath, d->qualArgs, d->normalize);
        } else {
            // No ffmpeg — fall back to WAV (with resampling if requested)
            wchar_t wavOut[MAX_PATH];
            swprintf_s(wavOut, ARRAYSIZE(wavOut), L"%s\\%s.wav", d->outDir, base);
            if (GetFileAttributesW(wavOut) != INVALID_FILE_ATTRIBUTES) {
                for (int suffix = 1; suffix <= 99; suffix++) {
                    swprintf_s(wavOut, ARRAYSIZE(wavOut), L"%s\\%s_%02d.wav", d->outDir, base, suffix);
                    if (GetFileAttributesW(wavOut) == INVALID_FILE_ATTRIBUTES) break;
                }
            }
            success = ConvWriteWav(d->items[i].path, wavOut, d->normalize, 16, d->outSampleRate);
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

// Subclass for the convert file list: Ctrl+A selects all
static WNDPROC g_OldConvListProc = NULL;
static LRESULT CALLBACK ConvListSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessage(hwnd, LB_SETSEL, TRUE, (LPARAM)-1);
        return 0;
    }
    return CallWindowProc(g_OldConvListProc, hwnd, msg, wParam, lParam);
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
        g_OldConvListProc = (WNDPROC)SetWindowLongPtr(hList, GWLP_WNDPROC, (LONG_PTR)ConvListSubProc);

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
        SetWindowTheme(hNorm, L"", L""); // themed checkboxes ignore WM_CTLCOLOR* — strip so dark mode text applies
        HWND hMeta = CreateWindow(L"BUTTON", L"Copy metadata tags",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 170, optY, 150, 18, hwnd, (HMENU)ID_CONV_METADATA, hInst, NULL);
        SendMessage(hMeta, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        SetWindowTheme(hMeta, L"", L"");
        SendMessage(hMeta, BM_SETCHECK, BST_CHECKED, 0);

        int py = 362;
        HWND hProg = CreateWindowEx(0, PROGRESS_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 8, py, cw - 16, 20, hwnd, (HMENU)ID_CONV_PROGRESS, hInst, NULL);
        SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

        HWND hStat = CreateWindow(L"STATIC", L"Ready. Add files and select an output folder to begin.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 8, 388, cw - 16, 30, hwnd, (HMENU)ID_CONV_STATUS, hInst, NULL);
        SendMessage(hStat, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        int bby2 = 420;
        HWND hStart = CreateWindow(L"BUTTON", L"Start Conversion",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 8, bby2, 200, 32, hwnd, (HMENU)ID_CONV_START, hInst, NULL);
        SendMessage(hStart, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
        HWND hClosC = CreateWindow(L"BUTTON", L"Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cw - 100, bby2, 92, 32, hwnd, (HMENU)ID_CONV_CLOSE, hInst, NULL);
        SendMessage(hClosC, WM_SETFONT, (WPARAM)g_fontUI, TRUE);

        // Pre-populate with current playlist (local files only — streams and
        // radio URLs can't be converted)
        g_convItems.clear();
        for (auto& t : g_playlist) {
            if (_wcsnicmp(t.path, L"http://", 7) == 0 || _wcsnicmp(t.path, L"https://", 8) == 0 ||
                _wcsnicmp(t.path, L"ftp://", 6) == 0)
                continue;
            ConvItem ci; wcsncpy_s(ci.path, t.path, _TRUNCATE);
            wcsncpy_s(ci.display, t.display, _TRUNCATE);
            g_convItems.push_back(ci);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ci.display);
        }
        wchar_t sb[64]; swprintf_s(sb, L"%d files loaded from current playlist.", (int)g_convItems.size());
        SetDlgItemText(hwnd, ID_CONV_STATUS, sb);

        DragAcceptFiles(hwnd, TRUE);  // enable drag & drop onto the convert window
        SetFocus(hList);              // keyboard (Ctrl+A, type-to-jump) works right away
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
                // Local files only — streams/radio URLs can't be converted
                if (_wcsnicmp(t.path, L"http://", 7) == 0 || _wcsnicmp(t.path, L"https://", 8) == 0 ||
                    _wcsnicmp(t.path, L"ftp://", 6) == 0)
                    continue;
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
            wchar_t folder[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Select Folder to Add", folder, MAX_PATH)) {
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
            wchar_t outDir[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Select Output Folder", outDir, MAX_PATH)) {
                wcsncpy_s(g_convOutDir, outDir, _TRUNCATE);
                SetDlgItemText(hwnd, ID_CONV_OUTDIR, g_convOutDir);
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
        case IDCANCEL:
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
        MessageBox(hwnd, sb, L"Convert",
            g_convAbort ? MB_ICONWARNING : MB_ICONINFORMATION);
        // Reset progress bar after the popup is dismissed
        SendMessage(GetDlgItem(hwnd, ID_CONV_PROGRESS), PBM_SETPOS, 0, 0);
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
        const wchar_t* desc = L"BillyPro Audio File";
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
            L"Select file types to always open with BillyPro:",
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
            MessageBox(hwnd, L"File associations updated.\n\nAudio files will now open with BillyPro when double-clicked.",
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
        L"BillyPro \u2014 File Associations", style, 0, 0, dw, dh, NULL, NULL, hInst, NULL);
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
//  SMTC  (System Media Transport Controls — lock screen / overlay)
// ============================================================
static void SmtcInit(HWND hwnd)
{
    try {
        auto interop = winrt::get_activation_factory<
            winrt::Windows::Media::SystemMediaTransportControls,
            ISystemMediaTransportControlsInterop>();
        if (!interop) return;

        winrt::Windows::Media::SystemMediaTransportControls smtc{ nullptr };
        HRESULT hr = interop->GetForWindow(
            hwnd,
            winrt::guid_of<winrt::Windows::Media::SystemMediaTransportControls>(),
            winrt::put_abi(smtc));
        if (FAILED(hr) || !smtc) return;

        smtc.IsEnabled(true);
        smtc.IsPlayEnabled(true);
        smtc.IsPauseEnabled(true);
        smtc.IsNextEnabled(true);
        smtc.IsPreviousEnabled(true);
        smtc.IsStopEnabled(true);
        smtc.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Closed);

        // Handle button presses from Windows
        g_smtcToken = smtc.ButtonPressed([](
            winrt::Windows::Media::SystemMediaTransportControls const&,
            winrt::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs const& args) {
            switch (args.Button()) {
            case winrt::Windows::Media::SystemMediaTransportControlsButton::Play:
            case winrt::Windows::Media::SystemMediaTransportControlsButton::Pause:
                PostMessage(g_hwnd, WM_COMMAND, IDM_PLAY_PLAYPAUSE, 0);
                break;
            case winrt::Windows::Media::SystemMediaTransportControlsButton::Next:
                PostMessage(g_hwnd, WM_COMMAND, IDM_PLAY_NEXT, 0);
                break;
            case winrt::Windows::Media::SystemMediaTransportControlsButton::Previous:
                PostMessage(g_hwnd, WM_COMMAND, IDM_PLAY_PREV, 0);
                break;
            case winrt::Windows::Media::SystemMediaTransportControlsButton::Stop:
                PostMessage(g_hwnd, WM_COMMAND, IDM_PLAY_STOP, 0);
                break;
            default: break;
            }
        });

        g_smtc = smtc;
    }
    catch (...) {
        // SMTC not available on this Windows version — silently ignore
    }
}

static void SmtcUpdatePlaybackStatus()
{
    if (!g_smtc) return;
    try {
        if (!currentStream)
            g_smtc.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Stopped);
        else if (BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING)
            g_smtc.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Playing);
        else if (BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PAUSED)
            g_smtc.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Paused);
        else
            g_smtc.PlaybackStatus(winrt::Windows::Media::MediaPlaybackStatus::Stopped);
    } catch (...) {}
}

static void SmtcUpdateMetadata()
{
    if (!g_smtc) return;
    try {
        auto updater = g_smtc.DisplayUpdater();
        updater.Type(winrt::Windows::Media::MediaPlaybackType::Music);
        auto props = updater.MusicProperties();

        // Internet radio: show the current ICY track (Artist / Title) on the
        // lock screen / media overlay, falling back to the station name.
        if (g_radioPlaying) {
            std::wstring title, artist;
            if (g_radioNowPlaying[0]) {
                const wchar_t* sep = wcsstr(g_radioNowPlaying, L" - ");
                if (sep) { artist.assign(g_radioNowPlaying, (size_t)(sep - g_radioNowPlaying)); title = sep + 3; }
                else     { title = g_radioNowPlaying; }
                if (artist.empty()) artist = g_radioStationName;
            } else {
                title = g_radioStationName[0] ? g_radioStationName : L"Internet Radio";
                artist = L"Internet Radio";
            }
            props.Title(title);
            props.Artist(artist);
            updater.Update();
            return;
        }

        if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
            // Parse display name: try "Artist - Title" format
            const wchar_t* display = g_playlist[g_currentIndex].display;
            const wchar_t* sep = wcsstr(display, L" - ");
            if (sep) {
                std::wstring artist(display, sep);
                std::wstring title(sep + 3);
                // Strip file extension from title
                size_t dot = title.rfind(L'.');
                if (dot != std::wstring::npos) title = title.substr(0, dot);
                props.Artist(artist);
                props.Title(title);
            } else {
                // Just filename, strip extension
                std::wstring title(display);
                size_t dot = title.rfind(L'.');
                if (dot != std::wstring::npos) title = title.substr(0, dot);
                props.Title(title);
                props.Artist(L"");
            }

            // Try to load album artwork from embedded ID3
            // Open a temporary BASS stream just for tag reading
            HSTREAM tagStream = BASS_StreamCreateFile(FALSE,
                g_playlist[g_currentIndex].path, 0, 0,
                BASS_UNICODE | BASS_STREAM_DECODE);
            if (tagStream) {
                const void* id3v2 = BASS_ChannelGetTags(tagStream, BASS_TAG_ID3V2);
                if (id3v2) {
                    // Find APIC frame and extract image data
                    const BYTE* data = (const BYTE*)id3v2;
                    if (memcmp(data, "ID3", 3) == 0) {
                        DWORD tagSize = ((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) |
                            ((data[8] & 0x7F) << 7) | (data[9] & 0x7F);
                        BYTE ver = data[3];
                        const BYTE* p = data + 10;
                        const BYTE* end = data + 10 + tagSize;
                        while (p + 10 < end) {
                            char fid[5] = {}; memcpy(fid, p, 4);
                            DWORD fsize;
                            if (ver >= 4) fsize = ((p[4] & 0x7F) << 21) | ((p[5] & 0x7F) << 14) | ((p[6] & 0x7F) << 7) | (p[7] & 0x7F);
                            else          fsize = (p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7];
                            p += 10;
                            if (p + fsize > end) break;
                            if (strcmp(fid, "APIC") == 0 && fsize > 4) {
                                const BYTE* fp = p;
                                fp++; // text encoding
                                const char* mimeStart = (const char*)fp;
                                while (fp < p + fsize && *fp) fp++;
                                if (fp < p + fsize) fp++; // skip null after mime
                                if (fp < p + fsize) fp++; // picture type
                                while (fp < p + fsize && *fp) fp++;
                                if (fp < p + fsize) fp++; // skip null after description
                                DWORD imgSize = (DWORD)(p + fsize - fp);
                                if (imgSize > 0) {
                                    // Determine content type
                                    winrt::hstring contentType = L"image/jpeg";
                                    if (strstr(mimeStart, "png")) contentType = L"image/png";

                                    // Create IRandomAccessStream from raw bytes
                                    auto ref = winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromUri(
                                        winrt::Windows::Foundation::Uri(L"ms-appx:///dummy"));
                                    // Use InMemoryRandomAccessStream instead
                                    winrt::Windows::Storage::Streams::InMemoryRandomAccessStream memStream;
                                    winrt::Windows::Storage::Streams::DataWriter writer(memStream);
                                    writer.WriteBytes(winrt::array_view<const uint8_t>(fp, fp + imgSize));
                                    writer.StoreAsync().get();
                                    writer.DetachStream();
                                    memStream.Seek(0);

                                    updater.Thumbnail(
                                        winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromStream(memStream));
                                }
                                break; // found artwork
                            }
                            p += fsize;
                        }
                    }
                }
                BASS_StreamFree(tagStream);
            }
        } else {
            props.Title(L"BillyPro");
            props.Artist(L"");
        }

        updater.Update();
    } catch (...) {}
}

static void SmtcCleanup()
{
    if (g_smtc) {
        try {
            g_smtc.ButtonPressed(g_smtcToken);
            g_smtc.IsEnabled(false);
        } catch (...) {}
        g_smtc = nullptr;
    }
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
#define ID_OPT_MODERN_SIZE   639
#define ID_OPT_REC_PATH      653
#define ID_OPT_REC_BROWSE    654
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
#define ID_OPT_SESSION_OFF    655
#define ID_OPT_SESSION_TRACK  656
#define ID_OPT_SESSION_SEEK   657
#define ID_OPT_DROP_LOADDIR   658
#define ID_OPT_DROP_LOADTREE  659
#define ID_OPT_REC_FORMAT     660
#define ID_OPT_SCROLL_PLAY    661
#define ID_OPT_COL_RELPATH    662
#define ID_OPT_HK_LIST        663
#define ID_OPT_HK_ASSIGN      664
#define ID_OPT_HK_RESET       665
#define ID_OPT_HK_RESETALL    666
#define ID_OPT_HK_STATUS      667
#define ID_OPT_PL_PATH        668
#define ID_OPT_PL_BROWSE      669
#define ID_OPT_SHAZ_LOCAL     670
#define ID_OPT_GLOBAL_LIMIT   671
#define ID_OPT_SETS_ENABLE    672
#define ID_OPT_SETS_SERVER    673
#define ID_OPT_SETS_TOKEN     674
#define ID_OPT_SETS_CHECK     675
#define ID_OPT_SETS_REGISTER  676
#define ID_OPT_SETS_STATUS    677
#define ID_OPT_SETS_DLDIR     678
#define ID_OPT_SETS_DLBROWSE  679

static HWND s_optPanels[8] = {};
static int  s_optPage = 0;

static LRESULT CALLBACK OptPanelProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wp; RECT rc; GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_brBg); return 1;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        // Child controls on the options panel (checkboxes, radios, group boxes)
        // must be colored by the panel itself because they are parented to this panel,
        // not directly to the options dialog. Transparent background removes the dark
        // rectangle/black text artifact in dark mode.
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        SetBkMode(dc, TRANSPARENT);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bgList);
        return (LRESULT)g_brList;
    }
    case WM_HSCROLL:
    case WM_COMMAND:
        return SendMessage(GetParent(hwnd), msg, wp, lp);
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

static void OptSwitchPage(int page)
{
    for (int i = 0; i < 8; i++)
        if (s_optPanels[i]) ShowWindow(s_optPanels[i], i == page ? SW_SHOW : SW_HIDE);
    s_optPage = page;
}

// ── Hotkeys page helpers ──────────────────────────────────────
static bool    s_hkCapturing = false;
static WNDPROC g_OldHkListProc = NULL;

// Format a binding like "Ctrl+Shift+N" (or "(none)") for display
static void HkFormatBinding(int i, wchar_t* out, int n)
{
    out[0] = 0;
    BYTE vk = g_hotkeys[i].vk;
    if (!vk) { wcsncpy_s(out, n, L"(none)", _TRUNCATE); return; }
    if (g_hotkeys[i].mods & 2) wcscat_s(out, n, L"Ctrl+");
    if (g_hotkeys[i].mods & 1) wcscat_s(out, n, L"Shift+");
    if (g_hotkeys[i].mods & 4) wcscat_s(out, n, L"Alt+");
    wchar_t keyName[64] = L"";
    UINT sc = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    switch (vk) { // extended keys need the extended bit for GetKeyNameText
    case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
    case VK_PRIOR: case VK_NEXT: case VK_HOME: case VK_END:
    case VK_INSERT: case VK_DELETE:
        sc |= 0x100; break;
    }
    if (!GetKeyNameText((LONG)(sc << 16), keyName, 64) || !keyName[0])
        swprintf_s(keyName, L"Key 0x%02X", vk);
    wcscat_s(out, n, keyName);
}

static void HkRefreshList(HWND hList)
{
    int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
    SendMessage(hList, WM_SETREDRAW, FALSE, 0);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < HK_COUNT; i++) {
        wchar_t bind[80]; HkFormatBinding(i, bind, 80);
        wchar_t line[192];
        swprintf_s(line, L"%s\t%s", g_hotkeys[i].name, bind);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)line);
    }
    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hList, NULL, TRUE);
    if (sel >= 0 && sel < HK_COUNT) SendMessage(hList, LB_SETCURSEL, sel, 0);
}

// Subclass: while capturing, the next key pressed becomes the binding
static LRESULT CALLBACK HkListSubProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (s_hkCapturing) {
        if (msg == WM_GETDLGCODE) return DLGC_WANTALLKEYS;
        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
            if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
                return 0; // modifier alone — keep waiting for the real key
            HWND panel = GetParent(hwnd);
            if (wParam == VK_ESCAPE) {
                s_hkCapturing = false;
                SetDlgItemText(panel, ID_OPT_HK_STATUS, L"Capture cancelled.");
                return 0;
            }
            int sel = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < HK_COUNT) {
                BYTE mods = 0;
                if (GetKeyState(VK_SHIFT) & 0x8000)   mods |= 1;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
                if (GetKeyState(VK_MENU) & 0x8000)    mods |= 4;
                // Unbind any other action using the same combo (no duplicates)
                for (int i = 0; i < HK_COUNT; i++)
                    if (i != sel && g_hotkeys[i].vk == (BYTE)wParam && g_hotkeys[i].mods == mods)
                        { g_hotkeys[i].vk = 0; g_hotkeys[i].mods = 0; }
                g_hotkeys[sel].vk = (BYTE)wParam;
                g_hotkeys[sel].mods = mods;
                s_hkCapturing = false;
                HkRefreshList(hwnd);
                SetDlgItemText(panel, ID_OPT_HK_STATUS, L"Assigned. Click Save to keep changes.");
            } else {
                s_hkCapturing = false;
            }
            return 0;
        }
        if (msg == WM_CHAR || msg == WM_SYSCHAR) return 0;
    }
    if (msg == WM_LBUTTONDBLCLK) {
        // Double-click an action = start key capture for it
        LRESULT r = CallWindowProc(g_OldHkListProc, hwnd, msg, wParam, lParam);
        int sel = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0);
        if (sel >= 0 && sel < HK_COUNT) {
            s_hkCapturing = true;
            SetDlgItemText(GetParent(hwnd), ID_OPT_HK_STATUS,
                L"Press the new key combination... (Esc to cancel)");
            SetFocus(hwnd);
        }
        return r;
    }
    return CallWindowProc(g_OldHkListProc, hwnd, msg, wParam, lParam);
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
            wcp.hbrBackground = NULL;
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
            SetWindowTheme(hC, L"", L""); // allow WM_CTLCOLOR* to control checkbox text/background
            SendMessage(hC, BM_SETCHECK, v ? BST_CHECKED : BST_UNCHECKED, 0);
            return hC;
        };
        auto rad = [&](HWND par, int x, int y, int w, int h, WORD id, const wchar_t* t, bool v) -> HWND {
            HWND hR = CreateWindow(L"BUTTON", t,
                WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|WS_TABSTOP,
                x, y, w, h, par, (HMENU)(UINT_PTR)id, hInst, NULL);
            SendMessage(hR, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            SetWindowTheme(hR, L"", L""); // allow WM_CTLCOLOR* to control radio text/background
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
            SetWindowTheme(hG, L"", L""); // keep groupbox caption readable in dark mode
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
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Hotkeys");
        SendMessage(hNav, LB_ADDSTRING, 0, (LPARAM)L"Sets.LIVE");
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

            grp(p, 0, 0, w, 108, L"Open BillyPro with these file types");
            chk(p, 10, 20, 80, 20, ID_OPT_FT_WAV,  L"WAV",  false);
            chk(p, 95, 20, 80, 20, ID_OPT_FT_MP3,  L"MP3",  false);
            chk(p, 180, 20, 80, 20, ID_OPT_FT_OGG,  L"OGG",  false);
            chk(p, 10, 42, 80, 20, ID_OPT_FT_FLAC, L"FLAC", false);
            chk(p, 95, 42, 80, 20, ID_OPT_FT_M3U,  L"M3U",  false);
            btn(p, 10, 68, w - 20, 26, ID_OPT_FILEASSOC, L"Associate selected types with BillyPro");

            grp(p, 0, 118, w, 124, L"General");
            chk(p, 10, 136, w - 20, 20, ID_OPT_MULTIINST,
                L"Allow multiple instances of BillyPro", g_multiInst);
            chk(p, 10, 156, w - 20, 20, ID_OPT_SCROLL_PLAY,
                L"Scroll to playing track on track change", g_scrollToPlaying);
            chk(p, 10, 176, w - 20, 20, ID_OPT_SHAZ_LOCAL,
                L"Allow Song Search (Shazam) on local files", g_shazAllowLocal);
            lbl(p, 10, 202, 160, 16, L"Arrow key seek (seconds):");
            {
                wchar_t sbuf[16]; swprintf_s(sbuf, L"%.0f", g_seekStep);
                edt(p, 174, 200, 50, 20, ID_OPT_SEEK_STEP, sbuf);
            }

            grp(p, 0, 250, w, 112, L"Drag && Drop");
            { HWND h = rad(p, 10, 268, w - 20, 20, ID_OPT_DROP_APPEND,
                L"Add dropped files to current playlist", g_dropAppend);
              SetWindowLongPtr(h, GWL_STYLE, GetWindowLongPtr(h, GWL_STYLE) | WS_GROUP); }
            rad(p, 10, 288, w - 20, 20, ID_OPT_DROP_REPLACE,
                L"Replace playlist with dropped files", !g_dropAppend);
            chk(p, 10, 310, w - 20, 20, ID_OPT_DROP_LOADDIR,
                L"Load entire folder when dropping a file", g_dropLoadDir);
            chk(p, 10, 330, w - 20, 20, ID_OPT_DROP_LOADTREE,
                L"Load entire directory tree (include subfolders)", g_dropLoadTree);

            grp(p, 0, 372, w, 92, L"Remember Session");
            { HWND h = rad(p, 10, 390, w - 20, 20, ID_OPT_SESSION_OFF,
                L"Don't remember (default)", g_rememberSession == 0);
              SetWindowLongPtr(h, GWL_STYLE, GetWindowLongPtr(h, GWL_STYLE) | WS_GROUP); }
            rad(p, 10, 410, w - 20, 20, ID_OPT_SESSION_TRACK,
                L"Remember playlist and track", g_rememberSession == 1);
            rad(p, 10, 430, w - 20, 20, ID_OPT_SESSION_SEEK,
                L"Remember playlist, track and seek position", g_rememberSession == 2);
        }

        // ── Page 1: Display ───────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[1] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 52, L"Theme");
            chk(p, 10, 18, w - 20, 24, ID_OPT_DARKMODE, L"Dark Mode", g_darkMode);

            grp(p, 0, 60, w, 74, L"Modern Style (Artwork View)");
            lbl(p, 10, 80, 130, 16, L"Thumbnail size:");
            {
                HWND hSzCombo = CreateWindow(L"COMBOBOX", NULL,
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                    144, 78, 150, 100, p, (HMENU)(UINT_PTR)ID_OPT_MODERN_SIZE, hInst, NULL);
                SendMessage(hSzCombo, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
                SendMessage(hSzCombo, CB_ADDSTRING, 0, (LPARAM)L"Small (32px)");
                SendMessage(hSzCombo, CB_ADDSTRING, 0, (LPARAM)L"Medium (48px)");
                SendMessage(hSzCombo, CB_ADDSTRING, 0, (LPARAM)L"Large (64px)");
                SendMessage(hSzCombo, CB_SETCURSEL, g_modernSize, 0);
            }
            lbl(p, 10, 106, w - 20, 16, L"Enable Modern Style from View menu to show artwork.");

            grp(p, 0, 142, w, 52, L"Column View");
            chk(p, 10, 160, w - 20, 20, ID_OPT_COL_RELPATH,
                L"Show relative file paths (File path column)", g_colPathRelative);
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
            lbl(p, 20, 202, w - 30, 16, L"Width 0 = mono reverb, 100 = full stereo.");

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

            chk(p, 10, 474, w - 20, 20, ID_OPT_GLOBAL_LIMIT,
                L"Master Limiter (prevent clipping)", g_globalLimiter);
        }

        // ── Page 3: Device ────────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[3] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 168, L"Audio Output");   // tall enough for the Sound details lines
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

            // Recording settings
            grp(p, 0, 180, w, 112, L"Recording");
            lbl(p, 10, 202, 70, 16, L"Save path:");
            edt(p, 82, 200, w - 126, 22, ID_OPT_REC_PATH, g_recSaveDir);
            btn(p, w - 40, 200, 36, 22, ID_OPT_REC_BROWSE, L"...");
            lbl(p, 10, 228, 54, 16, L"Format:");
            {
                HWND hFmtCombo = CreateWindow(L"COMBOBOX", NULL,
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                    66, 226, 130, 80, p, (HMENU)(UINT_PTR)ID_OPT_REC_FORMAT, hInst, NULL);
                SendMessage(hFmtCombo, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
                SendMessage(hFmtCombo, CB_ADDSTRING, 0, (LPARAM)L"FLAC (lossless)");
                SendMessage(hFmtCombo, CB_ADDSTRING, 0, (LPARAM)L"WAV (uncompressed)");
                SendMessage(hFmtCombo, CB_SETCURSEL, g_recFormat, 0);
            }
            lbl(p, 10, 252, w - 20, 32, L"FLAC = smaller file, lossless. WAV = larger, universal.\r\nLeave path empty to ask each time.");
        }

        // ── Page 4: Media ─────────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[4] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 200, L"Media Folders");   // tall enough for the ★ hint line
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
            lbl(p, 10, 176, w - 20, 16, L"Press the \u2605 button on the main toolbar to browse.");

            grp(p, 0, 210, w, 108, L"Playlists Folder");
            lbl(p, 10, 230, w - 20, 16, L"Where .bpp playlists are saved (empty = default):");
            {
                wchar_t curPl[MAX_PATH] = {};
                GetIniPath();
                GetPrivateProfileString(L"Paths", L"PlaylistsDir", L"", curPl, MAX_PATH, g_iniPath);
                edt(p, 10, 248, w - 60, 22, ID_OPT_PL_PATH, curPl);
            }
            btn(p, w - 46, 248, 36, 22, ID_OPT_PL_BROWSE, L"...");
            lbl(p, 10, 278, w - 20, 32,
                L"New playlists are created here automatically.\n"
                L"Default: %APPDATA%\\BillyPro\\library\\playlists");
        }

        // ── Page 5: Timestretch ───────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[5] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 164, L"Pitch / Speed (CDJ-style)");   // tall enough for both hint lines
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

            {
                // Align labels with the slider channel (10 .. 10+sw, ~13px thumb margins)
                int sw = w - 80;
                lbl(p, 10 + 13 - 10, 80, 26, 14, L"-12");
                lbl(p, 10 + sw / 2 - 4, 80, 16, 14, L"0");
                lbl(p, 10 + sw - 13 - 11, 80, 28, 14, L"+12");
            }
            lbl(p, 10, 100, w - 20, 32,
                L"Negative = lower pitch/slower.  Positive = higher pitch/faster.\n"
                L"Move slider or type a value (e.g. -8.4).");
            lbl(p, 10, 138, w - 20, 16, L"Time display updates to reflect changed duration.");
        }

        // ── Page 6: Hotkeys ───────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[6] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 392, L"Keyboard Shortcuts");   // tall enough for the fixed-keys note
            lbl(p, 10, 20, w - 20, 30,
                L"Double-click an action (or select it and click Assign Key),\n"
                L"then press the new key combination.");
            HWND hHkList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_TABSTOP
                | LBS_USETABSTOPS | LBS_NOINTEGRALHEIGHT,
                10, 54, w - 20, 250, p, (HMENU)ID_OPT_HK_LIST, hInst, NULL);
            SendMessage(hHkList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            { int tab = 108; SendMessage(hHkList, LB_SETTABSTOPS, 1, (LPARAM)&tab); }
            g_OldHkListProc = (WNDPROC)SetWindowLongPtr(hHkList, GWLP_WNDPROC, (LONG_PTR)HkListSubProc);
            HkRefreshList(hHkList);
            btn(p, 10,  312, 100, 24, ID_OPT_HK_ASSIGN,   L"Assign Key...");
            btn(p, 116, 312, 80,  24, ID_OPT_HK_RESET,    L"Reset");
            btn(p, 202, 312, 90,  24, ID_OPT_HK_RESETALL, L"Reset All");
            HWND hHkStatus = CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 344, w - 20, 16, p, (HMENU)ID_OPT_HK_STATUS, hInst, NULL);
            SendMessage(hHkStatus, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            lbl(p, 10, 368, w - 20, 16, L"Ctrl+Up/Down (volume) and Left/Right (seek) are fixed.");
        }

        // ── Page 7: Sets.LIVE ─────────────────────────────────────────────
        {
            HWND p = mkPanel(false);
            s_optPanels[7] = p;
            int w = PNL_W - 4;
            grp(p, 0, 0, w, 220, L"Sets.LIVE Integration");
            chk(p, 10, 20, w - 20, 20, ID_OPT_SETS_ENABLE,
                L"Enable Sets.LIVE integration (adds it to the Library menu)", g_setsEnabled);
            lbl(p, 10, 50, 70, 16, L"Server:");
            edt(p, 84, 48, w - 94, 22, ID_OPT_SETS_SERVER, g_setsServer);
            lbl(p, 10, 78, 70, 16, L"Token:");
            {
                HWND hE = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", g_setsToken,
                    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD | WS_TABSTOP,
                    84, 76, w - 94, 22, p, (HMENU)(UINT_PTR)ID_OPT_SETS_TOKEN, hInst, NULL);
                SendMessage(hE, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            }
            btn(p, 10, 106, 120, 26, ID_OPT_SETS_CHECK,    L"Validate Token");
            btn(p, 138, 106, 150, 26, ID_OPT_SETS_REGISTER, L"Get a Token (Register)");
            HWND hSt = CreateWindow(L"STATIC",
                g_setsExpiry[0] ? L"" : L"Not validated yet.",
                WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX, 10, 140, w - 20, 20,
                p, (HMENU)(UINT_PTR)ID_OPT_SETS_STATUS, hInst, NULL);
            SendMessage(hSt, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            lbl(p, 10, 168, 100, 16, L"Download folder:");
            edt(p, 112, 166, w - 158, 22, ID_OPT_SETS_DLDIR, g_setsDownloadDir);
            btn(p, w - 42, 166, 36, 22, ID_OPT_SETS_DLBROWSE, L"...");
            lbl(p, 10, 196, w - 20, 16,
                L"Right-click a file in the Sets.LIVE browser to Download it here.");
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
                if (sel >= 0 && sel < 8) OptSwitchPage(sel);
            }
            break;

        case ID_OPT_HK_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE && s_hkCapturing && s_optPanels[6]) {
                // Changing selection cancels a pending capture
                s_hkCapturing = false;
                SetDlgItemText(s_optPanels[6], ID_OPT_HK_STATUS, L"");
            }
            break;
        case ID_OPT_HK_ASSIGN: {
            if (!s_optPanels[6]) break;
            HWND hL = GetDlgItem(s_optPanels[6], ID_OPT_HK_LIST);
            if ((int)SendMessage(hL, LB_GETCURSEL, 0, 0) == LB_ERR) {
                SetDlgItemText(s_optPanels[6], ID_OPT_HK_STATUS, L"Select an action first.");
                break;
            }
            s_hkCapturing = true;
            SetDlgItemText(s_optPanels[6], ID_OPT_HK_STATUS,
                L"Press the new key combination... (Esc to cancel)");
            SetFocus(hL);
            break;
        }
        case ID_OPT_HK_RESET: {
            if (!s_optPanels[6]) break;
            HWND hL = GetDlgItem(s_optPanels[6], ID_OPT_HK_LIST);
            int sel = (int)SendMessage(hL, LB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < HK_COUNT) {
                g_hotkeys[sel].vk = g_hotkeys[sel].defVk;
                g_hotkeys[sel].mods = g_hotkeys[sel].defMods;
                HkRefreshList(hL);
                SetDlgItemText(s_optPanels[6], ID_OPT_HK_STATUS, L"Reset to default.");
            }
            break;
        }
        case ID_OPT_HK_RESETALL: {
            if (!s_optPanels[6]) break;
            for (int i = 0; i < HK_COUNT; i++) {
                g_hotkeys[i].vk = g_hotkeys[i].defVk;
                g_hotkeys[i].mods = g_hotkeys[i].defMods;
            }
            HkRefreshList(GetDlgItem(s_optPanels[6], ID_OPT_HK_LIST));
            SetDlgItemText(s_optPanels[6], ID_OPT_HK_STATUS, L"All shortcuts reset to defaults.");
            break;
        }
        case ID_OPT_SETS_DLBROWSE: {
            wchar_t folder[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Choose Sets.LIVE download folder", folder, MAX_PATH) && s_optPanels[7])
                SetDlgItemText(s_optPanels[7], ID_OPT_SETS_DLDIR, folder);
            break;
        }
        case ID_OPT_SETS_REGISTER: {
            if (!s_optPanels[7]) break;
            wchar_t srv[128] = {};
            GetDlgItemText(s_optPanels[7], ID_OPT_SETS_SERVER, srv, _countof(srv));
            // strip scheme
            const wchar_t* h = srv;
            if (_wcsnicmp(h, L"https://", 8) == 0) h += 8; else if (_wcsnicmp(h, L"http://", 7) == 0) h += 7;
            if (!h[0]) h = L"app.sets.live";
            wchar_t url[256]; swprintf_s(url, L"https://%s/register.php", h);
            ShellExecute(hwnd, L"open", url, NULL, NULL, SW_SHOW);
            break;
        }
        case ID_OPT_SETS_CHECK: {
            if (!s_optPanels[7]) break;
            // Apply the currently-typed server + token, then query whoami
            GetDlgItemText(s_optPanels[7], ID_OPT_SETS_SERVER, g_setsServer, _countof(g_setsServer));
            GetDlgItemText(s_optPanels[7], ID_OPT_SETS_TOKEN, g_setsToken, _countof(g_setsToken));
            if (!g_setsToken[0]) { SetDlgItemText(s_optPanels[7], ID_OPT_SETS_STATUS, L"Enter a token first."); break; }
            SetDlgItemText(s_optPanels[7], ID_OPT_SETS_STATUS, L"Validating...");
            wchar_t user[64] = {};
            int st = SetsValidateToken(user, 64);
            wchar_t msg[200];
            if (st == 200)
                swprintf_s(msg, L"✓ Valid. User: %s   Expires: %s", user[0] ? user : L"?", g_setsExpiry[0] ? g_setsExpiry : L"?");
            else if (st == 401) wcscpy_s(msg, L"✗ Invalid or expired token.");
            else if (st == 429) wcscpy_s(msg, L"Rate limited — wait a few minutes.");
            else if (st == 0)   wcscpy_s(msg, L"Cannot reach the server. Check the address.");
            else                swprintf_s(msg, L"Server error (HTTP %d).", st);
            SetDlgItemText(s_optPanels[7], ID_OPT_SETS_STATUS, msg);
            break;
        }
        case ID_OPT_MEDIA_ADD: {
            wchar_t folder[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Select Media Folder", folder, MAX_PATH)) {
                // Avoid duplicates
                bool dup = false;
                for (auto& f : g_mediaFolders) if (_wcsicmp(f.c_str(), folder) == 0) { dup = true; break; }
                if (!dup) {
                    g_mediaFolders.push_back(folder);
                    wcsncpy_s(g_mediaFolder, g_mediaFolders[0].c_str(), _TRUNCATE);
                    if (s_optPanels[4]) {
                        HWND hList = GetDlgItem(s_optPanels[4], ID_OPT_MEDIA_LIST);
                        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)folder);
                    }
                    LayoutControls(g_hwnd);
                }
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
            if (count > 0) MessageBox(hwnd, L"File associations updated.", L"BillyPro", MB_OK|MB_ICONINFORMATION);
            else MessageBox(hwnd, L"Select at least one file type first.", L"BillyPro", MB_OK|MB_ICONWARNING);
            break;
        }

        case ID_OPT_REC_BROWSE: {
            wchar_t folder[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Select Recording Save Folder", folder, MAX_PATH)) {
                SetDlgItemText(s_optPanels[3], ID_OPT_REC_PATH, folder);
            }
            break;
        }
        case ID_OPT_PL_BROWSE: {
            wchar_t folder[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Select Playlists Folder", folder, MAX_PATH) && s_optPanels[4]) {
                SetDlgItemText(s_optPanels[4], ID_OPT_PL_PATH, folder);
            }
            break;
        }
        case ID_OPT_DEVICE_RESET: {
            if (!s_optPanels[3]) break;
            HWND hCombo = GetDlgItem(s_optPanels[3], ID_OPT_DEVICE_COMBO);
            int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            if (sel == CB_ERR) break;
            int devIdx = (int)SendMessage(hCombo, CB_GETITEMDATA, sel, 0);
            // Save playback state before switching
            int savedIdx = g_currentIndex;
            double savedPos = GetPlayPos();
            bool wasPlaying = currentStream && BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING;
            // Stop everything cleanly
            StopAudio();
            // Switch device
            BASS_Free();
            if (!BASS_Init(devIdx, 44100, 0, g_hwnd, NULL)) {
                // Fallback to default device
                BASS_Init(-1, 44100, 0, g_hwnd, NULL);
                MessageBox(hwnd, L"Failed to switch device. Using default.", L"Device", MB_ICONWARNING);
            }
            // Restart playback at saved position
            if (savedIdx >= 0 && savedIdx < (int)g_playlist.size()) {
                PlayIndex(savedIdx);
                if (savedPos > 1.0) SeekToSeconds(savedPos);
                if (!wasPlaying && currentStream) {
                    BASS_ChannelPause(currentStream);
                    KillTimer(g_hwnd, IDT_PLAYBACK);
                    UpdatePlayBtn();
                }
            }
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
            g_globalLimiter = (SendMessage(GetDlgItem(s_optPanels[2], ID_OPT_GLOBAL_LIMIT), BM_GETCHECK,0,0)==BST_CHECKED);
            // Sets.LIVE page
            if (s_optPanels[7]) {
                g_setsEnabled = (SendMessage(GetDlgItem(s_optPanels[7], ID_OPT_SETS_ENABLE), BM_GETCHECK,0,0)==BST_CHECKED);
                GetDlgItemText(s_optPanels[7], ID_OPT_SETS_SERVER, g_setsServer, _countof(g_setsServer));
                GetDlgItemText(s_optPanels[7], ID_OPT_SETS_TOKEN, g_setsToken, _countof(g_setsToken));
                GetDlgItemText(s_optPanels[7], ID_OPT_SETS_DLDIR, g_setsDownloadDir, _countof(g_setsDownloadDir));
                if (!g_setsServer[0]) wcscpy_s(g_setsServer, L"app.sets.live");
                UpdateSetsLibMenu();
                DrawMenuBar(g_hwnd);
            }
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
            g_colPathRelative = (SendMessage(GetDlgItem(s_optPanels[1], ID_OPT_COL_RELPATH), BM_GETCHECK,0,0)==BST_CHECKED);
            if (g_columnView && hListBox) InvalidateRect(hListBox, NULL, FALSE);
            {
                int newSize = (int)SendMessage(GetDlgItem(s_optPanels[1], ID_OPT_MODERN_SIZE), CB_GETCURSEL, 0, 0);
                if (newSize >= 0 && newSize <= 2 && newSize != g_modernSize) {
                    g_modernSize = newSize;
                    // Clear cached thumbnails so they regenerate at new size
                    ClearThumbCache();
                    if (g_thumbPlaceholder) { DeleteObject(g_thumbPlaceholder); g_thumbPlaceholder = NULL; }
                    if (g_modernStyle) {
                        g_thumbPlaceholder = CreatePlaceholderBitmap();
                        RecreateListbox();
                    }
                }
            }
            // Setup page
            g_multiInst  = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_MULTIINST),    BM_GETCHECK,0,0)==BST_CHECKED);
            g_scrollToPlaying = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_SCROLL_PLAY), BM_GETCHECK,0,0)==BST_CHECKED);
            g_shazAllowLocal = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_SHAZ_LOCAL), BM_GETCHECK,0,0)==BST_CHECKED);
            g_dropAppend = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_DROP_APPEND),  BM_GETCHECK,0,0)==BST_CHECKED);
            g_dropLoadDir = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_DROP_LOADDIR), BM_GETCHECK,0,0)==BST_CHECKED);
            g_dropLoadTree = (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_DROP_LOADTREE), BM_GETCHECK,0,0)==BST_CHECKED);
            { wchar_t sb[16]; GetDlgItemText(s_optPanels[0], ID_OPT_SEEK_STEP, sb, 16);
              float sv = (float)_wtof(sb); if (sv >= 1.0f) g_seekStep = sv; }
            // Remember session
            if (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_SESSION_SEEK), BM_GETCHECK,0,0)==BST_CHECKED)
                g_rememberSession = 2;
            else if (SendMessage(GetDlgItem(s_optPanels[0], ID_OPT_SESSION_TRACK), BM_GETCHECK,0,0)==BST_CHECKED)
                g_rememberSession = 1;
            else
                g_rememberSession = 0;
            // Device page
            if (s_optPanels[3])
                g_rememberVolume = (SendMessage(GetDlgItem(s_optPanels[3], ID_OPT_REMEMBER_VOL), BM_GETCHECK,0,0)==BST_CHECKED);
            // Recording save path
            if (s_optPanels[3])
                GetDlgItemText(s_optPanels[3], ID_OPT_REC_PATH, g_recSaveDir, MAX_PATH);
                int recFmt = (int)SendMessage(GetDlgItem(s_optPanels[3], ID_OPT_REC_FORMAT), CB_GETCURSEL, 0, 0);
                if (recFmt >= 0 && recFmt <= 1) g_recFormat = recFmt;
            // Playlists folder override — takes effect immediately
            if (s_optPanels[4]) {
                wchar_t plPath[MAX_PATH] = {};
                GetDlgItemText(s_optPanels[4], ID_OPT_PL_PATH, plPath, MAX_PATH);
                GetIniPath();
                WritePrivateProfileString(L"Paths", L"PlaylistsDir", plPath, g_iniPath);
                g_libDir[0] = L'\0';   // force library dirs to re-resolve
                ScanPlaylists();
            }
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
            // Auto-switch audio device if changed
            if (s_optPanels[3]) {
                HWND hCombo = GetDlgItem(s_optPanels[3], ID_OPT_DEVICE_COMBO);
                int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    int devIdx = (int)SendMessage(hCombo, CB_GETITEMDATA, sel, 0);
                    if ((DWORD)devIdx != BASS_GetDevice()) {
                        int savedIdx = g_currentIndex;
                        double savedPos = GetPlayPos();
                        bool wasPlaying = currentStream && BASS_ChannelIsActive(currentStream) == BASS_ACTIVE_PLAYING;
                        StopAudio();
                        BASS_Free();
                        if (!BASS_Init(devIdx, 44100, 0, g_hwnd, NULL))
                            BASS_Init(-1, 44100, 0, g_hwnd, NULL);
                        if (savedIdx >= 0 && savedIdx < (int)g_playlist.size()) {
                            PlayIndex(savedIdx);
                            if (savedPos > 1.0) SeekToSeconds(savedPos);
                            if (!wasPlaying && currentStream) {
                                BASS_ChannelPause(currentStream);
                                KillTimer(g_hwnd, IDT_PLAYBACK);
                                UpdatePlayBtn();
                            }
                        }
                    }
                }
            }
            SaveSettings();
            DestroyWindow(hwnd);
            break;
        }
        case ID_OPT_CANCEL:
        case IDCANCEL:
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
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, g_theme.text);
        SetBkColor(dc, g_theme.bg);
        SetBkMode(dc, TRANSPARENT);
        return (LRESULT)g_brBg;
    }
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
        SetBkMode(dc, TRANSPARENT);
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
        L"Convert Audio",
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
        case ID_SEARCH_OK:
        case IDOK:
            SearchPlaySelected(); break;
        case ID_SEARCH_SAVEPL: {
            if (g_searchResults.empty()) {
                MessageBox(hwnd, L"No search results to save.", L"Save as Playlist", MB_ICONWARNING);
                break;
            }
            GetLibDir();
            // Use search query as default filename
            wchar_t defName[MAX_PATH];
            wchar_t query[128] = L"Search Results";
            if (hSearchEdit) GetWindowText(hSearchEdit, query, 128);
            // Sanitize query for filename
            for (wchar_t* p = query; *p; p++)
                if (*p==L'\\'||*p==L'/'||*p==L':'||*p==L'*'||*p==L'?'||*p==L'"'||*p==L'<'||*p==L'>'||*p==L'|') *p=L'_';
            swprintf_s(defName, L"%s\\%s.bpp", g_plDir, query);
            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = L"Billy Pro Playlist (*.bpp)\0*.bpp\0";
            ofn.lpstrFile = defName;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrInitialDir = g_plDir;
            ofn.lpstrTitle = L"Save Search Results as Playlist";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
            ofn.lpstrDefExt = L"bpp";
            if (GetSaveFileName(&ofn)) {
                FILE* f = _wfopen(defName, L"w,ccs=UTF-8");
                if (f) {
                    for (int idx : g_searchResults) {
                        if (idx >= 0 && idx < (int)g_playlist.size())
                            fwprintf(f, L"%s\n", g_playlist[idx].path);
                    }
                    fclose(f);
                    ScanPlaylists();
                    wchar_t msg[128];
                    swprintf_s(msg, L"Saved %d track(s) to playlist.", (int)g_searchResults.size());
                    MessageBox(hwnd, msg, L"Save as Playlist", MB_ICONINFORMATION);
                }
            }
            break;
        }
        case ID_SEARCH_CANCEL:
        case IDCANCEL:
            DestroyWindow(hwnd); break;
        }
        break;
    case WM_SIZE: {
        RECT rc; GetClientRect(hwnd, &rc);
        int cw = rc.right, ch = rc.bottom;
        if (hSearchEdit) MoveWindow(hSearchEdit, 8, 8, cw - 16, 26, TRUE);
        if (hSearchList) MoveWindow(hSearchList, 8, 42, cw - 16, ch - 42 - 46, TRUE);
        HWND hOK   = GetDlgItem(hwnd, ID_SEARCH_OK);
        HWND hSave = GetDlgItem(hwnd, ID_SEARCH_SAVEPL);
        HWND hCan  = GetDlgItem(hwnd, ID_SEARCH_CANCEL);
        int bh = 28, by = ch - 38;
        int totalW = 88 + 120 + 88 + 16; // 3 buttons + gaps
        int bx = (cw - totalW) / 2;
        if (hOK)   MoveWindow(hOK,   bx, by, 88, bh, TRUE); bx += 92;
        if (hSave) MoveWindow(hSave, bx, by, 120, bh, TRUE); bx += 124;
        if (hCan)  MoveWindow(hCan,  bx, by, 88, bh, TRUE);
        break;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
        if (wParam == VK_RETURN) { SearchPlaySelected(); return 0; }
        if (wParam == VK_DELETE) { SearchDeleteSelected(); return 0; }
        break;
    case WM_MEASUREITEM: {
        MEASUREITEMSTRUCT* ms = (MEASUREITEMSTRUCT*)lParam;
        if (ms->CtlID == ID_SEARCH_LIST) { ms->itemHeight = 20; return TRUE; }
        break;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID == ID_SEARCH_LIST) {
            int idx = (int)dis->itemID;
            int count = (int)SendMessage(dis->hwndItem, LB_GETCOUNT, 0, 0);
            HDC dc = dis->hDC;
            RECT rc = dis->rcItem;
            if (idx < 0 || idx >= count) {
                HBRUSH bg = CreateSolidBrush(g_theme.bgList);
                FillRect(dc, &rc, bg); DeleteObject(bg);
                return TRUE;
            }
            bool selected = (dis->itemState & ODS_SELECTED) != 0;
            bool isPlaying = (idx < (int)g_searchResults.size() && g_currentIndex >= 0
                && g_searchResults[idx] == g_currentIndex);
            COLORREF bgCol, txCol;
            if (g_darkMode) {
                bgCol = selected ? RGB(30, 80, 160) : (isPlaying ? RGB(35, 50, 45) : g_theme.bgList);
                txCol = selected ? RGB(255, 255, 255) : (isPlaying ? g_theme.accent : g_theme.text);
            } else {
                bgCol = selected ? GetSysColor(COLOR_HIGHLIGHT) : (isPlaying ? RGB(215, 235, 250) : GetSysColor(COLOR_WINDOW));
                txCol = selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : (isPlaying ? RGB(20, 80, 160) : GetSysColor(COLOR_WINDOWTEXT));
            }
            HBRUSH br = CreateSolidBrush(bgCol);
            FillRect(dc, &rc, br); DeleteObject(br);
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, txCol);
            HFONT oldF = (HFONT)SelectObject(dc, isPlaying ? g_fontBold : g_fontUI);
            // LB_GETTEXT has NO buffer bound — check the length first or a
            // long item (stream URL row) overruns the stack and crashes.
            wchar_t text[MAX_PATH + 8] = L"";
            if (SendMessage(dis->hwndItem, LB_GETTEXTLEN, idx, 0) < (LRESULT)_countof(text))
                SendMessage(dis->hwndItem, LB_GETTEXT, idx, (LPARAM)text);
            RECT tr = rc; tr.left += 4;
            if (isPlaying) {
                wchar_t buf[MAX_PATH + 4];
                swprintf_s(buf, L"\u25B6 %s", text);
                DrawText(dc, buf, -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
            } else {
                DrawText(dc, text, -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
            }
            SelectObject(dc, oldF);
            if (dis->itemState & ODS_FOCUS) DrawFocusRect(dc, &rc);
            return TRUE;
        }
        break;
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
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT
        | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
        8, 42, 344, 330, hDlg, (HMENU)ID_SEARCH_LIST, hInst, NULL);
    SendMessage(hSearchList, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hSearchList, LB_SETITEMHEIGHT, 0, 20);

    HWND hOK = CreateWindow(L"BUTTON", L"Play",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        0, 0, 88, 28, hDlg, (HMENU)ID_SEARCH_OK, hInst, NULL);
    HWND hSavePl = CreateWindow(L"BUTTON", L"Save as Playlist...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 120, 28, hDlg, (HMENU)ID_SEARCH_SAVEPL, hInst, NULL);
    HWND hCan = CreateWindow(L"BUTTON", L"Close",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 88, 28, hDlg, (HMENU)ID_SEARCH_CANCEL, hInst, NULL);
    SendMessage(hOK,     WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hSavePl, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    SendMessage(hCan,    WM_SETFONT, (WPARAM)g_fontUI, TRUE);

    // Apply dark mode theme to search listbox
    if (g_darkMode)
        SetWindowTheme(hSearchList, L"DarkMode_Explorer", NULL);

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

        // Column header repaint
        if (hColumnHeader) InvalidateRect(hColumnHeader, NULL, TRUE);
        if (g_columnView && hListBox) InvalidateRect(hListBox, NULL, FALSE);

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
//  Column View (playlist with sortable columns)
// ============================================================
//  Column View — header control + owner-draw listbox columns
// ============================================================
// ColID enum + g_colVisible + g_colWidth declared in forward section above
// Short labels for the header bar, long labels for the right-click menu
static const wchar_t* COL_HDR_LABELS[COL_COUNT] = {
    L"File name", L"Extension", L"Duration", L"File path", L"File size",
    L"Artist", L"Title", L"Album", L"#", L"Year",
    L"Genre", L"Bitrate", L"Sample rate", L"Channels"
};
static const wchar_t* COL_MENU_LABELS[COL_COUNT] = {
    L"File name", L"Extension", L"Duration", L"File path", L"File size",
    L"Artist", L"Title", L"Album", L"Track #", L"Year",
    L"Genre", L"Bitrate", L"Sample rate", L"Channels"
};
static const int COL_FMT[COL_COUNT] = {
    HDF_LEFT, HDF_LEFT, HDF_RIGHT, HDF_LEFT, HDF_RIGHT,
    HDF_LEFT, HDF_LEFT, HDF_LEFT, HDF_RIGHT, HDF_RIGHT,
    HDF_LEFT, HDF_RIGHT, HDF_RIGHT, HDF_LEFT
};

// Duration + metadata cache
static std::map<std::wstring, double> g_durCache;

struct TagCache {
    std::wstring artist, title, album, genre, year, track;
    int bitrate; int sampleRate; int channels;
    bool loaded;
};
static std::map<std::wstring, TagCache> g_tagCache;

static TagCache& GetTagsForFile(const wchar_t* path)
{
    std::wstring key(path);
    auto it = g_tagCache.find(key);
    if (it != g_tagCache.end()) return it->second;

    TagCache tc = {}; tc.loaded = true;
    HSTREAM s = BASS_StreamCreateFile(FALSE, path, 0, 0,
        BASS_UNICODE | BASS_STREAM_DECODE);
    if (s) {
        BASS_CHANNELINFO ci = {};
        BASS_ChannelGetInfo(s, &ci);
        tc.sampleRate = ci.freq;
        tc.channels = ci.chans;
        float br = 0; BASS_ChannelGetAttribute(s, BASS_ATTRIB_BITRATE, &br);
        tc.bitrate = (int)br;

        // Try OGG/FLAC vorbis comments first
        const char* tags = BASS_ChannelGetTags(s, BASS_TAG_OGG);
        if (tags) {
            for (const char* t = tags; *t; t += strlen(t) + 1) {
                wchar_t wval[256];
                auto readTag = [&](const char* name, std::wstring& out) {
                    int nl = (int)strlen(name);
                    if (_strnicmp(t, name, nl) == 0 && t[nl] == '=') {
                        MultiByteToWideChar(CP_UTF8, 0, t + nl + 1, -1, wval, 256);
                        out = wval;
                    }
                };
                readTag("ARTIST", tc.artist);
                readTag("TITLE", tc.title);
                readTag("ALBUM", tc.album);
                readTag("GENRE", tc.genre);
                readTag("DATE", tc.year);
                readTag("TRACKNUMBER", tc.track);
            }
        }
        // Try ID3v2 (MP3 — has track#, year, genre, etc.) if any field is still empty
        if (tc.artist.empty() || tc.title.empty() || tc.album.empty() ||
            tc.track.empty() || tc.year.empty() || tc.genre.empty()) {
            const void* id3v2 = BASS_ChannelGetTags(s, BASS_TAG_ID3V2);
            if (id3v2) {
                const BYTE* d = (const BYTE*)id3v2;
                if (memcmp(d, "ID3", 3) == 0) {
                    BYTE ver = d[3];
                    DWORD tagSz = ((d[6]&0x7F)<<21)|((d[7]&0x7F)<<14)|((d[8]&0x7F)<<7)|(d[9]&0x7F);
                    const BYTE* p = d + 10;
                    const BYTE* end = d + 10 + tagSz;
                    while (p + 10 < end) {
                        char fid[5] = {}; memcpy(fid, p, 4);
                        DWORD fsz;
                        if (ver >= 4) fsz = ((p[4]&0x7F)<<21)|((p[5]&0x7F)<<14)|((p[6]&0x7F)<<7)|(p[7]&0x7F);
                        else          fsz = (p[4]<<24)|(p[5]<<16)|(p[6]<<8)|p[7];
                        p += 10;
                        if (fsz == 0 || p + fsz > end) break;
                        // Read text frame: first byte = encoding, rest = string
                        auto readFrame = [&](const char* id, std::wstring& out) {
                            if (strcmp(fid, id) != 0 || !out.empty()) return;
                            if (fsz < 2) return;
                            BYTE enc = p[0];
                            wchar_t wbuf[512] = {};
                            if (enc == 0) // ISO-8859-1
                                MultiByteToWideChar(28591, 0, (const char*)p+1, (int)fsz-1, wbuf, 511);
                            else if (enc == 1 || enc == 2) { // UTF-16 LE/BE
                                const BYTE* txt = p + 1;
                                int txtLen = (int)fsz - 1;
                                if (txtLen >= 2 && txt[0] == 0xFF && txt[1] == 0xFE) { txt += 2; txtLen -= 2; } // skip BOM
                                else if (txtLen >= 2 && txt[0] == 0xFE && txt[1] == 0xFF) { txt += 2; txtLen -= 2; } // BE BOM
                                int wchars = txtLen / 2;
                                if (wchars > 511) wchars = 511;
                                memcpy(wbuf, txt, wchars * 2);
                                wbuf[wchars] = 0;
                            }
                            else if (enc == 3) // UTF-8
                                MultiByteToWideChar(CP_UTF8, 0, (const char*)p+1, (int)fsz-1, wbuf, 511);
                            // Trim trailing nulls/spaces
                            int wl = (int)wcslen(wbuf);
                            while (wl > 0 && (wbuf[wl-1] == 0 || wbuf[wl-1] == L' ')) wbuf[--wl] = 0;
                            if (wbuf[0]) out = wbuf;
                        };
                        readFrame("TPE1", tc.artist);
                        readFrame("TIT2", tc.title);
                        readFrame("TALB", tc.album);
                        readFrame("TRCK", tc.track);
                        readFrame("TYER", tc.year);
                        readFrame("TDRC", tc.year);  // ID3v2.4 date
                        readFrame("TCON", tc.genre);
                        p += fsz;
                    }
                }
            }
        }
        // Fallback: ID3v1
        if (tc.artist.empty()) {
            TAG_ID3* id3 = (TAG_ID3*)BASS_ChannelGetTags(s, BASS_TAG_ID3);
            if (id3) {
                wchar_t w[256];
                auto fromField = [&](const char* f, int len, std::wstring& out) {
                    if (out.empty() && f[0]) {
                        char tmp[64] = {}; memcpy(tmp, f, min(len, 63));
                        int l = (int)strlen(tmp); while (l > 0 && tmp[l-1] == ' ') tmp[--l] = 0;
                        if (tmp[0]) { MultiByteToWideChar(CP_ACP, 0, tmp, -1, w, 256); out = w; }
                    }
                };
                fromField(id3->artist, 30, tc.artist);
                fromField(id3->title, 30, tc.title);
                fromField(id3->album, 30, tc.album);
                fromField(id3->year, 4, tc.year);
            }
        }
        BASS_StreamFree(s);
    }
    // FLAC fallback: read vorbis comments via libFLAC metadata chain
    const wchar_t* fext = wcsrchr(path, L'.');
    if (fext && _wcsicmp(fext, L".flac") == 0 &&
        (tc.artist.empty() || tc.title.empty() || tc.album.empty())) {
        int uLen = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
        std::vector<char> u8(uLen > 0 ? uLen : 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, path, -1, u8.data(), uLen, NULL, NULL);
        FLAC__Metadata_Chain* fc = FLAC__metadata_chain_new();
        if (fc && FLAC__metadata_chain_read(fc, u8.data())) {
            FLAC__Metadata_Iterator* fi = FLAC__metadata_iterator_new();
            if (fi) {
                FLAC__metadata_iterator_init(fi, fc);
                do {
                    FLAC__StreamMetadata* blk = FLAC__metadata_iterator_get_block(fi);
                    if (blk && blk->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
                        const FLAC__StreamMetadata_VorbisComment& vc = blk->data.vorbis_comment;
                        for (FLAC__uint32 i = 0; i < vc.num_comments; i++) {
                            const char* e = (const char*)vc.comments[i].entry;
                            wchar_t wv[512] = {};
                            auto readVC = [&](const char* name, std::wstring& out) {
                                if (!out.empty()) return;
                                int nl = (int)strlen(name);
                                if (_strnicmp(e, name, nl) == 0 && e[nl] == '=') {
                                    MultiByteToWideChar(CP_UTF8, 0, e + nl + 1, -1, wv, 511);
                                    out = wv;
                                }
                            };
                            readVC("ARTIST", tc.artist);
                            readVC("TITLE", tc.title);
                            readVC("ALBUM", tc.album);
                            readVC("GENRE", tc.genre);
                            readVC("DATE", tc.year);
                            readVC("TRACKNUMBER", tc.track);
                        }
                    }
                } while (FLAC__metadata_iterator_next(fi));
                FLAC__metadata_iterator_delete(fi);
            }
        }
        if (fc) FLAC__metadata_chain_delete(fc);
    }
    g_tagCache[key] = tc;
    return g_tagCache[key];
}

static double QuickFileDuration(const wchar_t* path)
{
    if (_wcsnicmp(path, L"http://", 7) == 0 || _wcsnicmp(path, L"https://", 8) == 0)
        return 0;
    std::wstring key(path);
    auto it = g_durCache.find(key);
    if (it != g_durCache.end()) return it->second;
    HSTREAM s = BASS_StreamCreateFile(FALSE, path, 0, 0, BASS_UNICODE | BASS_STREAM_DECODE);
    double len = 0;
    if (s) { len = BASS_ChannelBytes2Seconds(s, BASS_ChannelGetLength(s, BASS_POS_BYTE)); BASS_StreamFree(s); }
    if (len < 0) len = 0;
    g_durCache[key] = len;
    return len;
}

static void SyncColumnView()
{
    if (g_columnView && hListBox) InvalidateRect(hListBox, NULL, FALSE);
}

// Rebuild the Win32 Header control items from g_colVisible/g_colOrder/g_colWidth
// Save current header item order + widths back to g_colOrder/g_colWidth
static void SaveColumnOrderFromHeader()
{
    if (!hColumnHeader) return;
    int n = Header_GetItemCount(hColumnHeader);
    // Build new order: visible columns in current header order, then hidden columns
    int newOrder[COL_COUNT];
    bool used[COL_COUNT] = {};
    int pos = 0;
    for (int i = 0; i < n && pos < COL_COUNT; i++) {
        HDITEM hdi = {}; hdi.mask = HDI_LPARAM | HDI_WIDTH;
        Header_GetItem(hColumnHeader, i, &hdi);
        int cid = (int)hdi.lParam;
        if (cid >= 0 && cid < COL_COUNT) {
            newOrder[pos++] = cid;
            used[cid] = true;
            g_colWidth[cid] = hdi.cxy;
        }
    }
    // Append hidden columns in their previous order
    for (int i = 0; i < COL_COUNT; i++) {
        int cid = g_colOrder[i];
        if (!used[cid]) newOrder[pos++] = cid;
    }
    memcpy(g_colOrder, newOrder, sizeof(g_colOrder));
}

static void RebuildColumnHeader()
{
    if (!hColumnHeader) return;
    // Save current arrangement before clearing
    if (Header_GetItemCount(hColumnHeader) > 0)
        SaveColumnOrderFromHeader();
    // Clear
    while (Header_GetItemCount(hColumnHeader) > 0)
        Header_DeleteItem(hColumnHeader, 0);
    // Insert visible columns in saved order
    int pos = 0;
    for (int i = 0; i < COL_COUNT; i++) {
        int cid = g_colOrder[i];
        if (!g_colVisible[cid]) continue;
        HDITEM hdi = {};
        hdi.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT | HDI_LPARAM;
        hdi.pszText = (LPWSTR)COL_HDR_LABELS[cid];
        hdi.cxy = g_colWidth[cid];
        hdi.fmt = COL_FMT[cid] | HDF_STRING;
        hdi.lParam = cid;
        Header_InsertItem(hColumnHeader, pos++, &hdi);
    }
}

// Get column rects (in listbox-relative coordinates) for painting
// Returns the number of visible columns, fills colRects[] and colIDs[]
static int GetColumnRects(RECT listRc, RECT* colRects, int* colIDs, int maxCols)
{
    if (!hColumnHeader) return 0;
    int n = Header_GetItemCount(hColumnHeader);
    if (n > maxCols) n = maxCols;
    int x = listRc.left;
    for (int i = 0; i < n; i++) {
        HDITEM hdi = {}; hdi.mask = HDI_WIDTH | HDI_LPARAM;
        Header_GetItem(hColumnHeader, i, &hdi);
        colRects[i] = listRc;
        colRects[i].left = x;
        colRects[i].right = x + hdi.cxy;
        colIDs[i] = (int)hdi.lParam;
        x += hdi.cxy;
    }
    return n;
}

// Get the text for a given column and track
static void GetColumnText(int colId, int trackIdx, const wchar_t* trackPath,
    const wchar_t* displayText, wchar_t* out, int maxLen)
{
    out[0] = 0;
    const wchar_t* raw = displayText;
    if (raw[0] == L'\u2605' && raw[1] == L' ') raw += 2;
    if (raw[0] == L'\u25B6' && raw[1] == L' ') raw += 2;
    bool isUrl = trackPath && (_wcsnicmp(trackPath, L"http", 4) == 0);

    switch (colId) {
    case COL_NAME: {
        wcsncpy_s(out, maxLen, raw, _TRUNCATE);
        // Only strip extension if the Extension column is visible
        if (g_colVisible[COL_EXT]) {
            wchar_t* dot = wcsrchr(out, L'.');
            if (dot) *dot = 0;
        }
        break;
    }
    case COL_EXT: {
        const wchar_t* dot = wcsrchr(raw, L'.');
        if (dot) wcsncpy_s(out, maxLen, dot + 1, _TRUNCATE);
        break;
    }
    case COL_DURATION: {
        if (!trackPath) break;
        if (isUrl) { wcsncpy_s(out, maxLen, L"?", _TRUNCATE); break; }
        double d = QuickFileDuration(trackPath);
        if (d > 0) FormatTime(d, out, maxLen);
        break;
    }
    case COL_PATH:
        if (trackPath) {
            wchar_t tmp[MAX_PATH];
            wcsncpy_s(tmp, trackPath, _TRUNCATE);
            wchar_t* lastSep = wcsrchr(tmp, L'\\');
            if (lastSep) *lastSep = 0;
            // Option: show path relative to the loaded playlist folder
            if (g_colPathRelative && g_playlistRoot[0]) {
                size_t rl = wcslen(g_playlistRoot);
                if (_wcsnicmp(tmp, g_playlistRoot, rl) == 0 &&
                    (tmp[rl] == L'\\' || tmp[rl] == L'\0')) {
                    const wchar_t* rel = tmp + rl;
                    if (*rel == L'\\') rel++;
                    wcsncpy_s(out, maxLen, *rel ? rel : L".", _TRUNCATE);
                    break;
                }
            }
            wcsncpy_s(out, maxLen, tmp, _TRUNCATE);
        }
        break;
    case COL_SIZE: {
        if (!trackPath || isUrl) break;
        WIN32_FILE_ATTRIBUTE_DATA fa = {};
        if (GetFileAttributesEx(trackPath, GetFileExInfoStandard, &fa)) {
            ULONGLONG sz = ((ULONGLONG)fa.nFileSizeHigh << 32) | fa.nFileSizeLow;
            if (sz > 1024ULL * 1024) swprintf_s(out, maxLen, L"%.1f MB", sz / 1048576.0);
            else swprintf_s(out, maxLen, L"%.0f KB", sz / 1024.0);
        }
        break;
    }
    case COL_ARTIST:
    case COL_TITLE:
    case COL_ALBUM:
    case COL_TRACK:
    case COL_YEAR:
    case COL_GENRE:
    case COL_BITRATE:
    case COL_SAMPLERATE:
    case COL_CHANNELS: {
        if (!trackPath || isUrl) break;
        TagCache& tc = GetTagsForFile(trackPath);
        switch (colId) {
        case COL_ARTIST:     wcsncpy_s(out, maxLen, tc.artist.c_str(), _TRUNCATE); break;
        case COL_TITLE:      wcsncpy_s(out, maxLen, tc.title.c_str(), _TRUNCATE); break;
        case COL_ALBUM:      wcsncpy_s(out, maxLen, tc.album.c_str(), _TRUNCATE); break;
        case COL_TRACK:      wcsncpy_s(out, maxLen, tc.track.c_str(), _TRUNCATE); break;
        case COL_YEAR:       wcsncpy_s(out, maxLen, tc.year.c_str(), _TRUNCATE); break;
        case COL_GENRE:      wcsncpy_s(out, maxLen, tc.genre.c_str(), _TRUNCATE); break;
        case COL_BITRATE:    if (tc.bitrate > 0) swprintf_s(out, maxLen, L"%d kbps", tc.bitrate); break;
        case COL_SAMPLERATE: if (tc.sampleRate > 0) swprintf_s(out, maxLen, L"%d Hz", tc.sampleRate); break;
        case COL_CHANNELS:   if (tc.channels == 1) wcsncpy_s(out, maxLen, L"Mono", _TRUNCATE);
                             else if (tc.channels == 2) wcsncpy_s(out, maxLen, L"Stereo", _TRUNCATE);
                             else if (tc.channels > 0) swprintf_s(out, maxLen, L"%d ch", tc.channels); break;
        }
        break;
    }
    }
}

// Show right-click column picker menu
// Auto-fit a single header column to content width
static void AutoFitColumn(int headerIdx)
{
    if (!hColumnHeader || !hListBox) return;
    HDITEM hdi = {}; hdi.mask = HDI_LPARAM;
    Header_GetItem(hColumnHeader, headerIdx, &hdi);
    int cid = (int)hdi.lParam;

    HDC dc = GetDC(hListBox);
    HFONT oldF = (HFONT)SelectObject(dc, g_fontUI);

    // Measure header text
    SIZE hsz = {};
    GetTextExtentPoint32(dc, COL_HDR_LABELS[cid], (int)wcslen(COL_HDR_LABELS[cid]), &hsz);
    int maxW = hsz.cx + 16;

    // Measure content
    int count = min((int)g_playlist.size(), 200); // sample first 200
    for (int i = 0; i < count; i++) {
        wchar_t colText[MAX_PATH] = L"";
        // LB_GETTEXT has NO buffer bound — check the length first or a
        // long item (stream URL row) overruns the stack and crashes.
        wchar_t dispText[MAX_PATH + 8] = L"";
        if (SendMessage(hListBox, LB_GETTEXTLEN, i, 0) < (LRESULT)_countof(dispText))
            SendMessage(hListBox, LB_GETTEXT, i, (LPARAM)dispText);
        GetColumnText(cid, i, i < (int)g_playlist.size() ? g_playlist[i].path : NULL,
            dispText, colText, MAX_PATH);
        SIZE sz = {};
        GetTextExtentPoint32(dc, colText, (int)wcslen(colText), &sz);
        int w = sz.cx + 12;
        if (cid == COL_NAME && g_modernStyle) w += THUMB_SZ + 6;
        if (w > maxW) maxW = w;
    }
    SelectObject(dc, oldF);
    ReleaseDC(hListBox, dc);

    if (maxW < 30) maxW = 30;
    if (maxW > 600) maxW = 600;
    hdi.mask = HDI_WIDTH; hdi.cxy = maxW;
    Header_SetItem(hColumnHeader, headerIdx, &hdi);
    g_colWidth[cid] = maxW;
    InvalidateRect(hListBox, NULL, FALSE);
}

static void ShowColumnPickerMenu(HWND hwnd, POINT pt)
{
    HMENU hm = CreatePopupMenu();
    // Size to fit options
    AppendMenu(hm, MF_STRING, 10100, L"Size Column to Fit");
    AppendMenu(hm, MF_STRING, 10101, L"Size All Columns to Fit");
    AppendMenu(hm, MF_SEPARATOR, 0, NULL);
    // Column visibility toggles
    for (int i = 0; i < COL_COUNT; i++) {
        UINT flags = MF_STRING | (g_colVisible[i] ? MF_CHECKED : MF_UNCHECKED);
        if (i == COL_NAME) flags |= MF_GRAYED;
        AppendMenu(hm, flags, 10000 + i, COL_MENU_LABELS[i]);
    }
    // Figure out which header column was right-clicked
    POINT hdrPt = pt; ScreenToClient(hColumnHeader, &hdrPt);
    HDHITTESTINFO hht = {}; hht.pt = hdrPt;
    int clickedCol = (int)SendMessage(hColumnHeader, HDM_HITTEST, 0, (LPARAM)&hht);

    int cmd = (int)TrackPopupMenu(hm, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hm);
    if (cmd == 10100 && clickedCol >= 0) {
        AutoFitColumn(clickedCol);
        SaveSettings();
    } else if (cmd == 10101) {
        int n = Header_GetItemCount(hColumnHeader);
        for (int i = 0; i < n; i++) AutoFitColumn(i);
        // Shrink columns proportionally if total exceeds listbox width
        RECT lbRc; GetClientRect(hListBox, &lbRc);
        int availW = lbRc.right - GetSystemMetrics(SM_CXVSCROLL);
        int totalW = 0;
        for (int i = 0; i < n; i++) {
            HDITEM hdi = {}; hdi.mask = HDI_WIDTH;
            Header_GetItem(hColumnHeader, i, &hdi);
            totalW += hdi.cxy;
        }
        if (totalW > availW && totalW > 0) {
            float scale = (float)availW / (float)totalW;
            for (int i = 0; i < n; i++) {
                HDITEM hdi = {}; hdi.mask = HDI_WIDTH | HDI_LPARAM;
                Header_GetItem(hColumnHeader, i, &hdi);
                hdi.cxy = max(30, (int)(hdi.cxy * scale));
                hdi.mask = HDI_WIDTH;
                Header_SetItem(hColumnHeader, i, &hdi);
            }
        }
        InvalidateRect(hListBox, NULL, FALSE);
        SaveSettings();
    } else if (cmd >= 10000 && cmd < 10000 + COL_COUNT) {
        int cid = cmd - 10000;
        if (cid == COL_NAME) return;
        g_colVisible[cid] = !g_colVisible[cid];
        RebuildColumnHeader();
        InvalidateRect(hListBox, NULL, FALSE);
        SaveSettings();
    }
}

// Column sort state: -1 = no sort, else ColID
// g_sortCol, g_sortAsc, g_hdrHotItem, g_unsortedPlaylist declared in forward section

// Sort the playlist by a column
// Helper: rebuild listbox from g_playlist, re-find current track by path
static void RebuildListboxAfterSort(const wchar_t* curPath)
{
    SendMessage(hListBox, WM_SETREDRAW, FALSE, 0);
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (auto& t : g_playlist) {
        wchar_t disp[MAX_PATH + 4];
        if (!g_favActive && IsFavorite(t.path))
            swprintf_s(disp, L"\u2605 %s", t.display);
        else
            wcsncpy_s(disp, t.display, _TRUNCATE);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)disp);
    }
    SendMessage(hListBox, WM_SETREDRAW, TRUE, 0);
    if (curPath && curPath[0]) {
        for (int i = 0; i < (int)g_playlist.size(); i++) {
            if (_wcsicmp(g_playlist[i].path, curPath) == 0) {
                g_currentIndex = i;
                SendMessage(hListBox, LB_SETCURSEL, i, 0);
                break;
            }
        }
    }
    RebuildShuffleOrder();
    InvalidateRect(hListBox, NULL, FALSE);
}

static void SortPlaylistByColumn(int colId, bool ascending)
{
    if (g_playlist.empty()) return;
    wchar_t curPath[1024] = {};
    if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
        wcsncpy_s(curPath, g_playlist[g_currentIndex].path, _TRUNCATE);

    // Snapshot original order before first sort
    if (g_unsortedPlaylist.empty())
        g_unsortedPlaylist = g_playlist;

    std::sort(g_playlist.begin(), g_playlist.end(),
        [colId, ascending](const Track& a, const Track& b) {
            wchar_t ta[1024] = {}, tb[1024] = {};
            GetColumnText(colId, 0, a.path, a.display, ta, _countof(ta));
            GetColumnText(colId, 0, b.path, b.display, tb, _countof(tb));
            int cmp = _wcsicmp(ta, tb);
            return ascending ? (cmp < 0) : (cmp > 0);
        });
    RebuildListboxAfterSort(curPath);
}

static void RestoreUnsortedPlaylist()
{
    if (g_unsortedPlaylist.empty()) return;
    wchar_t curPath[1024] = {};
    if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
        wcsncpy_s(curPath, g_playlist[g_currentIndex].path, _TRUNCATE);
    g_playlist = g_unsortedPlaylist;
    g_unsortedPlaylist.clear();
    RebuildListboxAfterSort(curPath);
}

// Subclass for header: dark mode bg, hover tracking
static WNDPROC g_OldHdrProc = NULL;
static LRESULT CALLBACK HdrSubProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // Hover tracking (works in both light and dark mode)
    if (msg == WM_MOUSEMOVE) {
        HDHITTESTINFO hht = {}; hht.pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        int hit = (int)SendMessage(hwnd, HDM_HITTEST, 0, (LPARAM)&hht);
        if (hit != g_hdrHotItem) {
            g_hdrHotItem = hit;
            InvalidateRect(hwnd, NULL, FALSE);
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
        }
    }
    if (msg == WM_MOUSELEAVE) {
        g_hdrHotItem = -1;
        InvalidateRect(hwnd, NULL, FALSE);
    }
    if (g_darkMode) {
        if (msg == WM_ERASEBKGND) {
            HDC dc = (HDC)wp; RECT rc; GetClientRect(hwnd, &rc);
            HBRUSH br = CreateSolidBrush(0x383838);
            FillRect(dc, &rc, br); DeleteObject(br);
            return 1;
        }
        if (msg == WM_PAINT) {
            LRESULT r = CallWindowProc(g_OldHdrProc, hwnd, msg, wp, lp);
            HDC dc = GetDC(hwnd); RECT rc; GetClientRect(hwnd, &rc);
            HBRUSH br = CreateSolidBrush(0x383838);
            RECT bot = { 0, rc.bottom - 1, rc.right, rc.bottom };
            FillRect(dc, &bot, br);
            int n = Header_GetItemCount(hwnd);
            if (n > 0) {
                RECT lastRc; Header_GetItemRect(hwnd, n - 1, &lastRc);
                if (lastRc.right < rc.right) {
                    RECT gap = { lastRc.right, rc.top, rc.right, rc.bottom };
                    FillRect(dc, &gap, br);
                }
            }
            DeleteObject(br);
            ReleaseDC(hwnd, dc);
            return r;
        }
    }
    return CallWindowProc(g_OldHdrProc, hwnd, msg, wp, lp);
}

static void CreateColumnHeader(HWND hwnd)
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    hColumnHeader = CreateWindowEx(0, WC_HEADER, NULL,
        WS_CHILD | HDS_HORZ | HDS_BUTTONS | HDS_DRAGDROP | HDS_FULLDRAG | CCS_NORESIZE,
        0, 0, 0, 0, hwnd, (HMENU)ID_COLUMNVIEW, hInst, NULL);
    SendMessage(hColumnHeader, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
    g_OldHdrProc = (WNDPROC)SetWindowLongPtr(hColumnHeader, GWLP_WNDPROC, (LONG_PTR)HdrSubProc);
    RebuildColumnHeader();
}

static void ToggleColumnView()
{
    g_columnView = !g_columnView;
    // Column view needs owner-draw — recreate listbox if needed
    RecreateListbox();
    ShowWindow(hColumnHeader, g_columnView ? SW_SHOW : SW_HIDE);
    LayoutControls(g_hwnd);
    HMENU hm = GetMenu(g_hwnd);
    if (hm) CheckMenuItem(hm, IDM_VIEW_COLUMNS,
        MF_BYCOMMAND | (g_columnView ? MF_CHECKED : MF_UNCHECKED));
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
        StatusSetParts();
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
    // Record button (red circle) right after transport, before Shuffle
    if (hRecordBtn) MoveWindow(hRecordBtn, m + 5 * (bw + bs), y, bw, bh, TRUE);
    int sbx = m + 5 * (bw + bs) + bw + 6;
    MoveWindow(hShuffleBtn, sbx, y, 46, bh, TRUE);
    MoveWindow(hRepeatBtn, sbx + 50, y, 46, bh, TRUE);
    MoveWindow(hMonoBtn, sbx + 100, y, 36, bh, TRUE);
    MoveWindow(hNormalizeBtn, sbx + 140, y, 36, bh, TRUE);
    MoveWindow(hBassBoostBtn, sbx + 180, y, 58, bh, TRUE);
    // FX (DSP) button: right of BassBoost, visible only when any extra effect is enabled
    if (hDspBtn) {
        bool showDsp = g_dspReverb || g_dspSaturate || g_dspVinyl || g_dspHifi;
        MoveWindow(hDspBtn, sbx + 242, y, 30, bh, TRUE);
        ShowWindow(hDspBtn, showDsp ? SW_SHOW : SW_HIDE);
    }
    // Row 2: seek bar
    int vw = 12;  // vol bar width

    // Library button: far right, next to vol bar — only when media folders set.
    // Back button: appears for the local browser OR a Sets.LIVE playlist
    // (to return to the directory), positioned left of Library if it's shown.
    bool showBack = (g_browserActive || g_browserReturn || g_setsReturnActive);
    // Keep the ★ button visible while browser mode is active even with no
    // media folders configured (e.g. "Open File Location in Billy" on a track
    // outside any root) — it's the designed way back out of the browser.
    bool showLib  = (hMediaBtn && (!g_mediaFolders.empty() || g_mediaActive));
    if (hMediaBtn) {
        if (showLib) {
            int libX = rc.right - vw - 4 - 42;
            MoveWindow(hMediaBtn, libX, y, 42, bh, TRUE);
            ShowWindow(hMediaBtn, SW_SHOW);
            if (hBackBtn && showBack) {
                MoveWindow(hBackBtn, libX - 20, y, 18, bh, TRUE);
                ShowWindow(hBackBtn, SW_SHOW);
            } else if (hBackBtn) ShowWindow(hBackBtn, SW_HIDE);
        } else {
            ShowWindow(hMediaBtn, SW_HIDE);
            if (hBackBtn && showBack) {
                MoveWindow(hBackBtn, rc.right - vw - 4 - 18, y, 18, bh, TRUE);
                ShowWindow(hBackBtn, SW_SHOW);
            } else if (hBackBtn) ShowWindow(hBackBtn, SW_HIDE);
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
        int remW = g_recording ? 130 : max(baseW + 10, 58);  // fixed wide when recording to fit "⏺ 999.9 MB"
        MoveWindow(hTimeCur,    m,                          ty, curW, th, TRUE);
        MoveWindow(hTimeTot,    m + curW + 2,               ty, totW, th, TRUE);
        MoveWindow(hTimeRemain, m + curW + 2 + totW + 2,   ty, remW, th, TRUE);
    }
    if (hVolPct) MoveWindow(hVolPct, pctX, ty, pctW, th, TRUE);

    // Vol bar: spans all 3 rows tall, flush right
    int volTop = y;
    int volBot = ty + th;
    MoveWindow(hVolumeCanvas, vx, volTop, vw, volBot - volTop, TRUE);

    // Playlist + optional column header
    int ly = ty + th + 2;
    int lw = rc.right - 2 * m;
    if (g_columnView && hColumnHeader) {
        // Use WINDOWPOS/HDM_LAYOUT to get proper header height
        HDLAYOUT hdl = {};
        WINDOWPOS wp = {};
        RECT hdrRc = { m, ly, m + lw, ly + 200 };
        hdl.prc = &hdrRc;
        hdl.pwpos = &wp;
        Header_Layout(hColumnHeader, &hdl);
        MoveWindow(hColumnHeader, wp.x, wp.y, wp.cx, wp.cy, TRUE);
        ly += wp.cy;
    }
    int lh = rc.bottom - sbh - ly - m;
    if (lh < 60) lh = 60;
    MoveWindow(hListBox, m, ly, lw, lh, TRUE);
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
        else if (IsAudio(p)) {
            if (g_dropLoadDir) {
                // Load entire parent folder instead of just this file
                wchar_t folder[MAX_PATH]; wcsncpy_s(folder, p, _TRUNCATE);
                wchar_t* sep = wcsrchr(folder, L'\\');
                if (!sep) sep = wcsrchr(folder, L'/');
                if (sep) *sep = 0;
                AddFolder(folder);
            } else {
                AddTrack(p);
            }
        }
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

    // Fixed bindings: Ctrl+Up / Ctrl+Down volume
    if (vk == VK_UP && (GetKeyState(VK_CONTROL) & 0x8000)) {
        currentVolume = min(1.0f, currentVolume + 0.05f);
        UpdateVolume(); return true;
    }
    if (vk == VK_DOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
        currentVolume = max(0.0f, currentVolume - 0.05f);
        UpdateVolume(); return true;
    }

    // Customizable bindings (Options -> Hotkeys) — exact match on key + modifiers
    BYTE mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)   mods |= 1;
    if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
    if (GetKeyState(VK_MENU) & 0x8000)    mods |= 4;
    int act = -1;
    for (int i = 0; i < HK_COUNT; i++)
        if (g_hotkeys[i].vk && g_hotkeys[i].vk == (BYTE)vk && g_hotkeys[i].mods == mods) { act = i; break; }
    if (act < 0) return false;

    switch (act) {
    case HK_PLAYPAUSE: TogglePlayPause();   return true;
    case HK_STOP:      StopAudio();         return true;
    case HK_NEXT:      PlayNext();          return true;
    case HK_PREV:      PlayPrev();          return true;
    case HK_SEARCH:    OpenSearchDialog();  return true;
    case HK_SHUFFLE:
        g_shuffle = !g_shuffle;
        CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_SHUFFLE,
            MF_BYCOMMAND | (g_shuffle ? MF_CHECKED : MF_UNCHECKED));
        InvalidateRect(hShuffleBtn, NULL, TRUE); return true;
    case HK_REPEAT:
        g_repeatMode = (g_repeatMode + 1) % 4; // cycle: off -> track -> playlist -> once -> off
        CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_REPEAT,
            MF_BYCOMMAND | (g_repeatMode ? MF_CHECKED : MF_UNCHECKED));
        InvalidateRect(hRepeatBtn, NULL, TRUE); PreloadNext(); return true;
    case HK_MONO:
        g_mono = !g_mono; ApplyDSP();
        InvalidateRect(hMonoBtn, NULL, TRUE); return true;
    case HK_BASSBOOST:
        g_bassBoost = !g_bassBoost; ApplyDSP();
        InvalidateRect(hBassBoostBtn, NULL, TRUE); return true;
    case HK_NORMALIZE:
        g_normalize = !g_normalize; ApplyDSP();
        InvalidateRect(hNormalizeBtn, NULL, TRUE); return true;
    case HK_INFO:
        if (g_currentIndex >= 0) OpenAudioInfoDialog(g_currentIndex);
        return true;
    case HK_REFRESH:
        RefreshPlaylistFolder(); return true;
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
        // Set user agent for internet streams (required for many HTTPS radio stations)
        BASS_SetConfigPtr(BASS_CONFIG_NET_AGENT, "BillyPro/0.5");
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
        hRecordBtn    = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_RECORD,    hInst, NULL);
        hMediaBtn     = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MEDIA,     hInst, NULL);
        hBackBtn      = CreateWindow(L"BUTTON", L"", OD, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_BACK,      hInst, NULL);

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
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_EXTENDEDSEL
            | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
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
        SetWindowLongPtr(hRecordBtn,    GWLP_WNDPROC, (LONG_PTR)BtnHotProc);
        SetWindowLongPtr(hBackBtn,      GWLP_WNDPROC, (LONG_PTR)BtnHotProc);

        // Tooltips for the toolbar toggle buttons
        {
            HWND hTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                WS_POPUP | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, hInst, NULL);
            if (hTip) {
                SendMessage(hTip, TTM_SETMAXTIPWIDTH, 0, 340);
                struct { HWND btn; const wchar_t* txt; } tips[] = {
                    { hNormalizeBtn, L"Loud: boosts quiet tracks up to 0 dB and limits peaks.\r\nRight-click: mode picker (files) or intensity slider (radio/streams)." },
                    { hBassBoostBtn, L"Bass Boost: adds low-end punch with a soft-clip limiter.\r\nAdjust frequency and gain in Settings → Audio." },
                    { hShuffleBtn,   L"Shuffle: play the tracks in random order." },
                    { hRepeatBtn,    L"Repeat: cycle Off → Track → Playlist → Play Once.\r\nRight-click to pick a mode directly." },
                    { hMonoBtn,      L"Mono: mix both channels down to a single mono signal." },
                    { hDspBtn,       L"DSP: toggle the extra effects (Reverb / Saturation / Vinyl / HiFi).\r\nConfigure them in Settings → Audio." },
                };
                for (auto& t : tips) {
                    if (!t.btn) continue;
                    TOOLINFO ti = { sizeof(ti) };
                    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                    ti.hwnd = hwnd;
                    ti.uId = (UINT_PTR)t.btn;
                    ti.lpszText = (LPWSTR)t.txt;
                    SendMessage(hTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
                }
            }
        }

        hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
            WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
            0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, hInst, NULL);
        SendMessage(hStatus, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
        StatusSetParts();
        // Subclass for dark mode text color
        g_OldStatusProc = (WNDPROC)SetWindowLongPtr(hStatus, GWLP_WNDPROC, (LONG_PTR)StatusBarProc);

        // Column header (real Win32 Header control, supports drag-reorder)
        InitColOrder();
        CreateColumnHeader(hwnd);

        LoadSettings();
        LoadFavorites();
        ApplyTheme();  // re-apply after LoadSettings so dark mode is applied on startup
        DragAcceptFiles(hwnd, TRUE);
        LayoutControls(hwnd);
        UpdateVolume();
        UpdateStatusBar();

        // Apply loaded column layout — rebuild header without saving (would overwrite loaded values)
        if (hColumnHeader) {
            while (Header_GetItemCount(hColumnHeader) > 0)
                Header_DeleteItem(hColumnHeader, 0);
            int pos = 0;
            for (int i = 0; i < COL_COUNT; i++) {
                int cid = g_colOrder[i];
                if (!g_colVisible[cid]) continue;
                HDITEM hdi = {};
                hdi.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT | HDI_LPARAM;
                hdi.pszText = (LPWSTR)COL_HDR_LABELS[cid];
                hdi.cxy = g_colWidth[cid];
                hdi.fmt = COL_FMT[cid] | HDF_STRING;
                hdi.lParam = cid;
                Header_InsertItem(hColumnHeader, pos++, &hdi);
            }
        }
        if (g_columnView) {
            ShowWindow(hColumnHeader, SW_SHOW);
            RecreateListbox();
        }

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

        // Initialize SMTC (System Media Transport Controls — lock screen / media overlay)
        SmtcInit(hwnd);
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
            ms->itemHeight = g_modernStyle ? (THUMB_SZ + 4) : 20;
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
        // Owner-draw listbox — favorites highlight + Modern Style artwork + Column View
        if (dis->CtlType == ODT_LISTBOX && dis->CtlID == ID_LISTBOX) {
            int idx = (int)dis->itemID;
            int lbCount = (int)SendMessage(dis->hwndItem, LB_GETCOUNT, 0, 0);
            if (idx < 0 || idx >= lbCount) {
                HBRUSH bgBr = CreateSolidBrush(g_theme.bgList);
                FillRect(dis->hDC, &dis->rcItem, bgBr); DeleteObject(bgBr);
                return TRUE;
            }
            HDC dc = dis->hDC;
            RECT rc = dis->rcItem;
            bool selected = (dis->itemState & ODS_SELECTED) != 0;

            // LB_GETTEXT has NO buffer bound — check the length first or a
            // long item (stream URL row) overruns the stack and crashes.
            wchar_t text[MAX_PATH + 8] = L"";
            if (SendMessage(dis->hwndItem, LB_GETTEXTLEN, idx, 0) < (LRESULT)_countof(text))
                SendMessage(dis->hwndItem, LB_GETTEXT, idx, (LPARAM)text);

            // Resolve file path for artwork + favorite check
            const wchar_t* trackPath = NULL;
            if (g_browserActive && idx < (int)g_browserItems.size() && !g_browserItems[idx].isDir)
                trackPath = g_browserItems[idx].path;
            else if (!g_browserActive && idx < (int)g_playlist.size())
                trackPath = g_playlist[idx].path;
            bool isFav = !g_favActive && trackPath && IsFavorite(trackPath);
            // Check if this item is the currently playing track
            bool isPlaying = false;
            if (!g_browserActive && g_currentIndex >= 0 && currentStream) {
                isPlaying = (idx == g_currentIndex);
            }

            // Colors: playing track gets a distinct tint, separate from selection
            COLORREF bgCol, txCol;
            if (g_darkMode) {
                if (selected)
                    { bgCol = RGB(30, 80, 160); txCol = RGB(255, 255, 255); }
                else if (isPlaying)
                    { bgCol = RGB(34, 38, 46); txCol = g_theme.accent; }
                else if (isFav)
                    { bgCol = RGB(38, 55, 60); txCol = RGB(140, 200, 220); }
                else
                    { bgCol = g_theme.bgList; txCol = g_theme.text; }
            } else {
                if (selected)
                    { bgCol = GetSysColor(COLOR_HIGHLIGHT); txCol = GetSysColor(COLOR_HIGHLIGHTTEXT); }
                else if (isPlaying)
                    { bgCol = RGB(235, 242, 252); txCol = RGB(30, 80, 160); }
                else if (isFav)
                    { bgCol = RGB(220, 235, 250); txCol = RGB(30, 80, 140); }
                else
                    { bgCol = GetSysColor(COLOR_WINDOW); txCol = GetSysColor(COLOR_WINDOWTEXT); }
            }

            HBRUSH br = CreateSolidBrush(bgCol);
            FillRect(dc, &rc, br); DeleteObject(br);
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, txCol);
            HFONT oldFont = (HFONT)SelectObject(dc, isPlaying ? g_fontBold : g_fontUI);

            int textLeft = rc.left + 4;

            // Column View: draw columns using the Header control layout
            // Skip for browser items (folders, go-back) — draw those as plain text
            if (g_columnView && !g_browserActive) {
                RECT colRects[COL_COUNT]; int colIDs[COL_COUNT];
                int nCols = GetColumnRects(rc, colRects, colIDs, COL_COUNT);

                for (int c = 0; c < nCols; c++) {
                    RECT cr = colRects[c];
                    cr.top = rc.top; cr.bottom = rc.bottom;

                    // For COL_NAME with modern style: draw thumbnail then text
                    if (colIDs[c] == COL_NAME && g_modernStyle) {
                        int thumbX = cr.left + 2;
                        int thumbY = cr.top + (cr.bottom - cr.top - THUMB_SZ) / 2;
                        HBITMAP thumb = trackPath ? GetThumbForPath(trackPath) : g_thumbPlaceholder;
                        if (!thumb) thumb = g_thumbPlaceholder;
                        if (thumb) {
                            HDC mdc = CreateCompatibleDC(dc);
                            HBITMAP ob = (HBITMAP)SelectObject(mdc, thumb);
                            BitBlt(dc, thumbX, thumbY, THUMB_SZ, THUMB_SZ, mdc, 0, 0, SRCCOPY);
                            SelectObject(mdc, ob); DeleteDC(mdc);
                        }
                        cr.left += THUMB_SZ + 6;
                    } else {
                        cr.left += 4;
                    }
                    cr.right -= 2;

                    wchar_t colText[MAX_PATH] = L"";
                    GetColumnText(colIDs[c], idx, trackPath, text, colText, MAX_PATH);

                    UINT fmt = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
                    fmt |= (COL_FMT[colIDs[c]] == HDF_RIGHT) ? DT_RIGHT : DT_LEFT;
                    DrawText(dc, colText, -1, &cr, fmt);
                }

                if (dis->itemState & ODS_FOCUS) DrawFocusRect(dc, &rc);
                SelectObject(dc, oldFont);
                return TRUE;
            }

            // Modern Style: draw artwork thumbnail on the left
            if (g_modernStyle) {
                int thumbX = rc.left + 2;
                int thumbY = rc.top + (rc.bottom - rc.top - THUMB_SZ) / 2;
                HBITMAP thumb = trackPath ? GetThumbForPath(trackPath) : g_thumbPlaceholder;
                if (!thumb) thumb = g_thumbPlaceholder;
                if (thumb) {
                    HDC mdc = CreateCompatibleDC(dc);
                    HBITMAP ob = (HBITMAP)SelectObject(mdc, thumb);
                    BitBlt(dc, thumbX, thumbY, THUMB_SZ, THUMB_SZ, mdc, 0, 0, SRCCOPY);
                    HPEN tp = CreatePen(PS_SOLID, 1, g_darkMode ? 0x555555 : 0xBBBBBB);
                    HPEN top2 = (HPEN)SelectObject(dc, tp);
                    SelectObject(dc, GetStockObject(NULL_BRUSH));
                    Rectangle(dc, thumbX, thumbY, thumbX + THUMB_SZ, thumbY + THUMB_SZ);
                    SelectObject(dc, top2); DeleteObject(tp);
                    SelectObject(mdc, ob); DeleteDC(mdc);
                }
                textLeft = thumbX + THUMB_SZ + 6;
            }

            // Draw text
            RECT textRc = rc;
            textRc.left = textLeft;
            DrawText(dc, text, -1, &textRc,
                DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
            SelectObject(dc, oldFont);
            if (dis->itemState & ODS_FOCUS) DrawFocusRect(dc, &rc);
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
        case ID_BTN_RECORD:    DrawBtn(dis, BTN_RECORD);    return TRUE;
        case ID_BTN_BACK:       DrawBtn(dis, BTN_BACK);       return TRUE;
        }
        break;
    }

    case WM_DROPFILES:
        HandleDrop((HDROP)wParam);
        break;

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lParam;
        // Dark mode custom-draw for the Header control
        if (g_darkMode && hColumnHeader && nm->hwndFrom == hColumnHeader && nm->code == NM_CUSTOMDRAW) {
            NMCUSTOMDRAW* cd = (NMCUSTOMDRAW*)lParam;
            switch (cd->dwDrawStage) {
            case CDDS_PREPAINT: {
                RECT hrc; GetClientRect(hColumnHeader, &hrc);
                HBRUSH bgBr = CreateSolidBrush(0x383838);
                FillRect(cd->hdc, &hrc, bgBr); DeleteObject(bgBr);
                return CDRF_NOTIFYITEMDRAW;
            }
            case CDDS_ITEMPREPAINT: {
                HDC hdc = cd->hdc;
                int itemIdx = (int)cd->dwItemSpec;
                bool isHot = (itemIdx == g_hdrHotItem);
                HDITEM hdiLp = {}; hdiLp.mask = HDI_LPARAM;
                Header_GetItem(hColumnHeader, itemIdx, &hdiLp);
                int cid = (int)hdiLp.lParam;
                bool isSorted = (cid == g_sortCol && g_sortCol >= 0);

                COLORREF bgc = isHot ? 0x484848 : (isSorted ? 0x404040 : 0x383838);
                HBRUSH br = CreateSolidBrush(bgc);
                FillRect(hdc, &cd->rc, br); DeleteObject(br);
                HPEN pen = CreatePen(PS_SOLID, 1, 0x585858);
                HPEN op = (HPEN)SelectObject(hdc, pen);
                MoveToEx(hdc, cd->rc.left, cd->rc.bottom - 1, NULL);
                LineTo(hdc, cd->rc.right, cd->rc.bottom - 1);
                MoveToEx(hdc, cd->rc.right - 1, cd->rc.top + 2, NULL);
                LineTo(hdc, cd->rc.right - 1, cd->rc.bottom - 2);
                SelectObject(hdc, op); DeleteObject(pen);
                SetTextColor(hdc, isHot ? 0xFFFFFF : g_theme.text);
                SetBkMode(hdc, TRANSPARENT);
                HFONT oldF = (HFONT)SelectObject(hdc, g_fontUI);
                wchar_t txt[64] = {};
                HDITEM hdi = {}; hdi.mask = HDI_TEXT | HDI_FORMAT;
                hdi.pszText = txt; hdi.cchTextMax = 64;
                Header_GetItem(hColumnHeader, itemIdx, &hdi);
                RECT tr = cd->rc; tr.left += 6;
                tr.right -= isSorted ? 16 : 4;
                UINT dtFmt = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS;
                dtFmt |= (hdi.fmt & HDF_RIGHT) ? DT_RIGHT : DT_LEFT;
                DrawText(hdc, txt, -1, &tr, dtFmt);
                if (isSorted) {
                    const wchar_t* arrow = g_sortAsc ? L"\u25B2" : L"\u25BC";
                    SetTextColor(hdc, g_theme.accent);
                    RECT ar = { cd->rc.right - 14, cd->rc.top, cd->rc.right - 2, cd->rc.bottom };
                    DrawText(hdc, arrow, 1, &ar, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
                }
                SelectObject(hdc, oldF);
                return CDRF_SKIPDEFAULT;
            }
            }
        }
        if (nm->hwndFrom == hColumnHeader) {
            switch (nm->code) {
            case HDN_ITEMCHANGED:   // column resized
                InvalidateRect(hListBox, NULL, FALSE);
                break;
            case HDN_ITEMCLICK: {  // column header clicked — sort
                NMHEADER* nhdr = (NMHEADER*)lParam;
                HDITEM hdi = {}; hdi.mask = HDI_LPARAM;
                Header_GetItem(hColumnHeader, nhdr->iItem, &hdi);
                int cid = (int)hdi.lParam;
                if (cid == g_sortCol) {
                    if (g_sortAsc) {
                        g_sortAsc = false;                    // asc -> desc
                        SortPlaylistByColumn(g_sortCol, g_sortAsc);
                    } else {
                        g_sortCol = -1; g_sortAsc = true;    // desc -> restore original
                        RestoreUnsortedPlaylist();
                    }
                } else {
                    g_sortCol = cid; g_sortAsc = true;       // new column -> asc
                    SortPlaylistByColumn(g_sortCol, g_sortAsc);
                }
                InvalidateRect(hColumnHeader, NULL, FALSE);
                break;
            }
            case HDN_TRACK: {      // column being resized — enforce minimum width
                NMHEADER* nhdr = (NMHEADER*)lParam;
                if (nhdr->pitem && (nhdr->pitem->mask & HDI_WIDTH)) {
                    if (nhdr->pitem->cxy < 30) { nhdr->pitem->cxy = 30; return TRUE; }
                }
                break;
            }
            case HDN_ENDDRAG: {    // column reordered via drag
                // Let the system apply the drag, then on next tick read back
                // the new order and rebuild our header to match.
                // Returning FALSE allows the header to apply the move itself.
                // We then pick up the result in a deferred message.
                PostMessage(hwnd, WM_APP + 98, 0, 0);
                return FALSE;
            }
            case NM_RCLICK: {
                POINT pt; GetCursorPos(&pt);
                ShowColumnPickerMenu(hwnd, pt);
                break;
            }
            }
        }
        break;
    }

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
            // Direct stream: refresh the download/cache info once a second
            // (right status panel only, so the scrolling name isn't reset)
            if (!g_decStream && !g_radioPlaying && hStatus) {
                static DWORD s_lastBufTick = 0;
                if (now - s_lastBufTick >= 1000) {
                    s_lastBufTick = now;
                    wchar_t rt[352];
                    wcsncpy_s(rt, g_stRightBase, _TRUNCATE);
                    AppendStreamBufferInfo(rt, _countof(rt));
                    SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)rt);
                    if (g_darkMode) InvalidateRect(hStatus, NULL, FALSE);
                }
            }
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
        if (wParam == IDT_RADIO_META && g_radioPlaying && currentStream) {
            // Poll ICY metadata for current track info
            const char* meta = BASS_ChannelGetTags(currentStream, BASS_TAG_META);
            if (meta) {
                // Format: "StreamTitle='Artist - Track';StreamUrl='...'"
                const char* st = strstr(meta, "StreamTitle='");
                if (st) {
                    st += 13;
                    // Title ends at "';" — a plain quote can appear INSIDE the
                    // title (e.g. Bass'Flo), so don't stop at the first one.
                    const char* end = strstr(st, "';");
                    if (!end) end = strrchr(st, '\'');
                    if (end && end > st) {
                        std::string title8(st, end - st);
                        wchar_t newMeta[512] = {};
                        MultiByteToWideChar(CP_UTF8, 0, title8.c_str(), -1, newMeta, 511);
                        if (wcscmp(newMeta, g_radioNowPlaying) != 0) {
                            // Station history: the previous track just ended — log it
                            // with its end time as a row in the playlist view
                            if (g_radioNowPlaying[0] && !g_browserActive &&
                                g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size()) {
                                SYSTEMTIME ht; GetLocalTime(&ht);
                                Track h = {};
                                wcsncpy_s(h.path, g_playlist[g_currentIndex].path, _TRUNCATE);
                                _snwprintf_s(h.display, _countof(h.display), _TRUNCATE,
                                    L"      %02d:%02d  %s", ht.wHour, ht.wMinute, g_radioNowPlaying);
                                g_playlist.push_back(h);
                                if (hListBox) SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)h.display);
                            }
                            wcsncpy_s(g_radioNowPlaying, newMeta, _TRUNCATE);
                            if (g_recLog && g_recording) {
                                SYSTEMTIME st; GetLocalTime(&st);
                                fwprintf(g_recLog, L"[%02d:%02d:%02d] %s\n", st.wHour, st.wMinute, st.wSecond, newMeta);
                                fflush(g_recLog);
                            }
                            // Update title bar
                            wchar_t wtitle[512];
                            if (g_radioNowPlaying[0])
                                _snwprintf_s(wtitle, _countof(wtitle), _TRUNCATE,
                                    L"%s - %s - BillyPro", g_radioNowPlaying, g_radioStationName);
                            else
                                _snwprintf_s(wtitle, _countof(wtitle), _TRUNCATE,
                                    L"%s - BillyPro", g_radioStationName);
                            SetWindowText(g_hwnd, wtitle);
                            UpdateStatusBar();
                            // Lock screen / media overlay follows the ICY track
                            SmtcUpdateMetadata();
                            // Update radio dialog status
                            if (g_hwndRadio) {
                                wchar_t rstatus[512];
                                _snwprintf_s(rstatus, _countof(rstatus), _TRUNCATE,
                                    L"\u25B6 %s\n%s", g_radioStationName, g_radioNowPlaying);
                                SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, rstatus);
                            }
                        }
                    }
                }
            }
            // Also check ICY headers for station info
            if (!g_radioNowPlaying[0]) {
                const char* icy = BASS_ChannelGetTags(currentStream, BASS_TAG_ICY);
                if (!icy) icy = BASS_ChannelGetTags(currentStream, BASS_TAG_HTTP);
            }
            // Auto-reconnect if radio stream dropped
            DWORD chState = BASS_ChannelIsActive(currentStream);
            if (chState == BASS_ACTIVE_STOPPED || chState == 0) {
                if (g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
                    wcsncpy_s(g_radioReconnUrl, g_playlist[g_currentIndex].path, _TRUNCATE);
                BASS_StreamFree(currentStream);
                currentStream = 0;
                g_radioStream = 0;
                g_radioPlaying = false;
                KillTimer(g_hwnd, IDT_RADIO_META);
                RadioStartReconnCountdown();
            }
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
        if (wParam == IDT_LOUD_HINT) {
            KillTimer(hwnd, IDT_LOUD_HINT);
            HideLoudHintBalloon();
        }
        if (wParam == IDT_RADIO_RECONN) {
            g_radioReconnCountdown--;
            if (g_radioReconnCountdown <= 0) {
                RadioLaunchReconnect();
            } else {
                wchar_t title[512];
                _snwprintf_s(title, _countof(title), _TRUNCATE, L"%s (reconnecting %d...) - BillyPro",
                    g_radioStationName, g_radioReconnCountdown);
                SetWindowText(g_hwnd, title);
                wchar_t status[256];
                swprintf_s(status, L"Reconnecting in %d... (attempt %d)",
                    g_radioReconnCountdown, g_radioReconnAttempt);
                if (g_hwndRadio) SetDlgItemText(g_hwndRadio, ID_RADIO_STATUS, status);
            }
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
                               (g_hwndConvert && IsWindow(g_hwndConvert)) ||
                               (g_hwndDiscogs && IsWindow(g_hwndDiscogs)) ||
                               (g_hwndRadio && IsWindow(g_hwndRadio)) ||
                               (g_hwndStream && IsWindow(g_hwndStream)) ||
                               (g_hwndPMgr && IsWindow(g_hwndPMgr)) ||
                               (g_hwndNamePrompt && IsWindow(g_hwndNamePrompt)) ||
                               (g_hwndShazam && IsWindow(g_hwndShazam)) ||
                               (g_hwndSets && IsWindow(g_hwndSets));
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
            for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++) {
                wchar_t esc[300]; MenuEscapeAmp(g_bppPlaylists[i].name.c_str(), esc, 300);
                AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_BASE + i, esc);
            }
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
            g_repeatMode = (g_repeatMode + 1) % 4;
            CheckMenuItem(GetMenu(g_hwnd), IDM_PLAY_REPEAT,
                MF_BYCOMMAND | (g_repeatMode ? MF_CHECKED : MF_UNCHECKED));
            InvalidateRect(hRepeatBtn, NULL, TRUE); PreloadNext(); break;
        case ID_BTN_BACK: {
            if (g_setsReturnActive) {
                // Reopen the Sets.LIVE browser at the folder we streamed from
                OpenSetsLiveDialog(g_setsReturnPath.c_str());
            } else if (g_browserActive && !g_browserItems.empty() && g_browserItems[0].isDir
                && wcsstr(g_browserItems[0].display, L"[..]") != NULL) {
                // Navigate back one level in the browser
                BrowserNavigate(0);
            } else if (g_browserReturn) {
                // Return to the browser folder we came from
                g_browserReturn = false;
                g_browserActive = true;
                FillBrowser(g_browserReturnPath);
                LayoutControls(hwnd);
            }
            break;
        }
        case ID_BTN_MONO:
            g_mono = !g_mono; ApplyDSP(); InvalidateRect(hMonoBtn, NULL, TRUE); break;
        case ID_BTN_NORMALIZE:
            g_normalize = !g_normalize; ApplyDSP(); InvalidateRect(hNormalizeBtn, NULL, TRUE);
            // First Loud click ever: point out the right-click options
            if (!g_loudHintShown) {
                g_loudHintShown = true;
                ShowLoudHintBalloon();
                SaveSettings();
            }
            break;
        case ID_BTN_BASSBOOST:
            g_bassBoost = !g_bassBoost; ApplyDSP(); InvalidateRect(hBassBoostBtn, NULL, TRUE); break;
        case ID_BTN_DSP:
            g_dspBypass = !g_dspBypass;
            ApplyDSP();
            if (hDspBtn) InvalidateRect(hDspBtn, NULL, TRUE);
            break;
        case ID_BTN_RECORD:
            if (g_recording) StopRecording();
            else StartRecording();
            break;
        case ID_BTN_MEDIA: {
            // Allow toggle-OFF even with no media folders configured, so the
            // browser opened via "Open File Location in Billy" can be exited.
            if (g_mediaFolders.empty() && !g_mediaActive) break;
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
                g_browserReturn = false;
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
            LayoutControls(hwnd); // update back button visibility
            UpdateStatusBar();
            break;
        }
        case ID_LISTBOX:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int s = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (s != LB_ERR) {
                    if (g_browserActive) BrowserNavigate(s);
                    else { PlayIndex(s); PlayHistResumeSeek(s); }
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
        case IDC_CTX_OPENINBILLY:
            // Open the track's folder in Billy's own media browser (the ★ view)
            // and highlight the file — like Explorer's "open file location" but
            // inside the library tree. Works from any view; ★/← goes back.
            if (g_ctxFilePath[0] && !IsUrlPath(g_ctxFilePath)) {
                wchar_t dir[1024];
                wcsncpy_s(dir, g_ctxFilePath, _TRUNCATE);
                wchar_t* sl = wcsrchr(dir, L'\\');
                if (sl) {
                    *sl = L'\0';
                    // Leave any library view (Favorites / playlist) cleanly so
                    // its saved playlist is restored, then enter browser mode
                    // exactly like the ★ button does (playback keeps running).
                    LibRestorePlaylist();
                    if (!g_mediaActive) {
                        g_savedPlaylist = g_playlist;
                        g_mediaActive = true;
                    }
                    g_browserActive = true;
                    FillBrowser(dir);
                    // Select + reveal the file's row in the browser list
                    for (int i = 0; i < (int)g_browserItems.size(); i++) {
                        if (!g_browserItems[i].isDir &&
                            _wcsicmp(g_browserItems[i].path, g_ctxFilePath) == 0) {
                            SendMessage(hListBox, LB_SETCURSEL, i, 0);
                            SendMessage(hListBox, LB_SETTOPINDEX, max(0, i - 3), 0);
                            break;
                        }
                    }
                    if (hMediaBtn) InvalidateRect(hMediaBtn, NULL, TRUE);
                    LayoutControls(hwnd);
                    UpdateStatusBar();
                }
            }
            break;
        case IDC_CTX_DOWNLOAD:
            // Download a Sets.LIVE stream from the playlist right-click
            if (IsSetsUrl(g_ctxFilePath)) {
                const wchar_t* nm = (g_ctxTrackIndex >= 0 && g_ctxTrackIndex < (int)g_playlist.size())
                    ? g_playlist[g_ctxTrackIndex].display : L"download.mp3";
                SetsDownloadUrl(g_hwnd, g_ctxFilePath, nm);
            }
            break;
        case IDC_CTX_FAV_ADD: {
            // Add ALL selected tracks to favorites (falls back to right-clicked one).
            // For stream URLs also remember the display name — the favorites
            // view shows that instead of the raw URL.
            auto favName = [](const wchar_t* path, const wchar_t* disp) {
                if (!disp || !disp[0] || !IsUrlPath(path)) return;
                if ((disp[0] == L'▶' || disp[0] == L'★') && disp[1] == L' ') disp += 2;
                if (disp[0]) g_favNames[path] = disp;
            };
            int added = 0;
            if (!g_ctxIsBrowser && hListBox) {
                int total = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
                for (int i = 0; i < total && i < (int)g_playlist.size(); i++) {
                    if (SendMessage(hListBox, LB_GETSEL, i, 0) > 0 && !IsFavorite(g_playlist[i].path)) {
                        g_favorites.push_back(g_playlist[i].path);
                        favName(g_playlist[i].path, g_playlist[i].display);
                        added++;
                    }
                }
            }
            if (added == 0 && g_ctxFilePath[0] && !IsFavorite(g_ctxFilePath)) {
                g_favorites.push_back(g_ctxFilePath);
                if (g_ctxTrackIndex >= 0 && g_ctxTrackIndex < (int)g_playlist.size() &&
                    _wcsicmp(g_playlist[g_ctxTrackIndex].path, g_ctxFilePath) == 0)
                    favName(g_ctxFilePath, g_playlist[g_ctxTrackIndex].display);
                added++;
            }
            if (added > 0) { SaveFavorites(); RefreshListboxStars(); }
            break;
        }
        case IDC_CTX_FAV_REMOVE: {
            // Remove ALL selected tracks from favorites (falls back to right-clicked one)
            int removed = 0;
            if (!g_ctxIsBrowser && hListBox) {
                int total = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
                for (int i = 0; i < total && i < (int)g_playlist.size(); i++) {
                    if (SendMessage(hListBox, LB_GETSEL, i, 0) > 0) {
                        for (auto it = g_favorites.begin(); it != g_favorites.end(); ++it)
                            if (_wcsicmp(it->c_str(), g_playlist[i].path) == 0) {
                                g_favNames.erase(*it);
                                g_favorites.erase(it); removed++; break;
                            }
                    }
                }
            }
            if (removed == 0 && g_ctxFilePath[0]) { RemoveFavorite(g_ctxFilePath); break; }
            if (removed > 0) {
                SaveFavorites();
                if (g_favActive) {
                    // Favorites view: rebuild so removed rows disappear
                    LibShowTracks(g_favorites);
                } else {
                    RefreshListboxStars();
                }
            }
            break;
        }
        case IDC_CTX_PL_NEW: {
            // New playlist from ALL selected tracks (name-only prompt,
            // saved automatically into the playlists folder)
            s_newPlTracks.clear();
            if (!g_ctxIsBrowser && hListBox) {
                int total = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
                for (int i = 0; i < total && i < (int)g_playlist.size(); i++)
                    if (SendMessage(hListBox, LB_GETSEL, i, 0) > 0)
                        s_newPlTracks.push_back(g_playlist[i].path);
            }
            if (s_newPlTracks.empty() && g_ctxFilePath[0])
                s_newPlTracks.push_back(g_ctxFilePath);
            OpenNamePromptDialog();
            break;
        }
        case IDM_LIB_FAVORITES:
            LoadFavoritesIntoPlaylist();
            break;
        case IDM_LIB_RADIO:
            OpenRadioDialog();
            break;
        case IDM_LIB_PL_MANAGER:
            OpenPlaylistManager();
            break;
        case IDM_LIB_SETSLIVE:
            OpenSetsLiveDialog();
            break;
        case IDM_SHAZAM:
            OpenShazamDialog(false);
            break;
        case IDM_SHAZAM_HISTORY:
            OpenShazamDialog(true);
            break;
        case IDM_LIB_PL_MANAGE: {
            // Open playlists folder in Explorer
            GetLibDir();
            ShellExecute(hwnd, L"open", g_plDir, NULL, NULL, SW_SHOW);
            break;
        }
            // Menu
        case IDM_FILE_OPENFOLDER: {
            wchar_t p[MAX_PATH] = {};
            if (PickFolder(hwnd, L"Select Music Folder", p, MAX_PATH))
                LoadFolder(p);
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
        case IDM_FILE_REFRESH:   RefreshPlaylistFolder(); break;
        case IDM_FILE_OPENSTREAM: OpenStreamLinkDialog(); break;
        case IDM_FILE_PLAYHISTORY: ShowPlayHistoryView(); break;

        case IDM_VIEW_DARKMODE:
            g_darkMode = !g_darkMode;
            ApplyTheme();
            SaveSettings();
            break;
        case IDM_VIEW_MODERN:
            ToggleModernStyle();
            SaveSettings();
            break;
        case IDM_VIEW_COLUMNS:
            ToggleColumnView();
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
            g_repeatMode = (g_repeatMode + 1) % 4;
            CheckMenuItem(GetMenu(hwnd), IDM_PLAY_REPEAT, g_repeatMode ? MF_CHECKED : MF_UNCHECKED);
            InvalidateRect(hRepeatBtn, NULL, TRUE); PreloadNext(); break;

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
                L"BillyPro V0.9.1\n\n"
                L"Lightweight Music Player\n\n"
                L"Created by MRJN/CLD.",
                L"About BillyPro", MB_ICONINFORMATION);
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
                L"  M              Toggle Mono\n"
                L"  B              Toggle Bass Boost\n"
                L"  L              Toggle Normalize\n"
                L"  Q              Stop playback\n"
                L"  I              Audio Information\n"
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
                if (pi < (int)g_bppPlaylists.size()) {
                    int total = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0);
                    int added = 0;
                    for (int i = 0; i < total; i++) {
                        if (SendMessage(hListBox, LB_GETSEL, i, 0) > 0 && i < (int)g_playlist.size()) {
                            AddTrackToPlaylist(g_bppPlaylists[pi].filePath.c_str(), g_playlist[i].path);
                            added++;
                        }
                    }
                    if (added == 0 && g_ctxFilePath[0]) {
                        AddTrackToPlaylist(g_bppPlaylists[pi].filePath.c_str(), g_ctxFilePath);
                        added = 1;
                    }
                    // Visible feedback in the status bar
                    if (hStatus) {
                        wchar_t sbmsg[300];
                        swprintf_s(sbmsg, L"Added %d track(s) to playlist \"%s\"",
                            added, g_bppPlaylists[pi].name.c_str());
                        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)sbmsg);
                    }
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
        // Free the decode stream(s) GaplessProc retired on the mixing thread —
        // it can't free them there, so they're deferred to us via the ring.
        DecDrainFrees();
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
            // New track's Loud makeup gain (cache was warmed by PreloadNext)
            g_loudNormGain = (g_normalize && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
                ? ComputeNormGain(g_playlist[g_currentIndex].path) : 1.0f;
            if (g_hwnd) LayoutControls(g_hwnd);
            if (hTimeTot) {
                wchar_t buf[32], tot[40];
                FormatTime(GetTrackLength(), buf, _countof(buf));
                swprintf_s(tot, L"/ %s", buf);
                SetWindowText(hTimeTot, tot);
            }
            SelectPlayingTrack(g_currentIndex);
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
                    // Re-arm end-of-stream sync on the freshly created master —
                    // without it, the "no next preloaded" path (GaplessProc
                    // breaks without posting) never advances and playback stalls.
                    BASS_ChannelSetSync(currentStream, BASS_SYNC_END, 0, EndSyncProc, g_hwnd);
                    ApplyDSP();
                    ApplyPitch();
                    UpdateVolume();
                    BASS_ChannelPlay(currentStream, FALSE);
                    SetTimer(g_hwnd, IDT_PLAYBACK, 16, NULL);
                }
                if (g_hwnd) LayoutControls(g_hwnd);
                SelectPlayingTrack(g_currentIndex);
                UpdatePlayBtn(); UpdateStatusBar(); UpdateWindowTitle(); UpdateTimeDisplays();
                SmtcUpdateMetadata(); SmtcUpdatePlaybackStatus();
                if (hSeekCanvas) InvalidateRect(hSeekCanvas, NULL, FALSE);
                if (hVolumeCanvas) InvalidateRect(hVolumeCanvas, NULL, FALSE);
                PreloadNext();
            }
        }
        else {
            // End-of-stream (wParam == 0, posted by EndSyncProc). lParam is the
            // master that ended. If it isn't the current master, this is a stale
            // sync from a master we already replaced (format-mismatch switch) —
            // ignore it so we don't advance a second time and skip a track.
            if (lParam != 0 && (HSTREAM)lParam != currentStream) {
                UpdateThumbButtons();
                break;
            }
            // End of playlist or no pre-loaded stream
            KillTimer(g_hwnd, IDT_PLAYBACK);
            SetWindowText(hTimeCur, L"0:00");
            SetWindowText(hTimeRemain, L"");
            InvalidateRect(hSeekCanvas, NULL, FALSE);
            if (g_repeatMode == 1 && g_currentIndex >= 0) PlayIndex(g_currentIndex);
            else PlayNext();
        }
        UpdateThumbButtons();
        break;

    // Deferred: system applied column drag — read back the new order and rebuild
    case WM_APP + 98: {
        if (!hColumnHeader) break;
        int n = Header_GetItemCount(hColumnHeader);
        if (n <= 0) break;
        // Read the order array the system applied
        std::vector<int> order(n);
        Header_GetOrderArray(hColumnHeader, n, order.data());
        // Collect items in the new visual order, then rebuild header sequentially
        struct CI { wchar_t txt[64]; int w; int fmt; LPARAM lp; };
        std::vector<CI> sorted(n);
        for (int i = 0; i < n; i++) {
            int idx = order[i];
            HDITEM hdi = {}; hdi.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT | HDI_LPARAM;
            hdi.pszText = sorted[i].txt; hdi.cchTextMax = 64;
            Header_GetItem(hColumnHeader, idx, &hdi);
            sorted[i].w = hdi.cxy; sorted[i].fmt = hdi.fmt; sorted[i].lp = hdi.lParam;
        }
        while (Header_GetItemCount(hColumnHeader) > 0)
            Header_DeleteItem(hColumnHeader, 0);
        for (int i = 0; i < n; i++) {
            HDITEM hdi = {};
            hdi.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT | HDI_LPARAM;
            hdi.pszText = sorted[i].txt; hdi.cxy = sorted[i].w;
            hdi.fmt = sorted[i].fmt; hdi.lParam = sorted[i].lp;
            Header_InsertItem(hColumnHeader, i, &hdi);
        }
        if (g_columnView && hListBox) InvalidateRect(hListBox, NULL, FALSE);
        SaveSettings();
        break;
    }

    // Deferred column view redraw after header drag-reorder completes
    case WM_APP + 99:
        if (g_columnView && hListBox) InvalidateRect(hListBox, NULL, FALSE);
        break;

    case WM_RADIO_CONNECTED:
        RadioOnConnected((HSTREAM)wParam, (LONG)lParam);
        break;

    // URL artwork fetched on a worker thread — store it (replacing the
    // "pending" sentinel) and repaint the list so the thumb appears.
    case WM_THUMB_READY: {
        ThumbUrlJob* j = (ThumbUrlJob*)wParam;
        if (j) {
            auto it = g_thumbCache.find(j->key);
            if (it != g_thumbCache.end() && it->second == (HBITMAP)1)
                it->second = j->bm;                    // may be NULL = no art
            else if (j->bm) DeleteObject(j->bm);       // cache cleared meanwhile
            delete j;
        }
        if (hListBox) InvalidateRect(hListBox, NULL, FALSE);
        break;
    }

    // Sets.LIVE download progress/done when started from the playlist right-click
    // (_TRUNCATE: name[260] + dir[260] can exceed the buffer — swprintf_s would
    //  abort the whole process on overflow, _snwprintf_s just clips the text)
    case WM_SETS_DLPROGRESS:
        if (hStatus) {
            wchar_t s[400];
            _snwprintf_s(s, _countof(s), _TRUNCATE, L"Downloading “%s” ... %d%%", g_setsDlName, (int)wParam);
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)s);
        }
        break;
    case WM_SETS_DLDONE:
        if (hStatus) {
            wchar_t s[600];
            if (wParam) _snwprintf_s(s, _countof(s), _TRUNCATE, L"✓ Downloaded “%s” to %s", g_setsDlName, g_setsDownloadDir);
            else        _snwprintf_s(s, _countof(s), _TRUNCATE, L"✗ Sets.LIVE download failed. Check the folder / connection.");
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)s);
        }
        break;

    case WM_POWERBROADCAST:
        if (wParam == PBT_APMRESUMEAUTOMATIC || wParam == PBT_APMRESUMESUSPEND) {
            SmtcUpdatePlaybackStatus();
            SmtcUpdateMetadata();
        }
        break;

    case WM_DESTROY:
        if (g_recording) StopRecording();   // finalize FLAC so file is seekable
        SaveSession();  // save playlist/track/position before stopping audio
        // Remember the resume position for Play History before streams are freed
        if (currentStream && g_decStream && g_currentIndex >= 0 && g_currentIndex < (int)g_playlist.size())
            UpdatePlayHistoryPos(g_playlist[g_currentIndex].path, GetPlayPos());
        ClearThumbCache();
        if (g_thumbPlaceholder) { DeleteObject(g_thumbPlaceholder); g_thumbPlaceholder = NULL; }
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
        DecDrainFrees();
        if (currentStream) { BASS_ChannelStop(currentStream); BASS_StreamFree(currentStream); }
        BASS_Free();
        // Unregister media key hotkeys
        UnregisterHotKey(hwnd, ID_HOTKEY_PLAYPAUSE);
        UnregisterHotKey(hwnd, ID_HOTKEY_NEXT);
        UnregisterHotKey(hwnd, ID_HOTKEY_PREV);
        UnregisterHotKey(hwnd, ID_HOTKEY_STOP);
        // Release SMTC
        SmtcCleanup();
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
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    RegisterCustomClasses(hInst);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc; wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BILLYPRO));
    if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // fallback
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_COMPOSITED, CLASS_NAME, APP_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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
    AppendMenu(hFile, MF_STRING, IDM_FILE_REFRESH,    L"Refresh Playlist\tShift+R");
    AppendMenu(hFile, MF_STRING, IDM_FILE_OPENSTREAM, L"Open Stream Link...\tCtrl+N");
    AppendMenu(hFile, MF_STRING, IDM_FILE_PLAYHISTORY, L"Play History");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_OPENM3U,   L"Load Playlist (M3U)...");
    AppendMenu(hFile, MF_STRING, IDM_FILE_SAVEM3U,   L"Save Playlist (M3U)...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_CONVERT,   L"Convert Audio...");
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
    AppendMenu(hView, MF_STRING, IDM_VIEW_MODERN,   L"Modern Style");
    AppendMenu(hView, MF_STRING, IDM_VIEW_COLUMNS,  L"Column View");

    AppendMenu(hOpts, MF_STRING, IDM_OPTIONS, L"Options...");

    AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT,    L"About...");
    AppendMenu(hHelp, MF_STRING, IDM_HELP_CONTROLS, L"Controls...");
    AppendMenu(hHelp, MF_SEPARATOR, 0, NULL);
    AppendMenu(hHelp, MF_STRING, IDM_HELP_UPDATE,  L"Check for Updates...");

    // Library menu (Favorites + cascading Playlists submenu)
    HMENU hLib = CreatePopupMenu();
    g_hLibMenu = hLib;
    AppendMenu(hLib, MF_STRING, IDM_LIB_FAVORITES, L"Favorites");
    AppendMenu(hLib, MF_STRING, IDM_LIB_PL_MANAGER, L"Playlist Manager...");
    AppendMenu(hLib, MF_STRING, IDM_LIB_RADIO, L"Internet Radio");
    AppendMenu(hLib, MF_SEPARATOR, 0, NULL);
    // Playlists as cascading submenu (rebuilt dynamically in WM_INITMENUPOPUP)
    g_hPlSubMenu = CreatePopupMenu();
    ScanPlaylists();
    for (int i = 0; i < (int)g_bppPlaylists.size() && i < 90; i++) {
        wchar_t escInit[300]; MenuEscapeAmp(g_bppPlaylists[i].name.c_str(), escInit, 300);
        AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_BASE + i, escInit);
    }
    if (!g_bppPlaylists.empty())
        AppendMenu(g_hPlSubMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(g_hPlSubMenu, MF_STRING, IDM_LIB_PL_MANAGE, L"Open Playlists Folder...");
    AppendMenu(hLib, MF_POPUP, (UINT_PTR)g_hPlSubMenu, L"Playlists");

    // Shazam menu (Song Recognition + History)
    HMENU hShaz = CreatePopupMenu();
    AppendMenu(hShaz, MF_STRING, IDM_SHAZAM,         L"Song Recognition");
    AppendMenu(hShaz, MF_STRING, IDM_SHAZAM_HISTORY, L"History");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPlay, L"&Play");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hView, L"&View");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLib,  L"&Library");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hShaz, L"&Shazam");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOpts, L"&Options");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"&Help");
    SetMenu(hwnd, hMenu);

    // Now that the menu exists, apply persisted checkmarks
    if (g_darkMode)    CheckMenuItem(hMenu, IDM_VIEW_DARKMODE,  MF_BYCOMMAND | MF_CHECKED);
    if (g_columnView)  CheckMenuItem(hMenu, IDM_VIEW_COLUMNS,  MF_BYCOMMAND | MF_CHECKED);
    if (g_modernStyle) {
        CheckMenuItem(hMenu, IDM_VIEW_MODERN, MF_BYCOMMAND | MF_CHECKED);
        g_thumbPlaceholder = CreatePlaceholderBitmap();
        RecreateListbox();
    }
    if (g_shuffle)    CheckMenuItem(hMenu, IDM_PLAY_SHUFFLE,  MF_BYCOMMAND | MF_CHECKED);
    if (g_repeatMode) CheckMenuItem(hMenu, IDM_PLAY_REPEAT,   MF_BYCOMMAND | MF_CHECKED);
    UpdateSetsLibMenu();  // add Sets.LIVE to Library if integration is enabled
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
        // No arguments: try restoring previous session first
        bool restored = RestoreSession();
        if (!restored) {
            // No session to restore: load media folder or exe folder
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
    }

    // Sync column view now that playlist is loaded
    if (g_columnView) SyncColumnView();

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
                (g_hwndConvert && IsWindow(g_hwndConvert) && (focusWnd == g_hwndConvert || IsChild(g_hwndConvert, focusWnd))) ||
                (g_hwndDiscogs && IsWindow(g_hwndDiscogs) && (focusWnd == g_hwndDiscogs || IsChild(g_hwndDiscogs, focusWnd))) ||
                (g_hwndRadio   && IsWindow(g_hwndRadio)   && (focusWnd == g_hwndRadio   || IsChild(g_hwndRadio,   focusWnd))) ||
                (g_hwndStream  && IsWindow(g_hwndStream)  && (focusWnd == g_hwndStream  || IsChild(g_hwndStream,  focusWnd))) ||
                (g_hwndPMgr    && IsWindow(g_hwndPMgr)    && (focusWnd == g_hwndPMgr    || IsChild(g_hwndPMgr,    focusWnd))) ||
                (g_hwndNamePrompt && IsWindow(g_hwndNamePrompt) && (focusWnd == g_hwndNamePrompt || IsChild(g_hwndNamePrompt, focusWnd))) ||
                (g_hwndShazam  && IsWindow(g_hwndShazam)  && (focusWnd == g_hwndShazam  || IsChild(g_hwndShazam,  focusWnd))) ||
                (g_hwndSets    && IsWindow(g_hwndSets)    && (focusWnd == g_hwndSets    || IsChild(g_hwndSets,    focusWnd))) ||
                (g_hwndAssoc   && IsWindow(g_hwndAssoc)   && (focusWnd == g_hwndAssoc   || IsChild(g_hwndAssoc,   focusWnd)));
            if (!searchFocused && !dialogFocused) {
                WPARAM vk = msg.wParam;
                // Ctrl+O opens folder
                if (vk == 'O' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    SendMessage(g_hwnd, WM_COMMAND, IDM_FILE_OPENFOLDER, 0);
                    continue;
                }
                // Ctrl+F opens search
                if (vk == 'F' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    OpenSearchDialog();
                    continue;
                }
                // Ctrl+N opens the stream link dialog (like VLC's Open Network)
                if (vk == 'N' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    OpenStreamLinkDialog();
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
            // Convert Audio dialog: Ctrl+A selects all tracks no matter which
            // control inside the dialog currently has focus
            if (msg.wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000) &&
                g_hwndConvert && IsWindow(g_hwndConvert) &&
                (focusWnd == g_hwndConvert || IsChild(g_hwndConvert, focusWnd))) {
                HWND hCl = GetDlgItem(g_hwndConvert, ID_CONV_LIST);
                if (hCl) {
                    SendMessage(hCl, LB_SETSEL, TRUE, (LPARAM)-1);
                    SetFocus(hCl);
                }
                continue;
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
        if (g_hwndConvert && IsWindow(g_hwndConvert) && IsDialogMessage(g_hwndConvert, &msg)) continue;
        if (g_hwndDiscogs && IsWindow(g_hwndDiscogs) && IsDialogMessage(g_hwndDiscogs, &msg)) continue;
        if (g_hwndRadio   && IsWindow(g_hwndRadio)   && IsDialogMessage(g_hwndRadio,   &msg)) continue;
        if (g_hwndStream  && IsWindow(g_hwndStream)  && IsDialogMessage(g_hwndStream,  &msg)) continue;
        if (g_hwndPMgr    && IsWindow(g_hwndPMgr)    && IsDialogMessage(g_hwndPMgr,    &msg)) continue;
        if (g_hwndNamePrompt && IsWindow(g_hwndNamePrompt) && IsDialogMessage(g_hwndNamePrompt, &msg)) continue;
        if (g_hwndShazam  && IsWindow(g_hwndShazam)  && IsDialogMessage(g_hwndShazam,  &msg)) continue;
        if (g_hwndSets    && IsWindow(g_hwndSets)    && IsDialogMessage(g_hwndSets,    &msg)) continue;
        if (g_hwndAssoc   && IsWindow(g_hwndAssoc)   && IsDialogMessage(g_hwndAssoc,   &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hMutex);
    CoUninitialize();
    return (int)msg.wParam;
}