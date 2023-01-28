//
// radix_sort.cpp
//
// See https://en.wikipedia.org/wiki/Radix_sort
//
// This is original work by Jay Krell (jay.krell@cornell.edu),
// though hints were provided and Wikipedia consulted somewhat.
// It was an intellectual exercise. Whether radix sort is actually
// useful, I do not know yet. (One "small" optimization copied from ChatGPT.
// ChatGPT based solution will be implemented in separate file).
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
// --------------------------------------------------------------------------
//
// ChatGPT has entered the fray and is very good at this, and much faster.
// It offers a somewhat different solution.
// ChatGPT's solution is not recursive.
// ChatGPT's solution does heap allocate and free more often,
// but has same peak storage, and this aspect is surely fixable.
// Related? ChatGPT does not use oscillating temporaries.
// ChatGPT sorts by least significant digit first.
// ChatGPT sorts the entire array at each iteration.
// Sorting the entire array is likely required due to
// least significant digit first.
//
// ChatGPT has one array where I have three, which is a simple
// optimization, and it computes ending positions instead of starting
// positions, then goes through the array in reverse, decrementing them.
// This does not seem like a very important difference.
//
// I believe at least some of the ideas are compatible.
// That is, you could use my heap allocation and oscillating temporaries,
// with ChatGPT's non-recursion, and LSB-first.
//
// ChatGPT likely exhibits worse data locality, since it visits the entire array
// forward and backward, k (max_digits) times. I probably visit the data the same
// amount, but with better locality.
//
// The recursive algorithm only recurses to at most max_digits depth, which
// does not seem terrible, and analogous to ChatGPT's k loop.
//
// ChatGPT writes in Rust instead of C++, which I aspire to.
// ChatGPT's solution suggests one very simple optimization that fits easily into my code,
// ChatGPT computes max and then loops like max > exp.
// This suggests, at least, I should compute log(max) instead of max(log).
//
// ChatGPT handles more type/value combinations correctly.
// Is mine fixable?
//
// --------------------------------------------------------------------------
//
#include <algorithm>
#include <array>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <numeric>
#include <random>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vector>
#if _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#endif

// T is type for temporary and sorted output data.
// The input data can be a different type.
// The interactions of output type, input type, values,
// and base is complex and some combinations do not work,
// e.g. divide by zero.
//
// TODO: Also, negative numbers do not work.
// This class is stateless, so a template function suffices.
//
#include "radix_sort_chatgpt.cpp"

template <typename T, int64_t Base>
class RadixSorter
{
public:
    bool chatGpt = false;
    bool handleNegativeNumbers = false;

    template <typename Iterator>
    std::vector<T> operator()(Iterator begin, Iterator end)
    {
        std::vector<T> copy(begin, end);

        if (chatGpt)
        {
            radix_sort<Base>(copy.begin(), copy.end());
            return copy;
        }
        else
        {
            // To limit copying, two temporaries repeatedly swap roles.
            auto const size{copy.size()};
            if (size < 2)
                return copy;

            std::vector<T> temp(size);

            T max = std::accumulate(copy.begin(), copy.end(), copy[0], [](T a, T b) { return std::max(a,b);});

            if (handleNegativeNumbers)
            {
                // Negative numbers require some work.
                // The model is roughly to double base.
                // Positive numbers get a biased by base mod.
                // There might be a better way, like if the numbers are -99..999
                // and max digits is 2 for negative and 3 for positive.
                T min = std::accumulate(copy.begin(), copy.end(), copy[0], [](T a, T b) { return std::min(a,b);});
                int64_t max_digits = std::max(get_digits(min), get_digits(max));
                helper(&copy[0], &temp[0], size, max_digits, get_power(max_digits));

                // max_digits determines recursion depth, determines number
                // of times data and temp have swapped.
                return (max_digits & 1) ? copy : temp;
            }
            int64_t max_digits = get_digits(max);
            helper(&copy[0], &temp[0], size, max_digits, get_power(max_digits));

            // max_digits determines recursion depth, determines number
            // of times data and temp have swapped.
            return (max_digits & 1) ? copy : temp;
        }
    }

private:

    static unsigned get_digits(T value)
    {
        // Return log(magnitude(value), Base)
        //
        // -99 => 2
        //  -1 => 1
        //   0 => 1
        //  99 => 2
        // Caller inevitably has to special case negative numbers.
        unsigned digits{1};

        if (value < 0)
        {
            if (value > -Base)
                return 1;

            // Avoid negating the most negative value, since it will overflow.
            // Divide first. The result cannot be the most negative value.
            // Base cannot be 1.
            value /= Base;
            value *= -1;
            ++digits;
        }

        while (value >= Base)
        {
            value /= Base;
            ++digits;
        }

        return digits;
    }

    static int64_t get_power(unsigned n)
    {
        int64_t value = 1;
        while (n > 0)
        {
            n -= 1;
            value *= Base;
        }
        return value;
    }

    T get_digit(T value, int64_t power)
    {
        if (handleNegativeNumbers)
            return (value < 0) ? (Base - ((value / -power) % Base)) : (Base + ((value / power) % Base));
        return (value / power) % Base;
    }

    void helper(
        T* data,
        T* temp,
        size_t size,
        int64_t max_digits,
        int64_t power)
    {
        if (size >= 2 && power >= 1)
        {
            using array = std::array<size_t, Base * 2>;

            array positions{};
            array counts{};
            size_t i{};
            size_t position{};

            // count them
            for (i = 0; i < size; ++i)
                counts[get_digit(data[i], power)] += 1;

            // compute range starts
            for (i = 0; i < Base * 2; ++i)
            {
                positions[i] = position;
                position += counts[i];
            }

            {
                auto current_position = positions;

                // place them in ranges
                for (i = 0; i < size; ++i)
                {
                    const T& d = data[i];
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
                for (i = 0; i < Base * 2; ++i)
                {
                    auto const offset = positions[i];
                    // Recursive depth is limited by log of the largest magintude data.
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
            // last copy and stop recursing. Well, odd vs. even is empirically derived,
            // to fix off by one.
            if (!(max_digits & 1))
                std::copy(data, data + size, temp);
        }
    }
    
    friend int main(int argc, char** argv);;
};

template <typename T, int64_t Base>
class TestRadixSorter : public RadixSorter<T, Base>
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
            return (a > 0 && a < 128 && isprint(a)) ? a : '?';
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

void Benchmark(size_t size)
{
    std::vector<int> orig(size, 0);
    std::vector<int> data(size, 0);

    for (size_t i = 0; i < size; ++i)
        orig[i] = (0x7fffffff & rand());

    TestRadixSorter<int, 16> test_sort;

    data = orig;
    time_t start_ChatGpt = time(0);
    test_sort.chatGpt = true;
    test_sort(false, &data[0], &data[size]);
    time_t end_ChatGpt = time(0);

    data = orig;
    time_t start_NoChatGpt = time(0);
    test_sort.chatGpt = false;
    test_sort(false, &data[0], &data[size]);
    time_t end_NoChatGpt = time(0);

    printf("noChatGpt:%d\n", (int)(end_NoChatGpt - start_NoChatGpt));
    printf("chatGpt:%d\n",   (int)(end_ChatGpt - start_ChatGpt));
}

int main(int argc, char** argv)
{
    bool chatGpt = false;
    bool benchmark = false;
    bool handleNegativeNumbers = false;
    uint64_t benchmark_size = 999999;

    while (*++argv)
    {
        if (strcmp(*argv, "chatgpt") == 0)
            chatGpt = true;
        else if (strcmp(*argv, "nochatgpt") == 0)
            chatGpt = false;
        else if (strcmp(*argv, "benchmark") == 0)
            benchmark = true;
        else if (strcmp(*argv, "handlenegativenumbers") == 0)
            handleNegativeNumbers = true;
        else if (strcmp(*argv, "benchmark_size") == 0 && argv[1])
        {
            uint64_t max = std::numeric_limits<uint64_t>::max();
            const char* s = argv[1];
            benchmark_size = 0;
            while (*s)
            {
                if (benchmark_size > (max / 10))
                {
                    printf("overflow\n");
                    exit(1);
                }
                benchmark_size *= 10;
                uint64_t digit = (*s - '0');
                if (benchmark_size > (max - digit))
                {
                    printf("overflow\n");
                    exit(1);
                }
                benchmark_size += digit;
                ++s;
            }
            assert(benchmark_size <= std::numeric_limits<size_t>::max());
            ++argv;
        }
    }

    if (chatGpt)
    {
        if (handleNegativeNumbers)
        {
            printf("chatGpt:handleNegativeNumbers = false");
            handleNegativeNumbers = false;
        }
    }

    if (benchmark)
    {
        Benchmark(benchmark_size);
        return 0;
    }


    {
        int data[] = {-9,-4,4,2,0};
        int min = std::accumulate(data, std::end(data), data[0], [](int a, int b) { return std::min(a,b);});
        int max = std::accumulate(data, std::end(data), data[0], [](int a, int b) { return std::max(a,b);});
        assert(min == -9);
        assert(max == 4);
    }

    {
        constexpr int Base{10};
        RadixSorter<int, Base> sort;
        if (sort.handleNegativeNumbers)
        {
            assert(sort.get_digit(1234, 1) == Base + 4);
            assert(sort.get_digit(1234, 10) == Base + 3);
            assert(sort.get_digit(1234, 100) == Base + 2);
            assert(sort.get_digit(1234, 1000) == Base + 1);
            assert(sort.get_digit(1234, 10000) == Base + 0);
            assert(sort.get_digit(0, 1) == Base);
            assert(sort.get_digit(0, 10) == Base);
            assert(sort.get_digit(0, 100) == Base);
            assert(sort.get_digit(0, 1000) == Base);
            assert(sort.get_digit(-1234, 1) == Base - 4);
            assert(sort.get_digit(-1234, 10) == Base - 3);
            assert(sort.get_digit(-1234, 100) == Base - 2);
            assert(sort.get_digit(-1234, 1000) == Base - 1);
            assert(sort.get_digit(-9234, 1000) == Base - 9);
            assert(sort.get_digit(-1234, 10000) == Base - 0);
        }
        else
        {
            assert(sort.get_digit(1234, 1) == 4);
            assert(sort.get_digit(1234, 10) == 3);
            assert(sort.get_digit(1234, 100) == 2);
            assert(sort.get_digit(1234, 1000) == 1);
            assert(sort.get_digit(1234, 10000) == 0);
            assert(sort.get_digit(0, 1) == 0);
            assert(sort.get_digit(0, 10) == 0);
            assert(sort.get_digit(0, 100) == 0);
            assert(sort.get_digit(0, 1000) == 0);
        }

        assert(sort.get_digits(4) == 1);
        assert(sort.get_digits(34) == 2);
        assert(sort.get_digits(234) == 3);
        assert(sort.get_digits(1234) == 4);

        assert(sort.get_digits(0) == 1);
        assert(sort.get_digits(-4) == 1);
        assert(sort.get_digits(-34) == 2);
        assert(sort.get_digits(-234) == 3);
        assert(sort.get_digits(-1234) == 4);
    }

    { // ChatGPT fork.
        assert(get_digit<10>(1234, 1) == 4);
        assert(get_digit<10>(1234, 10) == 3);
        assert(get_digit<10>(1234, 100) == 2);
        assert(get_digit<10>(1234, 1000) == 1);
        assert(get_digit<10>(1234, 10000) == 0);
    }

    constexpr int Base{10};
    TestRadixSorter<int, Base> test_sort;
    test_sort.chatGpt = chatGpt;
    test_sort.handleNegativeNumbers = handleNegativeNumbers;

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

        if (!chatGpt && test_sort.handleNegativeNumbers)
        {
            printf("\nline:%d\n", __LINE__);
            std::vector<int> data{9,-8,7,-1,2,-3,1000,-100,1234,-5678,1234,-5678};
            test_sort(reverse, data.begin(), data.end());
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
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 3;
            TestRadixSorter<short, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 8;
            TestRadixSorter<short, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 9;
            TestRadixSorter<short, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 10;
            TestRadixSorter<short, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 2;
            TestRadixSorter<unsigned char, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            unsigned char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 3;
            TestRadixSorter<unsigned char, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            unsigned char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 8;
            TestRadixSorter<unsigned char, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            unsigned char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 9;
            TestRadixSorter<unsigned char, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            unsigned char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        {
            printf("\nline:%d\n", __LINE__);
            constexpr int Base = 10;
            TestRadixSorter<unsigned char, Base> test_sort;
            test_sort.chatGpt = chatGpt;
            test_sort.handleNegativeNumbers = handleNegativeNumbers;
            unsigned char data[] = "foobar";
            auto const sorted = test_sort(reverse, data, std::end(data));
            assert(sorted.size() == 7);
        }

        // StackOverflow for random number generation.
        // ChatGPT code does not handle negative numbers
        using T = int32_t; // TODO: int64_t and uint64_t should work
        std::random_device dev;
        std::mt19937 engine(dev());
        std::uniform_int_distribution<T> distribution(
            handleNegativeNumbers ? std::numeric_limits<T>::min() : 0,
            std::numeric_limits<T>::max());

        // random data
        printf("\nline:%d\n", __LINE__);
        for (int size = 2; size < 999; ++size)
        {
            T data[999]{};
            for (int index = 0; index < size; ++index)
            {
                data[index] = distribution(engine);
                assert(handleNegativeNumbers || data[index] >= 0);
            }

            {
                TestRadixSorter<T, 2> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 3> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 4> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 5> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 10> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 16> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 20> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 100> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }

            {
                TestRadixSorter<T, 256> test_sort;
                test_sort.chatGpt = chatGpt;
                test_sort.handleNegativeNumbers = handleNegativeNumbers;
                test_sort(reverse, data, &data[size]);
            }
        }
    }
    printf("\nsuccess chatgpt:%d\n", (int)chatGpt);
}
