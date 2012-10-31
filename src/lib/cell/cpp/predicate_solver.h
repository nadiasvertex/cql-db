#ifndef __LATTICE_CELL_PREDICATE_H__
#define __LATTICE_CELL_PREDICATE_H__

#include <cstdint>
#include <memory>
#include <string>
#include <cell/cpp/page_cursor.h>

namespace lattice
{
namespace cell
{

  class predicate
  {
  public:
    virtual int cmp(page_cursor& cursor)=0;
  };

  class scalar_predicate : public predicate
  {
  /**
   * The type of data to request.
   */
  column::data_type type;
 
  union {
    std::int16_t i16;
    std::int32_t i32;
    std::int64_t i64;

    float        f32;
    double       f64; 

    std::string *s;

  } value;


public:
  scalar_predicate() {}
  virtual ~scalar_predicate() 
    {
      if (type==column::data_type::varchar)
	{
	  delete value.s;
	}
    }
  
  template <typename T>
  void set_value(column::data_type t, const T& data) {
    type = t;
    switch(type)
      {
      case column::data_type::smallint:
	value.i16 = data;
	break;

      case column::data_type::integer:
	value.i32 = data;
	break;

      case column::data_type::bigint:
	value.i32 = data;
	break;

      case column::data_type::real:
	value.f32 = data;
	break;

      case column::data_type::double_precision:
	value.f64 = data;
	break;      
      }
  }
 
    virtual int cmp(page_cursor& cursor)
    {
      switch(type)
      {
      case column::data_type::smallint:
	return cursor.cmp(value.i16);
	break;

      case column::data_type::integer:
	return cursor.cmp(value.i32);
	break;

      case column::data_type::bigint:
	return cursor.cmp(value.i32);
	break;

      case column::data_type::real:
	return cursor.cmp(value.f32);
	break;

      case column::data_type::double_precision:
	return cursor.cmp(value.f64);
	break;      
    
      case column::data_type::varchar:
	return cursor.cmp(*value.s);
	break;      
      }
    }   
};


 template<>
   void scalar_predicate::set_value<>(column::data_type t, const std::string& data)
      {
	type = t;
	switch(type)
	  {
	  case column::data_type::varchar:
	    value.s = new std::string(data);
	    break;
	  }
      }


} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_QUERY_H__
