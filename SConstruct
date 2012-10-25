from glob import glob
import os

toolchain_path = os.path.join(os.getcwd(), "toolchain", "tools")

# Add the path to clang
path = ":".join([os.environ['PATH'],
                 "/store/workspace/build/Release+Asserts/bin",
                 os.path.join(toolchain_path, "bin")])

env = Environment(ENV = {
    'PATH'    : path,
})

env.Replace(CC = "clang")
env.Replace(CXX = "clang++")
env.Append(CPPPATH = ['.', 'tests'])
env.Append(CXXFLAGS = "-std=c++11")
env.Append(LIBS = 'pthread')
env.Append(LIBPATH=os.path.join(toolchain_path, "lib"))
env['ENV']['TERM'] = os.environ['TERM']

env.Program("lattice_test", ["tests/gtest/gtest-all.cc",
                             "tests/gtest/gtest_main.cc"] + glob("tests/*.cpp"))
