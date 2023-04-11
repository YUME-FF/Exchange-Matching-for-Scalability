CC = g++
CFLAGS = -I/cptl -Wall -std=c++11
LDFLAGS = -lpqxx -lpq -pthread

SRCDIR = .
OBJDIR = .
BINDIR = .

SOURCES = $(SRCDIR)/exchangeMatching.cpp \
          $(SRCDIR)/main.cpp \
          $(SRCDIR)/query_funcs.h \
          $(SRCDIR)/query_funcs.cpp \
          $(SRCDIR)/server.h \
          $(SRCDIR)/sqlHelper.h \
          $(SRCDIR)/sqlHelper.cpp \
          $(SRCDIR)/XMLParser.cpp \
          $(SRCDIR)/XMLParser.h \
          $(SRCDIR)/pugixml/pugixml.cpp

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TARGET = $(BINDIR)/main

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
