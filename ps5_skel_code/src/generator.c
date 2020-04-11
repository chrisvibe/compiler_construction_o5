    /* puts ( "errgen: .string \"GENERIC ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\"" ); */
                /* puts ( "\tmovq $errgen, %rdi" ); */
                /* puts ( "\tcall puts" ); */

#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))

static void generate_stringtable ( void );
static void generate_main ( symbol_t *first );
void generate_program ( void );
static void generate_function ( node_t* node );
static void hello_world ( symbol_t *symbol );
void print_node(node_t* node);
void node_tree_to_assembly( node_t* node );
void handle_global_list();

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
    node_tree_to_assembly(root);
/*  Take the return value from that and return it to the calling shell */
/*  Return to shell */
}
    
static void generate_function ( node_t* node ) {
    // make function label
    printf( "_%s:\n", (char*)node->children[0]->data);

    // push run-time vars to stack
    puts( "\tpushq %rbp" );
    puts( "\tmovq %rsp, %rbp" );
    
    // local mutable data
    /* puts ( ".section .data" ); */

    // generate vsl code from node tree
    node_tree_to_assembly(node);

    // exit
    puts ( "\tcall exit" );
}

void handle_global_list() {
    size_t n_globals = tlhash_size(global_names);
    symbol_t* global_list[n_globals];
    tlhash_values(global_names, (void **)&global_list);
    generate_main(global_list[0]);

    if (n_globals)
        puts ( ".section .data" ); // mutable data
    for(int i = 0; i < n_globals; i++) {
        symbol_t* sym = global_list[i];
        if (sym->type == SYM_GLOBAL_VAR) {
            printf( "_%s: .zero 8\n", sym->name);
        }
    }
}

void node_tree_to_assembly( node_t* node ) {
    for (int i = 0; i < node->n_children; i++) {
        node_t* child = node->children[i];
        if (child) {
            switch(child->type) {
                /* case PROGRAM: */
                /*     break; */
                case GLOBAL_LIST: // add global variables (placeholders)
                    handle_global_list();
                    node_tree_to_assembly(child);
                    break;
                /* case GLOBAL: */
                /*     break; */
                /* case STATEMENT_LIST: */
                /*     break; */
                /* case PRINT_LIST: */
                /*     break; */
                /* case EXPRESSION_LIST: */
                /*     break; */
                /* case VARIABLE_LIST: */
                /*     break; */
                /* case ARGUMENT_LIST: */
                /*     break; */
                /* case PARAMETER_LIST: */
                /*     break; */
                /* case DECLARATION_LIST: */
                    /* break; */
                case FUNCTION: // nested function
                    generate_function(child);
                    break;
                /* case STATEMENT: */
                /*     break; */
                /* case BLOCK: */
                /*     break; */
                /* case ASSIGNMENT_STATEMENT: */
                /*     break; */
                case RETURN_STATEMENT:
                    puts( "\tmovq $0, %rax" ); // return 0
                    break;
                case PRINT_STATEMENT:
                    print_node(child);
                    break;
                /* case NULL_STATEMENT: */
                /*     break; */
                /* case IF_STATEMENT: */
                /*     break; */
                /* case WHILE_STATEMENT: */
                /*     break; */
                /* case EXPRESSION: */
                /*     break; */
                /* case RELATION: */
                /*     break; */
                /* case DECLARATION: */
                /*     break; */
                /* case PRINT_ITEM: */
                /*     break; */
                /* case IDENTIFIER_DATA: */
                    /* printf( "_%s: .zero 8\n", child->entry->name); */
                    /* break; */
                /* case NUMBER_DATA: */
                /*     break; */
                /* case STRING_DATA: */
                /*     break; */
                default:
                    node_tree_to_assembly( child );
                    break;
            }
        }
    }
}


void print_node(node_t* node) {
    for (int i = 0; i < node->n_children; i++) {
        node_t* child = node->children[i];

        switch(child->type) {
            case STRING_DATA:
                // print a string 
                puts( "\tmovq $strout, %rdi" );
                printf( "\tmovq $STR%zu, %%rsi\n", *((size_t *)child->data) );
                puts( "\tcall printf" );
                break;
            case IDENTIFIER_DATA:
                // print an int 
                puts( "\tmovq $intout, %rdi" );
                /* symbol_t *value = NULL; */
                /* tlhash_lookup(scopes[0], sym->name, strlen(sym->name), (void **)&value ); */
                /* printf( "\tmovq $%i, %%rsi\n", value); */
                puts( "\tcall printf" );
                break;
            default:
                // error msg: symbol not printable 
                puts ( "\tmovq $errprint, %rdi" );
                puts ( "\tcall puts" );
                break;
        }
        // print newline
        puts( "\tmovq $'\\n', %rdi" );
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
