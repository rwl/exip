# Specify compiler to be used
COMPILER = gcc

# Specify ar tool to be used
ARCHIVER = ar

# Debug flags if any
DEBUGGING = -O0 -g #-fno-stack-protector

# Warning flags
WARNING = -Wpacked -Wall

# Add aditional CFLAGS if any 
ADDITIONAL_CFLAGS = 

# Whether to include the grammar generation module in the build
INCLUDE_GRAMMAR_GENERATION = true