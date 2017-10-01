///
// optional - An implementation of std::optional with extensions
// Written in 2017 by Simon Brand (@TartanLlama)
//
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
///

#include <type_traits>
#include <utility>
#include <exception>
#include <new>

namespace tl {
    template <class T> using remove_cv_t = typename std::remove_cv<T>::type;
    template <class T> using remove_const_t = typename std::remove_const<T>::type;
    template <class T> using remove_volatile_t = typename std::remove_volatile<T>::type;
    template <class T> using add_cv_t = typename std::add_cv<T>::type;
    template <class T> using add_const_t = typename std::add_const<T>::type;
    template <class T> using add_volatile_t = typename std::add_volatile<T>::type;
    template <class T> using remove_reference_t = typename std::remove_reference<T>::type;
    template <class T> using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;
    template <class T> using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;
    template <class T> using remove_pointer_t = typename std::remove_pointer<T>::type;
    template <class T> using add_pointer_t = typename std::add_pointer<T>::type;
    template <class T> using make_signed_t = typename std::make_signed<T>::type;
    template <class T> using make_unsigned_t = typename std::make_unsigned<T>::type;
    template <class T> using remove_extent_t = typename std::remove_extent<T>::type;
    template <class T> using remove_all_extents_t = typename std::remove_all_extents<T>::type;
    template <std::size_t N, std::size_t A=N> using aligned_storage_t = typename std::aligned_storage<N,A>::type;
    template <std::size_t N, class... Ts> using aligned_union_t = typename std::aligned_union<N,Ts...>::type;
    template <class T> using decay_t = typename std::decay<T>::type;
    template <bool E, class T=void> using enable_if_t = typename std::enable_if<E,T>::type;
    template <bool B, class T, class F> using conditional_t = typename std::conditional<B,T,F>::type;
    template <class... Ts> using common_type_t = typename std::common_type<Ts...>::type;
    template <class T> using underlying_type_t = typename std::underlying_type<T>::type;
    template <class T> using result_of_t = typename std::result_of<T>::type;

    template<class...> struct voider { using type = void; };
    template<class...Ts> using void_t = typename voider<Ts...>::type;

    struct in_place_t {
        explicit in_place_t() = default;
    };
    static constexpr in_place_t in_place{};

  // [optional.optional], class template optional
  template <class T>
  class optional;

    namespace detail {
        template <class T, class U>
        using enable_forward_value = tl::enable_if_t<
            std::is_constructible<T, U&&>::value &&
            !std::is_same<tl::decay_t<U>, in_place_t>::value &&
            !std::is_same<optional<T>, tl::decay_t<U>>::value
        >;

        template <class T, class U, class Other>
        using enable_from_other = tl::enable_if_t<
            std::is_constructible<T, Other>::value &&
            !std::is_constructible<T, optional<U>&>::value &&
            !std::is_constructible<T, optional<U>&&>::value &&
            !std::is_constructible<T, const optional<U>&>::value &&
            !std::is_constructible<T, const optional<U>&&>::value &&
            !std::is_convertible<optional<U>&, T>::value &&
            !std::is_convertible<optional<U>&&, T>::value &&
            !std::is_convertible<const optional<U>&, T>::value &&
            !std::is_convertible<const optional<U>&&, T>::value
        >;
    }

    //TODO improve
    template <class T, class=void>
    struct is_swappable : std::false_type{};

    template <class T>
    struct is_swappable<T,void_t<decltype(swap(std::declval<T>(), std::declval<T>()))>>
        : std::true_type{};

    //TODO improve
    template <class T, class=void>
    struct is_nothrow_swappable : std::false_type{};

    template <class T>
    struct is_nothrow_swappable<T,void_t<decltype(swap(std::declval<T>(), std::declval<T>()))>>
        : std::true_type{};

  // [optional.nullopt], no-value state indicator
  struct nullopt_t{
      struct do_not_use{};
      constexpr explicit nullopt_t(do_not_use, do_not_use) noexcept{}
  };
    static constexpr nullopt_t nullopt{nullopt_t::do_not_use{}, nullopt_t::do_not_use{}};

  // [optional.bad.access], class bad_optional_access
    class bad_optional_access : public std::exception {
    public:
        bad_optional_access() = default;
        const char* what() const noexcept {
            return "Optional has no value";
        }
    };

  // [optional.relops], relational operators
  template <class T, class U>
  inline constexpr bool operator==(const optional<T>& lhs, const optional<U>& rhs) {
      if (lhs.has_value() != rhs.has_value())
          return false;
      if (lhs.has_value())
          return true;

      return lhs.value() == rhs.value();
  }
  template <class T, class U>
  inline constexpr bool operator!=(const optional<T>& lhs, const optional<U>& rhs) {
      if (lhs.has_value() != rhs.has_value())
          return true;
      if (lhs.has_value())
          return false;

      return lhs.value() != rhs.value();
  }
  template <class T, class U>
  inline constexpr bool operator<(const optional<T>& lhs, const optional<U>& rhs) {
      if (!rhs.has_value())
          return false;
      if (!lhs.has_value())
          return true;

      return lhs.value() < rhs.value();
  }
  template <class T, class U>
  inline constexpr bool operator>(const optional<T>& lhs, const optional<U>& rhs) {
      if (!lhs.has_value())
          return false;
      if (!rhs.has_value())
          return true;

      return lhs.value() > rhs.value();
  }
  template <class T, class U>
  inline constexpr bool operator<=(const optional<T>& lhs, const optional<U>& rhs) {
      if (!lhs.has_value())
          return true;
      if (!rhs.has_value())
          return false;

      return lhs.value() <= rhs.value();
  }
  template <class T, class U>
  inline constexpr bool operator>=(const optional<T>& lhs, const optional<U>& rhs) {
      if (!rhs.has_value())
          return true;
      if (!lhs.has_value())
          return false;

      return lhs.value() >= rhs.value();
  }

  // [optional.nullops], comparison with nullopt
template <class T> inline constexpr bool operator==(const optional<T>& lhs, nullopt_t) noexcept {
    return !lhs.has_value();
}
template <class T> inline constexpr bool operator==(nullopt_t, const optional<T>& rhs) noexcept {
    return !rhs.has_value();
}
template <class T> inline constexpr bool operator!=(const optional<T>& lhs, nullopt_t) noexcept {
    return lhs.has_value();
}
template <class T> inline constexpr bool operator!=(nullopt_t, const optional<T>& rhs) noexcept {
    return rhs.has_value();
}
template <class T> inline constexpr bool operator<(const optional<T>&, nullopt_t) noexcept {
    return false;
}
template <class T> inline constexpr bool operator<(nullopt_t, const optional<T>& rhs) noexcept {
    return rhs.has_value();
}
template <class T> inline constexpr bool operator<=(const optional<T>& lhs, nullopt_t) noexcept {
    return !lhs.has_value();
}
template <class T> inline constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept {
    return true;
}
template <class T> inline constexpr bool operator>(const optional<T>& lhs, nullopt_t) noexcept {
    return lhs.has_value();
}
template <class T> inline constexpr bool operator>(nullopt_t, const optional<T>&) noexcept {
    return false;
}
template <class T> inline constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept {
    return true;
}
template <class T> inline constexpr bool operator>=(nullopt_t, const optional<T>& rhs) noexcept {
    return !rhs.has_value();
}

  // [optional.comp_with_t], comparison with T
template <class T, class U> inline constexpr bool operator==(const optional<T>& lhs, const U& rhs) {
    return lhs.has_value() ? *lhs == rhs : false;
}
template <class T, class U> inline constexpr bool operator==(const U& lhs, const optional<T>& rhs) {
    return rhs.has_value() ? lhs == rhs.value() : false;
}
template <class T, class U> inline constexpr bool operator!=(const optional<T>& lhs, const U& rhs) {
    return lhs.has_value() ? lhs.value() != lhs : true;
}
template <class T, class U> inline constexpr bool operator!=(const U& lhs, const optional<T>& rhs) {
    return rhs.has_value() ? lhs != rhs.value() : true;
}
template <class T, class U> inline constexpr bool operator<(const optional<T>& lhs, const U& rhs) {
    return lhs.has_value() ? lhs.value() < lhs : true;
}
template <class T, class U> inline constexpr bool operator<(const U& lhs, const optional<T>& rhs) {
    return rhs.has_value() ? lhs < rhs.value() : false;
}
template <class T, class U> inline constexpr bool operator<=(const optional<T>& lhs, const U& rhs) {
    return lhs.has_value() ? lhs.value() <= lhs : true;
}
template <class T, class U> inline constexpr bool operator<=(const U& lhs, const optional<T>& rhs) {
    return rhs.has_value() ? lhs <= rhs.value() : false;
}
template <class T, class U> inline constexpr bool operator>(const optional<T>& lhs, const U& rhs) {
    return lhs.has_value() ? lhs.value() > lhs : false;
}
template <class T, class U> inline constexpr bool operator>(const U& lhs, const optional<T>& rhs) {
    return rhs.has_value() ? lhs > rhs.value() : true;
}
template <class T, class U> inline constexpr bool operator>=(const optional<T>& lhs, const U& rhs) {
    return lhs.has_value() ? lhs.value() >= lhs : false;
}
template <class T, class U> inline constexpr bool operator>=(const U& lhs, const optional<T>& rhs) {
    return rhs.has_value() ? lhs >= rhs.value() : true;
}


  // [optional.specalg], specialized algorithms
template <class T, tl::enable_if_t<std::is_move_constructible<T>::value>* = nullptr,
                   tl::enable_if_t<is_swappable<T>::value>* = nullptr>
void swap(optional<T>& lhs, optional<T>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    return lhs.swap(rhs);
}

  template <class T>
  inline constexpr optional<tl::decay_t<T>> make_optional(T&& v) {
      return optional<decay_t<T>>(std::forward<T>(v));
  }
  template <class T, class... Args>
  inline constexpr optional<T> make_optional(Args&&... args) {
      return optional<T>(in_place, std::forward<Args>(args)...);
  }
  template <class T, class U, class... Args>
  inline constexpr optional<T> make_optional(std::initializer_list<U> il, Args&&... args) {
      return optional<T>(in_place, il, std::forward<Args>(args)...);
  }
}

  // [optional.hash], hash support
namespace std {
    template <class T> struct hash;
    //TODO SFINAE
    template <class T>
    struct hash<tl::optional<T>> {
        std::size_t operator() (const tl::optional<T>& o) {
            if (!o.has_value())
                return 0;

            return hash<tl::remove_const_t<T>>()(o.value());
        }
    };
}

namespace tl {

    namespace detail {
        template <class T, bool = std::is_trivially_destructible<T>::value>
        struct optional_storage_base {
            ~optional_storage_base() {
                if (m_has_value) {
                    m_value.~T();
                    m_has_value = false;
                }
            }

            struct dummy{};
            union {
                dummy m_dummy;
                T m_value;
            };

            bool m_has_value = false;
        };

        template <class T>
        struct optional_storage_base<T, true> {
            ~optional_storage_base() {
                if (m_has_value) {
                    //don't destruct value
                    m_has_value = false;
                }
            }

            struct dummy{};
            union {
                dummy m_dummy;
                T m_value;
            };

            bool m_has_value = false;
        };
    }

template <class T>
class optional : private detail::optional_storage_base<T> {
  public:
    using value_type = T;

    // [optional.ctor], constructors
    constexpr optional() noexcept = default;
    constexpr optional(nullopt_t) noexcept {};
    constexpr optional(const optional& rhs) {
        if (rhs.has_value()) {
            this->m_has_value = true;
            new (std::addressof(this->m_value)) T (rhs.value());
        }
    }
            template <class U = T, tl::enable_if_t<std::is_move_constructible<T>::value>* = nullptr>
            constexpr optional(optional&& rhs) {
                if (rhs.has_value()) {
                    this->m_has_value = true;
                    new (std::addressof(this->m_value)) T (std::move(rhs.value()));
                }
            }
            template <class... Args>
            constexpr explicit optional(tl::enable_if_t<std::is_constructible<T, Args...>::value, in_place_t>,
                                        Args&&... args) {
                this->m_has_value = true;
                new (std::addressof(this->m_value)) T (std::forward<Args>(args)...);
            }
            template <class U, class... Args>
            constexpr explicit optional(
                tl::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value, in_place_t>,
                std::initializer_list<U> il, Args&&... args) {
                this->m_has_value = true;
                new (std::addressof(this->m_value)) T (il, std::forward<Args>(args)...);
            }

            template <class U = T, tl::enable_if_t<std::is_convertible<U&&, T>::value>* = nullptr,
                      detail::enable_forward_value<T,U>* = nullptr>
                constexpr optional(U&& u) {
                this->m_has_value = true;
                new (std::addressof(this->m_value)) T (std::forward<U>(u));
            }

            template <class U = T, tl::enable_if_t<!std::is_convertible<U&&, T>::value>* = nullptr,
                      detail::enable_forward_value<T,U>* = nullptr>
                constexpr explicit optional(U&& u) {
                this->m_has_value = true;
                new (std::addressof(this->m_value)) T (std::forward<U>(u));
            }

            template <class U, detail::enable_from_other<T,U,const U&>* = nullptr,
                      tl::enable_if_t<std::is_convertible<const U&, T>::value>* = nullptr>
            optional(const optional<U>& rhs) {
                this->m_has_value = true;
                new (std::addressof(this->m_value)) T (rhs.value());
            }

    template <class U, detail::enable_from_other<T,U,const U&>* = nullptr,
                      tl::enable_if_t<!std::is_convertible<const U&, T>::value>* = nullptr>
    optional(const optional<U>& rhs) {
        this->m_has_value = true;
                new (std::addressof(this->m_value)) T (rhs.value());
            }

          template <class U, detail::enable_from_other<T,U,U&&>* = nullptr,
                      tl::enable_if_t<std::is_convertible<U&&, T>::value>* = nullptr>
        optional(optional<U>&& rhs) {
        this->m_has_value = true;
                new (std::addressof(this->m_value)) T (std::move(rhs.value()));
            }


    template <class U, detail::enable_from_other<T,U,U&&>* = nullptr,
                      enable_if_t<!std::is_convertible<U&&, T>::value>* = nullptr>
              explicit optional(optional<U>&& rhs) {
        this->m_has_value = true;
        new (std::addressof(this->m_value)) T (std::move(rhs.value()));
            }

    // [optional.dtor], destructor
    ~optional() = default;

    // [optional.assign], assignment
    optional& operator=(nullopt_t) noexcept;
    optional& operator=(const optional&);
    optional& operator=(optional&&) noexcept;
    template <class U = T> optional& operator=(U&&);
    template <class U> optional& operator=(const optional<U>&);
    template <class U> optional& operator=(optional<U>&&);

    template <class... Args> T& emplace(Args&&... args) {
        static_assert(std::is_constructible<T, Args&&...>::value,
                      "T must be constructible with Args");

        *this = nullopt;
        new (std::addressof(this->m_value)) T(std::forward<Args>(args)...);
    }

    template <class U, class... Args>
    tl::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value, T&>
    emplace(std::initializer_list<U> il, Args&&... args) {
        *this = nullopt;
        new (std::addressof(this->m_value)) T(il, std::forward<Args>(args)...);
    }

    // [optional.swap], swap
    void swap(optional& rhs)
        noexcept(std::is_nothrow_move_constructible<T>::value && is_nothrow_swappable<T>::value)
    {
        if (has_value()) {
            if (rhs.has_value()) {
                using std::swap;
                swap(value(), rhs.value());
            }
            else {
                new (&rhs.m_value) T (std::move(this->m_value));
                this->m_value.T::~T();
            }
        }
        else if (rhs.has_value()) {
            new (std::addressof(this->m_value)) T (std::move(rhs.m_value));
            rhs.m_value.T::~T();
        }
    }

    // [optional.observe], observers
    constexpr const T* operator->() const {
        return std::addressof(this->m_value);
    }
    constexpr T* operator->() {
        return std::addressof(this->m_value);
    }
    constexpr const T& operator*() const& {
        return this->m_value;
    }
    constexpr T& operator*() & {
        return this->m_value;
    }
    constexpr T&& operator*() && {
        return std::move(this->m_value);
    }
    constexpr const T&& operator*() const&& {
        return std::move(this->m_value);
    }
    constexpr explicit operator bool() const noexcept {
        return this->m_has_value;
    }
    constexpr bool has_value() const noexcept {
        return this->m_has_value;
    }
    constexpr const T& value() const& {
        return has_value() ? this->m_value : throw bad_optional_access();
    }
    constexpr T& value() & {
        return has_value() ? this->m_value : throw bad_optional_access();
    }
    constexpr T&& value() && {
        return has_value() ? std::move(this->m_value) : throw bad_optional_access();
    }
    constexpr const T&& value() const&& {
        return has_value() ? std::move(this->m_value) : throw bad_optional_access();
    }
    template <class U> constexpr T value_or(U&& u) const& {
        static_assert(std::is_copy_constructible<T>::value && std::is_convertible<U&&, T>::value,
                      "T must be copy constructible and convertible from U");
        return has_value() ? value() : static_cast<T>(std::forward<U>(u));
    }
    template <class U> constexpr T value_or(U&& u) && {
        static_assert(std::is_move_constructible<T>::value && std::is_convertible<U&&, T>::value,
                      "T must be move constructible and convertible from U");
        return has_value() ? value() : static_cast<T>(std::forward<U>(u));
    }

    // [optional.mod], modifiers
    void reset() noexcept;

  private:

  };
}
