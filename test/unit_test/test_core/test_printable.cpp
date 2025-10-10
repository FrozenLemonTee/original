#include <gtest/gtest.h>
#include "printable.h"

namespace original {
    // 创建一个继承 printable 的测试类
    class TestClass : public printable {
    public:
        std::string className() const override {
            return "TestClass";
        }
    };

    enum class Color { Red = 1, Green, Blue };
}

// 测试 className() 方法
TEST(PrintableTest, ClassNameTest) {
    original::TestClass obj;
    EXPECT_EQ(obj.className(), "TestClass");
}

// 测试 toString() 方法
TEST(PrintableTest, ToStringTest) {
    original::TestClass obj;

    // 测试 toString 传入 false，不带换行
    std::string str = obj.toString(false);
    EXPECT_TRUE(str.find("TestClass") != std::string::npos); // 应该包含类名
    EXPECT_TRUE(str.find('@') != std::string::npos); // 应该包含地址信息

    // 测试 toString 传入 true，带换行
    str = obj.toString(true);
    EXPECT_TRUE(str.find('\n') != std::string::npos); // 应该包含换行符
}

// 测试 toCString() 方法
TEST(PrintableTest, ToCStringTest) {
    original::TestClass obj;
    const char* c_str = obj.toCString(false);
    EXPECT_TRUE(std::string(c_str).find("TestClass") != std::string::npos);
}

// 测试 formatString() 静态函数
TEST(PrintableTest, FormatStringTest) {
    // 测试整数
    EXPECT_EQ(original::printable::formatString(42), "42");

    // 测试字符串
    EXPECT_EQ(original::printable::formatString("hello"), "hello");

    // 测试布尔值
    EXPECT_EQ(original::printable::formatString(true), "true");
    EXPECT_EQ(original::printable::formatString(false), "false");

    // 测试字符
    EXPECT_EQ(original::printable::formatString('c'), "'c'");

    // 测试 nullptr
    EXPECT_EQ(original::printable::formatString(nullptr), "nullptr");
}

// 测试 formatCString() 静态函数
TEST(PrintableTest, FormatCStringTest) {
    // 测试整数
    EXPECT_EQ(std::string(original::printable::formatCString(42)), "42");

    // 测试字符串
    EXPECT_EQ(std::string(original::printable::formatCString("hello")), "hello");

    // 测试布尔值
    EXPECT_EQ(std::string(original::printable::formatCString(true)), "true");

    // 测试 nullptr
    EXPECT_EQ(std::string(original::printable::formatCString(nullptr)), "nullptr");
}

// 测试 formatEnum() 静态函数
TEST(PrintableTest, FormatEnumTest) {
    // 测试枚举类型
    EXPECT_EQ(original::printable::formatEnum(original::Color::Red), std::string(typeid(original::Color).name()) + "(1)");
    EXPECT_EQ(original::printable::formatEnum(original::Color::Green), std::string(typeid(original::Color).name()) + "(2)");
    EXPECT_EQ(original::printable::formatEnum(original::Color::Blue), std::string(typeid(original::Color).name()) + "(3)");
}

// 测试输出操作符
TEST(PrintableTest, OutputOperatorTest) {
    original::TestClass obj;
    std::ostringstream oss;
    oss << obj;
    EXPECT_TRUE(oss.str().find("TestClass") != std::string::npos); // 输出应包含类名
    EXPECT_TRUE(oss.str().find('@') != std::string::npos); // 输出应包含对象地址
}
