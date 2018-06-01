Deployment
==========

wp8548 + mangOH Green
---------------------

Get the latest release (15 as the writing of this document) from source.sierrawireless.com:
https://source.sierrawireless.com/resources/airprime/software/wpx5xx/wpx5xx-firmware-latest-release

1/ Fetch:
- a toolchain from https://source.sierrawireless.com/resources/airprime/software/wpx5xx/wpx5xx-firmware-latest-release-components
- the full device image: `WPx5xx_Release15_GENERIC_SPK.spk`

1/ Install the toolchain by running the script.

2/ Flash the device with the device image.
(use Legato `fwupdate` for instance: `fwupdate 192.168.2.2 WPx5xx_Release15_GENERIC_SPK.spk`)
After a few minutes your device should be up (check with `cm info` from the target).

3/ Build app by doing `make wp85`.
This should produce a `trafficLight.wp85.update` and a `trafficLight.wp85.cwe`.

3/ Update the device, either with :
  - a software update: `update 192.168.2.2 trafficLight.wp85.update`
or
  - a firmware update: `fwudpate 192.168.2.2 trafficLight.wp85.cwe`

4/ Configure the app:

For Sensu:
```
config set trafficLight:/url "http://<sensu host>/uchiwa/metrics"
config set trafficLight:/info/content/checkFlag true bool
config set trafficLight:/info/content/checkMode sensu
```

For Jenkins:
```
config set trafficLight:/url "http://<jenkins host>/job/<job>/lastCompletedBuild/api/xml"
config set trafficLight:/info/content/checkFlag true bool
config set trafficLight:/info/content/checkMode jenkins
```

