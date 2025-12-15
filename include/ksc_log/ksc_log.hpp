#pragma once
#include <string_view>
#include <filesystem>

namespace ksc_log
{
	/// <summary>
	/// Info - Error - Warn - Debug. Everything less than or eqal to the level chosen will be logged.
	/// Ex: Warn means that warn, error, and info will be logged but not debug. Info logs only info and
	/// debug logs everything.
	/// </summary>
	enum class Level : unsigned char { Info = 0, Error = 1, Warn = 2, Debug = 3 };
	
	/// <summary>
	/// Writes a message to the open .log file with the level of INFO.
	/// Throws: logic_error
	/// </summary>
	/// <param name="MSG">Message to be put into the .log file.</param>
	void info(const std::string_view MSG);

	/// <summary>
	/// Writes a message to the open .log file with the level of WARN.
	/// Throws: logic_error
	/// </summary>
	/// <param name="MSG">Message to be put into the .log file.</param>
	void warn(const std::string_view MSG);

	/// <summary>
	/// Writes a message to the open .log file with the level of ERROR.
	/// Throws: logic_error
	/// </summary>
	/// <param name="MSG">Message to be put into the .log file.</param>
	void error(const std::string_view MSG);

	/// <summary>
	/// Writes a message to the open .log file with the level of DEBUG.
	/// Throws: logic_error
	/// </summary>
	/// <param name="MSG">Message to be put into the .log file.</param>
	void debug(const std::string_view MSG);
	
	/// <summary>
	/// Initializes the logging library.
	/// Throws: logic_error, invalid_argument
	/// </summary>
	/// <param name="pAPPLICATION_NAME_STR">Application name for use in logging and file naming.</param>
	/// <param name="APPLICATION_PATH_ROOT">The root path that will have a ../ROOT_PATH/logs directory created.</param>
	/// <param name="USE_TIMESTAMP">Should logs have a UTC timestamp?</param>
	/// <param name="LOGGING_LEVEL">What level (and lower) logs should be recorded? (See ksc_log::Level for more details)</param>
	void begin(const char *pAPPLICATION_NAME_STR, const std::filesystem::path APPLICATION_PATH_ROOT, const bool USE_TIMESTAMP,
			   const Level LOGGING_LEVEL);

	/// <summary>
	/// Call this at program exit deliberately. This allows for destructors to log depending on the program design. Closes the log file.
	/// Throws: logic_error
	/// </summary>
	void end(void);
}