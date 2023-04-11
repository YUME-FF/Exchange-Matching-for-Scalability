CC = g++
CFLAGS = -Wall -std=c++11
LDFLAGS = -lpqxx -lpq -lpthread

SRCDIR = .
OBJDIR = .
BINDIR = .

SCALABILITY_SOURCES = $(SRCDIR)/scalability.cpp \
                      $(SRCDIR)/clientSocket.h \
                      $(SRCDIR)/sqlHelper.h \
                      $(SRCDIR)/myexception.h \
                      $(SRCDIR)/sqlHelper.cpp

FUNCTIONALITY_SOURCES = $(SRCDIR)/functionality.cpp \
                        $(SRCDIR)/clientSocket.h \
                        $(SRCDIR)/sqlHelper.h \
                        $(SRCDIR)/myexception.h \
                        $(SRCDIR)/sqlHelper.cpp

SCALABILITY_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SCALABILITY_SOURCES))

FUNCTIONALITY_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(FUNCTIONALITY_SOURCES))

SCALABILITY_TARGET = $(BINDIR)/scalability
FUNCTIONALITY_TARGET = $(BINDIR)/functionality

all: $(SCALABILITY_TARGET) $(FUNCTIONALITY_TARGET)

$(SCALABILITY_TARGET): $(SCALABILITY_OBJECTS)
	$(CC) $(CFLAGS) $(SCALABILITY_OBJECTS) -o $(SCALABILITY_TARGET) $(LDFLAGS)

$(FUNCTIONALITY_TARGET): $(FUNCTIONALITY_OBJECTS)
	$(CC) $(CFLAGS) $(FUNCTIONALITY_OBJECTS) -o $(FUNCTIONALITY_TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o $(SCALABILITY_TARGET) $(FUNCTIONALITY_TARGET)
