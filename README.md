# TTGO Tbeam Ultra Low Power Modification
 
 This page describes two (2) simple modifications which can significantly decrease the power consumption of the TTGO Tbeam boards based on the ESP32 both in normal operation and when in deep sleep.  These regions on the main board (T22_V2) for the two modifications are shown below in Figure 1.  Each of these proposed changes and their impact are described in more detail below.  
 
<b>_PLEASE NOTE:  These modification are for experienced hackers only.  Please do not attempt these changes unless you truely understand what I am proposing, understand how to perform the changes and understand the risks.  As these are hardware changes, if you make a mistake you can permanently damage your board._</b> 
 
 
 <div style="float: left  color: blue font-style: italic">
<img src="images/Fig0_TTGOlayout.png" alt="Figure 1. TTGO Tbeam showing the regions of the board with the proposed modification detailed below."  align="center" width="800"/></div>  <figcaption > <I><b>Figure 1: </b></I> TTGO Tbeam Image modifed from [original image]( https://github.com/LilyGO/TTGO-T-Beam/blob/master/images/image1.jpg ) showing the regions of the board with the proposed modification detailed below.

 # Background
 The TTGO is an extremely versatile board, providing WiFi, bluetooth, LoRa radio, GPS modules and I/O under the control of an ESP32 processor.  The entire board is powered from a [18650 LiPo which nominally is rated at 2600 mA.hr](https://www.jaycar.com.au/18650-rechargeable-li-ion-battery-2600mah-3-7v/p/SB2308).  However during deep sleep the best that has been [reported is around 10 mA](https://github.com/JoepSchyns/Low_power_TTGO_T-beam), so that the maximum operational time is just over 10 days before some form of recharging is required.  With the changes outlined here to significantly reduce the power of this board, this operational time could be extended to over a year.<br>
 <b>There are two modifications required to lower the power of the TTGO boards. </b> <I>
 * The first allows complete shutdown of the [NEC 6 GPS](NEO-6_DataSheet_(GPS.G6-HW-09005).pdf), USB Controller and LoRa board during operation, so that only the ESP32 power consumption dominates.  
 * The second allows shut down off the SPRAM and Flash attached to the [ESP32](esp32_datasheet_en_ELEC_SPECS.pdf) so that during deep sleep, minimal power is consumed.  With these modifications, a maximum of 170 &#181;A is consumed during deep sleep, considerably less than the nominal [10 mA reported](https://github.com/JoepSchyns/Low_power_TTGO_T-beam),</I><br>
<br>

Deep sleep modes are detailed in the [espressif page](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/sleep_modes.html) which describe how to shut down the ESP 32 into low power mode with all but the RTC operational.  The RTC needs to be active for the processor to wake on a timed interrupt.  The [ESP32 Electrical datasheet](esp32_datasheet_en_ELEC_SPECS.pdf) indicates the power consumption in the various modes as shown in the table below:
 
 <div style="float: left  color: blue font-style: italic">
<img src="images/Table1PowerConsumptionModes.png" alt="Table 1. Extracted from the Espressif Systems ESP32 Datasheet V3.3 where it was originally designed "Table 6: Power Consumption by Power Modes" on page 21 of that document."  align="center" width="800"/></div>  <figcaption > <I><b>Table  1: </b></I>Power Consumption of the ESP32 processor.  This table was extracted from the Espressif Systems ESP32 Datasheet V3.3 where it was originally designed "Table 6: Power Consumption by Power Modes" on page 21 of that document.
 
## Part #1: Low power during operation (GPS/USB/LoRa Off)

The [ME6211](ME6211C33M5G_N_C82942.pdf) is a a High Speed LDO Regulator and the [TTGO schematic](t22_gps_v07.pdf) includes 2 of these
* one (U8) to generate a 3.3V that powers the ESP32 and attached memory and 
* one (U5) that generates 3.3V (labelled <b>VCC_3.3V</b>) to power the GPS, USB I/O controller and LoRa board.  <br>

Both these ME6211 regulators contain an enable pin that is pulled high to keep the regulators on after power up.   During a regular boot sequence, the GPS is on and looks for satellites.  This typically draws 180&plusmn;10 mA and lock can take around 2 minutes.  After lock the current consumption is still around 100mA.  While the GPS can be shut down in software, it is difficult to lower the current consumption much less than 90 mA during regular operation.  
In this <b>first approach to lowering the power consumption </b> the enable pin of IC U5 (the  ME6211 that generates  VCC_3.3V) is connected to GPIO21 to allow the power supplied to the GPS/LoRa/USB controller to be placed under software control.  This provides much greater flexibility when determining which devices should be powered up and when.
<P></P>
The physical layout of the TTGO Tbeam T22_V22 PCB is shown in Figure 2a with the 10k&Omega; resistor R27 identified next to the [ME6211](ME6211C33M5G_N_C82942.pdf) SMD. It is possible to undertake this modification without removing R27, however when the ME6211 enable pin is LOW, this causes an additional 0.5mA to flow which may not be significant during operation, but is a sizeable proportion of the total current in deep sleep mode. To remove this surface mount resistor I suggest using a small amount on regular lead-tin solder with a flux resin to allow even flow across the small component.  I performed this task with a regular electronics style 
[soldering iron]( https://www.youtube.com/watch?v=8JM4oCpWnjU ), but it required a magnifying lens with a good light during the process.  After R27 is removed, a small piece of solid core [Kynar wire]( https://www.jaycar.com.au/red-wire-wrap-wire-on-spool/p/WW4344 ) (sometimes called wire-wrap wire) was used to connect from ME6211 Pin 3 to GPIO21 on the header connector.  Once used, GPIO21 should not be used for another other task.  The PCB after R27 removal and the addition of the wire (red) is shown in Figure 2b.  The corresponding circuit before and after the PCB modification is shown in Figure 2c and 2d, respectively.

<div style="float: left  color: blue font-style: italic">
<img src="images/Fig1_R27.png" alt="Figure 2. Physical layout of the TTGO Tbeam T22_V22 PCB showing (a) before and (b) after modification of the circuit to remove resistors R27 and add a jumper from Pin 3 of ME6211 (EN) to IO21.  This circuit schematic (c) before and (d) after modification is also shown."  align="center" width="1000"/></div>  <figcaption > <I><b>Figure 2: </b></I>Physical layout of the TTGO Tbeam T22_V22 PCB showing (a) before and (b) after modification of the circuit to remove resistors R27 and add a jumper from Pin 3 of ME6211 (EN) to IO21.  This circuit schematic (c) before and (d) after modification is also shown.

The results indicate:

State | Operation Mode |  Measured Current |
----- | ---------------| ----------------- |
BEFORE PCB MOD | OPERATION (GPS ON) | 170	&plusmn;5 mA |
BEFORE PCB MOD   | SLEEP (GPS ON)     |  96	&plusmn;3 mA |
BEFORE PCB MOD   | OPERATION |  70	&plusmn;2 mA |
BEFORE PCB MOD   | SLEEP     |  12.5	&plusmn;1 mA |
AFTER PCB MOD   | OPERATION |  63	&plusmn;2 mA |
AFTER PCB MOD   | SLEEP     |   2.1	&plusmn;0.2 mA |

Using  the <b>GPSFLAG = true</b> in the uploaded code <code> ARDUINO.INO</code> the board draws significant current prior to removal of the R27 resistor.  This drops considerably when <b>GPSFLAG = false</b> with the GPS being forced off.  However there is still considerable current drawn even in this state.
As can be seen, in the deep sleep mode, the current consumption has dropped to nominally 2.1mA after the removal of R27 and ME6211 (EN Pin 3) to GPIO21.  This result is an improvement over existing approaches and indicates a substantial current draw from GPS/USB-Controller/LoRA occurs during the deep sleep of the ESP32 when these elements are still powered.
## Part #2: Ultra Low power during deep sleep (SPRAM and Flash Off)
During extensive software testing, no conditions could be found to lower the power below 2.1mA in deep sleep.  It turns out the main schematics for the TTGO Tbeam is very similar to that used for the [ESP32 wroom-32](esp32-wroom-32_datasheet_en_with_SCHEMATICS.pdf) a generic Wi-Fi+BT+BLE MCU module.  From those schmatics, it appears that the external SPRAM and Flash could be shut down by powering both via <b>VDD_SDIO</b> which is a voltage generated through the ESP32 itself (it contains an internal LDO regulator).  However, on the [TTGO Tbeam](https://github.com/LilyGO/TTGO-T-Beam) board the <b>VDD_SDIO is directly connected to 3.3V</b>.  This 3.3V is the main battery supplied voltage, which is always on even during sleep.  As stated in [Section 2.3 Power Scheme of the ESP32 datasheet](esp32_datasheet_en_ELEC SPECS.pdf) <br>
><b> When VDD_SDIO is connected to the same PCB net together with VDD3P3_RTC, the internal LDO is disabled automatically.  </b><br>

Hence connecting VDD_SDIO directly connected to 3.3V over-rides (bypasses) the internally generated VDD_SDIO voltage (set to 3.3V), so that external SPRAM and Flash (and the associated resistor R35, see [schematic](t22_gps_v07.pdf)) are all drawing current during deep sleep.  However this connection from VDD_SDIO to 3.3V is done through  R46 (a 0&Omega; resistor) so I investigated removing the reisstor to determine the possible reduction in current during deep sleep. 
<div style="float: left  color: blue font-style: italic">
<img src="images/Fig2_VDD_SDIO_Circuit.png" alt="Figure 3. Relevant components connected to VDD_SDIO on the TTGO Tbeam T22_V22 circuit schematic.  Not shown in the link the shorts VDD_SDIO to 3.3V on the board through R46 ( a 0&Omega; resistor)."  align="center" width="1000"/></div>  <figcaption > <I><b>Figure 3: </b></I> Relevant components connected to VDD_SDIO on the TTGO Tbeam T22_V22 circuit schematic.  Not shown in the link the shorts VDD_SDIO to 3.3V on the board through R46 ( a 0&Omega; resistor).<br>

Using the same approach as described in Figure 2, a [standard soldering iron with a small amount of tin-lead solder with a resin flux](https://www.youtube.com/watch?v=8JM4oCpWnjU) was used to remove R46.  The physical layout of the board is shown in Figure 4a before and 3b after removing of the resistor.  In this image I have also added a small wire to VDD_SDIO to monitor what happens during deep sleep.  Sure enough , when the code enters a deep sleep using <code> esp_deep_sleep_start(); </code> the voltage VDD_SDIO drops to around 250mV, and jumps up to 3.3 V when the board exits from sleep mode.  The relevant section of the [circuit schematic](t22_gps_v07.pdf) have been extracted and shown in Figure 4c and 4d, before and after the modification, respectively.
<div style="float: left  color: blue font-style: italic">
<img src="images/Fig3_R46.png" alt="Figure 4. Relevant components connected to VDD_SDIO on the TTGO Tbeam T22_V22 circuit schematic.  Not shown in the link the shorts VDD_SDIO to 3.3V on the board through R46 ( a 0&Omega; resistor)."  align="center" width="1000"/></div>  <figcaption > <I><b>Figure 4: </b></I>Relevant components connected to VDD_SDIO on the TTGO Tbeam T22_V22 circuit schematic.  Not shown in the link the shorts VDD_SDIO to 3.3V on the board through R46 ( a 0&Omega; resistor).

The results indicate:

State | Operation Mode |  Measured Current |
----- | ---------------| -----------------|
BEFORE PCB MOD | OPERATION |180&plusmn;10mA |
BEFORE PCB MOD |   SLEEP   |  12&plusmn;1mA |
AFTER PCB MOD  |  OPERATION |56&plusmn;1mA |
AFTER PCB MOD  | SLEEP  |   170&plusmn;20 &#181;A |

I note that when the entire power to the board is turned off via switch <b>2JP1</b> in the [schematic](t22_gps_v07.pdf), the current drawn is only 70 &#181;A which is associated with the [TP5400, a 1A lithium battery and 5V / 1A boost control chip]( Translated_TOPPOWER-Nanjing-Extension-Microelectronics-TP5400_C24154.zh-CN.en.pdf) used to convert the [18650 LiPo battery](https://www.jaycar.com.au/18650-rechargeable-li-ion-battery-2600mah-3-7v/p/SB2308) from 3.7 V to 5 V.  The maximum Input supply current for this is 100  &#181;A so the measured value of 70 &#181;A is reasonable. Assuming the same current is consumed when the main power switch <b>2JP1</b> is on, then given the measured total of 170  &#181;A when the ESP32 is in deep sleep mode, I can assume that 100  &#181;A is associated with the ESP32 and 70  &#181;A is associated with the TP5400 power control chip.  Reviewing the power listed in Table #1 above from the Espressif Systems ESP32 Datasheet V3.3, this measured value of  100  &#181;A is expected from the ESP32 when in deep sleep but with the ULP enabled  (which it is as the processor is this work is arranged to wake on a timed interrupt with ULP and RTC).  Hence I believe there is not much more that can be squeezed out of the circuit to minimize the current.  Even if we shut down the ULP, the current consumption is dominated by the 70 &#181;A from the TP5400, power control chip, so that no more than a x0.6 reduction in total current could be achieved.  Given I have dropped the current from the previous reported values of [10mA](https://github.com/JoepSchyns/Low_power_TTGO_T-beam) to 0.17 mA (a factor of 58) a reduction of x0.6 is hardly worth the effort for further circuit modifications.  If a software solution could be found to further lower the current, I would be interested to see the code.
