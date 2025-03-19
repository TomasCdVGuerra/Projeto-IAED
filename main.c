#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "vaccineManagement.h"

// Language support
typedef struct {
    char *too_many_vaccines;
    char *duplicate_batch;
    char *invalid_batch;
    char *invalid_name;
    char *invalid_date;
    char *invalid_quantity;
    char *no_such_vaccine;
    char *no_stock;
    char *already_vaccinated;
    char *no_such_batch;
    char *no_such_user;
    char *memory_error;
} LanguageMessages;

// Define messages for English
LanguageMessages en_messages = {
    "too many vaccines",
    "duplicate batch number",
    "invalid batch",
    "invalid name",
    "invalid date",
    "invalid quantity",
    "no such vaccine",
    "no stock",
    "already vaccinated",
    "no such batch",
    "no such user",
    "memory allocation error"
};

// Define messages for Portuguese
LanguageMessages pt_messages = {
    "demasiadas vacinas",
    "número de lote duplicado",
    "lote inválido",
    "nome inválido",
    "data inválida",
    "quantidade inválida",
    "vacina inexistente",
    "esgotado",
    "já vacinado",
    "lote inexistente",
    "utente inexistente",
    "sem memória"
};

// Global pointer to the current language messages
LanguageMessages *messages;

// Initialize global variables
VaccineBatchNode *vaccine_batches_head = NULL;
InoculationNode *inoculations_head = NULL;
int vaccine_count = 0;
int inoculation_count = 0;
char *current_date;

// Helper functions for linked list operations
VaccineBatchNode* create_vaccine_batch(char *batch, char *expiry_date, int doses, char *vaccine_name) {
    VaccineBatchNode *new_node = (VaccineBatchNode*)malloc(sizeof(VaccineBatchNode));
    if (!new_node) return NULL;
    
    new_node->batch = strdup(batch);
    new_node->expiry_date = strdup(expiry_date);
    new_node->vaccine_name = strdup(vaccine_name);
    new_node->doses = doses;
    new_node->applications = 0;
    new_node->next = NULL;
    
    if (!new_node->batch || !new_node->expiry_date || !new_node->vaccine_name) {
        free_vaccine_batch(new_node);
        return NULL;
    }
    
    return new_node;
}

InoculationNode* create_inoculation(char *user_name, char *vaccine_name, char *batch, char *application_date) {
    InoculationNode *new_node = (InoculationNode*)malloc(sizeof(InoculationNode));
    if (!new_node) return NULL;
    
    new_node->user_name = strdup(user_name);
    new_node->vaccine_name = strdup(vaccine_name);
    new_node->batch = strdup(batch);
    new_node->application_date = strdup(application_date);
    new_node->next = NULL;
    
    if (!new_node->user_name || !new_node->vaccine_name || 
        !new_node->batch || !new_node->application_date) {
        free_inoculation(new_node);
        return NULL;
    }
    
    return new_node;
}

void free_vaccine_batch(VaccineBatchNode *node) {
    if (!node) return;
    free(node->batch);
    free(node->expiry_date);
    free(node->vaccine_name);
    free(node);
}

void free_inoculation(InoculationNode *node) {
    if (!node) return;
    free(node->user_name);
    free(node->vaccine_name);
    free(node->batch);
    free(node->application_date);
    free(node);
}

void free_all_vaccine_batches() {
    VaccineBatchNode *current = vaccine_batches_head;
    while (current) {
        VaccineBatchNode *temp = current;
        current = current->next;
        free_vaccine_batch(temp);
    }
    vaccine_batches_head = NULL;
    vaccine_count = 0;
}

void free_all_inoculations() {
    InoculationNode *current = inoculations_head;
    while (current) {
        InoculationNode *temp = current;
        current = current->next;
        free_inoculation(temp);
    }
    inoculations_head = NULL;
    inoculation_count = 0;
}

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
    // Check if we've reached some reasonable limit
    if (vaccine_count >= 10000) {
        printf("%s\n", messages->too_many_vaccines);
        return;
    }
    
    // Check for duplicate batches
    VaccineBatchNode *current = vaccine_batches_head;
    while (current) {
        if (strcmp(current->batch, batch) == 0) {
            printf("%s\n", messages->duplicate_batch);
            return;
        }
        current = current->next;
    }
    
    if (!is_valid_batch_name(batch)) {
        printf("%s\n", messages->invalid_batch);
        return;
    }
    
    if (!is_valid_date(expiry_date)) {
        printf("%s\n", messages->invalid_date);
        return;
    }
    
    if (doses <= 0) {
        printf("%s\n", messages->invalid_quantity);
        return;
    }
    
    if (!is_valid_vaccine_name(vaccine_name)) {
        printf("%s\n", messages->invalid_name);
        return;
    }
    
    // Create a new vaccine batch node
    VaccineBatchNode *new_batch = create_vaccine_batch(batch, expiry_date, doses, vaccine_name);
    if (!new_batch) {
        printf("%s\n", messages->memory_error);
        return;
    }
    
    // Add to the beginning of the list for simplicity
    new_batch->next = vaccine_batches_head;
    vaccine_batches_head = new_batch;
    vaccine_count++;
    
    printf("%s\n", batch);
}

int compare_batches(const VaccineBatchNode *a, const VaccineBatchNode *b) {
    int date_comparison = compare_dates(a->expiry_date, b->expiry_date);
    if (date_comparison != 0) {
        return date_comparison;
    }
    return strcmp(a->batch, b->batch);
}

// Merge two sorted linked lists
VaccineBatchNode* merge_sorted_lists(VaccineBatchNode *a, VaccineBatchNode *b) {
    VaccineBatchNode dummy;
    VaccineBatchNode *tail = &dummy;
    dummy.next = NULL;
    
    while (a && b) {
        if (compare_batches(a, b) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    
    tail->next = (a) ? a : b;
    return dummy.next;
}

// Split a linked list into two halves
void split_list(VaccineBatchNode *source, VaccineBatchNode **front, VaccineBatchNode **back) {
    VaccineBatchNode *fast;
    VaccineBatchNode *slow;
    slow = source;
    fast = source->next;
    
    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }
    
    *front = source;
    *back = slow->next;
    slow->next = NULL;
}

// Merge sort for linked list
void merge_sort(VaccineBatchNode **headRef) {
    VaccineBatchNode *head = *headRef;
    VaccineBatchNode *a;
    VaccineBatchNode *b;
    
    if ((head == NULL) || (head->next == NULL)) {
        return;
    }
    
    split_list(head, &a, &b);
    
    merge_sort(&a);
    merge_sort(&b);
    
    *headRef = merge_sorted_lists(a, b);
}

void list_vaccine_batches(char *vaccine_names[], int vaccine_names_count) {
    // Case: No vaccine names provided, list all vaccines
    if (vaccine_names_count == 0) {
        // Create a deep copy of the vaccine batches list
        VaccineBatchNode *filtered_head = NULL;
        VaccineBatchNode *filtered_tail = NULL;
        VaccineBatchNode *current = vaccine_batches_head;
        
        while (current) {
            VaccineBatchNode *new_node = create_vaccine_batch(
                current->batch, current->expiry_date, 
                current->doses, current->vaccine_name);
            
            if (!new_node) {
                printf("%s\n", messages->memory_error);
                // Clean up created list so far
                while (filtered_head) {
                    VaccineBatchNode *temp = filtered_head;
                    filtered_head = filtered_head->next;
                    free_vaccine_batch(temp);
                }
                return;
            }
            
            new_node->applications = current->applications;
            
            // Append to filtered list
            if (!filtered_head) {
                filtered_head = new_node;
                filtered_tail = new_node;
            } else {
                filtered_tail->next = new_node;
                filtered_tail = new_node;
            }
            
            current = current->next;
        }
        
        // Sort the filtered list
        if (filtered_head) {
            merge_sort(&filtered_head);
        }
        
        // Print the sorted list
        current = filtered_head;
        while (current) {
            int day, month, year;
            parse_date(current->expiry_date, &day, &month, &year);
            printf("%s %s %02d-%02d-%d %d %d\n",
                   current->vaccine_name,
                   current->batch,
                   day, month, year,
                   current->doses,
                   current->applications);
            
            current = current->next;
        }
        
        // Clean up the filtered list
        while (filtered_head) {
            VaccineBatchNode *temp = filtered_head;
            filtered_head = filtered_head->next;
            free_vaccine_batch(temp);
        }
    } 
    // Case: Process each vaccine name in the given order
    else {
        for (int j = 0; j < vaccine_names_count; j++) {
            // Create a list of matching batches
            VaccineBatchNode *matching_head = NULL;
            VaccineBatchNode *matching_tail = NULL;
            int found = 0;
            
            // Find all batches matching this vaccine name
            VaccineBatchNode *current = vaccine_batches_head;
            while (current) {
                if (strcmp(current->vaccine_name, vaccine_names[j]) == 0) {
                    found = 1;
                    
                    VaccineBatchNode *new_node = create_vaccine_batch(
                        current->batch, current->expiry_date, 
                        current->doses, current->vaccine_name);
                        
                    if (!new_node) {
                        printf("%s\n", messages->memory_error);
                        // Clean up created list so far
                        while (matching_head) {
                            VaccineBatchNode *temp = matching_head;
                            matching_head = matching_head->next;
                            free_vaccine_batch(temp);
                        }
                        return;
                    }
                    
                    new_node->applications = current->applications;
                    
                    // Append to matching list
                    if (!matching_head) {
                        matching_head = new_node;
                        matching_tail = new_node;
                    } else {
                        matching_tail->next = new_node;
                        matching_tail = new_node;
                    }
                }
                
                current = current->next;
            }
            
            // Either print "no such vaccine" or sort and print matching batches
            if (!found) {
                printf("%s: %s\n", vaccine_names[j], messages->no_such_vaccine);
            } else {
                // Sort the matching list
                if (matching_head) {
                    merge_sort(&matching_head);
                }
                
                // Print the sorted list
                current = matching_head;
                while (current) {
                    int day, month, year;
                    parse_date(current->expiry_date, &day, &month, &year);
                    printf("%s %s %02d-%02d-%d %d %d\n",
                          current->vaccine_name,
                          current->batch,
                          day, month, year,
                          current->doses,
                          current->applications);
                    
                    current = current->next;
                }
            }
            
            // Clean up the matching list
            while (matching_head) {
                VaccineBatchNode *temp = matching_head;
                matching_head = matching_head->next;
                free_vaccine_batch(temp);
            }
        }
    }
}

void apply_vaccine(char *user_name, char *vaccine_name) {
    // Validate vaccine name
    if (!is_valid_vaccine_name(vaccine_name)) {
        printf("%s\n", messages->invalid_name);
        return;
    }
    
    // Find best batch (earliest expiry with available doses)
    VaccineBatchNode *best_batch = NULL;
    VaccineBatchNode *current = vaccine_batches_head;
    
    while (current) {
        if (strcmp(current->vaccine_name, vaccine_name) == 0 && 
            current->doses > 0 && 
            compare_dates(current->expiry_date, current_date) >= 0) {
            
            if (best_batch == NULL || 
                compare_dates(current->expiry_date, best_batch->expiry_date) < 0) {
                best_batch = current;
            }
        }
        current = current->next;
    }
    
    // If no valid batch found
    if (best_batch == NULL) {
        printf("%s\n", messages->no_stock);
        return;
    }
    
    // Check if already vaccinated with this vaccine today
    InoculationNode *inoc = inoculations_head;
    while (inoc) {
        if (strcmp(inoc->user_name, user_name) == 0 && 
            strcmp(inoc->vaccine_name, vaccine_name) == 0 && 
            strcmp(inoc->application_date, current_date) == 0) {
            
            printf("%s\n", messages->already_vaccinated);
            return;
        }
        inoc = inoc->next;
    }
    
    // Create new inoculation record
    InoculationNode *new_inoculation = create_inoculation(
        user_name, vaccine_name, best_batch->batch, current_date);
    
    if (!new_inoculation) {
        printf("%s\n", messages->memory_error);
        return;
    }
    
    // Add to beginning of list
    new_inoculation->next = inoculations_head;
    inoculations_head = new_inoculation;
    inoculation_count++;
    
    // Update the vaccine batch
    best_batch->doses--;
    best_batch->applications++;
    
    printf("%s\n", best_batch->batch);
}

void remove_vaccine_batch(char *batch) {
    VaccineBatchNode *current = vaccine_batches_head;
    VaccineBatchNode *prev = NULL;
    
    while (current) {
        if (strcmp(current->batch, batch) == 0) {
            if (current->applications > 0) {
                printf("%d doses already applied\n", current->applications);
                return;
            }
            
            // Remove from the list
            if (prev) {
                prev->next = current->next;
            } else {
                vaccine_batches_head = current->next;
            }
            
            free_vaccine_batch(current);
            vaccine_count--;
            printf("batch %s removed\n", batch);
            return;
        }
        
        prev = current;
        current = current->next;
    }
    
    printf("%s: %s\n", batch, messages->no_such_batch);
}

void delete_inoculation(char *user_name, char *date, char *batch) {
    int deleted_inoculations = 0;
    int user_found = 0;
    int batch_found = 0;
    
    // First check if user exists
    InoculationNode *inoc = inoculations_head;
    while (inoc) {
        if (strcmp(inoc->user_name, user_name) == 0) {
            user_found = 1;
            break;
        }
        inoc = inoc->next;
    }
    
    // If user doesn't exist, report it
    if (!user_found) {
        printf("%s: %s\n", user_name, messages->no_such_user);
        return;
    }
    
    // Check if date is valid (if provided)
    if (date != NULL) {
        if (!is_valid_date(date) && strcmp(date, current_date) != 0) {
            printf("%s\n", messages->invalid_date);
            return;
        }
    }
    
    // Check if batch exists (if provided)
    if (batch != NULL) {
        VaccineBatchNode *current = vaccine_batches_head;
        while (current) {
            if (strcmp(current->batch, batch) == 0) {
                batch_found = 1;
                break;
            }
            current = current->next;
        }
        
        if (!batch_found) {
            printf("%s: %s\n", batch, messages->no_such_batch);
            return;
        }
    }
    
    // Now proceed with deletion
    InoculationNode *current = inoculations_head;
    InoculationNode *prev = NULL;
    
    while (current) {
        int should_delete = 0;
        
        if (strcmp(current->user_name, user_name) == 0 && 
            (date == NULL || strcmp(current->application_date, date) == 0) && 
            (batch == NULL || strcmp(current->batch, batch) == 0)) {
            should_delete = 1;
        }
        
        if (should_delete) {
            // Update the linked list
            if (prev) {
                prev->next = current->next;
            } else {
                inoculations_head = current->next;
            }
            
            InoculationNode *to_delete = current;
            current = current->next;
            
            free_inoculation(to_delete);
            deleted_inoculations++;
            inoculation_count--;
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    printf("%d inoculations deleted\n", deleted_inoculations);
}

void list_inoculations(char *user_name) {
    // If no specific user is requested, list all inoculations
    if (user_name == NULL) {
        InoculationNode *current = inoculations_head;
        while (current) {
            int day, month, year;
            parse_date(current->application_date, &day, &month, &year);
            printf("%s %s %02d-%02d-%d\n", 
                  current->user_name, 
                  current->batch, 
                  day, month, year);
            current = current->next;
        }
        return;
    }
    
    // Check if the user exists and list their inoculations
    int user_found = 0;
    InoculationNode *current = inoculations_head;
    
    while (current) {
        if (strcmp(current->user_name, user_name) == 0) {
            int day, month, year;
            parse_date(current->application_date, &day, &month, &year);
            printf("%s %s %02d-%02d-%d\n", 
                  current->user_name, 
                  current->batch, 
                  day, month, year);
            user_found = 1;
        }
        current = current->next;
    }
    
    // If the user doesn't exist, display the appropriate message
    if (!user_found && user_name != NULL) {
        printf("%s: %s\n", user_name, messages->no_such_user);
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
        free(current_date);
        current_date = strdup(new_date);
        
        int day, month, year;
        parse_date(current_date, &day, &month, &year);
        printf("%02d-%02d-%d\n", day, month, year);
    } else {
        printf("%s\n", messages->invalid_date);
    }
}

int main(int argc, char *argv[]) {
    char command;
    
    // Default to English
    messages = &en_messages;
    
    // Check for language argument
    if (argc > 1 && strcmp(argv[1], "pt") == 0) {
        messages = &pt_messages;
    }
    
    // Initialize current date with dynamic memory
    current_date = strdup("01-01-2025");
    if (!current_date) {
        printf("%s\n", messages->memory_error);
        return 1;
    }
    
    while (scanf(" %c", &command) != EOF) {
        switch (command) {
            case 'q':
                // Free all allocated memory before exiting
                free_all_vaccine_batches();
                free_all_inoculations();
                free(current_date);
                return 0;
            
            case 'c': {
                char batch[MAX_BATCH_NAME], expiry_date[MAX_DATE], vaccine_name[MAX_VACCINE_NAME];
                int doses;
                int scan_result = scanf("%s %s %d %s", batch, expiry_date, &doses, vaccine_name);
                
                if (scan_result == 4) {
                    add_vaccine_batch(batch, expiry_date, doses, vaccine_name);
                } else {
                    // Clear the input buffer
                    char c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("%s\n", messages->invalid_batch);
                }
                break;
            }
            
            case 'l': {
                char line[1024]; // Larger buffer for safety
                char *vaccine_names[10000]; // Support many vaccine names
                int vaccine_names_count = 0;
                
                // Read the entire line of input
                if (fgets(line, sizeof(line), stdin) != NULL) {
                    // Skip initial spaces
                    char *start = line;
                    while (*start == ' ' || *start == '\t') start++;
                    
                    // Remove trailing newline
                    char *newline = strchr(start, '\n');
                    if (newline) *newline = '\0';
                    
                    // If line is empty after skipping spaces, list all vaccines
                    if (*start == '\0') {
                        list_vaccine_batches(NULL, 0);
                    } else {
                        // Parse vaccine names
                        char *token = strtok(start, " \t\n");
                        while (token != NULL && vaccine_names_count < 10000) {
                            vaccine_names[vaccine_names_count] = strdup(token);
                            if (!vaccine_names[vaccine_names_count]) {
                                printf("%s\n", messages->memory_error);
                                // Free already allocated names
                                for (int i = 0; i < vaccine_names_count; i++) {
                                    free(vaccine_names[i]);
                                }
                                return 1;
                            }
                            vaccine_names_count++;
                            token = strtok(NULL, " \t\n");
                        }
                        
                        list_vaccine_batches(vaccine_names, vaccine_names_count);
                        
                        // Free dynamically allocated vaccine names
                        for (int i = 0; i < vaccine_names_count; i++) {
                            free(vaccine_names[i]);
                        }
                    }
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
                            printf("%s\n", messages->invalid_name);
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
                            printf("%s\n", messages->invalid_name);
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
                            *end_quote = '\0';
                            strcpy(user_name, start);
                            list_inoculations(user_name);
                        } else {
                            printf("%s\n", messages->invalid_name);
                        }
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
    
    // Free all memory when the program exits unexpectedly
    free_all_vaccine_batches();
    free_all_inoculations();
    free(current_date);
    
    return 0;
}
