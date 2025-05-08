#!/bin/bash

# Set the path to the test folder and filenames
TEST_DIR="tests"
TEST_FILENAME="ct_test.cpp"
TEST_DURATION="300s"
CT_TEST="src/$TEST_FILENAME"

# Botan Build Pfade – anpassen falls abweichend
BOTAN_INC="$HOME/botan/build/include/public"
BOTAN_LIB="$HOME/botan/build/lib"

DUDECT_INC="$HOME/dudect/src"

# Output binary name
OUTPUT_BINARY="ct_test_bin"

# Compile
g++ -std=c++20 -O2 -DDUDECT_IMPLEMENTATION \
    -I"$BOTAN_INC" \
    -I"$DUDECT_INC" \
    "$CT_TEST" \
    -o "$OUTPUT_BINARY" \
    -L"$BOTAN_LIB" -lbotan-3 -pthread -lm

# Check
if [ $? -ne 0 ]; then
  echo "Compilation failed."
  exit 1
fi

echo "Compilation successful. Running test..."

taskset -c 2 timeout "$TEST_DURATION" ./"$OUTPUT_BINARY"
RETURN_STATUS=$?
if [ $RETURN_STATUS -eq 0 ]; then
  echo "No timing vulnerability detected within the specified time."
  exit 0
elif [ $RETURN_STATUS -eq 124 ]; then
  echo "No timing vulnerability detected (timeout reached)."
  exit 0
else
  echo "Test failed. Timing vulnerability detected (exit code $RETURN_STATUS)."
  exit 1
fi

