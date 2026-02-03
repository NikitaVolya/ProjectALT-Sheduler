#include <mariadb/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void read_table_test(MYSQL *conn) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    char name[256];
    unsigned long name_length;
    my_bool is_null;

    if ((stmt = mysql_stmt_init(conn)) == NULL) {
        fprintf(stderr, "Error while initialize mysql stmt\n");
        return;
    }
    
    if (mysql_stmt_prepare(stmt, "SELECT name FROM test;", 23)) {
        fprintf(stderr, "Error while preparing query\n");
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error while executing query\n");
        mysql_stmt_close(stmt);
        return;
    }
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type   = MYSQL_TYPE_STRING;
    bind[0].buffer        = name;
    bind[0].buffer_length = sizeof(name);
    bind[0].length        = &name_length;
    bind[0].is_null       = &is_null;

    if (mysql_stmt_bind_result(stmt, bind)) {
        fprintf(stderr, "Bind result failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_store_result(stmt)) {
        fprintf(stderr, "Store result failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    while (mysql_stmt_fetch(stmt) == 0) {
        name[name_length] = '\0';
        printf("Name: %s\n", name);
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);
}

int main() {
    MYSQL *conn;


    
    if (!(conn = mysql_init(0))) {
        fprintf(stderr, "unable to initialize connection struct\n");
        exit(1);
    }

    if (!mysql_real_connect(
        conn,
        "127.0.0.1",
        "app_user",
        "strong_password",
        "test",
        3306,
        NULL,
        0
    )) {
        printf("Connection failed: %s\n", mysql_error(conn));
        return 1;
    }
    
    printf("Connected succesfully!\n");

    read_table_test(conn);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
