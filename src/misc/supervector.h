#ifndef LIBGEODECOMP_MISC_SUPERVECTOR_H
#define LIBGEODECOMP_MISC_SUPERVECTOR_H

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

#include <libgeodecomp/config.h>
#ifdef LIBGEODECOMP_FEATURE_BOOST_SERIALIZATION
#include <boost/serialization/vector.hpp>
#endif
#include <boost/move/move.hpp>

namespace LibGeoDecomp {

/**
 * This class adds some functionality the std::vector ought to
 * provide (but fails to).
 */
template<typename T, typename Allocator = std::allocator<T> >
class SuperVector : public std::vector<T, Allocator>
{
public:
    using std::vector<T, Allocator>::back;
    using std::vector<T, Allocator>::begin;
    using std::vector<T, Allocator>::insert;
    using std::vector<T, Allocator>::end;
    using std::vector<T, Allocator>::erase;
    using std::vector<T, Allocator>::front;
    using std::vector<T, Allocator>::pop_back;
    using std::vector<T, Allocator>::push_back;

    typedef std::vector<T, Allocator> VectorType;

    typedef typename std::vector<T, Allocator>::iterator iterator;
    typedef typename std::vector<T, Allocator>::const_iterator const_iterator;

    inline SuperVector()
    {}

    inline SuperVector(std::size_t i) :
        std::vector<T>(i)
    {}

    inline SuperVector(std::size_t i, T t) :
        std::vector<T>(i, t)
    {}

    template<typename ITERATOR>
    inline SuperVector(ITERATOR start, ITERATOR end) :
        std::vector<T>(start, end)
    {}

    inline SuperVector(std::vector<T, Allocator> const & other) :
        std::vector<T>(other)
    {}

    inline SuperVector(BOOST_RV_REF(std::vector<T, Allocator>) other) :
        std::vector<T>(boost::move(other))
    {}

    inline SuperVector(BOOST_RV_REF(VectorType) other) :
        std::vector<T>(boost::move(static_cast<std::vector<T>(other)))
    {}

    /**
     * Deletes items from _self_ that are equal to @param obj
     */
    inline void del(const T& obj)
    {
        erase(std::remove(begin(), end(), obj), end());
    }

    // We have to use the inherited operator by hand, as this requires a cast
    inline bool operator==(const SuperVector<T>& comp) const
    {
        return ((std::vector<T>)*this) == ((std::vector<T>)comp);
    }

    inline std::string toString() const
    {
        std::ostringstream temp;
        temp << "[";
        for (const_iterator i = begin(); i != end();) {
            temp << *i;
            i++;
            if (i != end())
                temp << ", ";
        }
        temp << "]";
        return temp.str();
    }

    inline SuperVector& operator<<(const T& obj)
    {
        push_back(obj);
        return *this;
    }

    inline void append(const SuperVector& other)
    {
        insert(end(), other.begin(), other.end());
    }

    inline SuperVector operator+(const SuperVector& other) const
    {
        SuperVector ret = *this;
        ret.append(other);
        return ret;
    }

    inline void push_front(const T& obj)
    {
        insert(begin(), obj);
    }

    inline T pop_front()
    {
        T ret = front();
        erase(begin());
        return ret;
    }

    inline T pop()
    {
        T ret = back();
        pop_back();
        return ret;
    }

    inline T sum() const
    {
        T res = 0;
        for (const_iterator i = begin(); i != end(); i++) {
            res += *i;
        }
        return res;

    }

    inline bool contains(const T& element) const
    {
        return std::find(begin(), end(), element) != end();
    }

    inline void sort()
    {
        std::sort(begin(), end());
    }

    T& (max)()
    {
        return *(std::max_element(begin(), end()));
    }

    const T& (max)() const
    {
        return *(std::max_element(begin(), end()));
    }

#ifdef LIBGEODECOMP_FEATURE_BOOST_SERIALIZATION
    template <typename ARCHIVE>
    void serialize(ARCHIVE& ar, unsigned)
    {
        ar & static_cast<std::vector<T, Allocator>&>(*this);
    }
#endif
};

}

template<typename _CharT, typename _Traits, typename T>
std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& __os,
           const LibGeoDecomp::SuperVector<T>& superVector)
{
    __os << superVector.toString();
    return __os;
}

#endif
