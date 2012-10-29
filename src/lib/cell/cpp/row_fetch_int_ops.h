
        case column::data_type::smallint:
          {
            std::uint16_t *data = static_cast<std::uint16_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(std::uint16_t))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;

        case column::data_type::integer:
          {
            std::uint32_t *data = static_cast<std::uint32_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(std::uint32_t))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;

        case column::data_type::bigint:
          {
            std::uint64_t *data = static_cast<std::uint64_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(std::uint64_t))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;
