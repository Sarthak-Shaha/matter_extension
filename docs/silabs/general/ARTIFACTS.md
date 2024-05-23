# Matter Software Artifacts

This page provides links to pre-built software image artifacts that can be
used to set up the Matter Demo for the Thread and Wi-Fi use cases. The majority of these artifacts can be found under the "Assets" section on the release page here:

https://github.com/SiliconLabs/matter/releases/tag/v2.3.0-1.3-alpha.2

## Matter Hub Raspberry Pi Image

The Matter Hub image is intended to be flashed onto an SD card for a Raspberry
Pi. The Matter Hub Image provides both an Open Thread Border Router and the
Matter chip-tool. Note the image is ~10GB in size so depending on your internet
connection this download may take some time. Start the Matter Hub Raspberry Pi
image download here:

https://www.silabs.com/documents/public/software/SilabsMatterPi_2.3.0-1.3-alpha.2.zip

## Radio Co-Processor (RCP) Images

The Radio Co-Processor firmware is used to turn an EFR into an RCP that can be
used with a Raspberry Pi to allow the Raspberry Pi's Open Thread Border Router
to access the Thread network. Radio Co-Processor (RCP) images are available here:

https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/ot-rcp-binaries-2.3.0-1.3-alpha.2.zip

## Matter Accessory Device Images

The Matter Accessory Device Images are used to turn an EFR into a Matter device.
These are pre-built binary images for the Matter Demo. Matter Accessory Device
Images are located here:

https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/matter-accessory-device-images_2.3.0-1.3-alpha.2.zip

For Matter over Thread, 3 different types of images are provided:

1. **Standard**: Includes all code including enabling the LCD for a QR Code that can be used for commissioning.
2. **Release**: A smaller image size with reduced functionality, including removal of the LCD support, thus no QR Code.
3. **Sleepy**: A sleepy device image for improved energy efficiency and running on battery power.

## Matter Bootloader Binaries

If you are using the OTA functionality and especially if you are using an
EFR32MG2x device, you will need to flash a bootloader binary on your device along
with the application image. Bootloader binaries for all of the Matter supported
devices are available here:

https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/bootloader_binaries_2.3.0-1.3-alpha.2.zip

## RS9116 Firmware

The RS9116 firmware (rs9116_firmware_files_with_rev_2.3.0-1.3-alpha.2.zip) is used to update the RS9116 which can be found here:
https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/rs9116_firmware_files_with_rev_2.3.0-1.3-alpha.2.zip

**Note**:
RS9116 chip/module needs to be flashed with proper firmware as mentioned below:

- `RS916.x.x.x.x.x.rps - This firmware image is valid for RS9116 1.5 revision chip/module`
- `RS9116.x.x.x.x.x.rps - This firmware image is valid for RS9116 1.4/1.3 revision chip/module`

## SiWx917 Firmware for SiWx917 NCP

The SiWx917 firmware(SiWx917NCP_firmware_files_2.3.0-1.3-alpha.2.zip) is used to update the SiWx917 NCP which can be found here:

https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/SiWx917NCP_firmware_files_2.3.0-1.3-alpha.2.zip

**Note**:
SiWx917 NCP board need to be flashed with proper firmware as mentioned below:
- `SiWG917-A.2.9.X.X.X.rps - This firmware image is valid for BRD8036A (A0 Expansion v1.1) board`

- `SiWG917-B.2.X.X.X.X.rps - This firmware image is valid for BRD4346A board`

## SiWx917 Firmware for SiWx917 SoC

The SiWx917 firmware (SiWx917SOC_firmware_files_2.3.0-1.3-alpha.2.zip) along with WiSeConnect 3 SDK is used to update the SiWx917 SoC which can be found here:

https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/SiWx917SOC_firmware_files_2.3.0-1.3-alpha.2.zip

**Note**:
SiWx917 SoC boards need to be flashed with proper firmware as mentioned below:

- `SiWG917-B.2.X.X.X.X.rps - This firmware image is valid for BRD4338A(B0 common flash v2.0) board`

## SiWx917 SoC Configuration Files For JLink RTT Logging

The **JLinkDevices.xml** and **ELF** files referenced in the instructions may be found 
here:

https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/JLinkDevices.xml
https://github.com/SiliconLabs/matter/releases/download/v2.3.0-1.3-alpha.2/RS9117_SF_4MB_42bsp.elf

**Note**:- For EFR32MG2x devices, JLink RTT Logging support is already available.
