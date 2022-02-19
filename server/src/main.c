#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ini.h"
#include "server.h"
#include "tree.h"

#define HELP \
    "server -c <path> [-p <port>] [-d] [-h]\n\n" \
    "-c <path>\tServer configuration file path\n" \
    "-p <port>\tSMTP server port (default: 465)\n" \
    "-d\t\tDaemonize process\n" \
    "-h\t\tPrint this help\n"

static int read_config(dict_t* config, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        return -1;
    }
    int ret = ini_read_file_to_dict(config, file);
    fclose(file);
    return ret;
}

int main(int argc, char* argv[]) {
    dict_t config;
    const char* config_path = NULL;
    int port = 465;
    int port_from_args = 0;
    int daemonize = 0;
    int opt = 0;

    dict_init(&config);

    while ((opt = getopt(argc, argv, "c:hp:d")) != -1) {
        switch (opt) {
            case 'c': {  // config file option
                config_path = optarg;
                break;
            }
            case 'p': {  // port option
                port = atoi(optarg);
                port_from_args = 1;
                break;
            }
            case 'd': {  // daemonize option
                daemonize = 1;
                break;
            }
            case 'h':    // help option
            case '?': {  // unknown option
                puts(HELP);
                exit(0);
            }
        }
    }
    if (!config_path || read_config(&config, config_path) != 0) {
        puts(HELP);
        exit(-1);
    }
    if (!port_from_args) {
        char *port_str = NULL;
        if (dict_get(&config, "port", (void**)&port_str) == 0) {
            port = atoi(port_str);
        }
    }

    int ret = server_start(port, &config);

    // Clear config
    tree_node_t* node = NULL;
    tree_foreach(config.root, node) {
        free(node->value);
    }
    dict_free(&config);

    return ret;
}

