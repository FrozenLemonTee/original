#include <gtest/gtest.h>
#include "generators.h"
#include "array.h"
#include "vector.h"
#include "sets.h"
#include <string>

using namespace original;

class GeneratorsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试数据
        int_array = array{1, 2, 3, 4, 5};
        str_vector = vector<std::string>{"apple", "banana", "cherry", "date"};
        empty_vector = vector<int>{};
        single_element = vector{42};
    }

    void TearDown() override {
        // 清理资源
    }

    array<int> int_array{};
    vector<std::string> str_vector{};
    vector<int> empty_vector{};
    vector<int> single_element{};
};

// 测试 enumerate 函数
TEST_F(GeneratorsTest, EnumerateBasic) {
    auto gen = int_array.generator();
    auto enumerated = enumerate(std::move(gen));

    u_integer expected_index = 0;
    for (auto [index, value] : enumerated) {
        EXPECT_EQ(index, expected_index);
        EXPECT_EQ(value, int_array.get(expected_index));
        expected_index++;
    }
    EXPECT_EQ(expected_index, int_array.size());
}

TEST_F(GeneratorsTest, EnumerateEmpty) {
    auto gen = empty_vector.generator();
    auto enumerated = enumerate(std::move(gen));

    int count = 0;
    for (auto _ : enumerated) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

// 测试 collect 函数
TEST_F(GeneratorsTest, CollectToHashSet) {
    auto gen = int_array.generator();
    auto set = collect<int>(std::move(gen));

    EXPECT_EQ(set.size(), int_array.size());
    for (int i = 0; i < int_array.size(); i++) {
        EXPECT_TRUE(set.contains(int_array.get(i)));
    }
}

TEST_F(GeneratorsTest, CollectToTreeSet) {
    auto gen = int_array.generator();
    auto set = collect<int, treeSet<int>>(std::move(gen));

    EXPECT_EQ(set.size(), int_array.size());
    for (int i = 0; i < int_array.size(); i++) {
        EXPECT_TRUE(set.contains(int_array.get(i)));
    }
}

// 测试 list 函数
TEST_F(GeneratorsTest, ListFromGenerator) {
    auto gen = int_array.generator();
    auto vec = list<int, vector>(std::move(gen));

    EXPECT_EQ(vec.size(), int_array.size());
    for (u_integer i = 0; i < int_array.size(); i++) {
        EXPECT_EQ(vec.get(i), int_array.get(i));
    }
}

// 测试 transforms 函数
TEST_F(GeneratorsTest, TransformsBasic) {
    auto gen = int_array.generator();
    auto transformed = transforms(std::move(gen), [](int x) { return x * 2; });

    u_integer index = 0;
    for (auto value : transformed) {
        EXPECT_EQ(value, int_array.get(index) * 2);
        index++;
    }
}

TEST_F(GeneratorsTest, TransformsString) {
    auto gen = str_vector.generator();
    auto transformed = transforms(std::move(gen),
        [](const std::string& s) { return s.length(); });

    u_integer index = 0;
    for (auto length : transformed) {
        EXPECT_EQ(length, str_vector.get(index).length());
        index++;
    }
}

// 测试 filters 函数
TEST_F(GeneratorsTest, FiltersBasic) {
    auto gen = int_array.generator();
    auto filtered = filters(std::move(gen), [](int x) { return x % 2 == 0; });

    std::vector<int> expected{2, 4};
    u_integer index = 0;
    for (auto value : filtered) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

TEST_F(GeneratorsTest, FiltersNone) {
    auto gen = int_array.generator();
    auto filtered = filters(std::move(gen), [](int) { return false; });

    int count = 0;
    for ([[maybe_unused]] auto x : filtered) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

// 测试 extract 函数
TEST_F(GeneratorsTest, ExtractBasic) {
    auto gen = int_array.generator();
    auto extracted = extract(std::move(gen), [](int x) { return x % 2 == 0; });

    std::vector expected{1, 3, 5}; // 提取偶数，保留奇数
    u_integer index = 0;
    for (auto value : extracted) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

// 测试 zip 函数
TEST_F(GeneratorsTest, ZipBasic) {
    auto nums = array{1, 2, 3};
    auto strs = array<std::string>{"a", "b", "c"};
    auto gen1 = nums.generator();
    auto gen2 = strs.generator();
    auto zipped = zip(std::move(gen1), std::move(gen2));

    std::vector<std::pair<int, std::string>> expected{
        {1, "a"}, {2, "b"}, {3, "c"}
    };

    u_integer index = 0;
    for (auto [val1, val2] : zipped) {
        EXPECT_EQ(val1, expected[index].first);
        EXPECT_EQ(val2, expected[index].second);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

TEST_F(GeneratorsTest, ZipDifferentLengths) {
    const auto arr1 = array{1, 2, 3, 4};
    const auto arr2 = array{10, 20};
    auto gen1 = arr1.generator(); // 较长
    auto gen2 = arr2.generator();     // 较短
    auto zipped = zip(std::move(gen1), std::move(gen2));

    const std::vector<std::pair<int, int>> expected{
        {1, 10}, {2, 20}
    };

    u_integer index = 0;
    for (auto [val1, val2] : zipped) {
        EXPECT_EQ(val1, expected[index].first);
        EXPECT_EQ(val2, expected[index].second);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

// 测试 count 函数
TEST_F(GeneratorsTest, CountBasic) {
    auto gen = int_array.generator();
    auto result = count(std::move(gen));
    EXPECT_EQ(result, int_array.size());
}

TEST_F(GeneratorsTest, CountEmpty) {
    auto gen = empty_vector.generator();
    auto result = count(std::move(gen));
    EXPECT_EQ(result, 0);
}

TEST_F(GeneratorsTest, CountWithCondition) {
    auto gen = int_array.generator();
    auto result = count(std::move(gen), [](int x) { return x > 2; });
    EXPECT_EQ(result, 3); // 3, 4, 5
}

// 测试 all 函数
TEST_F(GeneratorsTest, AllTrue) {
    auto gen = int_array.generator();
    auto result = all(std::move(gen), [](int x) { return x > 0; });
    EXPECT_TRUE(result);
}

TEST_F(GeneratorsTest, AllFalse) {
    auto gen = int_array.generator();
    auto result = all(std::move(gen), [](int x) { return x > 3; });
    EXPECT_FALSE(result);
}

TEST_F(GeneratorsTest, AllEmpty) {
    auto gen = empty_vector.generator();
    auto result = all(std::move(gen), [](int) { return false; });
    EXPECT_TRUE(result); // 空集合上all返回true
}

// 测试 none 函数
TEST_F(GeneratorsTest, NoneTrue) {
    auto gen = int_array.generator();
    auto result = none(std::move(gen), [](int x) { return x < 0; });
    EXPECT_TRUE(result);
}

TEST_F(GeneratorsTest, NoneFalse) {
    auto gen = int_array.generator();
    auto result = none(std::move(gen), [](int x) { return x == 3; });
    EXPECT_FALSE(result);
}

// 测试 any 函数
TEST_F(GeneratorsTest, AnyTrue) {
    auto gen = int_array.generator();
    auto result = any(std::move(gen), [](int x) { return x == 3; });
    EXPECT_TRUE(result);
}

TEST_F(GeneratorsTest, AnyFalse) {
    auto gen = int_array.generator();
    auto result = any(std::move(gen), [](int x) { return x > 10; });
    EXPECT_FALSE(result);
}

// 测试 join 函数
TEST_F(GeneratorsTest, JoinBasic) {
    const auto arr1 = array{1, 2};
    const auto arr2 = array{3, 4};
    auto gen1 = arr1.generator();
    auto gen2 = arr2.generator();
    auto joined = join(std::move(gen1), std::move(gen2));

    const std::vector expected{1, 2, 3, 4};
    u_integer index = 0;
    for (auto value : joined) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

// 测试 flatten 函数
TEST_F(GeneratorsTest, FlattenBasic) {
    const auto couples = vector<couple<int, int>>{
        {1, 2}, {3, 4}, {5, 6}
    };
    auto gen = couples.generator();
    auto flattened = flatten<int, int>(std::move(gen));

    const std::vector expected{1, 2, 3, 4, 5, 6};
    u_integer index = 0;
    for (auto value : flattened) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

// 测试 take 函数
TEST_F(GeneratorsTest, TakeBasic) {
    auto gen = int_array.generator();
    auto taken = take(std::move(gen), 3);

    std::vector<int> expected{1, 2, 3};
    u_integer index = 0;
    for (auto value : taken) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, 3);
}

TEST_F(GeneratorsTest, TakeMoreThanAvailable) {
    auto gen = int_array.generator();
    auto taken = take(std::move(gen), 10); // 要求10个，但只有5个

    u_integer count = 0;
    for (auto value : taken) {
        EXPECT_EQ(value, int_array.get(count));
        count++;
    }
    EXPECT_EQ(count, int_array.size());
}

// 测试 skip 函数
TEST_F(GeneratorsTest, SkipBasic) {
    auto gen = int_array.generator();
    auto skipped = skip(std::move(gen), 2);

    std::vector<int> expected{3, 4, 5};
    u_integer index = 0;
    for (auto value : skipped) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

TEST_F(GeneratorsTest, SkipAll) {
    auto gen = int_array.generator();
    auto skipped = skip(std::move(gen), 10); // 跳过所有

    int count = 0;
    for ([[maybe_unused]] auto x : skipped) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

// 测试 position 函数
TEST_F(GeneratorsTest, PositionFound) {
    auto gen = int_array.generator();
    auto pos = position(std::move(gen), [](int x) { return x == 3; });
    EXPECT_EQ(pos, 2);
}

TEST_F(GeneratorsTest, PositionNotFound) {
    auto gen = int_array.generator();
    auto pos = position(std::move(gen), [](int x) { return x == 10; });
    EXPECT_EQ(pos, int_array.size()); // 返回size表示未找到
}

// 测试 find 函数
TEST_F(GeneratorsTest, FindFound) {
    auto gen = int_array.generator();
    auto result = find(std::move(gen), [](int x) { return x == 4; });
    EXPECT_EQ(result, 4);
}

TEST_F(GeneratorsTest, FindNotFound) {
    auto gen = int_array.generator();
    auto result = find(std::move(gen), [](int x) { return x == 10; });
    EXPECT_EQ(result, int{}); // 返回默认构造的值
}

// 测试管道操作符
TEST_F(GeneratorsTest, PipeOperations) {
    auto result = int_array.generator()
        | transforms([](int x) { return x + 1; })
        | filters([](int x) { return x % 2 == 0; })
        | take(2);

    std::vector<int> expected{2, 4}; // 1+1=2, 3+1=4
    u_integer index = 0;
    for (auto value : result) {
        EXPECT_EQ(value, expected[index]);
        index++;
    }
    EXPECT_EQ(index, 2);
}

// 测试复杂的管道操作
TEST_F(GeneratorsTest, ComplexPipeOperations) {
    auto result = str_vector.generator()
        | transforms([](const std::string& s) { return s.length(); })
        | filters([](const size_t len) { return len > 4; })
        | enumerate();

    vector<couple<u_integer, size_t>> expected{
        {0, 5}, // "apple" -> 5
        {1, 6}, // "banana" -> 6
        {2, 6}, // "cherry" -> 6
    };

    u_integer index = 0;
    for (auto [pos, len] : result) {
        EXPECT_EQ(pos, expected[index].first());
        EXPECT_EQ(len, expected[index].second());
        index++;
    }
    EXPECT_EQ(index, 3);
}

// 性能测试：大数据集
TEST_F(GeneratorsTest, LargeDataset) {
    constexpr u_integer SIZE = 1000;
    vector<int> large_vector;
    for (u_integer i = 0; i < SIZE; i++) {
        large_vector.pushEnd(static_cast<int>(i));
    }

    auto gen = large_vector.generator();
    auto result = count(std::move(gen));
    EXPECT_EQ(result, SIZE);
}

// 测试异常安全性
TEST_F(GeneratorsTest, ExceptionInTransform) {
    auto gen = int_array.generator();
    auto transformed = transforms(std::move(gen), [](int x) {
        if (x == 3) {
            throw std::runtime_error("test error");
        }
        return x;
    });

    EXPECT_THROW({
        for ([[maybe_unused]] auto value : transformed) {
            // 消费生成器，应该在遇到3时抛出异常
        }
    }, std::runtime_error);
}

// 测试集合类型的generator接口
TEST_F(GeneratorsTest, SetGenerator) {
    hashSet<int> test_set;
    test_set.add(1);
    test_set.add(2);
    test_set.add(3);

    auto gen = test_set.generator();
    int sum = 0;
    for (const auto value : gen) {
        sum += value;
    }

    // 由于hashSet无序，我们只检查总和
    EXPECT_EQ(sum, 6);
}

// 测试 reduce 函数
TEST_F(GeneratorsTest, ReduceBasic) {
    auto gen = int_array.generator();
    const int sum = reduce(std::move(gen), 0, [](const int acc, const int val) {
        return acc + val;
    });
    EXPECT_EQ(sum, 15); // 1+2+3+4+5=15
}

TEST_F(GeneratorsTest, ReduceProduct) {
    auto gen = int_array.generator();
    const int product = reduce(std::move(gen), 1, [](const int acc, const int val) {
        return acc * val;
    });
    EXPECT_EQ(product, 120); // 1*2*3*4*5=120
}

TEST_F(GeneratorsTest, ReduceString) {
    auto gen = str_vector.generator();
    const std::string result = reduce(std::move(gen), std::string(""),
        [](const std::string& acc, const std::string& val) {
            return acc.empty() ? val : acc + "," + val;
        });
    EXPECT_EQ(result, "apple,banana,cherry,date");
}

TEST_F(GeneratorsTest, ReduceEmpty) {
    auto gen = empty_vector.generator();
    constexpr int init = 0;
    const int result = reduce(std::move(gen), init, [](const int acc, const int val) {
        return acc + val;
    });
    EXPECT_EQ(result, init); // 空生成器应该返回初始值
}

TEST_F(GeneratorsTest, ReduceSingleElement) {
    auto gen = single_element.generator();
    const int result = reduce(std::move(gen), 0, [](const int acc, const int val) {
        return acc + val;
    });
    EXPECT_EQ(result, 42); // 0 + 42 = 42
}

// 测试 maximum 函数
TEST_F(GeneratorsTest, MaximumBasic) {
    auto gen = int_array.generator();
    const int max_val = maximum(std::move(gen), 0);
    EXPECT_EQ(max_val, 5);
}

TEST_F(GeneratorsTest, MaximumWithNegative) {
    const vector nums = {-3, -1, -5, -2};
    auto gen = nums.generator();
    const int max_val = maximum(std::move(gen), (std::numeric_limits<int>::min()));
    EXPECT_EQ(max_val, -1);
}

TEST_F(GeneratorsTest, MaximumEmpty) {
    auto gen = empty_vector.generator();
    const int max_val = maximum(std::move(gen), 999);
    EXPECT_EQ(max_val, 999); // 空生成器应该返回初始值
}

// 测试 minimum 函数
TEST_F(GeneratorsTest, MinimumBasic) {
    auto gen = int_array.generator();
    const int min_val = minimum(std::move(gen), 100);
    EXPECT_EQ(min_val, 1);
}

TEST_F(GeneratorsTest, MinimumWithNegative) {
    const vector nums = {-3, -1, -5, -2};
    auto gen = nums.generator();
    const int min_val = minimum(std::move(gen), 0);
    EXPECT_EQ(min_val, -5);
}

TEST_F(GeneratorsTest, MinimumEmpty) {
    auto gen = empty_vector.generator();
    const int min_val = minimum(std::move(gen), -999);
    EXPECT_EQ(min_val, -999); // 空生成器应该返回初始值
}

// 测试 summation 函数
TEST_F(GeneratorsTest, SummationBasic) {
    auto gen = int_array.generator();
    const int sum = summation(std::move(gen), 0);
    EXPECT_EQ(sum, 15);
}

TEST_F(GeneratorsTest, SummationWithInitial) {
    auto gen = int_array.generator();
    const int sum = summation(std::move(gen), 10);
    EXPECT_EQ(sum, 25); // 10 + 15 = 25
}

TEST_F(GeneratorsTest, SummationEmpty) {
    auto gen = empty_vector.generator();
    const int sum = summation(std::move(gen), 100);
    EXPECT_EQ(sum, 100); // 空生成器应该返回初始值
}

// 测试 reduce 管道操作
TEST_F(GeneratorsTest, ReducePipeOperation) {
    const int result = int_array.generator()
        | reduce(0, [](const int acc, const int val) { return acc + val; });
    EXPECT_EQ(result, 15);
}

TEST_F(GeneratorsTest, MaximumPipeOperation) {
    const int result = int_array.generator()
        | maximum(0);
    EXPECT_EQ(result, 5);
}

TEST_F(GeneratorsTest, MinimumPipeOperation) {
    const int result = int_array.generator()
        | minimum(100);
    EXPECT_EQ(result, 1);
}

TEST_F(GeneratorsTest, SummationPipeOperation) {
    const int result = int_array.generator()
        | summation(0);
    EXPECT_EQ(result, 15);
}

// 测试复杂的 reduce 操作
TEST_F(GeneratorsTest, ComplexReduce) {
    struct Stats {
        int count;
        int sum;
        int min_val;
        int max_val;

        Stats() : count(0), sum(0), min_val(std::numeric_limits<int>::max()),
                 max_val(std::numeric_limits<int>::min()) {}

        Stats(const int c, const int s, const int min, const int max) : count(c), sum(s), min_val(min), max_val(max) {}

        // 合并两个 Stats 对象
        Stats operator+(const Stats& other) const {
            return Stats{
                this->count + other.count,
                this->sum + other.sum,
                std::min(this->min_val, other.min_val),
                std::max(this->max_val, other.max_val)
            };
        }
    };

    auto gen = int_array.generator();

    // 将每个 int 转换为 Stats
    auto transformed = transforms(std::move(gen), [](const int val) {
        return Stats{1, val, val, val}; // 每个元素转换为包含自身统计信息的 Stats
    });

    const Stats result = reduce(std::move(transformed), Stats{},
        [](const Stats acc, const Stats val) {
            return acc + val; // 使用重载的 + 操作符合并 Stats
        });

    EXPECT_EQ(result.count, 5);
    EXPECT_EQ(result.sum, 15);
    EXPECT_EQ(result.min_val, 1);
    EXPECT_EQ(result.max_val, 5);
}

// 测试 reduce 的移动语义
TEST_F(GeneratorsTest, ReduceMoveSemantics) {
    const vector<std::string> strings = {"a", "b", "c"};
    auto gen = strings.generator();

    const std::string result = reduce(std::move(gen), std::string(""),
        [](std::string acc, std::string val) {
            return std::move(acc) + std::move(val);
        });

    EXPECT_EQ(result, "abc");
}

// 测试 reduce 性能
TEST_F(GeneratorsTest, ReducePerformance) {
    constexpr u_integer SIZE = 1000;
    vector<int> large_vector;
    for (u_integer i = 1; i <= SIZE; i++) {
        large_vector.pushEnd(static_cast<int>(i));
    }

    auto gen = large_vector.generator();
    const int sum = reduce(std::move(gen), 0, [](const int acc, const int val) {
        return acc + val;
    });

    constexpr int expected_sum = SIZE * (SIZE + 1) / 2;
    EXPECT_EQ(sum, expected_sum);
}

// 测试 reduce 与 transforms 组合
TEST_F(GeneratorsTest, ReduceWithTransform) {
    auto gen = int_array.generator();
    auto transformed = transforms(std::move(gen), [](const int x) { return x * 2; });
    const int sum = reduce(std::move(transformed), 0, [](const int acc, const int val) {
        return acc + val;
    });

    EXPECT_EQ(sum, 30); // (1+2+3+4+5)*2 = 30
}

// 测试 reduce 与 filters 组合
TEST_F(GeneratorsTest, ReduceWithFilter) {
    auto gen = int_array.generator();
    auto filtered = filters(std::move(gen), [](const int x) { return x % 2 == 0; });
    const int sum = reduce(std::move(filtered), 0, [](const int acc, const int val) {
        return acc + val;
    });

    EXPECT_EQ(sum, 6); // 2+4=6
}

// 测试管道操作中的 reduce
TEST_F(GeneratorsTest, PipeWithReduce) {
    const int result = int_array.generator()
        | filters([](const int x) { return x > 2; })     // 3,4,5
        | transforms([](const int x) { return x * 3; })  // 9,12,15
        | reduce(0, [](const int acc, const int val) { return acc + val; }); // 9+12+15=36

    EXPECT_EQ(result, 36);
}

// 测试 collect 管道操作
TEST_F(GeneratorsTest, CollectPipeOperation) {
    const auto set = int_array.generator()
        | collect<int, hashSet<int>>();

    EXPECT_EQ(set.size(), 5);
    for (int i = 1; i <= 5; i++) {
        EXPECT_TRUE(set.contains(i));
    }
}

// 测试 list 管道操作
TEST_F(GeneratorsTest, ListPipeOperation) {
    const auto vec = int_array.generator()
        | list<vector>();

    EXPECT_EQ(vec.size(), 5);
    for (u_integer i = 0; i < 5; i++) {
        EXPECT_EQ(vec.get(i), int_array.get(i));
    }
}