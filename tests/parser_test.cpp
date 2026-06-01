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

// Simple input redirection
TEST_F(ParserTest, InputRedirection) {
    auto cmd = parser.parse("sort < input.txt");
    EXPECT_EQ(cmd.input_file, "input.txt");
    ASSERT_EQ(cmd.args.size(), 1);
}

// Simple output redirection (overwrite)
TEST_F(ParserTest, OutputRedirection) {
    auto cmd = parser.parse("echo 'teste' > output.txt");
    EXPECT_EQ(cmd.input_file, "");
    EXPECT_EQ(cmd.output_file, "output.txt");
    EXPECT_FALSE(cmd.append);
    ASSERT_EQ(cmd.args.size(), 2);
}

// Simple output redirection (append)
TEST_F(ParserTest, AppendRedirection) {
    auto cmd = parser.parse("echo 'teste' >> output.txt");
    EXPECT_EQ(cmd.input_file, "");
    EXPECT_EQ(cmd.output_file, "output.txt");
    EXPECT_TRUE(cmd.append);
    ASSERT_EQ(cmd.args.size(), 2);
}

// Input AND output redirection
TEST_F(ParserTest, IORedirection) {
    auto cmd = parser.parse("sort < input.txt > output.txt");
    EXPECT_EQ(cmd.input_file, "input.txt");
    EXPECT_EQ(cmd.output_file, "output.txt");
    EXPECT_FALSE(cmd.append);
    ASSERT_EQ(cmd.args.size(), 1);
}

//Input redirection before command
TEST_F(ParserTest, InputRedirectionBeforeCommand) {
    auto cmd = parser.parse("< input.txt sort");

    EXPECT_EQ(cmd.input_file, "input.txt");
    ASSERT_EQ(cmd.args.size(), 1);
    EXPECT_EQ(cmd.args[0], "sort");
}

//Input redirection with missing file name
TEST_F(ParserTest, MissingInputFile) {
    EXPECT_THROW(parser.parse("sort <"), std::exception);
}

//Output redirection with missing file name
TEST_F(ParserTest, MissingOutputFile) {
    EXPECT_THROW(parser.parse("echo >"), std::exception);
}