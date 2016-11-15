![watz energy monitor](http://cloud.scott.ee/images/watz.png)

# watz

* Status: ✅ Active
* Contributors: [@scottsweb](http://twitter.com/scottsweb)
* Description: A WiFi smart meter / pulse counter built upon a Spark / Particle Core.
* Author: [Scott Evans](http://scott.ee)
* Author URI: [http://scott.ee](http://scott.ee)
* License: GNU General Public License v2.0
* License URI: [http://www.gnu.org/licenses/gpl-2.0.html](http://www.gnu.org/licenses/gpl-2.0.html)

## About

watz is a WiFi smart meter / pulse counter. It mounts to the LED on your electricity meter and measures the number of pulses in a given time period. This is used to generate real time power information. This data is periodically pushed to the Particle cloud and can be subscribed to via [server-sent](https://docs.particle.io/reference/firmware/core/#particle-publish-) events.

The events are:

* `watz` - pushed every 10 minutes, contains a `kW` reading and a pulse `count`
* `watzup` - pushed every time 1000 pulses are detected to allow tracking of the meter total
* `watzbatt` - pushed every 10 minutes and uses the official power shield to grab batter information

To log the data I am using the hosted [emoncms service](http://emoncms.org). I relay the data via
[Node-RED](http://nodered.org/) on a Raspberry Pi using the [Particle](http://flows.nodered.org/node/node-red-contrib-particle) and [emoncms](http://flows.nodered.org/node/node-red-node-emoncms) nodes:

![watz node red](http://cloud.scott.ee/images/watz-node-red.png)

When the `kW` data arrives with emoncms you will need to multiply it by 1000 to get the current watts value. After a bit of tweaking your electricity dashboard should looks something like this:

![watz data in a graph](http://cloud.scott.ee/images/watz-output.png)

Issues/Problems/Questions? [Open a GitHub issue](https://github.com/scottsweb/watz/issues). You can also contact me via [scott.ee](http://scott.ee) or [twitter (@scottsweb)](http://twitter.com/scottsweb).

## Hardware

* [Particle Photon](https://www.particle.io/)
* [Particle Power Shield](https://docs.particle.io/datasheets/particle-shields/#power-shield)
* [TSL251R-LF 3.3v Photodiode](http://uk.farnell.com/ams/tsl251r-lf/photodiode-sensor-l-volts/dp/1182347)
* A power source (I went for battery + solar as my meter is outside the house)

## Circuit

![watz breadboard made with Fritzing](https://raw.githubusercontent.com/scottsweb/watz/master/watz.png)

The included watz.fzz file can be opened in [Fritzing](http://fritzing.org/). Replace the transistor with the TSL251R-LF. Version 2.0 of the code the board requires the Photon and the TSL251R-LF to be attached to the WKP pin to bring the board out of deep sleep.

## Installation

Copy the contents of `watz.ino` into the [Particle Build IDE](https://build.particle.io/build/) and flash your Photon.

## Further Reading

* http://openenergymonitor.org/emon/buildingblocks/introduction-to-pulse-counting
* http://www.reuk.co.uk/Flashing-LED-on-Electricity-Meter.ht
* http://jeelabs.net/projects/cafe/wiki/Electricity_consumption_meter
* http://www.airsensor.co.uk/component/zoo/item/energy-monitor-mk2.html

## Changelog

### 2.0
* Upgraded to Photon board
* Added support for Power Shield
* Battery life monitoring 
* Deep sleeping to improve battery performance
* SEMI_AUTOMATTIC to have better control over cloud connection

### 1.0
* Initial release
