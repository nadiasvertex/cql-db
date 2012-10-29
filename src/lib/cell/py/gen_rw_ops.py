insert_case_txt = """
        case {type}:
          {{
            {cpptype} *data = static_cast<{cpptype}*>(
                       static_cast<void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof({cpptype}))
              {{
               return false;
              }}
            offset += bytes_written;
          }}
          break;
"""

fetch_case_txt = """
        case {type}:
          {{
            {cpptype} *data = static_cast<{cpptype}*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof({cpptype}))
              {{
                 return false;
              }}

            offset += std::get<1>(results);
          }}
          break;
"""

to_binary_case_txt = """
        case {type}:
          {{
            {cpptype} *data = static_cast<{cpptype}*>(
                       static_cast<void *>(buffer+offset)
            );
            
            *data = static_cast<{cpptype}>(
                strtoll(tuple[i].c_str(), NULL, 10)
            );
                        
            offset += sizeof({cpptype});
          }}
          break;
"""


int_types = [
    ("std::int16_t", "column::data_type::smallint"),
    ("std::int32_t", "column::data_type::integer"),
    ("std::int64_t", "column::data_type::bigint"),
]

with open("src/lib/cell/cpp/row_insert_int_ops.h", "w") as out:
   for cpptype, sqltype in int_types:
      d = {"cpptype":cpptype,
           "type":sqltype }
      out.write(insert_case_txt.format(**d))

with open("src/lib/cell/cpp/row_fetch_int_ops.h", "w") as out:
   for cpptype, sqltype in int_types:
      d = {"cpptype":cpptype,
           "type":sqltype }
      out.write(fetch_case_txt.format(**d))

with open("src/lib/cell/cpp/row_to_binary_int_ops.h", "w") as out:
   for cpptype, sqltype in int_types:
      d = {"cpptype":cpptype,
           "type":sqltype }
      out.write(to_binary_case_txt.format(**d))
