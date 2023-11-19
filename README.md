### Prerequisites

- CMake
- ARM GNU GCC Tools
- MinGW

### Build the project (Manual)

- Create a build folder (ex. cmake-build-debug) and cd to that folder

- Run `cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi-gcc.cmake -DCMAKE_BUILD_TYPE=Debug ..`

- Then, run `cmake --build . -- -j 4`

- You should obtain .bin and .hex files for your target

### Run project

- Use STM32CubeProgrammer to program the device.

### Build the project (with CLion)

- Open the folder containing CMakeLists.txt and *.cmake file with CLion.
- In File -> Settings -> Build, Execution, Deployment section, add the following to CMake options field: `-DCMAKE_TOOLCHAIN_FILE=arm-none-eabi-gcc.cmake`
- CLion should reload the CMake project without any errors.
- Build the project

#### Debug using OpenOCD and CLion

- Create a new run configuration (Run -> Edit Configurations)
- Add a new OpenOCD Download & Run configuration
- In the board config file field, supply the appropriate configuration file. Ex. https://github.com/ntfreak/openocd/blob/master/tcl/board/st_nucleo_f4.cfg
- Also make sure to fill the Executable field ex. iaq_monitor.out file in the build folder
- Pressing the Run button should now flash the target
- To debug, place a breakpoint and click on the green bug (debug) icon. The debugger should stop at the breakpoint

#### Wiring 

##### UART
UART2 => PD5 (TX), PD6 (RX) => USB to UART (RaspberryPi Debugger Yellow PD5 / Orange PD6 ) => PC
UART3 => printf redirect to ST-Link / Virtual COM port => PC
UART5 => PC12 (TX), PD2 (RX) => PMS5003 (Green jumper cable (on PD2 / RX) to 'RX' on sensor breakout board)

##### Sensor Breakout Breadboard
I2C1 => SCL (PB8) Yellow jumper cable, SDA (PB9) Green jumper cable.
Go to the breadboard.
3.3V and GND supplied from CN8 connectors to breadboard rails. 

### Troubleshooting

- Undefined reference to _sbrk: https://stackoverflow.com/questions/5764414/undefined-reference-to-sbrk

### Sources

- https://github.com/posborne/cmsis-svd/blob/master/data/STMicro/STM32F401.svd
- https://dev.to/younup/cmake-on-stm32-the-beginning-3766
- https://dev.to/younup/cmake-on-stm32-episode-2-build-with-clion-2lae
- https://dev.to/younup/recompiler-vos-projets-c-et-c-plus-rapidement-avec-ccache-23in