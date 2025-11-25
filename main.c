#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <ctype.h>

typedef struct user
{
    int id;
    char *username; // creates an array username and points at username[0], we can not access it directly, we use malloc and define the length we need for that array.
    char *password; // hased(encrypted)
    char *role;
} user;

typedef struct usernode
{
    user userinfo;         // userinfo is off datatype user .
    struct usernode *next; // this points to the next variable of userode datatype.
} usernode;

int userid_callback(void *data, int argc, char **argv, char **colName);
int user_callback(void *data, int argc, char **argv, char **colName);
int staff_callback(void *data, int argc, char **argv, char **colName);
int doctor_callback(void *data, int argc, char **argv, char **colName);
int count_callback(void *data, int argc, char **argv, char **colName);

int patient_view_callback(void *data, int argc, char **argv, char **colName);
int doctor_view_callback(void *data, int argc, char **argv, char **colName);
int staff_view_callback(void *data, int argc, char **argv, char **colName);

int createTable(sqlite3 *db);

user display_login(sqlite3 *db);
user registerUser(sqlite3 *db);
user loginUser(sqlite3 *db);
user nullUser();

int staff_Menu(sqlite3 *db, user login);
int doctor_Menu(sqlite3 *db, user login);
int admin_Menu(sqlite3 *db, user login);

void clearScreen();
void clearInputBuffer();
void freeUserList(usernode *userlist);

int edit_staff(sqlite3 *db);
int edit_patient(sqlite3 *db);
int edit_doctor(sqlite3 *db);

int view_staff(sqlite3 *db);
int view_patient(sqlite3 *db);
int view_doctor(sqlite3 *db);

int add_patient(sqlite3 *db);

int userid_callback(void *data, int argc, char **argv, char **colName){
    int* id = (int*)data;
    if (argv[0]){
        *id = atoi(argv[0]);
    }
    else{
        *id = -1;
    }
    return 0;

}

int user_callback(void *data, int argc, char **argv, char **colName){
    // in data we can pass db on which we will works
    // argc returns number of outputs
    // argv returns values
    // column names like id, username.

    usernode **head = (usernode **)data; // we can say that if a pointer is pointing to the start of our list so head is pointing at that pointer

    if (argv == NULL)
    {
        return 0;
    }

    usernode *newnode = malloc(sizeof(usernode));
    newnode->userinfo.username = malloc(strlen(argv[1]) + 1);
    newnode->userinfo.password = malloc(strlen(argv[2]) + 1);
    newnode->userinfo.role = malloc(strlen(argv[3]) + 1);

    strcpy(newnode->userinfo.username, argv[1]);
    strcpy(newnode->userinfo.password, argv[2]);
    strcpy(newnode->userinfo.role, argv[3]);

    newnode->userinfo.id = atoi(argv[0]);
    newnode->next = NULL;

    if (*head == NULL)
    {
        *head = newnode;
    }
    else
    {
        usernode *temp = *head;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = newnode;
    }

    return 0;
}

int staff_callback(void *data, int argc, char **argv, char **colName){
    char (*staffInfo)[50] = data; // staffinfo s a pointer pointing to [0][50]

    if (argv[1] == NULL)
    {
        strcpy(staffInfo[0], "Not assigned");
    }
    else
    {
        strcpy(staffInfo[0], argv[1]);
    }

    if (argv[2] == NULL)
    {
        strcpy(staffInfo[1], "0.0");
    }
    else
    {
        strcpy(staffInfo[1], argv[2]);
    }

    if (argv[3] == NULL)
    {
        strcpy(staffInfo[2], "Not assigned");
    }
    else
    {
        strcpy(staffInfo[2], argv[3]);
    }
    return 0;
}

int doctor_callback(void *data, int argc, char **argv, char **colName)
{
    char (*doctorInfo)[50] = data;

    if (argv[1] == NULL)
    {
        strcpy(doctorInfo[0], "Not assigned");
    }
    else
    {
        strcpy(doctorInfo[0], argv[1]);
    }

    if (argv[2] == NULL)
    {
        strcpy(doctorInfo[1], "10-12");
    }
    else
    {
        strcpy(doctorInfo[1], argv[2]);
    }
    if (argv[0] == NULL)
    {
        strcpy(doctorInfo[2], "-1");
    }
    else
    {
        strcpy(doctorInfo[2], argv[0]);
    }

    return 0;
}

int count_callback(void *data, int argc, char **argv, char **colName)
{
    int *Count = (int *)data;

    *Count = atoi(argv[0]);
    return 0;
}

int patient_view_callback(void *data, int argc, char **argv, char **colName){

    printf("Patient #: %s\n", argv[0]);
    printf("Name: %s\n", argv[1]);
    printf("Status: %s\n", argv[2]);
    printf("Balance: %s\n\n\n", argv[3]);
    return 0;
}

int staff_view_callback(void *data, int argc, char **argv, char **colName) {
    
    printf("Staff Id #: %s\n", argv[0]);
    printf("job: %s\n", argv[1]);
    printf("Salary: %s\n", argv[2]);
    printf("shift: %s\n\n\n", argv[3]);

    return 0;
}

int doctor_view_callback(void *data, int argc, char **argv, char **colName){
    printf("Doctor Id # %s:\n", argv[0]);
    printf("Speciality: %s\n", argv[1]);
    printf("Timings: %s\n\n", argv[2]);
    return 0;
}

sqlite3 *createDatabase(){
    // createDatabase is a function and it will return sqlite3*, sqlite3 is a database connection, * means it will point to database connection.
    sqlite3 *db;
    int rc = sqlite3_open("hospital.db", &db); // database connectionis created and the address is stored in db
    if (rc != SQLITE_OK)                       // SQLITE_OK's value is 0, indicates successful operation;
    {
        fprintf(stderr, "Error1: Unable to open Database: %s\n", sqlite3_errmsg(db)); // sqlite3_errmsg(db) will print a human readable message > %s, easier to detect problem.
        return NULL;
    }
    return db;
}

int main(){
    sqlite3 *Hospital = createDatabase(); // this will store an
    if (Hospital == NULL)
    {

        return 1;
    }
    printf("Database opened successfully\n");

    int rc = createTable(Hospital);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(Hospital);
        return 2;
    }
    // program start

    user login = display_login(Hospital);
    if (login.id == -1)
    {
        printf("Error in account creation/ login\n");
        sqlite3_close(Hospital);
        return 1;
    }

    if (login.role == NULL)
    {
        printf("Fatal Error: Login succeeded but role not set.\n");
        sqlite3_close(Hospital);
        return 3;
    }

    if (strcmp(login.role, "Staff") == 0)
    {
        int val = staff_Menu(Hospital, login);
        if (val != SQLITE_OK)
        {
            printf("Database Error");
            return 1;
        }
    }

    if (strcmp(login.role, "Doctor") == 0)
    {
        int v = doctor_Menu(Hospital, login);
        if (v != SQLITE_OK)
        {
            printf("Database Error");
            return 1;
        }
    }

    if (strcmp(login.role, "Admin") == 0)
    {
        printf("Admin Menu \n");
        int value = admin_Menu(Hospital, login);
        if (value != SQLITE_OK)
        {
            printf("Database Error");
            return 1;
        }
    }

    // program end
    sqlite3_close(Hospital);

    return 0;
}

int createTable(sqlite3 *db){
    const char *statement =
        "CREATE TABLE IF NOT EXISTS USER("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE ,"
        "password TEXT NOT NULL,"
        "role TEXT CHECK(role IN('Doctor','Staff', 'Admin'))"
        ");"

        "CREATE TABLE IF NOT EXISTS PATIENTS("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "status BOOLEAN,"
        "balance REAL"
        ");"

        "CREATE TABLE IF NOT EXISTS DOCTOR_PATIENT("
        "id_patient INTEGER,"
        "id_doctor INTEGER,"
        "FOREIGN KEY (id_doctor) REFERENCES USER(id) ON DELETE CASCADE,"
        "FOREIGN KEY (id_patient) REFERENCES PATIENTS(id) ON DELETE CASCADE,"
        "PRIMARY KEY (id_patient, id_doctor)"
        ");"

        "CREATE TABLE IF NOT EXISTS STAFF("
        "id INTEGER PRIMARY KEY,"
        "job TEXT,"
        "salary REAL,"
        "shift TEXT CHECK(shift IN('Day','Night')),"
        "FOREIGN KEY (id) REFERENCES USER(id) ON DELETE CASCADE"
        ");"

        "CREATE TABLE IF NOT EXISTS DOCTOR("
        "id INTEGER PRIMARY KEY,"
        "speciality TEXT,"
        "timing TEXT,"
        "FOREIGN KEY (id) REFERENCES USER(id) ON DELETE CASCADE"
        ");";

    char *error; // itwill point to first position of error array.
    int rc = sqlite3_exec(db, statement, NULL, NULL, &error);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", error);
        sqlite3_free(error);
        return rc;
    }
    printf("Tables created successfully\n");
    return SQLITE_OK;
}

user display_login(sqlite3 *db){
    char option;

    printf("SELECT ONE OF THE FOLLOWING(A or B): \n");
    printf("[A]: Login \n");
    printf("[B]: Register \n");
    scanf(" %c", &option);
    option = toupper(option);

    while (option != 'A' && option != 'B')
    {
        clearScreen();
        printf("%c is not a valid option\n", option);
        printf("SELECT ONE OF THE FOLLOWING(A or B): \n");
        printf("[A]: Login \n");
        printf("[B]: Register \n");
        scanf(" %c", &option);
        option = toupper(option);
    }

    if (option == 'A')
    {
        user login = loginUser(db);
        return login;
    }
    else
    {
        user login = registerUser(db);
        return login;
    }
}

user loginUser(sqlite3 *db){

    char name[50], validation, password[20];
    int loginint = 0;
    while (loginint == 0)
    {

        validation = 'N';
        while (validation != 'Y')
        {

            printf("Enter username (without space):\n");
            scanf("%49s", name); // limit to prevent overflow
            clearInputBuffer();

            printf("%s", name);
            printf(" Is this your Username? [Y/N]: ");

            validation = getchar();
            clearInputBuffer();
            validation = toupper(validation);

            while (validation != 'Y' && validation != 'N')
            {
                clearScreen();
                printf("%c is not a valid input \n", validation);
                printf("%s", name);
                printf(" Is this your Username? [Y/N]: ");

                validation = getchar();
                clearInputBuffer();
                validation = toupper(validation);
            }
        }

        printf("Enter password: ");
        scanf("%s", password);
        clearInputBuffer();

        clearScreen();

        usernode *userlist = NULL; // userlist is a pointer of usernode datatype that is pointing nowhere at the moment.

        char *errMsg = 0;
        char *sql = "SELECT id,username,password,role FROM user;";
        int rc = sqlite3_exec(db, sql, user_callback, &userlist, &errMsg);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
            freeUserList(userlist);
            return nullUser();
        }

        usernode *temp = userlist;
        while (temp != NULL)
        {
            if (strcmp(temp->userinfo.username, name) == 0)
            {
                if (strcmp(temp->userinfo.password, password) == 0)
                {
                    user login;
                    login.id = temp->userinfo.id;

                    login.username = malloc(strlen(temp->userinfo.username) + 1);
                    strcpy(login.username, temp->userinfo.username);

                    login.password = malloc(strlen(temp->userinfo.password) + 1);
                    strcpy(login.password, temp->userinfo.password);

                    login.role = malloc(strlen(temp->userinfo.role) + 1);
                    strcpy(login.role, temp->userinfo.role);

                    loginint = 1;
                    printf("LOGIN SUCCESSFUL: Logged in as: %s \n", login.username);
                    freeUserList(userlist);
                    return login;
                }
            }
            temp = temp->next;
        }
        if (loginint != 1)
            printf("LOGIN FAILED! \n");
    }
}

user registerUser(sqlite3 *db){

    char name[50], password[20], conPassword[20], validation;

    validation = 'N';
    while (validation != 'Y')
    {

        printf("Enter username (without space):\n");
        scanf("%49s", name); // limit to prevent overflow
        clearInputBuffer();

        clearScreen();
        printf("%s", name);
        printf(" Is this your Username? [Y/N]: ");

        validation = getchar();
        clearInputBuffer();
        validation = toupper(validation);

        while (validation != 'Y' && validation != 'N')
        {
            clearScreen();
            printf("%c is not a valid input \n", validation);
            printf("%s", name);
            printf(" Is this your Username? [Y/N]: ");
            validation = getchar();
            clearInputBuffer();
            validation = toupper(validation);
        }

        usernode *userlist = NULL;

        char *errMsg = 0;
        char *sql = "SELECT id,username,password,role FROM user;";
        int rc = sqlite3_exec(db, sql, user_callback, &userlist, &errMsg);

        usernode *temp = userlist;
        while (temp != NULL)
        {
            if (strcmp(temp->userinfo.username, name) == 0)
            {
                printf("Username Already exists:(must be unique)");
                validation = 'N';
            }
            temp = temp->next;
        }
        freeUserList(userlist);
    }

    clearScreen();

    while (1)
    {
        printf("Enter your password: \n");
        scanf("%s", password);
        clearInputBuffer();
        printf("Confirm password: \n");
        scanf("%s", conPassword);
        clearInputBuffer();

        if (strcmp(password, conPassword) == 0)
        {

            if (strlen(password) < 8)
            {
                clearScreen();
                printf("Password should be atleast 8 character\n");
                continue;
            }

            int specialChar = 0, Spaces = 0, UpperChar = 0;

            for (int i = 0; i < strlen(password); i++)
            {

                if (password[i] >= 'A' && password[i] <= 'Z')
                    UpperChar += 1;
                else if (!(password[i] >= 'A' && password[i] <= 'Z') && !(password[i] >= 'a' && password[i] <= 'z'))
                    specialChar += 1;
                else if (password[i] == ' ')
                    Spaces += 1;
            }

            if (UpperChar < 1)
            {
                clearScreen();
                printf("Password must contain atleast one Upper Character\n");
                continue;
            }
            else if (specialChar < 1)
            {
                clearScreen();
                printf("Password must contain one special character or a digit\n");
                continue;
            }
            else if (Spaces > 0)
            {
                clearScreen();
                printf("Password must not contain spaces\n");
                continue;
            }
            break;
        }
        else
        {
            printf("Passwords do not match\n");
        }
    }
    clearScreen();

    printf("Enter your role\n[A]: Doctor\n[B]: Staff\n[C]: Admin\n");
    validation = getchar();
    clearInputBuffer();
    validation = toupper(validation);

    while (validation != 'A' && validation != 'B' && validation != 'C')
    {

        printf("%c is not a valid option\n", validation);
        printf("Enter your role\n[A]: Doctor\n[B]: Staff\n[C]: Admin\n");
        validation = getchar();
        clearInputBuffer();
        validation = toupper(validation);
    }

    char *role;
    switch (validation)
    {
    case 'A':
        role = "Doctor";
        break;

    case 'B':
        role = "Staff";
        break;

    default:
        role = "Admin";
    }

    printf("Your account has been created please Login\n");

    char *errMsg = 0;
    char sql[300];
    snprintf(sql, sizeof(sql),
             "INSERT INTO USER(username, password, role) "
             "VALUES ('%s', '%s', '%s')",
             name, password, role);

    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return nullUser();
    }

    char *statement = "SELECT COUNT(*) FROM USER";
    int Count = 0;
    rc = sqlite3_exec(db, statement, count_callback, &Count, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return nullUser();
    }

    if (strcmp(role, "Staff") == 0)
    {
        errMsg = 0;
        snprintf(sql, sizeof(sql),
                 "INSERT INTO STAFF(id, job, salary, shift) VALUES (%d, 'Not assigned', '5000', 'Day')",
                 Count);

        rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
            return nullUser();
        }
    }

    if (strcmp(role, "Doctor") == 0)
    {
        errMsg = 0;
        snprintf(sql, sizeof(sql),
                 "INSERT INTO DOCTOR(id, speciality, timing) VALUES (%d, 'Not assigned', '10-12')",
                 Count);

        rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
            return nullUser();
        }
    }

    user login = loginUser(db);
    return login;
}

int staff_Menu(sqlite3 *db, user login)
{

    char staffInfo[3][50];
    char *errMsg = 0;
    char sql[300];
    snprintf(sql, sizeof(sql),
             "SELECT * FROM STAFF WHERE id = %d", login.id);

    int rc = sqlite3_exec(db, sql, staff_callback, staffInfo, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    char validation = 'N';
    while (validation != 'Y')
    {
        printf("Your job is: %s\n", staffInfo[0]);
        printf("Your salary is: %s\n", staffInfo[1]);
        printf("Your shift is: %s\n \n \n", staffInfo[2]);

        printf("Do You Want To Continue? [Y]: ");
        clearInputBuffer();
        validation = toupper(getchar());
        clearInputBuffer();
    }

    return SQLITE_OK;
}

int doctor_Menu(sqlite3 *db, user login){

    char doctorInfo[3][50];
    char *errMsg = 0;
    char sql[300];
    snprintf(sql, sizeof(sql),
             "SELECT * FROM DOCTOR WHERE id = %d", login.id);

    int rc = sqlite3_exec(db, sql, doctor_callback, doctorInfo, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    char validation = 'N';
    while (validation != 'Y')
    {
        printf("Your speciality is: %s\n", doctorInfo[0]);
        printf("Your timings are: %s\n\n\n", doctorInfo[1]);

        printf("Do You Want To Continue? [Y]: ");
        clearInputBuffer();
        validation = toupper(getchar());
        clearInputBuffer();
    }

    return SQLITE_OK;
}

int admin_Menu(sqlite3 *db, user login)
{
    
    char option = '\0';
    
    while (option != 'H')
    {
        option = 'Z';
        printf("What do you want to do?\n");
        printf("[A]: Add Patient\n");
        printf("[B]: Edit Staff Table\n");
        printf("[C]: Edit Patient Table\n");
        printf("[D]: Edit Doctor Table\n");
        printf("[E]: View Staff Table\n");
        printf("[F]: View Patient Table\n");
        printf("[G]: View Doctor Table\n");
        printf("[H]: To Quit\n");

        clearInputBuffer();
        option = toupper(getchar());
        clearInputBuffer();
        
        int rc;

        switch (option)
        {
        case 'A':
            rc = add_patient(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;
        case 'B':
            rc = edit_staff(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;

        case 'C':
            rc = edit_patient(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;
        case 'D':
            rc = edit_doctor(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;

        case 'E':
            rc = view_staff(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;

        case 'F':
            rc = view_patient(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;

        case 'G':
            rc = view_doctor(db);
            if (rc != SQLITE_OK)
            {
                return rc;
            }
            break;
        case 'H':
            return SQLITE_OK;
        }
    }
}

int edit_doctor(sqlite3 *db)
{
    int choice = 0;
    char value[100], name[100];
    char column[30];
    char sql[300];
    char *errMsg = NULL;

    printf("Enter doctor Name to update: ");
    scanf("%s", name);
    clearInputBuffer();

    while (choice != 1 && choice != 2 && choice != 3)
    {

        printf("What do you want to update?\n");
        printf("1. Speciality\n");
        printf("2. Timings\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch (choice)
        {
     
        case 1:
            strcpy(column, "speciality");
            printf("Enter new speciality: ");
            scanf("%s", value);
            clearInputBuffer();
            break;

        case 2:
            strcpy(column, "timing");
            printf("Enter new timing: ");
            scanf("%s", value);
            clearInputBuffer();
            break;

        default:
            printf("Invalid choice!\n");
            break;
        }
    }
    int docid = -1;

    snprintf(sql, sizeof(sql),
             "SELECT id FROM USER WHERE username = '%s'", name);

    int rc = sqlite3_exec(db, sql, userid_callback,&docid , &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    snprintf(sql, sizeof(sql),
             "UPDATE DOCTOR SET %s = '%s' WHERE id = '%d';",
             column, value, docid);

    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL ERROR: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    printf("Doctor updated successfully!\n");
    return SQLITE_OK;
}

int edit_staff(sqlite3 *db)
{
    int id, choice = 0;
    char value[100];
    char column[30];
    char sql[300];
    char *errMsg = NULL;

    printf("Enter Staff ID to update: ");
    scanf("%d", &id);
    clearInputBuffer();
    
    while (choice != 1 && choice != 2 && choice != 3 )
    {
        
        printf("\nWhat do you want to update?\n");
        printf("1. Job Title\n");
        printf("2. Salary\n");
        printf("3. Shift\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        clearInputBuffer();
        
        switch (choice)
        {
        case 1:
            strcpy(column, "job");
            printf("Enter new job title: ");
            scanf("%s", value);
            clearInputBuffer();
            break;

        case 2:
            strcpy(column, "salary");
            printf("Enter new salary: ");
            scanf("%s", value);
            clearInputBuffer();
            break;

        case 3:
            strcpy(column, "shift");
            printf("Enter new shift (Day/Night): ");
            scanf("%s", value);
            clearInputBuffer();
            break;

        default:
            printf("Invalid option!\n");
            break;
        }
    }

    snprintf(sql, sizeof(sql),
             "UPDATE STAFF SET %s = '%s' WHERE id = '%d';",
             column, value, id);

    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL ERROR: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    printf("Staff updated successfully!\n");
    return SQLITE_OK;
}

int edit_patient(sqlite3 *db)
{
    clearInputBuffer();

    int choice = 0;
    char Value[100], name[50];
    char column[30];
    char sql[300];
    char *errMsg = NULL;

    printf("Enter patient Name to update: ");
    scanf("%s", name);
    clearInputBuffer();

    while (choice != 1 && choice != 2 && choice != 3)
    {
        printf("What do you want to update?\n");
        printf("1. Name\n");
        printf("2. Status\n");
        printf("3. Balance\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        clearInputBuffer();

        switch (choice)
        {
        case 1:
            strcpy(column, "name");
            printf("Enter new name: ");
            scanf("%s", Value);
            clearInputBuffer();
            break;

        case 2:
            strcpy(column, "status");
            printf("Enter new status: ");
            scanf("%s", Value);
            clearInputBuffer();
            break;

        case 3:
            strcpy(column, "balance");
            printf("Enter new balance: ");
            scanf("%s", Value);
            clearInputBuffer();
            break;

        default:
            printf("Invalid option!\n");
            return 1;
        }
    }
    snprintf(sql, sizeof(sql),
             "UPDATE PATIENTS SET %s = '%s' WHERE name = '%s';",
             column, Value, name);

    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL ERROR: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    return SQLITE_OK;
}

int add_patient(sqlite3 *db){
    char name[50];
    float balance;

    printf("Enter patient name: \n");
    scanf("%s", name);
    clearInputBuffer();

    printf("Enter initial balance: \n");
    scanf("%f", &balance);
    clearInputBuffer();

    char statement[300];
    char *errMsg = 0;

    snprintf(statement, sizeof(statement),
             "INSERT INTO PATIENTS(name, balance, status) VALUES ('%s', '%f', 'True')",name, balance);
    int rc = sqlite3_exec(db, statement, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    errMsg = 0;
    char sql[300];
    char Doc_name[50];

    int docid = -1;

    printf("Enter the name of doctor you want: \n");
    scanf("%s", Doc_name);

    snprintf(sql, sizeof(sql),
             "SELECT id FROM USER WHERE username = '%s'", Doc_name);

    rc = sqlite3_exec(db, sql, userid_callback,&docid , &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    int patient_id = 0;

    snprintf(sql, sizeof(sql),
             "SELECT COUNT(*) FROM PATIENTS");

    rc = sqlite3_exec(db, sql, count_callback, &patient_id, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    char query[300];
    errMsg = 0;

    snprintf(query, sizeof(query),
             "INSERT INTO DOCTOR_PATIENT (id_patient, id_doctor)VALUES ('%d', '%d')", docid, patient_id);

    rc = sqlite3_exec(db, query, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
}

int view_patient(sqlite3 *db){
    char sql[300];
    char * errMsg;

    printf("Patient Table\n");
    snprintf(sql, sizeof(sql),
             "SELECT * FROM PATIENTS");


    int rc = sqlite3_exec(db, sql, patient_view_callback, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL ERROR: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }


}


int view_doctor(sqlite3 *db){

    char sql[300];
    char * errMsg;

    printf("Doctor Table\n");
    snprintf(sql, sizeof(sql),
             "SELECT * FROM DOCTOR");


    int rc = sqlite3_exec(db, sql, doctor_view_callback, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL ERROR: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
}


int view_staff(sqlite3 *db){
    char sql[300];
    char * errMsg;

    printf("Staff Table\n");
    snprintf(sql, sizeof(sql),
             "SELECT * FROM STAFF");


    int rc = sqlite3_exec(db, sql, staff_view_callback, NULL, &errMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL ERROR: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
}

void freeUserList(usernode *userlist){

    usernode *temp;
    while (userlist != NULL)
    {
        temp = userlist;
        userlist = temp->next;
        if (temp->userinfo.username)
            free(temp->userinfo.username);
        if (temp->userinfo.password)
            free(temp->userinfo.password);
        if (temp->userinfo.role)
            free(temp->userinfo.role);
        free(temp);
    }
}

void clearScreen()
{
    system("clear");
}

user nullUser(){
    user u;
    u.id = -1;
    u.username = NULL;
    u.password = NULL;
    u.role = NULL;
    return u;
}

void clearInputBuffer(){
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}