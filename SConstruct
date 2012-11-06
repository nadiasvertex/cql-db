from glob import glob
import os

toolchain_path = os.path.join(os.getcwd(), "toolchain", "tools")

# Add the path to clang
path = ":".join([os.environ['PATH'],
                 "/store/workspace/build/Release+Asserts/bin",
                 os.path.join(toolchain_path, "bin")])

libs = ['zmq', 'protobuf', 'jit', 'jitplus', 'pthread', 'rt']

env = Environment(
    ENV={
    	'PATH'    : path,
    },
    CPPPATH=['toolchain/tools/include', 'src/lib', 'build', 'tests', '.'],
    tools=['default', 'protoc']
)

#env.Replace(CC="clang")
#env.Replace(CXX="clang++")

env.Append(CXXFLAGS="-std=c++11 -g")
env.Append(LIBPATH=['.', os.path.join(toolchain_path, "lib")])
env['ENV']['TERM'] = os.environ['TERM']

edge_proto = env.Protoc(
	   [],
	   glob("src/lib/edge/proto/*.proto"),
	   PROTOCPROTOPATH=["src/lib"],
	   PROTOCOUTDIR="build"
)


env.Library("edge", glob("build/edge/proto/*.cc") + \
                    glob("src/lib/edge/cpp/*.cpp"))

env.Library("plane", glob("src/lib/plane/cpp/*.cpp"))

env.Library("cell", glob("src/lib/cell/cpp/*.cpp"))

env.Library("processor", glob("src/lib/processor/cpp/*.cpp"))

env.Program("group",
            glob("src/group/*.cpp"),
       LIBS=['plane', 'edge', 'cell', 'processor'] + libs
)

env.Program("lattice_test",
            ["tests/gtest/gtest-all.cc",
             "tests/gtest/gtest_main.cc"] + glob("tests/*.cpp"),
	    LIBS=['plane', 'edge', 'cell', 'processor'] + libs
)
