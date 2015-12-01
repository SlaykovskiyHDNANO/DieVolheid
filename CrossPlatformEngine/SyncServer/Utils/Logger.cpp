#include "Logger.h"

namespace utils {

	LogEntry::LogEntry(const std::string& message, const std::string &origin, const LogType &severity) :
		severity(severity), origin(origin)
	{
		//TODO: Refactor
		*this << L"[origin: " << std::wstring(origin.begin(), origin.end()) << L"]" << std::endl;
		*this << std::wstring(message.begin(), message.end());
	}

	std::string LogEntry::GetTime(const char* format){
		std::time_t clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		char buf[256];
		std::strftime(buf, 256, format, std::localtime(&clock));
		return buf;
	}



	Logger* Logger::Log = nullptr;

	// ctor (remove parameters if you don´t need them)
	Logger::Logger(const LogOptions& lopts, bool write_in_std_too, bool replace_static_log_ptr)
		: numWarnings(0U),
		numErrors(0U), opts(lopts), omit(false), write_in_std(write_in_std_too)
	{
		std::stringstream ss;

		ss << lopts.filename << (lopts.append_date_to_filename ? LogEntry::GetTime("%Y-%m-%d_%H-%M-%S") : "") \
			<< (lopts.append_version_to_filename ? "_" : "") << (lopts.append_version_to_filename ? lopts.version : "") << "." << lopts.extension;
		this->fname = ss.str();
		this->open(this->fname);

		// Write the first lines
		if (this->is_open()) {
			*this << "Log started. Version: " << lopts.version << std::endl;
			*this << "Log file created: " << LogEntry::GetTime("%Y-%m-%d %H:%M:%S") << std::endl << std::endl;
		} // if
		if (replace_static_log_ptr)
			Logger::Log = this;

	}

	void Logger::start(){
		this->mutex.lock();
	}

	void Logger::end(){
		this->mutex.unlock();
	}

	// dtor
	Logger::~Logger() {

		if (this->is_open()) {
			*this << std::endl << std::endl;
			*this << "LOGGER ENDED ITS WORK." << std::endl;
			// Report number of errors and warnings
			*this << numWarnings << " warnings" << std::endl;
			*this << numErrors << " errors" << std::endl;

			this->close();
		}

	}
	// define an operator<< to take in std::endl
	Logger& Logger::operator<<(StandardEndLine manip)
	{
		std::lock_guard<std::recursive_mutex>lg(this->mutex);
		// call the function, but we cannot return it's value
		if (this->write_in_std)
			manip(std::wcout);
		manip(*this);
		return *this;
	}
}
