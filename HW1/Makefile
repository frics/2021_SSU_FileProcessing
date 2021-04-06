CC     = gcc #-g
CCOPTS = -c
LINK   = gcc #-g

SOURCE      = _src.txt
DESTINATION = _dst.txt
ANSWER		= _ans.txt

COPY      = copy
READ      = read
MERGE     = merge
OVERWRITE = overwrite
INSERT    = insert
DELETE    = delete
ALL = $(COPY) $(READ) $(MERGE) $(OVERWRITE) $(INSERT) $(DELETE)

OFFSET = 4
DATA   = QWER
BYTES  = 4
#################################################

.SUFFIXES : .C .c .o

.c.o : 
	$(CC) $(CCOPTS) $<

$(COPY) : copy.o
	$(LINK) -o $(COPY) copy.c
	./$(COPY) $(COPY)$(SOURCE) $(COPY)$(DESTINATION)

$(READ) : read.o
	$(LINK) -o $(READ) read.o
	./$(READ) $(READ)$(SOURCE) $(OFFSET) $(BYTES) > $(READ)$(DESTINATION)

$(MERGE) : merge.o
	$(LINK) -o $(MERGE) merge.o
	./$(MERGE) $(MERGE)$(ANSWER) $(MERGE)$(SOURCE) $(MERGE)$(DESTINATION)

$(OVERWRITE) : overwrite.o
	$(LINK) -o $(OVERWRITE) overwrite.o
	./$(OVERWRITE) $(OVERWRITE)$(SOURCE) $(OFFSET) $(DATA)

$(INSERT) : insert.o
	$(LINK) -o $(INSERT) insert.o
	./$(INSERT) $(INSERT)$(SOURCE) $(OFFSET) $(DATA)

$(DELETE) : delete.o
	$(LINK) -o $(DELETE) delete.o
	./$(DELETE) $(DELETE)$(SOURCE) $(OFFSET) $(BYTES)

clean :
	rm *.txt
	rm *.o
	rm $(ALL) 
