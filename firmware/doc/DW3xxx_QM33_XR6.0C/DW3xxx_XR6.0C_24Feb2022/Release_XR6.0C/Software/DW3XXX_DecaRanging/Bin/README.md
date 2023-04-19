# DecaRanging 5.02 release note

DecaRanging_5p02.exe
- Version number changed to 5.02 0 Adding definition of DWT_PLEN_72 in logging function
- Updated the driver to 06.00.04
- Supports of all DW3000 series chip

Previous releases

- Version number changed to 5.01 -  Fixed bug re register access
- Updated the driver to 06.00.02
- Version number changed to 5.00 - supports all DW3xxx family devices
- Updated the driver to 05.07.00
- Removed direct register read/writes form the application files. All access is now via the relevant APIs.
- Modified the Configure menu to make it more obvious that SDC and STS Length are separate fields.
- Fixed a number of warnings throughout the code.
- Added a handling of erroneous PHD event in the ISR, i.e. when an error in PHR is not detected and PHD event is raised but also frame length is 0. The ISR will now call RX error handler instead of RX OK.
- Added QM33100 driver files to the project. And sorted files into separate folders/filters in the Solution Explorer.
- Added a _DBG_LOG preprocessor definition to the project, and this will make sure that any specific debug/logging driver code will not be automatically included in the low-level driver. 
- Update the RX level (RSL) formula for QM33120 devices, the instancecalculatepower() adjusts automatically depending whether it is a DW3000 or QM33100device.
- Allow inter frame period > 4.2ms when using Continuous Frame.
- Disable automatic XTAL trim function. 
- Fix the reportPDOA() function, the TDoA filter - removal of PDoA when TDoA > 100 units, was not working correctly. The TDoA value was not correctly calculated, which meant that the filter was discarding good PDoA results.
- Version number changed to 3.19
- Fixed instancelowlevelwritereg function, it was passing incorrect parameters to dwt_writetodevice() API.

signed
30/08/2021
