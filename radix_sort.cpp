#include <array>
#include <ctype.h>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <vector>

template <typename T, size_t Base>
class RadixSorter
{
public:
    template <typename Iterator>
    std::vector<T> operator()(Iterator begin, Iterator end)
    {
        std::vector<T> copy(begin, end);
        const size_t size = copy.size();
        temp.resize(size);
        helper(&copy[0], size, get_power(get_max_digits(begin, end)));
        return copy;
    }

private:

    static unsigned get_digits(T value)
    {
        unsigned digits = 1;
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
        unsigned value {1};
        for (auto it = begin; it < end; ++it)
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

    std::vector<T> temp;

    void helper(T* data, size_t size, T power)
    {
        if (size < 2 || power < 1)
            return;

        using array = std::array<size_t, Base>;

        array positions{};
        array counts{};
        size_t i {};
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

        std::copy(temp.begin(), temp.begin() + size, data);

        if (power == 1)
            return;

        // sort ranges
        for (i = 0; i < Base; ++i)
            helper(&data[positions[i]], counts[i], power / Base);
    }
    
    friend int main();
};

template <typename Iterator>
void verbose(Iterator begin, Iterator end, bool success = true)
{
    auto ToChar = [](int a)
    {
        return isprint(a) ? a : '?';
    };

    if (!success) printf("%s\n", success ? "success" : "failed");
    for (auto it = begin; it < end; ++it)
        printf("%d %c ", (int)*it, ToChar(*it));
    assert(success);
}

template <typename Iterator>
void check(Iterator begin, Iterator end)
{
    auto previous = begin;
    for (auto it = begin; it < end; ++it)
    {
        if (it != begin)
        {
            if (!(*previous <= *it))
            {
                verbose(begin, end, false);
                return;
            }
        }
    }
}

int main()
{
    {
        constexpr int Base = 10;
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

    constexpr int Base = 10;
    RadixSorter<int, Base> sort;

    {
        printf("\nline:%d\n", __LINE__);
        std::vector<int> data{1,2,3,4,5,6,7,8,9,10,11,12};
        auto sorted = sort(data.begin(), data.end());
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        std::vector<int> data{1,2,3,4,5,6,7,8,9,10,12,11};
        auto sorted = sort(data.begin(), data.end());
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        std::vector<int> data{9,8,7,1,2,3,1000,100,1234,5678,1234,5678};
        auto sorted = sort(data.begin(), data.end());
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        std::vector<int> data{9,8,7,1,2,3,100,1000,1234,5678,1234,5678};
        auto sorted = sort(data.begin(), data.end());
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        std::vector<int> data{9,8,7,1,2,2234,3,100,1000,1234,5678,1234,5678};
        auto sorted = sort(data.begin(), data.end());
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        constexpr int Base = 2;
        RadixSorter<char, Base> sort;
        char data[] = {'f','o','o','b','a','r'};
        auto sorted = sort(data, data + 1);
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        constexpr int Base = 3;
        RadixSorter<char, Base> sort;
        char data[] = {'f','o','o','b','a','r'};
        auto sorted = sort(data, data + 1);
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    // Some bases/types/values interact poorly.
    // For example in base 4, the value 64 cannot represent
    // lower case letters, but the next value
    // is 256 which overflows unsigned char to zero.

    {
        printf("\nline:%d\n", __LINE__);
        constexpr int Base = 8;
        RadixSorter<short, Base> sort;
        unsigned char data[] = {'f','o','o','b','a','r'};
        auto sorted = sort(data, &data[sizeof(data) / sizeof(data[0])]);
        assert(sorted.size() == 6);
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        constexpr int Base = 9;
        RadixSorter<short, Base> sort;
        unsigned char data[] = {'f','o','o','b','a','r'};
        auto sorted = sort(data, &data[sizeof(data) / sizeof(data[0])]);
        assert(sorted.size() == 6);
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }

    {
        printf("\nline:%d\n", __LINE__);
        constexpr int Base = 10;
        RadixSorter<short, Base> sort;
        unsigned char data[] = {'f','o','o','b','a','r'};
        auto sorted = sort(data, &data[sizeof(data) / sizeof(data[0])]);
        assert(sorted.size() == 6);
        verbose(sorted.begin(), sorted.end());
        check(sorted.begin(), sorted.end());
    }
}
