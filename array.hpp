#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace util {

template <class Gen, class T>
concept TGenerator = requires (Gen gen) {
  {
    gen(size_t(0))
  } -> std::convertible_to<T>;
};

template <class T, size_t N, TGenerator<T> Gen>
[[gnu::always_inline]]
inline constexpr std::array<T, N> array_generate(Gen&& gen) {
  constexpr auto generate =
    []<size_t... Is> [[gnu::always_inline]]
    (std::index_sequence<Is...>, auto&& gen) {
      return std::array<T, sizeof...(Is)>{ std::forward<decltype(gen)>(gen)(Is)... };
    };
  return generate(std::make_index_sequence<N>{}, std::forward<Gen>(gen));
}

namespace generators {

template <class T, size_t... Ns>
[[gnu::always_inline]]
inline constexpr auto concat(std::array<T, Ns> const&... as) {
  return [&](size_t idx) -> T {
    constexpr auto get =
      [&]<size_t M, size_t... Ms>(
        auto&& call, size_t idx, std::array<T, M> const& a0, std::array<T, Ms> const&... as) {
        if (idx >= M) {
          if constexpr (sizeof...(as) == 0)
            throw std::out_of_range("idx out of range");
          else
            return call(std::forward<decltype(call)>(call), idx - M, as...);
        } else
          return a0[idx];
      };
    return get(get, idx, as...);
  };
}

} // namespace generators

} // namespace util
