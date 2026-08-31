#include <gtest/gtest.h>
#include <expected>
#include <vector>
#include <string>
#include <cstdlib>

import cppsh.dispatching;
import cppsh.pipeline;
import cppsh.command;
import cppsh.shell_errors;
import cppsh.shell_state;

class DispatcherTest : public ::testing::Test {
protected:
    pipeline_t make_pipeline(std::vector<std::string> args) {
        pipeline_t pl;
        command_t cmd;
        cmd.args = args;
        pl.cmds.push_back(cmd);
        return pl;
    }
};

TEST_F(DispatcherTest, EmptyCommandReturnsZero) {
    std::vector<pipeline_t> log_pl; 
    std::expected<int, shell_error_t> res = dispatch(log_pl);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, UnknownCommandReturnsError) {
    pipeline_t pl = make_pipeline({"unknowncommand"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, error_code_t::COMMAND_NOT_FOUND);
}

TEST_F(DispatcherTest, CdInvalidPathReturnsErrorCode) {
    pipeline_t pl = make_pipeline({"cd", "/this/path/does/not/exist"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), static_cast<int>(error_code_t::INVALID_PATH));
}

TEST_F(DispatcherTest, CdValidPathReturnsZero) {
    pipeline_t pl = make_pipeline({"cd", "/tmp"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, HistoryEmptyReturnsZero) {
    clear_history(); 
    
    pipeline_t pl = make_pipeline({"history"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, HistoryWithEntriesReturnsZero) {
    clear_history();
    add_to_history("cd /tmp");
    add_to_history("ls");
    add_to_history("help");
    
    pipeline_t pl = make_pipeline({"history"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
    
    clear_history();
}

TEST_F(DispatcherTest, LogicalAndSkipsOnFailure) {
    pipeline_t pl1 = make_pipeline({"cd", "/this/path/does/not/exist"});
    pl1.op = logical_op_t::AND;
    
    pipeline_t pl2 = make_pipeline({"help"});
    
    std::expected<int, shell_error_t> res = dispatch({pl1, pl2});
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), static_cast<int>(error_code_t::INVALID_PATH));
}

TEST_F(DispatcherTest, LogicalOrRunsOnFailure) {
    pipeline_t pl1 = make_pipeline({"cd", "/this/path/does/not/exist"});
    pl1.op = logical_op_t::OR;
    
    pipeline_t pl2 = make_pipeline({"help"});
    std::expected<int, shell_error_t> res = dispatch({pl1, pl2});
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, ExportSetsEnvironmentVariable) {
    pipeline_t pl = make_pipeline({"export", "CPPSH_TEST_VAR=123"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
    
    const char* val = std::getenv("CPPSH_TEST_VAR");
    ASSERT_NE(val, nullptr);
    EXPECT_STREQ(val, "123");
    
    ::unsetenv("CPPSH_TEST_VAR");
}

TEST_F(DispatcherTest, UnsetRemovesEnvironmentVariable) {
    ::setenv("CPPSH_TEST_VAR_UNSET", "456", 1);
    
    pipeline_t pl = make_pipeline({"unset", "CPPSH_TEST_VAR_UNSET"});
    std::expected<int, shell_error_t> res = dispatch({pl});
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
    
    const char* val = std::getenv("CPPSH_TEST_VAR_UNSET");
    EXPECT_EQ(val, nullptr);
}