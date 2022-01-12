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
FSM2DOT = $(UTILS_DIR)/fsm2dot
RE2TEX = $(UTILS_DIR)/re2tex
PDFLATEX = pdflatex -interaction=nonstopmode

# Files
RE_FILE = $(SRC_SRV_DIR)/server-re.h
TEX_FILES = $(wildcard $(TEX_DIR)/*.tex)

# TARGES:

all: clean report

report: $(TEX_FILES)
	cd $(TEX_DIR) && $(PDFLATEX) report.tex && $(PDFLATEX) report.tex && cp report.pdf ..

clean:
	rm -rf report.pdf

.PHONY: tex_subdirs
tex_subdirs:
	mkdir -p $(TEX_REGEXP_DIR)


# GENERATE SMTP FSM IMAGES

# .def -> _def.dot
./dot/serverfsm_def.dot: ./../source/server/autogen/server.def
	$(FSM2DOT) $< > $@

./dot/serverfsm_report_def.dot: ./../source/server/autogen/server_report.def
	$(FSM2DOT) $< > $@

.PHONY: generate_fsm_images
generate_fsm_images:
	 dot -Tpng $(DOT_DIR)/serverfsm_def.dot > $(IMG_DIR)/serverfsm.png; \
	 dot -Tpng $(DOT_DIR)/serverfsm_report_def.dot > $(IMG_DIR)/serverfsm_report.png


# TEX FILES GENERATION:

./tex/regexp/smtp_re.tex: ./../source/server/rexprs.h
	$(RE2TEX) ./../source/server/rexprs.h $(TEX_REGEXP_DIR)
