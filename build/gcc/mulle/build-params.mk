# Specify compiler to be used
COMPILER = m32c-elf-gcc

# Debug flags if any
DEBUGGING = #-O0 -g

# Warning flags
WARNING = -Wpacked -Wall

# Add aditional CFLAGS if any 
ADDITIONAL_CFLAGS = -mcpu=m16c -falign-functions=2 -nostartfiles -DROMSTART -Imulle -D__SIZE_MAX__=-1

# Check Unit Test installation directory
CHECK_UNIT_TEST_DIR = /usr/local/lib

# Whether to include the grammar generation module in the build
INCLUDE_GRAMMAR_GENERATION = false