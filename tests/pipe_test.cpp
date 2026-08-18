#include <gtest/gtest.h>
#include <expected>
#include <vector>

import cppsh.parsing;
import cppsh.pipeline;
import cppsh.shell_state;

class PipeTest : public ::testing::Test {
protected:
    shell_state_t state;

    pipeline_t parse_single_pipeline(const std::string& input) {
        auto res = parse(input, state.env_variables, state.last_exit_code);
        EXPECT_TRUE(res.has_value());
        EXPECT_GE(res->size(), 1);
        return res.value()[0];
    }
};

TEST_F(PipeTest, SimpleTwoStagePipe) {
    pipeline_t pl = parse_single_pipeline("ls | grep");
    ASSERT_EQ(pl.cmds.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
}

TEST_F(PipeTest, PipeWithArguments) {
    pipeline_t pl = parse_single_pipeline("ls -la | grep txt");
    ASSERT_EQ(pl.cmds.size(), 2);
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[0].args[1], "-la");
    ASSERT_EQ(pl.cmds[1].args.size(), 2);
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[1].args[1], "txt");
}

TEST_F(PipeTest, MultiplePipesChain) {
    pipeline_t pl = parse_single_pipeline("cat names.txt | grep silva | wc -l");
    ASSERT_EQ(pl.cmds.size(), 3);
    EXPECT_EQ(pl.cmds[0].args[0], "cat");
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[2].args[0], "wc");
    EXPECT_EQ(pl.cmds[2].args[1], "-l");
}

TEST_F(PipeTest, PipeWithExcessiveSpaces) {
    pipeline_t pl = parse_single_pipeline("ls   -la    |      grep      cpp");
    ASSERT_EQ(pl.cmds.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[1].args[1], "cpp");
}

TEST_F(PipeTest, PipeMixedWithIORedirection) {
    pipeline_t pl = parse_single_pipeline("sort < input.txt | grep erro > output.txt");
    ASSERT_EQ(pl.cmds.size(), 2);
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    EXPECT_EQ(pl.cmds[0].args[0], "sort");
    EXPECT_EQ(pl.cmds[1].output_file, "output.txt");
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[1].args[1], "erro");
}