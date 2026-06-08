#include <gtest/gtest.h>
#include "parser.hpp"

class ParserTest : public ::testing::Test {
protected:
    cppsh::Parser parser;
};

TEST_F(ParserTest, SimpleCommand) {
    cppsh::Pipeline pl = parser.parse("ls");
    ASSERT_EQ(pl.cmds.size(), 1);
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
}

TEST_F(ParserTest, CommandWithArguments) {
    cppsh::Pipeline pl = parser.parse("ls -la /home");
    ASSERT_EQ(pl.cmds[0].args.size(), 3);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[0].args[1], "-la");
    EXPECT_EQ(pl.cmds[0].args[2], "/home");
}

TEST_F(ParserTest, MultipleSpaces) {
    cppsh::Pipeline pl = parser.parse("ls  -la");
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[0].args[1], "-la");
}

TEST_F(ParserTest, OnlySpaces) {
    cppsh::Pipeline pl = parser.parse("   ");
    EXPECT_TRUE(pl.cmds.empty() || pl.cmds[0].args.empty());
}

TEST_F(ParserTest, InputRedirection) {
    cppsh::Pipeline pl = parser.parse("sort < input.txt");
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
}

TEST_F(ParserTest, OutputRedirection) {
    cppsh::Pipeline pl = parser.parse("echo 'teste' > output.txt");
    EXPECT_EQ(pl.cmds[0].input_file, "");
    EXPECT_EQ(pl.cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl.cmds[0].append);
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
}

TEST_F(ParserTest, AppendRedirection) {
    cppsh::Pipeline pl = parser.parse("echo 'teste' >> output.txt");
    EXPECT_EQ(pl.cmds[0].output_file, "output.txt");
    EXPECT_TRUE(pl.cmds[0].append);
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
}

TEST_F(ParserTest, IORedirection) {
    cppsh::Pipeline pl = parser.parse("sort < input.txt > output.txt");
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    EXPECT_EQ(pl.cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl.cmds[0].append);
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
}

TEST_F(ParserTest, InputRedirectionBeforeCommand) {
    cppsh::Pipeline pl = parser.parse("< input.txt sort");
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "sort");
}

TEST_F(ParserTest, PipeCreatesTwoCommands) {
    cppsh::Pipeline pl = parser.parse("ls | grep txt");
    ASSERT_EQ(pl.cmds.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
}

TEST_F(ParserTest, BackgroundFlag) {
    cppsh::Pipeline pl = parser.parse("sleep 10 &");
    EXPECT_TRUE(pl.bg);
    EXPECT_EQ(pl.cmds[0].args[0], "sleep");
}

TEST_F(ParserTest, MissingInputFile) {
    EXPECT_THROW(parser.parse("sort <"), std::exception);
}

TEST_F(ParserTest, MissingOutputFile) {
    EXPECT_THROW(parser.parse("echo >"), std::exception);
}