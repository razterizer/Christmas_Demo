# Christmas_Demo

![GitHub License](https://img.shields.io/github/license/razterizer/Christmas_Demo?color=blue)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build ubuntu](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-ubuntu.yml)
[![build macos](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-macos.yml)
[![build windows](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/Christmas_Demo/actions/workflows/build-windows.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/Christmas_Demo)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/Christmas_Demo)
![C++ LOC](https://raw.githubusercontent.com/razterizer/Christmas_Demo/badges/loc-badge.svg)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/Christmas_Demo)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/Christmas_Demo?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/Christmas_Demo?color=blue)

![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/razterizer/Christmas_Demo/total)


<img width="567" alt="image" src="https://github.com/user-attachments/assets/d348db98-e8cf-4ee5-b6fd-b023329ecab1" />
<img width="559" alt="image" src="https://github.com/user-attachments/assets/44560a58-542a-44a7-8658-e8f18219e178" />

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
* https://github.com/razterizer/AudioLibSwitcher_applaudio
* https://github.com/razterizer/TrainOfThought
* https://github.com/razterizer/applaudio

Make sure the folder structure looks like this:
```
<my_source_code_dir>/lib/Core/                      ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/                 ; Termin8or repo workspace/checkout goes here.
<my_source_code_dir>/lib/8Beat/                     ; 8Beat repo workspace/checkout goes here.
<my_source_code_dir>/lib/AudioLibSwitcher_applaudio ; AudioLibSwitcher_applaudio repo workspace/checkout goes here.
<my_source_code_dir>/lib/TrainOfThought             ; TrainOfThought repo workspace/checkout goes here.
<my_source_code_dir>/lib/applaudio                  ; applaudio repo workspace/checkout goes here.
<my_source_code_dir>Christmas_Demo/                 ; Christmas_Demo repo workspace/checkout goes here.
```

These repos are not guaranteed to all the time work with the latest version of `Christmas_Demo`. If you want the more stable aproach then look at Option 1 instead.

### Windows

Then just open `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/Christmas_Demo.vs.sln` and build and run. That's it!

You can also build it by going to `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/` and build with `build.bat`.
Then you run by typing `run.bat`.

### MacOS

Then goto `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/` and build with `./build.sh`.

Then run by typing `./bin/christmas_demo`.

### Linux

Then goto `<my_source_code_dir>/Christmas_Demo/Christmas_Demo/` and build with `./build.sh`.

Then run by typing `./bin/christmas_demo`.

## Make New Release

Trigger new release. For example:
```sh
git tag release-1.0.0.0
git push origin release-1.0.0.0
```

If release workflow failed, you can delete the tag and add it again which then retriggers the release workflow. For example::
```sh
# Delete local tag
git tag -d release-1.0.0.0

# Delete remote tag
git push --delete origin release-1.0.0.0

# Re-create the tag locally
git tag release-1.0.0.0

# Push the tag again (this retriggers the workflow)
git push origin release-1.0.0.0
```

But it is best to use the script `retag_release.sh` for such tasks. E.g.:
```sh
./retag_release bump patch "Some tag message."
```
or e.g.:
```sh
./retag_release 1.1.5.7 "Some tag message."
```

Note that the tag message is currently not used as release notes, but the last commit message is. I will change this in the future.

## Running from a Release

When you download a MacOS release then you need to tell the gatekeeper unblock the executable (here: `christmas_demo`), but only if you trust the program of course (check source code + release workflow if you're unsure).
When you feel ready, you can allow the binary to be run by going to the release folder and type the following:

```sh
xattr -dr com.apple.quarantine christmas_demo
```

On Windows, you might have to unblock the exe by right-clicking the exe-file and check the `Unblock` checkbox.
