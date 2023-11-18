TSTAMP := $(shell /bin/date "+%Y-%m-%d")
GLHASH := $(shell git log -1 --pretty=format:"%H")
GSHASH := $(shell git rev-parse --short HEAD)

CC = pdflatex
CFLAGS = -halt-on-error -shell-escape
SRC = UWB_Software_Guide.tex
JOBNAME = "$(TSTAMP)_${SRC%.tex}_$(GSHASH)"

all: document

document:
	$(CC) $(CFLAGS) $(SRC)
	$(CC) $(CFLAGS) $(SRC)
	if [ -f "${SRC%.tex}.bcf" ]; then \
		biber "${SRC%.tex}"; \
	fi
	$(CC) $(CFLAGS) $(SRC)

revision:
	$(CC) $(CFLAGS) -jobname="$(JOBNAME)" $(SRC)
	$(CC) $(CFLAGS) -jobname="$(JOBNAME)" $(SRC)
	if [ -f "${SRC%.tex}.bcf" ]; then \
		biber "$(TSTAMP)_presentation_$(GSHASH)"; \
	fi
	$(CC) $(CFLAGS) -jobname="$(JOBNAME)" $(SRC)

clean:
	find . -type d -name '.git' -prune -o -type f -name '*.aux' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.fdb_latexmk' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.bcf' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.bbl' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.blg' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.ent' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.idx' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.ilg' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.ind' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.lof' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.lot' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.log' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.nlo' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.nls' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.out' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.synctex.gz' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.tdo' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.thm' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.toc' -exec rm -f {} +
	find . -type d -name '.git' -prune -o -type f -name '*.run.xml' -exec rm -f {} +

.PHONY : all presentation revision clean
