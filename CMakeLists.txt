cmake_minimum_required(VERSION 3.15.3)

project(iaq_monitor)

enable_language(C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

#enable_language(CXX C ASM)
#set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CCACHE_PROGRAM ccache)

if (CCACHE_PROGRAM)
        message(NOTICE "ccache is enabled (found here: ${CCACHE_PROGRAM})")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "\"${CCACHE_PROGRAM}\"")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "\"${CCACHE_PROGRAM}\"")
else ()
        message(FATAL_ERROR "ccache has not been found")
endif ()

set(STM32_HAL_DRIVERS

        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c

        Drivers/BSP/STM32F4xx_Nucleo_144/stm32f4xx_nucleo_144.c

        Middleware/reusable/console.c
        Middleware/reusable/consoleIo.c
        Middleware/reusable/consoleCommands.c

        App/Inc/stm32f4xx_hal_conf.h
        App/Inc/stm32f4xx_it.h
        App/Inc/main.h
        App/Inc/debug.h
        App/Inc/config.h

        App/Src/main.c
        App/Src/debug.c
        Middleware/retarget/retarget.c
        App/Src/stm32f4xx_hal_msp.c
        App/Src/stm32f4xx_it.c
        App/Src/system_stm32f4xx.c
		
        startup_stm32f439xx.s

)

# To disable compiler warnings, etc. for certain files (3rd party files mainly)
set_source_files_properties(
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
        Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
        PROPERTIES
        COMPILE_FLAGS -Wno-unused-parameter
)

set(EXECUTABLE ${PROJECT_NAME}.out)

add_executable(${EXECUTABLE} ${STM32_HAL_DRIVERS}
        #source/retarget.c
)

target_compile_definitions(${EXECUTABLE} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32F439xx
)

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE # adding system will also discard warnings from header files
        App/Inc
        Middleware/reusable
        Middleware/retarget
        Drivers/STM32F4xx_HAL_Driver/Inc
        Drivers/CMSIS/Device/ST/STM32F4xx/Include
        Drivers/CMSIS/Include
        Drivers/BSP/STM32F4xx_Nucleo_144
)

target_include_directories(${EXECUTABLE} PRIVATE
        #source
)

target_compile_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard

        -fdata-sections
        -ffunction-sections

        -Wall
        -Wextra
        -pedantic

        #-fno-exceptions # C++ specific flag
        #-fno-rtti # C++ specific flag
        $<$<COMPILE_LANGUAGE:CXX>:@${CMAKE_SOURCE_DIR}/gcc-options-cxx.txt> # add C++ options (including the two above)

        $<$<CONFIG:Debug>:-Og> #  If CONFIG is Debug, then add option -Og, otherwise add nothing
        )

target_link_options(${EXECUTABLE} PRIVATE
        -T${CMAKE_SOURCE_DIR}/STM32F439ZITx_FLASH.ld
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -specs=nano.specs
        -specs=nosys.specs # added to fix _sbrk not found issue
        -lc
        -lm
        -lnosys
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
        )

# Print executable size
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-size ${EXECUTABLE})

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-objcopy -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND arm-none-eabi-objcopy -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)

# Improve clean target
set_target_properties(${EXECUTABLE} PROPERTIES ADDITIONAL_CLEAN_FILES
        "${PROJECT_NAME}.bin;${PROJECT_NAME}.hex;${PROJECT_NAME}.map")