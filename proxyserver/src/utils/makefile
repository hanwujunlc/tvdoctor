
# Add inputs and outputs from these tool invocations to the build variables 
SRCS = $(wildcard ./src/utils/*.cpp)

CPP_SRCS += \
$(SRCS)

OBJS += \
$(patsubst %cpp,%o,$(SRCS))

CPP_DEPS += \
$(patsubst %cpp,%d,$(SRCS))


# Each subdirectory must supply rules for building sources it contributes
src/utils/%.o: ./src/utils/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I"./src/flatbuffers" -I"./src/utils" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


