#include <gtest/gtest.h>
#include "parser.hpp"

// Test suite for the Parser class
class ParserTest : public ::testing::Test {
protected:
    cppsh::Parser parser;
};

// Simple command — "ls" -> ["ls"]
TEST_F(ParserTest, SimpleCommand) {
    auto cmd = parser.parse("ls");
    ASSERT_EQ(cmd.args.size(), 1);
    EXPECT_EQ(cmd.args[0], "ls");
}

// Command with arguments — "ls -la /home" -> ["ls", "-la", "/home"]
TEST_F(ParserTest, CommandWithArguments) {
    auto cmd = parser.parse("ls -la /home");
    ASSERT_EQ(cmd.args.size(), 3);
    EXPECT_EQ(cmd.args[0], "ls");
    EXPECT_EQ(cmd.args[1], "-la");
    EXPECT_EQ(cmd.args[2], "/home");
}

// Multiple spaces — "ls  -la" -> ["ls", "-la"]
TEST_F(ParserTest, MultipleSpaces) {
    auto cmd = parser.parse("ls  -la");
    ASSERT_EQ(cmd.args.size(), 2);
    EXPECT_EQ(cmd.args[0], "ls");
    EXPECT_EQ(cmd.args[1], "-la");
}

// Line with only spaces — "   " -> []
TEST_F(ParserTest, OnlySpaces) {
    auto cmd = parser.parse("   ");
    EXPECT_TRUE(cmd.args.empty());
}