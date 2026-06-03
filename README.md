<div align="center">

# asryx

<br/>

<p align="center">
  <a href="https://github.com/rccyx/asryx-for-mac/actions"><img src="https://img.shields.io/github/actions/workflow/status/rccyx/asryx-for-mac/ci.yml?style=for-the-badge&color=black&labelColor=111111&logo=githubactions&logoColor=white" alt="CI Status"/></a>
  <a href="#installation"><img src="https://img.shields.io/badge/Platform-macOS-black?style=for-the-badge&color=black&labelColor=111111&logo=apple&logoColor=white" alt="Platform: macOS"/></a>
  <a href="#runtime-model"><img src="https://img.shields.io/badge/Offline-No_Cloud-black?style=for-the-badge&color=black&labelColor=111111" alt="Offline"/></a>
  <a href="https://github.com/rccyx/asryx-for-mac/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-Apache-black?style=for-the-badge&color=black&labelColor=111111&logo=apache&logoColor=white" alt="License"/></a>
</p>

</div>

<p align="center">
  <a href="./assets/demo.gif">
    <img src="./assets/demo.gif" alt="asryx demo" width="100%">
  </a>
</p>

## Overview

asryx is a native C++ ASR binary for macOS. It builds locally against a pinned `whisper.cpp` source tree, records audio through the default system input, runs recognition in-process, writes the transcript to the clipboard, emits a Notification Center notification, and removes runtime artifacts after completion. Easily installed, and more easily removed.

Uses standard C++ and macOS system tools, so it works on any Mac. Links against `whisper.cpp` as an embedded library through its public C compatible API. On Apple Silicon, inference runs on the GPU via Metal. There is no ASR server, hosted API, Python runtime, Node runtime, container layer, resident daemon, GUI process, dashboard, subscription, or network dependency during transcription.

The program is basically a toggle, and a very simple [CLI](#cli).

```bash
asryx
```

The first invocation starts capture.

```bash
asryx
```

The next invocation stops capture, transcribes locally, copies the transcript, notifies via Notification Center, and cleans the runtime directory.

A hotkey double-fire, key repeat, or repeated invocation during an active phase cannot create parallel recorders or corrupt the current transcription.

**Runtime model:**

```text
press
  -> acquire lock
  -> start local recorder
  -> write recorder pid
  -> mark state as recording
  -> notify

press again
  -> acquire lock
  -> stop recorder
  -> mark state as transcribing
  -> decode wav into memory
  -> run whisper.cpp inference in-process
  -> trim transcript
  -> write transcript to clipboard
  -> notify
  -> remove runtime files
  -> release lock
```

Audio capture uses `rec` from sox:

```text
rec
```

`ffmpeg` is used as fallback:

```text
ffmpeg -f avfoundation
```

Captured audio is written as a temporary WAV file:

```text
mono
16 kHz
signed 16-bit
```

The second invocation stops the recorder by signal, waits for the recorder process to exit, decodes the WAV into memory as float samples, runs local inference, trims the result, and writes it to the clipboard.

Clipboard:

```text
pbcopy
```

Notifications:

```text
osascript  # Notification Center — no daemon required
```

**Runtime state:**

```text
$TMPDIR/asryx-$UID
```

**Runtime files:**

```text
lock/
rec.pid
rec.wav
rec.err
state
```

After a completed transcription, runtime files are removed. The transcript survives only through the clipboard.

## Installation

```bash
git clone https://github.com/rccyx/asryx-for-mac
cd asryx-for-mac && bash ./scripts/install
```

The installer validates the user environment, checks required tools, clones the pinned source, builds the binary locally, installs the executable, writes the version pin, writes the default config, installs the default model, selects it, and prints a PATH note when `~/.local/bin` is unavailable from the current shell.

Installed paths:

```text
~/.local/bin/asryx
~/.local/opt/whisper.cpp
~/.local/share/asryx/
~/.local/share/asryx/versions/whisper-cpp-sha
~/.asryx.conf
```

Default model:

```text
base.en
```

Model downloads pull from [Hugging Face.](https://huggingface.co/ggerganov/whisper.cpp)

After installation:

```bash
asryx status
```

Expected output:

```text
idle
```

`asryx status` prints one of:

```text
idle
recording
transcribing
```

> [!TIP]
> This output can be used for status bar tools such as xbar and SwiftBar.

## Dependencies

Build:

```text
bash
git
curl
cmake
ninja
clang++ (Xcode Command Line Tools)
```

Install build tools via Homebrew:

```bash
brew install cmake ninja sox
```

`sox` provides the `rec` command for audio capture. `pbcopy` and `osascript` are macOS built-ins — no extra packages needed for clipboard or notifications.

Xcode Command Line Tools (provides `clang++`):

```bash
xcode-select --install
```

## Keybind

The binary takes no arguments to toggle, so bind it to a key with any tool you prefer.

**skhd** (recommended for terminal-centric setups):

```
alt - w : asryx
```

**Raycast**: Add a Script Command with the script body `asryx`.

**Keyboard Maestro**: New macro → trigger Hot Key → action Execute Shell Script → `asryx`.

**System Settings** (no third-party tools): Settings → Keyboard → Keyboard Shortcuts → App Shortcuts — note this only works with menu items, so a wrapper app is needed for bare CLI invocation.

> [!TIP]
> A clipboard manager is highly recommended for long recordings, in case you copy something else by mistake after the transcription is emitted.

## CLI

The full surface area:

```text
asryx
asryx status
asryx --language <auto|CODE>
asryx --model list
asryx --model install <MODEL>
asryx --model use <MODEL>
asryx --model uninstall <MODEL>
```

List supported models:

```bash
asryx --model list
```

Install a model:

```bash
asryx --model install small.en
```

Select a model:

```bash
asryx --model use small.en
```

Remove a model:

```bash
asryx --model uninstall small.en
```

Set transcription language:

```bash
asryx --language auto
asryx --language en
asryx --language de
```

## Models

Supported models:

```text
tiny.en
tiny
base.en
base
small.en
small
medium.en
medium
large-v1
large-v2
large-v3
large-v3-turbo
large
```

| Model              | Disk    | RAM     | Speed vs large |
| ------------------ | ------- | ------- | -------------- |
| tiny / tiny.en     | 75 MiB  | ~273 MB | ~10x           |
| base / base.en     | 142 MiB | ~388 MB | ~7x            |
| small / small.en   | 466 MiB | ~852 MB | ~4x            |
| medium / medium.en | 1.5 GiB | ~2.1 GB | ~2x            |
| large-v3-turbo     | 1.5 GiB | ~2.3 GB | ~8x            |
| large-v1 / v2 / v3 | 2.9 GiB | ~3.9 GB | 1x             |

Speed is relative to large on CPU. On Apple Silicon, Metal acceleration significantly reduces inference time across all models.

`base.en` is the default. It starts quickly and covers the default English offline transcription path.

Installed models live under:

```text
~/.local/share/asryx/
```

Example:

```text
~/.local/share/asryx/ggml-base.en.bin
```

## Configuration

Configuration is stored in:

```text
~/.asryx.conf
```

Default:

```text
model=base.en
language=auto
```

`model` selects the active model. `language` controls transcription language. `auto` lets the model detect the language first before transcribing, which adds a little bit of unnecessary latency if you speak the same language all the time. Locking to a language code skips detection and transcribes instantly.

English-only models (`tiny.en`, `base.en`, `small.en`, `medium.en`) accept:

```text
auto
en
```

Multilingual models accept `auto` and every supported language code.

Invalid model and language values are rejected before recording starts.

Switching models through the CLI updates the config:

```bash
asryx --model use small.en
```

Switching language through the CLI updates the same config and preserves the active model:

```bash
asryx --language es
asryx --language auto
```

Manual edits are also valid:

```text
model=base
language=de
```

<details>
<summary><strong>Supported language codes</strong></summary>

<br/>

| Code | Language       |
| ---- | -------------- |
| en   | english        |
| zh   | chinese        |
| de   | german         |
| es   | spanish        |
| ru   | russian        |
| ko   | korean         |
| fr   | french         |
| ja   | japanese       |
| pt   | portuguese     |
| tr   | turkish        |
| pl   | polish         |
| ca   | catalan        |
| nl   | dutch          |
| ar   | arabic         |
| sv   | swedish        |
| it   | italian        |
| id   | indonesian     |
| hi   | hindi          |
| fi   | finnish        |
| vi   | vietnamese     |
| he   | hebrew         |
| uk   | ukrainian      |
| el   | greek          |
| ms   | malay          |
| cs   | czech          |
| ro   | romanian       |
| da   | danish         |
| hu   | hungarian      |
| ta   | tamil          |
| no   | norwegian      |
| th   | thai           |
| ur   | urdu           |
| hr   | croatian       |
| bg   | bulgarian      |
| lt   | lithuanian     |
| la   | latin          |
| mi   | maori          |
| ml   | malayalam      |
| cy   | welsh          |
| sk   | slovak         |
| te   | telugu         |
| fa   | persian        |
| lv   | latvian        |
| bn   | bengali        |
| sr   | serbian        |
| az   | azerbaijani    |
| sl   | slovenian      |
| kn   | kannada        |
| et   | estonian       |
| mk   | macedonian     |
| br   | breton         |
| eu   | basque         |
| is   | icelandic      |
| hy   | armenian       |
| ne   | nepali         |
| mn   | mongolian      |
| bs   | bosnian        |
| kk   | kazakh         |
| sq   | albanian       |
| sw   | swahili        |
| gl   | galician       |
| mr   | marathi        |
| pa   | punjabi        |
| si   | sinhala        |
| km   | khmer          |
| sn   | shona          |
| yo   | yoruba         |
| so   | somali         |
| af   | afrikaans      |
| oc   | occitan        |
| ka   | georgian       |
| be   | belarusian     |
| tg   | tajik          |
| sd   | sindhi         |
| gu   | gujarati       |
| am   | amharic        |
| yi   | yiddish        |
| lo   | lao            |
| uz   | uzbek          |
| fo   | faroese        |
| ht   | haitian creole |
| ps   | pashto         |
| tk   | turkmen        |
| nn   | nynorsk        |
| mt   | maltese        |
| sa   | sanskrit       |
| lb   | luxembourgish  |
| my   | myanmar        |
| bo   | tibetan        |
| tl   | tagalog        |
| mg   | malagasy       |
| as   | assamese       |
| tt   | tatar          |
| haw  | hawaiian       |
| ln   | lingala        |
| ha   | hausa          |
| ba   | bashkir        |
| jw   | javanese       |
| su   | sundanese      |
| yue  | cantonese      |

</details>

## Uninstallation

```bash
./scripts/uninstall
```

Removes owned files and leaves shared system packages untouched.

Removed paths:

```text
~/.local/bin/asryx
~/.local/opt/whisper.cpp
~/.local/share/asryx
~/.cache/asryx
~/.asryx.conf
$TMPDIR/asryx-$UID
```

## License

Apache-2.0 © @rccyx
