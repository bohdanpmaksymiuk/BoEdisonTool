# Executable Name
TARGET := BoET
OBJ1 := boMAX.o BoET.o boGPIO.o boVoltage.o edison-9dof-i2c.o

C_FLAGS := -g -Wall -Wextra
CC 		:= gcc
RM 		:= rm
LINKFLAGS := -lm -rdynamic /usr/local/lib/libmraa.so.1.7.0

.PHONY: $(TARGET)
.PHONY: clean

VPATH:= ./src/ ./obj/ ./include/

# Path for .c , .h and .o Files
SRC_PATH := ./src/
OBJ_PATH := ./obj/
INC_PATH := -I ./include
OBJ := $(patsubst %,$(OBJ_PATH)%,$(OBJ1))

# Build .o first
$(OBJ_PATH)%.o: $(SRC_PATH)%.c
	@echo [CC] $<
	@$(CC) $(C_FLAGS) -o $@ -c $< $(INC_PATH)

# Build final Binary
$(TARGET):      $(OBJ)
	@echo [INFO] Creating Binary Executable [$(TARGET)]
	@$(CC) -o $@ $^ $(LINKFLAGS)

# Clean all the object files and the binary
clean:
	@echo "[Cleaning]"
	@$(RM) -rfv $(OBJ_PATH)*
	@$(RM) -rfv $(TARGET)
