################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librertos/LibreRTOS.c \
../librertos/LibreRTOS_state.c \
../librertos/fifo.c \
../librertos/mutex.c \
../librertos/queue.c \
../librertos/semaphore.c \
../librertos/timer.c 

OBJS += \
./librertos/LibreRTOS.o \
./librertos/LibreRTOS_state.o \
./librertos/fifo.o \
./librertos/mutex.o \
./librertos/queue.o \
./librertos/semaphore.o \
./librertos/timer.o 

C_DEPS += \
./librertos/LibreRTOS.d \
./librertos/LibreRTOS_state.d \
./librertos/fifo.d \
./librertos/mutex.d \
./librertos/queue.d \
./librertos/semaphore.d \
./librertos/timer.d 


# Each subdirectory must supply rules for building sources it contributes
librertos/%.o: ../librertos/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -std=c90 -I"/home/djones/Documentos/Projetos/Posts/Boost.Test LibreRTOS/librertos_test/librertos" -I"/home/djones/Documentos/Projetos/Posts/Boost.Test LibreRTOS/librertos_test/tests" -O0 -g3 -pedantic -Wall -Wextra -c -fmessage-length=0 --coverage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


