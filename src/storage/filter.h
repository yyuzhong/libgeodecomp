#ifndef LIBGEODECOMP_STORAGE_FILTER_H
#define LIBGEODECOMP_STORAGE_FILTER_H

#include <libgeodecomp/config.h>
#include <libgeodecomp/io/logger.h>
#include <libgeodecomp/storage/filterbase.h>

#ifdef LIBGEODECOMP_WITH_SILO
#include <silo.h>
#endif

#ifdef LIBGEODECOMP_WITH_MPI
#include <libgeodecomp/communication/typemaps.h>
#endif

namespace LibGeoDecomp {

namespace FilterHelpers {

#ifdef LIBGEODECOMP_WITH_SILO

template<typename MEMBER>
class GetSiloTypeID
{
public:
    inline int operator()()
    {
        LOG(WARN, "Warning: using type unknown to Silo for output");
        return DB_NOTYPE;
    }
};

template<>
class GetSiloTypeID<int>
{
public:
    inline int operator()()
    {
        return DB_INT;
    }
};

template<>
class GetSiloTypeID<short int>
{
public:
    inline int operator()()
    {
        return DB_SHORT;
    }
};

template<>
class GetSiloTypeID<float>
{
public:
    inline int operator()()
    {
        return DB_FLOAT;
    }
};

template<>
class GetSiloTypeID<double>
{
public:
    inline int operator()()
    {
        return DB_DOUBLE;
    }
};

template<>
class GetSiloTypeID<char>
{
public:
    inline int operator()()
    {
        return DB_CHAR;
    }
};

template<>
class GetSiloTypeID<long long>
{
public:
    inline int operator()()
    {
        return DB_LONG_LONG;
    }
};

#endif

#ifdef LIBGEODECOMP_WITH_MPI

template<typename MEMBER, int FLAG>
class GetMPIDatatype0;

template<typename MEMBER, int FLAG>
class GetMPIDatatype1;

template<typename MEMBER>
class GetMPIDatatype0<MEMBER, 0>
{
public:
    inline MPI_Datatype operator()()
    {
        throw std::invalid_argument("no MPI data type defined for this type");
    }
};

template<typename MEMBER>
class GetMPIDatatype0<MEMBER, 1>
{
public:
    inline MPI_Datatype operator()()
    {
        return Typemaps::lookup<MEMBER>();
    }
};

template<typename MEMBER>
class GetMPIDatatype1<MEMBER, 0>
{
public:
    inline MPI_Datatype operator()()
    {
        return GetMPIDatatype0<MEMBER, APITraits::HasLookupMemberFunction<Typemaps, MPI_Datatype, MEMBER>::value>()();
    }
};

template<typename MEMBER>
class GetMPIDatatype1<MEMBER, 1>
{
public:
    inline MPI_Datatype operator()()
    {
        return APITraits::SelectMPIDataType<MEMBER>::value();
    }
};

template<typename MEMBER>
class GetMPIDatatype
{
public:
    inline MPI_Datatype operator()()
    {
        return GetMPIDatatype1<
            MEMBER,
            APITraits::HasValueFunction<APITraits::SelectMPIDataType<MEMBER>, MPI_Datatype>::value>()();
    }
};

#endif

/**
 * We're intentionally giving only few specializations for this helper
 * as it's mostly meant to be used with VisIt's BOV format, and this
 * is only defined on tree types.
 */
template<typename MEMBER>
class GetTypeName
{
public:
    std::string operator()() const
    {
        throw std::invalid_argument("no string representation known for member type");
    }
};

template<>
class GetTypeName<bool>
{
public:
    std::string operator()() const
    {
        return "BYTE";
    }
};

template<>
class GetTypeName<char>
{
public:
    std::string operator()() const
    {
        return "BYTE";
    }
};

template<>
class GetTypeName<float>
{
public:
    std::string operator()() const
    {
        return "FLOAT";
    }
};

template<>
class GetTypeName<double>
{
public:
    std::string operator()() const
    {
        return "DOUBLE";
    }
};

}

/**
 * Derive from this class if you wish to add custom data
 * adapters/converters to your Selector.
 */
template<typename CELL, typename MEMBER, typename EXTERNAL>
class Filter : public FilterBase<CELL>
{
public:
    friend class Serialization;

    std::size_t sizeOf() const
    {
        return sizeof(EXTERNAL);
    }

#ifdef LIBGEODECOMP_WITH_SILO
    int siloTypeID() const
    {
        return FilterHelpers::GetSiloTypeID<EXTERNAL>()();
    }
#endif

#ifdef LIBGEODECOMP_WITH_MPI
    virtual MPI_Datatype mpiDatatype() const
    {
        return FilterHelpers::GetMPIDatatype<EXTERNAL>()();
    }
#endif

    virtual std::string typeName() const
    {
        return FilterHelpers::GetTypeName<EXTERNAL>()();
    }

    virtual int arity() const
    {
        return 1;
    }

    /**
     * Copy a streak of variables to an AoS layout.
     */
    virtual void copyStreakInImpl(const EXTERNAL *first, const EXTERNAL *last, MEMBER *target) = 0;

    /**
     * Extract a steak of members from an AoS layout.
     */
    virtual void copyStreakOutImpl(const MEMBER *first, const MEMBER *last, EXTERNAL *target) = 0;

    /**
     * Copy a streak of variables to the members of a streak of cells.
     */
    virtual void copyMemberInImpl(
        const EXTERNAL *source, CELL *target, int num, MEMBER CELL:: *memberPointer) = 0;

    /**
     * Extract a streak of members from a streak of cells.
     */
    virtual void copyMemberOutImpl(
        const CELL *source, EXTERNAL *target, int num, MEMBER CELL:: *memberPointer) = 0;

    /**
     * Do not override this function! It is final.
     */
    void copyStreakIn(const char *first, const char *last, char *target)
    {
        copyStreakInImpl(
            reinterpret_cast<const EXTERNAL*>(first),
            reinterpret_cast<const EXTERNAL*>(last),
            reinterpret_cast<MEMBER*>(target));
    }

    /**
     * Do not override this function! It is final.
     */
    void copyStreakOut(const char *first, const char *last, char *target)
    {
        copyStreakOutImpl(
            reinterpret_cast<const MEMBER*>(first),
            reinterpret_cast<const MEMBER*>(last),
            reinterpret_cast<EXTERNAL*>(target));
    }

    /**
     * Do not override this function! It is final.
     */
    void copyMemberIn(
        const char *source, CELL *target, int num, char CELL:: *memberPointer)
    {
        copyMemberInImpl(
            reinterpret_cast<const EXTERNAL*>(source),
            target,
            num,
            reinterpret_cast<MEMBER CELL:: *>(memberPointer));
    }

    /**
     * Do not override this function! It is final.
     */
    void copyMemberOut(
        const CELL *source, char *target, int num, char CELL:: *memberPointer)
    {
        copyMemberOutImpl(
            source,
            reinterpret_cast<EXTERNAL*>(target),
            num,
            reinterpret_cast<MEMBER CELL:: *>(memberPointer));
    }

    bool checkExternalTypeID(const std::type_info& otherID) const
    {
        return typeid(EXTERNAL) == otherID;
    }
};

}

#endif