
#include "debug/role_model_debug.h"


int main() {
    MYSQL *conn;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    run_role_tests(conn);
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
