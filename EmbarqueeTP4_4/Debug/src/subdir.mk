################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/EmbarqueeTP4_4.c \
../src/algo.c \
../src/buffer_circulaire.c \
../src/buffer_circulaire_partagee.c 

OBJS += \
./src/EmbarqueeTP4_4.o \
./src/algo.o \
./src/buffer_circulaire.o \
./src/buffer_circulaire_partagee.o 

C_DEPS += \
./src/EmbarqueeTP4_4.d \
./src/algo.d \
./src/buffer_circulaire.d \
./src/buffer_circulaire_partagee.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	sparc-gaisler-rtems5-gcc -I"C:\Users\lenny\eclipse-workspace\windows-producer" -O0 -g3 -Wall -c -fmessage-length=0 -qbsp=gr712rc -msoft-float -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


