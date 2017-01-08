This is an exploit for the CTRSDK CTPK buffer overflow [vuln](https://www.3dbrew.org/wiki/3DS_System_Flaws#General.2FCTRSDK). This is supposed to be generic enough to be used with other titles, but it would likely need adjusted for use with other titles(if there's any other titles this could be used with for non-RomFS at all).

Currently the only inplementation of this is for "The Legend of Zelda: Tri Force Heroes", hence that implementation is called ctpkpwn_tfh.

# ctpkpwn_tfh

For vuln details, see [here](https://www.3dbrew.org/wiki/3DS_Userland_Flaws#Non-system_applications).

This automatically triggers during the initial "Loading..." screen, ~5s after the screen turns black @ 3ds-logo.

This is installed using custom [SpotPass](https://www.3dbrew.org/wiki/SpotPass) content. The SpotPass task is automatically deleted afterwards, leaving just the downloaded content(and \*hax payload). The content and *hax payload are both stored in the SD extdata(the former is only stored as BOSS content, it is never moved elsewhere by the game). The exploit can only be used on the same system it was installed on, since this is extdata. The normal savedata is not affected at all.

The manager app loads the \*hax payload from SD "/otherapp.bin" during installation, the user must [setup](https://smealum.github.io/3ds/#otherapp) this before using the app.

Your system has to allow unsigned SpotPass(BOSS-container) content in order for this to be installed. This can be done with ctr-httpwn >=v1.2(with the included bosshaxx on supported system-versions), or "CFW". ctr-httpwn would have to be run before running this manager app for installation.

## Supported regions/versions
Only update-title v2.1.0 is supported. Regular-application titles(if any) which include {update-version} without a seperate update-title are not supported.

Supported regions:
* JPN
* USA
* EUR (Should just need ROPBUF updated)

## Credits
* Myria: locating the ROPBUF addr for JPN + JPN testing.

