include Rules.mk

#######################################
# list of source files
#######################################
C_SOURCES =                                         \
src/core/task.c                                     \
src/core/item_pool.c                                \
src/core/task_timer.c                               \
src/core/watcher.c                                  \
src/core/log.c                                      \
src/core/idle_task.c                                \
src/debug/hex_dump.c                                \
src/utils/bhash.c                                   \
src/utils/circ_buffer.c                             \
src/utils/cJSON.c                                   \
src/utils/config_reader.c                           \
src/utils/cmd_line_opt.c                            \
src/utils/sbuf.c                                    \
src/utils/trace.c                                   \
src/utils/request_queue.c                           \
src/net/tcp_server.c                                \
src/net/tcp_server_ipv4.c                           \
src/net/tcp_server_unix_domain.c                    \
src/net/tcp_connector.c                             \
src/net/tcp_auto_connector.c                        \
src/net/sock_util.c                                 \
src/net/udp_socket.c                                \
src/serial/serial.c                                 \
src/stream/stream.c                                 \
src/cli/cli.c                                       \
src/cli/cli_telnet.c                                \
src/cli/cli_serial.c                                \
src/protocols/telnet_reader.c                       \
src/protocols/modbus/modbus_crc.c                   \
src/protocols/modbus/modbus_util.c                  \
src/protocols/modbus/modbus_func_coils.c            \
src/protocols/modbus/modbus_func_discrete.c         \
src/protocols/modbus/modbus_func_holding.c          \
src/protocols/modbus/modbus_func_input.c            \
src/protocols/modbus/modbus_rtu_request_handler.c   \
src/protocols/modbus/modbus_rtu_slave.c             \
src/protocols/modbus/modbus_tcp_slave.c             \
src/protocols/modbus/mbap_reader.c                  \
src/protocols/modbus/modbus_master.c                \
src/protocols/modbus/modbus_rtu_master.c            \
src/protocols/modbus/modbus_tcp_master.c            \
src/kongja_express.c                                \
src/demo/tcp_server_test.c                          \
src/demo/tcp_client_test.c                          \
src/demo/cli_test.c                                 \
src/demo/modbus_slave_test.c                        \
src/demo/modbus_master_test.c                       \
src/demo/demo.c

#######################################
# custom preprocessor defines
#######################################
C_DEFS  = -DTRACE_ENABLED

#######################################
# include and lib setup
#######################################
C_INCLUDES =                              \
-Ilibev/libev/install_dir/include         \
-Iinc                                     \
-Iinc/modbus

LIBS = -lpthread -lev -lm
LIBDIR = -Llibev/libev/install_dir/lib

#######################################
# for verbose output
#######################################
# Prettify output
V = 0
ifeq ($V, 0)
  Q = @
  P = > /dev/null
else
  Q =
  P =
endif

#######################################
# build directory and target setup
#######################################
BUILD_DIR = build
TARGET    = kongjaexpress

#######################################
# compile & link flags
#######################################
CFLAGS += $(C_DEFS) $(C_INCLUDES)

# Generate dependency information
CFLAGS += -MMD -MF .dep/$(*F).d 

LDFLAGS +=  $(LIBDIR) $(LIBS)

#######################################
# build target
#######################################
all: $(BUILD_DIR)/$(TARGET)

#######################################
# build rules
#######################################
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]					$(notdir $<)"
	$Q$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	@echo "[LD]					$(TARGET)"
	$Q$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR):
	@echo "MKDIR					$(BUILD_DIR)"
	$Qmkdir $@

#######################################
# clean up
#######################################
clean:
	@echo "[CLEAN]					$(TARGET) $(BUILD_DIR) .dep"
	$Q-rm -fR .dep $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
