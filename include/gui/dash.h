
#ifndef DASH_H
#define DASH_H

#include <stdint.h>

#define MAX_DASH_RESULTS 32
#define DASH_SEARCH_LEN 128

typedef enum {
    DASH_RESULT_APP,
        DASH_RESULT_FILE,
            DASH_RESULT_SETTING
            } dash_result_type_t;

            typedef struct {
                dash_result_type_t type;
                    char name[64];
                        char description[128];
                            char path[256];
                                uint8_t *icon_data;
                                    float relevance;
                                    } dash_result_t;

                                    typedef struct {
                                        char search_query[DASH_SEARCH_LEN];
                                            dash_result_t results[MAX_DASH_RESULTS];
                                                int result_count;
                                                    int selected_index;
                                                        uint8_t visible;
                                                            uint8_t searching;
                                                            } dash_t;

                                                            // Dash functions
                                                            void dash_init(void);
                                                            void dash_show(void);
                                                            void dash_hide(void);
                                                            void dash_render(void);
                                                            void dash_search(const char *query);
                                                            void dash_handle_key(char key);
                                                            void dash_handle_click(int x, int y);
                                                            void dash_execute_selected(void);

                                                            #endif
