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
		pad< plus< digit >, space> {};

struct null_value :
		pad< string<'n', 'u', 'l', 'l'>, space> {};

struct is_kw :
		pad< string<'i', 's'>, space> {};

 struct as_kw :
	 pad< string<'a', 's'>, space> {};

struct not_kw :
		pad< string<'n', 'o', 't'>, space> {};

struct between_kw :
	   pad< string<'b', 'e', 't', 'w', 'e', 'e', 'n'>, space> {};

struct values_kw :
	   pad< string<'v', 'a', 'l', 'u', 'e', 's'>, space> {};

struct from_kw :
		pad< string<'f', 'r', 'o', 'm'>, space> {};

struct comma_kw :
		pad< one<','>, space> {};

struct open_paren_kw :
		pad< one< '(' >, space > {};

struct close_paren_kw :
		pad< one< ')' >, space > {};

struct value :
		sor<
			ifapply< sql_string, actions::push_literal_str>,
			ifapply< numeric, actions::push_literal_num>
		> {};

struct column_name :
		ifapply< identifier, actions::push_column_ref > {};

struct list_literal :
		seq< open_paren_kw, list< expression, comma_kw >, close_paren_kw > {};

struct term :
		sor<
		   value,
			column_name,
	      seq< open_paren_kw, expression, close_paren_kw >
		 > {};

struct factor :
		seq< term, star< ifapply< seq< one<'*', '/', '%'>, term >, actions::push_binop > > > {};

struct summand :
		seq< factor, star< ifapply< seq< one<'+', '-'>, factor > , actions::push_binop > > > {};

struct operand :
		list< summand, string< '|', '|' > > {};

struct condition_rhs :
		sor<
			seq< is_kw, opt< not_kw >, null_value >,
			seq< between_kw, operand, string<'a', 'n', 'd'>, operand >,
			seq< string<'i', 'n'>, list_literal >
		> {};

struct condition :
		sor< seq< operand, opt< condition_rhs > >,
		     seq< string< 'n', 'o', 't' >, condition >,
		     seq< string< 'e', 'x', 'i', 's', 't', 's' >, open_paren_kw, select, close_paren_kw >
		> {};

struct and_condition :
		seq< condition, opt< string<'a', 'n', 'd'>, condition > > {};

struct or_condition :
		seq< condition, opt< string<'o', 'r'>, condition > > {};

struct expression :
		sor< and_condition, or_condition> {};

struct column_alias :
		seq< as_kw , column_name > {};

struct table_name :
		ifapply< identifier, actions::push_table_ref > {};

struct table_alias :
		seq< as_kw , table_name > {};

struct values_expression :
	seq< values_kw,
        list_literal
   >
	{};

struct table_expression :
	seq< sor< table_name,
	          seq< open_paren_kw , select, close_paren_kw >,
	          list< seq< open_paren_kw , values_expression, close_paren_kw >, comma_kw >
         >,
	     opt< table_alias >
   >
			  {};

struct from :
	seq< from_kw, table_expression > {};

struct select_expression :
		plus< sor< one<'*'>,
		           seq<expression, opt< column_alias > >
		         >
		> {};

struct select :
		seq< pad< string< 's', 'e', 'l', 'e', 'c', 't'>, space >,
			  list<select_expression, comma_kw >,
 	        apply< actions::select >,
           opt< from >
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
