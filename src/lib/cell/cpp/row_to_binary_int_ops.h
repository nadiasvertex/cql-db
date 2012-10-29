
        case column::data_type::smallint:
          {
            std::int16_t *data = static_cast<std::int16_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            *data = static_cast<std::int16_t>(
                strtoll(tuple[i].c_str(), NULL, 10)
            );
                        
            offset += sizeof(std::int16_t);
          }
          break;

        case column::data_type::integer:
          {
            std::int32_t *data = static_cast<std::int32_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            *data = static_cast<std::int32_t>(
                strtoll(tuple[i].c_str(), NULL, 10)
            );
                        
            offset += sizeof(std::int32_t);
          }
          break;

        case column::data_type::bigint:
          {
            std::int64_t *data = static_cast<std::int64_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            *data = static_cast<std::int64_t>(
                strtoll(tuple[i].c_str(), NULL, 10)
            );
                        
            offset += sizeof(std::int64_t);
          }
          break;
