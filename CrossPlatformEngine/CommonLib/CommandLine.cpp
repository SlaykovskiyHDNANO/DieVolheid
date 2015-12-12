<!DOCTYPE HTML>
<!DOCTYPE html PUBLIC "" ""><HTML><HEAD>
<META http-equiv="Content-Type" content="text/html; charset=utf-8"></HEAD>
<BODY>
<PRE>#define _CRT_SECURE_NO_WARNINGS
#include "CommandLine.h"
#include &lt;cstring&gt;
#include &lt;stdexcept&gt;
#include &lt;sstream&gt;
#include &lt;memory&gt;
#include &lt;exception&gt;
#include "utils.h"
#include &lt;algorithm&gt;
#pragma warning(disable:4996)


#pragma region Utils

using namespace utils;

bool NameCheck(const char* str) {
	bool only_spec_symbols = true;
	std::array&lt;char, 2&gt; allowed_spec_symbols = { '_', '.' };
	if (!str)
		return false;
    auto find_in_array_lambda = [&amp;allowed_spec_symbols](char ch)-&gt;bool {
        for (size_t i = 0, s = allowed_spec_symbols.size(); i &lt; s; ++i)
            if (tolower(allowed_spec_symbols[i]) == ch)
				return true;
		return false;
	};

	while (*str)
	{
		if (isalnum(*str))
			only_spec_symbols = false;

		if ( !isalnum(*str) &amp;&amp; !find_in_array_lambda(*str))
			return false;
		++str;
	}
	return !only_spec_symbols;
}

template &lt;typename T&gt;
typename std::vector&lt;T&gt;::iterator  FindFirstDuplicate(
    const typename std::vector&lt;T&gt;::iterator &amp;begin,
    const typename std::vector&lt;T&gt;::iterator &amp;end,
	bool(*less)(const T&amp; a, const T&amp; b),
	bool(*equals)(const T&amp; a, const T&amp;b)
	) {
	//First - sort it
	std::sort(begin, end, less);
    typename std::vector&lt;T&gt;::iterator iter;
	for (iter = begin; iter != end; ++iter)
		if (iter + 1 != end)
			if (equals(*iter, *(iter+1)))
				return iter;
	return iter;
}



void CopyAllocate(char** *pointers_to_allocate, const char* *source, size_t nump ) {
	for (size_t i = 0; i &lt; nump; ++i)
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



size_t FindNumObligatory(const std::vector&lt;Option&gt;::iterator &amp;begin,const std::vector&lt;Option&gt;::iterator &amp;end) {
	//Let's sort options!
	std::sort(begin, end, [](const Option&amp; a, const Option&amp; b) {
		return a.IsObligatory() &lt; b.IsObligatory();
	});
	bool less = false;
	size_t i = 0, s = end - begin;
	size_t del = s / 2;
    if(s == 1)
        less = !begin-&gt;IsObligatory();
    else if (begin != end)
	//binary search of the last non-obligatory / first obligatory option
	//less is false if found first obligatory
		for (i = del; del || !less; del &gt;&gt;= 1)
		{
			if ( (begin+i)-&gt;IsObligatory()) {
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
	delete stored_value_;
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

Option &amp; Option::operator=(const Option &amp; option)
{
	this-&gt;v = option.v;
	return *this;
}

bool CommandIsEqual(const Command&amp; a, const Command&amp; b) {
    return !my_strcmpi(a.Name(), b.Name());
}

bool CommandIsLess(const Command&amp; a, const Command&amp; b){
    return my_strcmpi(a.Name(), b.Name()) &lt; 0;
}

bool OptionIsLess(const Option&amp; a, const Option&amp; b) {
    int lng = my_strcmpi(a.LongName(), b.LongName()) &lt; 0;
    return lng == 0 ? my_strcmpi(a.ShortName(), b.ShortName()) &lt; 0: lng;
}

bool OptionIsEqual(const Option&amp; a, const Option&amp; b) {
    return !my_strcmpi(a.ShortName(), b.ShortName()) ? true : !my_strcmpi(a.LongName(), b.LongName());
}

#pragma endregion

#pragma region Check and Parse Functions

SelfTestResult Option::SelfTest() const
{
	//Self test includes:
	//Name Check: Name should only consists of A-Z and 0-9 characters. Symbols "_", "." also permitted.
	//Flag &amp; Obligatory test: 
	//	-	self test will return warning, if &lt;is_flag&gt; both &lt;obligatory&gt; is True. 
	//	-	self test will return warning, if &lt;is_flag&gt; True and default_value is not [nullptr]
	SelfTestResult warnings = SelfTestResult::STR_OK;
	if (	(	v-&gt;short_name_ == v-&gt;long_name_	&amp;&amp; 
				v-&gt;short_name_ == nullptr 			)||
			!NameCheck(v-&gt;short_name_)				|| 
			!NameCheck(v-&gt;long_name_)				) 
	{
		std::stringstream ss;
		ss	&lt;&lt; "At least one name of option \'" 
			&lt;&lt; (v-&gt;short_name_ ? v-&gt;short_name_ : v-&gt;long_name_)
			&lt;&lt; "\' is bad. Consider changing it.";
		const std::string &amp;str = ss.str(); //referencing instead of copying
        throw std::runtime_error(str.c_str());
	}

	if (v-&gt;is_flag_ &amp;&amp; v-&gt;is_obligatory_)
		warnings |= SelfTestResult::STR_WARNING_OPTION_IS_OBLIGATORY_FLAG;
	if (v-&gt;default_value_ &amp;&amp; v-&gt;is_flag_)
		warnings |= SelfTestResult::STR_WARNING_OPTION_IS_FLAG_WITH_DEFAULT_VALUE;

	return warnings;
}

bool Option::Check(const void * optional_parameter) const
{
	return v-&gt;check_function_ ? v-&gt;check_function_(*this, optional_parameter) : true;
}



Option::CheckFunction Option::CheckFunc() const
{
	return v-&gt;check_function_;
}


//offset will indicate the end of parsing sequence.
//Note: offset will reset to previous value, if parsing fails
//@example: --name=value	won't fail on name check
//			-- name=value	will fail
//			--name =value	will fail too
//			--name=value	will fail if &lt;name&gt; is flag type
//			--name= value	won't fail on name check
//			- n value		will fail
//			-n  value		won't fail on name check
//			-n=value		will fail


Option::ParseOptionResult Option::TryParse(const char * s, size_t &amp; offset, const void* opt_param) const
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
	iter = _short ? v-&gt;short_name_ : v-&gt;long_name_;
	//Check Name
	s += 1 + !_short;
	char ch = strlen(s) &lt; strlen(iter) ? 0 : *(s + strlen(iter));
	correct_name = stristr(s, iter) == s &amp;&amp; (!ch || isspace(ch) || ch=='=');
	if (!correct_name)
		return Option::ParseOptionResult::PR_NAME_NOT_RECOGNIZED;
	s += strlen(iter);
	//Check if parsing line have value when option is not a flag, or reverse	
	bool equals_sign = ch == '=';
	if (equals_sign &amp;&amp; v-&gt;is_flag_)
		return Option::ParseOptionResult::PR_VALUE_WHEN_FLAG;
	else if (!equals_sign &amp;&amp; !v-&gt;is_flag_)
		return Option::ParseOptionResult::PR_VALUE_NOT_PRESENT_OR_BAD_SYNTAX;
	//Get value and check it if Option is not a flag
	if (!v-&gt;is_flag_) {
		bool find_quotes;
		const char* end = nullptr;
		s = SkipSpace(s);
		find_quotes = *s == '\"';
		end = find_quotes ? strchr(s, '\"') : ([](const char* s)-&gt;const char* { 
			while (*s&amp;&amp;!isspace(*s++));
			return !*s  || isspace(*--s) ? s : nullptr;
		})(s);
		if (end == nullptr || s &gt;= end - 1)
			return Option::ParseOptionResult::PR_VALUE_NOT_PRESENT_OR_BAD_SYNTAX;
		//Copy value
		v-&gt;stored_value_ = new char[end-s];
		strncpy(v-&gt;stored_value_, s + 1, end - s - 1);
		v-&gt;stored_value_[end - s - 1] = 0;
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
	ss &lt;&lt; "--" &lt;&lt; v-&gt;long_name_ ;

	if (v-&gt;short_name_)
		ss &lt;&lt; " | -" &lt;&lt; v-&gt;short_name_;
	if (v-&gt;default_value_)
		ss &lt;&lt; " [DEFAULT: \'" &lt;&lt; v-&gt;default_value_ &lt;&lt; "\']";
	if (v-&gt;is_flag_ || v-&gt;is_obligatory_) {
		ss &lt;&lt; " { ";
		ss
			&lt;&lt; (v-&gt;is_flag_ ? "FLAG" : "")
			&lt;&lt; (v-&gt;is_obligatory_ &amp;&amp; v-&gt;is_flag_ ? " ; " : "")
			&lt;&lt; (v-&gt;is_obligatory_ ? "OBLIG" : "");
		ss &lt;&lt; " } ";
	}
	ss &lt;&lt; "\t - \t" &lt;&lt; v-&gt;help_text_;
	return ss.str();
}

#pragma endregion 

#pragma region Accessors

bool Option::IsFlag() const
{
	return v-&gt;is_flag_;
}

bool Option::IsObligatory() const
{
	return v-&gt;is_obligatory_;
}

const char * Option::DefaultValue() const
{
	return v-&gt;default_value_;
}

const char * Option::HelpText() const
{
	return v-&gt;help_text_;
}


const char * Option::LongName() const
{
	return v-&gt;long_name_;
}

const char * Option::ShortName() const
{
	return v-&gt;short_name_;
}

char * Option::StoredValue() const
{
	return v-&gt;stored_value_;
}

#pragma endregion

#pragma endregion


Command::CommandVals::CommandVals(const char * name, const long long int id, const std::vector&lt;Option&gt;&amp; options, const char * help_text) : 
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
		const std::vector&lt;Option&gt;&amp; options,	const char *		help_text
	):v(new CommandVals(name, id, options, help_text))
		
{
}

Command::~Command()
{
	
	
}

Command &amp; Command::operator=(const Command &amp; command)
{
	this-&gt;v = command.v;
	return *this;
}

//Self test includes: Name Check, Alias Check
//Command::SelfTest() performs SelfTest on all Options
SelfTestResult Command::SelfTest() const
{
	std::stringstream ss;
	if (	v-&gt;name_ == nullptr	|| 	!NameCheck(v-&gt;name_)		){
		
		ss &lt;&lt; "Command [id=" &lt;&lt; v-&gt;id_ &lt;&lt; "] has bad name: \'"&lt;&lt; v-&gt;name_ &lt;&lt;"\'";
		const std::string &amp;str = ss.str();
        throw std::runtime_error(str.c_str());
	}
	SelfTestResult warns = SelfTestResult::STR_OK;
	//find duplicates
    std::vector&lt;Option&gt;::const_iterator i = FindFirstDuplicate&lt;Option&gt;(
		v-&gt;options_.begin(),
		v-&gt;options_.end(), OptionIsLess, OptionIsEqual);
		
	//found duplicate
	if (i != v-&gt;options_.end())
	{
		ss &lt;&lt; "Command [id=" &lt;&lt; v-&gt;id_ &lt;&lt; "] has duplicate option: \'" &lt;&lt; i-&gt;LongName() &lt;&lt; "\'" &lt;&lt; "|\'" &lt;&lt; i-&gt;ShortName() &lt;&lt; "\'";
		const std::string &amp;str = ss.str();
        throw std::runtime_error(str.c_str());
	}

	for (auto iter = v-&gt;options_.begin(), end = v-&gt;options_.end(); iter != end; ++iter) {
		try {
			warns |= iter-&gt;SelfTest();
		}
		catch(std::exception&amp; e){
			ss &lt;&lt; "In command [id=" &lt;&lt; v-&gt;id_ &lt;&lt; "]" &lt;&lt; v-&gt;name_ &lt;&lt; "exception occured: " &lt;&lt; std::endl &lt;&lt; "\t" &lt;&lt; e.what();
			const std::string &amp;str = ss.str();
            throw std::runtime_error(str.c_str());
		}
	}
	
	return warns;
}

Option &amp; Command::operator[](size_t index)
{
	return v-&gt;options_[index];
}
const Option &amp; Command::operator[](size_t index) const
{
	return v-&gt;options_[index];
}
Option &amp; Command::operator[](const char * name)
{
	auto iter = std::find_if(v-&gt;options_.begin(), v-&gt;options_.end(), [&amp;name](const Option&amp; a) {
        return !my_strcmpi(a.ShortName(), name) || !my_strcmpi(a.LongName(), name);
	});
	return *iter;
}
const Option &amp; Command::operator[](const char * name) const
{
	auto iter = std::find_if(v-&gt;options_.begin(), v-&gt;options_.end(), [&amp;name](const Option&amp; a) {
        return !my_strcmpi(a.ShortName(), name) || !my_strcmpi(a.LongName(), name);
	});
	return const_cast&lt;const Option&amp;&gt;(*iter);
}

const std::vector&lt;Option&gt;&amp; Command::Options() const
{
	return v-&gt;options_;
}
const char * Command::Name() const
{
	return v-&gt;name_;
}
const long long int Command::Id() const
{
	return v-&gt;id_;
}
const char * Command::HelpText() const
{
	return v-&gt;help_text_;
}
size_t Command::NumObligatoryOptions() const
{
	return v-&gt;num_obligatiory_;
}

std::string Command::Help() const
{
	std::stringstream ss;
	ss &lt;&lt; v-&gt;name_;
	if (v-&gt;options_.size()) {
		ss &lt;&lt; std::endl &lt;&lt; "Command have "&lt;&lt; v-&gt;options_.size() &lt;&lt; " options: " &lt;&lt; std::endl;
		for (auto it = v-&gt;options_.begin(), s = v-&gt;options_.end(); it != s; ++it)
			ss &lt;&lt; "\t" &lt;&lt; it-&gt;Help() &lt;&lt; std::endl;
	}
	return ss.str();
}

#pragma region CommandLine Class

#pragma region Constructors and Destructors



CommandLine::CommandLine(	const std::vector&lt;Command&gt;&amp; commands,	/*const std::vector&lt;Option&gt;&amp; global_options,*/
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
	if (default_command_index_ == -1 || default_command_index_ &gt;= this-&gt;commands_.size())
		warns |= SelfTestResult::STR_WARNING_CMDLINE_INVALID_SELECTED_COMMAND_INDEX;
	//Check duplicates in global namespace

	/* 
	//TODO: MAKE IT LAMBDA OR FUNCTION
	std::vector&lt;Option&gt;::iterator it = FindFirstDuplicate&lt;Option&gt;(
		this-&gt;global_options_.begin(), this-&gt;global_options_.end(), OptionIsLess, OptionIsEqual);
	if (it != global_options_.end()) {
		ss &lt;&lt; "CommandLine has duplicate option: \'" &lt;&lt; it-&gt;LongName() &lt;&lt; "\'" &lt;&lt; "|\'" &lt;&lt; it-&gt;ShortName() &lt;&lt; "\'";
		const std::string &amp;str = ss.str();
		throw std::exception(str.c_str(), SelfTestResult::STR_ERROR_CMDLINE_DUPLICATE_OPTIONS);
	} 
	*/

	if (this-&gt;commands_.size() == 0)
		warns |= SelfTestResult::STR_WARNING_CMDLINE_NO_COMMANDS;
	else {
		//Find Command Duplicates
		//Check duplicates in global namespace
        std::vector&lt;Command&gt;::const_iterator it = FindFirstDuplicate&lt;Command&gt;(
            this-&gt;commands_.begin(),
            this-&gt;commands_.end(), CommandIsLess, CommandIsEqual);
		if (it != commands_.end()) {
			ss &lt;&lt; "CommandLine has duplicate command ["&lt;&lt;it-&gt;Id()&lt;&lt;"]: \'" &lt;&lt; it-&gt;Name() &lt;&lt; "\'";
			const std::string &amp;str = ss.str();
            throw std::runtime_error(str.c_str());
		}

		//Perform SelfTest on commands
		for (auto iter = this-&gt;commands_.begin(), end = this-&gt;commands_.end(); iter != end; ++iter)
		try {
			iter-&gt;SelfTest();
		}
		catch (std::exception&amp; e) {
			ss	&lt;&lt; "When performing CommandLine::SelfTest() exception occured: " &lt;&lt; std::endl &lt;&lt; "\t" &lt;&lt; e.what();
			const std::string &amp;str = ss.str();
            throw std::runtime_error(str.c_str());
		}
	}
	
		

	return warns;
}

Command &amp; CommandLine::operator[](int index)
{
	return this-&gt;commands_[index];

}

const Command &amp; CommandLine::operator[](int index) const
{
	return this-&gt;commands_[index];
}

Command &amp; CommandLine::operator[](const char * name)
{
	auto iter = std::find_if(this-&gt;commands_.begin(), this-&gt;commands_.end(), [&amp;name](const Command&amp; a) {
        return !my_strcmpi(a.Name(), name);
	});
	return *iter;
}

const Command &amp; CommandLine::operator[](const char * name) const
{
	auto iter = std::find_if(this-&gt;commands_.begin(), this-&gt;commands_.end(), [&amp;name](const Command&amp; a) {
        return !my_strcmpi(a.Name(), name);
	});
	return const_cast&lt;const Command&amp;&gt;(*iter);
}

ParsingResult CommandLine::ParseLine(const char * line) const
{
    throw std::runtime_error("ERROR:NOT IMPLEMENTED");

}

ParsingResult CommandLine::Parse(size_t argc, char ** argv) const
{
	//TODO: Make more generalized function for line parsing
	if (argc &lt; 2) {
		//NO line - default command
		if (this-&gt;default_command_index_ == -1 || this-&gt;default_command_index_ &gt;= this-&gt;commands_.size())
			return ParsingResult::CLPR_GLOB_EMPTY_LINE_AND_DEFAULT_COMMAND_INVALID_OR_NOT_SET;
		else
		{
			selected_command_ = this-&gt;default_command_index_;
			this-&gt;parsed_command_ = this-&gt;commands_[selected_command_];//copy command
			if (this-&gt;commands_[selected_command_].NumObligatoryOptions())
				return this-&gt;parse_result_ = ParsingResult::CLPR_COMMAND_OBLIGATORY_OPTION_NOT_SET;
		}
	}
	else {
		//if we have argv array
		char* name = argv[1];
		//find this name
		auto cmdit = std::find_if(
			commands_.begin(), commands_.end(),
            [&amp;name](const Command &amp;a) { return !my_strcmpi(a.Name(), name); }
		);
		if (cmdit == commands_.end())
			return ParsingResult::CLPR_COMMAND_UNKNOWN_COMMAND;
		else
			this-&gt;parsed_command_ = *cmdit;
		//otherwise - we have found our command.
		//now it is time to parse our options
		//TODO: Make another approach - search for obligatory options first
		std::vector&lt;const Option*&gt;	not_found_options;
		size_t										num_obligatory_left =	this-&gt;parsed_command_.NumObligatoryOptions();
		Option::ParseOptionResult					pres;
		size_t										offset				=	0;
		//populate vector
		not_found_options.reserve(parsed_command_.Options().size());
		for (auto	i = parsed_command_.Options().begin(),
			s = parsed_command_.Options().end(); i != s; ++i)
			not_found_options.push_back(&amp;(*i));
		for (size_t i = 2; i &lt; argc; ++i) {
			auto uit = not_found_options.begin(), uend = not_found_options.end();
			for (; uit != uend; ++uit)
			{
				pres = (*uit)-&gt;TryParse(argv[i], offset, nullptr);
				if (pres != Option::ParseOptionResult::PR_NAME_NOT_RECOGNIZED)
					break;
			}
			//check if we haven't found anything
			if (uit == uend)
				return this-&gt;parse_result_ = ParsingResult::CLPR_COMMAND_UNKNOWN_OPTION;
			//or if we found option (or it is not an option)
			switch (pres) {
			case Option::ParseOptionResult::PR_NAME_NOT_RECOGNIZED:
				return this-&gt;parse_result_ = ParsingResult::CLPR_COMMAND_UNKNOWN_OPTION;
			case Option::ParseOptionResult::PR_OK:
				//great! now we delete it from vector
				if ((*uit)-&gt;IsObligatory())
					--num_obligatory_left;
				not_found_options.erase(uit);
				break;
			default:
				//TODO: Remember error type and print error accordingly
				return this-&gt;parse_result_ = ParsingResult::CLPR_OPTION_INVALID_VALUE;
			}

		}
		//check obligatory option
		if (num_obligatory_left)
			return this-&gt;parse_result_ = ParsingResult::CLPR_COMMAND_OBLIGATORY_OPTION_NOT_SET;
	}
	return ParsingResult::CLPR_OK;
}

ParsingResult CommandLine::ParseResult() const
{
	return this-&gt;parse_result_;
}

const Command &amp; CommandLine::ParsedCommand() const
{
	return this-&gt;parsed_command_;

}

std::string CommandLine::Help() const
{
	std::stringstream ss;
	ss &lt;&lt; (help_text_?help_text_:"This program does not have any help info.") &lt;&lt; std::endl &lt;&lt; std::endl;
	if (commands_.size()) {
		ss &lt;&lt; "This program has " &lt;&lt; commands_.size() &lt;&lt; " commands:" &lt;&lt;std::endl;
		for (auto it = commands_.begin(), s = commands_.end(); it != s; ++it)
			ss &lt;&lt; it-&gt;Help() &lt;&lt; std::endl;
	}
	return ss.str();
}

#pragma endregion

</PRE></BODY></HTML>
