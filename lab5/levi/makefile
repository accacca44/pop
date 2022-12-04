FLAGS := -g -Wall -lrt -I ./src/protocol
# -Werror -Wextra

DEBUG_FOLDER := debug
SRC_FOLDER := src

$(DEBUG_FOLDER)/sz: $(SRC_FOLDER)/sz.cpp $(SRC_FOLDER)/protocol/messages.h
	g++ $(FLAGS) $< -o $@

$(DEBUG_FOLDER)/kl: $(SRC_FOLDER)/kl.cpp $(SRC_FOLDER)/protocol/messages.h
	g++ $(FLAGS) $< -o $@


.PHONY: all
all: $(DEBUG_FOLDER)/sz $(DEBUG_FOLDER)/kl

.PHONY: clean
clean:
	rm -rf $(DEBUG_FOLDER)/*
 


#  <===============RUN===============>
.PHONY: run_sz
run_sz: $(DEBUG_FOLDER)/sz
	./$<

.PHONY: run_kl
run_kl: $(DEBUG_FOLDER)/kl
	./$< "lab*"