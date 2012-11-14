#ifndef __LATTICE_CELL_RANGE_H__
#define __LATTICE_CELL_RANGE_H__

#include <cstdint>
#include <list>
#include <utility>

namespace lattice {
namespace common {

/**
 * Implements a set of one dimensional segments. The class provides the
 * ability to test for coverage, and to add a point to the segment set.
 *
 * The type implemented for the range boundary must implement operator+(T,int),
 * operator-(T,int), operator<(T,T), and operator==(T,T).
 */
template<typename T>
class range
{
public:
   typedef std::pair<T, T> value_type;

   typedef std::list<value_type> range_list_type;

   typedef typename range_list_type::iterator iterator;

   typedef typename range_list_type::size_type size_type;
private:

   range_list_type segments;

public:

   iterator begin()
   {
      return segments.begin();
   }

   iterator end()
   {
      return segments.end();
   }

   value_type& front()
   {
      return segments.front();
   }

   value_type& back()
   {
      return segments.back();
   }

   size_type size() const
   {
      return segments.size();
   }

   /**
    * Insert a new point into the range. This may cause the creation
    * of a new segment.
    *
    * @param point: The item to insert.
    *
    */
   iterator insert(const T& point)
   {
      using namespace std::rel_ops;

      for (auto pos = segments.begin(); pos != segments.end(); ++pos)
         {
            auto& el = *pos;
            if (el.first <= point && el.second >= point)
               {
                  // point is already contained in the range.
                  return pos;
               }

            if (el.first - 1 == point)
               {
                  // extend range to the left.
                  el.first = point;
                  return pos;
               }

            if (el.second + 1 == point)
               {
                  // extend range to the right
                  el.second = point;
                  return pos;
               }

            if (point < el.first)
               {
                  // range belongs before this element.
                  return segments.insert(pos, std::make_pair(point, point));
               }
         }

      return segments.emplace(segments.end(), point, point);
   }

   /**
    * Determines if the given point is present in the range.
    *
    * @param point: The item to find.
    *
    * @returns: true if the point is found in the range, false otherwise.
    */
   bool contains(const T& point)
   {
      using namespace std::rel_ops;

      return ! (find(point) == end());
   }

   /**
    * Finds the point and returns the segment that it
    * belongs to.
    *
    * @param point: The item to find.
    *
    * @returns: An iterator to the segment that the point belongs in, otherwise
    *           it returns end().
    */
   iterator find(const T& point)
   {
      using namespace std::rel_ops;

      for (auto pos = segments.begin(); pos!=segments.end(); ++pos)
         {
            auto& seg = *pos;
            if (seg.first<=point && seg.second>=point)
               {
                  return pos;
               }

            if (seg.first>point)
               {
                  return segments.end();
               }
         }

      return segments.end();
   }

};

} // namespace common
} // namespace lattice

#endif // __LATTICE_CELL_RANGE_H__
