#ifndef VACCINE_MANAGEMENT_H
#define VACCINE_MANAGEMENT_H

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
    char *user_name;  // Change to pointer for dynamic allocation
    char vaccine_name[MAX_VACCINE_NAME];
    char batch[MAX_BATCH_NAME];
    char application_date[MAX_DATE];
} Inoculation;

extern VaccineBatch vaccine_batches[MAX_VACCINES];
extern Inoculation inoculations[MAX_VACCINES * MAX_VACCINES];
extern int vaccine_count;
extern int inoculation_count;
extern char current_date[MAX_DATE];

void parse_date(const char *date_str, int *day, int *month, int *year);
int compare_dates(const char *date1, const char *date2);
int is_valid_date(const char *date);
int is_valid_batch_name(const char *batch);
int is_valid_vaccine_name(const char *name);
void add_vaccine_batch(char *batch, char *expiry_date, int doses, char *vaccine_name);
int compare_batches(const void *a, const void *b);
void list_vaccine_batches(char *vaccine_names[], int vaccine_names_count);
void apply_vaccine(char *user_name, char *vaccine_name);
void remove_vaccine_batch(char *batch);
void delete_inoculation(char *user_name, char *date, char *batch);
void list_inoculations(char *user_name);
void advance_time(char *new_date);

#endif // VACCINE_MANAGEMENT_H