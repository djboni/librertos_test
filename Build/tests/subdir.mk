################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/main.cpp \
../tests/test_Fifo.cpp \
../tests/test_Mutex.cpp \
../tests/test_OSevent.cpp \
../tests/test_OSlist.cpp \
../tests/test_Queue.cpp \
../tests/test_Scheduler.cpp \
../tests/test_Semaphore.cpp \
../tests/test_Timer.cpp \
../tests/test_func__OS_scheduler.cpp \
../tests/test_func__OS_schedulerUnlock.cpp \
../tests/test_func__OS_taskDelay.cpp 

OBJS += \
./tests/main.o \
./tests/test_Fifo.o \
./tests/test_Mutex.o \
./tests/test_OSevent.o \
./tests/test_OSlist.o \
./tests/test_Queue.o \
./tests/test_Scheduler.o \
./tests/test_Semaphore.o \
./tests/test_Timer.o \
./tests/test_func__OS_scheduler.o \
./tests/test_func__OS_schedulerUnlock.o \
./tests/test_func__OS_taskDelay.o 

CPP_DEPS += \
./tests/main.d \
./tests/test_Fifo.d \
./tests/test_Mutex.d \
./tests/test_OSevent.d \
./tests/test_OSlist.d \
./tests/test_Queue.d \
./tests/test_Scheduler.d \
./tests/test_Semaphore.d \
./tests/test_Timer.d \
./tests/test_func__OS_scheduler.d \
./tests/test_func__OS_schedulerUnlock.d \
./tests/test_func__OS_taskDelay.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -I"/home/djones/Documentos/Projetos/Posts/Boost.Test LibreRTOS/librertos_test/librertos" -I"/home/djones/Documentos/Projetos/Posts/Boost.Test LibreRTOS/librertos_test/tests" -O0 -g3 -pedantic -Wall -Wextra -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


