#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Data structure
typedef struct command {
    char** lines;
    int ind1;
    int ind2;
    char identifier;
    int deleted;
} cmd;

//Global variables
char** array = NULL; //array of pointers to the sentences
int total_length = 0; //total amount of pointers in the array
int effective_length = 0; //number of pointers to an actual sentence
short int flag_undo = 0;
short int flag_count = 0;
int count = 0;
int stack_length = 0;
int stack_total_length = 0;
cmd* stack_first = NULL;
cmd* stack_pointer = NULL;
cmd* stack_top = NULL;

//Prototypes
void menu();
char* str_input();
void change(int, int, char);
void del(int, int, char);
void print(int, int);
void stack_insert(int, int, char);
void stack_free();
void undo(int);
void undo_add(int ind1, cmd *command);
void undo_delete_overwrite(int ind1, int ind2);
void redo(int);
void redo_add_overwrite(int ind1, int ind2);
void redo_delete(int ind1, cmd *command);
void undo_reset();
void undo_or_redo();
void array_insert(int);
void array_resize();

int main() {
    menu();
    return 0;
}

void menu()
{
    int av_undo = 0;
    int av_redo = 0;
    int ind1 = 0, ind2 = 0;
    char identifier;
    char command[100];
    while (1)
    {
        fgets(command, 100, stdin);
        if (command[0] == 'q') //format /#1
        {
            break;
        }
        else //format #2 or #3
        {
            if (strchr(command, ',') != NULL) //comma => format #3
            {
                sscanf(command, "%d,%d%c", &ind1, &ind2, &identifier);
            }
            else //no comma => format #2
            {
                sscanf(command, "%d%c", &ind1, &identifier);
            }
            switch (identifier)
            {
                case 'c':
                    if (flag_count == 1)
                    {
                        undo_or_redo();
                    }
                    if (flag_undo == 1)
                        undo_reset();
                    change(ind1, ind2, identifier);
                    break;

                case 'd':
                    if (flag_count == 1)
                    {
                        undo_or_redo();
                    }
                    if (flag_undo == 1)
                        undo_reset();
                    del(ind1, ind2, identifier);
                    break;

                case 'p':
                    if (flag_count == 1)
                    {
                        undo_or_redo();
                    }
                    print(ind1, ind2);
                    break;

                case 'u':
                    if (flag_count == 1)
                    {
                        if (ind1 <= av_undo)
                        {
                            count = count + ind1;
                            av_undo = av_undo - ind1;
                            av_redo = av_redo + ind1;
                        }
                        else
                        {
                            count = count + av_undo;
                            av_redo = av_redo + av_undo;
                            av_undo = 0;
                        }
                    }
                    else //init
                    {
                        if (ind1 <= stack_pointer - stack_first)
                        {
                            count = ind1;
                            av_undo = stack_pointer - stack_first + 1 - ind1;
                            av_redo = stack_top - stack_pointer + ind1;
                        }
                        else
                        {
                            count = stack_pointer - stack_first + 1;
                            av_undo = 0;
                            av_redo = stack_length;
                        }
                        flag_count = 1;
                    }
                    break;

                case 'r':
                    if (flag_count == 1)
                    {
                        if (ind1 <= av_redo)
                        {
                            count = count - ind1;
                            av_undo = av_undo + ind1;
                            av_redo = av_redo - ind1;
                        }
                        else
                        {
                            count = count - av_redo;
                            av_undo = av_undo + av_redo;
                            av_redo = 0;
                        }
                    }
                    else //init
                    {
                        if (ind1 <= stack_top - stack_pointer)
                        {
                            count = -ind1;
                            av_undo = stack_pointer - stack_first + 1 + ind1;
                            av_redo = stack_top - stack_pointer - ind1;
                        }
                        else
                        {
                            count = -(stack_top - stack_pointer);
                            av_undo = stack_length;
                            av_redo = 0;
                        }
                        flag_count = 1;
                    }
                    break;

                default:
                    printf("Invalid command\n");
            }

        }
    }
}

void array_resize()
{
    char** check;
    int increment = 1000;
    if (total_length == effective_length)
    {
        check = (char**)realloc(array, sizeof(char*) * (total_length + increment));
        if (check != NULL)
        {
            array = check;
            total_length = total_length + increment;
        }
        else
            printf("Memory allocation error\n");
    }
}

void array_insert(int index)
{
    array_resize();
    array[index] = str_input();
    effective_length++;
}

void undo_or_redo()
{
    if (count > 0)
        undo(count);
    else if (count < 0)
        redo(-count);

    flag_count = 0;
    count = 0;
}

//Reads from stdin the sentences of a change command
char* str_input()
{
    char buffer[1024];
    char* str = NULL;
    fgets(buffer, 1024, stdin);
    int len = sizeof(char) * (strlen(buffer) + 1);
    str = (char*)malloc(len);
    if (str != NULL)
    {
        memcpy(str, buffer, len);
        return str;
    }
    else
    {
        printf("Memory allocation error\n");
        return NULL;
    }
}

//Reads a total of ind2-ind1+1 lines
void change(int ind1, int ind2, char identifier)
{
    int to_overwrite;
    //START UNDO
    stack_insert(ind1, ind2, identifier);
    //END UNDO

    if (ind1 - 1 != effective_length) //OVERWRITE OR HYBRID
    {
        //Saves the overridden sentences
        to_overwrite = ind2 - ind1 + 1;
        if (to_overwrite > effective_length - ind1 + 1)
            to_overwrite = effective_length - ind1 + 1;

        stack_pointer->lines = (char**)malloc(sizeof(char*) * to_overwrite);
        stack_pointer->deleted = to_overwrite;
        memcpy(stack_pointer->lines, array + ind1 - 1, sizeof(char*) * to_overwrite);
    }
    for (int i = 0; i < ind2 - ind1 + 1; i++)
    {
        if (ind1 + i - 1 >= effective_length) //ADD
        {
            array_insert(ind1 - 1 + i);
        }
        else  //OVERWRITE
        {
            array[ind1 - 1 + i] = str_input();
        }
    }
    getchar_unlocked(); //Removes "."
    getchar_unlocked(); //Removes "/n"
}

//Deletes a total of ind2-ind1+1 lines
void del(int ind1, int ind2, char identifier)
{
    int to_delete = ind2 - ind1 + 1;
    if (to_delete > effective_length - ind1 + 1)
        to_delete = effective_length - ind1 + 1;

    if (ind1 > 0 && ind1 <= effective_length)
    {
        //START UNDO);
        stack_insert(ind1, ind2, identifier);
        //END UNDO

        stack_pointer->lines = (char**)malloc(sizeof(char*) * to_delete);
        memcpy(stack_pointer->lines, array + ind1 - 1, sizeof(char*) * to_delete);
        effective_length = effective_length - to_delete;
        stack_pointer->deleted = to_delete;
        memmove(array + ind1 - 1, array + ind1 + to_delete - 1, sizeof(char*) * (effective_length - ind1 + 1));
    }
    else //command has no effect, save a token
    {
        //START UNDO
        stack_insert(ind1, ind2, 'N');
        //END UNDO
    }
}

void print(int ind1, int ind2)
{
    int is_zero = 0;
    if (ind1 == 0) is_zero = 1;

    for (int i = 0; i < ind2 - ind1 + 1; i++)
    {
        if (is_zero == 1)
        {
            fputs(".\n", stdout);
            is_zero = 0;
        }
        else if (ind1 - 1 + i >= effective_length || array[ind1 - 1 + i] == NULL)
        {
            fputs(".\n", stdout);
        }
        else
        {
            fputs(array[ind1 - 1 + i], stdout);
        }
    }
}

//Add a node at the head of the list
void stack_insert(int ind1, int ind2, char identifier)
{
    cmd* check;
    int increment = 1000;
    if (stack_length == stack_total_length)
    {
        check = (cmd*)realloc(stack_first, sizeof(cmd) * (stack_total_length + increment));
        if (check != NULL)
        {
            stack_first = check;
            stack_total_length = stack_total_length + increment;
        }
        else
            printf("Memory allocation error\n");
    }
    cmd* newCommand = stack_first + stack_length;
    newCommand->ind1 = ind1;
    newCommand->ind2 = ind2;
    newCommand->identifier = identifier;
    newCommand->lines = NULL;
    newCommand->deleted = 0;
    stack_top = newCommand;
    stack_pointer = newCommand;
    stack_length++;
}

//Deletes the first node of the list
void stack_free()
{
    if (stack_pointer <= stack_top)
    {
        stack_top--;
        stack_length--;
    }
}

//Does #index undo
void undo(int index)
{
    flag_undo = 1;
    for (int i = 0; i < index; i++)
    {
        if (stack_pointer >= stack_first)
        {
            switch (stack_pointer->identifier)
            {
                case 'c':
                    undo_delete_overwrite(stack_pointer->ind1, stack_pointer->ind2); break;
                case 'd':
                    undo_add(stack_pointer->ind1, stack_pointer); break;
                default: break;
            }
            stack_pointer--;
        }
        else break; //the command has no effect
    }
}

void undo_add(int ind1, cmd *command)
{
    //Increases the length of the array to insert the new lines
    memmove(array + ind1 - 1 + command->deleted, array + ind1 - 1, sizeof(char*) * (effective_length - ind1 + 1));
    effective_length = effective_length + command->deleted;

    //Copies that lines into the array
    memcpy(array + ind1 - 1, stack_pointer->lines, sizeof(char*) * stack_pointer->deleted);
}

void undo_delete_overwrite(int ind1, int ind2)
{
    if (stack_pointer->deleted == 0) //DELETE
    {
        stack_pointer->lines = (char**)malloc(sizeof(char*) * (ind2 - ind1 + 1));
        memcpy(stack_pointer->lines, array + ind1 - 1, sizeof(char*) * (ind2 - ind1 + 1));
        effective_length = effective_length - (ind2 - ind1 + 1);
    }
    else //OVERWRITE o HYBRID
    {
        //Saves the previous lines
        char** temp = (char**)malloc(sizeof(char*) * (ind2 - ind1 + 1));
        memcpy(temp, array + ind1 - 1, sizeof(char*) * (ind2 - ind1 + 1));

        //Overrides with the new lines
        memcpy(array + ind1 - 1, stack_pointer->lines, sizeof(char*) * stack_pointer->deleted);
        effective_length = effective_length - ((ind2 - ind1 + 1) - stack_pointer->deleted);

        //Updates the pointer to the previous lines
        free(stack_pointer->lines);
        stack_pointer->lines = temp;
    }
}

//Does #index redo
void redo(int index)
{
    for (int i = 0; i < index; i++)
    {
        if (stack_pointer < stack_top)
        {
            stack_pointer++;
            switch (stack_pointer->identifier)
            {
                case 'c':
                    redo_add_overwrite(stack_pointer->ind1, stack_pointer->ind2); break;
                case 'd':
                    redo_delete(stack_pointer->ind1, stack_pointer); break;
                default: break;
            }
        }
        else break; //the command has no effect
    }
    //Undo reset section => no commands are lost
    if (stack_pointer == stack_top)
        flag_undo = 0;
}

void redo_add_overwrite(int ind1, int ind2)
{
    if (ind1 - 1 == effective_length) //ADD
    {
        memcpy(array + ind1 - 1, stack_pointer->lines, sizeof(char*) * (ind2 - ind1 + 1));
        effective_length = effective_length + (ind2 - ind1 + 1);
    }
    else //OVERWRITE o HYBRID
    {
        //Saves the previous lines
        char** temp = (char**)malloc(sizeof(char*) * stack_pointer->deleted);
        memcpy(temp, array + ind1 - 1, sizeof(char*) * stack_pointer->deleted);

        //Overrides with the new lines
        memcpy(array + ind1 - 1, stack_pointer->lines, sizeof(char*) * (ind2 - ind1 + 1));
        effective_length = effective_length + ((ind2 - ind1 + 1) - stack_pointer->deleted);

        //Updates the pointer to the previous lines
        free(stack_pointer->lines);
        stack_pointer->lines = temp;
    }
}

void redo_delete(int ind1, cmd *command)
{
    //DELETE
    effective_length = effective_length - command->deleted;
    memmove(array + ind1 - 1, array + ind1 + command->deleted - 1, sizeof(char*) * (effective_length - ind1 + 1));
}

void undo_reset()
{
    if (stack_pointer <= stack_top)
    {
        while (stack_top != stack_pointer)
        {
            stack_free();
        }
    }
    flag_undo = 0;
}