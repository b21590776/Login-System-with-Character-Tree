#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct node{
    char letter;
    char* password;
    struct node* sibling;
    struct node* child;
} NODE;

NODE* new_node(char letter){
    NODE* toReturn = malloc(sizeof(NODE));
    toReturn->letter = letter;
    toReturn->child = NULL;
    toReturn->sibling = NULL;
    toReturn->password = NULL;
    return toReturn;
}


NODE* find_user(NODE *root, char *name){
    int i;
    NODE* current = root;
    for (i = 0; i < strlen(name) && current; ++i) {
        current = current->child;
        while (current && current->letter != name[i]){
            current = current->sibling;   /*  Walk through the linked list    */
        }
    }
    return current;
}


NODE* last_child(NODE* node){
    node = node->child;
    while (node && node->sibling){
        node = node->sibling;
    }
    return node;
}


int add_user(NODE* root, char* name, char* password){
    int i;
    NODE* current = root;
    NODE* next;
    NODE* temp;
    for (i = 0; i < strlen(name) && current; ++i) {
        next = current->child;
        while (next && next->letter != name[i]){
            next = next->sibling;   /*  Walk through the linked list    */
        }
        if (!next){ /*The tree did not come this far    */
            next = new_node(name[i]);
            temp = last_child(current);
            if(temp){
                temp->sibling = next;
            }else{
                current->child = next;
            }
        }
        current = next;
    }
    if(current->password){
        return 0;   /*  user exists */
    } else{
        current->password = password;
        return 1;
    }
}


int delete_user(NODE* current, char* name, int idx){    /*  idx is 0 when current is root   */
    NODE* next = current->child;
    NODE* prev = NULL;
    int result;
    while(next){
        if(next->letter == name[idx]){
            if(idx == strlen(name) -1){ /*  Reached the node to be deleted  */
                if(!next->child){   /*  It has no children, so we can simply delete it  */
                    if(prev){
                        prev->sibling = next->sibling;
                    } else{
                        current->child = next->sibling;
                    }
                    free(next);
                    return 1;
                } else{
                    next->password = NULL;
                    return 1;
                }
            } else{
                result = delete_user(next, name, idx+1);
                if (result){    /*  The node was found and deleted  */
                    if(!next->child){   /*  Its only child was deleted  */
                        if(prev){
                            prev->sibling = next->sibling;
                        } else{
                            current->child = next->sibling;
                        }
                        free(next);
                        return 1;
                    }
                    return 1;
                }
                return 0;
            }
        } else{
            prev = next;
            next = next->sibling;
        }
    }
    return 0;

}


void print_tabs(int level,FILE* output){
    /* For 0th and 1st levels branch with tabs, otherwise separate them with commas.    */
    int i;
    if (level <= 1) {
        fprintf(output, "\n");

        for (i = 0; i < level; ++i) {
            fprintf(output, "\t");
        }
        fprintf(output, "-");
        return;
    }
    fprintf(output, ",");

}




void list_tree(NODE* current, int level, char* word, FILE* output){
    size_t length = strlen(word);
    NODE* child = NULL;
    if(current->letter != '\0'){
        word = realloc(word, sizeof(char)*length + 2);
        word[length] = current->letter;
        word[length + 1] = '\0';
    }

    if (!current->child){
        fprintf(output, "%s", word); /*  Reached a leaf  */
    } else{
        if (!current->child->sibling){   /*  Has only one child, this means we will just continue    */
            if (current->password){
                fprintf(output, "%s,", word);
            }
            list_tree(current->child, level, word, output);
        } else{ /*  Tree will split */
            if (current->letter != '\0'){
                fprintf(output, "%s", word);
            }
            child = current->child;
            while(child){
                print_tabs(level, output);
                list_tree(child, level+1, word, output);
                child = child->sibling;
            }
        }

    }
    word = realloc(word, sizeof(char)*(length + 1));
    word[length] = '\0';

}



char** readFile(char* path){
    FILE *file;
    file = fopen(path, "r");
    char character = 'c';
    char** lines;
    char* thisLine;
    int size, length;

    lines = malloc(1*sizeof(char*));
    thisLine = malloc(1* sizeof(char));

    length = 0;
    size = 1;   /*  First index for writing the size of the file    */

    while (character != EOF) {
        thisLine = realloc(thisLine, (++length * sizeof(char)));
        character = (char) fgetc(file);
        if (character != '\n' && character != EOF) {
            thisLine[length - 1] = character;
        } else { /* end of line */
            thisLine[length - 1] = '\0'; /* null terminator */
            if (length > 1) {
                size++;
                lines = realloc(lines, size * (sizeof(char *))); /* the current line to the array */
                lines[size - 1] = thisLine;
            }
            length = 0;
            thisLine = malloc(length * sizeof(char)); /* a new array for the next line */

        }
    }


    lines[0] = malloc(sizeof(char) * 10);
    sprintf(lines[0], "%d", size-1);
    fclose(file);
    return lines;

}


int child_exists(NODE* root, char letter){
    NODE* child = root->child;
    while(child){
        if(child->letter == letter){
            return 1;
        }
        child = child->sibling;
    }
    return 0;
}


void execute_command(NODE* root, char* command, FILE* output){
    size_t d = strlen(command);
    char* word;
    word = strtok(command, " ");
    NODE* temp;
    char* password;
    int result;

    if (word[1] == 'a'){
        word = strtok(NULL, " ");
        password = strtok(NULL, " ");
        fprintf(output, "\"%s\" ", word);
        result = add_user(root, word, password);
        if (!result){
            fprintf(output, "reserved username\n");
        } else {
            fprintf(output, "was added\n");
        }

    } else if (word[1] == 's'){
        word = strtok(NULL, " ");
        fprintf(output, "\"%s\" ", word);
        temp = find_user(root, word);
        if (temp){
            if (temp->password){
                fprintf(output, "password \"%s\"\n", temp->password);
            } else {
                fprintf(output, "not enough username\n");
            }
        } else {
            if (child_exists(root, word[0])){
                fprintf(output, "incorrect username\n");
            } else {
                fprintf(output, "no record\n");
            }
        }

    } else if (word[1] == 'q'){
        word = strtok(NULL, " ");
        fprintf(output, "\"%s\" ", word);
        temp = find_user(root, word);
        if (temp){
            if (temp->password){
                password = strtok(NULL, " ");
                if (strcmp(password, temp->password)) {
                    fprintf(output, "incorrect password\n");
                } else {
                    fprintf(output, "successful login\n");
                }

            } else {
                fprintf(output, "not enough username\n");
            }
        } else {
            if (child_exists(root, word[0])){
                fprintf(output, "incorrect username\n");
            } else {
                fprintf(output, "no record\n");
            }
        }
    } else if (word[1] == 'd'){
        word = strtok(NULL, " ");
        fprintf(output, "\"%s\" ", word);
        temp = find_user(root, word);
        if (temp){
            if (temp->password){
                delete_user(root, word, 0);
                fprintf(output, "deletion is successful\n");
            } else {
                fprintf(output, "not enough username\n");
            }
        } else {
            if (child_exists(root, word[0])){
                fprintf(output, "incorrect username\n");
            } else {
                fprintf(output, "no record\n");
            }
        }
    } else if (word[1] == 'l'){
        word = malloc(sizeof(char));
        word[0] = '\0';
        list_tree(root, 0, malloc(0), output);
        fprintf(output, "\n");
    }
}


int main() {
    NODE* root = new_node('\0');
    NODE* temp;
    char** input = readFile("input.txt");
    int length = atoi(input[0]);
    char* line;
    int i;
    FILE* output = fopen("output.txt", "w+");
    for ( i = 1; i < length+1; ++i) {
        execute_command(root, input[i], output);
    }
    fclose(output);

    return 0;
}