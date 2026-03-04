
#include "debug/role_model_debug.h"
#include "debug/worker_model_debug.h"
#include "debug/line_model_debug.h"


int main() {
    MYSQL *conn;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    printf(LINE_TEXT);

    run_worker_tests(conn);
    
    printf(LINE_TEXT);

    run_role_tests(conn);
    
    printf(LINE_TEXT);

    run_line_tests(conn);
    
    printf(LINE_TEXT);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
