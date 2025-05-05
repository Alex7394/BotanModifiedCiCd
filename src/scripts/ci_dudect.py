import os
import subprocess
import sys
import shutil

# Konfiguration
DUDECT_REPO = "https://github.com/oreparaz/dudect.git"
DUDECT_DIR = "dudect"
BOTAN_INC = os.path.abspath("build/include")
BOTAN_LIB = os.path.abspath("build/lib")
CT_TEST_CPP = os.path.abspath("source/src/ct_test.cpp")
CT_TEST_BIN = "ct_test_bin"

def run(cmd, cwd=None, env=None, check=True):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, env=env, check=check)
    return result.returncode

def install_dependencies():
    print("Installing prerequisites (requires sudo)...")
    run(["sudo", "apt-get", "update"])
    run(["sudo", "apt-get", "install", "-y", "util-linux", "coreutils", "build-essential", "git"])

def clone_dudect():
    if not os.path.isdir(DUDECT_DIR):
        print("Cloning dudect repository...")
        run(["git", "clone", DUDECT_REPO])
    else:
        print("Dudect already cloned.")

def build_dudect():
    print("Building dudect...")
    run(["make"], cwd=DUDECT_DIR)

def compile_ct_test():
    print("Compiling ct_test.cpp...")
    dudect_inc = os.path.abspath(f"{DUDECT_DIR}/src")
    cmd = [
        "g++", "-std=c++20", "-O2",
        f"-I{BOTAN_INC}", f"-I{dudect_inc}",
        CT_TEST_CPP, "-o", CT_TEST_BIN,
        f"-L{BOTAN_LIB}", "-lbotan-3", "-pthread", "-lm"
    ]
    run(cmd)

def execute_ct_test():
    print("Executing ct_test...")
    try:
        run(["taskset", "-c", "2", "timeout", "300s", f"./{CT_TEST_BIN}"])
        print("PASSED")
    except subprocess.CalledProcessError:
        print("FAILED")
        sys.exit(1)

def main():
    install_dependencies()
    clone_dudect()
    build_dudect()
    compile_ct_test()
    execute_ct_test()

if __name__ == "__main__":
    main()
