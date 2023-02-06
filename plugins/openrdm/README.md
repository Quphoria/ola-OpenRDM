OpenRDM Plugin
===============================

This plugin allows OLA to use a ftdi USB-RS485 module as a DMX output with RDM support.

## Config file: `ola-openrdm.conf`

`enabled = true`
Enable this plugin (DISABLED by default).

`output_ports = <int>`  
The number of output ports to create.

`port_N_device = <FTDI device string>`
The FTDI device string to use for port N. Example device string: s:0x0403:0x6001:00418TL8

`port_N_dmx_refresh_ms = <int>`
The time in milliseconds for DMX value rebroadcasts to occur on port N.

`port_N_rdm_enabled = [true|false]`
Enable RDM on port N.