static int hex_char_to_decimal ( int ch );
static void re_compile_fastmap_aux ( unsigned char *code , int pos , unsigned char *visited , unsigned char *can_be_null , unsigned char *fastmap );
static int re_do_compile_fastmap ( unsigned char *buffer , int used , int pos , unsigned char *can_be_null , unsigned char *fastmap );
static int re_optimize_star_jump ( regexp_t bufp , unsigned char *code );
static int re_optimize ( regexp_t bufp );
