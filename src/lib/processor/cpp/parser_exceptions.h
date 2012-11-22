/*
 * parser_actions.h
 *
 *  Created on: Nov 1, 2012
 *      Author: cnelson
 */

#ifndef __LATTICE_PROCESSOR_PARSER_EXCEPTIONS_H__
#define __LATTICE_PROCESSOR_PARSER_EXCEPTIONS_H__

#include <exception>
#include <string>

namespace lattice {
namespace processor {

enum class syntax_error_code
{
	unknown_column,
	unknown_table,
	missing_from
};

class syntax_error: public std::exception
{
	std::string msg;
public:
	syntax_error(const std::string& _msg) :
			msg(_msg)
	{
	}
	virtual const char* what() const noexcept
	{
		return msg.c_str();
	}
	virtual syntax_error_code code() const noexcept=0;
};

class unknown_column_error: public syntax_error
{
public:
	unknown_column_error(const std::string& table_name,
			const std::string& column_name) :
			syntax_error(
					"The specified column named '" + column_name
							+ "' does not exist in the "
									"specification for table '" + table_name + "'.")
	{
	}

	virtual syntax_error_code code() const noexcept
	{
		return syntax_error_code::unknown_column;
	}
};

class unknown_table_error: public syntax_error
{
public:
	unknown_table_error(const std::string& table_name) :
			syntax_error(
					"The specified table named '" + table_name
							+ "' does not exist in the "
									"current database.")
	{
	}

	virtual syntax_error_code code() const noexcept
	{
		return syntax_error_code::unknown_table;
	}
};

class missing_from : public syntax_error
{
public:
	missing_from() :
			syntax_error("One or more column names were specified, but no "
					       "'from <table_expression>' clause was found.")
	{
	}

	virtual syntax_error_code code() const noexcept
	{
		return syntax_error_code::missing_from;
	}
};


} // namespace processor
} // namespace lattice

#endif // __LATTICE_PROCESSOR_PARSER_EXCEPTIONS_H__
