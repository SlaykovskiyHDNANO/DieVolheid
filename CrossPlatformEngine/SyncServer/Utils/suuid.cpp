#define _SCL_SECURE_NO_WARNINGS
#include "suuid.hpp"
#include <random>
#include <boost\algorithm\string\split.hpp>
#include <string>
#include <boost\tokenizer.hpp>
#include <boost\algorithm\algorithm.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <iostream>

#pragma region Service Functions

SUUID::SUUID(){
	this->_type = SUUIDType::SUUID_INVALID;
}

void SUUID::copy(const SUUID& value) {
	this->_type = value._type;
	this->_binary = value._binary;
	switch (value._type){
	case SUUIDType::SUUID_INT:
		this->value.integer = value.value.integer;
		break;
	case SUUIDType::SUUID_INT64:
		this->value.suppaint = value.value.suppaint;
		break;
	case SUUIDType::SUUID_STR_12:
		this->copy_str(value.value.bytes, 13); //with nullbyte
		break;
	case SUUIDType::SUUID_STR_16:
		this->copy_str(value.value.bytes, 17);
		break;
	}

}

void SUUID::copy_str(const char* ptr, int size){
	if (size < 0)
		size = strlen(ptr) + 1;

	this->value.bytes = new char[size];
	memcpy(this->value.bytes, ptr, size);
}

#pragma endregion

#pragma region Constructors, Destructors, Assign Operators
SUUID& SUUID::operator = (const SUUID& value) {
	this->copy(value);
	return *this;
}
SUUID::SUUID(const SUUID& value) {
	this->copy(value);
}
SUUID::SUUID(int value){
	this->_type = SUUIDType::SUUID_INT;
	this->value.integer = value;
	this->_binary = true;
}
SUUID::SUUID(int64_t value){
	this->value.suppaint = value;
	this->_type = SUUIDType::SUUID_INT64;
	this->_binary = true;
}


SUUIDType SUUID::validate(const std::vector<std::string>::const_iterator& string){
	const int bytes_16[5]	=	{ 4, 2, 2, 2, 6 };
	const int bytes_12[5] = { 2, 2, 1, 1, 6 };
	SUUIDType type = SUUIDType::SUUID_INVALID;
	for (int i = 0; i < 5; ++i)
		if ((type == SUUIDType::SUUID_INVALID || type == SUUIDType::SUUID_STR_16) && 2 * bytes_16[i] == (string+i)->size())
			type = SUUIDType::SUUID_STR_16;
		else if ((type == SUUIDType::SUUID_INVALID || type == SUUIDType::SUUID_STR_12) && 2 * bytes_12[i] == (string+i)->size())
			type = SUUIDType::SUUID_STR_12;
		else 
			return SUUIDType::SUUID_INVALID;
	return type;
}



bool SUUID::Binary(char* ptr,const size_t size, bool append_nullbyte){
	int to_write = SUUIDType::SUUID_INT ? 4 : SUUIDType::SUUID_INT64 ? 8 : SUUIDType::SUUID_STR_12 ? 12 : SUUID_STR_16 ? 16 : 0;
	if (!to_write || to_write+(int)append_nullbyte > size)
		return false;
	else {
		if (this->_binary)
			memcpy(ptr, (this->_type != SUUIDType::SUUID_STR_12 && this->_type != SUUIDType::SUUID_STR_16)?&this->value:(void*)this->value.bytes, to_write);
		else {
			//prepare bytes
			std::vector<char*> vasiris;
			throw std::logic_error("NOT IMPLEMENTED");
			//boost::split(vasiris, );

		}
		if (append_nullbyte)
			ptr[size] = 0;
		return false;
	}

	return true;
}



SUUID::SUUID(const std::string& value, bool binary){
	int len = value.size();
	this->_binary = binary;
	bool failed = false;
	if (!binary){
		std::vector<std::string> strings;
		boost::char_separator<char> sep("-");
		boost::tokenizer<boost::char_separator<char>> tokens(value, sep);
		for (auto t = tokens.begin(), et = tokens.end(); t != et; ++t)
			strings.push_back(*t);
		if (strings.size() != 5)
			failed = true;
		else {
			this->_type = SUUID::validate(strings.begin());
			failed = this->_type == SUUIDType::SUUID_INVALID;
		}
	}
	else
		failed = len == 16 || len == 12;

	if (failed)
		throw new std::logic_error("Invalid parameter: string can be either 16 or 12 bytes long.");
	else
		this->copy_str(value.c_str());
}
SUUID::~SUUID(){
	if (static_cast<int>(this->_type) & 0x0F)
		delete this->value.bytes;
}
#pragma endregion

#pragma region Accessors
bool SUUID::Valid() const {
	return this->_type != SUUIDType::SUUID_INVALID;
}

SUUIDType SUUID::Type() const{
	return this->_type;
}
int SUUID::AsInt() const {
	if (this->_type != SUUIDType::SUUID_INT)
		throw new std::logic_error("SUUID type is not an int");
	return this->value.integer;
}
int64_t SUUID::AsInt64() const{
	if (this->_type != SUUIDType::SUUID_INT64)
		throw new std::logic_error("SUUID type is not an int64");
	return this->value.suppaint;
}
std::string SUUID::AsStr() const {
	if (this->_type != SUUIDType::SUUID_STR_12 && this->_type != SUUIDType::SUUID_STR_16)
		throw new std::exception("SUUID type is not a string");
	return this->value.bytes;
}
#pragma endregion

#pragma region Verifiers
bool SUUID::IsInt(bool include_int64) const{
	return this->_type == SUUIDType::SUUID_INT || (include_int64&&this->_type == SUUIDType::SUUID_INT64);
}
#pragma endregion

#pragma region Type Converters
SUUID::operator int() const {
	return this->AsInt();
}
SUUID::operator std::string() const{
	return this->AsStr();
}
#pragma endregion

#pragma region Static

char* gen_bytes(char* bytes, int num){
	for (int i = 0; i < num; ++i)
		bytes[i] = std::rand() % CHAR_BIT;
	bytes[num] = '\0';
	return bytes;
}



SUUID SUUID::Generate(SUUIDType type){
	SUUID suuid = SUUID();
	char* bytes = nullptr;
	suuid._type = type;
	std::mt19937 gr(std::random_device());
	std::uniform_int_distribution<> gen(0, CHAR_MAX);
	std::uniform_int_distribution<int> intgen = std::uniform_int_distribution<int>();
	std::uniform_int_distribution<long long int> lintgen = std::uniform_int_distribution<long long int>();
	switch (type)
	{
	case SUUID_STR_12:
		bytes = new char[13];
		suuid.copy_str(gen_bytes(bytes,12));
		break;
	case SUUID_STR_16:
		bytes = new char[17];
		suuid.copy_str(gen_bytes(bytes,16));
		break;
	case SUUID_INT:
		suuid.value.integer = std::rand();
		break;
	case SUUID_INT64:
		suuid.value.suppaint = std::rand();
		break;
	}
	if (bytes != nullptr)
		delete bytes;
	return suuid;
}

#pragma endregion