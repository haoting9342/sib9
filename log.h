
#define LOG_LEVEL_DEBUG 0 
#define LOG_LEVEL_INFO 1 
#define LOG_LEVEL_WARN 2 
#define LOG_LEVEL_ERROR 3 
#define LOG_LEVEL_FATAL 4

#define LOG_CATEGORY_NETWORK 1 
#define LOG_CATEGORY_DATABASE 2 
#define LOG_CATEGORY_SECURITY 4 
#define LOG_CATEGORY_PERFORMANCE 8


void log_message(int level, int category, const char *message) {
     // Get the current logging level and category from some global variables or configuration files 
     int current_level = get_current_level(); 
     int current_category = get_current_category();
     // Check if the message level and category match the current level and category 
     if (level >= current_level && (category & current_category) != 0) { 
        // Print the message with some prefix or suffix indicating the level and category 
        printf(“[%d][%d] %s\n”, level, category, message); 
    } 
}