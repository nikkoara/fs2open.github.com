// -*- mode: c++; -*-

#ifndef FREESPACE2_UTIL_FLAGSET_HH
#define FREESPACE2_UTIL_FLAGSET_HH

#include "defs.hh"

#include <cstdint>

#include <bitset>

template< typename, size_t >
struct flagsubset;

template< typename T, size_t SIZE = static_cast< size_t >(T::SIZEOF_ENUM) >
struct flagset
{
        flagset() { }

        flagset(T arg)
        {
                set(arg);
        }

        flagset(std::initializer_list< T > args)
        {
                for (auto arg : args)
                        set(arg);
        }

        bool operator[](T idx) const
        {
                return fs[(static_cast< size_t >(idx))];
        };

        template< size_t M >
        bool operator[](const flagsubset< T, M > &fss) const
        {
                return fss.check_flags(*this);
        }

        flagset< T > operator&(const flagset< T > &other) const
        {
                flagset< T > result;
                result.fs = this->fs & other.fs;
                return result;
        }

        flagset< T > operator+(T f) const
        {
                flagset< T > result = *this;
                result.set(f);
                return result;
        }

        flagset< T > &operator+=(T f)
        {
                this->set(f);
                return *this;
        }

        flagset< T > operator-(T f) const
        {
                flagset< T > result = *this;
                result.remove(f);
                return result;
        }

        flagset< T > &operator-=(T f)
        {
                this->remove(f);
                return *this;
        }

        flagset< T > operator|(const flagset< T > &other) const
        {
                flagset< T > result;
                result.fs = this->fs | other.fs;
                return result;
        }

        flagset< T > &operator|=(const flagset< T > &other)
        {
                this->fs |= other.fs;
                return *this;
        }

        void operator|=(T f) { set(f); }

        bool operator==(const flagset< T > &other) const
        {
                return this->fs == other.fs;
        }

        bool operator!=(const flagset< T > &other) const
        {
                return this->fs != other.fs;
        }

        void reset() { fs.reset(); }

        flagset< T > &set(T idx, bool value = true)
        {
                fs.set(static_cast< size_t >(idx), value);
                return *this;
        }

        template< typename Iterator >
        flagset< T > &set_multiple(Iterator iter, const Iterator last)
        {
                for (; iter != last; ++iter)
                        set(*iter);

                return *this;
        }

        flagset< T > &remove(T idx)
        {
                return set(idx, false);
        }

        template< typename Iterator >
        flagset< T > &remove_multiple(Iterator iter, const Iterator last)
        {
                for (; iter != last; ++iter)
                        set(*iter, false);

                return *this;
        }

        flagset< T > &toggle(T idx)
        {
                fs[static_cast< size_t >(idx)] = !fs[static_cast< size_t >(idx)];
                return *this;
        }

        bool  any_set() { return fs.any();  }
        bool none_set() { return fs.none(); }

        void from_u64(std::uint64_t num) { fs = (unsigned long)num; }
        std::uint64_t to_u64() const { return (std::uint64_t)fs.to_ulong(); }

public:
        std::bitset< SIZE > fs;
};

template< typename T, size_t SIZE = static_cast< size_t >(T::SIZEOF_ENUM) >
struct flagsubset
{
        flagsubset(flagsubset< T, SIZE - 1 > lhs, T rhs)
                : lhs(lhs), rhs(rhs)
        { }

        bool check_flags(const flagset< T > &fs) const
        {
                //
                // check flags left to right
                //
                return lhs.check_flags(fs) || fs[rhs];
        }

protected:
        flagsubset< T, SIZE - 1 > lhs;
        T rhs;
};

template< typename T >
struct flagsubset< T, 2 >
{
        flagsubset(T lhs, T rhs) : lhs(lhs), rhs(rhs) { }

        bool check_flags(const flagset< T > &fs) const
        {
                return fs[lhs] || fs[rhs];
        }

        T lhs, rhs;
};

template< typename T >
struct is_flag_enum
{
        static const bool value = std::is_enum< T >::value;
};

template< typename T >
typename std::enable_if< is_flag_enum< T >::value, flagsubset< T, 2 > >::type
operator,(T lhs, T rhs)
{
        return flagsubset< T, 2 >(lhs, rhs);
}

template< typename T, size_t SIZE >
typename std::enable_if< is_flag_enum< T >::value, flagsubset< T, SIZE + 1 > >::type
operator,(flagsubset< T, SIZE > lhs, T rhs)
{
        return flagsubset< T, SIZE + 1 >(lhs, rhs);
}

#endif // FREESPACE2_UTIL_FLAGSET_HH
