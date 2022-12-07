################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/EmbarqueeTP3_3.c \
../src/algo.c \
../src/modifier_registre.c 

OBJS += \
./src/EmbarqueeTP3_3.o \
./src/algo.o \
./src/modifier_registre.o 

C_DEPS += \
./src/EmbarqueeTP3_3.d \
./src/algo.d \
./src/modifier_registre.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	sparc-gaisler-elf-gcc -I"C:\Users\lenny\eclipse-workspace\windows-producer" -O0 -g3 -Wall -c -fmessage-length=0 -msoft-float -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


