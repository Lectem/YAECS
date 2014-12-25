#ifndef TUPLEGET_H_INCLUDED
#define TUPLEGET_H_INCLUDED

/**
 * \file
 * \brief
 * \author Lectem
 */

#include <tuple>
namespace detail
{

    template <class Tsearched, class... Ttypes>
    struct typeIdx;

    template <class Tsearched, class Tfirst, class... Trest>
    struct typeIdx<Tsearched, Tfirst, Trest...>
    {
        static constexpr std::size_t value = 1u + typeIdx<Tsearched, Trest...>::value;
    };

    template <class Tsearched, class... Ttail>
    struct typeIdx<Tsearched, Tsearched, Ttail...>
    {
        static constexpr std::size_t value = 0u;
    };

    template <class Tsearched>
    struct typeIdx<Tsearched>
    {
        static_assert(! sizeof(Tsearched), "Type not found.");
    };
}

template <class Tsearched, class... Ttuple>
Tsearched &tupleGet(std::tuple<Ttuple...> &tuple)
{
    return std::get < detail::typeIdx<Tsearched, Ttuple...>::value > (tuple);
}

#endif // TUPLEGET_H_INCLUDED
