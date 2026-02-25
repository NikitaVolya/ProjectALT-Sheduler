
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "database/role_model.h"


int main() {
    MYSQL *conn;

    conn = mysql_create_connection("127.0.0.1", 3306, "test", "app_user", "strong_password");
    
    mysql_close(conn);
    
    exit(EXIT_SUCCESS);
}
