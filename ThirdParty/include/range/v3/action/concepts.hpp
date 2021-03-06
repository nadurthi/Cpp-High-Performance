/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_ACTION_CONCEPTS_HPP
#define RANGES_V3_ACTION_CONCEPTS_HPP

#include <utility>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/utility/functional.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \cond
        namespace detail
        {
            template<typename T>
            struct movable_input_iterator
            {
                using iterator_category = std::input_iterator_tag;
                using value_type = T;
                using difference_type = std::ptrdiff_t;
                using pointer = T *;
                using reference = T &&;

                movable_input_iterator() = default;
                movable_input_iterator &operator++();
                movable_input_iterator operator++(int);
                bool operator==(movable_input_iterator const &) const;
                bool operator!=(movable_input_iterator const &) const;
                T && operator*() const;
            };
        }
        /// \endcond

        /// \addtogroup group-concepts
        /// @{
        namespace concepts
        {
            // std::array is a SemiContainer, native arrays are not.
            struct SemiContainer
              : refines<ForwardRange>
            {
                template<typename T>
                auto requires_(T&&) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<DefaultConstructible, uncvref_t<T>>(),
                        concepts::model_of<Movable, uncvref_t<T>>(),
                        concepts::is_false(is_view<T>())
                    ));
            };

            // std::vector is a Container, std::array is not
            struct Container
              : refines<SemiContainer>
            {
                template<typename T, typename I = detail::movable_input_iterator<range_value_t<T>>>
                auto requires_(T&&) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Constructible, uncvref_t<T>, I, I>()
                    ));
            };
        }

        template<typename T>
        using SemiContainer = concepts::models<concepts::SemiContainer, T>;

        template<typename T>
        using Container = concepts::models<concepts::Container, T>;

        namespace concepts
        {
            struct Reservable
              : refines<Container>
            {
                template <typename C>
                using size_type =
                    decltype(std::declval<const C&>().size());

                template <typename C, typename S = size_type<C>>
                auto requires_(C&& c, S&& s = S{}) -> decltype(
                    concepts::valid_expr(
                        concepts::model_of<Integral, S>(),
                        ((void)c.reserve(s), 42)
                    ));
            };

            struct ReserveAndAssignable
              : refines<Reservable(_1), InputIterator(_2)>
            {
                template <typename C, typename I>
                auto requires_(C&& c, I&& i) -> decltype(
                    concepts::valid_expr(
                        ((void)c.assign(i, i), 42)
                    ));
            };
        }

        template <typename C>
        using Reservable = concepts::models<concepts::Reservable, C>;

        template <typename C, typename I>
        using ReserveAndAssignable =
            concepts::models<concepts::ReserveAndAssignable, C, I>;

        template <typename C>
        using RandomAccessReservable =
            meta::fast_and<Reservable<C>, RandomAccessRange<C>>;

        /// \cond
        namespace detail
        {
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename T, CONCEPT_REQUIRES_(Container<T>::value)>
#else
            template<typename T, CONCEPT_REQUIRES_(Container<T>())>
#endif
            std::true_type is_lvalue_container_like(T &);

#ifdef RANGES_WORKAROUND_MSVC_LVALUE_BINDS_RVALUE
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename T, CONCEPT_REQUIRES_(!std::is_reference<T>::value && Container<T>::value)>
#else
            template<typename T, CONCEPT_REQUIRES_(!std::is_reference<T>() && Container<T>())>
#endif
            void is_lvalue_container_like(T &&) = delete;
#endif

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename T, CONCEPT_REQUIRES_(Container<T>::value)>
#else
            template<typename T, CONCEPT_REQUIRES_(Container<T>())>
#endif
            std::true_type is_lvalue_container_like(reference_wrapper<T>);

#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
            template<typename T, CONCEPT_REQUIRES_(Container<T>::value)>
#else
            template<typename T, CONCEPT_REQUIRES_(Container<T>())>
#endif
            std::true_type is_lvalue_container_like(std::reference_wrapper<T>);
        }
        /// \endcond

        namespace concepts
        {
            struct LvalueContainerLike
              : refines<ForwardRange>
            {
                template<typename T>
                auto requires_(T&& t) -> decltype(
                    concepts::valid_expr(
                        detail::is_lvalue_container_like(std::forward<T>(t))
                    ));
            };
        }

        template<typename T>
        using LvalueContainerLike = concepts::models<concepts::LvalueContainerLike, T>;
        /// @}
    }
}

#endif
