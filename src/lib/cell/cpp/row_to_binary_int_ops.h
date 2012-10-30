
        case column::data_type::smallint:
          {
            std::int16_t *data = static_cast<std::int16_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            auto bytes=unstringify::to_binary(tuple[i], data);
            if (bytes==0)
              {
                 return false;
              }
                        
            offset += bytes;
          }
          break;

        case column::data_type::integer:
          {
            std::int32_t *data = static_cast<std::int32_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            auto bytes=unstringify::to_binary(tuple[i], data);
            if (bytes==0)
              {
                 return false;
              }
                        
            offset += bytes;
          }
          break;

        case column::data_type::bigint:
          {
            std::int64_t *data = static_cast<std::int64_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            auto bytes=unstringify::to_binary(tuple[i], data);
            if (bytes==0)
              {
                 return false;
              }
                        
            offset += bytes;
          }
          break;

        case column::data_type::real:
          {
            float *data = static_cast<float*>(
                       static_cast<void *>(buffer+offset)
            );
            
            auto bytes=unstringify::to_binary(tuple[i], data);
            if (bytes==0)
              {
                 return false;
              }
                        
            offset += bytes;
          }
          break;

        case column::data_type::double_precision:
          {
            double *data = static_cast<double*>(
                       static_cast<void *>(buffer+offset)
            );
            
            auto bytes=unstringify::to_binary(tuple[i], data);
            if (bytes==0)
              {
                 return false;
              }
                        
            offset += bytes;
          }
          break;

        case column::data_type::varchar:
          {
            std::uint8_t *data = static_cast<std::uint8_t*>(
                       static_cast<void *>(buffer+offset)
            );
            
            auto bytes=unstringify::to_binary(tuple[i], data);
            if (bytes==0)
              {
                 return false;
              }
                        
            offset += bytes;
          }
          break;
