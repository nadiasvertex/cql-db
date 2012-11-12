#ifndef __LATTICE_CELL_COLUMN_H__
#define __LATTICE_CELL_COLUMN_H__

#include <memory>
#include <string>

namespace lattice
{
namespace cell
{

struct column {
   enum class data_type {
    smallint,
    integer,
    bigint,
    decimal,
    numeric,
    real,
    double_precision,
    smallserial,
    serial,
    bigserial,
    money,
    varchar,
    fixed_char,
    text,
    bytea,
    timestamp,
    date,
    time,
    interval,
    boolean,
    uuid
   } type;

   std::string name;

   // The limit to the size of the element. For strings this
   // is the number of characters, for decimal it is the number
   // of digits to the left of the decimal point. For byte arrays
   // it is the number of bytes allowed in the column.
   int size;

   // The precision of a numeric value.
   int precision;

   // If true, this column can have a null value inserted.
   bool nullable;

   // The default value of the column, if one is not
   // provided.
   std::string default_value;
};

typedef std::unique_ptr<column> column_handle_type;

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_COLUMN_H__
