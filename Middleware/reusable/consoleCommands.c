// ConsoleCommands.c
// This is where you add commands:
//		1. Add a protoype
//			static eCommandResult_T ConsoleCommandVer(const char buffer[]);
//		2. Add the command to mConsoleCommandTable
//		    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
//		3. Implement the function, using ConsoleReceiveParam<Type> to get the parameters from the buffer.

#include <string.h>
#include "main.h"
#include "at24c256_eeprom.h"
#include "consoleCommands.h"
#include "console.h"
#include "consoleIo.h"

extern sensor_data_t sensor_data_all;
extern calibration_t write_calibration;
extern calibration_t read_calibration;

#define IGNORE_UNUSED_VARIABLE(x)     if ( &x == &x ) {}

static eCommandResult_T ConsoleCommandComment(const char buffer[]);
static eCommandResult_T ConsoleCommandVer(const char buffer[]);
static eCommandResult_T ConsoleCommandHelp(const char buffer[]);
static eCommandResult_T ConsoleCommandDumpPMSensorData(const char buffer[]);
static eCommandResult_T ConsoleCommandDumpSGP30SensorData(const char buffer[]);
static eCommandResult_T ConsoleCommandSetSensorCalibration(const char buffer[]);
static eCommandResult_T ConsoleCommandGetSensorCalibration(const char buffer[]);

static const sConsoleCommandTable_T mConsoleCommandTable[] =
{
    {";", &ConsoleCommandComment, HELP("Comment! You do need a space after the semicolon. ")},
    {"help", &ConsoleCommandHelp, HELP("Lists the commands available")},
    {"ver", &ConsoleCommandVer, HELP("Get the current firmware version")},
    {"pm", &ConsoleCommandDumpPMSensorData, HELP("Dump particulate matter sensor data")},
    {"sgp30", &ConsoleCommandDumpSGP30SensorData, HELP("Dump SGP30 sensor data")},
    {"setc", &ConsoleCommandSetSensorCalibration, HELP("Set calibration value")},
    {"getc", &ConsoleCommandGetSensorCalibration, HELP("Get calibration value")},

	CONSOLE_COMMAND_TABLE_END // must be LAST
};

static eCommandResult_T ConsoleCommandComment(const char buffer[])
{
	// do nothing
	IGNORE_UNUSED_VARIABLE(buffer);
	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandHelp(const char buffer[])
{
	uint32_t i;
	uint32_t tableLength;
	eCommandResult_T result = COMMAND_SUCCESS;

    IGNORE_UNUSED_VARIABLE(buffer);

	tableLength = sizeof(mConsoleCommandTable) / sizeof(mConsoleCommandTable[0]);
	for ( i = 0u ; i < tableLength - 1u ; i++ )
	{
		ConsoleIoSendString(mConsoleCommandTable[i].name);
#if CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(" : ");
		ConsoleIoSendString(mConsoleCommandTable[i].help);
#endif // CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandDumpPMSensorData(const char buffer[])
{
	eCommandResult_T result = COMMAND_SUCCESS;

    // TODO: respect the data types in the initial sensor_data_t struct
    ConsoleIoSendString("PM1.0 Atmospheric: ");
    ConsoleSendParamInt16((int16_t )sensor_data_all.PM1_0_atmospheric);
    ConsoleIoSendString("\r\n");
    ConsoleIoSendString("PM2.5 Atmospheric: ");
    ConsoleSendParamInt16((int16_t )sensor_data_all.PM2_5_atmospheric);
    ConsoleIoSendString("\r\n");
    ConsoleIoSendString("PM10 Atmospheric: ");
    ConsoleSendParamInt16((int16_t )sensor_data_all.PM10_atmospheric);
    ConsoleIoSendString("\r\n");

    ConsoleIoSendString(STR_ENDLINE);
	return result;
}

static eCommandResult_T ConsoleCommandDumpSGP30SensorData(const char buffer[])
{
    eCommandResult_T result = COMMAND_SUCCESS;

    // TODO: respect the data types in the initial sensor_data_t struct
    ConsoleIoSendString("CO2 (equivalent): ");
    ConsoleSendParamInt16((int16_t )sensor_data_all.co2_eq);
    ConsoleIoSendString("\r\n");
    ConsoleIoSendString("TVOC (ppb): ");
    ConsoleSendParamInt16((int16_t )sensor_data_all.tvoc_ppb);

    ConsoleIoSendString(STR_ENDLINE);
    return result;
}

static eCommandResult_T ConsoleCommandSetSensorCalibration(const char buffer[])
{
    int16_t parameterInt;
    eCommandResult_T result;
    result = ConsoleReceiveParamInt16(buffer, 1, &parameterInt);
    if ( COMMAND_SUCCESS == result )
    {
        write_calibration.data.sgp30 = parameterInt;
        EEPROM_Write(0,0,write_calibration.bytes,sizeof(write_calibration.bytes));

        ConsoleIoSendString("Parameter is ");
        ConsoleSendParamInt16(parameterInt);
        ConsoleIoSendString(" (0x");
        ConsoleSendParamHexUint16((uint16_t)parameterInt);
        ConsoleIoSendString(") saved to EEPROM.");
        ConsoleIoSendString(STR_ENDLINE);
    }
    return result;
}

static eCommandResult_T ConsoleCommandGetSensorCalibration(const char buffer[])
{
    eCommandResult_T result = COMMAND_SUCCESS;
    IGNORE_UNUSED_VARIABLE(buffer);

    EEPROM_Read(0,0,read_calibration.bytes,sizeof(write_calibration.bytes));

    ConsoleIoSendString("Current SGP30 calibration value: ");
    ConsoleSendParamInt16(read_calibration.data.sgp30);
    ConsoleIoSendString(STR_ENDLINE);

    return result;
}

static eCommandResult_T ConsoleCommandVer(const char buffer[])
{
	eCommandResult_T result = COMMAND_SUCCESS;

    IGNORE_UNUSED_VARIABLE(buffer);

	ConsoleIoSendString(FW_VERSION);
	ConsoleIoSendString(STR_ENDLINE);
	return result;
}


const sConsoleCommandTable_T* ConsoleCommandsGetTable(void)
{
	return (mConsoleCommandTable);
}


