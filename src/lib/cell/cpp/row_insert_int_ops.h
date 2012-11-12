
        case column::data_type::smallint:
          {
            const std::int16_t *data = static_cast<const std::int16_t*>(
                       static_cast<const void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(std::int16_t))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;

        case column::data_type::integer:
          {
            const std::int32_t *data = static_cast<const std::int32_t*>(
                       static_cast<const void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(std::int32_t))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;

        case column::data_type::bigint:
          {
            const std::int64_t *data = static_cast<const std::int64_t*>(
                       static_cast<const void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(std::int64_t))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;

        case column::data_type::real:
          {
            const float *data = static_cast<const float*>(
                       static_cast<const void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(float))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;

        case column::data_type::double_precision:
          {
            const double *data = static_cast<const double*>(
                       static_cast<const void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(double))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;
