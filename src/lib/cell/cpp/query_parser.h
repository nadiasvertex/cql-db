#ifndef __LATTICE_CELL_QUERY_PARSER_H__
#define __LATTICE_CELL_QUERY_PARSER_H__

#include <vector>

#include <cell/cpp/parser_actions.h>
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
		seq< one<'\''>, star< not_one<'\'' >  >, one<'\''> > {};

struct numeric :
		plus< digit > {};

struct null_value :
		string<'n', 'u', 'l', 'l'> {};

struct value :
		sor<
			ifapply< sql_string, actions::push_literal_str>,
			ifapply< numeric, actions::push_literal_num>
		> {};

struct column_name :
		ifapply< identifier, actions::push_column_ref > {};

struct term :
		sor<
		   value,
			column_name,
			seq< pad< one< '(' >, space >, expression, pad< one< ')'>, space > >
		 > {};

struct factor :
		seq< term, star< ifapply< seq< one<'*', '/', '%'>, term >, actions::push_binop > > > {};

struct summand :
		seq< factor, star< ifapply< seq< one<'+', '-'>, factor > , actions::push_binop > > > {};

struct operand :
		list< summand, string< '|', '|' > > {};

struct condition_rhs :
		sor<
			seq< string<'i', 's'>, opt< string<'n', 'o', 't'> >, null_value >,
			seq< string<'b', 'e', 't', 'w', 'e', 'e', 'n'>, operand, string<'a', 'n', 'd'>, operand >,
			seq< string<'i', 'n'>, pad<one< '(' > , space >, list<expression, one<','> >, pad< one < ')'>, space > >
		> {};

struct condition :
		sor< seq< operand, opt< condition_rhs > >,
		     seq< string< 'n', 'o', 't' >, condition >,
		     seq< string< 'e', 'x', 'i', 's', 't', 's' >, pad<one< '(' >, space >, select, pad< one< ')' >, space > >
		> {};

struct and_condition :
		seq< condition, opt< string<'a', 'n', 'd'>, condition > > {};

struct or_condition :
		seq< condition, opt< string<'o', 'r'>, condition > > {};

struct expression :
		sor< and_condition, or_condition> {};

struct column_alias :
		seq< pad< string<'a', 's'>, space> , column_name > {};

struct from :
		seq< string< 'f', 'r', 'o', 'm'> > {};

struct select_expression :
		plus< sor< one<'*'>,
		           seq<expression, opt< column_alias > >
		         >
		> {};

struct select :
		seq< pad< string< 's', 'e', 'l', 'e', 'c', 't'>, space >,
			  list<select_expression, pad< one<','>, space > >,
			  apply< actions::select >
		> {};

} //end parser namespace

class query_parser
{
public:

private:
  database&   db;
  std::string query_data;
  actions::query q;

public:
  query_parser(database& _db, std::string _query_data) :
      db(_db), query_data(_query_data)
  {
  }

  bool parse()
  {
	  actions::node_list_type s;
	  pegtl::basic_parse_string< recognizer::select>(query_data, s, q);
	  return true;
  }

  actions::query& get_query()
  {
	  return std::ref(q);
  }

};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_QUERY_PARSER_H__
