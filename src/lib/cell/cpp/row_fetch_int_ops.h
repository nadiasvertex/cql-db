
        case column::data_type::smallint:
          {
            std::int16_t *data = static_cast<std::int16_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(std::int16_t))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;

        case column::data_type::integer:
          {
            std::int32_t *data = static_cast<std::int32_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(std::int32_t))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;

        case column::data_type::bigint:
          {
            std::int64_t *data = static_cast<std::int64_t*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(std::int64_t))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;

        case column::data_type::real:
          {
            float *data = static_cast<float*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(float))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;

        case column::data_type::double_precision:
          {
            double *data = static_cast<double*>(
                       static_cast<void *>(buffer+offset)
            );
            auto results = p->fetch_object(oid, *data);
            if (std::get<0>(results)==false ||
                std::get<1>(results)==0 ||
                std::get<1>(results)<sizeof(double))
              {
                 return false;
              }

            offset += std::get<1>(results);
          }
          break;
