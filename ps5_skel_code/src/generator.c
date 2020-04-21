
            /* printf("\tmovq $errgen, %s\n", record[0]); */
            /* puts("\tcall puts"); */

#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
#define return_rec "%rax"

static void generate_stringtable ( void );
static void generate_main ( symbol_t *first );
void generate_program ( void );
static void generate_function(symbol_t* sym);
void node_to_assembly(node_t* node, int n_parms);
void tree_to_assembly(node_t* node, int n_parms);
void handle_global_list();
void handle_print_statement(node_t* print_statement, int n_parms);
void handle_pluss_minus(char* assembly_op, node_t* node, int n_parms);
void handle_mult_div(char* assembly_op, node_t* node, int n_parms);
void handle_comparator(char* assembly_op, node_t* node, int n_parms);
void handle_if_statement(node_t* node, int n_parms);

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

    puts ( "\tsubq $1, %rdi" ); // allocate stack space
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
    node_to_assembly(root, 0);
}
    
static void generate_function (symbol_t* sym) {

    // make function label
    puts ( ".section .text" );
    printf( "_%s:\n", sym->name);
    
    // push run time variables to stack
    puts ( "\tpushq %rbp" );  // save old base pointer to stack
    puts ( "\tmovq %rsp, %rbp" );  // current stack location: new base
    
    // load parameters -8(%rbp) stores first parameter and so on
    for (int i = 0; i < sym->nparms; i++) {  // below 2 lines are equivalent
        /* printf( "\tmovq %s, -%i(%%rbp)\n", record[i], (i+1)*8); */
        printf( "\tpushq %s\n", record[i]); 
    }

    // allocate space on stack 
    int stack_size = sym->locals->size;
    printf("\tsubq $%i, %%rsp\n" , 8 * stack_size);
    if (((sym->locals->size+sym->nparms) * 8) % 16 != 0) { // 16 byte allignment dont ask me why
        puts("\tpushq $0");
    }
    
    // generate vsl code from node tree
    tree_to_assembly(sym->node, sym->nparms);

    // restore previos base pointer
    puts( "\tleave" );
    puts( "\tret" );

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
        } else if (sym->type == SYM_FUNCTION) {
            if (!sym->seq)
                first = sym;
        }
    }
    generate_main(first);
    for(int i = 0; i < n_globals; i++) {
        symbol_t* sym = global_list[i];
        if (sym->type == SYM_FUNCTION) {
            generate_function(sym);
        }
    }
}

void node_to_assembly( node_t* node, int n_parms) {
    if (node) {
        symbol_t* sym;
        switch(node->type) {
            case PROGRAM:
                tree_to_assembly(node, n_parms);
                break;
            case GLOBAL_LIST:
                handle_global_list();
                tree_to_assembly(node, n_parms);
                break;
            case GLOBAL:
                tree_to_assembly(node, n_parms);
                break;
            case STATEMENT_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case PRINT_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case EXPRESSION_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case VARIABLE_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case ARGUMENT_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case PARAMETER_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case DECLARATION_LIST:
                tree_to_assembly(node, n_parms);
                break;
            case FUNCTION:
                break;
            case STATEMENT:
                tree_to_assembly(node, n_parms);
                break;
            case BLOCK:
                tree_to_assembly(node, n_parms);
                break;
            case RETURN_STATEMENT:
                node_to_assembly(node->children[0], n_parms);
                break;
            case ASSIGNMENT_STATEMENT: ;
                // identifier always first child of assignment_statement
                sym = node->n_children ? node->children[0]->entry : NULL;
                if (sym) {
                    switch (sym->type) {
                        case SYM_LOCAL_VAR: // TODO what if many
                            node_to_assembly(node->children[1], n_parms); // resolve expression
                            printf("\tmovq %s, -%i(%%rbp)\n", return_rec, (int)(8 * (sym->seq + MIN(n_parms, 6) + 1)));
                            break;
                        case SYM_GLOBAL_VAR:
                            node_to_assembly(node->children[1], n_parms); // resolve expression
                            printf("\tmovq %s, _%s\n", return_rec, sym->name);
                            break;
                    }
                }
                break;
            case PRINT_STATEMENT:
                handle_print_statement(node, n_parms);
                break;
            case NULL_STATEMENT:
                break;
            case IF_STATEMENT:
                handle_if_statement(node, n_parms);
                break;
            case WHILE_STATEMENT:
                break;
            case EXPRESSION:
                if (node->data) {
                    switch (*((char*)node->data)){
                        case '+':
                            handle_pluss_minus("addq", node, n_parms);
                            break;
                        case '-':
                            if (node->n_children == 2) {
                                handle_pluss_minus("subq", node, n_parms);
                            } else {
                                node_to_assembly(node->children[0], n_parms); // arg1 on return_rec
                                printf("\tnegq %s\n", return_rec);
                            }
                            break;
                        case '*':
                            handle_mult_div("imulq", node, n_parms);
                            break;
                        case '/':
                            handle_mult_div("idivq", node, n_parms);
                            break;
                        case '~':
                            node_to_assembly(node->children[0], n_parms); // arg1 on return_rec
                            printf("\tnotq %s\n", return_rec);
                            break;
                        case '|':
                            node_to_assembly(node->children[0], n_parms); // arg1 on return_rec
                            printf("\torq %s\n", return_rec);
                            break;
                        case '&':
                            node_to_assembly(node->children[0], n_parms); // arg1 on return_rec
                            printf("\tandq %s\n", return_rec);
                            break;
                        case '^':
                            node_to_assembly(node->children[0], n_parms); // arg1 on return_rec
                            printf("\txorq %s\n", return_rec);
                            break;
                        case '<':
                            handle_comparator("addq", node, n_parms);
                            break;
                    }
                } else if (node->n_children == 2) {
                    // function call identifier and expression list of parameters are 
                    // not indented always indented on same level. Thats why this is here... pretty sure parameters should be in parameter list! :(
                    // Cant access parameters from identifier node.
                    sym = node->children[0]->entry;
                    if (sym->type == SYM_FUNCTION && node->children[1]->type == EXPRESSION_LIST) {
                        // load into record
                        for (int i = 0; i < sym->nparms; i++) {
                            node_to_assembly(node->children[1]->children[i], n_parms);
                            printf( "\tmovq %s, %s\n", return_rec, record[i]);
                        }
                        printf("\tcall _%s\n", sym->name);
                    } else {
                        tree_to_assembly(node, n_parms);
                    }
                } else {
                    tree_to_assembly(node, n_parms);
                }
                break;
            case RELATION:
                if (node->n_children == 2) {
                    switch (*((char*)node->data)){
                        case '<':
                            break;
                    }
                }
                break;
            case DECLARATION:
                break;
            case PRINT_ITEM:
                handle_print_statement(node, n_parms);
                break;
            case NUMBER_DATA:
                printf("\tmovq $%zu, %s\n", *((size_t *)node->data), return_rec);
                break;
            case STRING_DATA:
                printf("\tmovq $STR%zu, %s\n", *((size_t *)node->data), return_rec);
                break;
            case IDENTIFIER_DATA: ;
                sym = node->entry;
                if (sym) {
                    switch (sym->type) {
                        case SYM_GLOBAL_VAR:
                            printf("\tmovq _%s, %s\n", sym->name, return_rec);
                            break;
                        case SYM_LOCAL_VAR: // TODO what if many
                            printf("\tmovq -%i(%%rbp), %s\n", (int)(8 * (sym->seq + MIN(n_parms, 6) + 1)), return_rec);
                            break;
                        case SYM_PARAMETER: // TODO what if many
                            printf("\tmovq -%i(%%rbp), %s\n", (int)(8 * (sym->seq + 1)), return_rec);
                            break;
                        case SYM_FUNCTION: ;
                            // put parameters in registers TODO what if many
                            for (int i = 0; i < sym->nparms; i++) {
                                node_to_assembly(node->children[i], n_parms);
                                printf( "\tmovq %s, %s\n", return_rec, record[i]);
                            }
                            printf("\tcall _%s\n", sym->name);
                            break;
                    } 
                }
                break;
            default:
                printf("\tmovq $errgen, %s\n", record[0]);
                puts("\tcall puts");
                break;

        }
    }
}


void tree_to_assembly(node_t* node, int n_parms) {
    for (int i = 0; i < node->n_children; i++) {
        node_t* child = node->children[i];
        node_to_assembly(child, n_parms);
    } 
}

void handle_print_statement(node_t* print_statement, int n_parms) {
    for (int i = 0; i < print_statement->n_children; i++) {
        node_t* print_item = print_statement->children[i];
        node_to_assembly(print_item, n_parms);  // print_item should now reside in rax
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
                break;
        }
        puts( "\tcall printf" );
        // print newline
        printf( "\tmovq $'\\n', %s\n", record[0]);
        puts( "\tcall putchar" );
    }
}

// binary in terms of assembly operands (implicit parameter passing)
void handle_pluss_minus(char* assembly_op, node_t* node, int n_parms) {
    node_to_assembly(node->children[1], n_parms); // arg2
    printf("\tpushq %s\n", return_rec); // arg2 on stack
    node_to_assembly(node->children[0], n_parms); // arg1
    printf("\t%s %s, (%%rsp)\n", assembly_op, return_rec);
    printf("\tpopq %s\n", return_rec);
}

// unary in terms of assembly operands (implicit parameter passing)
void handle_mult_div(char* assembly_op, node_t* node, int n_parms) {
    printf("\tpushq %s\n", record[0]);
    node_to_assembly(node->children[1], n_parms);
    printf("\tpushq %s\n", return_rec);
    node_to_assembly(node->children[0], n_parms);
    puts("\tcqo");
    printf("\t%s (%%rsp)\n", assembly_op);
    printf("\tpopq %s\n", record[0]);
    printf("\tpopq %s\n", record[0]);
}
/* void handle_mult_div(char* assembly_op, node_t* node, int n_parms) { */
/*     node_to_assembly(node->children[1], n_parms); // arg2 on return_rec */
/*     printf("\tpushq %s\n", return_rec); // arg2 temp on stack */
/*     node_to_assembly(node->children[0], n_parms); // arg1 on return_rec */
/*     printf("\tpopq %s\n", record[0]); // recover arg2 from stack */
/*     printf("\t%s (%%rsp)\n", assembly_op); // result stored in rax implicitly */
/* } */

void handle_comparator(char* assembly_op, node_t* node, int n_parms) {
    node_to_assembly(node->children[0], n_parms); // arg1 on return_rec
    printf("\tmovq %s, %s\n", return_rec, record[0]); // arg1 on record[0]
    node_to_assembly(node->children[1], n_parms); // arg2 on return_rec
    printf("\t%s %s, %s\n", assembly_op, record[0], return_rec); // set flags
}

// TODO not complete atm and should be recursive
void handle_if_statement(node_t* node, int n_parms) {
    static int id = 0;

    // set flags by resolving boolean
    node_to_assembly(node->children[0], n_parms);

    // figure out target_false location
    char* target_false;

    // route to target_false or continue
    switch (*((char*)node->data)) {
        case '=':
            printf("\tjne (%s)\n", target_false);
            break;
        case '>':
            printf("\tjle (%s)\n", target_false);
            break;
        case '<':
            printf("\tjge (%s)\n", target_false);
            break;
    }
    
    // translate true path
    node_to_assembly(node->children[0], n_parms);

    // translate false path
    node_to_assembly(node->children[0], n_parms);

    // make labels for targets
    char* label;
    printf("\t\t%s%d\n", label, id++);
}
