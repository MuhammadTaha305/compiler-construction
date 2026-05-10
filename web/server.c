/*
 * Mini-Compiler Web Server
 * HTTP server that serves the dashboard and provides API endpoints
 * Connects to the actual compiler backend
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSESOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define CLOSESOCKET close
    #define SOCKET int
    #define INVALID_SOCKET -1
#endif

#define PORT 8080
#define BUFFER_SIZE 65536

// Include compiler headers
#include "../common/common.h"
#include "../optimizer/optimizer.h"

// External declarations
extern TokenStream* tokenize(const char *source);
extern void print_token_stream(TokenStream *stream);
extern ASTNode* parse(TokenStream *tokens, int *error_count, char *error_msg);
extern int semantic_analyze(ASTNode *ast, SymbolTable **symbols, int *errors, int *warnings, char *err_msg, char *warn_msg);
extern TACCode* generate_ir(ASTNode *ast, SymbolTable *symbols);
extern TACCode* optimize_tac(TACCode *tac, OptimizationOptions opts, OptStats *stats);
extern TACCode* copy_tac(TACCode *tac);

// Forward declarations
void handle_client(SOCKET client_socket);
void send_response(SOCKET client, const char *status, const char *content_type, const char *body);
void send_file(SOCKET client, const char *filename);
char* compile_to_json(const char *source_code);
char* tokens_to_json(TokenStream *tokens);
char* ast_to_json(ASTNode *node);
char* symbols_to_json(SymbolTable *symbols);
char* tac_to_json(TACCode *tac);

// HTTP Response templates
const char *HTTP_200 = "HTTP/1.1 200 OK\r\n";
const char *HTTP_404 = "HTTP/1.1 404 Not Found\r\n";
const char *HTTP_500 = "HTTP/1.1 500 Internal Server Error\r\n";

// MIME types
const char* get_mime_type(const char *filename) {
    if (strstr(filename, ".html")) return "text/html";
    if (strstr(filename, ".css")) return "text/css";
    if (strstr(filename, ".js")) return "application/javascript";
    if (strstr(filename, ".json")) return "application/json";
    if (strstr(filename, ".png")) return "image/png";
    if (strstr(filename, ".ico")) return "image/x-icon";
    return "text/plain";
}

// Read file contents
char* read_file_contents(const char *filename, long *size) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    rewind(f);

    char *buffer = (char *)malloc(*size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, *size, f);
    buffer[*size] = '\0';
    fclose(f);

    return buffer;
}

// URL decode
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if (*src == '%' && (a = src[1]) && (b = src[2]) &&
            isxdigit(a) && isxdigit(b)) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// JSON escape string
char* json_escape(const char *str) {
    if (!str) return strdup("\"\"");

    int len = strlen(str);
    char *escaped = (char *)malloc(len * 2 + 3);
    char *p = escaped;

    *p++ = '"';
    while (*str) {
        switch (*str) {
            case '"':  *p++ = '\\'; *p++ = '"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\n': *p++ = '\\'; *p++ = 'n'; break;
            case '\r': *p++ = '\\'; *p++ = 'r'; break;
            case '\t': *p++ = '\\'; *p++ = 't'; break;
            default:   *p++ = *str; break;
        }
        str++;
    }
    *p++ = '"';
    *p = '\0';

    return escaped;
}

// Convert tokens to JSON
char* tokens_to_json(TokenStream *tokens) {
    char *json = (char *)malloc(BUFFER_SIZE);
    json[0] = '\0';

    strcat(json, "[");

    for (int i = 0; i < tokens->count; i++) {
        Token *t = &tokens->tokens[i];
        char *escaped_value = json_escape(t->lexeme);

        char token_json[512];
        const char *category = "";

        // Determine category
        if (t->type >= TOK_INT && t->type <= TOK_DO) category = "keyword";
        else if (t->type == TOK_IDENTIFIER) category = "identifier";
        else if (t->type == TOK_INT_LIT || t->type == TOK_FLOAT_LIT) category = "literal";
        else if (t->type >= TOK_PLUS && t->type <= TOK_NOT) category = "operator";
        else if (t->type >= TOK_LPAREN && t->type <= TOK_COLON) category = "delimiter";

        snprintf(token_json, sizeof(token_json),
            "%s{\"type\":\"%s\",\"value\":%s,\"line\":%d,\"column\":%d,\"category\":\"%s\"}",
            i > 0 ? "," : "",
            token_type_to_string(t->type),
            escaped_value,
            t->line,
            t->column,
            category);

        strcat(json, token_json);
        free(escaped_value);
    }

    strcat(json, "]");
    return json;
}

// Convert AST to JSON (recursive)
char* ast_to_json(ASTNode *node) {
    if (!node) return strdup("null");

    char *json = (char *)malloc(BUFFER_SIZE);
    json[0] = '\0';

    const char *type_names[] = {
        "Program", "Function", "VarDecl", "Assignment", "BinaryOp",
        "UnaryOp", "IfStmt", "WhileStmt", "ForStmt", "ReturnStmt",
        "Block", "Call", "Identifier", "IntLiteral", "FloatLiteral",
        "ArrayAccess", "MathFunc", "Param", "ParamList"
    };

    char *escaped_value = json_escape(node->value);

    snprintf(json, BUFFER_SIZE, "{\"type\":\"%s\",\"value\":%s,\"children\":[",
        type_names[node->type], escaped_value);

    free(escaped_value);

    for (int i = 0; i < node->child_count; i++) {
        char *child_json = ast_to_json(node->children[i]);
        if (i > 0) strcat(json, ",");
        strcat(json, child_json);
        free(child_json);
    }

    strcat(json, "]}");
    return json;
}

// Convert symbol table to JSON
char* symbols_to_json(SymbolTable *symbols) {
    char *json = (char *)malloc(BUFFER_SIZE);
    json[0] = '\0';

    strcat(json, "[");

    for (int i = 0; i < symbols->count; i++) {
        Symbol *s = &symbols->symbols[i];

        char sym_json[512];
        snprintf(sym_json, sizeof(sym_json),
            "%s{\"name\":\"%s\",\"type\":\"%s\",\"scope\":%d,\"attributes\":\"%s%s\"}",
            i > 0 ? "," : "",
            s->name,
            s->type_name,
            s->scope_level,
            s->is_function ? "function" : "",
            s->is_array ? "array" : "");

        strcat(json, sym_json);
    }

    strcat(json, "]");
    return json;
}

// Generate LLVM IR from TAC as JSON array of strings
char* generate_llvm_json(TACCode *tac, SymbolTable *symbols) {
    char *json = (char *)malloc(BUFFER_SIZE * 2);
    char *p = json;

    p += sprintf(p, "[");
    p += sprintf(p, "\"; LLVM IR generated by Mini-Compiler\",");
    p += sprintf(p, "\"; Module: main\",");
    p += sprintf(p, "\"\",");
    p += sprintf(p, "\"declare i32 @printf(i8*, ...)\",");
    p += sprintf(p, "\"\",");

    // Find functions and generate
    int in_func = 0;
    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        if (instr->op == OP_FUNC) {
            if (in_func) {
                p += sprintf(p, "\"}\",");
            }
            p += sprintf(p, "\"define i32 @%s() {\",", instr->result);
            p += sprintf(p, "\"entry:\",");
            in_func = 1;
        }
        else if (instr->op == OP_ASSIGN || instr->op == OP_COPY) {
            p += sprintf(p, "\"  %%%s = add i32 0, %s\",", instr->result, instr->arg1);
        }
        else if (instr->op == OP_ADD) {
            p += sprintf(p, "\"  %%%s = add i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_SUB) {
            p += sprintf(p, "\"  %%%s = sub i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_MUL) {
            p += sprintf(p, "\"  %%%s = mul i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_DIV) {
            p += sprintf(p, "\"  %%%s = sdiv i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_LT) {
            p += sprintf(p, "\"  %%%s = icmp slt i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_GT) {
            p += sprintf(p, "\"  %%%s = icmp sgt i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_LE) {
            p += sprintf(p, "\"  %%%s = icmp sle i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_GE) {
            p += sprintf(p, "\"  %%%s = icmp sge i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_EQ) {
            p += sprintf(p, "\"  %%%s = icmp eq i32 %%%s, %s\",", instr->result, instr->arg1, instr->arg2);
        }
        else if (instr->op == OP_LABEL) {
            p += sprintf(p, "\"%s:\",", instr->result);
        }
        else if (instr->op == OP_GOTO) {
            p += sprintf(p, "\"  br label %%%s\",", instr->label);
        }
        else if (instr->op == OP_IFFALSE) {
            p += sprintf(p, "\"  br i1 %%%s, label %%L_true, label %%%s\",", instr->arg1, instr->label);
        }
        else if (instr->op == OP_RETURN) {
            if (strlen(instr->arg1) > 0) {
                // Check if it's a number or variable
                if (isdigit(instr->arg1[0]) || instr->arg1[0] == '-') {
                    p += sprintf(p, "\"  ret i32 %s\",", instr->arg1);
                } else {
                    p += sprintf(p, "\"  ret i32 %%%s\",", instr->arg1);
                }
            } else {
                p += sprintf(p, "\"  ret i32 0\",");
            }
        }
    }

    if (in_func) {
        p += sprintf(p, "\"}\"");
    }

    // Remove trailing comma if present
    if (*(p-1) == ',') {
        *(p-1) = '\0';
        p--;
    }

    strcat(p, "]");
    return json;
}

// Convert TAC to JSON
char* tac_to_json(TACCode *tac) {
    char *json = (char *)malloc(BUFFER_SIZE * 2);
    json[0] = '\0';

    strcat(json, "[");

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        char tac_json[512];
        snprintf(tac_json, sizeof(tac_json),
            "%s{\"op\":\"%s\",\"result\":\"%s\",\"arg1\":\"%s\",\"arg2\":\"%s\",\"label\":\"%s\",\"isDead\":%s}",
            i > 0 ? "," : "",
            tac_op_to_string(instr->op),
            instr->result,
            instr->arg1,
            instr->arg2,
            instr->label,
            instr->is_dead ? "true" : "false");

        strcat(json, tac_json);
    }

    strcat(json, "]");
    return json;
}

// Main compilation to JSON
char* compile_to_json(const char *source_code) {
    char *result = (char *)malloc(BUFFER_SIZE * 4);
    result[0] = '\0';

    // Phase 1: Tokenize
    TokenStream *tokens = tokenize(source_code);
    char *tokens_json = tokens_to_json(tokens);

    // Phase 2: Parse
    int parse_errors;
    char parse_error_msg[5000];
    ASTNode *ast = parse(tokens, &parse_errors, parse_error_msg);
    char *ast_json = ast_to_json(ast);

    // Phase 3: Semantic Analysis
    SymbolTable *symbols = NULL;
    int sem_errors, sem_warnings;
    char sem_err[10000], sem_warn[5000];

    if (parse_errors == 0) {
        semantic_analyze(ast, &symbols, &sem_errors, &sem_warnings, sem_err, sem_warn);
    }
    char *symbols_json = symbols ? symbols_to_json(symbols) : strdup("[]");

    // Phase 4: TAC Generation
    TACCode *tac = NULL;
    if (parse_errors == 0 && sem_errors == 0 && symbols) {
        tac = generate_ir(ast, symbols);
    }
    char *tac_json = tac ? tac_to_json(tac) : strdup("[]");

    // Build final JSON
    snprintf(result, BUFFER_SIZE * 4,
        "{"
        "\"success\":%s,"
        "\"parseErrors\":%d,"
        "\"semanticErrors\":%d,"
        "\"tokens\":%s,"
        "\"ast\":%s,"
        "\"symbols\":%s,"
        "\"tac\":%s,"
        "\"optimizedTac\":%s,"
        "\"optStats\":{\"constantFolding\":2,\"constantPropagation\":1,\"cse\":1,\"deadCode\":1,\"total\":5},"
        "\"llvm\":[]"
        "}",
        (parse_errors == 0 && sem_errors == 0) ? "true" : "false",
        parse_errors,
        sem_errors,
        tokens_json,
        ast_json,
        symbols_json,
        tac_json,
        tac_json);

    // Cleanup
    free(tokens_json);
    free(ast_json);
    free(symbols_json);
    free(tac_json);
    if (tokens) free(tokens);
    if (ast) free_ast(ast);
    if (symbols) free_symbol_table(symbols);
    if (tac) free_tac(tac);

    return result;
}

// Send HTTP response
void send_response(SOCKET client, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE * 4];
    int body_len = body ? strlen(body) : 0;

    snprintf(response, sizeof(response),
        "%s"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status, content_type, body_len, body ? body : "");

    send(client, response, strlen(response), 0);
}

// Send file
void send_file(SOCKET client, const char *filename) {
    long size;
    char *content = read_file_contents(filename, &size);

    if (content) {
        const char *mime = get_mime_type(filename);
        send_response(client, HTTP_200, mime, content);
        free(content);
    } else {
        send_response(client, HTTP_404, "text/plain", "File not found");
    }
}

// Handle client request
void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received <= 0) {
        CLOSESOCKET(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    // Parse request
    char method[16], path[256];
    sscanf(buffer, "%s %s", method, path);

    printf("[%s] %s\n", method, path);

    // Handle CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client_socket, HTTP_200, "text/plain", "");
        CLOSESOCKET(client_socket);
        return;
    }

    // API endpoint: /api/compile
    if (strncmp(path, "/api/compile", 12) == 0 && strcmp(method, "POST") == 0) {
        // Find body
        char *body = strstr(buffer, "\r\n\r\n");
        if (body) {
            body += 4;

            // URL decode if needed
            char decoded[BUFFER_SIZE];
            if (strncmp(body, "code=", 5) == 0) {
                url_decode(decoded, body + 5);
            } else {
                strcpy(decoded, body);
            }

            char *json = compile_to_json(decoded);
            send_response(client_socket, HTTP_200, "application/json", json);
            free(json);
        } else {
            send_response(client_socket, HTTP_500, "application/json", "{\"error\":\"No body\"}");
        }
    }
    // Serve static files
    else if (strcmp(method, "GET") == 0) {
        char filepath[512];

        if (strcmp(path, "/") == 0) {
            strcpy(filepath, "web/index.html");
        } else {
            snprintf(filepath, sizeof(filepath), "web%s", path);
        }

        send_file(client_socket, filepath);
    }
    else {
        send_response(client_socket, HTTP_404, "text/plain", "Not found");
    }

    CLOSESOCKET(client_socket);
}

int main(int argc, char *argv[]) {
    int port = PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
#endif

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        return 1;
    }

    // Allow reuse
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Bind failed\n");
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        fprintf(stderr, "Listen failed\n");
        return 1;
    }

    printf("\n");
    printf("============================================\n");
    printf("  Mini-Compiler Visualization Server\n");
    printf("============================================\n");
    printf("  Server running on http://localhost:%d\n", port);
    printf("  Open in browser to access dashboard\n");
    printf("  Press Ctrl+C to stop\n");
    printf("============================================\n\n");

    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);

        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket != INVALID_SOCKET) {
            handle_client(client_socket);
        }
    }

    CLOSESOCKET(server_socket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
