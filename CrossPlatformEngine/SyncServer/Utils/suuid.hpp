#pragma once
#include <string>
#include <boost\uuid\uuid.hpp>
#include <boost\lexical_cast.hpp>
#include <memory>
#include <exception>
#include <vector>

/**
*	SUUID stands for Solvex UUID
*	It is a wrapper for convenient searches and queries by Uid
*	As the Uid can be UUID string in one file, and 
*
**/

enum SUUIDType{
	SUUID_INVALID	=	0x00,
	SUUID_STR_12	=	0x01,
	SUUID_STR_16	=	0x02,
	SUUID_INT		=	0x10,
	SUUID_INT64		=	0x20,
};

class SUUID
{
protected:
	SUUIDType _type;
	union {
		char* bytes;
		int integer;
		int64_t suppaint;
	} value;
	bool	_binary;

	void copy(const SUUID& value);

	void copy_str(const char* ptr, int size = -1);

	SUUID();

	static SUUIDType validate(const std::vector<std::string>::const_iterator&);
public:
	static SUUID Generate(SUUIDType type = SUUIDType::SUUID_STR_16);

	SUUID& operator = (const SUUID& value);

	operator int() const;
	operator std::string() const;

	SUUID(const SUUID& value);
	SUUID(int value);
	SUUID(int64_t value);
	~SUUID();
	SUUID(const std::string &value, bool is_binary = false);

	
	/*
	*	Returns false, if buffsize smaller than	size of binary UUID (12 or 16)
	*	If append_nullbyte buffsize is needed to be larger by one byte.
	*/
	bool Binary(char* ptr_to_write,const size_t buffisze, bool append_nullbyte = false);
	SUUIDType Type() const;
	bool IsInt(bool include_int64 = false) const;
	int AsInt() const;
	int64_t AsInt64() const;
	std::string AsStr() const;
	bool SUUID::Valid() const;

};