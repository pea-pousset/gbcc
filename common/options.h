/**
 * \addtogroup Commons
 * \{
 * \addtogroup Options
 * \{
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#define GBCC        0   /**< gbcc program id */
#define GBAS        1   /**< gbas program id */
#define GBLD        2   /**< gbld program id */
#define NUM_OPTIONS 7

typedef enum
{
    flag,
    number,     /**< The option requires a numeric parameter */
    string      /**< The option requires a string parameter */
} option_type_t;

typedef union
{
    int   num;
    char* str;
} option_value_t;

typedef struct
{
    const char*     name;   /**< Option tag name */
    option_type_t   type;   /**< Value type */
    option_value_t  value;  /**< Default value */
    const char*     argname;/**< Description of the argument */
    int             set;    /**< 1 means the default value has been modified */
    int             as_opt; /**< 1 means this option is used by the assembler */
    int             ld_opt; /**< 1 means this option is used by the linker */
} option_t;

extern option_t options[NUM_OPTIONS];

void      parse_options(int argc, char** argv, int program, void(*help)(),
                        void(*version)());
option_t* get_option(const char* name);
char*     gen_options_str(int program);

#endif

/**
 * \} Options
 * \} Commons
 */
