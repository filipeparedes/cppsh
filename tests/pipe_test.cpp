#include <gtest/gtest.h>
#include "parser.hpp"

class PipeTest : public ::testing::Test {
protected:
    cppsh::Parser parser;
};

// simple pipe without extra arguments
TEST_F(PipeTest, SimpleTwoStagePipe) {
    cppsh::Pipeline pl = parser.parse("ls | grep");
    
    ASSERT_EQ(pl.cmds.size(), 2);
    
    // first command
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    
    // second command
    ASSERT_EQ(pl.cmds[1].args.size(), 1);
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
}

// pipe with arguments
TEST_F(PipeTest, PipeWithArguments) {
    cppsh::Pipeline pl = parser.parse("ls -la | grep txt");
    
    ASSERT_EQ(pl.cmds.size(), 2);
    
    // first command
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[0].args[1], "-la");
    
    // second command
    ASSERT_EQ(pl.cmds[1].args.size(), 2);
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[1].args[1], "txt");
}

// multiple pipes in a row
TEST_F(PipeTest, MultiplePipesChain) {
    cppsh::Pipeline pl = parser.parse("cat names.txt | grep silva | wc -l");
    
    ASSERT_EQ(pl.cmds.size(), 3);
    
    // first command
    ASSERT_EQ(pl.cmds[0].args.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "cat");
    
    // second command
    ASSERT_EQ(pl.cmds[1].args.size(), 2);
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    
    // third command
    ASSERT_EQ(pl.cmds[2].args.size(), 2);
    EXPECT_EQ(pl.cmds[2].args[0], "wc");
    EXPECT_EQ(pl.cmds[2].args[1], "-l");
}
// pipe with multiple spaces
TEST_F(PipeTest, PipeWithExcessiveSpaces) {
    cppsh::Pipeline pl = parser.parse("ls   -la    |      grep      cpp");
    
    ASSERT_EQ(pl.cmds.size(), 2);
    EXPECT_EQ(pl.cmds[0].args[0], "ls");
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[1].args[1], "cpp");
}

// mix pipes and io redirection
TEST_F(PipeTest, PipeMixedWithIORedirection) {
    cppsh::Pipeline pl = parser.parse("sort < input.txt | grep erro > output.txt");
    
    ASSERT_EQ(pl.cmds.size(), 2);
    
    // first command io and args
    EXPECT_EQ(pl.cmds[0].input_file, "input.txt");
    ASSERT_EQ(pl.cmds[0].args.size(), 1);
    EXPECT_EQ(pl.cmds[0].args[0], "sort");
    
    // second command io and args
    EXPECT_EQ(pl.cmds[1].output_file, "output.txt");
    ASSERT_EQ(pl.cmds[1].args.size(), 2);
    EXPECT_EQ(pl.cmds[1].args[0], "grep");
    EXPECT_EQ(pl.cmds[1].args[1], "erro");
}