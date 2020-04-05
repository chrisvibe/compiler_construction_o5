#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))

static void generate_stringtable ( void );
static void generate_main ( symbol_t *first );
void generate_program_dummy ( void );
void generate_program ( void );
static void generate_function ( symbol_t *symbol );

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
    puts ( "intout: .string \"\%ld \"" );
    puts ( "strout: .string \"\%s \"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );

    /* TODO:  handle the strings from the program */

    /* add string list variables */ 
    for(int i = 0; i < stringc; i++) {
        char* str = string_list[i];
        printf( "STR%i: .string %s\n", i, str);
    }


    /* add globals */
    /* puts ( ".section .data" ); // mutable data */
    /* size_t n_globals = tlhash_size(global_names); */
    /* symbol_t* global_list[n_globals]; */
    /* tlhash_values(global_names, (void **)&global_list); */
    /* for(int i = 0; i < n_globals; i++) { */
    /*     symbol_t* sym = global_list[i]; */
    /*     /1* symbol_t *value = NULL; *1/ */
    /*     /1* tlhash_lookup(scopes[0], sym->name, strlen(sym->name), (void **)&value ); *1/ */
    /*     /1* printf( "_%s: %s\n", var->name, (char*) value); *1/ */
    /*     switch (sym->type) { */
    /*         case SYM_GLOBAL_VAR: */    
    /*             printf( "_%s: .zero 8\n", var->name); */
    /*         case SYM_FUNCTION: */
    /*             /1* generate_function(var); *1/ */
    /*             break; */
    /*     } */
    /* } */
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
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
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
    printf ( "\tcall\t_%s\n", first->name );
    puts ( "\tjmp END" );
    puts ( "ABORT:" );
    puts ( "\tmovq $errout, %rdi" );
    puts ( "\tcall puts" );

    puts ( "END:" );
    puts ( "\tmovq %rax, %rdi" );
    puts ( "\tcall exit" );
}


void
generate_program_dummy ( void )
{
    generate_stringtable();

    /* Put some dummy stuff to keep the skeleton from crashing */
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tmovq $0, %rax" );
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
    size_t n_globals = tlhash_size(global_names);
    symbol_t* global_list[n_globals];
    tlhash_values(global_names, (void **)&global_list);
    generate_main(global_list[0]);
    generate_function(global_list[0]);
/*  Take the return value from that and return it to the calling shell */
/*  Return to shell */
}
    
static void
generate_function ( symbol_t *symbol )
{
    puts( "_hello:" );
    puts( "\tpushq %rbp" );
    puts( "\tmovq %rsp, %rbp" );
    puts( "\tmovq $STR0, %rsi" );
    puts( "\tmovq $strout, %rdi" );
    puts( "\tcall printf" );
    puts( "\tmovq $'\\n', %rdi" );
    puts( "\tcall putchar" );
    puts( "\tmovq $0, %rax" );
    puts( "\tleave" );
    puts( "\tret" );
}
