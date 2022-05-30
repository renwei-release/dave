#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

NAME_PRODUCT=dave    # SYNC LOG ...
LEVEL_PRODUCT=alpha	# profiling alpha beta
DEPLOYPATH=../../../../Deploy/deploy/$(shell echo $(NAME_PRODUCT) | tr A-Z a-z)/project

ifeq ($(LEVEL_PRODUCT), profiling)
	MAKE_LEVEL_PRODUCT=alpha
else ifeq ($(LEVEL_PRODUCT), beta)
	MAKE_LEVEL_PRODUCT=beta
else
	MAKE_LEVEL_PRODUCT=alpha
endif

include SRC
include INC
include LIB

OBJS := $(patsubst %.c,%.o,$(SOURCES))
OBJS := $(patsubst %.cpp,%.o,$(OBJS))
OBJS := $(patsubst %.cu,%.o,$(OBJS))
OBJS := $(patsubst %.S,%.o,$(OBJS))

GCNO := $(patsubst %.c,%.gcno,$(SOURCES))
GCNO := $(patsubst %.cpp,%.gcno,$(GCNO))
GCNO := $(patsubst %.cu,%.gcno,$(GCNO))
GCNO := $(patsubst %.S,%.gcno,$(GCNO))
GCDA := $(patsubst %.c,%.gcda,$(SOURCES))
GCDA := $(patsubst %.cpp,%.gcda,$(GCDA))
GCDA := $(patsubst %.cu,%.gcda,$(GCDA))
GCDA := $(patsubst %.S,%.gcda,$(GCDA))

CC := gcc
CPLUS := g++
NVCC := /usr/local/cuda/bin/nvcc

CFLAGS := -m64 -std=gnu99 -Wall -fPIC -mcmodel=large -fopenmp -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS += -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT)
CPLUSFLAGS := -m64 -std=c++11 -fPIC -Wall -mcmodel=large -fopenmp -ffunction-sections -fdata-sections -Wl,--gc-sections
CPLUSFLAGS += -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT)
NVCCFLAGS := -std=c++11
NVCCFLAGS += -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT)

ifeq ($(LEVEL_PRODUCT), profiling)
	CFLAGS += -g -fprofile-arcs -ftest-coverage -pg -O3 -lc
	CPLUSFLAGS += -g -fprofile-arcs -ftest-coverage -pg -O3 -lc
else ifeq ($(LEVEL_PRODUCT), beta)
	CFLAGS += -O3
	CPLUSFLAGS += -O3
else
	CFLAGS += -g -O0
	CPLUSFLAGS += -g -O0
endif

$(NAME_PRODUCT)-BIN: $(OBJS)
	@$(CC) -o $@ $^ $(CFLAGS) $(LIB)
%.o: %.c
	@echo $<
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)
%.o: %.cpp
	@echo $<
	@$(CPLUS) -o $@ -c $< $(CPLUSFLAGS) $(INCLUDE)
%.o: %.cu
	@echo $<
	@$(NVCC) -o $@ -c $< $(NVCCFLAGS) $(INCLUDE)
%.o: %.S
	@echo $<
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

pre_build:
	python3 ../../../../Tools/refresh_version/refresh_version.py "../../../../" $(NAME_PRODUCT)
	../../../../Tools/auto_code/AUTOCODE > /dev/null

end_build:
	$(shell if [ ! -d $(DEPLOYPATH) ]; then mkdir -p $(DEPLOYPATH); fi)
	cp $(NAME_PRODUCT)-BIN $(DEPLOYPATH)/$(NAME_PRODUCT)-BIN
	mv $(NAME_PRODUCT)-BIN ./$(shell echo $(NAME_PRODUCT) | tr A-Z a-z)/

clean:
	rm $(OBJS) $(GCNO) $(GCDA) $(NAME_PRODUCT).* -f