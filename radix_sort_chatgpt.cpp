//
// radix_sort_chatgpt.cpp
//
// See https://en.wikipedia.org/wiki/Radix_sort
//
// This is a manual translation by Jay Krell (jay.krell@cornell.edu)
// of ChatGPT's Rust implementation of radix_sort into C++.
// Some minor optimizations are applied.
// ChatGPT required a small amount of prompting from Sean, but arrived
// at a convincing efficient working implementation rapidly.
//
// Neighboring radix_sort.cpp says more, including some comparison
// of ChatGPT's answer from mine. ChatGPT arrived at its answer
// much more rapidly than I did.
//
// https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&gist=1bec16cec99f75c3e5aacded43c18fad
//
// To test this code, see radix_sort.cpp and pass chatgpt on command line.
//
#include <array>
#include <ctype.h>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <time.h>

template <size_t Base, typename T1, typename T2>
static size_t get_digit(T1 value, T2 power)
{
    return (value / power) % Base;
}

template <size_t Base, typename Iterator, typename T>
static void
counting_sort(Iterator begin, Iterator end, size_t exp, T const & /* deduction helper */)
{
    if (begin == end)
        return;

    using array = std::array<size_t, Base>;
    array counts{};
    size_t i{};
    size_t size{};

    // Count them, by digit and overall.
    for (auto it{begin}; it != end; ++it)
    {
        counts[get_digit<Base>(*it, exp)] += 1;
        ++size;
    }

    if (size < 2)
        return;

    std::vector<T> temp(size);

    // Change counts to ending positions.
    for (i = 1; i < Base; ++i)
        counts[i] += counts[i - 1];

    // Place elements in array, going backwards,
    // because we have ending positions.
    for (i = 0; i < size; ++i)
    {
        auto & data = *--end;
        temp[counts[get_digit<Base>(data, exp)] -= 1] = data;
    }

    std::copy(temp.begin(), temp.end(), begin);
}

template <size_t Base, typename Iterator>
static void
radix_sort(Iterator begin, Iterator end)
{
    if (begin == end)
        return;

    // TODO: std::accumulate
    auto max = *begin;
    size_t size{};
    for (auto it{begin}; it != end; ++it)
    {
        max = std::max(max, *it);
        ++size;
    }

    if (size < 2)
        return;

    int64_t exp = 1;

    // ChatGPT said: while ((max / exp) > 0)
                     while (max >= exp) // :JayKrell changed to
    // These are the same, at least for positive numbers.
    // If we fix for negative numbers, revisit.
    // Notice that ChatGPT did not accept < 0, and it does not presently work.
    {
        counting_sort<Base>(begin, end, exp, *begin);
        exp *= Base;
    }
}
