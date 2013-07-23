# Specify compiler to be used
COMPILER = m32c-elf-gcc

# Specify ar tool to be used
ARCHIVER = m32c-elf-ar

# Debug flags if any
DEBUGGING = #-O0 -g

# Warning flags
WARNING = -Wpacked -Wall

# Add aditional CFLAGS if any 
ADDITIONAL_CFLAGS = -mcpu=m16c -falign-functions=2 -nostartfiles -DROMSTART

# Whether to include the grammar generation module in the build
INCLUDE_GRAMMAR_GENERATION = false

# In case INCLUDE_GRAMMAR_GENERATION equals true; whether to
# support parsing of schema-mode EXI encoded XML Schemas
INCLUDE_SCHEMA_EXI_GRAMMAR_GENERATION = false