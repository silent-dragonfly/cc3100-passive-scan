# Passive scan for CC3100 (Proof-Of-Concept)

This project is proof-of-concept of using transceiver mode with CC3100 to reach the passive scan the nearest networks. The CC3100 BoosterPack and Advanced Emulation Kit ware used.

Also there was power consumption [tests](https://github.com/silent-dragonfly/docs/blob/master/03-measuring/measuring.md) to evaluate the difference between this passive scan and the default active scan behavior.

Please, pay attention to the issues if you want to use it to be informed.

## Build and run

1. Install `MinGW` and add it ot the `PATH`
2. Install CC3100SDK 1.2.0 (`C:\TI\CC3100SDK_1.2.0`)
3. Copy:
    - all from `${SDK}\cc3100-sdk\simplelink` to `simple-link\simple_link`
    - files from `${SDK}\cc3100-sdk\platform\simplelinkstudio` to `simple-link\simple_link_studio`
4. `mingw32-make -f Makefile` - build project
5. `Debug\cc3100=passive-scan.exe` - run
