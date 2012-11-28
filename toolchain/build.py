#!/usr/bin/python
import argparse
import os
import sys


libraries = ( 
  ("apr-1.4.6",
      ("./configure --prefix={BINDIR} --disable-shared --enable-static",
       "make", "make install")),
             
  ("apr-util-1.5.1",
      ("./configure --prefix={BINDIR} --with-apr={BINDIR}",
       "make", "make install")),
        
  ("apache-log4cxx-0.10.0",
      ("./configure --prefix={BINDIR} --disable-shared --enable-static --disable-doxygen --with-apr={BINDIR} --with-apr-util={BINDIR}",
       "make", "make install")),
 
  ("protobuf-2.4.1",
      ("./configure --prefix={BINDIR}",
       "make", "make install")),
 
  ("zeromq-3.2.1",
      ("./configure --prefix={BINDIR} --with-pgm --disable-shared --enable-static",
       "make", "make install")),
 
  ("libjit-0.1.2",
      ("rm -fr {BINDIR}/include/jit/",
       "./configure --prefix={BINDIR} --disable-shared --enable-static",
       "cd tools", "make", "make install", "cd ..",
       "cd jit", "make", "make install", "cd ..",
       "cd jitplus", "make", "make install", "cd ..",
       "cd jitdynamic", "make", "make install", "cd ..",
       "mkdir -p {BINDIR}/include/jit/",
       "cp -fr include/jit/*.h {BINDIR}/include/jit/")),
             
  ("openssl-1.0.1c",
      ("./config --prefix={BINDIR} no-shared ",
       "make", "make install")),
 
 
  ("zmqcpp",
      ("cp -f zmq.hpp {BINDIR}/include")),
 
  ("pegtl-0.31",
      ("cp -fr include/* $BINDIR/include"))
)

def build(path, cmds):
    print(path)
    
    cur_dir = os.getcwd()
    os.chdir(path)
    
    try:
        for cmd in cmds:
            processed = cmd.format(BINDIR=BINDIR)
            if args.print_cmd:
                print processed
            else:
                if os.system(processed)!=0:
                    sys.exit(1)
    finally:
        os.chdir(cur_dir)
        

BINDIR = os.path.join(os.getcwd(), "tools")

parser = argparse.ArgumentParser(description='Build the toolchain')
parser.add_argument('--list', dest='list', action='store_true',
                   default=False,
                   help='show the library dependencies needed by the toolchain')
parser.add_argument('--build-all', dest='build', action='store_true',
                   default=False,
                   help='build all dependencies')
parser.add_argument('--build', dest='build_one', action='append',
                   default=[], choices=[l[0] for l in libraries],
                   help='build a single dependency')
parser.add_argument('--print', dest='print_cmd', action='store_true',
                   default=False,
                   help="print the commands, don't actually run them")
parser.add_argument('--clean', dest='clean', action='store_false',
                   default=False,
                   help='if building only one dependency, clean up')


args = parser.parse_args()

if not os.path.exists(BINDIR):
    os.make_dirs(BINDIR)

if args.list:
    print("="*60)
    print("Libraries".center(60))
    print("="*60)
    
    for l in libraries:
        print(" %s" % l[0])
    print("\n")
    sys.exit(0)

if args.build_one:
    d = dict(libraries)
    for lib in args.build_one:
        build(lib, d[lib])
        
    if args.clean:
        os.system("hg clean -X toolchain/tools/*")        
    sys.exit(0)
        
for path, cmds in libraries:
    build(path, cmds)

# Cleanup
os.system("hg clean -X toolchain/tools/*")
