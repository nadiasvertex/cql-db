
        case column::data_type::smallint:
          {
            std::int16_t *data = static_cast<std::int16_t*>(
                       static_cast<void *>(buffer+offset)
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
            std::int32_t *data = static_cast<std::int32_t*>(
                       static_cast<void *>(buffer+offset)
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
            std::int64_t *data = static_cast<std::int64_t*>(
                       static_cast<void *>(buffer+offset)
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
            float *data = static_cast<float*>(
                       static_cast<void *>(buffer+offset)
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
            double *data = static_cast<double*>(
                       static_cast<void *>(buffer+offset)
            );
            auto bytes_written = p->insert_object(oid, *data);
            if (bytes_written == 0 || bytes_written!=sizeof(double))
              {
               return false;
              }
            offset += bytes_written;
          }
          break;
