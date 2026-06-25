#include <gtest/gtest.h>
#include <expected>

import cppsh.parsing;
import cppsh.pipeline;
import cppsh.command;

class ParserTest : public ::testing::Test {};

TEST_F(ParserTest, SimpleCommand) {
    std::expected<pipeline_t, std::string> pl = parse("ls");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 1);
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
}

TEST_F(ParserTest, CommandWithArguments) {
    std::expected<pipeline_t, std::string> pl = parse("ls -la /home");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 3);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[0].args[1], "-la");
    EXPECT_EQ(pl->cmds[0].args[2], "/home");
}

TEST_F(ParserTest, MultipleSpaces) {
    std::expected<pipeline_t, std::string> pl = parse("ls  -la");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[0].args[1], "-la");
}

TEST_F(ParserTest, OnlySpaces) {
    std::expected<pipeline_t, std::string> pl = parse("   ");
    ASSERT_TRUE(pl.has_value());
    EXPECT_TRUE(pl->cmds.empty() || pl->cmds[0].args.empty());
}

TEST_F(ParserTest, InputRedirection) {
    std::expected<pipeline_t, std::string> pl = parse("sort < input.txt");
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
}

TEST_F(ParserTest, OutputRedirection) {
    std::expected<pipeline_t, std::string> pl = parse("echo 'teste' > output.txt");
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "");
    EXPECT_EQ(pl->cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl->cmds[0].append);
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
}

TEST_F(ParserTest, AppendRedirection) {
    std::expected<pipeline_t, std::string> pl = parse("echo 'teste' >> output.txt");
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].output_file, "output.txt");
    EXPECT_TRUE(pl->cmds[0].append);
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
}

TEST_F(ParserTest, IORedirection) {
    std::expected<pipeline_t, std::string> pl = parse("sort < input.txt > output.txt");
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    EXPECT_EQ(pl->cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl->cmds[0].append);
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
}

TEST_F(ParserTest, InputRedirectionBeforeCommand) {
    std::expected<pipeline_t, std::string> pl = parse("< input.txt sort");
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
    EXPECT_EQ(pl->cmds[0].args[0], "sort");
}

TEST_F(ParserTest, BackgroundFlag) {
    std::expected<pipeline_t, std::string> pl = parse("sleep 10 &");
    ASSERT_TRUE(pl.has_value());
    EXPECT_TRUE(pl->bg);
    EXPECT_EQ(pl->cmds[0].args[0], "sleep");
}

TEST_F(ParserTest, MissingInputFile) {
    std::expected<pipeline_t, std::string> pl = parse("sort <");
    EXPECT_FALSE(pl.has_value());
}

TEST_F(ParserTest, MissingOutputFile) {
    std::expected<pipeline_t, std::string> pl = parse("echo >");
    EXPECT_FALSE(pl.has_value());
}