#include <gtest/gtest.h>
#include <expected>
#include <vector>
#include <string>

import cppsh.dispatching;
import cppsh.pipeline;
import cppsh.command;
import cppsh.shell_errors;
import cppsh.shell_state;

class DispatcherTest : public ::testing::Test {
protected:
    shell_state_t state;

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
    std::expected<int, shell_error_t> res = dispatch(log_pl, state);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, UnknownCommandReturnsError) {
    pipeline_t pl = make_pipeline({"unknowncommand"});
    std::expected<int, shell_error_t> res = dispatch({pl}, state);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, error_code_t::COMMAND_NOT_FOUND);
}

TEST_F(DispatcherTest, CdInvalidPathReturnsErrorCode) {
    pipeline_t pl = make_pipeline({"cd", "/this/path/does/not/exist"});
    std::expected<int, shell_error_t> res = dispatch({pl}, state);
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), static_cast<int>(error_code_t::INVALID_PATH));
}

TEST_F(DispatcherTest, CdValidPathReturnsZero) {
    pipeline_t pl = make_pipeline({"cd", "/tmp"});
    std::expected<int, shell_error_t> res = dispatch({pl}, state);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, HistoryEmptyReturnsZero) {
    pipeline_t pl = make_pipeline({"history"});
    std::expected<int, shell_error_t> res = dispatch({pl}, state);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, HistoryWithEntriesReturnsZero) {
    state.history = {"cd /tmp", "ls", "help"};
    pipeline_t pl = make_pipeline({"history"});
    std::expected<int, shell_error_t> res = dispatch({pl}, state);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}

TEST_F(DispatcherTest, LogicalAndSkipsOnFailure) {
    pipeline_t pl1 = make_pipeline({"cd", "/this/path/does/not/exist"});
    pl1.op = logical_op_t::AND;
    
    pipeline_t pl2 = make_pipeline({"help"});
    
    std::expected<int, shell_error_t> res = dispatch({pl1, pl2}, state);
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), static_cast<int>(error_code_t::INVALID_PATH));
}

TEST_F(DispatcherTest, LogicalOrRunsOnFailure) {
    pipeline_t pl1 = make_pipeline({"cd", "/this/path/does/not/exist"});
    pl1.op = logical_op_t::OR;
    
    pipeline_t pl2 = make_pipeline({"help"});
    std::expected<int, shell_error_t> res = dispatch({pl1, pl2}, state);
    
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 0);
}