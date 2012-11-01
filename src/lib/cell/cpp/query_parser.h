#ifndef __LATTICE_CELL_QUERY_PARSER_H__
#define __LATTICE_CELL_QUERY_PARSER_H__

#include <vector>

#include <cell/cpp/predicate.h>
#include <cell/cpp/database.h>

#include <pegtl.hh>

namespace lattice
{
namespace cell
{

namespace recognizer
{

using namespace pegtl;

struct expression;
struct select;

struct sql_string :
		enclose< alnum, one<'\'' > {};

struct numeric :
		plus< digit > {};

struct value :
		sor<
			sql_string,
			numeric
		> {};

struct column_name :
		identifier {};

struct term :
		sor<
			column_name,
			seq< pad<one< '(', space >>, expression, pad<one< ')', space >> >
		 > {};

struct factor :
		seq< term, star< seq< one<'*', '/', '%'>, term > > {};

struct summand :
		seq< factor, star< seq< one<'+', '-'>, factor > > {};

struct operand :
		list< summand, one< '||' > > {};

struct condition_rhs :
		sor<
			seq< string<'i', 's'>, opt< string<'n', 'o', 't'> >, string<'n', 'u', 'l', 'l'> >,
			seq< string<'b', 'e', 't', 'w', 'e', 'e', 'n'>, operand, string<'a', 'n', 'd'>, operand >,
			seq< string<'i', 'n'>, pad<one< '(', space >>, list<expression, one<','> >, pad< one < ')', space > > >
		> {};

struct condition :
		sor< seq< operand, opt< condition_rhs > >,
		     seq< string< 'n', 'o', 't' >, condition >,
		     seq< string< 'e', 'x', 'i', 's', 't', 's' >, pad<one< '(' >>, select, pad< one< ')' > > >
		> {};

struct and_condition :
		seq< condition, string<'a', 'n', 'd'>, condition> {};

struct or_condition :
		seq< condition, string<'o', 'r'>, condition> {};

struct expression :
		sor< and_condition, or_condition> {};

struct column_alias :
		seq< string<'a', 's'>, column_name > {};

struct from :
		seq< string< 'f', 'r', 'o', 'm'> > {};

struct select_expression :
		plus< sor< one<'*'>,
		           seq<expression, opt< column_alias > >
		         >
		> {};

struct select :
		seq< string< 's', 'e', 'l', 'e', 'c', 't'>, list< select_expression, one<','> > > {};


} //end parser namespace

class query_parser
{
public:

private:
  database&   db;
  std::string query_data;

public:
  query_parser(database& _db, std::string _query_data) :
      db(_db), query_data(_query_data)
  {
  }

  bool parse()
  {
	  pegtl::basic_parse_string< recognizer::select >(query_data);
  }

};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_QUERY_PARSER_H__
