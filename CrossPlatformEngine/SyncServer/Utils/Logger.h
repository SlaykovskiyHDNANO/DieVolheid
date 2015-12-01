#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <ostream>
#include <string>
#include <vector>
#include <ios>
#include <chrono>
#include <sstream>
#include <ctime>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <fstream>

// Use the namespace you want
namespace utils {
	// If you can´t/dont-want-to use C++11, remove the "class" word after enum
	enum LogType { LOG_CRITICAL_ERROR = 4, LOG_ERROR = 2 , LOG_WARNING = 1, LOG_INFO = 0 };
	struct LogOptions {
		std::string filename;
		std::string extension;
		std::string version;
		bool		append_version_to_filename;
		bool		append_date_to_filename;
		LogType		log_level;
		bool		append_date;

		LogOptions(const std::string& filename, const  std::string& extension = ".txt", const std::string& version = "0.1a", const LogType& log_level = LogType::LOG_INFO,
				   const bool& append_date_to_filename = true, const bool& append_version_to_filename = true, bool append_date = true):
				   filename(filename), extension(extension), version(version), append_date_to_filename(append_date_to_filename), 
				   append_version_to_filename(append_version_to_filename), log_level(log_level), append_date(append_date)
		{
		}

		
		
	};

	class LogEntry:public std::wstringstream {
		
	public:
		LogType		severity;
		std::string origin;
		explicit LogEntry(const std::string& message = "", const std::string& origin = "", const LogType &severity = LogType::LOG_INFO);

		static std::string GetTime(const char* format);

		LogEntry(const LogEntry&) = delete;
	};
	

	//make it Singleton
	class Logger:private std::wofstream {
		
	private:
		std::vector<LogEntry>	entries;
		std::recursive_mutex 	mutex;
		bool					capture_entries;
		bool					write_in_std;
		LogType					last_type;
		std::string				fname;
		LogOptions				opts;
		bool					omit;
		unsigned int            numWarnings;
		unsigned int            numErrors;
	public:
		// ctor (remove parameters if you don´t need them)
		explicit Logger(const LogOptions& lopts, bool write_in_std_too = true, bool replace_static_log_ptr = true);
		//access to one log for all
		static Logger* Log;

		void start();

		void end();
		// dtor
		~Logger();


		

		// Overload << operator using log type
		friend Logger& operator << (Logger &logger, const LogType &l_type) {
			std::lock_guard<std::recursive_mutex> lg(logger.mutex);
			if ((logger.omit = l_type < logger.opts.log_level))
				return logger;
			std::wstringstream ss;

			switch (l_type) {
			case LogType::LOG_CRITICAL_ERROR:
				ss << "[CRITICAL ERROR]";
			case LogType::LOG_ERROR:
				ss << "[ERROR]: ";
				++logger.numErrors;
				break;

			case LogType::LOG_WARNING:
				ss << "[WARNING]: ";
				++logger.numWarnings;
				break;

			default:
				ss << "[INFO]: ";
				break;
			} // sw
			//append date
			if (logger.opts.append_date) {
				auto tmp = LogEntry::GetTime("%Y-%m-%d %H:%M:%S");
				ss << "[time: " << std::wstring(tmp.begin(), tmp.end()) << "]:" << std::endl;
			}
			if (logger.write_in_std)
				std::wcout << ss.str();
			logger << ss.str();

			return logger;

		}
		template<typename T>
		Logger& operator<<(const T &val){
			std::lock_guard<std::recursive_mutex> lg(this->mutex);
			
			*((std::wostream*)this)<<(val);
			if (this->write_in_std)
				std::wcout << val;
			return *this;
		}
		template<>
		Logger& operator<<(const std::string &val){
			std::lock_guard<std::recursive_mutex> lg(this->mutex);

			
			*((std::ostream*)this) << (val.c_str());
			if (this->write_in_std)
				std::wcout << val.c_str();
			return *this;
		}

		/*Logger& operator<<(const char *val){
			std::lock_guard<std::recursive_mutex> lg(this->mutex);


			*((std::ostream*)this) << (val);
			if (this->write_in_std)
				std::wcout << val;
			return *this;
		}*/

		// this is the type of std::cout
		typedef std::basic_ostream<wchar_t, std::char_traits<wchar_t> > CoutType;

		// this is the function signature of std::endl
		typedef CoutType& (*StandardEndLine)(CoutType&);

		// define an operator<< to take in std::endl
		Logger& operator<<(StandardEndLine manip);


		friend Logger& operator << (Logger &logger, const LogEntry& lentry){
			std::lock_guard<std::recursive_mutex> r(logger.mutex);
			logger << lentry.severity;
			logger << lentry << std::endl;
			return logger;
		}

		Logger(const Logger &) = delete;
		Logger &operator= (const Logger &) = delete;



		

	}; // class end


}  // namespace