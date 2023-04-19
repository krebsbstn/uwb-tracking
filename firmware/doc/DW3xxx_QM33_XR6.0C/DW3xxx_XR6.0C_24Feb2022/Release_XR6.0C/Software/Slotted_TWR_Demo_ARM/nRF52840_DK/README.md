# Slotted TWR Demo for nRF52840-DK

## Target

This project targets the [nRF52840-DK development board] with a DW3000 EVB Arduino
shield. This project support the chip-down DW3000 Arduino shields (aka "red
boards") shipped with the "DW3000 PDoA kit and the [DWS3000 shield].

## Compiling, flashing and debugging the code using the Segger ES IDE
1. Load the project into Segger ES for ARM:
    File->Open Solution->twr_demo_nRF52840.emProject
2. Compile the project:
    Build->Build twr_demo_nRF52840 (F7)
3. Build directory:
    twr_demo\Output\Release\Exe\
    twr_demo\Output\Debug\Exe\
4. Build executable
    twr_demo_nRF52840.elf
    twr_demo_nRF52840.hex
5. How to run:
    By default the "twr_demo" starts in the STOP operational mode.

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

The user button 1 can be used to select the run mode on reset:

- If the user button isn't pressed on reset, the code will start in the mode
  that was last saved using the `save` command.
- If the user button is pressed on reset for longer than 1 second, but less
  than 3 seconds, the TAG application will be started.
- If the user button is pressed on reset for 3 seconds or longer the NODE
  application will be started.


/signed
05-Oct-2021