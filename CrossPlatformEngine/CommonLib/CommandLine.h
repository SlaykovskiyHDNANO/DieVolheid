#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <array>
#include <vector>
#include "CommandLineParseEnum.h"
#include <exception>

enum SelfTestResult {
	STR_OK												=	0x00,

	STR_ERROR_CMDLINE_DUPLICATE_COMMANDS				=	0x01,
	STR_ERROR_CMDLINE_DUPLICATE_OPTIONS					=	0x02,

	STR_ERROR_COMMAND_BAD_NAME							=	0x000100,
	STR_ERROR_COMMAND_DUPLICATE_OPTIONS					=	0x000200,

	STR_ERROR_OPTION_BAD_NAME							=	0x010000,

	STR_WARNING_CMDLINE_INVALID_SELECTED_COMMAND_INDEX	=	0x10,
	STR_WARNING_CMDLINE_NO_COMMANDS						=	0x20,
	
	STR_WARNING_OPTION_IS_OBLIGATORY_FLAG				=	0x100000,
	STR_WARNING_OPTION_IS_FLAG_WITH_DEFAULT_VALUE		=	0x200000,


	
};
inline SelfTestResult operator|(SelfTestResult a, SelfTestResult b)
{
	return static_cast<SelfTestResult>(static_cast<int>(a) | static_cast<int>(b));
}
inline SelfTestResult operator|=(SelfTestResult a, SelfTestResult b)
{
	a = a | b;
	return a;
}


class Option;
class Command;
class CommandLine;


// Represents one flag or option
// Options can be attached to a command, or they can exist in the global scope
class Option {
public:
	typedef bool(*CheckFunction)(const Option&, const void*);
	struct OptionVals {
		CheckFunction	check_function_ = { nullptr };
		char			*default_value_ = { nullptr };
		char			*short_name_ = { nullptr },
						*long_name_ = { nullptr },
						*help_text_ = { nullptr };

		mutable char	*stored_value_ = { nullptr };

		bool			is_obligatory_ = { false },
						is_flag_ = { false };
		~OptionVals();

		OptionVals(const char*	short_name, const char*	long_name = nullptr,
					const char*	default_value = nullptr, const char*	help_text = nullptr,
			bool		obligatory = false, bool		is_flag = false);
	};


	enum ParseOptionResult {
		PR_OK					,
		
		PR_SYNTAX_NOT_AN_OPTION	,

		PR_NAME_NOT_RECOGNIZED	,
		PR_NAME_BAD_SYNTAX		,
		
		PR_VALUE_NOT_PRESENT_OR_BAD_SYNTAX	,
		PR_VALUE_WHEN_FLAG		,
		PR_VALUE_CHECK_FAIL		,

		PR_NRESULT_TOTAL_ERRORS	,
	};

	static const std::array<char*, (size_t)(ParseOptionResult::PR_NRESULT_TOTAL_ERRORS-1)> ErrorDescriptors;

private:
	std::shared_ptr<OptionVals>	v;

protected:


public:

	



	Option();
	Option(	const char*	short_name,						const char*	long_name	=	nullptr,
			const char*	default_value	=	nullptr,	const char*	help_text	=	nullptr,
			bool		obligatory		=	false,		bool		is_flag		=	false);

	~Option();

	Option&				operator=(const Option& option);

	SelfTestResult		SelfTest()										const;
	
	bool				Check(const void* optional_parameter = nullptr)	const;

	bool				IsFlag()										const;
	bool				IsObligatory()									const;
	const char*			DefaultValue()									const;
	const char*			HelpText()										const;
	std::string			Help()											const;
	const char*			LongName()										const;
	const char*			ShortName()										const;
	char*				StoredValue()									const;
	CheckFunction		CheckFunc()										const;

	ParseOptionResult	TryParse(	const char* string		,
									size_t &offset			,
									const void* check_param	)			const;


#if defined(_IOSTREAM_)
	//Returns information about given option
	friend std::ostream& operator << (std::ostream& os, const Option& op);
#endif
};

// Represents a command - an API call with trailing API call options.
class Command {
public:
	struct CommandVals {
		//TODO: make normal algorithm or another assumption in it
		//crotch for FindFirstDuplicate
		//Can't do sort without it
		mutable std::vector<Option>	options_;

		char				*name_ = { nullptr };

		long long int		id_ = { 0 };

		char				*help_text_ = { nullptr };

		size_t				num_obligatiory_ = { 0 };
		
		CommandVals(const char* name, const long long int id = 0, const std::vector<Option> &options = {}, const char* help_text = nullptr);

		~CommandVals();

	};
private:

	std::shared_ptr<CommandVals> v;

public:
	Command();
	Command(const char* name, const long long int id = 0, const std::vector<Option> &options = {}, const char* help_text = nullptr);
	
	~Command();

	Command&		operator =	(const Command& command);

	SelfTestResult			SelfTest()							const;

	inline Option&			operator[]	(size_t	index);
	inline const Option&	operator[]	(size_t	index)			const;
	Option&			operator[]	(const char*	name);
	const Option&	operator[]	(const char*	name)	const;

	const std::vector<Option>&	Options()				const;
	const char*					Name()					const;
	const long long int			Id()					const;
	const char*					HelpText()				const;

	size_t						NumObligatoryOptions()	const;

	std::string					Help()					const;
#if defined(_IOSTREAM_)
	//Returns information about given command
	friend std::ostream& operator << (std::ostream& os, const Command& cmd);
#endif

};

class CommandLine {
public:
	


private:
	//DISABLED
	//mutable std::vector<Option>			global_options_;
	mutable std::vector<Command>		commands_;

	char*								help_text_;

	//size_t							obligatory_global_		=	{ 0 };
	size_t								default_command_index_	=	{ (size_t)-1 };
	mutable size_t						selected_command_		=	{ (size_t)-1 };

	mutable Command						parsed_command_;
	mutable ParsingResult	parse_result_;

public:
	CommandLine(const std::vector<Command> &commands, /*const std::vector<Option> &global_options,*/
				size_t default_command_index = -1, const char* help_text = nullptr);

	



	/*
	*	Performs a self test on all commands and parameters.
	*	Throws std::exception with a message and a code, describing self test error
	*/
	SelfTestResult	SelfTest()					const;

	Command&		operator[]	(int	index);
	const Command&	operator[]	(int	index)			const;
	Command&		operator[]	(const char*	name);
	const Command&	operator[]	(const char*	name)	const;


	ParsingResult	ParseLine(const char* line)			const;
	ParsingResult	Parse(size_t argc, char** argv )	const;
	
	ParsingResult	ParseResult()						const;
	const Command&				ParsedCommand()			const;

	std::string Help()									const;


	
#if defined(_IOSTREAM_)
	//Returns information about given command
	friend std::ostream& operator << (std::ostream& os, const CommandLine& cmd);
#endif
};</PRE></BODY></HTML>
