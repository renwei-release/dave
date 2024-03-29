#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

NAME_PRODUCT=dave	# SYNC LOG ...
LEVEL_PRODUCT=alpha	# profiling alpha beta
FORM=BIN
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
OBJS := $(patsubst %.S,%.o,$(OBJS))

CC := gcc
CPLUS := g++

CFLAGS := -m64 -std=gnu99 -Wall -fPIC 
CFLAGS += -D__DAVE_CYGWIN__ -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT) -DFORM_PRODUCT_$(FORM)
CPLUSFLAGS := -m64 -std=c++11 -Wall -fPIC
CPLUSFLAGS += -D__DAVE_CYGWIN__ -D__DAVE_PRODUCT_$(NAME_PRODUCT)__ -DLEVEL_PRODUCT_$(MAKE_LEVEL_PRODUCT) -DFORM_PRODUCT_$(FORM)

ifeq ($(LEVEL_PRODUCT), beta)
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
%.o: %.S
	@echo $<
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

pre_build:
	python3 ../../../../Tools/refresh_version/refresh_version.py "../../../../" $(NAME_PRODUCT)
	../../../../Tools/auto_code/AUTOCODE > /dev/null

end_build:
	$(shell if [ ! -d $(DEPLOYPATH) ]; then mkdir -p $(DEPLOYPATH); fi)
	cp $(NAME_PRODUCT)-BIN.exe $(DEPLOYPATH)/$(NAME_PRODUCT)-BIN.exe
	mv $(NAME_PRODUCT)-BIN.exe ./$(shell echo $(NAME_PRODUCT) | tr A-Z a-z)/

clean_build:
	rm $(OBJS) $(NAME_PRODUCT).* -f