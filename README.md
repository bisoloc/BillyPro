<p align="center">
  <img src="logo.png" alt="BillyPro" width="120"/>
</p>

# BillyPro 🎵

# BillyPro 🎵

**A blazing-fast, lightweight music player for Windows - inspired by the legendary BillyMp3.**

![Platform](https://img.shields.io/badge/platform-Windows-blue) ![Language](https://img.shields.io/badge/language-C%2B%2B-informational) ![RAM](https://img.shields.io/badge/RAM-%3C5MB-brightgreen) ![CPU](https://img.shields.io/badge/CPU-~0%25-brightgreen) ![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## Why BillyPro?

Most modern music players are Electron apps or .NET wrappers that eat 200MB of RAM before you've even hit play. BillyPro is different. It's pure **native Win32 C++** - no runtime, no framework, no bloat. It starts instantly, stays out of your way, and plays your music.

---

## Features

### ⚡ Performance
- **~0% CPU** during playback - BASS handles audio on its own thread, the UI does nothing unless you touch it
- **Under 5MB RAM** - the entire player fits comfortably alongside your music, not instead of it
- **Instant startup** - no splash screen, no loading, no bullshit. Opens in under 100ms
- **Native Win32** - compiled directly to machine code, zero managed runtime overhead

### 🎵 Audio
- Plays **every format you'll ever need** - MP3, FLAC, AAC, OGG, WAV, WMA, OPUS, AIFF, M4A and more via the BASS audio library
- **Gapless-friendly playback** with seamless track transitions
- **DSP effects chain** - Bass Boost with true peak limiting, Mono mixdown, Normalization
- **Bass Boost** with a proper brickwall limiter - boosts low frequencies without introducing clipping or distortion. When the bass hits the ceiling, all frequencies duck together cleanly
- **Mono** - fold stereo to mono for single-speaker setups or hearing checks
- **Normalization** - scans track peak and adjusts volume so every song plays at a consistent level

### 🎛️ Controls
- Keyboard shortcuts for everything - Space to play/pause, N/P for next/previous, S for shuffle, R for repeat
- **Seek bar** - click anywhere to jump, hold left/right arrow to scrub
- **Volume bar** with segmented LED display - shows volume fill, audio peak hold, and live bass FFT in red
- **Peak meter** with hold and decay
- **System tray** - minimize to tray, restore on double-click

### 📋 Playlist
- Drag & drop files and folders directly onto the window
- Multi-select with Ctrl+Click and Ctrl+A
- Drag to reorder tracks
- Delete key removes selected tracks
- **CTRL+F** search - filter your playlist instantly

### 🔧 Options
- Adjustable **Bass Boost** parameters - low freq, high freq, gain (dB)
- Settings saved to `BillyPro.ini` next to the executable - no registry pollution, fully portable
- **Convert** dialog - batch convert your library to MP3/OGG/FLAC/WAV with quality control

### 🖼️ Audio Information
- Full tag viewer - title, artist, album, year, genre, comment
- **Embedded artwork** display with save-to-file support
- Bitrate, sample rate, channels, duration, file size at a glance

---

## Screenshots

> *Coming soon*

---

## Getting Started

1. Drop `BillyPro.exe` and `bass.dll` into the same folder
2. Double-click `BillyPro.exe`
3. Drag a folder of music onto the window
4. Done

No installer. No setup wizard. No UAC prompt.

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Play / Pause |
| `N` | Next track |
| `P` | Previous track |
| `S` | Toggle Shuffle |
| `R` | Toggle Repeat |
| `←` / `→` | Seek backward / forward |
| `Ctrl+O` | Open folder |
| `Ctrl+A` | Select all in playlist |
| `Ctrl+F` | Search playlist |
| `Del` | Remove selected tracks |

---

## Building

Requires the [BASS audio library](https://www.un4seen.com/) (`bass.h`, `bass.lib`, `bass.dll`).

```
cl BillyPro.cpp /W3 /O2 /link bass.lib User32.lib Gdi32.lib Comctl32.lib Shell32.lib Ole32.lib Winmm.lib Uxtheme.lib windowscodecs.lib Comdlg32.lib
```

Or open `BillyPro.vcxproj` in Visual Studio and build.

---

## Philosophy

BillyPro is built on the principle that a music player should be invisible. You should hear your music, not your software. It takes up no space on screen, no space in memory, and no time to start. It's inspired by [BillyMp3 by SheepFriends](http://www.sheepfriends.com/?page=billy) - one of the best tiny players ever made.

---

## Credits

- Audio engine: [BASS by un4seen](https://www.un4seen.com/)
- Inspired by: BillyMp3 (SheepFriends)
- Built by: **MRJN / CLD**

---

*BillyPro v0.2 - small by design.*
