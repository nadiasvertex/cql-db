#ifndef __LATTICE_CELL_COLUMN_H__
#define __LATTICE_CELL_COLUMN_H__

#include <memory>
#include <cell/cpp/page.h>

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

   int size;
   int precision;

};

typedef std::shared_ptr<column> column_handle_type;

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_COLUMN_H__
