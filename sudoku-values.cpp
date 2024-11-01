#include "array.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>
#include <ranges>
#include <stack>
#include <vector>

namespace sudoku {

using bitnum_t = uint8_t;

struct digit {
  bitnum_t bit = 0;

  constexpr explicit digit(bitnum_t bit)
    : bit(bit) {}

  static constexpr char
  verify(char d) {
    if (!valid(d))
      throw std::runtime_error("invalid char for digit");
    return d;
  }

  static constexpr bitnum_t
  verify(bitnum_t bit) {
    if (!valid(bit))
      throw std::runtime_error("invalid int for digit");
    return bit;
  }

  constexpr
  operator bitnum_t () const {
    return bit;
  }

  constexpr char
  to_char() {
    return bit + '0';
  }

  static constexpr digit
  from(char d) {
    return digit{ bitnum_t(verify(d) - '0') };
  }

  static constexpr digit
  from(bitnum_t bit) {
    return digit{ bit };
  }

  static constexpr bool
  valid(char d) {
    return d >= '0' && d <= '9';
  }

  static constexpr bool
  valid(bitnum_t d) {
    return d >= 0 && d <= 9;
  }
};

struct set {
  using bits_t = uint16_t;

  static constexpr uint16_t mask = 0x3ff;

  bits_t bits{};

  constexpr bool
  test(bitnum_t bit) const {
    return (bits & (1 << bit)) != 0;
  }

  static constexpr set
  from(bitnum_t bit) {
    return set{ .bits = bits_t(1 << bit) };
  }

  static constexpr set
  from_not(bitnum_t bit) {
    bits_t value = ~(1 << bit) & mask;
    return set{ .bits = value };
  }

  static constexpr set
  from(size_t num, std::predicate<bitnum_t> auto pred);

  static constexpr set
  from(std::predicate<bitnum_t> auto pred);

  constexpr set
  operator~() const {
    bits_t value = ~bits & mask;
    return set{ .bits = value };
  }

  constexpr bitnum_t
  front() const {
    return std::countr_zero(bits);
  }

  constexpr bitnum_t
  back() const {
    return std::bit_width(bits) - 1;
  }

  constexpr size_t
  size() const {
    return std::popcount(bits);
  }

  auto
  operator<=> (set const&) const = default;

  struct iter_sent {};

  struct iter {
    using value_type      = bitnum_t;
    using difference_type = int8_t;

    static constexpr bitnum_t
    find(set::bits_t digits, bitnum_t bit = 0) {
      return bit + std::countr_zero(bits_t(digits >> bit));
    }

    set::bits_t digits;
    bitnum_t    bit = find(digits);

    constexpr value_type
    operator* () const {
      return bit;
    }

    constexpr iter&
    operator++ () {
      bit = find(digits, ++bit);
      return *this;
    }

    constexpr iter
    operator++ (int) {
      iter tmp{ .digits = digits, .bit = bit };
      ++(*this);
      return tmp;
    }
  };

  constexpr iter
  begin() const {
    return iter{ .digits = bits };
  }

  constexpr iter_sent
  end() const {
    return set::iter_sent{};
  }
};

constexpr set
operator| (set const& a, set const& b) {
  return set{ .bits = set::bits_t(a.bits | b.bits) };
}

constexpr set
operator& (set const& a, set const& b) {
  return set{ .bits = set::bits_t(a.bits & b.bits) };
}

constexpr bool
operator== (set::iter const& a, set::iter const& b) {
  return a.digits == b.digits && a.bit == b.bit;
}

constexpr bool
operator!= (set::iter const& a, set::iter const& b) {
  return a.digits != b.digits || a.bit != b.bit;
}

// clang-format off
static_assert( set::from    (digit::from('0')).back()  == digit::from('0'));
static_assert( set::from    (digit::from('1')).back()  == digit::from('1'));
static_assert( set::from    (digit::from('9')).back()  == digit::from('9'));
static_assert( set::from    (digit::from('0')).front() == digit::from('0'));
static_assert( set::from    (digit::from('1')).front() == digit::from('1'));
static_assert( set::from    (digit::from('9')).front() == digit::from('9'));

static_assert( set::from    (digit::from('0')).test(digit::from('0')));
static_assert( set::from    (digit::from('1')).test(digit::from('1')));
static_assert( set::from    (digit::from('2')).test(digit::from('2')));
static_assert( set::from    (digit::from('3')).test(digit::from('3')));
static_assert( set::from    (digit::from('4')).test(digit::from('4')));
static_assert( set::from    (digit::from('5')).test(digit::from('5')));
static_assert( set::from    (digit::from('6')).test(digit::from('6')));
static_assert( set::from    (digit::from('7')).test(digit::from('7')));
static_assert( set::from    (digit::from('8')).test(digit::from('8')));
static_assert( set::from    (digit::from('9')).test(digit::from('9')));

static_assert(!set::from_not(digit::from('0')).test(digit::from('0')));
static_assert( set::from_not(digit::from('0')).test(digit::from('1')));
static_assert( set::from_not(digit::from('0')).test(digit::from('9')));
static_assert(!set::from_not(digit::from('1')).test(digit::from('1')));
static_assert(!set::from_not(digit::from('2')).test(digit::from('2')));
static_assert(!set::from_not(digit::from('3')).test(digit::from('3')));
static_assert(!set::from_not(digit::from('4')).test(digit::from('4')));
static_assert(!set::from_not(digit::from('5')).test(digit::from('5')));
static_assert(!set::from_not(digit::from('6')).test(digit::from('6')));
static_assert(!set::from_not(digit::from('7')).test(digit::from('7')));
static_assert(!set::from_not(digit::from('8')).test(digit::from('8')));
static_assert(!set::from_not(digit::from('9')).test(digit::from('9')));

static_assert( set::from_not(digit::from('0')).bits == 0b1111111110);
static_assert( set::from_not(digit::from('4')).bits == 0b1111101111);
static_assert( set::from_not(digit::from('9')).bits == 0b0111111111);

static_assert( (  set::from(digit::from('0'))
                | set::from(digit::from('1'))
               )
               .test(digit::from('0')));

static_assert( (  set::from(digit::from('0'))
                | set::from(digit::from('1'))
               ).test(digit::from('1')));

static_assert(!(  set::from(digit::from('0'))
                & set::from(digit::from('1'))
               ).test(digit::from('0')));

static_assert(!(  set::from(digit::from('0'))
                & set::from(digit::from('1'))
               ).test(digit::from('1')));

static_assert((~set{}).bits == set::mask);

static_assert(std::forward_iterator<set::iter>);

static_assert(
  (  set::iter {
    .digits = (  set::from(digit::from('0'))
               | set::from(digit::from('9'))
              ).bits
  }).bit == 0);
static_assert(
  (++set::iter {
    .digits = (  set::from(digit::from('0'))
               | set::from(digit::from('9'))
              ).bits
  }).bit >= 9);
// clang-format on

bool
operator== (set::iter const iter, set::iter_sent const sent) {
  return iter.bit > 9;
}

static_assert(std::sentinel_for<set::iter_sent, set::iter>);

static_assert(std::ranges::range<set>);

constexpr set
set::from(size_t num, std::predicate<bitnum_t> auto pred) {
  set value;
  if (num > (~set()).size())
    throw std::overflow_error("num is too big");
  for (bitnum_t bit = 0; bit < num; ++bit)
    value = value | (pred(bit) ? set::from(bit) : set{});
  return value;
}

constexpr set
set::from(std::predicate<bitnum_t> auto pred) {
  set value;
  for (bitnum_t const& bit: ~set{})
    value = value | (pred(bit) ? set::from(bit) : set{});
  return value;
}

static constexpr set cell_fixed_mask = set::from(0);
static constexpr set cell_initial    = ~set{};

using coord_t = uint8_t;

struct raw_index;

struct index {
  coord_t x: 4;
  coord_t y: 4;

  constexpr bool
  valid() const {
    // clang-format off
    return (x >= 0 && x <= 8)
        && (y >= 0 && y <= 8);
    // clang-format on
  }

  static constexpr index
  validate(index idx) {
    if (!idx.valid())
      throw std::domain_error("invalid idx");
    return idx;
  }

  static constexpr index
  from(raw_index idx);
};

// clang-format off
constexpr index
operator+ (index a, index b) {
  return {
    .x = coord_t(a.x + b.x),
    .y = coord_t(a.y + b.y)
  };
}

constexpr index
operator- (index a, index b) {
  return {
    .x = coord_t(a.x - b.x),
    .y = coord_t(a.y - b.y)
  };
}

// clang-format on

struct raw_index {
  coord_t idx;

  constexpr bool
  valid() const {
    return idx < 81;
  }

  static constexpr raw_index
  validate(raw_index idx) {
    if (!idx.valid())
      throw std::domain_error("invalid raw idx");
    return idx;
  }

  static constexpr raw_index
  from(index idx) {
    constexpr auto to_raw = [](index idx) -> raw_index {
      return { .idx = coord_t(idx.x + (9 * idx.y)) };
    };
    return to_raw(index::validate(idx));
  }

  auto
  operator<=> (raw_index const&) const = default;
};

constexpr index
index::from(raw_index idx) {
  constexpr auto to_idx = [](raw_index idx) -> index {
    // clang-format off
    return {
      .x = coord_t(idx.idx % 9),
      .y = coord_t(idx.idx / 9)
    };
    // clang-format on
  };
  return to_idx(raw_index::validate(idx));
}

constexpr inline auto fixed = [](set cell) -> bool {
  return !cell.test(0); // does not contain 0
};

// clang-format off
static_assert(!fixed(set::from(0) | set::from(1)));
static_assert( fixed(set::from(1)               ));
// clang-format on

constexpr inline auto possibilities = [](set cell) -> size_t {
  if (fixed(cell))
    return 0;
  return (cell & ~cell_fixed_mask).size();
};

constexpr inline auto singleton = [](set cell) -> bool {
  return possibilities(cell) == 1;
};

// clang-format off
static_assert(!singleton(set::from(1)               ));
static_assert(!singleton(set::from(2)               ));
static_assert(!singleton(set::from(3)               ));
static_assert(!singleton(set::from(4)               ));
static_assert(!singleton(set::from(5)               ));
static_assert(!singleton(set::from(6)               ));
static_assert(!singleton(set::from(7)               ));
static_assert(!singleton(set::from(8)               ));
static_assert(!singleton(set::from(9)               ));
static_assert( singleton(set::from(1) | set::from(0)));
static_assert( singleton(set::from(2) | set::from(0)));
static_assert( singleton(set::from(3) | set::from(0)));
static_assert( singleton(set::from(4) | set::from(0)));
static_assert( singleton(set::from(5) | set::from(0)));
static_assert( singleton(set::from(6) | set::from(0)));
static_assert( singleton(set::from(7) | set::from(0)));
static_assert( singleton(set::from(8) | set::from(0)));
static_assert( singleton(set::from(9) | set::from(0)));

static_assert(possibilities(set::from(1) | set::from(0)) == 1);
static_assert(possibilities(set::from(2) | set::from(0)) == 1);
static_assert(possibilities(set::from(3) | set::from(0)) == 1);
static_assert(possibilities(set::from(4) | set::from(0)) == 1);
static_assert(possibilities(set::from(5) | set::from(0)) == 1);
static_assert(possibilities(set::from(6) | set::from(0)) == 1);
static_assert(possibilities(set::from(7) | set::from(0)) == 1);
static_assert(possibilities(set::from(8) | set::from(0)) == 1);
static_assert(possibilities(set::from(9) | set::from(0)) == 1);

constexpr index
row_starter(index idx) {
  return {
    .x = 0,
    .y = idx.y
  };
}

constexpr coord_t
row_idx(index idx) {
  return idx.y;
}

constexpr index
column_starter(index idx) {
  return {
    .x = idx.x,
    .y = 0
  };
}

constexpr coord_t
column_idx(index idx) {
  return idx.x;
}

constexpr index
square_starter(index idx) {
  return {
    .x = coord_t(3 * (idx.x / 3)),
    .y = coord_t(3 * (idx.y / 3))
  };
}

constexpr coord_t
square_idx(index idx) {
  auto starter = square_starter(idx);
  return   (starter.x / 3)
    + (3 * (starter.y / 3));
}


using indices_t = std::array<raw_index, 9>;

constexpr inline auto rows = util::array_generate<indices_t, 9>([](size_t row) {
  return util::array_generate<raw_index, 9>([row](size_t pos) {
    // clang-format off
    return raw_index::from(index{
      .x = coord_t(pos),
      .y = coord_t(row)
    });
    // clang-format on
  });
});

// clang-format off
static_assert(
  rows[0] == std::array<raw_index, 9> {
    raw_index{ 0 },
    raw_index{ 1 },
    raw_index{ 2 },
    raw_index{ 3 },
    raw_index{ 4 },
    raw_index{ 5 },
    raw_index{ 6 },
    raw_index{ 7 },
    raw_index{ 8 }
  }
);
// clang-format on

constexpr inline auto columns =
  util::array_generate<indices_t, 9>([](size_t column) {
    return util::array_generate<raw_index, 9>([column](size_t pos) {
      // clang-format off
      return raw_index::from(index{
        .x = coord_t(column),
        .y = coord_t(pos)
      });
      // clang-format on
    });
  });

constexpr std::array<index, 9> square_starters{
  index{ .x = 0, .y = 0 },
  index{ .x = 3, .y = 0 },
  index{ .x = 6, .y = 0 },
  index{ .x = 0, .y = 3 },
  index{ .x = 3, .y = 3 },
  index{ .x = 6, .y = 3 },
  index{ .x = 0, .y = 6 },
  index{ .x = 3, .y = 6 },
  index{ .x = 6, .y = 6 }
};

constexpr inline auto squares =
  util::array_generate<indices_t, 9>([](size_t square) {
    auto const starter = square_starters[square];
    return util::array_generate<raw_index, 9>([starter](size_t pos) {
      coord_t x = pos % 3;
      coord_t y = pos / 3;
      return raw_index::from(starter + index{ .x = x, .y = y });
    });
  });

// clang-format off
static_assert(
  squares[0] == std::array<raw_index, 9> {
    raw_index{ .idx =  0 },
    raw_index{ .idx =  1 },
    raw_index{ .idx =  2 },
    raw_index{ .idx =  9 },
    raw_index{ .idx = 10 },
    raw_index{ .idx = 11 },
    raw_index{ .idx = 18 },
    raw_index{ .idx = 19 },
    raw_index{ .idx = 20 }
  }
);

static_assert(
  squares[3] == std::array<raw_index, 9> {
    raw_index{ .idx = 27 },
    raw_index{ .idx = 28 },
    raw_index{ .idx = 29 },
    raw_index{ .idx = 36 },
    raw_index{ .idx = 37 },
    raw_index{ .idx = 38 },
    raw_index{ .idx = 45 },
    raw_index{ .idx = 46 },
    raw_index{ .idx = 47 }
  }
);

static_assert(
  squares[8] == std::array<raw_index, 9> {
    raw_index{ .idx = 60 },
    raw_index{ .idx = 61 },
    raw_index{ .idx = 62 },
    raw_index{ .idx = 69 },
    raw_index{ .idx = 70 },
    raw_index{ .idx = 71 },
    raw_index{ .idx = 78 },
    raw_index{ .idx = 79 },
    raw_index{ .idx = 80 }
  }
);
// clang-format on

constexpr inline auto indices_sets     = { rows, squares, columns };
constexpr inline auto all_indices_sets = std::views::join(indices_sets);

using constraints_indices_t     = std::array<index, 21>;
using constraints_raw_indices_t = std::array<raw_index, 21>;

consteval constraints_raw_indices_t
gen_constraints_indices(raw_index idx) {
  // clang-format off
  auto const row = rows   [   row_idx(index::from(idx))];
  auto const col = columns[column_idx(index::from(idx))];
  auto const sqr = squares[square_idx(index::from(idx))];
  // clang-format on

  auto all = util::array_generate<raw_index, 27>(
    util::generators::concat(row, col, sqr));

  std::ranges::sort(all);

  auto [all_end, end] = std::ranges::unique(all);

  assert(std::distance(begin(all), all_end) == 21);

  return util::array_generate<raw_index, 21>([&](size_t i) {
    return all[i];
  });
}

constexpr inline auto raw_indices =
  util::array_generate<raw_index, 81>([](size_t i) {
    return raw_index{ .idx = coord_t(i) };
  });

constinit inline auto constraints_per_raw_index =
  util::array_generate<constraints_raw_indices_t, 81>([](size_t i) {
    return gen_constraints_indices(raw_indices[i]);
  });

struct map {
  std::array<set, 81> cells = util::array_generate<set, 81>([](size_t) {
    return cell_initial;
  });

  constexpr set&
  operator[] (raw_index const& idx) {
    return cells[idx.idx];
  }

  constexpr set const&
  operator[] (raw_index const& idx) const {
    return cells[idx.idx];
  }
};

map
operator& (map const& a, map const& b) {
  // clang-format off
  return map{
    .cells = util::array_generate<set, 81>(
      [&](size_t i) { return a.cells[i] & b.cells[i]; })
  };
  // clang-format on
}

static_assert(map{}.cells[1] == cell_initial);

bool
valid(set const cell) {
  return (cell & ~cell_fixed_mask).bits != 0;
}

bool
valid(map const& s) {
  auto valid_cell = [](set const cell) {
    return valid(cell);
  };
  return std::ranges::all_of(s.cells, valid_cell);
}

struct actions {
  struct fix {
    raw_index idx;
    digit     dig;
  };

  struct disable {
    raw_index idx;
    digit     dig;
  };

  struct put {
    raw_index idx;
    digit     dig;
  };

  map    actions_map;
  size_t count = 0;

  constexpr actions&
  expand(actions::fix act) {
    actions_map[act.idx] = set::from(act.dig);
    ++count;
    return *this;
  }

  constexpr actions&
  expand(actions::disable act) {
    actions_map[act.idx] = actions_map[act.idx] & set::from_not(act.dig);
    ++count;
    return *this;
  }

  constexpr actions&
  expand(actions::put act) {
    // clang-format off
    for (raw_index const& idx: constraints_per_raw_index[act.idx.idx]) {
      expand(actions::disable{
        .idx = idx,
        .dig = act.dig
      });
    }
    return expand(actions::fix{
      .idx = act.idx,
      .dig = act.dig
    });
    // clang-format on
  }

  constexpr auto
  on(raw_index idx) {
    struct act {
      actions*  acts;
      raw_index idx;

      constexpr actions&
      put(digit d) {
        // clang-format off
        return acts->expand(actions::put{
          .idx = idx,
          .dig = d
        });
        // clang-format on
      }

      constexpr actions&
      disable(digit d) {
        // clang-format off
        return acts->expand(actions::disable{
          .idx = idx,
          .dig = d
        });
        // clang-format on
      }

      constexpr actions&
      fix(digit d) {
        // clang-format off
        return acts->expand(actions::fix{
          .idx = idx,
          .dig = d
        });
        // clang-format on
      }
    };

    return act{ .acts = this, .idx = idx };
  }

  constexpr actions const&
  apply(map& s) const {
    s = s & actions_map;
    return *this;
  }

  operator size_t () const {
    return count;
  }
};

constexpr inline auto obvious_singletons = [](map const& s) -> actions {
  actions acts;
  for (raw_index const& idx: raw_indices) {
    set const& cell = s[idx];
    if (!singleton(cell))
      continue;
    acts.on(idx).put(digit{ cell.back() });
    // return acts;
  }
  return acts;
};

constexpr inline auto hidden_singletons = [](map const& s) -> actions {
  actions acts;
  for (indices_t const& is: all_indices_sets) {
    std::array<raw_index, 10> digit_idx;
    std::array<uint8_t, 10>   counts{};

    for (raw_index const& idx: is) {
      if (fixed(s[idx]))
        continue;
      set const cell = s[idx] & ~cell_fixed_mask;
      for (bitnum_t const& bit: cell) {
        ++counts[bit];
        digit_idx[bit] = idx;
      }
    }

    bool found = false;
    for (bitnum_t const& bit: std::views::iota(1, 10)) {
      if (counts[bit] != 1)
        continue;
      found = true;
      acts.on(digit_idx[bit]).put(digit{ bit });
    }
    if (found)
      return acts;
  };
  return acts;
};

using coord_pair_t = std::tuple<coord_t, coord_t>;

constexpr inline auto ascending_pairs =
  util::array_generate<coord_pair_t, 36>([](size_t i) {
    int p = 8;
    while (i >= p && p > 0) {
      i -= p;
      p -= 1;
    }
    return coord_pair_t(8 - p, i + 9 - p);
  });

// clang-format off
static_assert(ascending_pairs[ 0] == coord_pair_t(0, 1));
static_assert(ascending_pairs[ 1] == coord_pair_t(0, 2));
static_assert(ascending_pairs[ 7] == coord_pair_t(0, 8));
static_assert(ascending_pairs[ 8] == coord_pair_t(1, 2));
static_assert(ascending_pairs[ 9] == coord_pair_t(1, 3));
static_assert(ascending_pairs[14] == coord_pair_t(1, 8));
static_assert(ascending_pairs[15] == coord_pair_t(2, 3));
static_assert(ascending_pairs[20] == coord_pair_t(2, 8));
static_assert(ascending_pairs[21] == coord_pair_t(3, 4));
static_assert(ascending_pairs[25] == coord_pair_t(3, 8));
static_assert(ascending_pairs[26] == coord_pair_t(4, 5));
static_assert(ascending_pairs[29] == coord_pair_t(4, 8));
static_assert(ascending_pairs[30] == coord_pair_t(5, 6));
static_assert(ascending_pairs[32] == coord_pair_t(5, 8));
static_assert(ascending_pairs[33] == coord_pair_t(6, 7));
static_assert(ascending_pairs[34] == coord_pair_t(6, 8));
static_assert(ascending_pairs[35] == coord_pair_t(7, 8));
// clang-format on

constexpr auto
region(map const& s, indices_t const& is) {
  return util::array_generate<set, 9>([&](size_t i) {
    return s[is[i]];
  });
}

constexpr inline auto obvious_pairs = [](map const& s) -> actions {
  actions acts;

  for (indices_t const& is: all_indices_sets) {
    auto const& cells = region(s, is);

    set const cells_fixed = set::from(9, [&cells](bitnum_t i) {
      return fixed(cells[coord_t(i)]);
    });

    set const cells_2possibilities = set::from(9, [&cells](bitnum_t i) {
      return possibilities(cells[coord_t(i)]) == 2;
    });

    for (auto const& [left, right]: ascending_pairs) {
      if (!cells_2possibilities.test(left))
        continue;
      if (!cells_2possibilities.test(right))
        continue;
      if (cells[left] != cells[right])
        continue;
      // clang-format off
      set const pair_digits =  cells[left]
                            &  cells[right]
                            & ~cell_fixed_mask;
      // clang-format on
      assert(pair_digits.size() == 2);
      for (coord_t const& i: std::views::iota(0, 9)) {
        if (i == left)
          continue;
        if (i == right)
          continue;
        if (cells_fixed.test(i))
          continue;
        set const common = cells[i] & pair_digits;
        if (common.bits == 0)
          continue;
        if (common == pair_digits)
          continue;
        auto act = acts.on(is[i]);
        for (bitnum_t const& bit: pair_digits)
          act.disable(digit{ bit });
      }
      return acts;
    }
  }
  return acts;
};

constexpr bool
solved(map const& s) {
  return std::ranges::all_of(s.cells, fixed);
}

constexpr std::optional<raw_index>
split_candidate(map const& s) {
  for (uint8_t p: std::views::iota(2, 9)) {
    for (raw_index idx: raw_indices) {
      if (possibilities(s[idx]) != p)
        continue;
      return std::make_optional(idx);
    }
  }
  return std::nullopt;
}

inline bool
propagate(map& s) {
  int technique = 0;
  while (true) {
    if (!valid(s))
      return false;
    switch (technique) {
    case 0: {
      if (actions acts = obvious_singletons(s); acts != 0) {
        technique = 0;
        acts.apply(s);
      } else {
        ++technique;
      }
      break;
    }
    case 1: {
      if (actions acts = hidden_singletons(s); acts != 0) {
        technique = 0;
        acts.apply(s);
      } else {
        ++technique;
      }
      break;
    }
    case 2: {
      if (actions acts = obvious_pairs(s); acts != 0) {
        technique = 0;
        acts.apply(s);
      } else {
        ++technique;
      }
    }
    default:
      return true;
    }
  }

  return true;
};

constexpr actions&
put(size_t i, char d, actions& acts) {
  if (d != '0')
    acts.on(raw_indices[i]).put(digit::from(d));
  return acts;
}

constexpr void
dump(map const& s) {
  std::string chars;
  chars.reserve(81);
  for (auto const& cell: s.cells) {
    if (fixed(cell))
      chars.push_back(digit{ cell.back() }.to_char());
    else
      chars.push_back('0');
  }
  std::println("{}", chars);
}

} // namespace sudoku

int
main(int argc, char* argv[]) {
  if (argc != 2)
    throw std::runtime_error("file name argument required");

  std::ifstream file(argv[1], std::ios::binary | std::ios::in);

  while (true) {
    std::string line;
    line.reserve(100);

    std::getline(file, line);

    if (file.bad() || file.eof())
      break;

    std::vector<sudoku::map> stack_vec;
    stack_vec.reserve(32);
    std::stack<sudoku::map, std::vector<sudoku::map>> stack(stack_vec);

    {
      auto& current = stack.emplace();
      {
        sudoku::actions acts;
        for (int const& i: std::views::iota(0, 81))
          put(i, line[i], acts);
        acts.apply(current);
      }

      if (!propagate(current))
        throw std::runtime_error("cannot propagate sudoku");
    }

    while (true) {
      if (stack.empty())
        throw std::runtime_error("sudoku cannot be solved");

      auto current = std::move(stack.top());
      stack.pop();

      if (solved(current)) {
        dump(current);
        break;
      }

      {
        auto cand = split_candidate(current);
        if (!cand)
          throw std::runtime_error("cannot split sudoku");

        auto idx  = *cand;
        auto cell = current[idx];

        for (auto d: cell) {
          auto& next = stack.emplace(current);

          sudoku::actions{}.on(idx).put(sudoku::digit{ d }).apply(next);
          if (!propagate(next)) {
            stack.pop();
            continue;
          }
        }
      }
    }
  }
}
