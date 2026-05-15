#include <gtest/gtest.h>
#include "shell_error.hpp"

class ShellErrorTest : public ::testing::Test {};

// ShellError is thrown and caught correctly
TEST_F(ShellErrorTest, InvalidPathThrows) {
    EXPECT_THROW(
        throw ShellError(ShellErrorCode::INVALID_PATH, "cd", "/naoexiste"),
        ShellError
    );
}

TEST_F(ShellErrorTest, CommandNotFoundThrows) {
    EXPECT_THROW(
        throw ShellError(ShellErrorCode::COMMAND_NOT_FOUND, "sleeep"),
        ShellError
    );
}

TEST_F(ShellErrorTest, ForkFailedThrows) {
    EXPECT_THROW(
        throw ShellError(ShellErrorCode::FORK_FAILED),
        ShellError
    );
}

// ShellError is not thrown when no error
TEST_F(ShellErrorTest, NoThrowOnSuccess) {
    EXPECT_NO_THROW(
        ShellError(ShellErrorCode::INVALID_PATH, "cd", "/tmp")
    );
}