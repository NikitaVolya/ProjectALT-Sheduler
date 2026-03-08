
#include "worker_work_day_model_debug.h"
#include "role_model_debug.h"
#include "worker_model_debug.h"
#include "line_model_debug.h"


int main() {
    MYSQL *conn;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");

    
    printf(LINE_TEXT);

    run_worker_work_day_tests(conn);
    
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
