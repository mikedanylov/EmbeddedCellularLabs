#ifndef Unauthorized_h
#define Unauthorized_h

#define PHONE 14
#define MAX_UNAUTH 10

typedef struct{
    char phone_number[PHONE];
    int attempts;
} Unauthorized;

void unauthorized_init(Unauthorized* unauthorized_entries){
    for (int i = 0; i < MAX_UNAUTH; i++){
        strcmp(unauthorized_entries[i].phone_number, "\0");
    }
}

bool is_known(char* phone, Unauthorized* unauthorized_entries){

    for (int i = 0; i < MAX_UNAUTH; i++){
        if (strcmp(unauthorized_entries[i].phone_number, phone) == 0){
            return true;
        }
    }
    return false;
}

void increment_attempts(char* phone_number, Unauthorized* unauthorized_entries){
    for (int i = 0; i < MAX_UNAUTH; i++){
        if (strcmp(unauthorized_entries[i].phone_number, phone_number) == 0){
            unauthorized_entries[i].attempts++;
        }
    }
}

int get_attempts(char* phone_number, Unauthorized* unauthorized_entries){
    for (int i = 0; i < MAX_UNAUTH; i++){
        if (strcmp(unauthorized_entries[i].phone_number, phone_number) == 0){
            return unauthorized_entries[i].attempts;
        }
    }
    return -1;
}

#endif
