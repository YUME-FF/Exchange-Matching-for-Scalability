CC = g++
CFLAGS = -Wall -std=c++11
LDFLAGS = -lpqxx -lpq

SRCDIR = .
OBJDIR = .
BINDIR = .

SOURCES = $(SRCDIR)/client.cpp \
          $(SRCDIR)/clientSocket.h \
          $(SRCDIR)/sqlHelper.h \
		  $(SRCDIR)/myexception.h \
          $(SRCDIR)/sqlHelper.cpp

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TARGET = $(BINDIR)/client

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
