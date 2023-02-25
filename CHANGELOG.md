# Change Log
All notable changes to this project will be documented in this file.

## [0.7.1] - 2023-02-25

### Fixed

* Disable WinRT in plugins due to performance issues

## [0.7.0] - 2023-02-15

### Fixed

* LV2 plugins didn't work due to incorrect URL
* [ui] incorrect sizing of list elements

### Added

* [ui] colors mode for remapped keyboard
* buttons to cycle through the modes with the same keys selected
* icon for standalone app
* [LUMI keys](https://roli.com/products/blocks/lumi-keys-studio-edition) hardware support (4 modes, if plugin is disabled it will only highlight the key, otherwise it will highlight remapped keys, power button is changing color mode)
* custom scales override is possible if `scales.txt` is placed near the plugin binary file. Text file should list scales in format `intervals scaleName`, ex:
    
    ```
    2 1 2 2 2 2 1 Melodic Minor 
    2 1 2 2 1 3 1 Harmonic Minor
    ```

### Changed

* updated JUCE to 7.0.5
* [ui] updated minor deps

## [0.6.1] - 2023-02-08

### Changed

* [ui] event system to support correct multiple parameters change

## [0.5.2] - 2022-07-26

### Added

* lv2 support
* vst support for hosts incompatible with vst3 midi (won't be distributed due to the license limitations)

## [0.5.1] - 2022-01-07

### Added

* [ui] interactive keyboard feature

## [0.4.2] - 2021-12-26

### Changed

* [ui] restyled color theme
* resticted window resize

## [0.4.1] - 2021-12-10

### Changed

* removed loading indicator

### Fixed

* [ui] index was set to 0 after first scale switching
* initial scale intervals were set to follow default scale (major)

## [0.4.0] - 2021-12-10

### Changed

* moved state to redux store
* now the saved params are restored correctly

### Fixed

* multiple inconsistencies in behavior

## [0.3.12] - 2021-12-5

### Fixed

* extract-release-notes action couldn't find changelog

## [0.3.11] - 2021-12-5

### Added

* Added changelog
* Integrated changelog into pipeline