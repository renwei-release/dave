#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

VER_PRODUCT=dave    # SYNC LOG ...
LEVEL_PRODUCT=alpha	# profiling alpha beta
DEPLOYPATH=../../../Deploy/deploy/$(shell echo $(VER_PRODUCT) | tr A-Z a-z)

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

GCNO := $(patsubst %.c,%.gcno,$(SOURCES))
GCNO := $(patsubst %.cpp,%.gcno,$(GCNO))
GCNO := $(patsubst %.cu,%.gcno,$(GCNO))
GCDA := $(patsubst %.c,%.gcda,$(SOURCES))
GCDA := $(patsubst %.cpp,%.gcda,$(GCDA))
GCDA := $(patsubst %.cu,%.gcda,$(GCDA))

CC := gcc
CPLUS := g++
NVCC := /usr/local/cuda/bin/nvcc

CFLAGS := -m64 -std=gnu99 -Wall -fPIC -mcmodel=large -fopenmp -DDAVE_PRODUCT_$(VER_PRODUCT) -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT)
CPLUSFLAGS := -m64 -std=c++11 -fPIC -Wall -mcmodel=large -fopenmp -DDAVE_PRODUCT_$(VER_PRODUCT) -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT)
NVCCFLAGS := -std=c++11 -DDAVE_PRODUCT_$(VER_PRODUCT) -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT)

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

dave: $(OBJS)
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

pre_build:
	python ../../../Tools/refresh_version/refresh_version.py "../../../" $(VER_PRODUCT)
	../../../Tools/auto_code/AUTOCODE

end_build:
	./build-py-go-lib.sh $(MAKE_LEVEL_PRODUCT)

running_bin:
	../../../Tools/running_bin/running_bin ../../build/linux/dave NULL
	cp dave $(VER_PRODUCT)-BIN
	$(shell if [ ! -d $(DEPLOYPATH) ]; then mkdir -p $(DEPLOYPATH); fi)
	cp dave $(DEPLOYPATH)/$(VER_PRODUCT)-BIN

clean:
	rm $(OBJS) $(VER_PRODUCT).* -f

clean_gcno_gcda:
	rm $(GCNO) $(GCDA) -f

clean_bin:
	rm $(VER_PRODUCT).* -f