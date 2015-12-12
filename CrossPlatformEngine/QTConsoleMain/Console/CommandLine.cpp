#define _CRT_SECURE_NO_WARNINGS
#include "CommandLine.h"
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <exception>
#include <algorithm>

#pragma region Utils

int strcmpi(const char* a, const char* b) {
#ifdef __ANDROID_API__
    return strcasecmp(a, b);
#else
    return _stricmp(a,b);
#endif
}

bool NameCheck(const char* str) {
	bool only_spec_symbols = true;
	std::array<char, 2> allowed_spec_symbols = { '_', '.' };
	if (!str)
		return false;
    auto find_in_array_lambda = [&allowed_spec_symbols](char ch)->bool {
        for (size_t i = 0, s = allowed_spec_symbols.size(); i < s; ++i)
            if (tolower(allowed_spec_symbols[i]) == ch)
				return true;
		return false;
	};

	while (*str)
	{
		if (isalnum(*str))
			only_spec_symbols = false;

		if ( !isalnum(*str) && !find_in_array_lambda(*str))
			return false;
		++str;
	}
	return !only_spec_symbols;
}

template <typename T>
typename std::vector<T>::iterator  FindFirstDuplicate(
    const typename std::vector<T>::iterator &begin,
    const typename std::vector<T>::iterator &end,
	bool(*less)(const T& a, const T& b),
	bool(*equals)(const T& a, const T&b)
	) {
	//First - sort it
	std::sort(begin, end, less);
    typename std::vector<T>::iterator iter;
	for (iter = begin; iter != end; ++iter)
		if (iter + 1 != end)
			if (equals(*iter, *(iter+1)))
				return iter;
	return iter;
}

const char* stristr(const char* str1, const char* str2)
{
	const char* p1 = str1;
	const char* p2 = str2;
	const char* r = *p2 == 0 ? str1 : 0;

	while (*p1 != 0 && *p2 != 0)
	{
		if (tolower(*p1) == tolower(*p2))
		{
			if (r == 0)
				r = p1;
			p2++;
		}
		else
		{
			p2 = str2;
			if (tolower(*p1) == tolower(*p2))
			{
				r = p1;
				p2++;
			}
			else
				r = 0;
		}
		p1++;
	}

	return *p2 == 0 ? r : 0;
}

void CopyAllocate(char** *pointers_to_allocate, const char* *source, size_t nump ) {
	for (size_t i = 0; i < nump; ++i)
	{
		*(pointers_to_allocate[i]) = source[i]?new char[strlen(source[i])+1]:nullptr;
		if(source[i])
			strcpy(*(pointers_to_allocate[i]), source[i]);
	}
}

char* CopyAllocateOne(const char* src) {
	if (!src)
		return nullptr;
	else
	{
		char* buf = new char[strlen(src)+1];
		strcpy(buf, src);
		return buf;
	}
}

const char* SkipSpace(const char* str) {
	while (str&&isspace(*str))
		++str;
	return str;
}

size_t FindNumObligatory(const std::vector<Option>::iterator &begin,const std::vector<Option>::iterator &end) {
	//Let's sort options!
	std::sort(begin, end, [](const Option& a, const Option& b) {
		return a.IsObligatory() < b.IsObligatory();
	});
	bool less = false;
	size_t i = 0, s = end - begin;
	size_t del = s / 2;
    if(s == 1)
        less = !begin->IsObligatory();
    else if (begin != end)
	//binary search of the last non-obligatory / first obligatory option
	//less is false if found first obligatory
		for (i = del; del || !less; del >>= 1)
		{
			if ( (begin+i)->IsObligatory()) {
				less = false;
				i -= del;

			}
			else {
				i += del;
				less = true;
			}
		}
	return s - i - less;
}


#pragma endregion

#pragma region Option Class

#pragma region Constructor Definition

Option::OptionVals::~OptionVals() {
	delete short_name_;
	delete long_name_;
	delete help_text_;
	delete default_value_;
}

Option::OptionVals::OptionVals(	const char*	short_name, const char*	long_name,
								const char*	default_value, const char*	help_text,
								bool		obligatory, bool		is_flag):
	short_name_(CopyAllocateOne(short_name)), long_name_(CopyAllocateOne(long_name)),
	default_value_(CopyAllocateOne(default_value)), help_text_(CopyAllocateOne(help_text)),
	is_obligatory_(obligatory), is_flag_(is_flag)
{

}


Option::Option()
{
	// nothing to do here - we have created all default initializers for each member in declaration
}

Option::Option	(	const char * short_name,	const char * long_name,
					const char * default_value,	const char * help_text,
					bool obligatory,			bool is_flag
				): v(new OptionVals(short_name, long_name, default_value, help_text, obligatory, is_flag))
	
{
}



Option::~Option()
{
	
}

Option & Option::operator=(const Option & option)
{
	this->v = option.v;
	return *this;
}

bool CommandIsEqual(const Command& a, const Command& b) {
    return !strcmpi(a.Name(), b.Name());
}

bool CommandIsLess(const Command& a, const Command& b){
    return strcmpi(a.Name(), b.Name()) < 0;
}

bool OptionIsLess(const Option& a, const Option& b) {
    int lng = strcmpi(a.LongName(), b.LongName()) < 0;
    return lng == 0 ? strcmpi(a.ShortName(), b.ShortName()) < 0: lng;
}

bool OptionIsEqual(const Option& a, const Option& b) {
    return !strcmpi(a.ShortName(), b.ShortName()) ? true : !strcmpi(a.LongName(), b.LongName());
}

#pragma endregion

#pragma region Check and Parse Functions

SelfTestResult Option::SelfTest() const
{
	//Self test includes:
	//Name Check: Name should only consists of A-Z and 0-9 characters. Symbols "_", "." also permitted.
	//Flag & Obligatory test: 
	//	-	self test will return warning, if <is_flag> both <obligatory> is True. 
	//	-	self test will return warning, if <is_flag> True and default_value is not [nullptr]
	SelfTestResult warnings = SelfTestResult::STR_OK;
	if (	(	v->short_name_ == v->long_name_	&& 
				v->short_name_ == nullptr 			)||
			!NameCheck(v->short_name_)				|| 
			!NameCheck(v->long_name_)				) 
	{
		std::stringstream ss;
		ss	<< "At least one name of option \'" 
			<< (v->short_name_ ? v->short_name_ : v->long_name_)
			<< "\' is bad. Consider changing it.";
		const std::string &str = ss.str(); //referencing instead of copying
        throw std::runtime_error(str.c_str());
	}

	if (v->is_flag_ && v->is_obligatory_)
		warnings |= SelfTestResult::STR_WARNING_OPTION_IS_OBLIGATORY_FLAG;
	if (v->default_value_ && v->is_flag_)
		warnings |= SelfTestResult::STR_WARNING_OPTION_IS_FLAG_WITH_DEFAULT_VALUE;

	return warnings;
}

bool Option::Check(const void * optional_parameter) const
{
	return v->check_function_ ? v->check_function_(*this, optional_parameter) : true;
}



Option::CheckFunction Option::CheckFunc() const
{
	return v->check_function_;
}


//offset will indicate the end of parsing sequence.
//Note: offset will reset to previous value, if parsing fails
//@example: --name=value	won't fail on name check
//			-- name=value	will fail
//			--name =value	will fail too
//			--name=value	will fail if <name> is flag type
//			--name= value	won't fail on name check
//			- n value		will fail
//			-n  value		won't fail on name check
//			-n=value		will fail


Option::ParseOptionResult Option::TryParse(const char * s, size_t & offset, const void* opt_param) const
{
	Option::ParseOptionResult pres = Option::ParseOptionResult::PR_OK;
	const char *s0 = s;
	const char	*short_prefix = "-", *long_prefix = "--";
	bool _short;
	bool correct_name = true;
	char* iter;
	offset = 0;
	s = SkipSpace(s);
	//Check prefix and select short or long name to check
	if (strstr(s, long_prefix) == s)
		_short = false; //then this is the long_name
	else if (strstr(s, short_prefix) == s)
		_short = true; //then this is the short_name
	else
		return Option::ParseOptionResult::PR_SYNTAX_NOT_AN_OPTION; //it is not a name
	iter = _short ? v->short_name_ : v->long_name_;
	//Check Name
	s += 1 + !_short;
	char ch = strlen(s) < strlen(iter) ? 0 : *(s + strlen(iter));
	correct_name = stristr(s, iter) == s && (!ch || isspace(ch) || ch=='=');
	if (!correct_name)
		return Option::ParseOptionResult::PR_NAME_NOT_RECOGNIZED;
	s += strlen(iter);
	//Check if parsing line have value when option is not a flag, or reverse	
	bool equals_sign = ch == '=';
	if (equals_sign && v->is_flag_)
		return Option::ParseOptionResult::PR_VALUE_WHEN_FLAG;
	else if (!equals_sign && !v->is_flag_)
		return Option::ParseOptionResult::PR_VALUE_NOT_PRESENT_OR_BAD_SYNTAX;
	//Get value and check it if Option is not a flag
	if (!v->is_flag_) {
		bool find_quotes;
		const char* end = nullptr;
		s = SkipSpace(s);
		find_quotes = *s == '\"';
		end = find_quotes ? strchr(s, '\"') : ([](const char* s)->const char* { 
			while (*s&&!isspace(*s++));
			return isspace(*--s) ? s : nullptr;
		})(s);
		if (end == nullptr || s >= end - 1)
			return Option::ParseOptionResult::PR_VALUE_NOT_PRESENT_OR_BAD_SYNTAX;
		//Copy value
		v->stored_value_ = new char[end-s-1];
		strncpy(v->stored_value_, s + 1, end - s - 1);
		//Check value
		if(!Check(opt_param))
			return Option::ParseOptionResult::PR_VALUE_CHECK_FAIL;
		//update s
		s = end;
	}
	//Set offset and return OK
	offset = s - s0 + 1;
	return Option::ParseOptionResult::PR_OK;
}

std::string Option::Help() const
{
	std::stringstream ss;
	ss << "--" << v->long_name_ ;

	if (v->short_name_)
		ss << " | -" << v->short_name_;
	if (v->default_value_)
		ss << "[default: " << v->default_value_ << "]";
	if (v->is_flag_ || v->is_obligatory_) {
		ss << " { ";
		ss
			<< (v->is_flag_ ? "FLAG" : "")
			<< (v->is_obligatory_ && v->is_flag_ ? " ; " : "")
			<< (v->is_obligatory_ ? "OBLIG" : "");
		ss << " } ";
	}
	ss << "\t - \t" << v->help_text_;
	return ss.str();
}

#pragma endregion 

#pragma region Accessors

bool Option::IsFlag() const
{
	return v->is_flag_;
}

bool Option::IsObligatory() const
{
	return v->is_obligatory_;
}

const char * Option::DefaultValue() const
{
	return v->default_value_;
}

const char * Option::HelpText() const
{
	return v->help_text_;
}


const char * Option::LongName() const
{
	return v->long_name_;
}

const char * Option::ShortName() const
{
	return v->short_name_;
}

char * Option::StoredValue() const
{
	return v->stored_value_;
}

#pragma endregion

#pragma endregion


Command::CommandVals::CommandVals(const char * name, const long long int id, const std::vector<Option>& options, const char * help_text) : 
	name_(CopyAllocateOne(name)), id_(id),
	options_(options), help_text_(CopyAllocateOne(help_text))
{
	num_obligatiory_ = FindNumObligatory(options_.begin(), options_.end());
}

Command::CommandVals::~CommandVals()
{
	delete name_;
	delete help_text_;
}


Command::Command()
{
}

Command::Command
	(	const char * name,					const long long int id, 
		const std::vector<Option>& options,	const char *		help_text
	):v(new CommandVals(name, id, options, help_text))
		
{
}

Command::~Command()
{
	
	
}

Command & Command::operator=(const Command & command)
{
	this->v = command.v;
	return *this;
}

//Self test includes: Name Check, Alias Check
//Command::SelfTest() performs SelfTest on all Options
SelfTestResult Command::SelfTest() const
{
	std::stringstream ss;
	if (	v->name_ == nullptr	|| 	!NameCheck(v->name_)		){
		
		ss << "Command [id=" << v->id_ << "] has bad name: \'"<< v->name_ <<"\'";
		const std::string &str = ss.str();
        throw std::runtime_error(str.c_str());
	}
	SelfTestResult warns = SelfTestResult::STR_OK;
	//find duplicates
    std::vector<Option>::const_iterator i = FindFirstDuplicate<Option>(
		v->options_.begin(),
		v->options_.end(), OptionIsLess, OptionIsEqual);
		
	//found duplicate
	if (i != v->options_.end())
	{
		ss << "Command [id=" << v->id_ << "] has duplicate option: \'" << i->LongName() << "\'" << "|\'" << i->ShortName() << "\'";
		const std::string &str = ss.str();
        throw std::runtime_error(str.c_str());
	}

	for (auto iter = v->options_.begin(), end = v->options_.end(); iter != end; ++iter) {
		try {
			warns |= iter->SelfTest();
		}
		catch(std::exception& e){
			ss << "In command [id=" << v->id_ << "]" << v->name_ << "exception occured: " << std::endl << "\t" << e.what();
			const std::string &str = ss.str();
            throw std::runtime_error(str.c_str());
		}
	}
	
	return warns;
}

Option & Command::operator[](size_t index)
{
	return v->options_[index];
}
const Option & Command::operator[](size_t index) const
{
	return v->options_[index];
}
Option & Command::operator[](const char * name)
{
	auto iter = std::find_if(v->options_.begin(), v->options_.end(), [&name](const Option& a) {
        return !strcmpi(a.ShortName(), name) || !strcmpi(a.LongName(), name);
	});
	return *iter;
}
const Option & Command::operator[](const char * name) const
{
	auto iter = std::find_if(v->options_.begin(), v->options_.end(), [&name](const Option& a) {
        return !strcmpi(a.ShortName(), name) || !strcmpi(a.LongName(), name);
	});
	return const_cast<const Option&>(*iter);
}

const std::vector<Option>& Command::Options() const
{
	return v->options_;
}
const char * Command::Name() const
{
	return v->name_;
}
const long long int Command::Id() const
{
	return v->id_;
}
const char * Command::HelpText() const
{
	return v->help_text_;
}
size_t Command::NumObligatoryOptions() const
{
	return v->num_obligatiory_;
}

std::string Command::Help() const
{
	std::stringstream ss;
	ss << v->name_;
	if (v->options_.size()) {
		ss << std::endl << "Command have "<< v->options_.size() << " options: " << std::endl;
		for (auto it = v->options_.begin(), s = v->options_.end(); it != s; ++it)
			ss << "\t" << it->Help() << std::endl;
	}
	return ss.str();
}

#pragma region CommandLine Class

#pragma region Constructors and Destructors



CommandLine::CommandLine(	const std::vector<Command>& commands,	/*const std::vector<Option>& global_options,*/
							size_t default_command_index,			const char* help_text) 
	:	help_text_(CopyAllocateOne(help_text)),	commands_(commands),
		/*global_options_(global_options), */	default_command_index_(default_command_index)
		
{
	//obligatory_global_ = FindNumObligatory(global_options_.begin(), global_options_.end());
}

#pragma endregion

#pragma region Self Test and Parse Functions

//Self test checks:
//	-	default_command_index
//	-	global options
//	-	find all command names duplicates
//	-	perform SelfTest on all commands
//	-	find all option duplicates
SelfTestResult CommandLine::SelfTest() const
{
	std::stringstream ss;
	SelfTestResult warns = SelfTestResult::STR_OK;
	if (default_command_index_ == -1 || default_command_index_ >= this->commands_.size())
		warns |= SelfTestResult::STR_WARNING_CMDLINE_INVALID_SELECTED_COMMAND_INDEX;
	//Check duplicates in global namespace

	/* 
	//TODO: MAKE IT LAMBDA OR FUNCTION
	std::vector<Option>::iterator it = FindFirstDuplicate<Option>(
		this->global_options_.begin(), this->global_options_.end(), OptionIsLess, OptionIsEqual);
	if (it != global_options_.end()) {
		ss << "CommandLine has duplicate option: \'" << it->LongName() << "\'" << "|\'" << it->ShortName() << "\'";
		const std::string &str = ss.str();
		throw std::exception(str.c_str(), SelfTestResult::STR_ERROR_CMDLINE_DUPLICATE_OPTIONS);
	} 
	*/

	if (this->commands_.size() == 0)
		warns |= SelfTestResult::STR_WARNING_CMDLINE_NO_COMMANDS;
	else {
		//Find Command Duplicates
		//Check duplicates in global namespace
        std::vector<Command>::const_iterator it = FindFirstDuplicate<Command>(
            this->commands_.begin(),
            this->commands_.end(), CommandIsLess, CommandIsEqual);
		if (it != commands_.end()) {
			ss << "CommandLine has duplicate command ["<<it->Id()<<"]: \'" << it->Name() << "\'";
			const std::string &str = ss.str();
            throw std::runtime_error(str.c_str());
		}

		//Perform SelfTest on commands
		for (auto iter = this->commands_.begin(), end = this->commands_.end(); iter != end; ++iter)
		try {
			iter->SelfTest();
		}
		catch (std::exception& e) {
			ss	<< "When performing CommandLine::SelfTest() exception occured: " << std::endl << "\t" << e.what();
			const std::string &str = ss.str();
            throw std::runtime_error(str.c_str());
		}
	}
	
		

	return warns;
}

Command & CommandLine::operator[](int index)
{
	return this->commands_[index];

}

const Command & CommandLine::operator[](int index) const
{
	return this->commands_[index];
}

Command & CommandLine::operator[](char * name)
{
	auto iter = std::find_if(this->commands_.begin(), this->commands_.end(), [&name](const Command& a) {
        return !strcmpi(a.Name(), name);
	});
	return *iter;
}

const Command & CommandLine::operator[](char * name) const
{
	auto iter = std::find_if(this->commands_.begin(), this->commands_.end(), [&name](const Command& a) {
        return !strcmpi(a.Name(), name);
	});
	return const_cast<const Command&>(*iter);
}

CommandLine::ParsingResult CommandLine::ParseLine(const char * line) const
{
    throw std::runtime_error("ERROR:NOT IMPLEMENTED");

}

CommandLine::ParsingResult CommandLine::Parse(size_t argc, char ** argv) const
{
	//TODO: Make more generalized function for line parsing
	if (argc < 2) {
		//NO line - default command
		if (this->default_command_index_ == -1 || this->default_command_index_ >= this->commands_.size())
			return ParsingResult::CLPR_GLOB_EMPTY_LINE_AND_DEFAULT_COMMAND_INVALID_OR_NOT_SET;
		else
		{
			selected_command_ = this->default_command_index_;
			this->parsed_command_ = this->commands_[selected_command_];//copy command
			if (this->commands_[selected_command_].NumObligatoryOptions())
				return this->parse_result_ = ParsingResult::CLPR_COMMAND_OBLIGATORY_OPTION_NOT_SET;
		}
	}
	else {
		//if we have argv array
		char* name = argv[1];
		//find this name
		auto cmdit = std::find_if(
			commands_.begin(), commands_.end(),
            [&name](const Command &a) { return !strcmpi(a.Name(), name); }
		);
		if (cmdit == commands_.end())
			return ParsingResult::CLPR_COMMAND_UNKNOWN_COMMAND;
		//otherwise - we have found our command.
		//now it is time to parse our options
		//TODO: Make another approach - search for obligatory options first
		std::vector<std::shared_ptr<const Option>>	not_found_options;
		size_t										num_obligatory_left =	this->parsed_command_.NumObligatoryOptions();
		Option::ParseOptionResult					pres;
		size_t										offset				=	0;
		//populate vector
		not_found_options.reserve(parsed_command_.Options().size());
		for (auto	i = parsed_command_.Options().begin(),
			s = parsed_command_.Options().end(); i != s; ++i)
			not_found_options.push_back(std::shared_ptr<const Option>(&(*i)));
		for (size_t i = 2; i < argc; ++i) {
			auto uit = not_found_options.begin(), uend = not_found_options.end();
			for (; uit != uend; ++uit)
			{
				pres = (*uit)->TryParse(argv[i], offset, nullptr);
				if (pres != Option::ParseOptionResult::PR_NAME_NOT_RECOGNIZED)
					break;
			}
			//check if we haven't found anything
			if (uit != uend)
				return this->parse_result_ = ParsingResult::CLPR_COMMAND_UNKNOWN_OPTION;
			//or if we found option (or it is not an option)
			switch (pres) {
			case Option::ParseOptionResult::PR_NAME_NOT_RECOGNIZED:
				return this->parse_result_ = ParsingResult::CLPR_COMMAND_UNKNOWN_OPTION;
			case Option::ParseOptionResult::PR_OK:
				//great! now we delete it from vector
				if ((*uit)->IsObligatory())
					--num_obligatory_left;
				not_found_options.erase(uit);
				break;
			default:
				//TODO: Remember error type and print error accordingly
				return this->parse_result_ = ParsingResult::CLPR_OPTION_INVALID_VALUE;
			}

		}
		//check obligatory option
		if (num_obligatory_left)
			return this->parse_result_ = ParsingResult::CLPR_COMMAND_OBLIGATORY_OPTION_NOT_SET;
	}
	return ParsingResult::CLPR_OK;
}

CommandLine::ParsingResult CommandLine::ParseResult() const
{
	return this->parse_result_;
}

const Command & CommandLine::ParsedCommand() const
{
	return this->parsed_command_;

}

std::string CommandLine::Help() const
{
	std::stringstream ss;
	ss << (help_text_?help_text_:"This program does not have any help info.") << std::endl << std::endl;
	if (commands_.size()) {
		ss << "This program has " << commands_.size() << " commands:" <<std::endl;
		for (auto it = commands_.begin(), s = commands_.end(); it != s; ++it)
			ss << it->Help() << std::endl;
	}
	return ss.str();
}

#pragma endregion

