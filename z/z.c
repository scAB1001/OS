#include <stdio.h>

// Get the length of a string
int count_chars(char *str)
{
    int length = 0;
    while (*str != '\0')
    {
        length++;
        str++;
    }
    return length;
}

// Get the length of the array of strings
int count_strings(char **str)
{
    int count = 0;
    while (*str != NULL)
    {
        count++;
        str++;
    }
    return count;
}

// Remove a final spaces from a string
void rm_trailing_whitespace(char *str)
{
    // - 1 as the actual last is '\0'
    int strLen = count_chars(str) - 1;
    char lastChar = str[strLen];

    if (lastChar == ' ')
        str[strLen] = '\0';
}

// Remove consecutive spaces from a string
void rm_consecutive_whitespace(char *str)
{
    // Store output
    char output[512];
    // Track consecutive spaces (single spaces eg "cd .." are allowed)
    int numSpaces = 0, i;

    // Iterate until a null terminator is reached
    for (i = 0; str[i]; i++)
    {
        // Copy cur char or prev non-space chars (not the first)
        if (str[i] != ' ' || (i > 0 && str[i - 1] != ' '))
            output[i - numSpaces] = str[i];
        else
            numSpaces++;
    }

    output[i - numSpaces] = '\0';

    // Copy the modified string back to str
    for (i = 0; output[i]; i++)
        str[i] = output[i];

    // Remove trailing (consecutive) whitespace
    str[i] = '\0';
}

// Group function to remove all possible excess whitespace
void rm_whitespace(char *str)
{
    rm_consecutive_whitespace(str);
    rm_trailing_whitespace(str);
    printf("Cleaned str:\t'%s'\n", str);
}

// Tokenize a cleaned string input
void tokenize_string(char *str, char **tokens)
{
    int i = 0;
    while (*str != '\0')
    {
        // Store the start of the token
        tokens[i] = str;

        // Find the end of the token
        while (*str != ' ' && *str != '\0')
            str++;

        // End the token when a space is found
        if (*str == ' ')
        {
            *str = '\0';
            str++;
        }

        // Proceed to the next token
        i++;
    }

    // Ensure the last element is NULL to mark the end of tokens
    tokens[i] = NULL;
}

void display_tokens(char** tokens)
{
    printf("Tokens:\t\t");
    //
    for (int i = 0; i < count_strings(tokens); i++)
        printf("'%s'\t", tokens[i]);

    /*
    int i = 0;
    while (tokens[i] != NULL)
    {
        printf("'%s'\t", tokens[i]);
        i++;
    }
    */
    printf("\n");
}


int main(int argc, char **argv)
{
    // >> "   cd    ..    "
    //printf("\n>>> ");
    char *str = argv[1], *tokens[10];

    rm_whitespace(str);
    tokenize_string(str, tokens);
    display_tokens(tokens);
    
    return 0;
}

void ptr_help()
{
    // ptr to a char, used to point to a single string.
    char *str1 = "Hello";
    printf("str1: %s\n", str1);

    // ptr to a ptr of a char, used to point to an array of strings.
    char *strings[] = {"Hello", "World"};
    char **str2 = strings;
    printf("str2[0]: %s\n", str2[0]);
    printf("str2[1]: %s\n", str2[1]);

    // an array of chars, where the size of the array is specified.
    char str3[] = "Hello";
    printf("str3: %s\n", str3);
}