#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/*small change*/ 

#define MAX_VACCINES 1000
#define MAX_VACCINE_NAME 50
#define MAX_BATCH_NAME 10
#define MAX_DATE 11
#define MAX_USER_NAME 100

typedef struct {
    char batch[MAX_BATCH_NAME];
    char expiry_date[MAX_DATE];
    int doses;
    char vaccine_name[MAX_VACCINE_NAME];
    int applications;
} VaccineBatch;

typedef struct {
    char user_name[MAX_USER_NAME];
    char vaccine_name[MAX_VACCINE_NAME];
    char batch[MAX_BATCH_NAME];
    char application_date[MAX_DATE];
} Inoculation;

VaccineBatch vaccine_batches[MAX_VACCINES];
Inoculation inoculations[MAX_VACCINES * MAX_VACCINES];
int vaccine_count = 0;
int inoculation_count = 0;
char current_date[MAX_DATE] = "01-01-2025";

void parse_date(const char *date_str, int *day, int *month, int *year) {
    sscanf(date_str, "%d-%d-%d", day, month, year);
}

int compare_dates(const char *date1, const char *date2) {
    int day1, month1, year1;
    int day2, month2, year2;
    parse_date(date1, &day1, &month1, &year1);
    parse_date(date2, &day2, &month2, &year2);
    if (year1 != year2) return year1 - year2;
    if (month1 != month2) return month1 - month2;
    return day1 - day2;
}

int is_valid_date(const char *date) {
    int day, month, year;
    parse_date(date, &day, &month, &year);
    if (year < 2025 || month < 1 || month > 12 || day < 1 || day > 31) return 0;
    if (compare_dates(date, current_date) < 0) return 0;
    return 1;
}

void add_vaccine_batch(char *batch, char *expiry_date, int doses, char *vaccine_name) {
    if (vaccine_count >= MAX_VACCINES) {
        printf("too many vaccines\n");
        return;
    }
    for (int i = 0; i < vaccine_count; i++) {
        if (strcmp(vaccine_batches[i].batch, batch) == 0) {
            printf("duplicate batch number\n");
            return;
        }
    }
    if (strlen(batch) > MAX_BATCH_NAME || !is_valid_date(expiry_date) || doses <= 0) {
        printf("invalid input\n");
        return;
    }
    strcpy(vaccine_batches[vaccine_count].batch, batch);
    strcpy(vaccine_batches[vaccine_count].expiry_date, expiry_date);
    vaccine_batches[vaccine_count].doses = doses;
    strcpy(vaccine_batches[vaccine_count].vaccine_name, vaccine_name);
    vaccine_batches[vaccine_count].applications = 0;
    vaccine_count++;
    printf("%s\n", batch);
}

void list_vaccine_batches(char *vaccine_name) {
    for (int i = 0; i < vaccine_count; i++) {
        if (vaccine_name == NULL || strcmp(vaccine_batches[i].vaccine_name, vaccine_name) == 0) {
            printf("%s %s %s %d %d\n", vaccine_batches[i].vaccine_name, vaccine_batches[i].batch, vaccine_batches[i].expiry_date, vaccine_batches[i].doses, vaccine_batches[i].applications);
        }
    }
}

void apply_vaccine(char *user_name, char *vaccine_name) {
    int index = -1;
    for (int i = 0; i < vaccine_count; i++) {
        if (strcmp(vaccine_batches[i].vaccine_name, vaccine_name) == 0 && vaccine_batches[i].doses > 0 && compare_dates(vaccine_batches[i].expiry_date, current_date) >= 0) {
            if (index == -1 || compare_dates(vaccine_batches[i].expiry_date, vaccine_batches[index].expiry_date) < 0) {
                index = i;
            }
        }
    }
    if (index == -1) {
        printf("no stock\n");
        return;
    }
    for (int i = 0; i < inoculation_count; i++) {
        if (strcmp(inoculations[i].user_name, user_name) == 0 && strcmp(inoculations[i].vaccine_name, vaccine_name) == 0 && strcmp(inoculations[i].application_date, current_date) == 0) {
            printf("already vaccinated\n");
            return;
        }
    }
    strcpy(inoculations[inoculation_count].user_name, user_name);
    strcpy(inoculations[inoculation_count].vaccine_name, vaccine_name);
    strcpy(inoculations[inoculation_count].batch, vaccine_batches[index].batch);
    strcpy(inoculations[inoculation_count].application_date, current_date);
    inoculation_count++;
    vaccine_batches[index].doses--;
    vaccine_batches[index].applications++;
    printf("%s\n", vaccine_batches[index].batch);
}

void remove_vaccine_batch(char *batch) {
    for (int i = 0; i < vaccine_count; i++) {
        if (strcmp(vaccine_batches[i].batch, batch) == 0) {
            if (vaccine_batches[i].applications > 0) {
                printf("%d doses already applied\n", vaccine_batches[i].applications);
                return;
            }
            for (int j = i; j < vaccine_count - 1; j++) {
                vaccine_batches[j] = vaccine_batches[j + 1];
            }
            vaccine_count--;
            printf("batch %s removed\n", batch);
            return;
        }
    }
    printf("no such batch\n");
}

void delete_inoculation(char *user_name, char *date, char *batch) {
    int deleted_inoculations = 0;
    for (int i = 0; i < inoculation_count; i++) {
        if (strcmp(inoculations[i].user_name, user_name) == 0 && (date == NULL || strcmp(inoculations[i].application_date, date) == 0) && (batch == NULL || strcmp(inoculations[i].batch, batch) == 0)) {
            for (int j = i; j < inoculation_count - 1; j++) {
                inoculations[j] = inoculations[j + 1];
            }
            inoculation_count--;
            deleted_inoculations++;
            i--;
        }
    }
    printf("%d inoculations deleted\n", deleted_inoculations);
}

void list_inoculations(char *user_name) {
    for (int i = 0; i < inoculation_count; i++) {
        if (user_name == NULL || strcmp(inoculations[i].user_name, user_name) == 0) {
            printf("%s %s %s %s\n", inoculations[i].user_name, inoculations[i].batch, inoculations[i].application_date, inoculations[i].vaccine_name);
        }
    }
}

void advance_time(char *new_date) {
    if (new_date == NULL) {
        printf("%s\n", current_date);
        return;
    }
    if (is_valid_date(new_date)) {
        strcpy(current_date, new_date);
        printf("%s\n", current_date);
    } else {
        printf("invalid date\n");
    }
}

int main() {
    char command;
    while (scanf(" %c", &command) != EOF) {
        switch (command) {
            case 'q':
                return 0;
            case 'c': {
                char batch[MAX_BATCH_NAME], expiry_date[MAX_DATE], vaccine_name[MAX_VACCINE_NAME];
                int doses;
                scanf("%s %s %d %s", batch, expiry_date, &doses, vaccine_name);
                add_vaccine_batch(batch, expiry_date, doses, vaccine_name);
                break;
            }
            case 'l': {
                char vaccine_name[MAX_VACCINE_NAME];
                if (scanf("%s", vaccine_name) == 1) {
                    list_vaccine_batches(vaccine_name);
                } else {
                    list_vaccine_batches(NULL);
                }
                break;
            }
            case 'a': {
                char user_name[MAX_USER_NAME], vaccine_name[MAX_VACCINE_NAME];
                scanf("%s %s", user_name, vaccine_name);
                apply_vaccine(user_name, vaccine_name);
                break;
            }
            case 'r': {
                char batch[MAX_BATCH_NAME];
                scanf("%s", batch);
                remove_vaccine_batch(batch);
                break;
            }
            case 'd': {
                char user_name[MAX_USER_NAME], date[MAX_DATE], batch[MAX_BATCH_NAME];
                int args = scanf("%s %s %s", user_name, date, batch);
                if (args == 1) {
                    delete_inoculation(user_name, NULL, NULL);
                } else if (args == 2) {
                    delete_inoculation(user_name, date, NULL);
                } else {
                    delete_inoculation(user_name, date, batch);
                }
                break;
            }
            case 'u': {
                char user_name[MAX_USER_NAME];
                if (scanf("%s", user_name) == 1) {
                    list_inoculations(user_name);
                } else {
                    list_inoculations(NULL);
                }
                break;
            }
            case 't': {
                char new_date[MAX_DATE];
                if (scanf("%s", new_date) == 1) {
                    advance_time(new_date);
                } else {
                    advance_time(NULL);
                }
                break;
            }
            default:
                printf("invalid command\n");
                break;
        }
    }
    return 0;
}