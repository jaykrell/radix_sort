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
    // TODO: std::accumulate
    auto max = *begin;
    for (auto it{begin}; it != end; ++it)
        max = std::max(max, *it);

    size_t exp = 1;

    while ((max / exp) > 0)
    {
        counting_sort<Base>(begin, end, exp, *begin);
        exp *= Base;
    }
}

template <typename T, size_t Base>
class TestRadixSorter
{
public:
    template <typename Iterator>
    std::vector<T> operator()(bool reverse, Iterator begin, Iterator end)
    {
        if (reverse)
            std::reverse(begin, end);

        std::vector<T> sorted(begin, end);
        if (begin != end)
            radix_sort<Base>(sorted.begin(), sorted.end());
        assert(sorted.size() == (end - begin));
        if (sorted.size() <= 10)
            verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
        return sorted;
    }

    template <typename Iterator>
    void verbose(Iterator begin, Iterator end, bool success = true)
    {
        auto ToChar = [](int a)
        {
            return isprint(a) ? a : '?';
        };

        if (!success) printf("%s\n", success ? "success" : "failed");
        for (auto it{begin}; it != end; ++it)
            printf("%d %c ", (int)*it, ToChar(*it));
        assert(success);
    }

    template <typename Iterator>
    void check(Iterator begin, Iterator end)
    {
        auto previous{begin};
        for (auto it{begin}; it != end; ++it)
        {
            if (it != begin)
            {
                if (!(*previous++ <= *it))
                {
                    verbose(begin, end, false);
                    return;
                }
            }
        }
    }
};

int main()
{
    {
        assert(get_digit<10>(1234, 1) == 4);
        assert(get_digit<10>(1234, 10) == 3);
        assert(get_digit<10>(1234, 100) == 2);
        assert(get_digit<10>(1234, 1000) == 1);
        assert(get_digit<10>(1234, 10000) == 0);
    }

    constexpr int Base{10};
    TestRadixSorter<int, Base> test_sort;

    for (int reverse = 0; reverse <= 1; ++reverse)
    {
        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{2,3,1};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3,11,22};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3,22,11};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3,11,22,333,444};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3,11,5555,22,333,444};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{22,23,21, 32,33,31, 12,13,11};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{222,323,121, 232,333,131, 212,313,111};
            test_sort(reverse, data.begin(), data.end());
        }


        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3,4,5,6,7,8,9,10,11,12};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{1,2,3,4,5,6,7,8,9,10,12,11};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{9,8,7,1,2,3,1000,100,1234,5678,1234,5678};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            //std::vector<int> data{9,-8,7,-1,2,-3,1000,-100,1234,-5678,1234,-5678};
            //auto sorted = test_sort(reverse, data.begin(), data.end());
            //verbose(sorted.begin(), sorted.end());
            //check(sorted.begin(), sorted.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{9,8,7,1,2,3,100,1000,1234,5678,1234,5678};
            test_sort(reverse, data.begin(), data.end());
        }

        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{9,8,7,1,2,2234,3,100,1000,1234,5678,1234,5678};
            test_sort(reverse, data.begin(), data.end());
        }

        // Some bases/types/values interact poorly.
        // For example in base 4, the value 64 cannot represent
        // lower case letters, but the next value
        // is 256 which overflows unsigned char to zero.

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 2;
            TestRadixSorter<short, Base> test_sort;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 3;
            TestRadixSorter<short, Base> test_sort;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 8;
            TestRadixSorter<short, Base> test_sort;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 9;
            TestRadixSorter<short, Base> test_sort;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 10;
            TestRadixSorter<short, Base> test_sort;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        srand(static_cast<unsigned>(time(0)));

        // random data
        printf("\nline:%d\n", __LINE__);
        for (int size = 0; size < 999; ++size)
        {
            int data[999]{};
            for (int index = 0; index < size; ++index)
                data[index] = (0x7fffffff & rand());

            TestRadixSorter<int, 2>()(reverse, data, &data[size]);
            TestRadixSorter<int, 3>()(reverse, data, &data[size]);
            TestRadixSorter<int, 4>()(reverse, data, &data[size]);
            TestRadixSorter<int, 5>()(reverse, data, &data[size]);
            TestRadixSorter<int, 10>()(reverse, data, &data[size]);
            TestRadixSorter<int, 16>()(reverse, data, &data[size]);
            TestRadixSorter<int, 20>()(reverse, data, &data[size]);
            TestRadixSorter<int, 100>()(reverse, data, &data[size]);
            TestRadixSorter<int, 256>()(reverse, data, &data[size]);
        }
    }
}
