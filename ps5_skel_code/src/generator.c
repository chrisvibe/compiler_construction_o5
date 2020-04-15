
            /* printf("\tmovq $errgen, %s\n", record[0]); */
            /* puts("\tcall puts"); */

#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
#define return_rec "%rax"

static void hello_world ( symbol_t *symbol );
static void generate_stringtable ( void );
static void generate_main ( symbol_t *first );
void generate_program ( void );
static void generate_function ( node_t* node );
void node_to_assembly( node_t* node );
void tree_to_assembly(node_t* node);
void handle_global_list();
void handle_print_statement(node_t* print_statement);

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};


static void
generate_stringtable ( void )
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */ 
    puts ( ".section .rodata" );
    puts ( "intout: .string \"\%ld\"" );
    puts ( "strout: .string \"\%s\"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );
    puts ( "errprint: .string \"Cant print this symbol\"" );
    puts ( "errgen: .string \"GENERIC ERROR!!!\"" );

    /* TODO:  handle the strings from the program */

    /* add string list variables */ 
    for(int i = 0; i < stringc; i++) {
        char* str = string_list[i];
        printf( "STR%i: .string %s\n", i, str);
    }
}


static void
generate_main ( symbol_t *first )
{
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tpushq %rbp" );
    puts ( "\tmovq %rsp, %rbp" );

    puts ( "\tsubq $1, %rdi" );
    printf ( "\tcmpq $%zu, %%rdi\n", first->nparms );
    puts ( "\tjne ABORT" );
    puts ( "\tcmpq $0, %rdi" );
    puts ( "\tjz SKIP_ARGS" );

    puts ( "\tmovq %rdi, %rcx" );
    printf ( "\taddq $%zu, %%rsi\n", 8*first->nparms );
    puts ( "PARSE_ARGV:" );
    puts ( "\tpushq %rcx" );
    puts ( "\tpushq %rsi" );

    puts ( "\tmovq (%rsi), %rdi" );
    puts ( "\tmovq $0, %rsi" );
    puts ( "\tmovq $10, %rdx" );
    puts ( "\tcall strtol" );

    /*  Now a new argument is an integer in rax */
    puts ( "\tpopq %rsi" );
    puts ( "\tpopq %rcx" );
    puts ( "\tpushq %rax" );
    puts ( "\tsubq $8, %rsi" );
    puts ( "\tloop PARSE_ARGV" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    puts ( "SKIP_ARGS:" );
    printf ( "\tcall _%s\n", first->name );
    puts ( "\tjmp END" );
    puts ( "ABORT:" );
    puts ( "\tmovq $errout, %rdi" );
    puts ( "\tcall puts" );

    puts ( "END:" );
    puts ( "\tmovq %rax, %rdi" );
    puts ( "\tcall exit" );

}


void
generate_program ( void )
{
    generate_stringtable();

/* At runtime this has to be done: */
/*  Find the count of arguments */
/*  If there are some translate them from text to numbers */
/*  Put them in the right places for an ordinary call */
/*  Call the 1st function defined in the VSL source program */
    node_to_assembly(root);
/*  Take the return value from that and return it to the calling shell */
/*  Return to shell */
}
    
static void generate_function ( node_t* node ) {
    // make function label
    printf( "_%s:\n", (char*)node->children[0]->data);

    // push run-time vars to stack
    puts( "\tpushq %rbp" );
    puts( "\tmovq %rsp, %rbp" );
    
    // generate vsl code from node tree
    tree_to_assembly(node);

    // exit
    puts ( "\tcall exit" );
}

void handle_global_list() {
    size_t n_globals = tlhash_size(global_names);
    symbol_t* global_list[n_globals];
    tlhash_values(global_names, (void **)&global_list);
    symbol_t* first = NULL;

    puts ( ".section .data" ); // mutable data
    for(int i = 0; i < n_globals; i++) {
        symbol_t* sym = global_list[i];
        if (sym->type == SYM_GLOBAL_VAR) { // add global variables (placeholders)
            printf( "_%s: .zero 8\n", sym->name);
        } else if (sym->type == SYM_FUNCTION && !first) {
            first = sym;
        }
    }
    generate_main(first);
}

void node_to_assembly( node_t* node ) {
    if (node) {
        switch(node->type) {
            case PROGRAM:
                tree_to_assembly(node);
                break;
            case GLOBAL_LIST:
                handle_global_list();
                tree_to_assembly(node);
                break;
            case GLOBAL:
                tree_to_assembly(node);
                break;
            case STATEMENT_LIST:
                tree_to_assembly(node);
                break;
            case PRINT_LIST:
                tree_to_assembly(node);
                break;
            case EXPRESSION_LIST:
                tree_to_assembly(node);
                break;
            case VARIABLE_LIST:
                tree_to_assembly(node);
                break;
            case ARGUMENT_LIST:
                tree_to_assembly(node);
                break;
            case PARAMETER_LIST:
                tree_to_assembly(node);
                break;
            case DECLARATION_LIST:
                tree_to_assembly(node);
                break;
            case FUNCTION:
                generate_function(node);
                break;
            case STATEMENT:
                tree_to_assembly(node);
                break;
            case BLOCK:
                tree_to_assembly(node);
                break;
            case RETURN_STATEMENT:
                node_to_assembly(node->children[0]);
                break;
            case ASSIGNMENT_STATEMENT: ;
                // identifier always first child of assignment_statement
                symbol_t* identifier = node->children[0]->entry;
                switch (identifier->type) {
                    case SYM_LOCAL_VAR:
                        node_to_assembly(node->children[1]); // resolve expression
                        printf("\tmovq %s, %li(%%rbp)\n", return_rec, (8*identifier->seq));
                        break;
                    case SYM_GLOBAL_VAR:
                        node_to_assembly(node->children[1]); // resolve expression
                        printf("\tmovq %s, _%s\n", return_rec, identifier->name);
                        break;
                }
                break;
            case PRINT_STATEMENT:
                handle_print_statement(node);
                break;
            case NULL_STATEMENT:
                break;
            case IF_STATEMENT:
                break;
            case WHILE_STATEMENT:
                break;
            case EXPRESSION:
                break;
            case RELATION:
                if (node->n_children == 2) { // relation
                    switch (*((char*)node->data)){
                        case '+':
                            printf("addq, ");
                            node_to_assembly(node->children[0]); // arg1
                            printf(", ");
                            node_to_assembly(node->children[1]); // arg2
                            printf("\n");
                    }
                }
                break;
            case DECLARATION:
                break;
            case PRINT_ITEM:
                handle_print_statement(node);
                break;
            case NUMBER_DATA:
                printf("\tmovq $%zu, %s\n", *((size_t *)node->data), return_rec);
                break;
            case STRING_DATA:
                printf("\tmovq $STR%zu, %s\n", *((size_t *)node->data), return_rec);
                break;
            case IDENTIFIER_DATA:
                if (node->entry) {
                    switch (node->entry->type) {
                        case SYM_GLOBAL_VAR:
                            printf("\tmovq _%s, %s\n", node->entry->name, return_rec);
                            break;
                        case SYM_LOCAL_VAR:
                            printf("\tmovq %li(%%rbp), %s\n", (8*node->entry->seq), return_rec);
                            break;
                        case SYM_PARAMETER:
                            break;
                    } 
                }
                break;
        }
    }
}


void tree_to_assembly(node_t* node) {
    for (int i = 0; i < node->n_children; i++) {
        node_t* child = node->children[i];
        node_to_assembly(child);
    } 
}

void handle_print_statement(node_t* print_statement) {
    for (int i = 0; i < print_statement->n_children; i++) {
        node_t* print_item = print_statement->children[i];
        node_to_assembly(print_item);  // print_item should now reside in rax
        printf("\tmovq %s, %s\n", return_rec, record[1]);
        switch(print_item->type) {
            case STRING_DATA: // print a string 
                printf("\tmovq $strout, %s\n", record[0]);
                break;
            case NUMBER_DATA: // print an int 
                printf( "\tmovq $intout, %s\n", record[0]);
                break;
            case IDENTIFIER_DATA: // print an int 
                printf( "\tmovq $intout, %s\n", record[0]);
                break;
            case EXPRESSION: // print an int 
                printf( "\tmovq $intout, %s\n", record[0]);
                break;
            default:
                // print newline
                printf( "\tmovq $'\\n', %s\n", record[0]);
                puts( "\tcall putchar" );
                // print error msg
                printf ( "\tmovq $errprint, %s\n", record[0]);
                puts ( "\tcall puts" );
                puts( "\tcall printf" );
                break;
        }
        puts( "\tcall printf" );
        // print newline
        printf( "\tmovq $'\\n', %s\n", record[0]);
        puts( "\tcall putchar" );
    }
}


static void
hello_world ( symbol_t *symbol ) // for hello.vsl
{
    // make function label
    printf( "_%s:\n", symbol->name);
    // push run-time vars to stack
    puts( "\tpushq %rbp" );
    puts( "\tmovq %rsp, %rbp" );

    // print a string 
    puts( "\tmovq $strout, %rdi" );
    puts( "\tmovq $STR0, %rsi" );
    puts( "\tcall printf" );

    // print newline
    puts( "\tmovq $'\\n', %rdi" );
    puts( "\tcall putchar" );

    // print an int 
    puts( "\tmovq $intout, %rdi" );
    puts( "\tmovq $9, %rsi" );
    puts( "\tcall printf" );

    // print newline
    puts( "\tmovq $'\\n', %rdi" );
    puts( "\tcall putchar" );
    // return 0
    puts( "\tmovq $0, %rax" );
    // exit
    puts ( "\tcall exit" );
}
