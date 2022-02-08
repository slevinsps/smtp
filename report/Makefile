# Report name
REPORT_NAME = report.pdf

# Compiler
CC = gcc
STD = -std=gnu99

# Directories
UTILS_DIR = ./utils
DOT_DIR = ./dot
TEX_DIR = ./tex
TEX_REGEXP_DIR = $(TEX_DIR)/regexp
IMG_DIR = ./images
SRC_SRV_DIR = ./../source/server
PYTHON2_ENV_DIR = ./report_env

# Utilities
MAKE2DOT = $(UTILS_DIR)/makefile2dot
CFLOW2DOT = $(UTILS_DIR)/cflow2dot
FSM2DOT = $(UTILS_DIR)/fsm2dot
RE2TEX = $(UTILS_DIR)/re2tex
PDFLATEX = pdflatex -interaction=nonstopmode

# Files
RE_FILE = $(SRC_SRV_DIR)/server-re.h
TEX_FILES = $(wildcard $(TEX_DIR)/*.tex)

# TARGES:

all: clean make_fsm_image make_cflow make_makedot report

report: $(TEX_FILES)
	cd $(TEX_DIR) && $(PDFLATEX) report.tex && $(PDFLATEX) report.tex && cp report.pdf ..

clean:
	rm -rf report.pdf $(DOT_DIR)/serverfsm_def.dot $(IMG_DIR)/serverfsm.png $(IMG_DIR)/cflow.png $(IMG_DIR)/makefile_report.png $(IMG_DIR)/makefile_server.png $(IMG_DIR)/makefile_common.png

.PHONY: tex_subdirs
tex_subdirs:
	mkdir -p $(TEX_REGEXP_DIR)


make_fsm_image:  
	$(FSM2DOT) ./../server/autogen/server.def |dot -Tpng  > $(IMG_DIR)/serverfsm.png; \

SOURCES_PATH = ../server/
SOURCES = state_handlers.c main.c client_info.c maildir.c mail.c server.c autogen/server-fsm.c help_funcs.c

make_cflow:
	cflow --level "0= " $(addprefix $(SOURCES_PATH),$(SOURCES))  | grep -v -f cflow.ignore | $(CFLOW2DOT) | dot -Tpng > ./images/cflow.png

make_makedot:
	python $(MAKE2DOT) < Makefile  |dot -Tpng > ./images/makefile_report.png &&\
	python $(MAKE2DOT) < ../server/Makefile  |dot -Tpng > ./images/makefile_server.png &&\
	python $(MAKE2DOT) < ../Makefile  |dot -Tpng > ./images/makefile_common.png


# TEX FILES GENERATION:

./tex/regexp/smtp_re.tex: ./../source/server/reg_exprs.h
	$(RE2TEX) ./../source/server/reg_exprs.h $(TEX_REGEXP_DIR)
