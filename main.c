#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "vaccineManagement.h"

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
    if (month == 2) {
        int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (day > (is_leap ? 29 : 28)) return 0;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day > 30) return 0;
    }
    if (compare_dates(date, current_date) < 0) return 0;
    return 1;
}

int is_valid_batch_name(const char *batch) {
    if (strlen(batch) > MAX_BATCH_NAME) return 0;
    for (int i = 0; batch[i] != '\0'; i++) {
        if (!isxdigit(batch[i]) || (isalpha(batch[i]) && !isupper(batch[i]))) return 0;
    }
    return 1;
}

int is_valid_vaccine_name(const char *name) {
    if (strlen(name) > MAX_VACCINE_NAME) return 0;
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-') return 0;
    }
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
    if (!is_valid_batch_name(batch)) {
        printf("invalid batch\n");
        return;
    }
    if (!is_valid_date(expiry_date)) {
        printf("invalid date\n");
        return;
    }
    if (doses <= 0) {
        printf("invalid quantity\n");
        return;
    }
    if (!is_valid_vaccine_name(vaccine_name)) {
        printf("invalid name\n");
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

int compare_batches(const void *a, const void *b) {
    VaccineBatch *batchA = (VaccineBatch *)a;
    VaccineBatch *batchB = (VaccineBatch *)b;
    int date_comparison = compare_dates(batchA->expiry_date, batchB->expiry_date);
    if (date_comparison != 0) {
        return date_comparison;
    }
    return strcmp(batchA->batch, batchB->batch);
}

void list_vaccine_batches(char *vaccine_names[], int vaccine_names_count) {
    // Case: No vaccine names provided, list all vaccines
    if (vaccine_names_count == 0) {
        VaccineBatch filtered_batches[MAX_VACCINES];
        int filtered_count = 0;
        
        for (int i = 0; i < vaccine_count; i++) {
            filtered_batches[filtered_count++] = vaccine_batches[i];
        }

        qsort(filtered_batches, filtered_count, sizeof(VaccineBatch), compare_batches);

        for (int i = 0; i < filtered_count; i++) {
            int day, month, year;
            parse_date(filtered_batches[i].expiry_date, &day, &month, &year);
            printf("%s %s %02d-%02d-%d %d %d\n", 
                   filtered_batches[i].vaccine_name, 
                   filtered_batches[i].batch, 
                   day, month, year, 
                   filtered_batches[i].doses, 
                   filtered_batches[i].applications);
        }
    } 
    // Case: Process each vaccine name in the given order
    else {
        for (int j = 0; j < vaccine_names_count; j++) {
            VaccineBatch matching_batches[MAX_VACCINES];
            int batch_count = 0;
            int found = 0;
            
            // Find all batches matching this vaccine name
            for (int i = 0; i < vaccine_count; i++) {
                if (strcmp(vaccine_batches[i].vaccine_name, vaccine_names[j]) == 0) {
                    matching_batches[batch_count++] = vaccine_batches[i];
                    found = 1;
                }
            }
            
            // Either print "no such vaccine" or sort and print matching batches
            if (!found) {
                printf("%s: no such vaccine\n", vaccine_names[j]);
            } else {
                qsort(matching_batches, batch_count, sizeof(VaccineBatch), compare_batches);
                
                for (int i = 0; i < batch_count; i++) {
                    int day, month, year;
                    parse_date(matching_batches[i].expiry_date, &day, &month, &year);
                    printf("%s %s %02d-%02d-%d %d %d\n", 
                           matching_batches[i].vaccine_name, 
                           matching_batches[i].batch, 
                           day, month, year, 
                           matching_batches[i].doses, 
                           matching_batches[i].applications);
                }
            }
        }
    }
}

void apply_vaccine(char *user_name, char *vaccine_name) {
    // Validate vaccine name
    if (!is_valid_vaccine_name(vaccine_name)) {
        printf("invalid name\n");
        return;
    }
    
    int index = -1;
    
    // First find the best batch to use (earliest expiring with available doses)
    for (int i = 0; i < vaccine_count; i++) {
        if (strcmp(vaccine_batches[i].vaccine_name, vaccine_name) == 0 && 
            vaccine_batches[i].doses > 0 && 
            compare_dates(vaccine_batches[i].expiry_date, current_date) >= 0) {
            
            if (index == -1 || compare_dates(vaccine_batches[i].expiry_date, vaccine_batches[index].expiry_date) < 0) {
                index = i;
            }
        }
    }
    
    // If no valid batch found
    if (index == -1) {
        printf("no stock\n");
        return;
    }
    
    // Check if this person has already been vaccinated with this vaccine today
    for (int i = 0; i < inoculation_count; i++) {
        if (strcmp(inoculations[i].user_name, user_name) == 0 && 
            strcmp(inoculations[i].vaccine_name, vaccine_name) == 0 && 
            strcmp(inoculations[i].application_date, current_date) == 0) {
            
            printf("already vaccinated\n");
            return;
        }
    }
    
    // Record the vaccination - dynamically allocate memory for the user name
    inoculations[inoculation_count].user_name = strdup(user_name);
    if (inoculations[inoculation_count].user_name == NULL) {
        printf("memory allocation error\n");
        return;
    }
    
    strcpy(inoculations[inoculation_count].vaccine_name, vaccine_name);
    strcpy(inoculations[inoculation_count].batch, vaccine_batches[index].batch);
    strcpy(inoculations[inoculation_count].application_date, current_date);
    inoculation_count++;
    
    // Update the vaccine batch
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
        if (strcmp(inoculations[i].user_name, user_name) == 0 && 
            (date == NULL || strcmp(inoculations[i].application_date, date) == 0) && 
            (batch == NULL || strcmp(inoculations[i].batch, batch) == 0)) {
            
            // Free dynamically allocated memory
            free(inoculations[i].user_name);
            
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
    // If no specific user is requested, list all inoculations
    if (user_name == NULL) {
        for (int i = 0; i < inoculation_count; i++) {
            int day, month, year;
            parse_date(inoculations[i].application_date, &day, &month, &year);
            printf("%s %s %02d-%02d-%d\n", inoculations[i].user_name, inoculations[i].batch, day, month, year);
        }
        return;
    }
    
    // Check if the user exists and list their inoculations
    int user_found = 0;
    for (int i = 0; i < inoculation_count; i++) {
        if (strcmp(inoculations[i].user_name, user_name) == 0) {
            int day, month, year;
            parse_date(inoculations[i].application_date, &day, &month, &year);
            printf("%s %s %02d-%02d-%d\n", inoculations[i].user_name, inoculations[i].batch, day, month, year);
            user_found = 1;
        }
    }
    
    // If the user doesn't exist, display the appropriate message
    if (!user_found && user_name != NULL) {
        printf("%s: no such user\n", user_name);
    }
}

void advance_time(char *new_date) {
    if (new_date == NULL) {
        int day, month, year;
        parse_date(current_date, &day, &month, &year);
        printf("%02d-%02d-%d\n", day, month, year);
        return;
    }
    if (is_valid_date(new_date)) {
        strcpy(current_date, new_date);
        int day, month, year;
        parse_date(current_date, &day, &month, &year);
        printf("%02d-%02d-%d\n", day, month, year);
    } else {
        printf("invalid date\n");
    }
}

int main() {
    char command;
    while (scanf(" %c", &command) != EOF) {
        switch (command) {
            case 'q':
                // Free allocated memory before exiting
                for (int i = 0; i < inoculation_count; i++) {
                    free(inoculations[i].user_name);
                }
                return 0;
            
            case 'c': {
                char batch[MAX_BATCH_NAME], expiry_date[MAX_DATE], vaccine_name[MAX_VACCINE_NAME];
                int doses;
                int scan_result = scanf("%s %s %d %s", batch, expiry_date, &doses, vaccine_name);
                
                // Check if we correctly read all 4 parameters
                if (scan_result == 4) {
                    add_vaccine_batch(batch, expiry_date, doses, vaccine_name);
                } else {
                    // Clear the input buffer
                    char c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("invalid batch\n");
                }
                break;
            }
            
            case 'l': {
                char line[MAX_VACCINE_NAME];
                char *vaccine_names[MAX_VACCINES];
                int vaccine_names_count = 0;
            
                // Read the entire line of input
                if (fgets(line, sizeof(line), stdin) != NULL) {
                    char *token = strtok(line, " \n");
                    while (token != NULL) {
                        vaccine_names[vaccine_names_count] = strdup(token);
                        vaccine_names_count++;
                        token = strtok(NULL, " \n");
                    }
                }
            
                list_vaccine_batches(vaccine_names, vaccine_names_count);
            
                for (int i = 0; i < vaccine_names_count; i++) {
                    free(vaccine_names[i]);
                }
                break;
            }
            
            case 'a': {
                char user_name[MAX_USER_NAME] = {0}, vaccine_name[MAX_VACCINE_NAME] = {0};
                char line[MAX_USER_NAME + MAX_VACCINE_NAME + 10]; // Buffer for the entire line
                
                if (fgets(line, sizeof(line), stdin) != NULL) {
                    // Skip initial spaces
                    char *start = line;
                    while (*start == ' ' || *start == '\t') start++;
                    
                    // Remove trailing newline
                    char *newline = strchr(start, '\n');
                    if (newline) *newline = '\0';
                    
                    // Check if username is quoted
                    if (*start == '"') {
                        // Username enclosed in quotes
                        start++; // Skip opening quote
                        char *end_quote = strchr(start, '"');
                        if (end_quote && end_quote - start < MAX_USER_NAME) {
                            *end_quote = '\0'; // Replace closing quote with null terminator
                            strcpy(user_name, start); // Copy quoted username
                            
                            // Move to after the closing quote
                            char *after_quote = end_quote + 1;
                            while (*after_quote == ' ' || *after_quote == '\t') after_quote++;
                            
                            // The rest is the vaccine name
                            strcpy(vaccine_name, after_quote);
                            apply_vaccine(user_name, vaccine_name);
                        } else {
                            printf("invalid batch\n");
                        }
                    } else {
                        // No quotes, extract user name and vaccine name by finding the last space
                        char *last_space = strrchr(start, ' ');
                        if (last_space != NULL && last_space - start < MAX_USER_NAME) {
                            strncpy(user_name, start, last_space - start);
                            user_name[last_space - start] = '\0';
                            strcpy(vaccine_name, last_space + 1);
                            apply_vaccine(user_name, vaccine_name);
                        } else {
                            printf("invalid batch\n");
                        }
                    }
                }
                break;
            }
            
            case 'r': {
                char batch[MAX_BATCH_NAME];
                scanf("%s", batch);
                remove_vaccine_batch(batch);
                break;
            }
            
            case 'd': {
                char line[MAX_USER_NAME + MAX_DATE + MAX_BATCH_NAME + 10]; // Buffer for the entire line
                
                // Read the entire line
                if (fgets(line, sizeof(line), stdin) != NULL) {
                    // Skip initial spaces
                    char *start = line;
                    while (*start == ' ' || *start == '\t') start++;
                    
                    // Remove trailing newline
                    char *newline = strchr(start, '\n');
                    if (newline) *newline = '\0';
                    
                    char *current = start;
                    char *args[3] = {NULL, NULL, NULL};
                    int arg_count = 0;
                    
                    // Parse arguments, handling quoted usernames
                    if (*current == '"') {
                        // Username enclosed in quotes
                        current++; // Skip opening quote
                        char *end_quote = strchr(current, '"');
                        if (end_quote) {
                            *end_quote = '\0'; // Replace closing quote with null terminator
                            args[arg_count++] = current; // Store username
                            current = end_quote + 1;
                            
                            // Skip spaces after the quoted username
                            while (*current == ' ' || *current == '\t') current++;
                            
                            // Parse remaining arguments
                            char *token = strtok(current, " \t");
                            while (token != NULL && arg_count < 3) {
                                args[arg_count++] = token;
                                token = strtok(NULL, " \t");
                            }
                        }
                    } else {
                        // No quotes, just parse space-separated arguments
                        char *token = strtok(current, " \t");
                        while (token != NULL && arg_count < 3) {
                            args[arg_count++] = token;
                            token = strtok(NULL, " \t");
                        }
                    }
                    
                    // Call delete_inoculation with the parsed arguments
                    if (arg_count == 1) {
                        delete_inoculation(args[0], NULL, NULL);
                    } else if (arg_count == 2) {
                        delete_inoculation(args[0], args[1], NULL);
                    } else if (arg_count == 3) {
                        delete_inoculation(args[0], args[1], args[2]);
                    }
                }
                break;
            }
            
            case 'u': {
                char user_name[MAX_USER_NAME];
                char line[MAX_USER_NAME + 10]; // Buffer for the entire line
                
                // Read the entire line
                if (fgets(line, sizeof(line), stdin) != NULL) {
                    // Skip initial spaces
                    char *start = line;
                    while (*start == ' ' || *start == '\t') start++;
                    
                    // Remove trailing newline
                    char *newline = strchr(start, '\n');
                    if (newline) *newline = '\0';
                    
                    // If line is empty after skipping spaces
                    if (*start == '\0') {
                        list_inoculations(NULL);
                    } 
                    // If username is quoted
                    else if (*start == '"') {
                        start++; // Skip opening quote
                        char *end_quote = strchr(start, '"');
                        if (end_quote) {
                            *end_quote = '\0'; // Replace closing quote with null terminator
                            strcpy(user_name, start);
                            list_inoculations(user_name);
                        }
                    } 
                    // Simple username without spaces
                    else {
                        // Get first word only
                        char *space = strchr(start, ' ');
                        if (space) *space = '\0';
                        strcpy(user_name, start);
                        list_inoculations(user_name);
                    }
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
                // Ignore unrecognized commands
                break;
        }
    }
    return 0;
}