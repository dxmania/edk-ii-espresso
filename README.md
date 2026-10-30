# Samsung Galaxy® Tab 2 EDK-II

> [!CAUTION]
> This is nonfunctional in its current state. Compiling and flashing is not supported as of yet.

This project attempts to port EDK-II as a module chainloadable by SBoot for the Tab 2. Considering how the PandaPkg reference platform is entirely dead within the context of EDK-II, a port will have to be redone for the OMAP 4 4430/4460. The initial target, being the Tab 2, is 4430 based, so this port is focused SPECIFICALLY on that.

## Supported Devices:

- 7.0: 3100 (Baseband support will NOT be added), 3110, 3113

- 10.1: 5100 (Baseband support will NOT be added), 5110, 5113

In the event you have a less common device configuration that is not listed here, leave an issue on this repo, and I will consider whether it is worth the effort to make  such a port to your device.

It should be noted that this port may eventually extend to support other OMAP 4 4430/4460 devices, but the 4470 will remain unsupported at all times.

## Supported Operating Systems:

The goal is to get Windows RT 8.0/8.1 working. Linux may be an eventual goal, but for Linux support you'd be better off consulting the [Nura Project](https://wiki.nura.eco/wiki/Samsung_Galaxy_Tab_2_7.0_\(samsung-espresso7\)).
