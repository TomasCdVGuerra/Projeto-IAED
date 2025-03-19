#ifndef VACCINE_MANAGEMENT_H
#define VACCINE_MANAGEMENT_H

#define MAX_VACCINE_NAME 50
#define MAX_BATCH_NAME 10
#define MAX_DATE 11
#define MAX_USER_NAME 100

// Vaccine batch structure with linked list implementation
typedef struct VaccineBatchNode {
    char *batch;
    char *expiry_date;
    int doses;
    char *vaccine_name;
    int applications;
    struct VaccineBatchNode *next;
} VaccineBatchNode;

// Inoculation structure with linked list implementation
typedef struct InoculationNode {
    char *user_name;
    char *vaccine_name;
    char *batch;
    char *application_date;
    struct InoculationNode *next;
} InoculationNode;

// Global linked list heads
extern VaccineBatchNode *vaccine_batches_head;
extern InoculationNode *inoculations_head;
extern int vaccine_count;
extern int inoculation_count;
extern char *current_date;

// Helper functions for linked list operations
VaccineBatchNode* create_vaccine_batch(char *batch, char *expiry_date, int doses, char *vaccine_name);
InoculationNode* create_inoculation(char *user_name, char *vaccine_name, char *batch, char *application_date);
void free_vaccine_batch(VaccineBatchNode *node);
void free_inoculation(InoculationNode *node);
void free_all_vaccine_batches();
void free_all_inoculations();

// Core functions
void parse_date(const char *date_str, int *day, int *month, int *year);
int compare_dates(const char *date1, const char *date2);
int is_valid_date(const char *date);
int is_valid_batch_name(const char *batch);
int is_valid_vaccine_name(const char *name);
void add_vaccine_batch(char *batch, char *expiry_date, int doses, char *vaccine_name);
int compare_batches(const VaccineBatchNode *a, const VaccineBatchNode *b);
void list_vaccine_batches(char *vaccine_names[], int vaccine_names_count);
void apply_vaccine(char *user_name, char *vaccine_name);
void remove_vaccine_batch(char *batch);
void delete_inoculation(char *user_name, char *date, char *batch);
void list_inoculations(char *user_name);
void advance_time(char *new_date);

#endif // VACCINE_MANAGEMENT_H
