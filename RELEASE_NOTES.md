# Release Notes

## 1.3.1.7

- Adopted Forge with applaudio cboxes as the default dependency style and reproducible release locks.
- Added an opt-in OpenAL build through `--style=git-source --profile=openal` without publishing OpenAL cboxes.
- Updated hosted applaudio builds to 8Beat 1.0.4.7 with asynchronous chiptune playback and shutdown fixes.
- Pinned OpenAL source builds to adapter release 1.0.1.14 through 8Beat.
- Documented Forge build modes for applaudio, OpenAL, Release, and local development.
- Declared all chiptune files as runtime assets for Forge builds and release archives.

## 1.3.0.6
- Bumped Termin8or dependency to 3.0.0.6.
- Updated compatibility with Termin8or's Unicode/glyph API changes.

## 1.2.0.5
- Using Termin8or 2.0.0.2 with 8-bit color support.
- Improving volume levels.

## 1.1.2.4
- Adding command line arguments.
- jingle_bells.ct: Fixed note bug that caused the chiptune player to bail out internally upon attempting to play that note.
- Added per snowflake horizontal speed factors.
- Correct use of master music volume.
- Only bumping the patch number here because of earlier bump of minor number in 1.1.0.2.

## 1.1.1.3
- Added unblocking instructions in readme for macos and windows and added additional chmodding instructions in readme for linux.

## 1.1.0.2
- Trying building two different binaries for linux. One which uses glibc2.38 (e.g. ubuntu latest), and another one that uses glibc2.35 (e.g. ubuntu wsl).

## 1.0.1.1
- Fixed partial redraw for linux and wsl. So now the rendering should l…

## 1.0.0.0
- If you fail, try try again.
