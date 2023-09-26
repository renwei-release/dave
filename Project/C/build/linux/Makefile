#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

NAME_PRODUCT=dave	# SYNC LOG ...
name_PRODUCT=$(shell echo $(NAME_PRODUCT) | tr A-Z a-z)
LEVEL_PRODUCT=alpha	# profiling alpha beta
FORM=BIN
DEPLOYPATH=../../../../Deploy/deploy/$(name_PRODUCT)/file_system/project

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
OBJS := $(patsubst %.cc,%.o,$(OBJS))
OBJS := $(patsubst %.S,%.o,$(OBJS))
totalOBJS := $(patsubst %.c,%.o,$(totalSOURCES))
totalOBJS := $(patsubst %.cpp,%.o,$(totalOBJS))
totalOBJS := $(patsubst %.cc,%.o,$(totalOBJS))
totalOBJS := $(patsubst %.S,%.o,$(totalOBJS))

CC := gcc
CPLUS := g++

CFLAGS := -fPIC -Wall -Wno-missing-braces -fopenmp -ffunction-sections -fdata-sections -Wl,--gc-sections
CFLAGS += -D__DAVE_LINUX__ -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT) -DFORM_PRODUCT_$(FORM)
CPLUSFLAGS := -fPIC -Wall -Wno-missing-braces -fopenmp -ffunction-sections -fdata-sections -Wl,--gc-sections
CPLUSFLAGS += -D__DAVE_LINUX__ -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT) -DFORM_PRODUCT_$(FORM)

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
%.o: %.cc
	@echo $<
	@$(CPLUS) -o $@ -c $< $(CPLUSFLAGS) $(INCLUDE)
%.o: %.S
	@echo $<
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

pre_build:
	python3 ../../../../Tools/refresh_version/refresh_version.py "../../../../" $(NAME_PRODUCT)
	chmod a+x ../../../../Tools/auto_code/AUTOCODE
	../../../../Tools/auto_code/AUTOCODE

end_build:
	$(shell if [ ! -d $(DEPLOYPATH) ]; then mkdir -p $(DEPLOYPATH); fi)
	cp $(NAME_PRODUCT)-BIN $(DEPLOYPATH)/$(NAME_PRODUCT)-BIN
	mv $(NAME_PRODUCT)-BIN ./$(shell echo $(NAME_PRODUCT) | tr A-Z a-z)/

clean_build:
	rm $(totalOBJS) $(NAME_PRODUCT).* -f
	chmod a+x ../../../../Tools/auto_code/AUTOCLEAN
	../../../../Tools/auto_code/AUTOCLEAN