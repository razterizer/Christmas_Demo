# Christmas_Demo

![GitHub License](https://img.shields.io/github/license/razterizer/Christmas_Demo?color=blue)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build ubuntu](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-ubuntu.yml)
[![build macos](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-macos.yml)
[![build windows](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-windows.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/Christmas_Demo)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/Christmas_Demo)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/Christmas_Demo)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/Christmas_Demo?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/Christmas_Demo?color=blue)

<img width="567" alt="image" src="https://github.com/user-attachments/assets/d348db98-e8cf-4ee5-b6fd-b023329ecab1" />
<img width="559" alt="image" src="https://github.com/user-attachments/assets/44560a58-542a-44a7-8658-e8f18219e178" />

## Licensing

For 3rd-party license compatibility issues, please refer to the information here: https://github.com/razterizer/8Beat/blob/main/README.md

## Build & Run Instructions

There are two options on dealing with repo dependencies:

### Repo Dependencies Option 1

This method will ensure that you are running the latest stable versions of the dependencies that work with `Christmas_Demo`.

The script `fetch-dependencies.py` used for this was created by [Thibaut Buchert](https://github.com/thibautbuchert).
`fetch-dependencies.py` is used in the following scripts below:

After a successful build, the scripts will then prompt you with the question if you want to run the program.

When the script has been successfully run for the first time, you can then go to sub-folder `Christmas_Demo` and use the `build.sh` / `build.bat` script instead, and after you have built, just run the `run.sh` or `run.bat` script.

#### Windows

Run the following script:
```sh
setup_and_build.bat
```

#### MacOS

Run the following script:
```sh
setup_and_build_macos.sh
```

#### Linux (Debian-based, e.g. Ubuntu)

Run the following script:
```sh
setup_and_build_debian.bat
```

### Repo Dependencies Option 2

In this method we basically outline the things done in the `setup_and_build`-scripts in Option 1.

This method is more suitable for development as we're not necessarily working with "locked" dependencies.

You need the following header-only libraries:
* https://github.com/razterizer/Core
* https://github.com/razterizer/Termin8or
* https://github.com/razterizer/8Beat
* https://github.com/razterizer/AudioLibSwitcher_OpenAL ; Will be changed to something like AudioLibSwitcher_libsoundio in the future.
* https://github.com/razterizer/TrainOfThought
* https://github.com/razterizer/3rdparty_OpenAL ; Only needed for building on Windows.

Make sure the folder structure looks like this:
```
<my_source_code_dir>/lib/Core/                   ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/              ; Termin8or repo workspace/checkout goes here.
<my_source_code_dir>/lib/8Beat/                  ; 8Beat repo workspace/checkout goes here.
<my_source_code_dir>/lib/AudioLibSwitcher_OpenAL ; AudioLibSwitcher_OpenAL repo workspace/checkout goes here.
<my_source_code_dir>/lib/TrainOfThought          ; TrainOfThought repo workspace/checkout goes here.
<my_source_code_dir>/lib/3rdparty_OpenAL         ; 3rdparty_OpenAL repo workspace/checkout goes here (only needed for Windows).
<my_source_code_dir>Christmas_Demo/              ; Christmas_Demo repo workspace/checkout goes here.
```

These repos are not guaranteed to all the time work with the latest version of `Christmas_Demo`. If you want the more stable aproach then look at Option 1 instead.

### Windows

Then just open `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/Christmas_Demo.vs.sln` and build and run. That's it!

You can also build it by going to `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/` and build with `build.bat`.
Then you run by typing `run.bat`.

### MacOS

Make sure you have OpenAL-Soft installed: `brew install openal-soft`.

Then goto `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/` and build with `./build.sh`.

Then run by typing `./bin/christmas_demo`.

### Linux (Ubuntu)

Make sure you have OpenAL-Soft installed: `sudo apt install libopenal-dev`.

Then goto `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/` and build with `./build.sh`.

Then run by typing `./bin/christmas_demo`.
