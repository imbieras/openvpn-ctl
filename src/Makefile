GFLAGS := -Wall -Wextra -pedantic -std=c99 -O2
CFLAGS := $(GFLAGS)
SOURCES := $(wildcard *.c)
OBJS := $(SOURCES:.c=.o)
BIN := openvpn-ctl
LDFLAGS := -lblobmsg_json -lubox -lubus

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS)
