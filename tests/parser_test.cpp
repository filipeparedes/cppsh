#include <gtest/gtest.h>
#include <expected>
#include <unordered_map>
#include <string>

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
    std::expected<pipeline_t, std::string> pl = parse("ls", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds.size(), 1);
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
}

TEST_F(ParserTest, CommandWithArguments) {
    std::expected<pipeline_t, std::string> pl = parse("ls -la /home", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 3);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[0].args[1], "-la");
    EXPECT_EQ(pl->cmds[0].args[2], "/home");
}

TEST_F(ParserTest, MultipleSpaces) {
    std::expected<pipeline_t, std::string> pl = parse("ls  -la", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[0], "ls");
    EXPECT_EQ(pl->cmds[0].args[1], "-la");
}

TEST_F(ParserTest, OnlySpaces) {
    std::expected<pipeline_t, std::string> pl = parse("   ", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_TRUE(pl->cmds.empty() || pl->cmds[0].args.empty());
}

TEST_F(ParserTest, InputRedirection) {
    std::expected<pipeline_t, std::string> pl = parse("sort < input.txt", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
}

TEST_F(ParserTest, OutputRedirection) {
    std::expected<pipeline_t, std::string> pl = parse("echo 'teste' > output.txt", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "");
    EXPECT_EQ(pl->cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl->cmds[0].append);
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
}

TEST_F(ParserTest, AppendRedirection) {
    std::expected<pipeline_t, std::string> pl = parse("echo 'teste' >> output.txt", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].output_file, "output.txt");
    EXPECT_TRUE(pl->cmds[0].append);
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
}

TEST_F(ParserTest, IORedirection) {
    std::expected<pipeline_t, std::string> pl = parse("sort < input.txt > output.txt", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    EXPECT_EQ(pl->cmds[0].output_file, "output.txt");
    EXPECT_FALSE(pl->cmds[0].append);
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
}

TEST_F(ParserTest, InputRedirectionBeforeCommand) {
    std::expected<pipeline_t, std::string> pl = parse("< input.txt sort", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_EQ(pl->cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
    EXPECT_EQ(pl->cmds[0].args[0], "sort");
}

TEST_F(ParserTest, BackgroundFlag) {
    std::expected<pipeline_t, std::string> pl = parse("sleep 10 &", env_vars);
    ASSERT_TRUE(pl.has_value());
    EXPECT_TRUE(pl->bg);
    EXPECT_EQ(pl->cmds[0].args[0], "sleep");
}

TEST_F(ParserTest, MissingInputFile) {
    std::expected<pipeline_t, std::string> pl = parse("sort <", env_vars);
    EXPECT_FALSE(pl.has_value());
}

TEST_F(ParserTest, MissingOutputFile) {
    std::expected<pipeline_t, std::string> pl = parse("echo >", env_vars);
    EXPECT_FALSE(pl.has_value());
}

// Variable Expansion Tests

TEST_F(ParserTest, SimpleVarExpansion) {
    std::expected<pipeline_t, std::string> pl = parse("echo $USER", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[1], "user123");
}

TEST_F(ParserTest, VarExpansionInDoubleQuotes) {
    std::expected<pipeline_t, std::string> pl = parse("echo \"user: $USER\"", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[1], "user: user123");
}

TEST_F(ParserTest, VarExpansionInSingleQuotes) {
    std::expected<pipeline_t, std::string> pl = parse("echo '$USER'", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[1], "$USER");
}

TEST_F(ParserTest, VarConcatenation) {
    std::expected<pipeline_t, std::string> pl = parse("cat prefix_$FILE.txt", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[1], "prefix_doc.txt");
}

TEST_F(ParserTest, UnsetVarExpandsToEmpty) {
    std::expected<pipeline_t, std::string> pl = parse("echo $NON_EXISTENT", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 1);
    EXPECT_EQ(pl->cmds[0].args[0], "echo");
}

TEST_F(ParserTest, MultipleVarsInOneToken) {
    std::expected<pipeline_t, std::string> pl = parse("cd $DIR/$FILE", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[1], "/home/user/doc");
}

TEST_F(ParserTest, IsolatedDollarSign) {
    std::expected<pipeline_t, std::string> pl = parse("echo $", env_vars);
    ASSERT_TRUE(pl.has_value());
    ASSERT_EQ(pl->cmds[0].args.size(), 2);
    EXPECT_EQ(pl->cmds[0].args[1], "$");
}