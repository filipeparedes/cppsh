#include <gtest/gtest.h>
#include "dispatcher.hpp"
#include "errors/shell_error.hpp"
#include "context.hpp"
#include "pipeline.hpp"
#include <algorithm>

class DispatcherTest : public ::testing::Test {
protected:
    Dispatcher dispatcher;
    ShellContext context{dispatcher};

    cppsh::Pipeline make_pipeline(std::vector<std::string> args) {
        cppsh::Pipeline pl;
        cppsh::Command cmd;
        cmd.args = args;
        pl.cmds.push_back(cmd);
        return pl;
    }
};

TEST_F(DispatcherTest, UnknownCommandThrows) {
    cppsh::Pipeline pl = make_pipeline({"unknowncommand"});
    EXPECT_THROW(dispatcher.dispatch(pl, context), ShellError);
}

TEST_F(DispatcherTest, EmptyCommandReturnsZero) {
    cppsh::Pipeline pl;
    EXPECT_EQ(dispatcher.dispatch(pl, context), 0);
}

TEST_F(DispatcherTest, CdInvalidPathThrows) {
    cppsh::Pipeline pl = make_pipeline({"cd", "/this/path/does/not/exist"});
    EXPECT_THROW(dispatcher.dispatch(pl, context), ShellError);
}

TEST_F(DispatcherTest, CdValidPathReturnsZero) {
    cppsh::Pipeline pl = make_pipeline({"cd", "/tmp"});
    EXPECT_EQ(dispatcher.dispatch(pl, context), 0);
}

TEST_F(DispatcherTest, HistoryEmptyReturnsZero) {
    cppsh::Pipeline pl = make_pipeline({"history"});
    EXPECT_EQ(dispatcher.dispatch(pl, context), 0);
}

TEST_F(DispatcherTest, HistoryWithEntriesReturnsZero) {
    context.history = {"cd /tmp", "ls", "help"};
    cppsh::Pipeline pl = make_pipeline({"history"});
    EXPECT_EQ(dispatcher.dispatch(pl, context), 0);
}

TEST_F(DispatcherTest, HelpReturnsZero) {
    cppsh::Pipeline pl = make_pipeline({"help"});
    EXPECT_EQ(dispatcher.dispatch(pl, context), 0);
}

TEST_F(DispatcherTest, HelpIReturnsZero) {
    cppsh::Pipeline pl = make_pipeline({"heLP"});
    EXPECT_EQ(dispatcher.dispatch(pl, context), 0);
}

TEST_F(DispatcherTest, GetEntriesNotEmpty) {
    EXPECT_FALSE(dispatcher.get_entries().empty());
}

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