
        case column::data_type::smallint:
          {
            std::uint16_t *data = static_cast<std::uint16_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(std::uint16_t))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;

        case column::data_type::integer:
          {
            std::uint32_t *data = static_cast<std::uint32_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(std::uint32_t))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;

        case column::data_type::bigint:
          {
            std::uint64_t *data = static_cast<std::uint64_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(std::uint64_t))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;
