# Hantek DSO4202C (049f:505a) Screenshot Tool

I needed to capture the display of my oscilloscope for a school assignment, but
surprisingly I couldn't find any Linux tools for this. Fortunately, someone had
already [reversed and documented](https://elinux.org/Das_Oszi_Protocol) the
proprietary Hantek usb protocol. This was my first experience with both `libusb-1.0`
and `libpng`.

I only implemented the screenshot command, and it writes the results to
`test.png` every time. I got what I needed and ceased development, but feel free
to extend, copy, or modify my code.

## Sample Screenshots
<center><img src=screenshot.png></center>

<center><img src=wavegen.png></center>
