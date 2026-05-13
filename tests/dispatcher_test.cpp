#include <gtest/gtest.h>
#include "dispatcher.hpp"
#include "context.hpp"

class DispatcherTest : public ::testing::Test {
protected:
    Dispatcher dispatcher;
    ShellContext context{dispatcher};
};

// Unknown command returns error code
TEST_F(DispatcherTest, UnknownCommandReturnsError) {
    cppsh::Command cmd;
    cmd.args = {"unknowncommand"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 1);
}

// Empty command returns 0
TEST_F(DispatcherTest, EmptyCommandReturnsZero) {
    cppsh::Command cmd;
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// cd with no arguments returns 0
TEST_F(DispatcherTest, CdNoArgsReturnsZero) {
    cppsh::Command cmd;
    cmd.args = {"cd"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// cd with valid path returns 0
TEST_F(DispatcherTest, CdValidPathReturnsZero) {
    cppsh::Command cmd;
    cmd.args = {"cd", "/tmp"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// cd with invalid path returns error
TEST_F(DispatcherTest, CdInvalidPathReturnsError) {
    cppsh::Command cmd;
    cmd.args = {"cd", "/this/path/does/not/exist"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 1);
}

// history with empty history returns 0
TEST_F(DispatcherTest, HistoryEmptyReturnsZero) {
    cppsh::Command cmd;
    cmd.args = {"history"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// history with entries returns 0
TEST_F(DispatcherTest, HistoryWithEntriesReturnsZero) {
    context.history = {"cd /tmp", "ls", "help"};
    cppsh::Command cmd;
    cmd.args = {"history"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// help returns 0
TEST_F(DispatcherTest, HelpReturnsZero) {
    cppsh::Command cmd;
    cmd.args = {"help"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// heLP returns 0 (case insensitive test)
TEST_F(DispatcherTest, HelpIReturnsZero) {
    cppsh::Command cmd;
    cmd.args = {"heLP"};
    EXPECT_EQ(dispatcher.dispatch(cmd, context), 0);
}

// get_entries returns non-empty list
TEST_F(DispatcherTest, GetEntriesNotEmpty) {
    EXPECT_FALSE(dispatcher.get_entries().empty());
}

// get_entries contains expected commands
TEST_F(DispatcherTest, GetEntriesContainsBuiltins) {
    const auto& entries = dispatcher.get_entries();
    auto has_command = [&](const std::string& name) {
        return std::any_of(entries.begin(), entries.end(), [&](const CommandEntry& e) {
            return e.name == name;
        });
    };
    EXPECT_TRUE(has_command("cd"));
    EXPECT_TRUE(has_command("history"));
    EXPECT_TRUE(has_command("help"));
    EXPECT_TRUE(has_command("exit"));
}