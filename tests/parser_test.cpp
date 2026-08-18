#include <gtest/gtest.h>
#include <expected>
#include <unordered_map>
#include <string>
#include <vector>

import cppsh.parsing;
import cppsh.pipeline;
import cppsh.command;
import cppsh.shell_state;
import cppsh.env_entry;

class ParserTest : public ::testing::Test {
protected:
    std::unordered_map<std::string, env_entry_t> env_vars = {
        {"USER", {"user123", false}},
        {"DIR", {"/home/user", false}},
        {"FILE", {"doc", false}}
    };
};

TEST_F(ParserTest, SimpleCommand) {
    auto pl_res = parse("ls", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds.size(), 1);
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
}

TEST_F(ParserTest, CommandWithArguments) {
    auto pl_res = parse("ls -la /home", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 3);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[0].args[1], "-la");
    EXPECT_EQ(pl.cmds[0].args[2], "/home");
}

TEST_F(ParserTest, MultipleSpaces) {
    auto pl_res = parse("ls  -la", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[0].args[1], "-la");
}

TEST_F(ParserTest, OnlySpaces) {
    auto pl_res = parse("   ", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    EXPECT_TRUE(pl_res.value().empty() || pl_res.value()[0].cmds.empty() || pl_res.value()[0].cmds[0].args.empty());
}

TEST_F(ParserTest, InputRedirection) {
    auto pl_res = parse("sort < input.txt", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
}

TEST_F(ParserTest, OutputRedirection) {
    auto pl_res = parse("echo 'teste' > output.txt", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    EXPECT_EQ(pl.cmds[0].input_file, "");
    EXPECT_EQ(pl.cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl.cmds[0].append);
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
}

TEST_F(ParserTest, AppendRedirection) {
    auto pl_res = parse("echo 'teste' >> output.txt", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    EXPECT_EQ(pl.cmds[0].output_file, "output.txt");
    EXPECT_TRUE(pl.cmds[0].append);
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
}

TEST_F(ParserTest, IORedirection) {
    auto pl_res = parse("sort < input.txt > output.txt", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    EXPECT_EQ(pl.cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl.cmds[0].append);
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
}

TEST_F(ParserTest, InputRedirectionBeforeCommand) {
    auto pl_res = parse("< input.txt sort", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "sort");
}

TEST_F(ParserTest, BackgroundFlag) {
    auto pl_res = parse("sleep 10 &", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    EXPECT_TRUE(pl.bg);
    EXPECT_EQ(pl.cmds[0].args[0], "sleep");
}

TEST_F(ParserTest, MissingInputFile) {
    auto pl_res = parse("sort <", env_vars, 0);
    EXPECT_FALSE(pl_res.has_value());
}

TEST_F(ParserTest, MissingOutputFile) {
    auto pl_res = parse("echo >", env_vars, 0);
    EXPECT_FALSE(pl_res.has_value());
}

// Variable Expansion Tests

TEST_F(ParserTest, SimpleVarExpansion) {
    auto pl_res = parse("echo $USER", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[1], "user123");
}

TEST_F(ParserTest, VarExpansionInDoubleQuotes) {
    auto pl_res = parse("echo \"user: $USER\"", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[1], "user: user123");
}

TEST_F(ParserTest, VarExpansionInSingleQuotes) {
    auto pl_res = parse("echo '$USER'", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[1], "$USER");
}

TEST_F(ParserTest, VarConcatenation) {
    auto pl_res = parse("cat prefix_$FILE.txt", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[1], "prefix_doc.txt");
}

TEST_F(ParserTest, UnsetVarExpandsToEmpty) {
    auto pl_res = parse("echo $NON_EXISTENT", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "echo");
}

TEST_F(ParserTest, MultipleVarsInOneToken) {
    auto pl_res = parse("cd $DIR/$FILE", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[1], "/home/user/doc");
}

TEST_F(ParserTest, IsolatedDollarSign) {
    auto pl_res = parse("echo $", env_vars, 0);
    ASSERT_TRUE(pl_res.has_value());
    pipeline_t pl = pl_res.value()[0];
    
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[1], "$");
}