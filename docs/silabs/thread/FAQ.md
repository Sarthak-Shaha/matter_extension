# Frequently Asked Questions for Matter over Thread

## Demo

-   Why are the `mattertool` commands not working after all the steps?

    -   Check if the Radio Co-Processor (RCP) image was built and/or
        flashed correctly to the device.
    -   Make sure you see a QR code on the display of the Matter Accessory
        Device (MAD).
    -   Make sure the images being used to flash the Raspberry Pi, RCP and MAD
        are correct.

<br>

-   How can I find the IP address of my Raspberry Pi?

    -   First, make sure the Raspberry Pi is connected to a network (ethernet or
        Wi-Fi). This page has more information:
        [Setting up the Matter Hub (Raspberry Pi)](RASPI_IMG.md)
    -   Refer to this page for general questions on finding the Raspberry Pi on
        your network: [Finding your Raspberry Pi](../general/FIND_RASPI.md)
    -   For more detailed information, refer to this page:
        [Raspberry Pi Remote Access](https://www.raspberrypi.com/documentation/computers/remote-access.html)

- How can I use a crystal value different than the default (39.0 MHz) for my device?

  - When using an alternative crystal value (i.e.: different than 39.0 MHz), updating the clock speed for both the HFXO and DPLL values is needed and both must match, where the DPLL is set to 2x the HFXO value. These values are used by the RAIL library to determine the radio frequency and the proper timings for the BLE packets.
  - If the DPLL value is left unchanged with a modified HFXO, the radio will be on the right frequency but the BLE packet timings will not be correct, which will cause issues within a few packets due to BLE's strict timing requirements.

<br>
