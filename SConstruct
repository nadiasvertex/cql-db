from glob import glob
import os

# Add the path to clang
path = os.environ['PATH'] + ":/store/workspace/build/Release+Asserts/bin" 

env = Environment(ENV = {
    'PATH'    : path,
})

env.Replace(CC = "clang")
env.Replace(CXX = "clang++")
env.Append(CPPPATH = ['.', 'tests'])
env.Append(CXXFLAGS = "-std=c++11")
env.Append(LIBS = 'pthread')
env['ENV']['TERM'] = os.environ['TERM']

env.Program("lattice_test", ["tests/gtest/gtest-all.cc",
                             "tests/gtest/gtest_main.cc"] + glob("tests/*.cpp"))
