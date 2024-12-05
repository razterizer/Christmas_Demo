# Christmas_Demo

## Build & Run Instructions

There are two options on dealing with repo dependencies:
### Repo Dependencies Option 1

Run the following command from `<my_source_code_dir>`.
```sh
./Christmas_Demo/fetch-dependencies.py ./Christmas_Demo/dependencies
```
This will make sure you are running the latest stable versions that work with `Christmas_Demo`.

This script was created by [Thibaut Buchert](https://github.com/thibautbuchert).

### Repo Dependencies Option 2

You need the following header-only libraries that I've made:
* https://github.com/razterizer/Core
* https://github.com/razterizer/Termin8or
* https://github.com/razterizer/8Beat
* https://github.com/razterizer/AudioLibSwitcher_OpenAL ; Will be changed to something like AudioLibSwitcher_libsoundio in the future.
* https://github.com/razterizer/TrainOfThought

Make sure the folder structure looks like this:
```
<my_source_code_dir>/lib/Core/                   ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/              ; Termin8or repo workspace/checkout goes here.
<my_source_code_dir>/lib/8Beat/                  ; 8Beat repo workspace/checkout goes here.
<my_source_code_dir>/lib/AudioLibSwitcher_OpenAL ; AudioLibSwitcher_OpenAL repo workspace/checkout goes here.
<my_source_code_dir>/lib/TrainOfThought          ; TrainOfThought repo workspace/checkout goes here.
<my_source_code_dir>Christmas_Demo/              ; Christmas_Demo repo workspace/checkout goes here.
```

These repos are not guaranteed to all the time work with the latest version of `Christmas_Demo`. If you want the more stable aproach then look at Option 1 instead.

### Windows

You also need the following 3rdparty folder with subfolders:
```
<my_source_code_dir>/lib/3rdparty/
<my_source_code_dir>/lib/3rdparty/include/
<my_source_code_dir>/lib/3rdparty/include/OpenAL_Soft/
<my_source_code_dir>/lib/3rdparty/lib/
```

`<my_source_code_dir>/lib/3rdparty/lib/` should contain:
* `OpenAL32.lib`.

`<my_source_code_dir>/lib/3rdparty/include/OpenAL_Soft/` should contain:
* `al.h`.
* `alc.h`.
* `alext.h`.
* `efx.h`.
* `efx-creative.h`.
* `efx-presets.h`.

I will change the audio library to something like `libsoundio` in the future though for licensing reasons.

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
