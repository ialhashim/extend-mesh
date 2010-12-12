################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
src/cg.cpp 

CC_SRCS += \
src/compcol_double.cc \
src/comprow_double.cc \
src/coord_double.cc \
src/diagpre_double.cc \
src/icpre_double.cc \
src/ilupre_double.cc \
src/iohb_double.cc \
src/iotext_double.cc \
src/mvblasd.cc \
src/mvblasf.cc \
src/mvblasi.cc \
src/mvmd.cc \
src/mvmf.cc \
src/mvmi.cc \
src/mvvcio.cc \
src/mvvd.cc \
src/mvvdio.cc \
src/mvvf.cc \
src/mvvi.cc \
src/qsort_double.cc \
src/qsort_int.cc \
src/spmm.cc \
src/spsm.cc 

OBJS += \
src/cg.o \
src/compcol_double.o \
src/comprow_double.o \
src/coord_double.o \
src/diagpre_double.o \
src/icpre_double.o \
src/ilupre_double.o \
src/iohb_double.o \
src/iotext_double.o \
src/mvblasd.o \
src/mvblasf.o \
src/mvblasi.o \
src/mvmd.o \
src/mvmf.o \
src/mvmi.o \
src/mvvcio.o \
src/mvvd.o \
src/mvvdio.o \
src/mvvf.o \
src/mvvi.o \
src/qsort_double.o \
src/qsort_int.o \
src/spmm.o \
src/spsm.o 

CC_DEPS += \
src/compcol_double.d \
src/comprow_double.d \
src/coord_double.d \
src/diagpre_double.d \
src/icpre_double.d \
src/ilupre_double.d \
src/iohb_double.d \
src/iotext_double.d \
src/mvblasd.d \
src/mvblasf.d \
src/mvblasi.d \
src/mvmd.d \
src/mvmf.d \
src/mvmi.d \
src/mvvcio.d \
src/mvvd.d \
src/mvvdio.d \
src/mvvf.d \
src/mvvi.d \
src/qsort_double.d \
src/qsort_int.d \
src/spmm.d \
src/spsm.d 

CPP_DEPS += \
src/cg.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"iml" -I"include" -O3 -Wall -c -fmessage-length=0 -fopenmp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"iml" -I"include" -O3 -Wall -c -fmessage-length=0 -fopenmp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


