//
// radix_sort.cpp
//
// See https://en.wikipedia.org/wiki/Radix_sort
//
// This is original work by Jay Krell (jay.krell@cornell.edu),
// though many hints were provided and Wikipedia consulted.
// It was an intellectual exercise. Whether radix sort is actually
// useful, I do not know yet.
//
// Radix is also known as "base", base 10 is decimal, base 2 is binary.
// base 16 is hexadecimal, etc.
//
// "Decimal point" is more generally "radix point" such
// as in fixed point numbers. Digits to the left of a radix
// point are raised to positive powers of the base or radix, starting
// at 0, and numbers to the right of radix point are raised
// to negative powers. 0.1 binary is one half.
//
// While it maybe confusing, numbers can be represented in any base.
// At least integers 2 and higher, maybe more generally.
//
// Base 10 comes naturally to humans counting on fingers.
// Time is in base 60. 1 hour is 60 minutes.
// 1:00 is to base 60 as "100" or "60" is to base 10.
//
// Base 2 is natural in computers because of how transistors work.
// Base 16 is essentially a shorthand for base 2. Every 4 binary
// digits ("bits") become one hexadecimal digit ("nibble").
// Base64 is used to encode "binary" files as text, using
// "digits" a through z, A through Z, 0 through 9 and a few more.
//
// All that being said, the idea of radix sort is to partition
// values one digit at a time. Any base can be used.
// Into "buckets".
//
// First the size of each partition is computed, by running
// through the data divide/modding by a power of the base.
// For example the number 1234 is split into 1, 2, 3, 4.
//
// Temporary storage comeasurate to the input/output is used.
// The sort is stable. Equivalent values are kept in the original order.
//
// Given bucket sizes, implies starting positions of each partition
// in the combined output, or the temporary storage.
// The first bucket is at offset 0, the second is at the offset
// the size of the first bucket, etc.
//
// So given starting positions, the input is copied into temporary storage.
// A partial sort by one digit is therefore established.
//
// Each bucket is then sorted in the same way, using the next digit.
// The current implementation is recursive and serial.
// Two temporaries are swapped repeatedly to limit copying.
//
// Making it non-recursive, without mere queuing, was attempted
// but appears difficult or impossible. Of course, queueing is easy enough.
// There is a sort of "arbitrary fan out" or tree to the recursion.
//
// One progression would be:
//  22 23 21  32 33 31  12 13 11 original input with ups and downs in both digits.
//  12 13 11  22 23 21  32 33 31 sorted in first digit
//  11 12 13  21 22 23  31 32 33 sorted in both digits
//
#include <array>
#include <ctype.h>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <time.h>

// T is type for temporary and sorted output data.
// The input data can be a different type.
// The interactions of output type, input type, values,
// and base is complex and some combinations do not work,
// e.g. divide by zero.
//
// TODO: Also, negative numbers do not work.
// This class is stateless, so a template function suffices.
//
template <typename T, size_t Base>
class RadixSorter
{
public:
    template <typename Iterator>
    std::vector<T> operator()(Iterator begin, Iterator end)
    {
        // To limit copying, two temporaries repeatedly swap roles.
        // Is it possible to further reduce copying?
        std::vector<T> copy(begin, end);
        auto const size{copy.size()};
        if (size < 2)
            return copy;
        std::vector<T> temp(size);
        auto const max_digits = get_max_digits(copy.begin(), copy.end());
        helper(&copy[0], &temp[0], size, max_digits, get_power(max_digits));

        // max_digits determines recursion depth, determines number
        // of times data and temp have swapped.
        return (max_digits & 1) ? copy : temp;
    }

private:

    static unsigned get_digits(T value)
    {
        unsigned digits{1};
        while (value >= Base)
        {
            value /= Base;
            ++digits;
        }
        return digits;
    }

    template <typename Iterator>
    static unsigned get_max_digits(Iterator begin, Iterator end) // log
    {
        // TODO: There is a nicer way, e.g. std::accumulate
        unsigned value{1};
        for (auto it{begin}; it != end; ++it)
            value = std::max(value, get_digits(*it));
        return value;
    }

    static T get_power(unsigned n)
    {
        T value = 1;
        while (n > 0)
        {
            n -= 1;
            value *= Base;
        }
        return value;
    }

    static T get_digit(T value, T power)
    {
        return (value / power) % Base;
    }

    static void helper(
        T* data,
        T* temp,
        size_t size,
        unsigned max_digits,
        T power)
    {
        if (size >= 2 && power >= 1)
        {
            using array = std::array<size_t, Base>;

            array positions{};
            array counts{};
            size_t i{};
            size_t position{};

            // count them
            for (i = 0; i < size; ++i)
                counts[get_digit(data[i], power)] += 1;

            // compute range starts
            for (i = 0; i < Base; ++i)
            {
                positions[i] = position;
                position += counts[i];
            }

            {
                auto current_position = positions;

                // place them in ranges
                for (i = 0; i < size; ++i)
                {
                    const T d = data[i];
                    const T digit = get_digit(d, power);
                    temp[current_position[digit]] = d;
                    current_position[digit] += 1;
                }
            }

            // temp is now partially sorted (more than data)
            // swap temp and data
            // sort ranges

            if (power > 1)
            {
                for (i = 0; i < Base; ++i)
                {
                    auto const offset = positions[i];
                    helper(temp + offset, data + offset, counts[i], max_digits - 1, power / Base);
                }
            }
        }
        else
        {
            // Recursion depth is limited to max_digits, but also stops when size==1.
            // Copy is needed if size==1 an odd number of times before the maximum recursion.
            // That is, we could recurse till max_digits == 0, but that would only
            // move elements back and forth between data and temp. Instead, do one
            // last move and stop recursing. Well, odd vs. even is empirically derived,
            // to fix off by one.
            if (!(max_digits & 1))
                std::copy(data, data + size, temp);
        }
    }
    
    friend int main();
};

template <typename T, size_t Base>
class TestRadixSorter : RadixSorter<T, Base>
{
public:
    template <typename Iterator>
    std::vector<T> operator()(bool reverse, Iterator begin, Iterator end)
    {
        if (reverse)
            std::reverse(begin, end);

        auto const sorted = RadixSorter<T, Base>::operator()(begin, end);
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
        constexpr int Base{10};
        RadixSorter<int, Base> sort;
        assert(sort.get_digit(1234, 1) == 4);
        assert(sort.get_digit(1234, 10) == 3);
        assert(sort.get_digit(1234, 100) == 2);
        assert(sort.get_digit(1234, 1000) == 1);
        assert(sort.get_digit(1234, 10000) == 0);

        assert(sort.get_digits(4) == 1);
        assert(sort.get_digits(34) == 2);
        assert(sort.get_digits(234) == 3);
        assert(sort.get_digits(1234) == 4);
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
