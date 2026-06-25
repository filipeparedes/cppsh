#include <gtest/gtest.h>
#include <expected>

import cppsh.parsing;
import cppsh.pipeline;

class PipeTest : public ::testing::Test {};

TEST_F(PipeTest, SimpleTwoStagePipe) {
    std::expected<pipeline_t, std::string> pl = parse("ls | grep");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[1].args[0], "grep");
}

TEST_F(PipeTest, PipeWithArguments) {
    std::expected<pipeline_t, std::string> pl = parse("ls -la | grep txt");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 2);
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[0].args[1], "-la");
    ASSERT_EQ(pl->cmds[1].args.size(), 2);
    EXPECT_EQ(pl->cmds[1].args[0], "grep");
    EXPECT_EQ(pl->cmds[1].args[1], "txt");
}

TEST_F(PipeTest, MultiplePipesChain) {
    std::expected<pipeline_t, std::string> pl = parse("cat names.txt | grep silva | wc -l");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 3);
    EXPECT_EQ(pl->cmds[0].args[0], "cat");
    EXPECT_EQ(pl->cmds[1].args[0], "grep");
    EXPECT_EQ(pl->cmds[2].args[0], "wc");
    EXPECT_EQ(pl->cmds[2].args[1], "-l");
}

TEST_F(PipeTest, PipeWithExcessiveSpaces) {
    std::expected<pipeline_t, std::string> pl = parse("ls   -la    |      grep      cpp");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[1].args[0], "grep");
    EXPECT_EQ(pl->cmds[1].args[1], "cpp");
}

TEST_F(PipeTest, PipeMixedWithIORedirection) {
    std::expected<pipeline_t, std::string> pl = parse("sort < input.txt | grep erro > output.txt");
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 2);
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    EXPECT_EQ(pl->cmds[0].args[0], "sort");
    EXPECT_EQ(pl->cmds[1].output_file, "output.txt");
    EXPECT_EQ(pl->cmds[1].args[0], "grep");
    EXPECT_EQ(pl->cmds[1].args[1], "erro");
}