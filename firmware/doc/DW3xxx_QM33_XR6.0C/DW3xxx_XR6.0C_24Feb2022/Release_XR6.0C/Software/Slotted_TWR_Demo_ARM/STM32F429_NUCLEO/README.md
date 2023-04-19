# Slotted TWR Demo for NUCLEO-F429

## Target

This project targets the [NUCLEO-F429ZI development board] with a DW3000 EVB Arduino
shield. This project support the chip-down DW3000 Arduino shields (aka "red
boards") shipped with the "DW3000 PDoA kit and the [DWS3000 shield].

## Compiling, flashing and debugging the code using the Eclipse IDE

Eclipse with CDT and ARM cross GCC can be used to build the project. [System
Workbench for STM32][SW4STM32] is a convenient combination of both also
including ST-link debugging tools.

### Importing the project

- Open Eclipse/SW4STM32.
- Import the project as a makefile project: File > New > "Makefile Project with
  Existing Code".
- Under "project Name", enter a name of choice, e.g twr_demo.
- Under "Existing Code Location", select the location of the extracted twr_demo
  folder.
- Under "Toolchain for Indexer Settings", select "Ac6 STM32 MCU GCC", "ARM cross
  GCC" or "Cross GCC".
- Open the properties of the project (e.g right click > Properties).
- Select C/C++ Build.
- Under the "Builder Settings" tab, under "Build location", add "gcc" to the
  path, e.g ${workspace_loc:/twr_demo}/gcc.
- Click "Apply" followed by "OK".

The project can now be build using Project > "Build Project" or the "hammer"
icon.

### Flashing and debugging

The resulting twr_demo.hex binary can be flashed using the STM32 ST-LINK
utility, see section 6.2 in the DWS3000 Arduino shield user guide for more
information.

Alternatively and often more conveniently the project can be flashed and
debugged using an Eclipse CDT run configuration.
This can be either an "Ac6 STM32 Debugging" or "GDB OpenOCD Debugging" run
configuration.

To create a run configuration:

- Select "Run" > "Run Configurations...".
- Double click on "Ac6 STM32 Debugging" or "GDB OpenOCD Debugging".

#### Ac6 STM32 Debugging

- Under "C/C++ application", enter the location of the .elf binary e.g
  ${workspace_loc:twr_demo}/Debug/Debug_fw3/twr_demo.elf.
- Under Project *the project name, e.g "twr_demo", should be set.
- Under the "Debugger" tab, select "user Defined" and enter
  ${ProjDirPath}\NUCLEO_F429.cfg under "Script File".

Note: Search Project will not work since the Debug folder is not in the project
path (gcc).

The project can now be flashed using Run > Run or the "play" icon and debugged
using Run > Debug or the "bug" icon.

### GDB OpenOCD Debugging:

- Under Project the project name, e.g "twr_demo", should be set.
- Under "C/C++ application", enter the location of the .elf binary e.g
  ${workspace_loc:twr_demo}/Debug/Debug_fw3/twr_demo.elf.
- Under "Debugger" tab, in "Config options", enter
  "-f NUCLEO_F429.cfg -s${openstm32_openocd_script_root_path}".

Note: Search Project will not work since the Debug folder is not in the project
      path (gcc).

The project can now be flashed on the microcontroller using Run > Run or the
"play" icon. A debugging session can be started using Run > Debug or the "bug"
icon.

## Compiling the code using command-line tool

Make sure "make" and the "arm-none-eabi-" cross-toolchain are installed in a
location listed in the `PATH` environmental variable.

1. Build the code using shell

```
cd ./gcc

make all
# or
make fw3
```

Hint: Use the -j make flag for faster initial builds.

2. Build directory:

Debug/Debug_FW3

3. Build executables:

- twr_demo.elf: A binary executable containing debug symbols
- twr_demo.hex: An Intel hex formatted binary that can be flashed

# How to run

By default the "twr_demo" starts in the STOP operational mode.
From the STOP mode following commands will start specific applications:

- `tag` : starts the embedded TAG application (in blinking-discovery mode).
- `node`: starts the embedded NODE application. It will automatically start
          discovering tags. It will enable PDoA functionality when run with a
          PDoA capable DW3000 IC.
- `trilat`: starts the embedded NODE application and applies the trilateration
            on top of distance measurements.
- `listener`: starts the embedded LISTENER application. It will listen for UWB
              packets in the air using the current UWB configuration and
              report any packets that are heard.
- `uspi`: starts the embedded USB2SPI binary bridge application. This
          application can be used with the DecaRanging desktop application.

The `save` command saves the default mode of operation.
E.g If it executed while the `tag` application is running, on reset the `tag`
application will start by default.

The `stop` command can be used to stop any running application.

The `help` command can be used to get a list of available commands and to get
more information on the usage and functionality of the commands.

## Using the gui application

The DecaPDOARTLS gui application uses a device running the NODE application and
can be used together with one ore more devices running the tag application to
evaluate two way ranging and PDoA (on devices that support it).
See the DW3000 Arduino Shield QSG for more information.

## Using the cli applications

The NODE and TAG applications can be used stand alone using the uart command
line interface over USB. See the output of the `help` command and section 5 in
the DW3000 Arduino Shield QSG for more information.

## Selecting run mode using the user button

The blue user button (labelled "B1 USER") can be used to select the run mode on
reset:

- If the user button isn't pressed on reset, the code will start in the mode
  that was last saved using the `save` command.
- If the user button is pressed on reset for longer than 1 second, but less
  than 3 seconds, the TAG application will be started.
- If the user button is pressed on reset for 3 seconds or longer the NODE
  application will be started.

[NUCLEO-F429ZI development board]: https://www.st.com/en/evaluation-tools/nucleo-f429zi.html
[DWS3000 shield]: https://www.mouser.ie/new/qorvo/qorvo-dws3000-arduino-shield/
[SW4STM32]: https://www.openstm32.org/HomePage


/signed
11-Nov-2021
