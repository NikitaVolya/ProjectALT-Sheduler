
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "database/mysql_base_functions.h"


int main() {
    MYSQL *conn;
    REQUESTF_RESULT *requestf_r;
    unsigned int role_id, line_id, test;
    short count;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    test = 2;
    requestf_r = mysql_request_f_result(conn, 
        "SELECT line_id, role_id, count FROM line_role WHERE line_id = %ui", &test,
        MYSQL_BIND_UINT, MYSQL_BIND_UINT, MYSQL_BIND_SHORT);

    printf("Request DONE\n");

    while (requestf_result_fetch(requestf_r, &line_id, &role_id, &count) == 0) {
        printf("%d %d %d\n", line_id, role_id, count);
    }

    free_requestf_result(requestf_r);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
