# Change Log
All notable changes to this project will be documented in this file.

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