SUBDIRS = library test

all: subdirs

subdirs:
	for dir in $(SUBDIRS); do\
	  $(MAKE) -C $$dir || break; \
	done

clean:
	for dir in $(SUBDIRS); do\
	  $(MAKE) -C $$dir clean; \
	done
