################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/EmbarqueeTP4_3.c \
../src/algo.c 

OBJS += \
./src/EmbarqueeTP4_3.o \
./src/algo.o 

C_DEPS += \
./src/EmbarqueeTP4_3.d \
./src/algo.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	sparc-gaisler-rtems5-gcc -I"C:\Users\lenny\eclipse-workspace\windows-producer" -O0 -g3 -Wall -c -fmessage-length=0 -qbsp=gr712rc -msoft-float -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


